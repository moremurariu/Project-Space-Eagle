// x_win: windowed beam search with a plain linear objective. From the end of a prefix, search the inputs up to race
// tick T maximising a*x + b*y + c*vx + d*vy of the end state (optionally a box the end state must be in). States are
// ranked by the objective after a short exact rollout (hook held / released, same direction), deduplicated on a fine
// state grid. Point-blank shots (exploding in the same step) are optional children. Used to probe what a short section
// allows (e.g. the exit speed of a turn) independent of any reference run.
// usage: x_win <map> <prefix> T=RT obj=a,b,c,d [box=x0,x1,y0,y1] [beam=4000] [qp=2] [qv=0.5] [hookang=32] [roll=8]
//        obj=geo,Kv: -geodesic distance to the finish + Kv * velocity along the route instead of the linear form
//        [pb=0] [pbang=32] [threads=4] [n=4] [out=PATH]   (writes PATH_1.txt.. = prefix + found inputs)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <queue>
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

struct SGeo
{
	static constexpr int C = 8;
	int m_W = 0, m_H = 0;
	std::vector<float> m_vD;
	std::vector<uint8_t> m_vLegal;
	static bool SolidPx(const SMapInfo &M, float x, float y)
	{
		int T = M.Tile((int)std::floor(x / 32), (int)std::floor(y / 32));
		return T == TILE_SOLID || T == TILE_NOHOOK;
	}
	void Build()
	{
		const SMapInfo &M = CTasGame::Map();
		m_W = M.m_W * 32 / C;
		m_H = M.m_H * 32 / C;
		m_vD.assign((size_t)m_W * m_H, 1e9f);
		m_vLegal.assign((size_t)m_W * m_H, 0);
		using SItem = std::pair<float, int>;
		std::priority_queue<SItem, std::vector<SItem>, std::greater<SItem>> Q;
		for(int y = 0; y < m_H; y++)
			for(int x = 0; x < m_W; x++)
			{
				float px = x * C + C * 0.5f, py = y * C + C * 0.5f;
				int Tc = M.Tile((int)(px / 32), (int)(py / 32));
				bool Legal = Tc != TILE_FREEZE && !SolidPx(M, px - 14, py - 14) && !SolidPx(M, px + 14, py - 14) && !SolidPx(M, px - 14, py + 14) &&
					     !SolidPx(M, px + 14, py + 14) && !SolidPx(M, px, py);
				m_vLegal[y * m_W + x] = Legal;
				if(Legal && Tc == TILE_FINISH)
				{
					m_vD[y * m_W + x] = 0;
					Q.push({0.0f, y * m_W + x});
				}
			}
		const int aDx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
		const int aDy[8] = {0, 0, 1, -1, 1, -1, 1, -1};
		while(!Q.empty())
		{
			auto [D, i] = Q.top();
			Q.pop();
			if(D > m_vD[i])
				continue;
			int x = i % m_W, y = i / m_W;
			for(int k = 0; k < 8; k++)
			{
				int nx = x + aDx[k], ny = y + aDy[k];
				if(nx < 0 || ny < 0 || nx >= m_W || ny >= m_H || !m_vLegal[ny * m_W + nx])
					continue;
				if(k >= 4 && (!m_vLegal[y * m_W + nx] || !m_vLegal[ny * m_W + x]))
					continue;
				float ND = D + (k < 4 ? (float)C : C * 1.41421356f);
				int j = ny * m_W + nx;
				if(ND < m_vD[j])
				{
					m_vD[j] = ND;
					Q.push({ND, j});
				}
			}
		}
	}
	// min over the 3x3 nearby cells of D(cell) + |P - cell centre| (continuous, never below the true field by much)
	float At(vec2 P) const
	{
		int cx = (int)std::floor(P.x / C), cy = (int)std::floor(P.y / C);
		float Best = 1e9f;
		for(int dy = -1; dy <= 1; dy++)
			for(int dx = -1; dx <= 1; dx++)
			{
				int x = cx + dx, y = cy + dy;
				if(x < 0 || y < 0 || x >= m_W || y >= m_H)
					continue;
				float D = m_vD[y * m_W + x];
				if(D > 1e8f)
					continue;
				float d = D + distance(P, vec2(x * C + C * 0.5f, y * C + C * 0.5f));
				Best = std::min(Best, d);
			}
		return Best;
	}
	// direction of steepest descent (towards the finish)
	vec2 Dir(vec2 P) const
	{
		const float h = 12;
		float gx = At(P + vec2(h, 0)) - At(P - vec2(h, 0));
		float gy = At(P + vec2(0, h)) - At(P - vec2(0, h));
		vec2 g(-gx, -gy);
		float l = length(g);
		return l > 1e-6f ? g / l : vec2(1, 0);
	}
};
static SGeo gs_Geo;
static bool gs_GeoObj = false; // objective: -geodesic distance to the finish + Kv * velocity along the route
static float gs_GeoKv = 4;
static float gs_Obj[4] = {1, 0, 0, 0};
static float gs_Box[4] = {-1e9f, 1e9f, -1e9f, 1e9f};

