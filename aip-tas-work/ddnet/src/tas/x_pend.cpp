// x_pend: brute-force hook swing ("pendulum") through a turn. Teero takes U-turn 1 on one hook held ~23 ticks on the
// dividing wall's tip: gravity speeds him up while the hook turns the fall into horizontal speed, and he exits without
// spending a kick (NOTES "Pendulum turns"). The beam searches (x_ds) re-hook every few ticks and kick at the apex.
// From the incumbent's state at rt p0, every combination of: press tick tp in [p0, p0+np), hook aim (angle step
// astep), direction during the hold (+1 or 0 before the switch tick ts, -1 after), release tick tr, then dir -1 and no
// hook up to rt e. The end state is scored by projection onto the incumbent's path (lead in ticks, + spw x speed
// difference vs the incumbent at that point); the best (diverse) ones are written as prefixes for x_ds.
// usage: x_pend <map> <inc> p0=1118 e=1152 np=10 astep=1 trmin=6 out=PATH n=12 threads=4 spw=0.3 dirafter=-1
//   box=x0,y0,x1,y1 (optional: the anchor must lie in this box)
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
#include <mutex>
#include <string>
#include <thread>
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

struct SRes
{
	float m_Score, m_Lead, m_Speed, m_DSpeed;
	int m_Tp, m_Tr, m_Ts, m_DMode;
	float m_Ang;
	vec2 m_Pos, m_Vel, m_Anchor;
};

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_pend <map> <inc> key=val...\n");
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
	const int P0 = std::stoi(Get("p0", "1118")), E = std::stoi(Get("e", "1152")), NP = std::stoi(Get("np", "10"));
	const float AStep = std::stof(Get("astep", "1"));
	const int TrMin = std::stoi(Get("trmin", "6")), NOut = std::stoi(Get("n", "12")), NThreads = std::stoi(Get("threads", "4"));
	const float SpW = std::stof(Get("spw", "0.3"));
	const int DirAfter = std::stoi(Get("dirafter", "-1"));
	const std::string Out = Get("out", "pend");
	float Box[4] = {-1e9f, -1e9f, 1e9f, 1e9f};
	if(Kv.count("box"))
		std::sscanf(Kv["box"].c_str(), "%f,%f,%f,%f", &Box[0], &Box[1], &Box[2], &Box[3]);

	std::vector<STasInput> vInc = ReadInputs(argv[2]);
	CTasGame Gm;
	Gm.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i = 0;
	for(; i < vInc.size() && !Gm.HasGrenade(); i++)
		Gm.Step(vInc[i]);
	CFastG G;
	G.FromGame(Gm);
	CFastG G0;
	bool Have = false;
	size_t I0 = 0;
	// incumbent path from p0 on (positions and speeds per race tick) for the projection
	std::vector<vec2> vP, vV;
	for(; i < vInc.size(); i++)
	{
		if(G.m_Started && G.RaceTick() == P0 && !Have)
		{
			G0 = G;
			Have = true;
			I0 = i;
		}
		if(Have)
		{
			vP.push_back(G.m_Pos);
			vV.push_back(G.m_Core.m_Vel);
			if(G.RaceTick() > E + 120)
				break;
		}
		G.Step(vInc[i]);
	}
	if(!Have)
	{
		std::printf("incumbent never reaches rt %d\n", P0);
		return 1;
	}
	std::printf("start rt %d pos %.0f %.0f vel %.2f %.2f; incumbent at rt %d: pos %.0f %.0f |v| %.2f\n", G0.RaceTick(), G0.m_Pos.x,
		G0.m_Pos.y, G0.m_Core.m_Vel.x, G0.m_Core.m_Vel.y, E, vP[E - P0].x, vP[E - P0].y, length(vV[E - P0]));

	auto Project = [&](vec2 X, float &Rt, float &Spd) {
		float Best = 1e18f;
		Rt = -1;
		Spd = 0;
		for(size_t k = 0; k + 1 < vP.size(); k++)
		{
			vec2 A = vP[k], B = vP[k + 1], AB = B - A;
			float L2 = dot(AB, AB);
			float u = L2 > 1e-6f ? std::clamp(dot(X - A, AB) / L2, 0.0f, 1.0f) : 0.0f;
			float d = length(X - (A + AB * u));
			if(d < Best)
			{
				Best = d;
				Rt = P0 + k + u;
				Spd = length(vV[k]) * (1 - u) + length(vV[k + 1]) * u;
			}
		}
		return Best;
	};

	// one input of the incumbent with the hook released (before the press)
	auto IncIn = [&](int Rt) {
		STasInput In = vInc[I0 + (Rt - P0)];
		In.m_Hook = 0;
		In.m_Fire = 0;
		In.m_Jump = 0;
		return In;
	};

	const int NAng = (int)std::lround(360.0f / AStep);
	std::vector<SRes> vAll;
	std::mutex Mu;
	std::atomic<int> Next(0);
	std::atomic<long> Sims(0);
	auto Work = [&]() {
		std::vector<SRes> vLoc;
		for(;;)
		{
			int Job = Next++;
			if(Job >= NP * NAng)
				break;
			const int Tp = P0 + Job / NAng;
			const float Ang = (Job % NAng) * AStep - 180.0f;
			const int Tx = (int)std::lround(std::cos(Ang * pi / 180.0f) * 1000), Ty = (int)std::lround(std::sin(Ang * pi / 180.0f) * 1000);
			// approach with the incumbent's inputs (hook released) up to the press tick
			CFastG A = G0;
			bool Dead = false;
			for(int t = P0; t < Tp && !Dead; t++)
			{
				A.Step(IncIn(t));
				Dead = A.m_Dead;
			}
			if(Dead)
				continue;
			for(int DMode = 0; DMode < 2; DMode++)
				for(int Ts = Tp; Ts <= E; Ts += 2)
				{
					CFastG H = A;
					vec2 Anchor(0, 0);
					bool Grabbed = false, BadAnchor = false;
					for(int t = Tp; t < E; t++)
					{
						STasInput In;
						In.m_Dir = t < Ts ? (DMode == 0 ? 1 : 0) : -1;
						In.m_Jump = 0;
						In.m_Hook = 1;
						In.m_Fire = 0;
						In.m_TX = Tx;
						In.m_TY = Ty;
						In.m_Weapon = -1;
						H.Step(In);
						Sims++;
						if(H.m_Dead)
							break;
						if(H.m_Core.m_HookState == HOOK_GRABBED && !Grabbed)
						{
							Grabbed = true;
							Anchor = H.m_Core.m_HookPos;
							if(Anchor.x < Box[0] || Anchor.y < Box[1] || Anchor.x > Box[2] || Anchor.y > Box[3])
								BadAnchor = true;
						}
						if(BadAnchor)
							break;
						// release after this tick (t + 1 - Tp ticks held), fly to E
						if(t + 1 - Tp >= TrMin && Grabbed && (t + 1 >= Ts || t + 1 == E))
						{
							CFastG R = H;
							bool D2 = false;
							for(int u = t + 1; u < E; u++)
							{
								STasInput In2 = In;
								In2.m_Hook = 0;
								In2.m_Dir = DirAfter;
								R.Step(In2);
								Sims++;
								if(R.m_Dead)
								{
									D2 = true;
									break;
								}
							}
							if(D2)
								continue;
							float Rt, Spd;
							float Dist = Project(R.m_Pos, Rt, Spd);
							if(Dist > 120 || Rt < 0)
								continue;
							SRes S;
							S.m_Lead = Rt - E;
							S.m_Speed = length(R.m_Core.m_Vel);
							S.m_DSpeed = S.m_Speed - Spd;
							S.m_Score = S.m_Lead + SpW * S.m_DSpeed;
							S.m_Tp = Tp;
							S.m_Tr = t + 1;
							S.m_Ts = Ts;
							S.m_DMode = DMode;
							S.m_Ang = Ang;
							S.m_Pos = R.m_Pos;
							S.m_Vel = R.m_Core.m_Vel;
							S.m_Anchor = Anchor;
							vLoc.push_back(S);
						}
					}
					if(!Grabbed)
						break; // the hook never grabs with this aim: other switch ticks do not change that
				}
		}
		std::lock_guard<std::mutex> L(Mu);
		vAll.insert(vAll.end(), vLoc.begin(), vLoc.end());
	};
	std::vector<std::thread> vT;
	for(int k = 0; k < NThreads; k++)
		vT.emplace_back(Work);
	for(auto &T : vT)
		T.join();
	std::printf("%ld steps, %zu end states\n", Sims.load(), vAll.size());
	std::sort(vAll.begin(), vAll.end(), [](const SRes &a, const SRes &b) { return a.m_Score > b.m_Score; });
	// diverse picks: different (press tick, aim within 2 deg, release tick) or a different end cell
	std::vector<SRes> vPick;
	for(const SRes &S : vAll)
	{
		bool Dup = false;
		for(const SRes &P : vPick)
			if(length(P.m_Pos - S.m_Pos) < 6 && length(P.m_Vel - S.m_Vel) < 1.0f)
				Dup = true;
		if(Dup)
			continue;
		vPick.push_back(S);
		if((int)vPick.size() >= NOut)
			break;
	}
	for(size_t k = 0; k < vPick.size(); k++)
	{
		const SRes &S = vPick[k];
		// rebuild the inputs: incumbent up to p0, approach, hold, release
		std::vector<STasInput> v(vInc.begin(), vInc.begin() + I0);
		for(int t = P0; t < S.m_Tp; t++)
			v.push_back(IncIn(t));
		const int Tx = (int)std::lround(std::cos(S.m_Ang * pi / 180.0f) * 1000), Ty = (int)std::lround(std::sin(S.m_Ang * pi / 180.0f) * 1000);
		for(int t = S.m_Tp; t < E; t++)
		{
			STasInput In;
			In.m_Dir = t < S.m_Tr ? (t < S.m_Ts ? (S.m_DMode == 0 ? 1 : 0) : -1) : DirAfter;
			In.m_Jump = 0;
			In.m_Hook = t < S.m_Tr ? 1 : 0;
			In.m_Fire = 0;
			In.m_TX = Tx;
			In.m_TY = Ty;
			In.m_Weapon = -1;
			v.push_back(In);
		}
		char aPath[512];
		std::snprintf(aPath, sizeof(aPath), "%s_%zu.txt", Out.c_str(), k + 1);
		WriteInputs(aPath, v);
		std::printf("#%zu score %.2f lead %+.2f |v| %.1f (%+.1f vs inc) pos %.0f %.0f vel %.1f %.1f | press %d aim %.1f anchor %.0f %.0f dir %s->-1 at %d release %d -> %s\n",
			k + 1, S.m_Score, S.m_Lead, S.m_Speed, S.m_DSpeed, S.m_Pos.x, S.m_Pos.y, S.m_Vel.x, S.m_Vel.y, S.m_Tp, S.m_Ang, S.m_Anchor.x,
			S.m_Anchor.y, S.m_DMode == 0 ? "+1" : "0", S.m_Ts, S.m_Tr, aPath);
	}
	return 0;
}
