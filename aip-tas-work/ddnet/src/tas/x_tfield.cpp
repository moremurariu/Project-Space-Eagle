// x_tfield: velocity-aware time-to-go field T(x, y, heading, speed) for the post-grenade part, by value iteration on a
// reduced point-mass model of the tee (tfield.h). x_ds tfield=FILE ranks beam states by race tick + T.
//
// Why: x_ds ranks by the lag behind the incumbent at the same geodesic distance plus an energy credit. A free search
// with that ranking races into turns (geodesic progress pays now, the braking it forces comes later) and ends 13-25
// ticks behind the polished incumbent over U-turn 1 (NOTES "Speed field"). Here the value of speed is physical: a
// state is worth the time the reduced model needs from it, which already includes braking for the next turn and the
// heading it has to turn through.
//
// Model per tick (same order as the game: velocity first, then the ramped move): gravity 0.5; one control per DP step
// (dt ticks): lateral hook pull to either side (lossless turn, 3 px/t^2 scaled like the hook: downward x0.3,
// horizontal x0.85, times lat=), longitudinal brake (hook backwards + direction key, times brake=) / coast / kick
// (kick= px/t^2 average along the motion; below 15 px/t the hook accelerates, at least 2); speed cap vmax=; the
// horizontal move is ramped (vx * 1.4^-((50|v|-550)/2000)); the tee centre must stay out of freeze and the tee box out
// of solid (no wall contact at all in the model); a finish tile ends it.
//
// usage: x_tfield MAP out=FILE [cell=16 nh=24 nv=16 dv=5 dt=2 lat=0.8 brake=0.8 kick=0.3 vmax=75 sweeps=40 threads=4
//                  region=geo_px_limit (default: geodesic distance of the pickup + 1500)]
//        x_tfield MAP field=FILE inc=RUN [from=RT step=10]   (prints T along RUN vs its real remaining time)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include "tfield.h"
#include "tasio.h"

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <string>
#include <thread>
#include <vector>

struct SModel
{
	float m_Lat = 0.8f, m_Brake = 0.8f, m_Kick = 0.3f, m_VMax = 75.0f;
	int m_Dt = 2;
};
static SModel gs_M;
static const SMapInfo *gs_pMap;

static inline bool SolidAt(float x, float y)
{
	int T = gs_pMap->Tile((int)std::floor(x / 32), (int)std::floor(y / 32));
	return T == TILE_SOLID || T == TILE_NOHOOK;
}
static inline int TileAt(float x, float y) { return gs_pMap->Tile((int)std::floor(x / 32), (int)std::floor(y / 32)); }
static inline bool LegalPos(float x, float y)
{
	int Tc = TileAt(x, y);
	if(Tc == TILE_FREEZE || Tc == TILE_SOLID || Tc == TILE_NOHOOK)
		return false;
	return !SolidAt(x - 13, y - 13) && !SolidAt(x + 13, y - 13) && !SolidAt(x - 13, y + 13) && !SolidAt(x + 13, y + 13);
}
static inline float Ramp(float v)
{
	float s = v * 50.0f;
	return s < 550.0f ? 1.0f : 1.0f / std::pow(1.4f, (s - 550.0f) / 2000.0f);
}

static inline bool BoxSolid(float x, float y)
{
	return SolidAt(x - 14, y - 14) || SolidAt(x + 14, y - 14) || SolidAt(x - 14, y + 14) || SolidAt(x + 14, y + 14) || SolidAt(x, y - 14) ||
	       SolidAt(x, y + 14) || SolidAt(x - 14, y) || SolidAt(x + 14, y);
}

