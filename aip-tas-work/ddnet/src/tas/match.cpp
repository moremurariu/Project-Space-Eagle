// match: beam search from spawn for an exact target state (position, velocity, hook idle, jumps unused, last input
// without hook / jump), so that the rest of a run made on another version of the map can be replayed unchanged.
// usage: match <map> x,y,vx,vy [beam=100000] [angles=48] [maxticks=120] [threads=4] [out=FILE]
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
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

struct SNode
{
	std::unique_ptr<CTasGame> m_pG;
	int m_Parent;
	STasInput m_In;
};
struct SCand
{
	float m_D;
	int m_Parent;
	STasInput m_In;
	uint64_t m_Hash;
};

static vec2 gs_TP, gs_TV;

static float Dist(const CTasGame &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	float D = std::fabs(P.x - gs_TP.x) + std::fabs(P.y - gs_TP.y) + 6.0f * (std::fabs(V.x - gs_TV.x) + std::fabs(V.y - gs_TV.y));
	if(G.HookState() > 0)
		D += 1.0f;
	// past the target in the direction of motion: it has to come back
	if((P.x - gs_TP.x) * (V.x >= 0 ? 1.0f : -1.0f) > 0 && std::fabs(gs_TV.x) > 1)
		D += 4.0f * std::fabs(P.x - gs_TP.x);
	if(P.y < gs_TP.y - 0.5f)
		D += 4.0f * (gs_TP.y - P.y);
	if(G.Jumped() & 2)
		D += 1000.0f; // the air jump is gone for good (until a ground touch)
	return D;
}

