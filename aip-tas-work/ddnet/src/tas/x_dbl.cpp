// x_dbl: double-kick rendezvous search. The prefix ends with a grenade (a lob) in flight; its explosion point and step
// are fixed (projectiles ignore the tee). Beam-search the tee's inputs from the end of the prefix up to the state before
// that step, so that a point-blank shot fired in the same step adds its kick to the lob's. Every end state is scored
// exactly (all point-blank aims and directions); the value of a post-kick state is the number of ticks a free (idle)
// flight needs to rise to the line y = yt, so the search is for the strongest upward double kick.
// usage: x_dbl <map> <prefix> key=val...
//   beam=4000 qp=2 qv=0.5 hookang=24 yt=3000 out=PATH n=8 (writes PATH_1.txt..PATH_n.txt) threads=4 rot=0
//   kick=T (explosion step override: the step to server tick T; default the lob's) nojump=0
//   xt=X: value a post-kick state by the ticks a free flight (holding right, horizontal speed ramp) needs to reach x = X
//   instead of the rise to yt (a double kick along a corridor)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include <game/collision.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	int d, j, h, fi, tx, ty, w;
	char aLine[256];
	while(f && std::fgets(aLine, sizeof(aLine), f))
	{
		int n = std::sscanf(aLine, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w);
		if(n < 6)
			continue;
		STasInput In;
		In.m_Dir = d;
		In.m_Jump = j;
		In.m_Hook = h;
		In.m_Fire = fi;
		In.m_TX = tx;
		In.m_TY = ty;
		In.m_Weapon = n >= 7 ? w : -1;
		v.push_back(In);
	}
	if(f)
		std::fclose(f);
	return v;
}

