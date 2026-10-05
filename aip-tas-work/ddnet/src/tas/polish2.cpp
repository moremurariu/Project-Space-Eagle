// polish2: hill climbing on a complete pre-grenade run (spawn -> grenade pickup) against the exact objective
//   t* = sub-tick race time at which the tee enters the 48 px pickup circle (the grenade is taken one tick after the
//   first tick that ends inside it), with the air jump available at the pickup (post-pickup braking), no freeze,
//   no double start. Lower is better.
// usage: polish2 <map> base=FILE out=FILE [seconds=600] [threads=4] [from=RT] [to=RT] [seed=N] [ties=0.05]
//   from/to: only mutate inputs whose race tick is in [from, to] (default: whole race)
// Mutations: hook aim rotation (small/large), hook press/release edge +-1, jump press +-1, dir change on 1-3 ticks,
// hook hold split (release one tick and re-press with a nearby aim), hook add (short press at a random aim).
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "sim.h"

#include <game/mapitems.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
	if(!f)
		return;
	for(const auto &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

static vec2 gs_Gren;

static void FindGrenade()
{
	const SMapInfo &M = CTasGame::Map();
	// the route's grenade: the first grenade entity in map order (the finish room has another one)
	for(int y = 0; y < M.m_H; y++)
		for(int x = 0; x < M.m_W; x++)
			if(M.Tile(x, y) == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
			{
				gs_Gren = vec2(x * 32 + 16, y * 32 + 16);
				return;
			}
}

struct SEval
{
	bool m_Ok = false;
	float m_T = 1e9f; // t*
	int m_PickIdx = -1; // input index (0-based) of the pickup tick
};

// replay v from state S (which has already applied v[0..From)), up to the pickup; t* as in seg's PickupTime
static SEval EvalFrom(const CTasGame &S, const std::vector<STasInput> &v, int From, CTasGame &G)
{
	SEval R;
	G.CopyFrom(S);
	for(int i = From; i < (int)v.size(); i++)
	{
		vec2 PrevPos = G.Pos();
		int PrevRt = G.m_Tick - G.m_StartTick;
		vec2 PrevPrevPos = G.Chr()->m_PrevPos;
		G.Step(v[i]);
		if(G.Frozen() || G.EnteredFreeze() || G.m_StartTick == -2)
			return R;
		if(G.HasGrenade())
		{
			if(!(G.Grounded() || !(G.Jumped() & 2)))
				return R; // no air jump left for the post-pickup braking
			// the previous tick ended inside the circle; the one before outside
			float Dc = distance(PrevPos, gs_Gren), Dp = distance(PrevPrevPos, gs_Gren);
			float F = Dp > Dc + 1e-4f ? std::clamp((Dp - 48.0f) / (Dp - Dc), 0.0f, 1.0f) : 1.0f;
			R.m_Ok = true;
			R.m_T = (float)(PrevRt - 1) + F;
			R.m_PickIdx = i;
			return R;
		}
	}
	return R;
}

static thread_local uint64_t tl_Rng = 88172645463325252ull;
static uint64_t Rnd64()
{
	tl_Rng ^= tl_Rng << 13;
	tl_Rng ^= tl_Rng >> 7;
	tl_Rng ^= tl_Rng << 17;
	return tl_Rng;
}
static int Rnd(int n) { return (int)(Rnd64() % (uint64_t)n); }
static float RndF() { return (float)(Rnd64() & 0xffffff) / (float)0xffffff; }

static void SetAim(STasInput &In, float Ang)
{
	In.m_TX = (int16_t)std::lround(std::cos(Ang) * 10000);
	In.m_TY = (int16_t)std::lround(std::sin(Ang) * 10000);
	if(!In.m_TX && !In.m_TY)
		In.m_TY = -1;
}
static float AimOf(const STasInput &In) { return std::atan2((float)In.m_TY, (float)In.m_TX); }

// mutate v within [Lo, Hi) (input indices); returns the first changed index or -1
static int Mutate(std::vector<STasInput> &v, int Lo, int Hi, int &Kind)
{
	const int n = Hi - Lo;
	if(n <= 2)
		return -1;
	int t = Lo + Rnd(n);
	Kind = Rnd(8);
	if(Kind <= 1)
	{
		// rotate the aim of the hook press run containing t (the aim only matters when the hook is fired)
		if(!v[t].m_Hook)
			return -1;
		int a = t, b = t;
		while(a > Lo && v[a - 1].m_Hook)
			a--;
		while(b + 1 < Hi && v[b + 1].m_Hook)
			b++;
		float Scale = Kind == 0 ? 0.002f : 0.03f; // radians
		float Ang = AimOf(v[a]) + Scale * (RndF() * 2 - 1);
		for(int k = a; k <= b; k++)
			SetAim(v[k], Ang);
		return a;
	}
	if(Kind == 2)
	{
		// move a hook press / release edge by one tick
		if(t == Lo || v[t].m_Hook == v[t - 1].m_Hook)
			return -1;
		if(Rnd(2))
		{
			v[t - 1].m_Hook = v[t].m_Hook;
			if(v[t].m_Hook)
			{
				v[t - 1].m_TX = v[t].m_TX;
				v[t - 1].m_TY = v[t].m_TY;
			}
			return t - 1;
		}
		v[t].m_Hook = v[t - 1].m_Hook;
		if(v[t].m_Hook)
		{
			v[t].m_TX = v[t - 1].m_TX;
			v[t].m_TY = v[t - 1].m_TY;
		}
		return t;
	}
	if(Kind == 3)
	{
		// move a jump press by one tick
		if(t == Lo || v[t].m_Jump == v[t - 1].m_Jump)
			return -1;
		if(Rnd(2))
		{
			v[t - 1].m_Jump = v[t].m_Jump;
			return t - 1;
		}
		v[t].m_Jump = v[t - 1].m_Jump;
		return t;
	}
	if(Kind == 4)
	{
		// direction on a short run
		int L = 1 + Rnd(3);
		int d = Rnd(3) - 1;
		bool Changed = false;
		for(int k = t; k < std::min(Hi, t + L); k++)
		{
			Changed |= v[k].m_Dir != d;
			v[k].m_Dir = d;
		}
		return Changed ? t : -1;
	}
	if(Kind == 5)
	{
		// split a hold: release for one tick, re-press with a nearby aim (fires a new hook)
		if(!v[t].m_Hook || t + 1 >= Hi || !v[t + 1].m_Hook)
			return -1;
		v[t].m_Hook = 0;
		float Ang = AimOf(v[t + 1]) + 0.35f * (RndF() * 2 - 1);
		for(int k = t + 1; k < Hi && v[k].m_Hook; k++)
			SetAim(v[k], Ang);
		return t;
	}
	if(Kind == 6)
	{
		// add a short press (1-3 ticks) at a random aim where the hook is released
		if(v[t].m_Hook || (t > Lo && v[t - 1].m_Hook))
			return -1;
		int L = 1 + Rnd(3);
		float Ang = 2 * pi * RndF();
		for(int k = t; k < std::min(Hi, t + L); k++)
		{
			if(v[k].m_Hook)
				break;
			v[k].m_Hook = 1;
			SetAim(v[k], Ang);
		}
		return t;
	}
	// Kind 7: remove a short press (<= 3 ticks)
	if(!v[t].m_Hook)
		return -1;
	int a = t, b = t;
	while(a > Lo && v[a - 1].m_Hook)
		a--;
	while(b + 1 < Hi && v[b + 1].m_Hook)
		b++;
	if(b - a + 1 > 3)
		return -1;
	for(int k = a; k <= b; k++)
		v[k].m_Hook = 0;
	return a;
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: polish2 <map> base=FILE out=FILE [seconds=600] [threads=4] [from=RT] [to=RT] [seed=N] [ties=0.05]\n");
		return 1;
	}
	std::string Base, Out = "polish2_out.txt";
	double Seconds = 600;
	int Threads = 4, FromRt = -1000, ToRt = 100000;
	uint64_t Seed = 1;
	float Ties = 0.05f;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "base") Base = V;
		else if(K == "out") Out = V;
		else if(K == "seconds") Seconds = std::stod(V);
		else if(K == "threads") Threads = std::stoi(V);
		else if(K == "from") FromRt = std::stoi(V);
		else if(K == "to") ToRt = std::stoi(V);
		else if(K == "seed") Seed = std::stoull(V);
		else if(K == "ties") Ties = std::stof(V);
		else
		{
			std::printf("unknown option %s\n", K.c_str());
			return 1;
		}
	}
	FindGrenade();
	std::vector<STasInput> vBest = ReadInputs(Base.c_str());
	// a few spare ticks so a later pickup can still happen
	for(int k = 0; k < 8 && !vBest.empty(); k++)
		vBest.push_back(vBest.back());

	CTasGame Spawn;
	Spawn.Spawn(CTasGame::Map().m_vSpawns[0]);
	CTasGame Tmp;
	SEval Best = EvalFrom(Spawn, vBest, 0, Tmp);
	if(!Best.m_Ok)
	{
		std::printf("base run has no valid pickup\n");
		return 1;
	}
	vBest.resize(Best.m_PickIdx + 1 + 8);
	std::printf("start: t* %.4f pickup rt %d (input %d)\n", Best.m_T, (int)std::floor(Best.m_T) + 2, Best.m_PickIdx + 1);
	std::fflush(stdout);

	// checkpoints of the current best run every CP inputs (rebuilt on every improvement)
	const int CP = 16;
	std::mutex Mx;
	std::vector<std::unique_ptr<CTasGame>> vCk;
	std::vector<int> vRt; // race tick after each input of the best run
	int Version = 0;
	auto Rebuild = [&]() {
		vCk.clear();
		vRt.assign(vBest.size(), -1);
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(int i = 0; i < (int)vBest.size(); i++)
		{
			if(i % CP == 0)
			{
				auto C = std::make_unique<CTasGame>();
				C->CopyFrom(G);
				vCk.push_back(std::move(C));
			}
			G.Step(vBest[i]);
			vRt[i] = G.m_Started ? G.m_Tick - G.m_StartTick : -1;
		}
		Version++;
	};
	Rebuild();
	int Lo = 0, Hi = Best.m_PickIdx + 1;
	for(int i = 0; i < (int)vRt.size(); i++)
		if(vRt[i] >= FromRt && vRt[i] >= 0)
		{
			Lo = i;
			break;
		}
	for(int i = (int)vRt.size() - 1; i >= 0; i--)
		if(vRt[i] >= 0 && vRt[i] <= ToRt)
		{
			Hi = std::min(Hi, i + 1);
			break;
		}
	if(FromRt <= -1000)
		Lo = 0;
	std::printf("mutating inputs %d..%d\n", Lo, Hi - 1);

	auto t0 = std::chrono::steady_clock::now();
	std::atomic<long> Evals{0}, Accepted{0};
	auto Worker = [&](int Id) {
		tl_Rng = 0x9E3779B97F4A7C15ull * (Seed * 131 + Id + 1);
		CTasGame G;
		std::vector<STasInput> v;
		int MyVersion = -1;
		float MyBest = 1e9f;
		std::vector<std::unique_ptr<CTasGame>> vMyCk;
		while(std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() < Seconds)
		{
			{
				std::lock_guard<std::mutex> L(Mx);
				if(MyVersion != Version)
				{
					v = vBest;
					MyBest = Best.m_T;
					vMyCk.clear();
					for(auto &C : vCk)
					{
						auto D = std::make_unique<CTasGame>();
						D->CopyFrom(*C);
						vMyCk.push_back(std::move(D));
					}
					MyVersion = Version;
				}
			}
			std::vector<STasInput> w = v;
			int First = 1 << 30;
			int NM = 1 + (Rnd(4) == 0 ? Rnd(3) : 0);
			for(int m = 0; m < NM; m++)
			{
				int Kind;
				int f = Mutate(w, Lo, std::min(Hi, (int)w.size()), Kind);
				if(f >= 0)
					First = std::min(First, f);
			}
			if(First >= (int)w.size())
				continue;
			int Ck = std::min(First / CP, (int)vMyCk.size() - 1);
			SEval R = EvalFrom(*vMyCk[Ck], w, Ck * CP, G);
			Evals++;
			if(!R.m_Ok)
				continue;
			bool Better = R.m_T < MyBest - 1e-4f;
			bool Tie = !Better && std::fabs(R.m_T - MyBest) <= 1e-4f && RndF() < Ties;
			if(!Better && !Tie)
				continue;
			std::lock_guard<std::mutex> L(Mx);
			if(R.m_T < Best.m_T - 1e-4f || (Tie && std::fabs(R.m_T - Best.m_T) <= 1e-4f && MyVersion == Version))
			{
				w.resize(R.m_PickIdx + 1 + 8);
				bool Improved = R.m_T < Best.m_T - 1e-4f;
				vBest = w;
				Best = R;
				Rebuild();
				Accepted++;
				if(Improved)
				{
					std::vector<STasInput> o(vBest.begin(), vBest.begin() + Best.m_PickIdx + 1);
					WriteInputs(Out, o);
					std::printf("t* %.4f pickup rt %d first change at input %d (%ld evals, %.0fs)\n", Best.m_T, (int)std::floor(Best.m_T) + 2, First, Evals.load(),
						std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
					std::fflush(stdout);
				}
			}
		}
	};
	std::vector<std::thread> vTh;
	for(int T = 1; T < Threads; T++)
		vTh.emplace_back(Worker, T);
	Worker(0);
	for(auto &Th : vTh)
		Th.join();
	std::vector<STasInput> o(vBest.begin(), vBest.begin() + Best.m_PickIdx + 1);
	WriteInputs(Out, o);
	std::printf("done: t* %.4f pickup rt %d (%ld evals, %ld accepted)\n", Best.m_T, (int)std::floor(Best.m_T) + 2, Evals.load(), Accepted.load());
	return 0;
}
