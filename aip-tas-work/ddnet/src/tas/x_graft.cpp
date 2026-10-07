// x_graft: graft a faster prefix onto a complete run (exact CFastG stepper).
//
// The prefix run (e.g. a pre-grenade run that dives into the pickup) is cut at race tick C. From there a beam
// search steers towards the complete run's own states D ticks further on (the prefix is ahead by D ticks on the
// same line): states are ranked by
//   |dpos| + wv |dvel| + wh [hook state differs] + 0.25 |dhookpos| + wj [jump state differs] + wr [reload differs]
// against the complete run's state at (race tick + D). Every step the `test` closest states are continued with the
// complete run's own remaining inputs; a continuation that reaches the finish without freezing is a graft, written
// as prefix + searched inputs + the complete run's tail. The best (earliest finish) graft over the whole horizon is
// kept. Actions: dir -1/0/1 x jump 0/1 x hook (release / hold / press at `angles` evenly spaced aims, fine aims
// around the target's own hook direction and the target run's own input at the matched tick).
// usage: x_graft <map> prefix=FILE cut=C run=FILE D=6 [horizon=40 beam=20000 angles=64 test=200 threads=4
//        wv=4 wh=30 wj=60 wr=5 out=FILE]
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "sim.h"
#include "tasfast.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	int d, j, h, fi, tx, ty, w;
	while(f && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
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
	if(f)
		std::fclose(f);
	return v;
}