static void WriteInputs(const char *pPath, const std::vector<STasInput> &v)
{
	FILE *f = std::fopen(pPath, "w");
	if(!f)
		return;
	for(const STasInput &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

static float gs_Yt = 3000;
static float gs_Xt = -1; // > 0: horizontal objective (reach x >= gs_Xt)
static float Ramp(float V)
{
	float w = V * 50;
	return w < 550 ? 1.0f : std::pow(1.4f, -(w - 550) / 2000);
}
static float gs_Zone[4] = {-1e9f, 1e9f, -1e9f, 1e9f}; // pre-kick position box for the fast (heuristic) value
static vec2 gs_ZoneC(0, 0);
static int gs_NumThreads = 4;

// free flight (vertical only: gravity, no walls) from a post-kick state: ticks to rise to y = yt; past the apex the
// remaining height is charged at 1 px/tick
static float RiseValue(vec2 P, vec2 V)
{
	if(gs_Xt > 0)
	{
		float x = P.x, vx = V.x, vy = V.y;
		for(int t = 1; t <= 60; t++)
		{
			vy += 0.5f;
			float nx = x + vx * Ramp(std::sqrt(vx * vx + vy * vy));
			if(nx >= gs_Xt)
				return t - 1 + (gs_Xt - x) / std::max(nx - x, 1e-3f);
			x = nx;
		}
		return 60 + (gs_Xt - x);
	}
	float y = P.y, vy = V.y;
	for(int t = 1; t <= 60; t++)
	{
		vy += 0.5f;
		float ny = y + vy;
		if(ny <= gs_Yt)
			return t - 1 + (y - gs_Yt) / std::max(y - ny, 1e-3f);
		if(vy >= 0)
			return t + (ny - gs_Yt);
		y = ny;
	}
	return 60 + (y - gs_Yt);
}

// exact: the step to the kick tick with a point-blank shot; best over aims (and dirs if AllDirs); also no shot
struct SKick
{
	float m_Val = 1e9f;
	STasInput m_In;
	vec2 m_V;
	vec2 m_P;
};

static SKick KickEval(const CFastG &G, const STasInput &Prev, int NumAims, bool AllDirs, bool Exact)
{
	SKick Best;
	if(!Exact && (G.m_Pos.x < gs_Zone[0] || G.m_Pos.x > gs_Zone[1] || G.m_Pos.y < gs_Zone[2] || G.m_Pos.y > gs_Zone[3]))
	{
		Best.m_Val = 1000 + distance(G.m_Pos, gs_ZoneC) / 10;
		return Best;
	}
	const int nd = AllDirs ? 3 : 1;
	for(int di = 0; di < nd; di++)
	{
		int Dir = AllDirs ? di - 1 : 0;
		for(int a = -1; a < NumAims; a++)
		{
			STasInput In = Prev;
			In.m_Dir = Dir;
			In.m_Jump = 0;
			In.m_Hook = 0;
			In.m_Weapon = -1;
			if(a >= 0)
			{
				float Ang = 2 * pi * a / NumAims;
				In.m_TX = (int)std::lround(std::cos(Ang) * 1000);
				In.m_TY = (int)std::lround(std::sin(Ang) * 1000);
				In.m_Fire = 1;
			}
			else
				In.m_Fire = 0;
			CFastG T = G;
			T.Step(In);
			if(T.m_Dead || T.m_Bad)
				continue;
			if(a >= 0 && T.m_NumProj >= G.m_NumProj)
				continue; // the shot did not go off in this step (reload, or no solid in reach): not a point-blank
			float Val;
			if(Exact)
			{
				// exact free flight to the line
				CFastG F = T;
				STasInput Idle = In;
				Idle.m_Fire = 0;
				Idle.m_Dir = gs_Xt > 0 ? 1 : 0;
				Val = 1e9f;
				float y = F.m_Pos.y;
				float x = F.m_Pos.x;
				for(int t = 1; t <= 60 && gs_Xt > 0; t++)
				{
					F.Step(Idle);
					if(F.m_Dead)
						break;
					if(F.m_Pos.x >= gs_Xt)
					{
						Val = t - 1 + (gs_Xt - x) / std::max(F.m_Pos.x - x, 1e-3f);
						break;
					}
					x = F.m_Pos.x;
				}
				for(int t = 1; t <= 60 && gs_Xt <= 0; t++)
				{
					F.Step(Idle);
					if(F.m_Dead)
						break;
					if(F.m_Pos.y <= gs_Yt)
					{
						Val = t - 1 + (y - gs_Yt) / std::max(y - F.m_Pos.y, 1e-3f);
						break;
					}
					if(F.m_Core.m_Vel.y >= 0)
					{
						Val = t + (F.m_Pos.y - gs_Yt);
						break;
					}
					y = F.m_Pos.y;
				}
			}
			else
				Val = RiseValue(T.m_Pos, T.m_Core.m_Vel);
			if(Val < Best.m_Val)
			{
				Best.m_Val = Val;
				Best.m_In = In;
				Best.m_V = T.m_Core.m_Vel;
				Best.m_P = T.m_Pos;
			}
		}
	}
	return Best;
}

struct SNode
{
	CFastG m_G;
	STasInput m_In; // input that led here
	int m_Parent;
	float m_H;
};

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_dbl <map> <prefix> key=val...\n");
		return 1;
	}
	CFastG::Init();
	std::map<std::string, std::string> Kv;
	for(int i = 3; i < argc; i++)
	{
		const char *e = std::strchr(argv[i], '=');
		if(e)
			Kv[std::string(argv[i], e - argv[i])] = e + 1;
	}
	auto Get = [&](const char *k, const char *d) { return Kv.count(k) ? Kv[k] : std::string(d); };
	const int Beam = std::stoi(Get("beam", "4000"));
	const float Qp = std::stof(Get("qp", "2")), Qv = std::stof(Get("qv", "0.5"));
	const int HookAng = std::stoi(Get("hookang", "24"));
	gs_Yt = std::stof(Get("yt", "3000"));
	gs_Xt = std::stof(Get("xt", "-1"));
	if(Kv.count("zone"))
	{
		std::sscanf(Kv["zone"].c_str(), "%f,%f,%f,%f", &gs_Zone[0], &gs_Zone[1], &gs_Zone[2], &gs_Zone[3]);
		gs_ZoneC = vec2((gs_Zone[0] + gs_Zone[1]) / 2, (gs_Zone[2] + gs_Zone[3]) / 2);
	}
	gs_NumThreads = std::stoi(Get("threads", "4"));
	const int NOut = std::stoi(Get("n", "8"));
	const bool NoJump = std::stoi(Get("nojump", "0")) != 0;
	const std::string Out = Get("out", "dbl");
	std::vector<STasInput> vBase = Kv.count("base") ? ReadInputs(Kv["base"].c_str()) : std::vector<STasInput>();

	std::vector<STasInput> vPre = ReadInputs(argv[2]);
	CTasGame Gm;
	Gm.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i = 0;
	for(; i < vPre.size() && !Gm.HasGrenade(); i++)
		Gm.Step(vPre[i]);
	CFastG G0;
	G0.FromGame(Gm);
	for(; i < vPre.size(); i++)
		G0.Step(vPre[i]);
	vec2 X;
	int Te;
	if(!G0.NextExplosion(X, Te))
	{
		std::printf("no grenade in flight at the end of the prefix\n");
		return 1;
	}
	if(Kv.count("kick"))
		Te = std::stoi(Kv["kick"]) + G0.m_StartTick;
	const int NFree = Te - 1 - G0.m_Tick;
	std::printf("start rt %d pos %.0f %.0f vel %.2f %.2f reload %d; lob explodes at %.1f %.1f in the step to rt %d; %d free steps\n",
		G0.RaceTick(), G0.m_Pos.x, G0.m_Pos.y, G0.m_Core.m_Vel.x, G0.m_Core.m_Vel.y, G0.m_ReloadTimer, X.x, X.y, Te - G0.m_StartTick, NFree);
	if(NFree < 0)
		return 1;

	std::vector<std::vector<SNode>> vL(NFree + 1);
	vL[0].push_back({G0, vPre.empty() ? STasInput() : vPre.back(), -1, 0});

	// heuristic: best over a few simple continuations (hold / release the hook, dir -1/0/1) of the fast kick value
	auto Heur = [&](const CFastG &G, const STasInput &Prev) {
		float Best = 1e9f;
		int Left = Te - 1 - G.m_Tick;
		if(!vBase.empty())
		{
			// the base run's own inputs from here (index = prefix length + ticks done)
			CFastG T = G;
			size_t Ix = vPre.size() + (G.m_Tick - G0.m_Tick);
			STasInput In = Prev;
			bool Dead = false;
			for(int k = 0; k < Left; k++)
			{
				if(Ix + k < vBase.size())
				{
					In = vBase[Ix + k];
					In.m_Fire = 0;
				}
				T.Step(In);
				if(T.m_Dead)
				{
					Dead = true;
					break;
				}
			}
			if(!Dead)
				Best = std::min(Best, KickEval(T, In, 48, false, false).m_Val);
		}
		for(int Rel = 0; Rel < 2; Rel++)
			for(int Dir = -1; Dir <= 1; Dir++)
			{
				if(Rel && !Prev.m_Hook)
					continue;
				CFastG T = G;
				STasInput In = Prev;
				In.m_Dir = Dir;
				In.m_Jump = 0;
				In.m_Fire = 0;
				In.m_Weapon = -1;
				if(Rel)
					In.m_Hook = 0;
				bool Dead = false;
				for(int k = 0; k < Left; k++)
				{
					T.Step(In);
					if(T.m_Dead)
					{
						Dead = true;
						break;
					}
				}
				if(Dead)
					continue;
				Best = std::min(Best, KickEval(T, In, 48, false, false).m_Val);
			}
		return Best;
	};

	for(int t = 0; t < NFree; t++)
	{
		const std::vector<SNode> &L = vL[t];
		// expand
		std::vector<std::vector<SNode>> vPart(gs_NumThreads);
		std::atomic<int> Next(0);
		auto Work = [&](int Th) {
			std::vector<SNode> &R = vPart[Th];
			while(true)
			{
				int k = Next++;
				if(k >= (int)L.size())
					break;
				const SNode &N = L[k];
				const STasInput &P = N.m_In;
				std::vector<STasInput> vC;
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int J = 0; J < (NoJump ? 1 : 2); J++)
					{
						if(J && P.m_Jump)
							continue;
						STasInput In = P;
						In.m_Dir = Dir;
						In.m_Jump = J;
						In.m_Fire = 0;
						In.m_Weapon = -1;
						if(P.m_Hook)
						{
							vC.push_back(In);
							In.m_Hook = 0;
							vC.push_back(In);
						}
						else
						{
							In.m_Hook = 0;
							vC.push_back(In);
							for(int a = 0; a < HookAng; a++)
							{
								float Ang = 2 * pi * a / HookAng;
								In.m_Hook = 1;
								In.m_TX = (int)std::lround(std::cos(Ang) * 1000);
								In.m_TY = (int)std::lround(std::sin(Ang) * 1000);
								vC.push_back(In);
							}
						}
					}
				for(const STasInput &In : vC)
				{
					CFastG T = N.m_G;
					T.Step(In);
					if(T.m_Dead || T.m_Bad)
						continue;
					R.push_back({T, In, k, 0});
				}
			}
		};
		{
			std::vector<std::thread> vT;
			for(int Th = 0; Th < gs_NumThreads; Th++)
				vT.emplace_back(Work, Th);
			for(auto &T : vT)
				T.join();
		}
		// dedup on a coarse state key (keep the first; all children are scored below, then the best per key is kept)
		std::vector<SNode> vAll;
		for(auto &P : vPart)
			for(auto &N : P)
				vAll.push_back(N);
		std::unordered_map<uint64_t, int> Seen;
		std::vector<SNode> vU;
		for(SNode &N : vAll)
		{
			const CFastG &G = N.m_G;
			auto Q = [](float v, float q) { return (uint64_t)(int64_t)std::lround(v / q) & 0xffff; };
			uint64_t K = Q(G.m_Pos.x, Qp) | Q(G.m_Pos.y, Qp) << 16 | Q(G.m_Core.m_Vel.x, Qv) << 32 | Q(G.m_Core.m_Vel.y, Qv) << 48;
			int Hs = G.m_Core.m_HookState;
			uint64_t K2 = (uint64_t)(Hs + 2) * 1000003ull + (Hs == HOOK_GRABBED ? (uint64_t)(G.m_Core.m_HookPos.x / 8) * 7919 + (uint64_t)(G.m_Core.m_HookPos.y / 8) : 0) +
				     (uint64_t)G.m_Core.m_Jumped * 31 + (N.m_In.m_Hook ? 17 : 0);
			K ^= K2 * 0x9E3779B97F4A7C15ull;
			if(Seen.emplace(K, (int)vU.size()).second)
				vU.push_back(N);
		}
		// score
		Next = 0;
		auto Sc = [&]() {
			while(true)
			{
				int k = Next++;
				if(k >= (int)vU.size())
					break;
				vU[k].m_H = Heur(vU[k].m_G, vU[k].m_In);
			}
		};
		{
			std::vector<std::thread> vT;
			for(int Th = 0; Th < gs_NumThreads; Th++)
				vT.emplace_back(Sc);
			for(auto &T : vT)
				T.join();
		}
		std::sort(vU.begin(), vU.end(), [](const SNode &a, const SNode &b) { return a.m_H < b.m_H; });
		if((int)vU.size() > Beam)
			vU.resize(Beam);
		std::printf("rt %d: %zu children, %zu kept, best h %.2f (pos %.0f %.0f vel %.1f %.1f)\n", vU.empty() ? -1 : vU[0].m_G.RaceTick(), vAll.size(),
			vU.size(), vU.empty() ? 0 : vU[0].m_H, vU.empty() ? 0 : vU[0].m_G.m_Pos.x, vU.empty() ? 0 : vU[0].m_G.m_Pos.y, vU.empty() ? 0 : vU[0].m_G.m_Core.m_Vel.x,
			vU.empty() ? 0 : vU[0].m_G.m_Core.m_Vel.y);
		std::fflush(stdout);
		vL[t + 1] = std::move(vU);
	}
	// exact evaluation of the final layer
	std::vector<SNode> &L = vL[NFree];
	std::vector<SKick> vK(L.size());
	std::atomic<int> Next(0);
	auto Ev = [&]() {
		while(true)
		{
			int k = Next++;
			if(k >= (int)L.size())
				break;
			vK[k] = KickEval(L[k].m_G, L[k].m_In, 360, true, true);
		}
	};
	{
		std::vector<std::thread> vT;
		for(int Th = 0; Th < gs_NumThreads; Th++)
			vT.emplace_back(Ev);
		for(auto &T : vT)
			T.join();
	}
	std::vector<int> vOrd(L.size());
	for(size_t k = 0; k < L.size(); k++)
		vOrd[k] = k;
	std::sort(vOrd.begin(), vOrd.end(), [&](int a, int b) { return vK[a].m_Val < vK[b].m_Val; });
	int Written = 0;
	std::vector<vec2> vDone;
	for(int k : vOrd)
	{
		if(Written >= NOut || vK[k].m_Val >= 1e8f)
			break;
		const SKick &Kk = vK[k];
		bool Dup = false;
		for(vec2 v : vDone)
			if(distance(v, Kk.m_P) < 3)
				Dup = true;
		if(Dup)
			continue;
		vDone.push_back(Kk.m_P);
		std::vector<STasInput> vPath;
		int t = NFree, idx = k;
		while(t > 0)
		{
			vPath.push_back(vL[t][idx].m_In);
			idx = vL[t][idx].m_Parent;
			t--;
		}
		std::reverse(vPath.begin(), vPath.end());
		std::vector<STasInput> vAll = vPre;
		vAll.insert(vAll.end(), vPath.begin(), vPath.end());
		vAll.push_back(Kk.m_In);
		Written++;
		char aName[512];
		std::snprintf(aName, sizeof(aName), "%s_%d.txt", Out.c_str(), Written);
		WriteInputs(aName, vAll);
		const CFastG &E = L[k].m_G;
		std::printf("#%d value %.2f: before pos %.1f %.1f vel %.2f %.2f; kick aim %d %d dir %d fire %d -> pos %.1f %.1f vel %.2f %.2f -> %s\n", Written, Kk.m_Val, E.m_Pos.x,
			E.m_Pos.y, E.m_Core.m_Vel.x, E.m_Core.m_Vel.y, Kk.m_In.m_TX, Kk.m_In.m_TY, Kk.m_In.m_Dir, Kk.m_In.m_Fire, Kk.m_P.x, Kk.m_P.y, Kk.m_V.x, Kk.m_V.y, aName);
	}
	return 0;
}
