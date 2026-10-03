// tas2: time-synchronous beam search whose selection keeps a Pareto front of
// (distance to go, speed) instead of a single weighted score.
// usage: tas2 <map> key=value...
//   prefix=FILE        inputs replayed from spawn first
//   tp=x,y,vx,vy       teleport after the prefix (px, px/tick); started=1 marks the race as started
//   gren=1             give the grenade; jumped=N sets the core's jump bits
//   goal=finish | goal=grenade | goal=x0,y0,x1,y1 (tile rectangle, inclusive)
//   beam=N maxticks=N threads=N angles=N fireangles=N firerange=PX
//   slack=PX           drop states more than this far behind the most advanced one
//   cell=PX vcell=PX/T cell dedupe resolution;  alpha=A tie-break weight of speed in a cell
//   smode=0|1          speed = along the route (0) or |v| (1)
//   survive=N          selected states must survive N ticks of a simple continuation
//   arrivals=N         keep searching N ticks after the first arrival, write each new (tick, speed) Pareto arrival
//   out=PREFIX         output prefix (PREFIX.best.txt, PREFIX.arr<k>.txt)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "distfield.h"
#include "sim.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
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

struct SParams
{
	std::string m_Prefix;
	bool m_Tp = false;
	float m_TpX = 0, m_TpY = 0, m_TpVx = 0, m_TpVy = 0;
	int m_Started = 0;
	int m_Gren = 0;
	int m_Jumped = -1;
	std::string m_Goal = "finish";
	int m_Beam = 1000;
	int m_MaxTicks = 3000;
	int m_Threads = 2;
	int m_Angles = 48;
	int m_FireAngles = 48;
	float m_FireRange = 260;
	float m_Slack = 800;
	float m_Cell = 8;
	float m_VCell = 1;
	float m_Alpha = 8;
	int m_SMode = 0;
	int m_Survive = 0;
	int m_Arrivals = 0;
	float m_Eps = 0.25f; // px/tick: a state joins a front layer only if faster than the previous member by this much
	std::string m_Out = "run";
	float m_ClearW = 0; // distance field: extra cost near walls (multiplier at the wall)
	float m_ClearR = 3; // tiles over which the extra cost fades out
	int m_Quiet = 0;
	int m_Dump = -1;
	int m_Roll = 0; // lookahead ticks for the rollout value (0 = off)
	int m_RollPre = 3; // rollout-evaluate this many times the beam size of the best candidates
	float m_Vref = 25;
	float m_Group = 64; // px: spatial group size for the per-group Pareto layers
};
static SParams gs_P;
static SDistField gs_Dist;
static float gs_aGoal[4];
static int gs_GoalMode = 0; // 0 finish, 1 grenade, 2 rect

static bool GoalReached(const CTasGame &G)
{
	if(gs_GoalMode == 0)
		return G.m_FinishTick >= 0;
	if(gs_GoalMode == 1)
		return G.HasGrenade();
	vec2 P = G.Pos();
	return P.x >= gs_aGoal[0] * 32 && P.y >= gs_aGoal[1] * 32 && P.x < (gs_aGoal[2] + 1) * 32 && P.y < (gs_aGoal[3] + 1) * 32;
}

static int JumpsLeft(const CTasGame &G)
{
	return G.Grounded() ? 2 : ((G.Jumped() & 2) ? 0 : 1);
}