// one model tick; returns 0 ok, -1 dead, 1 finished
static inline int ModelTick(vec2 &P, vec2 &V, vec2 &Hd, float SLat, int SLong)
{
	V.y += 0.5f;
	float Sp = length(V);
	if(Sp > 0.5f)
		Hd = V / Sp;
	if(SLat != 0)
	{
		vec2 N = vec2(-Hd.y, Hd.x) * (SLat > 0 ? 1.0f : -1.0f);
		vec2 Hv = N * 3.0f;
		if(Hv.y > 0)
			Hv.y *= 0.3f;
		Hv.x *= 0.85f;
		float A = std::max(0.0f, dot(Hv, N)) * std::fabs(SLat) * gs_M.m_Lat;
		V += N * A;
	}
	Sp = length(V);
	if(SLong < 0 && Sp > 0.01f)
	{
		vec2 Bv = -Hd * 3.0f;
		if(Bv.y > 0)
			Bv.y *= 0.3f;
		Bv.x *= 0.95f;
		if(std::fabs(V.x) > 5.0f)
			Bv.x += V.x > 0 ? -1.5f : 1.5f;
		float B = std::max(0.0f, -dot(Bv, Hd)) * gs_M.m_Brake;
		V -= Hd * std::min(B, Sp);
	}
	else if(SLong > 0)
	{
		float K = Sp < 15.0f ? std::max(gs_M.m_Kick, 2.0f) : gs_M.m_Kick;
		V += Hd * K;
	}
	Sp = length(V);
	if(Sp > gs_M.m_VMax)
		V *= gs_M.m_VMax / Sp;
	vec2 D(V.x * Ramp(length(V)), V.y);
	float L = length(D);
	int N = std::max(1, (int)std::ceil(L / 12.0f));
	vec2 Q = P, St = D / (float)N;
	for(int i = 1; i <= N; i++)
	{
		// like MoveBox: a step into solid is cancelled per axis and that velocity component is lost (bare blocks)
		if(St.x != 0 && BoxSolid(Q.x + St.x, Q.y))
		{
			St.x = 0;
			V.x = 0;
		}
		if(St.y != 0 && BoxSolid(Q.x + St.x, Q.y + St.y))
		{
			St.y = 0;
			V.y = 0;
		}
		Q += St;
		int Tc = TileAt(Q.x, Q.y);
		if(Tc == TILE_FINISH)
		{
			P = Q;
			return 1;
		}
		if(Tc == TILE_FREEZE)
			return -1;
	}
	P = Q;
	return 0;
}

static STField gs_F;

