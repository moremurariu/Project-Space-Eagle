// pf: brute-force a pre-fired grenade plus a point-blank follow-up shot on a fixed input sequence (e.g. Teero's
// shaft-exit stack). The base inputs keep everything but the shots.
// Stage 1: every fire tick t1 in [t1lo, t1hi] and aim (res degrees in [a0, a1)): the projectile's explosion point and
//   tick are deterministic and the tee's path is the base path until then, so kicks are found without simulating;
//   candidates with an explosion within 140 px of the tee are simulated and ranked by the kick along dir.
// Stage 2: for the best `keep` stage-1 shots, a second shot at t2 in [t1 + reload, explosion + 2] at res2 degrees.
// Every combination (and the single shot) is scored by an estimated time to reach the line dot(pos, dir) = xf:
//   end + (xf - dot(pos, dir)) / dot(vel, dir), end = input index one after the last explosion.
// The best distinct results are written as OUT<k>.txt (base inputs up to end + 1 with the shots).
// usage: pf <map> base=FILE t1lo=N t1hi=N tend=N xf=X dir=1,0 [res=0.05 a0=0 a1=360 res2=1 keep=300 top=8 out=PREFIX threads=4]
//   rel=1: t1lo/t1hi count from the grenade pickup; tend<=0: counts back from the end of the base
//   survive=15: the result must stay out of freeze for this many ticks under a simple continuation (0 = off)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "sim.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	if(!f)
		return v;
	int d, j, h, fi, tx, ty, w;
	while(std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
	{
		STasInput In;
		In.m_Dir = d;
		In.m_Jump = j;
		In.m_Hook = h;
		In.m_Fire = fi;
		In.m_TX = tx;
		In.m_TY = ty;
		In.m_Weapon = w;
		v.push_back(In);
	}
	std::fclose(f);
	return v;
}