static void HookTargets(const CTasGame &G, int NumAngles, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.Pos();
	const float Range = 380.0f + 6.0f * length(G.Vel()) + 20.0f;
	int aSeen[256];
	int NumSeen = 0;
	for(int a = 0; a < NumAngles; a++)
	{
		float Ang = 2 * pi * a / NumAngles;
		vec2 D(std::cos(Ang), std::sin(Ang));
		int Hit = -1;
		for(float r = 42.0f; r < Range; r += 4.0f)
		{
			vec2 Q = P + D * r;
			int tx = (int)std::floor(Q.x / 32), ty = (int)std::floor(Q.y / 32);
			if(M.Tile(tx, ty) == TILE_SOLID)
			{
				Hit = ty * M.m_W + tx;
				break;
			}
		}
		if(Hit < 0)
			continue;
		bool Dup = false;
		for(int i = 0; i < NumSeen; i++)
			Dup |= aSeen[i] == Hit;
		if(Dup || NumSeen >= 256)
			continue;
		aSeen[NumSeen++] = Hit;
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

static void FireTargets(const CTasGame &G, int NumAngles, float Range, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.Pos();
	for(int a = 0; a < NumAngles; a++)
	{
		float Ang = 2 * pi * a / NumAngles;
		vec2 D(std::cos(Ang), std::sin(Ang));
		bool Hit = false;
		for(float r = 8.0f; r < Range && !Hit; r += 6.0f)
		{
			vec2 Q = P + D * r;
			Hit = M.Tile((int)std::floor(Q.x / 32), (int)std::floor(Q.y / 32)) == TILE_SOLID;
		}
		if(!Hit)
			continue;
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

static void GenActions(const CTasGame &G, const STasInput &Prev, std::vector<STasInput> &vOut)
{
	vOut.clear();
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vHooks, s_vFire;
	const bool Hooking = G.m_LastHook;
	if(!Hooking)
		HookTargets(G, gs_P.m_Angles, s_vHooks);
	const bool CanJump = G.Grounded() || !(G.Jumped() & 2);
	const int Weapon = G.HasGrenade() ? WEAPON_GRENADE : -1;
	const bool CanFire = G.HasGrenade() && G.ActiveWeapon() == WEAPON_GRENADE && G.ReloadTimer() == 0;
	if(CanFire)
		FireTargets(G, gs_P.m_FireAngles, gs_P.m_FireRange, s_vFire);
	for(int Dir = -1; Dir <= 1; Dir++)
		for(int Jump = 0; Jump <= 1; Jump++)
		{
			if(Jump && (G.m_LastJump || !CanJump))
				continue;
			STasInput In;
			In.m_Dir = Dir;
			In.m_Jump = Jump;
			In.m_Weapon = Weapon;
			In.m_TX = Prev.m_TX;
			In.m_TY = Prev.m_TY;
			In.m_Hook = Hooking;
			vOut.push_back(In);
			if(Hooking)
			{
				In.m_Hook = 0;
				vOut.push_back(In);
				In.m_Hook = 1;
			}
			else
				for(auto [TX, TY] : s_vHooks)
				{
					STasInput H = In;
					H.m_Hook = 1;
					H.m_TX = TX;
					H.m_TY = TY;
					vOut.push_back(H);
				}
			if(CanFire && !Jump)
				for(auto [TX, TY] : s_vFire)
				{
					// the aim also steers a held hook? no: the hook direction is fixed when it was fired
					STasInput F = In;
					F.m_Fire = 1;
					F.m_TX = TX;
					F.m_TY = TY;
					vOut.push_back(F);
				}
		}
}

static float Dist(const CTasGame &G) { return gs_Dist.Sample(G.Pos()); }

static float Speed(const CTasGame &G)
{
	if(gs_P.m_SMode == 1)
		return length(G.Vel());
	vec2 Gr = gs_Dist.Grad(G.Pos());
	return -dot(G.Vel(), Gr);
}

// short lookahead: best "time to go" (in px at vref) over a few simple continuations
static float RollValue(const CTasGame &G, int K, float Vref)
{
	vec2 aPos[128];
	K = std::min(K, 128);
	float Best = Dist(G);
	int Dx = G.Vel().x > 0 ? 1 : -1;
	for(int Keep = 0; Keep <= (G.m_LastHook ? 1 : 0); Keep++)
		for(int d : {Dx, -Dx})
		{
			int n = G.Rollout(K, d, Keep, aPos);
			for(int k = 0; k < n; k++)
			{
				float D = gs_Dist.Sample(aPos[k]);
				if(D > 1e8f)
					break;
				Best = std::min(Best, D + (k + 1) * Vref);
			}
		}
	return Best;
}

static bool Dead(const CTasGame &G)
{
	return G.Frozen() || G.EnteredFreeze() || G.m_StartTick == -2;
}

// can the tee avoid freeze for N ticks with some simple continuation?
static bool Survives(const CTasGame &G, int N)
{
	vec2 aPos[128];
	N = std::min(N, 128);
	int Dx = G.Vel().x > 0 ? 1 : -1;
	const int aDirs[3] = {Dx, 0, -Dx};
	for(int d : aDirs)
	{
		if(G.Rollout(N, d, true, aPos) == N)
			return true;
		if(G.m_LastHook && G.Rollout(N, d, false, aPos) == N)
			return true;
	}
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vHooks;
	HookTargets(G, 32, s_vHooks);
	for(auto [TX, TY] : s_vHooks)
		for(int d : aDirs)
			if(G.RolloutHook(N, d, TX, TY, aPos) == N)
				return true;
	return false;
}

struct SChild
{
	float m_D, m_S;
	int m_Parent;
	STasInput m_In;
	uint64_t m_Hash;
	uint64_t m_Cell;
	uint64_t m_Group;
};

static uint64_t CellKey(const CTasGame &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	int64_t k = (int64_t)std::floor(P.x / gs_P.m_Cell);
	k = k * 4096 + (int64_t)std::floor(P.y / gs_P.m_Cell);
	k = k * 256 + ((int64_t)std::floor(V.x / gs_P.m_VCell) & 255);
	k = k * 256 + ((int64_t)std::floor(V.y / gs_P.m_VCell) & 255);
	int hs = G.HookState();
	int h = hs <= 0 ? 0 : (hs == HOOK_GRABBED ? 2 : 1);
	k = k * 3 + h;
	if(h == 2)
	{
		vec2 HP = G.HookPos();
		k = k * 1000003 + (int64_t)(HP.x / 16) * 1000 + (int64_t)(HP.y / 16);
	}
	k = k * 3 + JumpsLeft(G);
	k = k * 4 + (G.ReloadTimer() > 0 ? 1 : 0) + (G.NumProjectiles() > 0 ? 2 : 0);
	k = k * 2 + G.m_LastJump;
	return (uint64_t)k;
}

static uint64_t GroupKey(const CTasGame &G)
{
	vec2 P = G.Pos();
	uint64_t k = (uint64_t)(int64_t)std::floor(P.x / gs_P.m_Group);
	k = k * 4096 + (uint64_t)(int64_t)std::floor(P.y / gs_P.m_Group);
	return k * 3 + JumpsLeft(G);
}

struct SArrival
{
	int m_RaceTick;
	float m_Speed;
	vec2 m_Pos, m_Vel;
	std::vector<STasInput> m_vIn;
};

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: tas2 <map> key=value...\n");
		return 1;
	}
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		const char *v = V.c_str();
		if(K == "prefix")
			gs_P.m_Prefix = V;
		else if(K == "tp")
		{
			gs_P.m_Tp = std::sscanf(v, "%f,%f,%f,%f", &gs_P.m_TpX, &gs_P.m_TpY, &gs_P.m_TpVx, &gs_P.m_TpVy) == 4;
		}
		else if(K == "started")
			gs_P.m_Started = atoi(v);
		else if(K == "gren")
			gs_P.m_Gren = atoi(v);
		else if(K == "jumped")
			gs_P.m_Jumped = atoi(v);
		else if(K == "goal")
			gs_P.m_Goal = V;
		else if(K == "beam")
			gs_P.m_Beam = atoi(v);
		else if(K == "maxticks")
			gs_P.m_MaxTicks = atoi(v);
		else if(K == "threads")
			gs_P.m_Threads = atoi(v);
		else if(K == "angles")
			gs_P.m_Angles = atoi(v);
		else if(K == "fireangles")
			gs_P.m_FireAngles = atoi(v);
		else if(K == "firerange")
			gs_P.m_FireRange = atof(v);
		else if(K == "slack")
			gs_P.m_Slack = atof(v);
		else if(K == "cell")
			gs_P.m_Cell = atof(v);
		else if(K == "vcell")
			gs_P.m_VCell = atof(v);
		else if(K == "alpha")
			gs_P.m_Alpha = atof(v);
		else if(K == "smode")
			gs_P.m_SMode = atoi(v);
		else if(K == "survive")
			gs_P.m_Survive = atoi(v);
		else if(K == "arrivals")
			gs_P.m_Arrivals = atoi(v);
		else if(K == "eps")
			gs_P.m_Eps = atof(v);
		else if(K == "out")
			gs_P.m_Out = V;
		else if(K == "roll")
			gs_P.m_Roll = atoi(v);
		else if(K == "rollpre")
			gs_P.m_RollPre = atoi(v);
		else if(K == "vref")
			gs_P.m_Vref = atof(v);
		else if(K == "group")
			gs_P.m_Group = atof(v);
		else if(K == "dump")
			gs_P.m_Dump = atoi(v);
		else if(K == "quiet")
			gs_P.m_Quiet = atoi(v);
		else if(K == "clearw")
			gs_P.m_ClearW = atof(v);
		else if(K == "clearr")
			gs_P.m_ClearR = atof(v);
		else if(K == "stencil")
			gs_DFStencil = atoi(v);
		else
		{
			std::printf("unknown key %s\n", K.c_str());
			return 1;
		}
	}
	const SMapInfo &M = CTasGame::Map();
	static std::vector<float> s_vTileCost;
	if(gs_P.m_ClearW > 0)
	{
		// clearance: chebyshev-ish BFS distance (in tiles) to the nearest freeze/solid tile
		std::vector<int> vC(M.m_W * M.m_H, 1 << 20);
		std::vector<int> Q;
		for(int y = 0; y < M.m_H; y++)
			for(int x = 0; x < M.m_W; x++)
			{
				int t = M.Tile(x, y), f = M.Front(x, y);
				if(t == TILE_SOLID || t == TILE_NOHOOK || t == TILE_FREEZE || f == TILE_FREEZE)
				{
					vC[y * M.m_W + x] = 0;
					Q.push_back(y * M.m_W + x);
				}
			}
		for(size_t q = 0; q < Q.size(); q++)
		{
			int i = Q[q], x = i % M.m_W, y = i / M.m_W;
			for(int dy = -1; dy <= 1; dy++)
				for(int dx = -1; dx <= 1; dx++)
				{
					int nx = x + dx, ny = y + dy;
					if(nx < 0 || ny < 0 || nx >= M.m_W || ny >= M.m_H)
						continue;
					int j = ny * M.m_W + nx;
					if(vC[j] > vC[i] + 1)
					{
						vC[j] = vC[i] + 1;
						Q.push_back(j);
					}
				}
		}
		s_vTileCost.resize(M.m_W * M.m_H);
		for(int i = 0; i < M.m_W * M.m_H; i++)
		{
			float c = std::min((float)vC[i], 1000.0f);
			s_vTileCost[i] = 1.0f + gs_P.m_ClearW * std::max(0.0f, (gs_P.m_ClearR - (c - 1)) / gs_P.m_ClearR);
		}
		gs_Dist.m_pTileCost = &s_vTileCost;
	}
	if(gs_P.m_Goal == "finish")
	{
		gs_GoalMode = 0;
		gs_Dist.Build(M, {TILE_FINISH});
	}
	else if(gs_P.m_Goal == "grenade")
	{
		gs_GoalMode = 1;
		vec2 GP(0, 0);
		for(int y = 0; y < M.m_H; y++)
			for(int x = 0; x < M.m_W; x++)
				if(M.Tile(x, y) == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
					GP = vec2(x * 32 + 16, y * 32 + 16);
		gs_Dist.BuildFromPoint(M, GP);
	}
	else
	{
		gs_GoalMode = 2;
		if(std::sscanf(gs_P.m_Goal.c_str(), "%f,%f,%f,%f", &gs_aGoal[0], &gs_aGoal[1], &gs_aGoal[2], &gs_aGoal[3]) != 4)
		{
			std::printf("bad goal\n");
			return 1;
		}
		SMapInfo Tmp = M;
		const uint8_t GOAL = 250;
		for(int y = (int)gs_aGoal[1]; y <= (int)gs_aGoal[3]; y++)
			for(int x = (int)gs_aGoal[0]; x <= (int)gs_aGoal[2]; x++)
				if(Tmp.m_vGame[y * Tmp.m_W + x] == 0)
					Tmp.m_vGame[y * Tmp.m_W + x] = GOAL;
		gs_Dist.Build(Tmp, {GOAL});
	}

	// start state
	std::vector<STasInput> vPrefix;
	if(!gs_P.m_Prefix.empty())
		vPrefix = ReadInputs(gs_P.m_Prefix.c_str());
	auto pStart = std::make_unique<CTasGame>();
	pStart->Spawn(M.m_vSpawns[0]);
	for(const auto &In : vPrefix)
		pStart->Step(In);
	if(gs_P.m_Gren)
	{
		pStart->Chr()->GiveWeapon(WEAPON_GRENADE);
		pStart->Chr()->m_Core.m_ActiveWeapon = WEAPON_GRENADE;
	}
	if(gs_P.m_Tp)
		pStart->SetState(vec2(gs_P.m_TpX, gs_P.m_TpY), vec2(gs_P.m_TpVx, gs_P.m_TpVy));
	if(gs_P.m_Jumped >= 0)
		pStart->Chr()->m_Core.m_Jumped = gs_P.m_Jumped;
	if(gs_P.m_Started && !pStart->m_Started)
	{
		pStart->m_Started = true;
		pStart->m_StartTick = pStart->m_Tick;
	}
	const int StartTick = pStart->m_Tick;
	std::printf("start: tick %d pos %.1f %.1f vel %.2f %.2f started %d (race tick %d) dist %.0f\n", StartTick, pStart->Pos().x, pStart->Pos().y,
		pStart->Vel().x, pStart->Vel().y, pStart->m_Started, pStart->m_Started ? StartTick - pStart->m_StartTick : -1, Dist(*pStart));

	struct SState
	{
		std::unique_ptr<CTasGame> m_pG;
		STasInput m_Prev;
	};
	std::vector<SState> vBeam;
	vBeam.push_back({std::move(pStart), vPrefix.empty() ? STasInput() : vPrefix.back()});
	std::vector<std::vector<std::pair<int, STasInput>>> vHist; // per step: (parent, input) of each beam slot

	auto Reconstruct = [&](int Step, int Slot, const STasInput *pLast) {
		std::vector<STasInput> v;
		if(pLast)
			v.push_back(*pLast);
		for(int s = Step; s >= 0; s--)
		{
			v.push_back(vHist[s][Slot].second);
			Slot = vHist[s][Slot].first;
		}
		std::reverse(v.begin(), v.end());
		std::vector<STasInput> Full = vPrefix;
		Full.insert(Full.end(), v.begin(), v.end());
		return Full;
	};

	std::vector<SArrival> vArr;
	int FirstArrStep = -1;
	auto T0 = std::chrono::steady_clock::now();
	const int NT = std::max(1, gs_P.m_Threads);
	std::vector<SChild> vAll;
	for(int Step = 0; Step < gs_P.m_MaxTicks && !vBeam.empty(); Step++)
	{
		std::vector<std::vector<SChild>> vTC(NT);
		std::vector<std::vector<SArrival>> vTA(NT);
		std::atomic<int> Next{0};
		auto Worker = [&](int T) {
			CTasGame Tmp, Look;
			std::vector<STasInput> vActs;
			while(true)
			{
				int i = Next.fetch_add(1);
				if(i >= (int)vBeam.size())
					break;
				const CTasGame &G = *vBeam[i].m_pG;
				GenActions(G, vBeam[i].m_Prev, vActs);
				for(const STasInput &In : vActs)
				{
					Tmp.CopyFrom(G);
					Tmp.Step(In);
					if(Dead(Tmp))
						continue;
					if(GoalReached(Tmp) && (gs_GoalMode != 0 || Tmp.m_Started))
					{
						SArrival A;
						A.m_RaceTick = Tmp.m_Started ? Tmp.m_Tick - Tmp.m_StartTick : Tmp.m_Tick;
						A.m_Speed = length(Tmp.Vel());
						A.m_Pos = Tmp.Pos();
						A.m_Vel = Tmp.Vel();
						A.m_vIn.push_back(In);
						A.m_vIn.push_back(STasInput{(int8_t)0, 0, 0, 0, (int16_t)(i & 0x7fff), (int16_t)(i >> 15), 0});
						vTA[T].push_back(std::move(A));
						continue;
					}
					const CTasGame *pE = &Tmp;
					if(In.m_Fire && Tmp.NumProjectiles() > 0)
					{
						// judge a shot by the state after its explosion
						Look.CopyFrom(Tmp);
						STasInput L = In;
						L.m_Fire = 0;
						bool Bad = false;
						for(int k = 0; k < 10 && Look.NumProjectiles() > 0; k++)
						{
							Look.Step(L);
							if(Dead(Look))
							{
								Bad = true;
								break;
							}
						}
						if(Bad)
							continue;
						pE = &Look;
					}
					float D = Dist(*pE);
					if(D > 1e8f)
						continue;
					vTC[T].push_back({D, Speed(*pE), i, In, Tmp.Hash(), CellKey(Tmp), GroupKey(Tmp)});
				}
			}
		};
		std::vector<std::thread> vTh;
		for(int T = 1; T < NT; T++)
			vTh.emplace_back(Worker, T);
		Worker(0);
		for(auto &Th : vTh)
			Th.join();

		// arrivals
		for(auto &v : vTA)
			for(auto &A : v)
			{
				int Parent = A.m_vIn[1].m_TX | (A.m_vIn[1].m_TY << 15);
				std::vector<STasInput> Full = Reconstruct(Step - 1, Parent, &A.m_vIn[0]);
				if(Step == 0)
				{
					Full = vPrefix;
					Full.push_back(A.m_vIn[0]);
				}
				bool Dominated = false;
				for(auto &B : vArr)
					Dominated |= B.m_RaceTick <= A.m_RaceTick && B.m_Speed >= A.m_Speed - 0.25f;
				if(Dominated)
					continue;
				A.m_vIn = Full;
				std::printf("ARRIVAL race tick %d speed %.2f pos %.0f %.0f vel %.2f %.2f\n", A.m_RaceTick, A.m_Speed, A.m_Pos.x, A.m_Pos.y, A.m_Vel.x, A.m_Vel.y);
				char aBuf[64];
				std::snprintf(aBuf, sizeof(aBuf), ".arr%d.txt", (int)vArr.size());
				WriteInputs(gs_P.m_Out + aBuf, Full);
				if(vArr.empty())
					WriteInputs(gs_P.m_Out + ".best.txt", Full);
				vArr.push_back(std::move(A));
				if(FirstArrStep < 0)
					FirstArrStep = Step;
			}
		std::fflush(stdout);
		if(FirstArrStep >= 0 && Step - FirstArrStep >= gs_P.m_Arrivals)
			break;

		// selection
		vAll.clear();
		for(auto &v : vTC)
			vAll.insert(vAll.end(), v.begin(), v.end());
		if(vAll.empty())
		{
			std::printf("beam died at step %d\n", Step);
			break;
		}
		float MinD = 1e30f;
		for(auto &C : vAll)
			MinD = std::min(MinD, C.m_D);
		// cell dedupe: keep the best (d - alpha s) per cell
		std::unordered_map<uint64_t, int> Best;
		Best.reserve(vAll.size());
		std::unordered_set<uint64_t> Hashes;
		Hashes.reserve(vAll.size());
		std::vector<int> vIdx;
		for(int k = 0; k < (int)vAll.size(); k++)
		{
			auto &C = vAll[k];
			if(C.m_D > MinD + gs_P.m_Slack)
				continue;
			if(!Hashes.insert(C.m_Hash).second)
				continue;
			auto It = Best.find(C.m_Cell);
			if(It == Best.end())
				Best[C.m_Cell] = k;
			else
			{
				auto &O = vAll[It->second];
				if(C.m_D - gs_P.m_Alpha * C.m_S < O.m_D - gs_P.m_Alpha * O.m_S)
					It->second = k;
			}
		}
		for(auto &[K, k] : Best)
			vIdx.push_back(k);
		if(gs_P.m_Roll > 0)
		{
			// rollout value for the best RollPre*Beam candidates (by d - alpha s), the rest are dropped
			std::sort(vIdx.begin(), vIdx.end(), [&](int a, int b) { return vAll[a].m_D - gs_P.m_Alpha * vAll[a].m_S < vAll[b].m_D - gs_P.m_Alpha * vAll[b].m_S; });
			if((int)vIdx.size() > gs_P.m_RollPre * gs_P.m_Beam)
				vIdx.resize(gs_P.m_RollPre * gs_P.m_Beam);
			std::atomic<int> NextR{0};
			auto RW = [&]() {
				CTasGame Tmp;
				while(true)
				{
					int q = NextR.fetch_add(1);
					if(q >= (int)vIdx.size())
						break;
					auto &C = vAll[vIdx[q]];
					Tmp.CopyFrom(*vBeam[C.m_Parent].m_pG);
					Tmp.Step(C.m_In);
					C.m_D = RollValue(Tmp, gs_P.m_Roll, gs_P.m_Vref);
				}
			};
			std::vector<std::thread> vRT;
			for(int T = 1; T < NT; T++)
				vRT.emplace_back(RW);
			RW();
			for(auto &Th : vRT)
				Th.join();
		}
		// Pareto layers over (d ascending, s descending), computed separately in each spatial group;
		// the beam is filled layer by layer across all groups
		std::sort(vIdx.begin(), vIdx.end(), [&](int a, int b) { return vAll[a].m_D < vAll[b].m_D || (vAll[a].m_D == vAll[b].m_D && vAll[a].m_S > vAll[b].m_S); });
		std::vector<int> vLayer(vIdx.size(), -1);
		int Layers = 0;
		{
			std::unordered_map<uint64_t, std::vector<int>> Groups; // indices into vIdx, already d-sorted
			for(size_t q = 0; q < vIdx.size(); q++)
				Groups[vAll[vIdx[q]].m_Group].push_back((int)q);
			for(auto &[GK, vQ] : Groups)
			{
				int L = 0, Left = (int)vQ.size();
				while(Left > 0)
				{
					float BestS = -1e30f;
					for(int q : vQ)
					{
						if(vLayer[q] >= 0)
							continue;
						if(vAll[vIdx[q]].m_S > BestS + gs_P.m_Eps)
						{
							vLayer[q] = L;
							BestS = vAll[vIdx[q]].m_S;
							Left--;
						}
					}
					L++;
				}
				Layers = std::max(Layers, L);
			}
		}
		std::vector<int> vOrder(vIdx.size());
		for(size_t q = 0; q < vIdx.size(); q++)
			vOrder[q] = (int)q;
		std::sort(vOrder.begin(), vOrder.end(), [&](int a, int b) {
			if(vLayer[a] != vLayer[b])
				return vLayer[a] < vLayer[b];
			const auto &A = vAll[vIdx[a]], &B = vAll[vIdx[b]];
			return A.m_D - gs_P.m_Alpha * A.m_S < B.m_D - gs_P.m_Alpha * B.m_S;
		});
		std::vector<int> vSel;
		for(size_t q = 0; q < vOrder.size() && (int)vSel.size() < gs_P.m_Beam * (gs_P.m_Survive > 0 ? 2 : 1); q++)
			vSel.push_back(vIdx[vOrder[q]]);
		// materialize
		std::vector<SState> vNew;
		vNew.reserve(vSel.size());
		std::vector<std::pair<int, STasInput>> vH;
		for(int k : vSel)
		{
			const auto &C = vAll[k];
			SState S;
			S.m_pG = std::make_unique<CTasGame>();
			S.m_pG->CopyFrom(*vBeam[C.m_Parent].m_pG);
			S.m_pG->Step(C.m_In);
			if((int)vNew.size() >= gs_P.m_Beam)
				break;
			if(gs_P.m_Survive > 0 && !Survives(*S.m_pG, gs_P.m_Survive))
				continue;
			S.m_Prev = C.m_In;
			vH.push_back({C.m_Parent, C.m_In});
			vNew.push_back(std::move(S));
		}
		vHist.push_back(std::move(vH));
		if(Step == gs_P.m_Dump)
			for(auto &S : vNew)
				std::printf("DUMP %.1f %.1f %.2f %.2f hook %d j %d\n", S.m_pG->Pos().x, S.m_pG->Pos().y, S.m_pG->Vel().x, S.m_pG->Vel().y, S.m_pG->HookState(), JumpsLeft(*S.m_pG));
		vBeam = std::move(vNew);
		if(!gs_P.m_Quiet && (Step % 25 == 0))
		{
			float MaxS = -1e30f;
			const CTasGame *pLead = nullptr;
			float LeadD = 1e30f;
			for(auto &S : vBeam)
			{
				MaxS = std::max(MaxS, Speed(*S.m_pG));
				float D = Dist(*S.m_pG);
				if(D < LeadD)
				{
					LeadD = D;
					pLead = S.m_pG.get();
				}
			}
			double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
			if(pLead)
				std::printf("step %d: cands %zu cells %zu layers %d beam %zu | lead d %.0f pos %.0f %.0f vel %.1f %.1f | max s %.1f [%.1fs]\n", Step, vAll.size(), vIdx.size(), Layers,
					vBeam.size(), LeadD, pLead->Pos().x, pLead->Pos().y, pLead->Vel().x, pLead->Vel().y, MaxS, Sec);
			std::fflush(stdout);
		}
	}
	if(vArr.empty())
		std::printf("NO ARRIVAL\n");
	else
	{
		int Best = 1 << 30;
		for(auto &A : vArr)
			Best = std::min(Best, A.m_RaceTick);
		std::printf("BEST race tick %d (%zu Pareto arrivals) [%.1fs]\n", Best, vArr.size(), std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count());
	}
	return 0;
}