static int Build(int argc, const char **argv)
{
	std::string Out = "tfield.bin", Init;
	int Sweeps = 40, Threads = 4;
	float Region = -1;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "out") Out = V;
		else if(K == "cell") gs_F.m_C = std::stoi(V);
		else if(K == "nh") gs_F.m_NH = std::stoi(V);
		else if(K == "nv") gs_F.m_NV = std::stoi(V);
		else if(K == "dv") gs_F.m_DV = std::stof(V);
		else if(K == "dt") gs_M.m_Dt = std::stoi(V);
		else if(K == "lat") gs_M.m_Lat = std::stof(V);
		else if(K == "brake") gs_M.m_Brake = std::stof(V);
		else if(K == "kick") gs_M.m_Kick = std::stof(V);
		else if(K == "vmax") gs_M.m_VMax = std::stof(V);
		else if(K == "sweeps") Sweeps = std::stoi(V);
		else if(K == "threads") Threads = std::stoi(V);
		else if(K == "region") Region = std::stof(V);
		else if(K == "init") Init = V; // warm start from a field of the same grid (values only decrease: use a weaker model's field)
		else
		{
			std::printf("unknown option %s\n", K.c_str());
			return 1;
		}
	}
	auto t0 = std::chrono::steady_clock::now();
	const int C = gs_F.m_C;
	gs_F.m_W = gs_pMap->m_W * 32 / C;
	gs_F.m_H = gs_pMap->m_H * 32 / C;
	const int W = gs_F.m_W, H = gs_F.m_H;
	// legal cells and geodesic distance to the finish (8-neighbour Dijkstra on cell centres)
	std::vector<uint8_t> vLegal((size_t)W * H, 0), vFin((size_t)W * H, 0);
	std::vector<float> vD((size_t)W * H, 1e9f);
	using SItem = std::pair<float, int>;
	std::priority_queue<SItem, std::vector<SItem>, std::greater<SItem>> Q;
	for(int y = 0; y < H; y++)
		for(int x = 0; x < W; x++)
		{
			float px = x * C + C * 0.5f, py = y * C + C * 0.5f;
			bool Fin = TileAt(px, py) == TILE_FINISH;
			vLegal[y * W + x] = Fin || LegalPos(px, py);
			vFin[y * W + x] = Fin;
			if(Fin)
			{
				vD[y * W + x] = 0;
				Q.push({0.0f, y * W + x});
			}
		}
	const int aDx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
	const int aDy[8] = {0, 0, 1, -1, 1, -1, 1, -1};
	while(!Q.empty())
	{
		auto [D, i] = Q.top();
		Q.pop();
		if(D > vD[i])
			continue;
		int x = i % W, y = i / W;
		for(int k = 0; k < 8; k++)
		{
			int nx = x + aDx[k], ny = y + aDy[k];
			if(nx < 0 || ny < 0 || nx >= W || ny >= H || !vLegal[ny * W + nx])
				continue;
			if(k >= 4 && (!vLegal[y * W + nx] || !vLegal[ny * W + x]))
				continue;
			float ND = D + (k < 4 ? (float)C : C * 1.41421356f);
			int j = ny * W + nx;
			if(ND < vD[j])
			{
				vD[j] = ND;
				Q.push({ND, j});
			}
		}
	}
	if(Region < 0)
	{
		// the grenade pickup (first grenade entity next to the post-grenade route) + 1500 px
		float Best = 1e9f;
		for(int y = 0; y < gs_pMap->m_H; y++)
			for(int x = 0; x < gs_pMap->m_W; x++)
				if(gs_pMap->Tile(x, y) == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
				{
					int cx = (x * 32 + 16) / C, cy = (y * 32 + 16) / C;
					float D = vD[cy * W + cx];
					if(D < 1e8f)
						Best = std::max(Best == 1e9f ? 0.0f : Best, D);
				}
		Region = (Best == 1e9f ? 60000.0f : Best) + 1500.0f;
	}
	gs_F.m_vIdx.assign((size_t)W * H, -1);
	std::vector<std::pair<float, int>> vOrder;
	for(int i = 0; i < W * H; i++)
		if(vLegal[i] && vD[i] <= Region)
			vOrder.push_back({vD[i], i});
	std::sort(vOrder.begin(), vOrder.end());
	std::vector<int> vCellOf;
	for(auto &[D, i] : vOrder)
	{
		gs_F.m_vIdx[i] = (int)vCellOf.size();
		vCellOf.push_back(i);
	}
	gs_F.m_NC = (int)vCellOf.size();
	const int NH = gs_F.m_NH, NV = gs_F.m_NV;
	gs_F.m_vT.assign((size_t)gs_F.m_NC * NH * NV, STField::INF);
	for(int c = 0; c < gs_F.m_NC; c++)
		if(vFin[vCellOf[c]])
			for(int h = 0; h < NH; h++)
				for(int k = 0; k < NV; k++)
					gs_F.At(c, h, k) = 0;
	if(!Init.empty())
	{
		STField F0;
		if(!F0.Load(Init.c_str()) || F0.m_NC != gs_F.m_NC || F0.m_NH != NH || F0.m_NV != NV || F0.m_C != C)
		{
			std::printf("cannot warm start from %s (missing or another grid)\n", Init.c_str());
			return 1;
		}
		gs_F.m_vT = F0.m_vT;
		std::printf("warm start from %s\n", Init.c_str());
	}
	std::printf("x_tfield: %dx%d cells of %d px, %d in the region (geo <= %.0f), %d headings x %d speeds (dv %.1f) = %.1fM states\n", W, H, C,
		gs_F.m_NC, Region, NH, NV, gs_F.m_DV, gs_F.m_NC * (double)NH * NV / 1e6);
	std::printf("model: dt %d lat %.2f brake %.2f kick %.2f vmax %.0f\n", gs_M.m_Dt, gs_M.m_Lat, gs_M.m_Brake, gs_M.m_Kick, gs_M.m_VMax);
	std::fflush(stdout);
	const float aLat[5] = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};
	for(int Sw = 0; Sw < Sweeps; Sw++)
	{
		std::atomic<int> Next{0};
		std::atomic<long> NChanged{0};
		std::vector<float> vMaxD(Threads, 0);
		std::vector<long> vReached(Threads, 0);
		const int Block = 32;
		auto Work = [&](int Tid) {
			while(true)
			{
				int b = Next.fetch_add(1);
				int c0 = b * Block;
				if(c0 >= gs_F.m_NC)
					break;
				int c1 = std::min(gs_F.m_NC, c0 + Block);
				for(int c = c0; c < c1; c++)
				{
					int i = vCellOf[c];
					if(vFin[i])
						continue;
					vec2 P0((i % W) * C + C * 0.5f, (i / W) * C + C * 0.5f);
					for(int h = 0; h < NH; h++)
					{
						float Ang = 2 * pi * h / NH;
						vec2 Hd0(std::cos(Ang), std::sin(Ang));
						for(int k = 0; k < NV; k++)
						{
							float Best = gs_F.At(c, h, k);
							float Old = Best;
							for(int l = 0; l < 5; l++)
								for(int s = -1; s <= 1; s++)
								{
									vec2 P = P0, V = Hd0 * (k * gs_F.m_DV), Hd = Hd0;
									int r = 0, t = 0;
									for(t = 1; t <= gs_M.m_Dt; t++)
									{
										r = ModelTick(P, V, Hd, aLat[l], s);
										if(r != 0)
											break;
									}
									float Cand;
									if(r < 0)
										continue;
									if(r > 0)
										Cand = (float)t;
									else
									{
										float Vn = gs_F.Value(P, length(V) > 0.5f ? V : Hd * 0.01f);
										if(Vn >= STField::INF * 0.5f)
											continue;
										Cand = gs_M.m_Dt + Vn;
									}
									if(Cand < Best)
										Best = Cand;
								}
							if(Best < Old - 0.01f)
							{
								gs_F.At(c, h, k) = Best;
								NChanged++;
								vMaxD[Tid] = std::max(vMaxD[Tid], Old >= STField::INF * 0.5f ? 1e4f : Old - Best);
							}
							if(Best < STField::INF * 0.5f)
								vReached[Tid]++;
						}
					}
				}
			}
		};
		std::vector<std::thread> vT;
		for(int t = 0; t < Threads; t++)
			vT.emplace_back(Work, t);
		for(auto &t : vT)
			t.join();
		float MaxD = *std::max_element(vMaxD.begin(), vMaxD.end());
		long Reached = 0;
		for(long r : vReached)
			Reached += r;
		double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
		std::printf("sweep %d: changed %ld, max improvement %.2f, reached %.1f%% (%.0fs)\n", Sw, (long)NChanged, MaxD,
			100.0 * Reached / ((double)gs_F.m_NC * NH * NV), Sec);
		std::fflush(stdout);
		if(Sw > 2 && MaxD < 0.05f)
			break;
	}
	gs_F.Save(Out.c_str());
	std::printf("wrote %s\n", Out.c_str());
	return 0;
}