static void WriteInputs(const std::string &Path, const std::vector<STasInput> &v)
{
	FILE *f = std::fopen(Path.c_str(), "w");
	for(const auto &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

struct SShot
{
	int m_T = -1;
	float m_Aim = 0;
};

struct SCand
{
	SShot m_S1, m_S2;
	int m_End = 0; // input index after which the kicks are done
	float m_Score = 1e9f; // estimated tick of reaching xf
	float m_Kick = 0;
	vec2 m_Pos, m_Vel;
	bool m_Alive = true; // survives on its own (without a follow-up shot)
};

static vec2 g_Dir(1, 0);
static float g_Xf = 0;
static std::vector<STasInput> g_vBase;

static STasInput ShotInput(STasInput In, float Aim)
{
	In.m_Fire = 1;
	In.m_Weapon = 3;
	In.m_TX = (int16_t)std::lround(std::cos(Aim * pi / 180) * 1000);
	In.m_TY = (int16_t)std::lround(std::sin(Aim * pi / 180) * 1000);
	return In;
}

// a shot can't share a tick with a hook launch (the shot's aim would also aim the hook)
static bool HookLaunch(int k)
{
	return g_vBase[k].m_Hook && !(k > 0 && g_vBase[k - 1].m_Hook);
}

// steps input k of the base with the given shots; false if the tee dies
static bool StepK(CTasGame &G, int k, const SShot &S1, const SShot &S2)
{
	STasInput In = g_vBase[k];
	In.m_Fire = 0;
	if(k == S1.m_T)
		In = ShotInput(In, S1.m_Aim);
	else if(k == S2.m_T)
		In = ShotInput(In, S2.m_Aim);
	G.Step(In);
	return !(G.Frozen() || G.EnteredFreeze() || G.m_StartTick == -2);
}

static int g_Survive = 15;
// some simple continuation (direction held, hook kept or released) stays out of freeze for g_Survive ticks
static bool Survives(const CTasGame &G)
{
	if(g_Survive <= 0)
		return true;
	vec2 aPos[128];
	const int N = std::min(g_Survive, 128);
	for(int d = 1; d >= -1; d--)
	{
		if(G.Rollout(N, d, true, aPos) == N)
			return true;
		if(G.Rollout(N, d, false, aPos) == N)
			return true;
	}
	return false;
}

static float Score(const CTasGame &G, int End)
{
	float Along = dot(G.Vel(), g_Dir);
	return End + (g_Xf - dot(G.Pos(), g_Dir)) / std::max(Along, 1.0f);
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
		return 1;
	std::string Base, Out = "pf_";
	int T1lo = 0, T1hi = 0, TEnd = 0, Top = 8, Threads = 4, Keep = 300, Rel = 0;
	float Res = 0.05f, Res2 = 1.0f, A0 = 0, A1 = 360;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "base") Base = V;
		else if(K == "t1lo") T1lo = std::stoi(V);
		else if(K == "t1hi") T1hi = std::stoi(V);
		else if(K == "tend") TEnd = std::stoi(V);
		else if(K == "xf") g_Xf = std::stof(V);
		else if(K == "res") Res = std::stof(V);
		else if(K == "res2") Res2 = std::stof(V);
		else if(K == "a0") A0 = std::stof(V);
		else if(K == "a1") A1 = std::stof(V);
		else if(K == "top") Top = std::stoi(V);
		else if(K == "keep") Keep = std::stoi(V);
		else if(K == "rel") Rel = std::stoi(V);
		else if(K == "survive") g_Survive = std::stoi(V);
		else if(K == "threads") Threads = std::stoi(V);
		else if(K == "out") Out = V;
		else if(K == "dir") std::sscanf(V.c_str(), "%f,%f", &g_Dir.x, &g_Dir.y);
		else
		{
			std::printf("unknown key %s\n", K.c_str());
			return 1;
		}
	}
	g_Dir = normalize(g_Dir);
	g_vBase = ReadInputs(Base.c_str());
	if(Rel)
	{
		// t1lo/t1hi are relative to the grenade pickup (first input after which the tee has it)
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		int Pick = -1;
		for(int k = 0; k < (int)g_vBase.size() && Pick < 0; k++)
		{
			G.Step(g_vBase[k]);
			if(G.HasGrenade())
				Pick = k + 1;
		}
		std::printf("pickup after input %d\n", Pick);
		std::fflush(stdout);
		if(Pick < 0)
			return 1;
		T1lo += Pick;
		T1hi += Pick;
	}
	if(TEnd <= 0)
		TEnd += (int)g_vBase.size() - 3;
	TEnd = std::min(TEnd, (int)g_vBase.size() - 3);
	T1hi = std::min(T1hi, TEnd);
	if(T1lo < 0 || T1lo > T1hi)
	{
		std::printf("empty fire range %d..%d\n", T1lo, T1hi);
		return 1;
	}
	// base path: states before each input index from t1lo, positions/velocities after each input
	const int N = TEnd + 3;
	std::vector<std::unique_ptr<CTasGame>> vAt(N);
	std::vector<vec2> vPos(N), vVel(N);
	{
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(int k = 0; k < N; k++)
		{
			if(k >= T1lo)
			{
				vAt[k] = std::make_unique<CTasGame>();
				vAt[k]->CopyFrom(G);
			}
			STasInput In = g_vBase[k];
			if(k >= T1lo)
				In.m_Fire = 0;
			G.Step(In);
			vPos[k] = G.Pos();
			vVel[k] = G.Vel();
		}
		std::printf("base end %d: pos %.0f %.0f vel %.2f %.2f score %.1f\n", TEnd, vPos[TEnd].x, vPos[TEnd].y, vVel[TEnd].x, vVel[TEnd].y, Score(G, N));
	}
	// stage 1
	const int NA = (int)std::ceil((A1 - A0) / Res);
	std::vector<std::vector<SCand>> vThr(Threads);
	std::atomic<long> Next{0};
	const long Total = (long)(T1hi - T1lo + 1) * NA;
	auto W1 = [&](int T) {
		CTasGame G;
		while(true)
		{
			long j = Next.fetch_add(1);
			if(j >= Total)
				break;
			int t1 = T1lo + (int)(j / NA);
			float Aim = A0 + (j % NA) * Res;
			const CTasGame &S = *vAt[t1];
			if(S.ReloadTimer() > 0 || !S.HasGrenade() || HookLaunch(t1))
				continue;
			G.CopyFrom(S);
			SShot S1{t1, Aim}, S2;
			if(!StepK(G, t1, S1, S2))
				continue;
			vec2 P;
			int Te;
			if(!G.NextExplosion(P, Te))
				continue;
			int Je = t1 + (Te - G.m_Tick); // input index whose step explodes it
			if(Je > TEnd || distance(vPos[Je], P) > 140)
				continue;
			bool Ok = true;
			for(int k = t1 + 1; k <= Je && Ok; k++)
				Ok = StepK(G, k, S1, S2);
			if(!Ok)
				continue;
			vec2 Kick = G.Vel() - vVel[Je];
			if(dot(Kick, g_Dir) < 1.0f)
				continue;
			SCand C;
			C.m_S1 = S1;
			C.m_End = Je;
			C.m_Kick = dot(Kick, g_Dir);
			C.m_Score = Score(G, Je + 1);
			C.m_Pos = G.Pos();
			C.m_Vel = G.Vel();
			C.m_Alive = Survives(G);
			vThr[T].push_back(C);
		}
	};
	{
		std::vector<std::thread> vTh;
		for(int T = 1; T < Threads; T++)
			vTh.emplace_back(W1, T);
		W1(0);
		for(auto &Th : vTh)
			Th.join();
	}
	std::vector<SCand> vOne;
	for(auto &x : vThr)
		vOne.insert(vOne.end(), x.begin(), x.end());
	std::printf("stage 1: %zu kicks\n", vOne.size());
	// keep the best few aims per (t1, explosion tick)
	std::sort(vOne.begin(), vOne.end(), [](const SCand &a, const SCand &b) { return a.m_Score < b.m_Score; });
	std::map<std::pair<int, int>, std::vector<float>> mSeen;
	std::vector<SCand> vKeep;
	for(const auto &C : vOne)
	{
		auto &vA = mSeen[{C.m_S1.m_T, C.m_End}];
		bool Dup = (int)vA.size() >= 4;
		for(float a : vA)
			Dup |= std::fabs(a - C.m_S1.m_Aim) < 0.3f;
		if(Dup)
			continue;
		vA.push_back(C.m_S1.m_Aim);
		vKeep.push_back(C);
		if((int)vKeep.size() >= Keep)
			break;
	}
	for(int i = 0; i < std::min(10, (int)vKeep.size()); i++)
		std::printf("  s1 t1 %d aim %.2f explodes %d kick %.2f vel %.2f %.2f score %.1f\n", vKeep[i].m_S1.m_T, vKeep[i].m_S1.m_Aim, vKeep[i].m_End, vKeep[i].m_Kick, vKeep[i].m_Vel.x, vKeep[i].m_Vel.y, vKeep[i].m_Score);
	// stage 2
	const int NA2 = (int)std::ceil(360 / Res2);
	std::vector<std::vector<SCand>> vThr2(Threads);
	std::atomic<int> NextC{0};
	auto W2 = [&](int T) {
		CTasGame G, H;
		while(true)
		{
			int c = NextC.fetch_add(1);
			if(c >= (int)vKeep.size())
				break;
			const SCand &C1 = vKeep[c];
			if(C1.m_Alive)
				vThr2[T].push_back(C1);
			G.CopyFrom(*vAt[C1.m_S1.m_T]);
			SShot S1 = C1.m_S1, S0;
			const int T2lo = C1.m_S1.m_T + 1, T2hi = std::min(C1.m_End + 2, TEnd);
			bool Ok = true;
			for(int k = C1.m_S1.m_T; k < T2lo && Ok; k++)
				Ok = StepK(G, k, S1, S0);
			for(int t2 = T2lo; t2 <= T2hi && Ok; t2++)
			{
				if(G.ReloadTimer() == 0 && G.HasGrenade() && !HookLaunch(t2))
				{
					for(int a = 0; a < NA2; a++)
					{
						SShot S2{t2, a * Res2};
						H.CopyFrom(G);
						bool Ok2 = StepK(H, t2, S1, S2);
						vec2 P;
						int Te;
						if(!Ok2 || !H.NextExplosion(P, Te))
							continue;
						int Je = std::max(C1.m_End, t2 + (Te - H.m_Tick));
						if(Je > TEnd)
							continue;
						for(int k = t2 + 1; k <= Je && Ok2; k++)
							Ok2 = StepK(H, k, S1, S2);
						if(!Ok2 || !Survives(H))
							continue;
						SCand C = C1;
						C.m_S2 = S2;
						C.m_End = Je;
						C.m_Score = Score(H, Je + 1);
						C.m_Pos = H.Pos();
						C.m_Vel = H.Vel();
						if(C.m_Score < C1.m_Score)
							vThr2[T].push_back(C);
					}
				}
				Ok = StepK(G, t2, S1, S0);
			}
		}
	};
	{
		std::vector<std::thread> vTh;
		for(int T = 1; T < Threads; T++)
			vTh.emplace_back(W2, T);
		W2(0);
		for(auto &Th : vTh)
			Th.join();
	}
	std::vector<SCand> vAll;
	for(auto &x : vThr2)
		vAll.insert(vAll.end(), x.begin(), x.end());
	std::sort(vAll.begin(), vAll.end(), [](const SCand &a, const SCand &b) { return a.m_Score < b.m_Score; });
	std::vector<SCand> vPick;
	for(const auto &R : vAll)
	{
		bool Dup = false;
		for(const auto &P : vPick)
			Dup |= P.m_S1.m_T == R.m_S1.m_T && std::fabs(P.m_S1.m_Aim - R.m_S1.m_Aim) < 1.0f && P.m_S2.m_T == R.m_S2.m_T;
		if(Dup)
			continue;
		vPick.push_back(R);
		if((int)vPick.size() >= Top)
			break;
	}
	for(int k = 0; k < (int)vPick.size(); k++)
	{
		const SCand &R = vPick[k];
		std::vector<STasInput> vO(g_vBase.begin(), g_vBase.begin() + R.m_End + 1);
		for(int i = T1lo; i <= R.m_End; i++)
			vO[i].m_Fire = 0;
		vO[R.m_S1.m_T] = ShotInput(vO[R.m_S1.m_T], R.m_S1.m_Aim);
		if(R.m_S2.m_T >= 0)
			vO[R.m_S2.m_T] = ShotInput(vO[R.m_S2.m_T], R.m_S2.m_Aim);
		std::string Path = Out + std::to_string(k) + ".txt";
		WriteInputs(Path, vO);
		std::printf("PF %d score %.1f t1 %d aim %.2f t2 %d aim %.1f end %d pos %.0f %.0f vel %.2f %.2f -> %s\n", k, R.m_Score, R.m_S1.m_T, R.m_S1.m_Aim,
			R.m_S2.m_T, R.m_S2.m_Aim, R.m_End, R.m_Pos.x, R.m_Pos.y, R.m_Vel.x, R.m_Vel.y, Path.c_str());
	}
	return 0;
}