static bool Exact(const CTasGame &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	return P.x == gs_TP.x && P.y == gs_TP.y && std::fabs(V.x - gs_TV.x) < 1e-4f && std::fabs(V.y - gs_TV.y) < 1e-4f &&
	       G.HookState() <= 0 && G.Jumped() == 0 && !G.m_LastHook && !G.m_LastJump && !G.m_Started;
}

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: match <map> x,y,vx,vy [beam=] [angles=] [maxticks=] [threads=] [out=]\n");
		return 1;
	}
	std::sscanf(argv[2], "%f,%f,%f,%f", &gs_TP.x, &gs_TP.y, &gs_TV.x, &gs_TV.y);
	int Beam = 100000, Angles = 48, MaxTicks = 120, NT = 4;
	std::string Out = "match_out.txt";
	for(int i = 3; i < argc; i++)
	{
		std::string A = argv[i];
		auto Eq = A.find('=');
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "beam") Beam = std::stoi(V);
		else if(K == "angles") Angles = std::stoi(V);
		else if(K == "maxticks") MaxTicks = std::stoi(V);
		else if(K == "threads") NT = std::stoi(V);
		else if(K == "out") Out = V;
	}
	std::vector<std::vector<SNode>> vSteps(1);
	{
		SNode N;
		N.m_pG = std::make_unique<CTasGame>();
		N.m_pG->Spawn(CTasGame::Map().m_vSpawns[0]);
		N.m_Parent = -1;
		vSteps[0].push_back(std::move(N));
	}
	std::vector<std::pair<int16_t, int16_t>> vAims;
	for(int a = 0; a < Angles; a++)
	{
		float Ang = 2 * pi * a / Angles;
		vAims.push_back({(int16_t)std::lround(std::cos(Ang) * 1000), (int16_t)std::lround(std::sin(Ang) * 1000)});
	}
	for(int Step = 0; Step < MaxTicks; Step++)
	{
		auto &vCur = vSteps.back();
		std::vector<std::vector<SCand>> vTC(NT);
		std::atomic<int> Next{0};
		std::atomic<int> Found{-1};
		std::vector<STasInput> vFoundIn(1);
		auto Worker = [&](int T) {
			CTasGame Tmp;
			while(true)
			{
				int i = Next.fetch_add(1);
				if(i >= (int)vCur.size())
					break;
				const CTasGame &G = *vCur[i].m_pG;
				const bool Hooking = G.m_LastHook;
				const bool CanJump = !G.m_LastJump && (G.Grounded() || !(G.Jumped() & 2));
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int Jump = 0; Jump <= (CanJump ? 1 : 0); Jump++)
					{
						std::vector<STasInput> vIn;
						STasInput In;
						In.m_Dir = Dir;
						In.m_Jump = Jump;
						In.m_Weapon = -1;
						In.m_TX = vCur[i].m_In.m_TX;
						In.m_TY = vCur[i].m_In.m_TY;
						if(!In.m_TX && !In.m_TY)
							In.m_TY = -1;
						In.m_Hook = 0;
						vIn.push_back(In);
						if(Hooking)
						{
							In.m_Hook = 1;
							vIn.push_back(In);
						}
						else
							for(auto [TX, TY] : vAims)
							{
								STasInput H = In;
								H.m_Hook = 1;
								H.m_TX = TX;
								H.m_TY = TY;
								vIn.push_back(H);
							}
						for(const auto &I : vIn)
						{
							Tmp.CopyFrom(G);
							Tmp.Step(I);
							if(Tmp.Frozen() || Tmp.EnteredFreeze() || Tmp.m_Started)
								continue;
							if(Exact(Tmp))
							{
								int Exp = -1;
								if(Found.compare_exchange_strong(Exp, i))
									vFoundIn[0] = I;
							}
							vTC[T].push_back({Dist(Tmp), i, I, Tmp.Hash()});
						}
					}
			}
		};
		std::vector<std::thread> vTh;
		for(int T = 1; T < NT; T++)
			vTh.emplace_back(Worker, T);
		Worker(0);
		for(auto &Th : vTh)
			Th.join();
		if(Found.load() >= 0)
		{
			// rebuild the input path
			std::vector<STasInput> vPath{vFoundIn[0]};
			int Idx = Found.load();
			for(int s = (int)vSteps.size() - 1; s > 0; s--)
			{
				vPath.push_back(vSteps[s][Idx].m_In);
				Idx = vSteps[s][Idx].m_Parent;
			}
			std::reverse(vPath.begin(), vPath.end());
			FILE *f = std::fopen(Out.c_str(), "w");
			for(const auto &I : vPath)
				std::fprintf(f, "%d %d %d %d %d %d %d\n", I.m_Dir, I.m_Jump, I.m_Hook, I.m_Fire, I.m_TX, I.m_TY, I.m_Weapon);
			std::fclose(f);
			std::printf("EXACT match after %zu inputs -> %s\n", vPath.size(), Out.c_str());
			return 0;
		}
		std::vector<SCand> vAll;
		for(auto &v : vTC)
			vAll.insert(vAll.end(), v.begin(), v.end());
		std::sort(vAll.begin(), vAll.end(), [](const SCand &a, const SCand &b) { return a.m_D != b.m_D ? a.m_D < b.m_D : a.m_Hash < b.m_Hash; });
		std::unordered_set<uint64_t> Seen;
		std::unordered_set<int64_t> Cells;
		std::vector<SNode> vNext;
		CTasGame Tmp2;
		for(const auto &C : vAll)
		{
			if((int)vNext.size() >= Beam)
				break;
			if(!Seen.insert(C.m_Hash).second)
				continue;
			Tmp2.CopyFrom(*vCur[C.m_Parent].m_pG);
			Tmp2.Step(C.m_In);
			{
				vec2 P = Tmp2.Pos(), V = Tmp2.Vel();
				int64_t k = (int64_t)std::floor(P.x) + 4096;
				k = k * 8192 + (int64_t)std::floor(P.y) + 4096;
				k = k * 4096 + (int64_t)std::floor(V.x * 16) + 2048;
				k = k * 4096 + (int64_t)std::floor(V.y * 16) + 2048;
				k = k * 8 + (Tmp2.HookState() + 1);
				if(!Cells.insert(k).second)
					continue;
			}
			SNode N;
			N.m_pG = std::make_unique<CTasGame>();
			N.m_pG->CopyFrom(*vCur[C.m_Parent].m_pG);
			N.m_pG->Step(C.m_In);
			N.m_Parent = C.m_Parent;
			N.m_In = C.m_In;
			vNext.push_back(std::move(N));
		}
		if(vNext.empty())
			break;
		const CTasGame &B = *vNext[0].m_pG;
		std::printf("step %d beam %zu best d %.3f pos %.0f %.0f vel %.4f %.4f hook %d jumped %d\n", Step, vNext.size(), Dist(B), B.Pos().x, B.Pos().y, B.Vel().x, B.Vel().y, B.HookState(), B.Jumped());
		std::fflush(stdout);
		// keep only the last two steps' games in memory
		if(vSteps.size() >= 2)
			for(auto &N : vSteps.back())
				N.m_pG.reset();
		vSteps.push_back(std::move(vNext));
	}
	std::printf("no exact match\n");
	return 1;
}