static int Line(const char *pSpec, int Ticks);

static int Query(int argc, const char **argv)
{
	std::string Field, Inc;
	int From = 966, Step = 10;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "field") Field = V;
		else if(K == "inc") Inc = V;
		else if(K == "from") From = std::stoi(V);
		else if(K == "step") Step = std::stoi(V);
		else if(K == "dt") gs_M.m_Dt = std::stoi(V);
		else if(K == "lat") gs_M.m_Lat = std::stof(V);
		else if(K == "brake") gs_M.m_Brake = std::stof(V);
		else if(K == "kick") gs_M.m_Kick = std::stof(V);
		else if(K == "vmax") gs_M.m_VMax = std::stof(V);
	}
	std::string LineSpec;
	int LTicks = 400;
	for(int i = 2; i < argc; i++)
	{
		if(!std::strncmp(argv[i], "line=", 5))
			LineSpec = argv[i] + 5;
		if(!std::strncmp(argv[i], "ticks=", 6))
			LTicks = std::atoi(argv[i] + 6);
	}
	if(!gs_F.Load(Field.c_str()))
	{
		std::printf("cannot load %s\n", Field.c_str());
		return 1;
	}
	if(!LineSpec.empty())
		return Line(LineSpec.c_str(), LTicks);
	std::vector<STasInput> vIn = ReadInputs(Inc.c_str());
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i = 0;
	for(; i < vIn.size() && !G.HasGrenade(); i++)
		G.Step(vIn[i]);
	CFastG F;
	F.FromGame(G);
	std::vector<std::pair<int, float>> vRow;
	int Fin = -1;
	for(; i < vIn.size(); i++)
	{
		F.Step(vIn[i]);
		int rt = F.RaceTick();
		if(F.m_FinishTick >= 0)
		{
			Fin = rt;
			break;
		}
		if(rt >= From && rt % Step == 0)
			vRow.push_back({rt, gs_F.Value(F.m_Core.m_Pos, F.m_Core.m_Vel)});
		(void)rt;
	}
	std::printf("finish rt %d\n", Fin);
	for(auto &[rt, T] : vRow)
		std::printf("rt %d T %.1f real %d ratio %.3f\n", rt, T, Fin - rt, Fin > rt ? T / (Fin - rt) : 0.0);
	return 0;
}