static void WriteInputs(const char *pPath, const std::vector<STasInput> &v)
{
	FILE *f = std::fopen(pPath, "w");
	for(const auto &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

struct SNode
{
	int m_Parent;
	STasInput m_In;
};

struct SState
{
	CTasFast m_S;
	int m_Node;
	float m_D;
	STasInput m_Prev;
};

struct SCand
{
	float m_D;
	int m_Parent; // index into the current beam
	uint64_t m_Hash;
	STasInput m_In;
};

static uint64_t StateHash(const CTasFast &S, const STasInput &In)
{
	uint64_t h = 1469598103934665603ull;
	auto Mix = [&](int64_t v) { h = (h ^ (uint64_t)v) * 1099511628211ull; };
	Mix((int64_t)S.Pos().x);
	Mix((int64_t)S.Pos().y);
	Mix((int64_t)std::lround(S.Vel().x * 256));
	Mix((int64_t)std::lround(S.Vel().y * 256));
	Mix(S.HookState());
	Mix((int64_t)S.HookPos().x);
	Mix((int64_t)S.HookPos().y);
	Mix(S.Jumped() * 16 + S.m_Core.m_JumpedTotal);
	Mix(In.m_Hook);
	Mix(S.ReloadTimer());
	return h;
}

static std::vector<CTasFast> gs_vRun; // complete run's state after input i (index = input count)
static std::vector<STasInput> gs_vRunIn;
static int gs_D = 6;
static float gs_Wv = 4, gs_Wh = 30, gs_Wj = 60, gs_Wr = 5;
static int gs_RunFin = -1;

static int RunIdx(const CTasFast &S) { return S.m_Tick + gs_D; } // inputs applied = server tick (spawn at tick 0)

static float Dist(const CTasFast &S)
{
	const int i = RunIdx(S);
	if(i < 0 || i >= (int)gs_vRun.size())
		return 1e9f;
	const CTasFast &R = gs_vRun[i];
	float d = std::fabs(S.Pos().x - R.Pos().x) + std::fabs(S.Pos().y - R.Pos().y);
	d += gs_Wv * (std::fabs(S.Vel().x - R.Vel().x) + std::fabs(S.Vel().y - R.Vel().y));
	if(S.HookState() != R.HookState())
		d += gs_Wh;
	else if(R.HookState() != 0)
		d += 0.25f * distance(S.HookPos(), R.HookPos());
	if(S.Jumped() != R.Jumped() || S.m_Core.m_JumpedTotal != R.m_Core.m_JumpedTotal)
		d += gs_Wj;
	if(S.ReloadTimer() != R.ReloadTimer() || S.HasGrenade() != R.HasGrenade())
		d += gs_Wr;
	return d;
}

// continue S with the complete run's remaining inputs: finish race tick or -1
static int Cont(const CTasFast &S)
{
	CTasFast F = S;
	for(int i = RunIdx(S); i < (int)gs_vRunIn.size(); i++)
	{
		F.Step(gs_vRunIn[i]);
		if(F.m_Dead || F.m_StartTick == -2 || F.m_Bad)
			return -1;
		if(F.m_FinishTick >= 0)
			return F.m_FinishTick - F.m_StartTick;
	}
	return -1;
}

int main(int argc, const char **argv)
{
	if(argc < 2)
	{
		std::printf("usage: x_graft <map> prefix=FILE cut=C run=FILE D=6 [horizon beam angles test threads wv wh wj wr out]\n");
		return 1;
	}
	std::string Prefix, Run, Out = "graft.txt";
	int Cut = -1, Horizon = 40, Beam = 20000, Angles = 64, Test = 200, Threads = 4;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t e = A.find('=');
		if(e == std::string::npos)
			continue;
		std::string K = A.substr(0, e), V = A.substr(e + 1);
		if(K == "prefix") Prefix = V;
		else if(K == "run") Run = V;
		else if(K == "out") Out = V;
		else if(K == "cut") Cut = std::stoi(V);
		else if(K == "D") gs_D = std::stoi(V);
		else if(K == "horizon") Horizon = std::stoi(V);
		else if(K == "beam") Beam = std::stoi(V);
		else if(K == "angles") Angles = std::stoi(V);
		else if(K == "test") Test = std::stoi(V);
		else if(K == "threads") Threads = std::stoi(V);
		else if(K == "wv") gs_Wv = std::stof(V);
		else if(K == "wh") gs_Wh = std::stof(V);
		else if(K == "wj") gs_Wj = std::stof(V);
		else if(K == "wr") gs_Wr = std::stof(V);
		else
		{
			std::printf("unknown key %s\n", K.c_str());
			return 1;
		}
	}
	CTasGame::LoadMap(argv[1]);
	CFastG::Init();
	std::vector<STasInput> vPre = ReadInputs(Prefix.c_str());
	gs_vRunIn = ReadInputs(Run.c_str());
	if(Cut < 0 || Cut + 68 > (int)vPre.size() || gs_vRunIn.empty())
	{
		std::printf("bad prefix / cut / run\n");
		return 1;
	}
	vPre.resize(Cut + 68);
	// the complete run on the exact stepper (its own finish is the bar)
	{
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		CTasFast F;
		F.FromGameFull(G);
		gs_vRun.push_back(F);
		for(const auto &In : gs_vRunIn)
		{
			F.Step(In);
			gs_vRun.push_back(F);
			if(F.m_FinishTick >= 0 && gs_RunFin < 0)
				gs_RunFin = F.m_FinishTick - F.m_StartTick;
		}
	}
	CTasFast Start;
	{
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(const auto &In : vPre)
			G.Step(In);
		Start.FromGameFull(G);
	}
	std::printf("run %s finishes at %d; prefix %s cut at race tick %d (state rt %d), D %d -> a graft finishes by %d at best\n", Run.c_str(), gs_RunFin,
		Prefix.c_str(), Cut, Start.RaceTick(), gs_D, gs_RunFin - gs_D);

	std::vector<std::vector<SNode>> vLayers;
	std::vector<SState> vBeam;
	vBeam.push_back({Start, -1, Dist(Start), vPre.back()});
	int BestFin = 1 << 30;
	std::vector<STasInput> vBest;
	auto T0 = std::chrono::steady_clock::now();
	for(int Step = 0; Step < Horizon && !vBeam.empty(); Step++)
	{
		// expand: children are scored and hashed, only (parent, input, score, hash) is kept
		std::vector<std::vector<SCand>> vOut(Threads);
		std::atomic<int> Next{0};
		auto Work = [&](int t) {
			std::vector<STasInput> vAct;
			std::vector<std::pair<int, std::pair<int, int>>> vHook; // (hook, aim)
			CTasFast C;
			for(int k; (k = Next.fetch_add(1)) < (int)vBeam.size();)
			{
				const SState &P = vBeam[k];
				const CTasFast &S = P.m_S;
				vAct.clear();
				vHook.clear();
				const int Ri = RunIdx(S);
				const STasInput *pShadow = Ri >= 0 && Ri < (int)gs_vRunIn.size() ? &gs_vRunIn[Ri] : nullptr;
				vHook.push_back({0, {P.m_Prev.m_TX, P.m_Prev.m_TY}});
				if(P.m_Prev.m_Hook)
					vHook.push_back({1, {P.m_Prev.m_TX, P.m_Prev.m_TY}});
				else
				{
					for(int a = 0; a < Angles; a++)
					{
						float Ang = a * 2 * pi / Angles;
						vHook.push_back({1, {(int)std::lround(std::cos(Ang) * 10000), (int)std::lround(std::sin(Ang) * 10000)}});
					}
					// fine aims around the target's hook direction (where the target hooks next)
					const int Rj = std::min((int)gs_vRun.size() - 1, Ri + 3);
					if(Rj >= 0)
					{
						const CTasFast &R = gs_vRun[Rj];
						if(R.HookState() != 0)
						{
							vec2 Dv = R.HookPos() - S.Pos();
							float Base = std::atan2(Dv.y, Dv.x);
							for(float o : {-4.f, -2.f, -1.f, -0.5f, 0.f, 0.5f, 1.f, 2.f, 4.f})
							{
								float Ang = Base + o * pi / 180;
								vHook.push_back({1, {(int)std::lround(std::cos(Ang) * 10000), (int)std::lround(std::sin(Ang) * 10000)}});
							}
						}
					}
				}
				const bool CanJump = !(S.Jumped() & 2) || S.Grounded();
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int J = 0; J <= (CanJump ? 1 : 0); J++)
						for(auto &H : vHook)
						{
							STasInput In = P.m_Prev;
							In.m_Dir = Dir;
							In.m_Jump = J;
							In.m_Hook = H.first;
							In.m_TX = H.second.first;
							In.m_TY = H.second.second;
							In.m_Fire = 0;
							vAct.push_back(In);
						}
				if(pShadow)
					vAct.push_back(*pShadow);
				for(const auto &In : vAct)
				{
					C = S;
					C.Step(In);
					if(C.m_Dead || C.m_StartTick == -2)
						continue;
					vOut[t].push_back({Dist(C), k, StateHash(C, In), In});
				}
			}
		};
		std::vector<std::thread> vT;
		for(int t = 0; t < Threads; t++)
			vT.emplace_back(Work, t);
		for(auto &T : vT)
			T.join();
		std::vector<SCand> vAll;
		for(auto &V : vOut)
			vAll.insert(vAll.end(), V.begin(), V.end());
		std::sort(vAll.begin(), vAll.end(), [](const SCand &a, const SCand &b) { return a.m_D < b.m_D; });
		std::unordered_set<uint64_t> Seen;
		std::vector<SState> vNext;
		std::vector<SNode> vNodes;
		for(auto &C : vAll)
		{
			if(!Seen.insert(C.m_Hash).second)
				continue;
			SState N;
			N.m_S = vBeam[C.m_Parent].m_S;
			N.m_S.Step(C.m_In);
			N.m_D = C.m_D;
			N.m_Prev = C.m_In;
			vNodes.push_back({vBeam[C.m_Parent].m_Node, C.m_In});
			N.m_Node = (int)vNodes.size() - 1;
			vNext.push_back(std::move(N));
			if((int)vNext.size() >= Beam)
				break;
		}
		vLayers.push_back(std::move(vNodes));
		vBeam = std::move(vNext);
		// test the closest states
		int Found = 0;
		for(int k = 0; k < (int)vBeam.size() && k < Test; k++)
		{
			const int F = Cont(vBeam[k].m_S);
			if(F < 0)
				continue;
			Found++;
			if(F < BestFin)
			{
				BestFin = F;
				std::vector<STasInput> vPath;
				int L = (int)vLayers.size() - 1, N = vBeam[k].m_Node;
				while(L >= 0 && N >= 0)
				{
					vPath.push_back(vLayers[L][N].m_In);
					N = vLayers[L][N].m_Parent;
					L--;
				}
				std::reverse(vPath.begin(), vPath.end());
				vBest = vPre;
				vBest.insert(vBest.end(), vPath.begin(), vPath.end());
				vBest.insert(vBest.end(), gs_vRunIn.begin() + RunIdx(vBeam[k].m_S), gs_vRunIn.end());
				WriteInputs(Out.c_str(), vBest);
				std::printf("GRAFT step %d (rt %d = run rt %d): finish %d (run %d) dist %.2f -> %s\n", Step, vBeam[k].m_S.RaceTick(),
					vBeam[k].m_S.RaceTick() + gs_D, F, gs_RunFin, vBeam[k].m_D, Out.c_str());
				std::fflush(stdout);
			}
		}
		double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
		if(!vBeam.empty())
		{
			const CTasFast &B = vBeam[0].m_S;
			const CTasFast &R = gs_vRun[std::min((int)gs_vRun.size() - 1, RunIdx(B))];
			std::printf("step %d rt %d beam %zu best dist %.2f pos %.0f %.0f vel %.2f %.2f hk %d j %d | run %.0f %.0f vel %.2f %.2f hk %d j %d | cont ok %d [%.0fs]\n",
				Step, B.RaceTick(), vBeam.size(), vBeam[0].m_D, B.Pos().x, B.Pos().y, B.Vel().x, B.Vel().y, B.HookState(), B.Jumped(),
				R.Pos().x, R.Pos().y, R.Vel().x, R.Vel().y, R.HookState(), R.Jumped(), Found, Sec);
			std::fflush(stdout);
		}
	}
	if(BestFin < (1 << 30))
		std::printf("RESULT graft finish %d (run %d) -> %s\n", BestFin, gs_RunFin, Out.c_str());
	else
		std::printf("RESULT none\n");
	return 0;
}
