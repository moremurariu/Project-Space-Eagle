// brute: TrackMania-style bruteforce of a pre-grenade run on the exact fast stepper (CFast).
//
//   brute MAP INPUTS OUT [seconds=S] [threads=N] [from=RT] [to=RT] [window=W] [seed=K]
//
// Random small edits to the run's inputs in a window (direction, jump, hook press/release moved by a tick, hook aim
// nudged by a fraction of a degree or replaced), each replayed exactly from a cached state to the grenade pickup.
// Objective, lexicographic: earlier pickup tick, then the tee's distance to the grenade one tick before the pickup
// (closer = nearer to picking it up a tick earlier). Improvements are written to OUT (verified on CTasGame with the
// fast collision paths off).
#include "fast.h"
#include "tasio.h"

#include <game/collision.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

struct SEval
{
	int m_Rt = 1 << 30; // pickup race tick
	float m_D = 1e9f; // distance to the grenade one tick before the pickup
	bool Better(const SEval &o) const { return m_Rt < o.m_Rt || (m_Rt == o.m_Rt && m_D < o.m_D - 1e-4f); }
};

static vec2 gs_Grenade;

// replay from state F (after input index Start-1) with inputs v[Start..]; returns the objective
static SEval Eval(CFast F, const std::vector<STasInput> &v, int Start)
{
	SEval R;
	vec2 Prev = F.m_Core.m_Pos;
	for(int i = Start; i < (int)v.size() + 40; i++)
	{
		const STasInput &In = i < (int)v.size() ? v[i] : v.back();
		vec2 Before = F.m_Core.m_Pos;
		F.Step(In);
		if(F.m_Dead)
			return R;
		if(F.m_Got)
		{
			R.m_Rt = F.RaceTick();
			R.m_D = distance(Prev, gs_Grenade);
			return R;
		}
		Prev = Before;
	}
	return R;
}