// the model's own greedy line from a state: x_tfield MAP field=FILE line=x,y,vx,vy [ticks=N] (prints k x y vx vy T)
static int Line(const char *pSpec, int Ticks)
{
	vec2 P, V;
	if(std::sscanf(pSpec, "%f,%f,%f,%f", &P.x, &P.y, &V.x, &V.y) != 4)
		return 1;
	vec2 Hd = length(V) > 0.5f ? normalize(V) : vec2(1, 0);
	const float aLat[5] = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};
	for(int k = 0; k < Ticks; k += gs_M.m_Dt)
	{
		float T = gs_F.Value(P, V);
		std::printf("%d %.1f %.1f %.2f %.2f %.1f\n", k, P.x, P.y, V.x, V.y, T);
		float Best = 1e9f;
		vec2 BP = P, BV = V, BH = Hd;
		bool Fin = false;
		for(int l = 0; l < 5; l++)
			for(int s = -1; s <= 1; s++)
			{
				vec2 P1 = P, V1 = V, H1 = Hd;
				int r = 0, t;
				for(t = 1; t <= gs_M.m_Dt; t++)
					if((r = ModelTick(P1, V1, H1, aLat[l], s)) != 0)
						break;
				if(r < 0)
					continue;
				float C = r > 0 ? t : gs_M.m_Dt + gs_F.Value(P1, V1);
				if(C < Best)
				{
					Best = C;
					BP = P1;
					BV = V1;
					BH = H1;
					Fin = r > 0;
				}
			}
		if(Best >= 1e8f || Fin)
			break;
		P = BP;
		V = BV;
		Hd = BH;
	}
	return 0;
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_tfield MAP out=FILE [...] | x_tfield MAP field=FILE inc=RUN\n");
		return 1;
	}
	CFastG::Init();
	gs_pMap = &CTasGame::Map();
	for(int i = 2; i < argc; i++)
		if(!std::strncmp(argv[i], "field=", 6))
			return Query(argc, argv);
	return Build(argc, argv);
}