static float Objective(const CFastG &G)
{
	if(G.m_Dead)
		return -1e9f;
	vec2 P = G.m_Pos, V = G.m_Core.m_Vel;
	if(gs_GeoObj)
		return -gs_Geo.At(P) + gs_GeoKv * dot(V, gs_Geo.Dir(P));
	float o = gs_Obj[0] * P.x + gs_Obj[1] * P.y + gs_Obj[2] * V.x + gs_Obj[3] * V.y;
	// outside the box: charged by the distance to it
	float dx = std::max({gs_Box[0] - P.x, 0.0f, P.x - gs_Box[1]});
	float dy = std::max({gs_Box[2] - P.y, 0.0f, P.y - gs_Box[3]});
	return o - 3 * (dx + dy);
}

struct SNode
{
	CFastG m_G;
	STasInput m_In;
	int m_Parent;
	float m_H;
};

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_win <map> <prefix> T=RT obj=a,b,c,d ...\n");
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
	const int HookAng = std::stoi(Get("hookang", "32"));
	const int Roll = std::stoi(Get("roll", "8"));
	const bool Pb = std::stoi(Get("pb", "0")) != 0;
	const int PbAng = std::stoi(Get("pbang", "32"));
	const int NumThreads = std::stoi(Get("threads", "4"));
	const int NOut = std::stoi(Get("n", "4"));
	const std::string Out = Get("out", "win");
	if(Get("obj", "").rfind("geo", 0) == 0)
	{
		gs_GeoObj = true;
		std::sscanf(Get("obj", "").c_str(), "geo,%f", &gs_GeoKv);
		gs_Geo.Build();
	}
	else
		std::sscanf(Get("obj", "1,0,0,0").c_str(), "%f,%f,%f,%f", &gs_Obj[0], &gs_Obj[1], &gs_Obj[2], &gs_Obj[3]);
	if(Kv.count("box"))
		std::sscanf(Kv["box"].c_str(), "%f,%f,%f,%f", &gs_Box[0], &gs_Box[1], &gs_Box[2], &gs_Box[3]);

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
	const int T = std::stoi(Get("T", "0"));
	const int N = T - G0.RaceTick();
	std::printf("start rt %d pos %.0f %.0f vel %.2f %.2f reload %d; %d steps to rt %d\n", G0.RaceTick(), G0.m_Pos.x, G0.m_Pos.y, G0.m_Core.m_Vel.x,
		G0.m_Core.m_Vel.y, G0.m_ReloadTimer, N, T);
	if(N <= 0)
		return 1;

	if(Kv.count("base"))
	{
		// reference: a run's own inputs over the same ticks (it must share the prefix's state at the start)
		std::vector<STasInput> vB = ReadInputs(Kv["base"].c_str());
		CTasGame Gb;
		Gb.Spawn(CTasGame::Map().m_vSpawns[0]);
		size_t j = 0;
		for(; j < vB.size() && !Gb.HasGrenade(); j++)
			Gb.Step(vB[j]);
		CFastG B;
		B.FromGame(Gb);
		for(; j < vB.size() && B.RaceTick() < T; j++)
			B.Step(vB[j]);
		std::printf("base %s: rt %d obj %.1f pos %.1f %.1f vel %.2f %.2f\n", Kv["base"].c_str(), B.RaceTick(), Objective(B), B.m_Pos.x, B.m_Pos.y, B.m_Core.m_Vel.x,
			B.m_Core.m_Vel.y);
	}
	auto Heur = [&](const CFastG &G, const STasInput &Prev) {
		int Left = std::min(Roll, T - G.RaceTick());
		float Best = -1e9f;
		for(int Rel = 0; Rel < 2; Rel++)
		{
			if(Rel && !Prev.m_Hook)
				continue;
			CFastG R = G;
			STasInput In = Prev;
			In.m_Jump = 0;
			In.m_Fire = 0;
			In.m_Weapon = -1;
			if(Rel)
				In.m_Hook = 0;
			for(int k = 0; k < Left && !R.m_Dead; k++)
				R.Step(In);
			Best = std::max(Best, Objective(R));
		}
		return Best;
	};

	std::vector<std::vector<SNode>> vL(N + 1);
	vL[0].push_back({G0, vPre.empty() ? STasInput() : vPre.back(), -1, 0});
	for(int t = 0; t < N; t++)
	{
		const std::vector<SNode> &L = vL[t];
		std::vector<std::vector<SNode>> vPart(NumThreads);
		std::atomic<int> Next(0);
		auto Work = [&](int Th) {
			std::vector<SNode> &R = vPart[Th];
			std::vector<STasInput> vC;
			while(true)
			{
				int k = Next++;
				if(k >= (int)L.size())
					break;
				const SNode &Nd = L[k];
				const STasInput &P = Nd.m_In;
				vC.clear();
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int J = 0; J < 2; J++)
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
				size_t NPlain = vC.size();
				if(Pb && Nd.m_G.m_ReloadTimer == 0)
					for(int a = 0; a < PbAng; a++)
					{
						// point-blank: no hook press in this step (the aim is the shot's), dir free
						for(int Dir = -1; Dir <= 1; Dir++)
						{
							STasInput In = P;
							In.m_Dir = Dir;
							In.m_Jump = 0;
							In.m_Weapon = -1;
							In.m_Fire = 1;
							float Ang = 2 * pi * a / PbAng;
							In.m_TX = (int)std::lround(std::cos(Ang) * 1000);
							In.m_TY = (int)std::lround(std::sin(Ang) * 1000);
							vC.push_back(In);
						}
					}
				for(size_t c = 0; c < vC.size(); c++)
				{
					CFastG G = Nd.m_G;
					G.Step(vC[c]);
					if(G.m_Dead || G.m_Bad)
						continue;
					if(c >= NPlain && G.m_NumProj > Nd.m_G.m_NumProj)
						continue; // not a point-blank
					STasInput In = vC[c];
					R.push_back({G, In, k, 0});
				}
			}
		};
		{
			std::vector<std::thread> vT;
			for(int Th = 0; Th < NumThreads; Th++)
				vT.emplace_back(Work, Th);
			for(auto &Th : vT)
				Th.join();
		}
		std::vector<SNode> vU;
		std::unordered_map<uint64_t, int> Seen;
		for(auto &P : vPart)
			for(auto &Nd : P)
			{
				const CFastG &G = Nd.m_G;
				auto Q = [](float v, float q) { return (uint64_t)(int64_t)std::lround(v / q) & 0xffff; };
				uint64_t K = Q(G.m_Pos.x, Qp) | Q(G.m_Pos.y, Qp) << 16 | Q(G.m_Core.m_Vel.x, Qv) << 32 | Q(G.m_Core.m_Vel.y, Qv) << 48;
				int Hs = G.m_Core.m_HookState;
				uint64_t K2 = (uint64_t)(Hs + 2) * 1000003ull +
					      (Hs == HOOK_GRABBED || Hs == HOOK_FLYING ? (uint64_t)(G.m_Core.m_HookPos.x / 8) * 7919 + (uint64_t)(G.m_Core.m_HookPos.y / 8) : 0) +
					      (uint64_t)G.m_Core.m_Jumped * 31 + (Nd.m_In.m_Hook ? 17 : 0) + (uint64_t)G.m_ReloadTimer * 131071;
				K ^= K2 * 0x9E3779B97F4A7C15ull;
				if(Seen.emplace(K, (int)vU.size()).second)
					vU.push_back(Nd);
			}
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
			for(int Th = 0; Th < NumThreads; Th++)
				vT.emplace_back(Sc);
			for(auto &Th : vT)
				Th.join();
		}
		std::sort(vU.begin(), vU.end(), [](const SNode &a, const SNode &b) { return a.m_H > b.m_H; });
		if((int)vU.size() > Beam)
			vU.resize(Beam);
		if(!vU.empty())
			std::printf("rt %d: %zu kept, best h %.1f (pos %.0f %.0f vel %.1f %.1f)\n", vU[0].m_G.RaceTick(), vU.size(), vU[0].m_H, vU[0].m_G.m_Pos.x, vU[0].m_G.m_Pos.y,
				vU[0].m_G.m_Core.m_Vel.x, vU[0].m_G.m_Core.m_Vel.y);
		std::fflush(stdout);
		vL[t + 1] = std::move(vU);
		if(vL[t + 1].empty())
		{
			std::printf("beam died\n");
			return 1;
		}
	}
	std::vector<SNode> &L = vL[N];
	std::vector<int> vOrd(L.size());
	for(size_t k = 0; k < L.size(); k++)
		vOrd[k] = k;
	std::sort(vOrd.begin(), vOrd.end(), [&](int a, int b) { return Objective(L[a].m_G) > Objective(L[b].m_G); });
	int Written = 0;
	std::vector<vec2> vDone;
	for(int k : vOrd)
	{
		if(Written >= NOut)
			break;
		const CFastG &E = L[k].m_G;
		bool Dup = false;
		for(vec2 v : vDone)
			if(distance(v, E.m_Pos) < 4)
				Dup = true;
		if(Dup)
			continue;
		vDone.push_back(E.m_Pos);
		std::vector<STasInput> vPath;
		int t = N, idx = k;
		while(t > 0)
		{
			vPath.push_back(vL[t][idx].m_In);
			idx = vL[t][idx].m_Parent;
			t--;
		}
		std::reverse(vPath.begin(), vPath.end());
		std::vector<STasInput> vAll = vPre;
		vAll.insert(vAll.end(), vPath.begin(), vPath.end());
		Written++;
		char aName[512];
		std::snprintf(aName, sizeof(aName), "%s_%d.txt", Out.c_str(), Written);
		WriteInputs(aName, vAll);
		std::printf("#%d obj %.1f: rt %d pos %.1f %.1f vel %.2f %.2f |v| %.2f -> %s\n", Written, Objective(E), E.RaceTick(), E.m_Pos.x, E.m_Pos.y, E.m_Core.m_Vel.x,
			E.m_Core.m_Vel.y, length(E.m_Core.m_Vel), aName);
	}
	return 0;
}