int main(int argc, const char **argv)
{
	if(argc < 4)
	{
		std::printf("usage: brute MAP INPUTS OUT [seconds= threads= from= to= window= seed=]\n");
		return 1;
	}
	double Seconds = 600;
	int Threads = 4, From = 0, To = 100000, Window = 40;
	unsigned Seed = 1;
	for(int i = 4; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq);
		const char *v = A.c_str() + Eq + 1;
		if(K == "seconds")
			Seconds = atof(v);
		else if(K == "threads")
			Threads = atoi(v);
		else if(K == "from")
			From = atoi(v);
		else if(K == "to")
			To = atoi(v);
		else if(K == "window")
			Window = atoi(v);
		else if(K == "seed")
			Seed = atoi(v);
	}
	if(!CTasGame::LoadMap(argv[1]))
		return 1;
	CFast::Init();
	CCollision::ms_FastPaths = true;
	const SMapInfo &M = CTasGame::Map();
	gs_Grenade = vec2(170 * 32 + 16, 77 * 32 + 16);
	std::vector<STasInput> vBest = ReadInputs(argv[2]);
	CTasGame G;
	G.Spawn(M.m_vSpawns[0]);
	CFast F0;
	F0.FromGame(G);

	// states before each input of the current best
	std::mutex Mtx;
	std::vector<CFast> vStates;
	auto Rebuild = [&](const std::vector<STasInput> &v) {
		std::vector<CFast> vS;
		CFast F = F0;
		for(const STasInput &In : v)
		{
			vS.push_back(F);
			F.Step(In);
			if(F.m_Got || F.m_Dead)
				break;
		}
		return vS;
	};
	vStates = Rebuild(vBest);
	SEval Best = Eval(F0, vBest, 0);
	int StartTick = vStates.back().m_StartTick;
	std::printf("start: pickup race tick %d, distance before %.2f, start tick %d\n", Best.m_Rt, Best.m_D, StartTick);
	std::fflush(stdout);
	if(Best.m_Rt >= (1 << 30))
		return 1;
	vBest.resize(vStates.size());

	std::atomic<long> Trials(0), Accepts(0);
	auto T0 = std::chrono::steady_clock::now();
	auto Worker = [&](int Th) {
		std::mt19937 Rng(Seed * 1000003u + Th * 7919u);
		std::vector<STasInput> v;
		std::vector<CFast> vS;
		SEval Cur;
		int Version = -1;
		static std::atomic<int> s_Version(0);
		while(std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count() < Seconds)
		{
			if(Version != s_Version.load())
			{
				std::lock_guard<std::mutex> L(Mtx);
				v = vBest;
				vS = vStates;
				Cur = Best;
				Version = s_Version.load();
			}
			int N = (int)v.size();
			// window start (input index), in [From, To] race ticks
			int Lo = std::max(0, From + StartTick - 1), Hi = std::min(N - 2, To + StartTick - 1);
			if(Lo > Hi)
				return;
			int S = Lo + (int)(Rng() % (unsigned)(Hi - Lo + 1));
			std::vector<STasInput> w(v.begin() + S, v.end());
			int Edits = 1 + Rng() % 3;
			for(int e = 0; e < Edits; e++)
			{
				int k = (int)(Rng() % (unsigned)std::min(Window, (int)w.size()));
				STasInput &In = w[k];
				int Kind = Rng() % 8;
				if(Kind == 0)
					In.m_Dir = (int)(Rng() % 3) - 1;
				else if(Kind == 1)
					In.m_Jump ^= 1;
				else if(Kind == 2)
				{
					// move a hook press/release by one tick
					if(k + 1 < (int)w.size() && w[k].m_Hook != w[k + 1].m_Hook)
						std::swap(w[k].m_Hook, w[k + 1].m_Hook);
					else if(k > 0 && w[k].m_Hook != w[k - 1].m_Hook)
						std::swap(w[k].m_Hook, w[k - 1].m_Hook);
					else
						In.m_Hook ^= 1;
				}
				else if(Kind <= 5)
				{
					// nudge the aim (it matters on the tick the hook is fired)
					float A = std::atan2((float)In.m_TY, (float)In.m_TX);
					float Step = (Kind == 3 ? 0.002f : Kind == 4 ? 0.01f : 0.05f) * (Rng() % 2 ? 1 : -1);
					A += Step;
					In.m_TX = (int)std::lround(std::cos(A) * 10000);
					In.m_TY = (int)std::lround(std::sin(A) * 10000);
				}
				else if(Kind == 6)
				{
					float A = (Rng() % 36000) / 36000.0f * 2 * pi;
					In.m_TX = (int)std::lround(std::cos(A) * 10000);
					In.m_TY = (int)std::lround(std::sin(A) * 10000);
				}
				else
					In.m_Dir = w[std::max(0, k - 1)].m_Dir; // copy the previous direction
			}
			std::vector<STasInput> Cand(v.begin(), v.begin() + S);
			Cand.insert(Cand.end(), w.begin(), w.end());
			SEval E = Eval(vS[S], Cand, S);
			Trials++;
			if(E.Better(Cur))
			{
				std::lock_guard<std::mutex> L(Mtx);
				if(E.Better(Best))
				{
					std::vector<CFast> vNS = Rebuild(Cand);
					Cand.resize(vNS.size());
					Best = E;
					vBest = Cand;
					vStates = vNS;
					s_Version++;
					Accepts++;
					double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
					std::printf("[%6.0fs %9ld trials] pickup %d, distance before %.3f (edit at race tick %d)\n", Sec, Trials.load(), Best.m_Rt, Best.m_D,
						S + 1 - StartTick);
					std::fflush(stdout);
					// write if verified on the plain prediction code
					CCollision::ms_FastPaths = false;
					CTasGame V;
					V.Spawn(M.m_vSpawns[0]);
					bool Ok = false;
					for(const STasInput &In : vBest)
					{
						V.Step(In);
						if(V.Frozen() || V.EnteredFreeze() || V.m_StartTick == -2)
							break;
						if(V.HasGrenade())
						{
							Ok = V.m_Tick - V.m_StartTick == Best.m_Rt;
							break;
						}
					}
					CCollision::ms_FastPaths = true;
					if(Ok)
						WriteInputs(argv[3], vBest);
					else
						std::printf("  (verification failed)\n");
				}
			}
		}
	};
	std::vector<std::thread> vTh;
	for(int t = 0; t < Threads; t++)
		vTh.emplace_back(Worker, t);
	for(auto &T : vTh)
		T.join();
	std::printf("end: pickup %d, distance before %.3f, %ld trials, %ld accepted\n", Best.m_Rt, Best.m_D, Trials.load(), Accepts.load());
	return 0;
}
