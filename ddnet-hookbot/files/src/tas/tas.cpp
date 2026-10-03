// TAS search: beam search over real DDRace physics (see sim.h).
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "sim.h"

#include <game/mapitems.h>

#include <algorithm>
#include <atomic>
#include <ctime>
#include <mutex>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
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

static void WriteInputs(const char *pPath, const std::vector<STasInput> &v)
{
	FILE *f = std::fopen(pPath, "w");
	for(const auto &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

// ---------------------------------------------------------------------------
// geodesic distance (in px) from the tee center to the finish, on a fine grid
static int gs_Stencil16 = 1;

struct SDistField
{
	static constexpr int CELL = 4;
	int m_W = 0, m_H = 0;
	std::vector<float> m_vD;
	const std::vector<float> *m_pTileCost = nullptr; // optional per-map-tile cost multiplier (vref / expected speed)

	static bool CenterOk(const SMapInfo &M, float x, float y)
	{
		// the tee box (28x28) must not overlap solid tiles, and its center must stay out of freeze
		const float r = 14.0f;
		float axs[2] = {x - r, x + r - 0.01f}, ays[2] = {y - r, y + r - 0.01f};
		for(float ax : axs)
			for(float ay : ays)
			{
				int t = M.Tile((int)std::floor(ax / 32), (int)std::floor(ay / 32));
				if(t == TILE_SOLID || t == TILE_NOHOOK)
					return false;
			}
		int cx = (int)std::floor(x / 32), cy = (int)std::floor(y / 32);
		int t = M.Tile(cx, cy), f = M.Front(cx, cy);
		if(t == TILE_FREEZE || f == TILE_FREEZE || t == TILE_DEATH || f == TILE_DEATH)
			return false;
		return true;
	}

	void Build(const SMapInfo &M, const std::vector<int> &vGoalTiles)
	{
		m_W = M.m_W * 32 / CELL;
		m_H = M.m_H * 32 / CELL;
		std::vector<uint8_t> vOk(m_W * m_H);
		for(int y = 0; y < m_H; y++)
			for(int x = 0; x < m_W; x++)
				vOk[y * m_W + x] = CenterOk(M, x * CELL + CELL / 2.0f, y * CELL + CELL / 2.0f);
		m_vD.assign(m_W * m_H, 1e9f);
		using QE = std::pair<float, int>;
		std::priority_queue<QE, std::vector<QE>, std::greater<QE>> Q;
		for(int y = 0; y < m_H; y++)
			for(int x = 0; x < m_W; x++)
			{
				int tx = x * CELL / 32, ty = y * CELL / 32;
				int t = M.Tile(tx, ty), f = M.Front(tx, ty);
				if(std::find(vGoalTiles.begin(), vGoalTiles.end(), t) != vGoalTiles.end() ||
					std::find(vGoalTiles.begin(), vGoalTiles.end(), f) != vGoalTiles.end())
				{
					m_vD[y * m_W + x] = 0;
					Q.push({0.f, y * m_W + x});
				}
			}
		// 16-neighbour stencil (knight moves too): the 8-neighbour one overestimates some diagonal
		// distances by up to 8%, which steers the search away from diagonal lines
		// stencil: all moves with |dx|,|dy| <= R and gcd 1 (R = 1: 8-neighbour, 2: 16, 3: 32)
		std::vector<int> aDx, aDy;
		std::vector<float> aC;
		const int R = gs_Stencil16 >= 2 ? gs_Stencil16 : (gs_Stencil16 ? 2 : 1);
		for(int pass = 0; pass < 2; pass++)
			for(int dy = -R; dy <= R; dy++)
				for(int dx = -R; dx <= R; dx++)
				{
					if(!dx && !dy)
						continue;
					int a = std::abs(dx), b = std::abs(dy);
					while(b)
					{
						int t = a % b;
						a = b;
						b = t;
					}
					if(a != 1)
						continue;
					bool Unit = std::abs(dx) <= 1 && std::abs(dy) <= 1;
					if(Unit != (pass == 0))
						continue;
					aDx.push_back(dx);
					aDy.push_back(dy);
					aC.push_back(std::sqrt((float)(dx * dx + dy * dy)));
				}
		const int NumN = (int)aDx.size();
		auto Ok = [&](int x, int y) { return x >= 0 && y >= 0 && x < m_W && y < m_H && vOk[y * m_W + x]; };
		while(!Q.empty())
		{
			auto [d, i] = Q.top();
			Q.pop();
			if(d > m_vD[i])
				continue;
			int x = i % m_W, y = i / m_W;
			for(int k = 0; k < NumN; k++)
			{
				int nx = x + aDx[k], ny = y + aDy[k];
				if(nx < 0 || ny < 0 || nx >= m_W || ny >= m_H)
					continue;
				int j = ny * m_W + nx;
				if(!vOk[j])
					continue;
				if(k >= 8)
				{
					// every cell the move passes through must be free
					bool Clear = true;
					const int Steps = 4 * std::max(std::abs(aDx[k]), std::abs(aDy[k]));
					for(int t = 1; t < Steps && Clear; t++)
					{
						float fx = x + 0.5f + aDx[k] * (float)t / Steps, fy = y + 0.5f + aDy[k] * (float)t / Steps;
						Clear = Ok((int)std::floor(fx), (int)std::floor(fy));
					}
					if(!Clear)
						continue;
				}
				float Cost = 1.0f;
				if(m_pTileCost)
				{
					const int MW = m_W * CELL / 32;
					const int tx0 = x * CELL / 32, ty0 = y * CELL / 32, tx1 = nx * CELL / 32, ty1 = ny * CELL / 32;
					Cost = 0.5f * ((*m_pTileCost)[ty0 * MW + tx0] + (*m_pTileCost)[ty1 * MW + tx1]);
				}
				float nd = d + aC[k] * CELL * Cost;
				if(nd < m_vD[j])
				{
					m_vD[j] = nd;
					Q.push({nd, j});
				}
			}
		}
	}

	void BuildFromPoint(const SMapInfo &M, vec2 P)
	{
		// goal = cells within pickup range of P
		SMapInfo Tmp = M;
		int tx = (int)(P.x / 32), ty = (int)(P.y / 32);
		const uint8_t GOAL = 250;
		Tmp.m_vGame[ty * Tmp.m_W + tx] = GOAL;
		Build(Tmp, {GOAL});
	}

	float At(int x, int y) const
	{
		x = std::clamp(x, 0, m_W - 1);
		y = std::clamp(y, 0, m_H - 1);
		return m_vD[y * m_W + x];
	}
	// bilinear sample; unreachable cells are 1e9, so avoid mixing them in
	float Sample(vec2 p) const
	{
		float fx = p.x / CELL - 0.5f, fy = p.y / CELL - 0.5f;
		int x0 = (int)std::floor(fx), y0 = (int)std::floor(fy);
		float ax = fx - x0, ay = fy - y0;
		float a[4] = {At(x0, y0), At(x0 + 1, y0), At(x0, y0 + 1), At(x0 + 1, y0 + 1)};
		float w[4] = {(1 - ax) * (1 - ay), ax * (1 - ay), (1 - ax) * ay, ax * ay};
		float s = 0, ws = 0, mn = 1e9f;
		for(int k = 0; k < 4; k++)
		{
			mn = std::min(mn, a[k]);
			if(a[k] < 1e8f)
			{
				s += a[k] * w[k];
				ws += w[k];
			}
		}
		if(ws <= 0)
			return 1e9f;
		return s / ws;
	}
	vec2 Grad(vec2 p) const
	{
		const float h = 6.0f;
		float dx = Sample(p + vec2(h, 0)) - Sample(p - vec2(h, 0));
		float dy = Sample(p + vec2(0, h)) - Sample(p - vec2(0, h));
		if(std::fabs(dx) > 1e6f || std::fabs(dy) > 1e6f)
			return vec2(0, 0);
		vec2 g(dx, dy);
		float l = length(g);
		return l > 1e-6f ? g / l : vec2(0, 0);
	}
};

// Progress coordinate from a harmonic potential (Laplace's equation over the free space, 0 at the
// source, 1 at the goal). Its level lines cross corridors perpendicular to the walls, unlike
// geodesic distance, which favors hugging inside corners. Mapped back to px via the geodesic distance.
struct SHarmonic
{
	static constexpr int CELL = 16;
	int m_W = 0, m_H = 0;
	std::vector<float> m_vPhi; // -1 = blocked
	std::vector<float> m_vMap; // phi bin -> distance to go (px)

	void Build(const SMapInfo &M, const std::vector<vec2> &vSource, float SourceR, const std::vector<int> &vGoalTiles, const SDistField &Geo, const SDistField &FromSource)
	{
		m_W = M.m_W * 32 / CELL;
		m_H = M.m_H * 32 / CELL;
		const int N = m_W * m_H;
		std::vector<int8_t> vType(N, 0); // 0 free, 1 blocked, 2 source (phi 0), 3 goal (phi 1)
		for(int y = 0; y < m_H; y++)
			for(int x = 0; x < m_W; x++)
			{
				vec2 C(x * CELL + CELL / 2.0f, y * CELL + CELL / 2.0f);
				int i = y * m_W + x;
				int tx = (int)(C.x / 32), ty = (int)(C.y / 32);
				int t = M.Tile(tx, ty), f = M.Front(tx, ty);
				bool Goal = std::find(vGoalTiles.begin(), vGoalTiles.end(), t) != vGoalTiles.end() ||
					    std::find(vGoalTiles.begin(), vGoalTiles.end(), f) != vGoalTiles.end();
				if(Goal)
					vType[i] = 3;
				else if(!SDistField::CenterOk(M, C.x, C.y))
					vType[i] = 1;
				for(vec2 S : vSource)
					if(distance(S, C) < SourceR && vType[i] == 0)
						vType[i] = 2;
			}
		// only cells connected to both ends matter; unknowns = free cells
		std::vector<double> x(N, 0.0), r(N, 0.0), p(N, 0.0), Ap(N, 0.0);
		const int aD[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
		auto Fixed = [&](int i) { return vType[i] == 2 ? 0.0 : 1.0; };
		// b = sum of fixed neighbor values; A = degree - adjacency among free cells
		auto ApplyA = [&](const std::vector<double> &v, std::vector<double> &out) {
			for(int y = 0; y < m_H; y++)
				for(int xx = 0; xx < m_W; xx++)
				{
					int i = y * m_W + xx;
					if(vType[i] != 0)
					{
						out[i] = 0;
						continue;
					}
					double Acc = 0;
					for(auto &d : aD)
					{
						int nx = xx + d[0], ny = y + d[1];
						if(nx < 0 || ny < 0 || nx >= m_W || ny >= m_H)
							continue;
						int j = ny * m_W + nx;
						if(vType[j] == 1)
							continue;
						Acc += v[i];
						if(vType[j] == 0)
							Acc -= v[j];
					}
					out[i] = Acc;
				}
		};
		std::vector<double> b(N, 0.0);
		for(int y = 0; y < m_H; y++)
			for(int xx = 0; xx < m_W; xx++)
			{
				int i = y * m_W + xx;
				if(vType[i] != 0)
					continue;
				for(auto &d : aD)
				{
					int nx = xx + d[0], ny = y + d[1];
					if(nx < 0 || ny < 0 || nx >= m_W || ny >= m_H)
						continue;
					int j = ny * m_W + nx;
					if(vType[j] >= 2)
						b[i] += Fixed(j);
				}
			}
		// start from the geodesic-based guess to speed up convergence
		for(int i = 0; i < N; i++)
			x[i] = 0.5;
		ApplyA(x, Ap);
		double rr = 0;
		for(int i = 0; i < N; i++)
		{
			r[i] = vType[i] == 0 ? b[i] - Ap[i] : 0;
			p[i] = r[i];
			rr += r[i] * r[i];
		}
		const double rr0 = rr;
		int It = 0;
		for(; It < 200000 && rr > rr0 * 1e-20 && rr > 1e-24; It++)
		{
			ApplyA(p, Ap);
			double pAp = 0;
			for(int i = 0; i < N; i++)
				pAp += p[i] * Ap[i];
			if(pAp <= 0)
				break;
			double a = rr / pAp;
			double rr2 = 0;
			for(int i = 0; i < N; i++)
			{
				x[i] += a * p[i];
				r[i] -= a * Ap[i];
				rr2 += r[i] * r[i];
			}
			double be = rr2 / rr;
			rr = rr2;
			for(int i = 0; i < N; i++)
				p[i] = r[i] + be * p[i];
		}
		m_vPhi.assign(N, -1.0f);
		for(int i = 0; i < N; i++)
		{
			if(vType[i] == 0)
				m_vPhi[i] = (float)std::clamp(x[i], 0.0, 1.0);
			else if(vType[i] >= 2)
				m_vPhi[i] = (float)Fixed(i);
		}
		// map phi -> geodesic distance to go (bin average, then made monotone)
		const int NB = 20000;
		std::vector<double> vSum(NB, 0), vCnt(NB, 0);
		for(int y = 0; y < m_H; y++)
			for(int xx = 0; xx < m_W; xx++)
			{
				int i = y * m_W + xx;
				if(m_vPhi[i] < 0 || vType[i] != 0)
					continue;
				vec2 C(xx * CELL + CELL / 2.0f, y * CELL + CELL / 2.0f);
				float D = Geo.Sample(C);
				if(D > 1e8f)
					continue;
				// only cells on the through-route (not in dead ends hanging off it)
				float Total = Geo.Sample(vSource[0]);
				if(D + FromSource.Sample(C) > Total + 1500.0f)
					continue;
				int k = std::min(NB - 1, (int)(m_vPhi[i] * NB));
				vSum[k] += D;
				vCnt[k] += 1;
			}
		m_vMap.assign(NB + 1, 0.0f);
		// fill: interpolate empty bins
		std::vector<float> vVal(NB, -1);
		for(int k = 0; k < NB; k++)
			if(vCnt[k] > 0)
				vVal[k] = vSum[k] / vCnt[k];
		float Last = -1;
		for(int k = NB - 1; k >= 0; k--)
		{
			if(vVal[k] < 0)
				vVal[k] = Last < 0 ? 0 : Last;
			// non-increasing in phi: distance-to-go at bin k must be >= at bin k+1
			if(Last >= 0 && vVal[k] < Last)
				vVal[k] = Last;
			Last = vVal[k];
		}
		for(int k = 0; k < NB; k++)
			m_vMap[k] = vVal[k];
		m_vMap[NB] = 0;
		std::printf("harmonic: %d cells, CG %d iterations, residual %.3g, D(phi=0)=%.0f\n", N, It, std::sqrt(rr / std::max(rr0, 1e-30)), m_vMap[0]);
	}

	float PhiAt(int x, int y) const
	{
		x = std::clamp(x, 0, m_W - 1);
		y = std::clamp(y, 0, m_H - 1);
		return m_vPhi[y * m_W + x];
	}
	float Phi(vec2 p) const
	{
		float fx = p.x / CELL - 0.5f, fy = p.y / CELL - 0.5f;
		int x0 = (int)std::floor(fx), y0 = (int)std::floor(fy);
		float ax = fx - x0, ay = fy - y0;
		float a[4] = {PhiAt(x0, y0), PhiAt(x0 + 1, y0), PhiAt(x0, y0 + 1), PhiAt(x0 + 1, y0 + 1)};
		float w[4] = {(1 - ax) * (1 - ay), ax * (1 - ay), (1 - ax) * ay, ax * ay};
		float s = 0, ws = 0;
		for(int k = 0; k < 4; k++)
			if(a[k] >= 0)
			{
				s += a[k] * w[k];
				ws += w[k];
			}
		return ws > 0 ? s / ws : -1.0f;
	}
	// distance to go in px (1e9 if outside)
	float Sample(vec2 p) const
	{
		float f = Phi(p);
		if(f < 0)
			return 1e9f;
		float k = f * (float)(m_vMap.size() - 1);
		int k0 = std::clamp((int)k, 0, (int)m_vMap.size() - 2);
		float t = k - k0;
		return m_vMap[k0] * (1 - t) + m_vMap[k0 + 1] * t;
	}
	vec2 Grad(vec2 p) const
	{
		const float h = 8.0f;
		float dx = Sample(p + vec2(h, 0)) - Sample(p - vec2(h, 0));
		float dy = Sample(p + vec2(0, h)) - Sample(p - vec2(0, h));
		if(std::fabs(dx) > 1e6f || std::fabs(dy) > 1e6f)
			return vec2(0, 0);
		vec2 g(dx, dy);
		float l = length(g);
		return l > 1e-6f ? g / l : vec2(0, 0);
	}
};

static SHarmonic gs_HarmA, gs_HarmB; // spawn -> grenade, grenade -> finish
static float gs_StartOffset = 0;

static SDistField gs_Dist;
static std::vector<float> gs_vTileCost; // from speedfield=
static float gs_aHookPrior[3][24], gs_aShotPrior[3][24];
static bool gs_HasPrior = false;
static thread_local std::vector<float> gs_vActCost; // -log prior per generated action (parallel to GenActions output)
static float PriorOf(const float aP[3][24], vec2 Aim, vec2 V)
{
	float Sp = length(V);
	int Band = Sp < 15 ? 0 : (Sp < 30 ? 1 : 2);
	float a = std::atan2(Aim.y, Aim.x), b = Sp > 1 ? std::atan2(V.y, V.x) : 0.0f;
	float d = std::fmod(a - b + 3 * pi, 2 * pi) - pi;
	int Bin = std::clamp((int)((d + pi) / (2 * pi) * 24), 0, 23);
	return aP[Band][Bin];
}
static SDistField gs_DistGrenade; // distance to the grenade pickup

// ---------------------------------------------------------------------------
struct SParams
{
	int m_Beam = 500;
	int m_Angles = 96;
	int m_MaxTicks = 4000;
	float m_Vref = 16.0f;
	float m_Alpha = 8.0f;
	int m_Threads = 2;
	int m_PerCell = 2;
	float m_CellSize = 12.0f;
	float m_VelCell = 3.0f;
	int m_PreStartMax = 250;
	bool m_UseGrenade = true;
	std::string m_Out = "best.txt";
	std::string m_Prefix; // optional input prefix to start from
	int m_PrefixLen = -1;
	float m_GrenadeBonus = 3000.0f; // px credit for having the grenade (in D units)
	bool m_StopAtGrenade = false;
	int m_Rollout = 0; // 0 = plain score
	int m_Survive = 0; // survival check horizon (0 = off)
	float m_PrestartObj = 0;
	int m_SpeedObj = 0;
	float m_PrestartEnergy = 0;
	int m_Harmonic = 0;
	int m_ProgX = 0;
	float m_JumpBonus = 0;
	int m_RichKey = 0;
	int m_PerParent = 0;
	float m_SpeedShare = 0;
	float m_JumpEnergy = 0;
	int m_FireLookahead = 0;
	int m_FireAngles = 64;
	float m_FireRange = 300;
	float m_DynVref = 0; // px horizon over which the current speed counts (0 = off)
	float m_DynVmin = 12;
	float m_DynVmax = 40;
	float m_EnergyShare = 0;
	float m_RandShare = 0;
	float m_GhostShare = 0;
	int m_Visited = 0;
	int m_Stencil16 = 1;
	float m_VCell = 32;
	float m_VVel = 4;
	std::string m_RefFile;
	std::string m_GhostFile;
	int m_Track = 0;
	int m_Repeat = 1;
	float m_EKappa = 0;
	std::vector<int> m_vMacro;
	double m_Brute = 0;
	int m_MacroOnly = 0;
	int m_Teleport = 0;
	float m_TpX = 0, m_TpY = 0, m_TpVx = 0, m_TpVy = 0;
	int m_TpJumped = -1;
	int m_PrefixJumped = -1;
	int m_TpStarted = 0;
	int m_TargetMinTick = 0;
	int m_GhostShift = 0;
	int m_GateCands = 0;
	std::string m_SpeedField;
	std::string m_Prior; // action prior file (hook/shot aim histograms)
	int m_PriorTop = 0; // keep only the top-M hook aims by prior (0 = all)
	int m_PriorShots = 0; // same for shot aims
	float m_PriorKappa = 0; // score penalty per -log prior // per-tile expected speed file: distance fields become time-at-vref fields
	float m_SpeedFieldDefault = 25.0f; // distance gate: write the top-K crossing states as prefixes
	float m_GateLambda = 1.0f; // ticks credited per px/tick of speed along the route at the gate // track mode: compare our race tick + shift with the ghost's // brute: the target only counts this many race ticks after the start
	float m_TmpGoalX = 0, m_TmpGoalY = 0; // experiment: steer the pre-grenade distance field to this point // teleport: race already started // experiment: override the jump state after replaying the prefix
	float m_PreYW = 1.0f; // weight of height in the prestart energy
	float m_PreYMin = -1e9f, m_PreVyMax = 1e9f;
	float m_PreVyW = 0; // prestart score bonus per px/tick of upward speed // crossing filter for precands
	float m_PreEvalX = 0;
	float m_JumpVyMin = -1e9f;
	float m_SpeedGamma = 0; // time to go = D / (vref + gamma*(speed - vref)) instead of the alpha term
	float m_StopXLt = 0;
	float m_StopBox[4] = {0, 0, 0, 0}; // stop when grounded inside this box (x0,y0,x1,y1), saved like stopgren
	int m_StopBoxOn = 0;
	float m_StopRegion[4] = {0, 0, 0, 0}; // stop when inside this box (any state)
	int m_StopRegionOn = 0;
	int m_GateSpeed = 0; // gate keeps the fastest arrival instead of the earliest // stop when a started state passes this x leftwards // no new air-jump press while rising faster than this (vy below it) // rollout gate: rank crossings by a short corridor search to this x
	float m_TrackVel = 3;
	float m_ShareSlack = 40; // ticks
	int m_HookDedup = 1;
	float m_HeightBonus = 0;
	float m_StopX = 0; // stop when a started state passes this x (segment experiments)
	float m_StopD = 0; // gate: distance to go below this
	int m_GateSlack = 8; // keep collecting gate crossings for this many ticks
	int m_BestCross = 0;
	int m_RolloutDirs = 1;
	int m_Prefilter = 4; // rollout-score the best Prefilter*Beam candidates by the plain score
	int m_Quiet = 0;
	int m_PreCands = 0; // prestart: write the top-N highest-energy crossings as prefixes
	int m_NoFireBefore = 0; // experiment: no shots before this race tick
	int m_CommitFire = 0; // allow shots during committed hook macros
	float m_BandShare = 0; // beam share for the best candidates BandLo..BandHi ticks behind the leader (one per cell)
	float m_BandLo = 3, m_BandHi = 12;
	float m_CrashPen = 0; // px of score per unit of v^2 a short continuation loses to collisions
	int m_CrashK = 12;
	float m_HookPen = 0; // px of score per unit of v^2 braked away by hooks
	int m_PadAims = 0; // coarse aim count for edge-refined pad shots (0 = off)
	float m_BendCredit = 0; // credit (fraction of a kick) for a loaded grenade before a bend
	float m_BendLen = 500; // px of route ahead considered for the bend
	float m_PadOpt = 0; // optimistic credit (fraction of a full kick) for a reachable pending explosion
	int m_TpFirst = 0; // teleport before replaying the prefix
	float m_PadCredit = 0; // credit per px/tick of kick a pending grenade would give on the current course
	int m_OptVal = 0; // option value of a loaded grenade for the best OptVal*Beam candidates (0 = off)
	float m_KCredit = 0; // credit per px/tick of point-blank kick potential when the grenade is (nearly) loaded
	int m_KReady = 4; // reload ticks within which the potential counts
	int m_ClanCap = 0; // max beam states per lineage (0 = off)
	int m_ClanPeriod = 10; // steps between lineage resets
	float m_GCredit = 0; // px/tick of speed credited for a loaded grenade
	int m_TpGren = 0; // teleport: also give the grenade
	int m_PendRoll = 0; // pendshare quota ranked by the rollout-predicted miss distance to the pad
	float m_PendWin = 200; // px
	float m_PendBonus = 0; // ticks of credit for a well placed pending explosion (pendshare ranking only)
	float m_PendShare = 0; // beam share for states with a pending explosion
	int m_ProjKey = 0; // cell key includes the pending explosion (place, time)
	int m_FireLookMax = 12; // judge states with a grenade in flight this many ticks ahead at most
};

static SParams gs_P;

struct SCrossCand
{
	float m_E;
	int64_t m_Key;
	int m_Parent;
	STasInput m_In;
	vec2 m_Pos, m_Vel;
	int m_Jumps;
	int m_Tick;
};
struct SCrossBest
{
	float m_E = -1e30f;
	vec2 m_Pos, m_Vel;
	int m_Jumps = 0, m_Tick = 0;
	std::vector<STasInput> m_vIn;
};
static std::unordered_map<int64_t, SCrossBest> gs_Cross;
static std::mutex gs_CrossMx;
static int gs_StopTicks = 1 << 30; // ticks after the start at which the last search reached stopx
static vec2 gs_GrenadePos;

// progress along a reference trajectory (a previous run): distance to go = remaining arc length
struct SRefPath
{
	std::vector<vec2> m_vPos;
	std::vector<float> m_vLen; // cumulative arc length
	float m_Total = 0;
	std::vector<float> m_vTime; // ghost mode: race tick of each point
	int m_GrenadeIdx = 1 << 30; // ghost mode: no progress past this point without the grenade
	bool Active() const { return !m_vPos.empty(); }
	bool Ghost() const { return !m_vTime.empty(); }
	void LoadGhost(const char *pPath, vec2 GrenadePos)
	{
		FILE *f = std::fopen(pPath, "r");
		int t;
		float x, y;
		while(f && std::fscanf(f, "%d %f %f", &t, &x, &y) == 3)
		{
			m_vPos.emplace_back(x, y);
			m_vTime.push_back((float)t);
		}
		if(f)
			std::fclose(f);
		m_vLen.resize(m_vPos.size());
		m_vLen[0] = 0;
		float Best = 1e30f;
		for(size_t i = 0; i < m_vPos.size(); i++)
		{
			if(i)
				m_vLen[i] = m_vLen[i - 1] + distance(m_vPos[i - 1], m_vPos[i]);
			float d = distance(m_vPos[i], GrenadePos);
			if(d < Best)
			{
				Best = d;
				m_GrenadeIdx = (int)i;
			}
		}
		m_Total = m_vLen.back();
		std::printf("ghost: %zu points, finish at race tick %.0f, grenade at point %d (tick %.0f)\n", m_vPos.size(), m_vTime.back(), m_GrenadeIdx, m_vTime[m_GrenadeIdx]);
	}
	// ghost's remaining race time from the projection of P near index Idx
	float TimeToGo(int Idx, vec2 P) const
	{
		int i = std::clamp(Idx, 0, (int)m_vPos.size() - 2);
		vec2 A = m_vPos[i], B = m_vPos[i + 1];
		vec2 AB = B - A;
		float L2 = dot(AB, AB);
		float t = L2 > 1e-6f ? std::clamp(dot(P - A, AB) / L2, -1.0f, 1.0f) : 0.0f;
		return m_vTime.back() - (m_vTime[i] + t * (m_vTime[i + 1] - m_vTime[i]));
	}
	void Load(const std::vector<STasInput> &vIn)
	{
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		m_vPos.push_back(G.Pos());
		for(const auto &In : vIn)
		{
			G.Step(In);
			m_vPos.push_back(G.Pos());
			if(G.m_FinishTick >= 0)
				break;
		}
		m_vLen.resize(m_vPos.size());
		m_vLen[0] = 0;
		for(size_t i = 1; i < m_vPos.size(); i++)
			m_vLen[i] = m_vLen[i - 1] + distance(m_vPos[i - 1], m_vPos[i]);
		m_Total = m_vLen.back();
		std::printf("reference path: %zu points, %.0f px\n", m_vPos.size(), m_Total);
	}
	// nearest point on the path near the previous index (never jumps to a parallel corridor)
	int Track(int Prev, vec2 P) const
	{
		int Best = Prev;
		float BestD = 1e30f;
		int Lo = std::max(0, Prev - 8), Hi = std::min((int)m_vPos.size() - 1, Prev + 60);
		for(int i = Lo; i <= Hi; i++)
		{
			float d = distance(m_vPos[i], P);
			if(d < BestD)
			{
				BestD = d;
				Best = i;
			}
		}
		return Best;
	}
	float ToGo(int Idx, vec2 P) const
	{
		// project onto the segment after (or before) the index for continuity
		int i = std::clamp(Idx, 0, (int)m_vPos.size() - 2);
		vec2 A = m_vPos[i], B = m_vPos[i + 1];
		vec2 AB = B - A;
		float L2 = dot(AB, AB);
		float t = L2 > 1e-6f ? std::clamp(dot(P - A, AB) / L2, -1.0f, 1.0f) : 0.0f;
		return m_Total - (m_vLen[i] + t * std::sqrt(L2));
	}
	vec2 Tangent(int Idx) const
	{
		int i = std::clamp(Idx, 0, (int)m_vPos.size() - 2);
		int j = std::min(i + 3, (int)m_vPos.size() - 1);
		vec2 D = m_vPos[j] - m_vPos[i];
		float l = length(D);
		return l > 1e-6f ? D / l : vec2(1, 0);
	}
};
static SRefPath gs_Ref;

static void UpdateRef(CTasGame &G)
{
	if(gs_Ref.Active())
	{
		G.m_RefIdx = gs_Ref.Track(G.m_RefIdx, G.Pos());
		if(gs_Ref.Ghost() && !G.HasGrenade())
			G.m_RefIdx = std::min(G.m_RefIdx, gs_Ref.m_GrenadeIdx);
	}
}

// one search step: starts a macro commitment when the input asks for one
static void StepIn(CTasGame &G, const STasInput &In)
{
	if(In.m_Commit && G.m_CommitLeft == 0)
	{
		G.m_CommitLeft = In.m_Commit;
		G.m_CommitIn = In;
		G.m_CommitIn.m_Commit = 0;
	}
	vec2 V0 = G.Vel();
	G.Step(In);
	UpdateRef(G);
	if(G.m_CommitLeft > 0)
		G.m_CommitLeft--;
	if(gs_P.m_HookPen > 0 && G.HookState() == 5 && length(V0) > 15.0f)
	{
		vec2 V1 = G.Vel();
		// gravity changes v^2 by about 2*vy*g; only count what the hook removed
		float Expected = dot(V0, V0) + V0.y + 0.25f;
		G.m_HookLoss += std::max(0.0f, Expected - dot(V1, V1));
	}
}

static float GeoDistTo(const CTasGame &G, vec2 P);

static float DistTo(const CTasGame &G, vec2 P)
{
	if(gs_Ref.Ghost())
	{
		// racing the ghost: its remaining time at our point on its line, in "px" so vref cancels
		float T = gs_Ref.TimeToGo(G.m_RefIdx, P);
		if(!G.HasGrenade() && G.m_RefIdx >= gs_Ref.m_GrenadeIdx)
			T += distance(P, gs_GrenadePos) / gs_P.m_Vref; // still has to reach the grenade
		return T * gs_P.m_Vref;
	}
	if(gs_Ref.Active())
		return gs_Ref.ToGo(G.m_RefIdx, P);
	return GeoDistTo(G, P);
}

// geodesic distance to go (via the grenade before picking it up); used for gates
static float GeoDistTo(const CTasGame &G, vec2 P)
{
	if(gs_P.m_ProgX)
		return 20000.0f - P.x;
	if(gs_P.m_Harmonic)
	{
		if(G.HasGrenade() || !gs_P.m_UseGrenade)
			return gs_HarmB.Sample(P);
		if(!G.m_Started)
			return gs_Dist.Sample(P) + gs_StartOffset; // the harmonic field is flat near the spawn
		return gs_HarmA.Sample(P) + gs_Dist.Sample(gs_GrenadePos);
	}
	if(G.HasGrenade() || !gs_P.m_UseGrenade)
		return gs_Dist.Sample(P);
	return gs_DistGrenade.Sample(P) + gs_Dist.Sample(gs_GrenadePos);
}

static vec2 GradTo(const CTasGame &G, vec2 P)
{
	if(gs_Ref.Active())
		return -gs_Ref.Tangent(G.m_RefIdx);
	if(gs_P.m_ProgX)
		return vec2(-1, 0);
	if(gs_P.m_Harmonic && !G.m_Started && !G.HasGrenade())
		return gs_Dist.Grad(P);
	if(gs_P.m_Harmonic)
		return G.HasGrenade() || !gs_P.m_UseGrenade ? gs_HarmB.Grad(P) : gs_HarmA.Grad(P);
	return G.HasGrenade() || !gs_P.m_UseGrenade ? gs_Dist.Grad(P) : gs_DistGrenade.Grad(P);
}

// estimated total ticks using short core-only rollouts: min over the rollout of (k + D_k / Vref)
static float RolloutScore(const CTasGame &G)
{
	vec2 aPos[128];
	const int K = std::min(gs_P.m_Rollout, 128);
	float Best = DistTo(G, G.Pos()) / gs_P.m_Vref;
	if(Best > 1e7f)
		return 1e9f;
	vec2 Gr = GradTo(G, G.Pos());
	int Dir = Gr.x < -0.2f ? 1 : Gr.x > 0.2f ? -1 : 0;
	for(int Keep = 0; Keep <= (G.m_LastHook ? 1 : 0); Keep++)
		for(int d = -1; d <= 1; d++)
		{
			if(d != Dir && gs_P.m_RolloutDirs == 1)
				continue;
			int n = G.Rollout(K, d, Keep, aPos);
			for(int k = 0; k < n; k++)
			{
				float D = DistTo(G, aPos[k]);
				if(D > 1e7f)
					break;
				Best = std::min(Best, (k + 1) + D / gs_P.m_Vref);
			}
		}
	float Elapsed = G.m_Started ? (float)(G.m_Tick - G.m_StartTick) : 0.0f;
	return Elapsed + Best;
}

// tracking mode: distance (position + velocity) to the ghost at the same race time
static float TrackScore(const CTasGame &G)
{
	const auto &R = gs_Ref;
	int T0 = (int)R.m_vTime[0]; // ghost's first point is at this race tick (negative: before the start)
	int RaceTick = (G.m_Started ? G.m_Tick - G.m_StartTick : G.m_Tick + T0) + gs_P.m_GhostShift;
	int i = std::clamp(RaceTick - T0, 0, (int)R.m_vPos.size() - 2);
	vec2 GP = R.m_vPos[i];
	vec2 GV = R.m_vPos[i + 1] - R.m_vPos[i];
	return distance(G.Pos(), GP) + gs_P.m_TrackVel * distance(G.Vel(), GV);
}

// per map tile: how much the route turns over the next BendLen px (radians), following the distance gradient
static std::vector<float> gs_vBend;
static void BuildBend()
{
	const SMapInfo &M = CTasGame::Map();
	gs_vBend.assign(M.m_W * M.m_H, 0.0f);
	for(int y = 0; y < M.m_H; y++)
		for(int x = 0; x < M.m_W; x++)
		{
			vec2 P(x * 32 + 16, y * 32 + 16);
			if(gs_Dist.Sample(P) > 1e8f)
				continue;
			vec2 G0 = -gs_Dist.Grad(P);
			if(length(G0) < 0.5f)
				continue;
			vec2 Q = P;
			float MaxAng = 0;
			for(float l = 0; l < gs_P.m_BendLen; l += 16.0f)
			{
				vec2 G = -gs_Dist.Grad(Q);
				if(length(G) < 0.5f)
					break;
				Q += G * 16.0f;
				float c = std::clamp(dot(G0, G), -1.0f, 1.0f);
				MaxAng = std::max(MaxAng, std::acos(c));
			}
			gs_vBend[y * M.m_W + x] = MaxAng;
		}
}

static float BendAt(vec2 P)
{
	const SMapInfo &M = CTasGame::Map();
	int x = std::clamp((int)(P.x / 32), 0, M.m_W - 1), y = std::clamp((int)(P.y / 32), 0, M.m_H - 1);
	return gs_vBend.empty() ? 0.0f : gs_vBend[y * M.m_W + x];
}

// best route-forward kick (px/tick) a point-blank grenade fired now could give, from the solid surfaces nearby
static float KickPotential(const CTasGame &G, vec2 Fwd)
{
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.Pos(), V = G.Vel();
	float Sp = length(V);
	float Ramp = Sp * 50 > 550 ? std::pow(1.4f, -(Sp * 50 - 550) / 2000.0f) : 1.0f;
	vec2 Q = P + V * Ramp; // where the tee is when the explosion is applied
	float Best = 0;
	for(int a = 0; a < 48; a++)
	{
		float Ang = 2 * pi * a / 48;
		vec2 D(std::cos(Ang), std::sin(Ang));
		vec2 S0 = P + D * 21.0f;
		vec2 E;
		bool Hit = false;
		for(float r = 0; r < 70.0f; r += 3.0f)
		{
			E = S0 + D * r;
			int t = M.Tile((int)std::floor(E.x / 32), (int)std::floor(E.y / 32));
			if(t == TILE_SOLID || t == TILE_NOHOOK)
			{
				Hit = true;
				break;
			}
		}
		if(!Hit)
			continue;
		vec2 Diff = Q - E;
		float l = length(Diff);
		if(l < 1e-3f)
			continue;
		float Str = 6.0f * (1.0f - std::clamp((l - 48.0f) / 87.0f, 0.0f, 1.0f));
		if(Str < 1.0f)
			continue;
		vec2 K = Diff / l * Str * 2.0f;
		Best = std::max(Best, dot(K, Fwd));
	}
	return Best;
}

// estimated total ticks (lower is better)
static float Score(const CTasGame &G)
{
	if(gs_P.m_Track && gs_Ref.Ghost())
		return TrackScore(G);
	if(gs_P.m_PrestartObj && (!G.m_Started || gs_P.m_SpeedObj))
	{
		// pre-start experiment: build speed towards the start line
		vec2 V = G.Vel();
		if(gs_P.m_PrestartEnergy)
		{
			int Jumps = G.Grounded() ? 2 : ((G.Jumped() & 2) ? 0 : 1);
			return -(dot(V, V) - gs_P.m_PreYW * G.Pos().y + gs_P.m_PrestartEnergy * V.x - gs_P.m_PreVyW * V.y + gs_P.m_JumpEnergy * Jumps) + gs_Dist.Sample(G.Pos()) / 1000.0f;
		}
		if(gs_P.m_SpeedObj == 2)
			return -length(V) + gs_Dist.Sample(G.Pos()) / 1000.0f; // max total speed
		return -(V.x * gs_P.m_PrestartObj) + gs_Dist.Sample(G.Pos()) / 1000.0f;
	}
	vec2 P = G.Pos();
	// before the pickup, head to the grenade first, then to the finish from there
	float D = DistTo(G, P);
	vec2 Gr = GradTo(G, P);
	if(D > 1e8f)
		return 1e9f;
	float Along = -dot(G.Vel(), Gr);
	if(gs_P.m_BendCredit > 0 && G.HasGrenade())
	{
		// a loaded grenade before a bend turns the tee without the hook's braking
		float Ready = 1.0f - std::min(G.ReloadTimer(), 25) / 25.0f;
		Along += gs_P.m_BendCredit * 12.0f * std::min(1.0f, BendAt(G.Pos()) / (pi / 2)) * Ready;
	}
	if(gs_P.m_PadOpt > 0 && G.NumProjectiles() > 0)
	{
		// optimistic: a pending explosion that the tee could still reach counts as a full kick along the route
		vec2 E;
		int T;
		if(G.NextExplosion(E, T) && T > G.m_Tick)
		{
			float Dt = (float)(T - G.m_Tick);
			float Reach = Dt * std::max(length(G.Vel()), 15.0f) * 0.85f + 48.0f;
			float DE = DistTo(G, E), DP = DistTo(G, G.Pos());
			if(distance(E, G.Pos()) <= Reach && DE < DP + 64.0f && DE > DP - Reach)
				Along += gs_P.m_PadOpt * 12.0f;
		}
	}
	if(gs_P.m_PadCredit > 0 && G.NumProjectiles() > 0)
	{
		// a grenade in flight is a boost pad at a known place and time: credit the kick the tee would get
		// there if it kept its current input
		vec2 E;
		int T;
		if(G.NextExplosion(E, T) && T > G.m_Tick && T - G.m_Tick <= 60)
		{
			vec2 aPos[64];
			int K = T - G.m_Tick;
			int n = G.Rollout(K, G.Chr()->m_Input.m_Direction, true, aPos);
			if(n == K)
			{
				float d = distance(aPos[K - 1], E);
				float Str = 12.0f * (1.0f - std::clamp((d - 48.0f) / 87.0f, 0.0f, 1.0f));
				Along += gs_P.m_PadCredit * Str;
			}
		}
	}
	if(gs_P.m_KCredit > 0 && G.HasGrenade() && G.ReloadTimer() <= gs_P.m_KReady)
		Along += gs_P.m_KCredit * KickPotential(G, -Gr) * (1.0f - (float)G.ReloadTimer() / (gs_P.m_KReady + 1));
	if(gs_P.m_GCredit > 0 && G.HasGrenade())
	{
		// a loaded grenade is future speed: credit it so a weak shot now doesn't beat a good one later
		float Ready = 1.0f - std::min(G.ReloadTimer(), 25) / 25.0f;
		Along += gs_P.m_GCredit * Ready;
	}
	// available jumps: ground jump + air jump, or just the air jump
	int Jumps = G.Grounded() ? 2 : ((G.Jumped() & 2) ? 0 : 1);
	float Remaining = (D - gs_P.m_Alpha * Along - gs_P.m_JumpBonus * Jumps - gs_P.m_HeightBonus * (-G.Vel().y + 0.0f)) / gs_P.m_Vref;
	if(gs_P.m_SpeedGamma > 0)
	{
		float V = std::clamp(Along, 8.0f, 34.0f);
		Remaining = D / (gs_P.m_Vref + gs_P.m_SpeedGamma * (V - gs_P.m_Vref)) - (gs_P.m_JumpBonus * Jumps) / gs_P.m_Vref;
	}
	if(gs_P.m_EKappa > 0)
	{
		// energy (speed^2/2 + height + available jumps) is future speed: hooks that steer at high speed burn it
		float E = 0.5f * dot(G.Vel(), G.Vel()) - 0.5f * G.Pos().y + 0.5f * gs_P.m_JumpEnergy * Jumps;
		Remaining -= gs_P.m_EKappa * E / gs_P.m_Vref;
	}
	if(gs_P.m_DynVref > 0)
	{
		// time at the current speed along the route, blended with the reference speed over a horizon
		float V = std::clamp(Along, gs_P.m_DynVmin, gs_P.m_DynVmax);
		float H = std::min(D, gs_P.m_DynVref); // the current speed only lasts so long
		Remaining = H / V + (D - H) / gs_P.m_Vref;
	}
	if(gs_P.m_HookPen > 0)
		Remaining += gs_P.m_HookPen * G.m_HookLoss / gs_P.m_Vref;
	if(gs_P.m_CrashPen > 0)
		Remaining += gs_P.m_CrashPen * G.CrashLoss(gs_P.m_CrashK) / gs_P.m_Vref;
	float Elapsed = G.m_Started ? (float)(G.m_Tick - G.m_StartTick) : 0.0f;
	return Elapsed + Remaining;
}

// hook directions whose ray reaches a solid tile (one direction per tile hit)
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
		for(int i = 0; i < NumSeen && gs_P.m_HookDedup; i++)
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

// fire directions whose ray hits a solid tile close enough for the explosion to matter
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
		for(float r = 8.0f; r < Range; r += 6.0f)
		{
			vec2 Q = P + D * r;
			if(M.Tile((int)std::floor(Q.x / 32), (int)std::floor(Q.y / 32)) == TILE_SOLID)
			{
				Hit = true;
				break;
			}
		}
		if(!Hit)
			continue;
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

// can the tee avoid freeze for N ticks with some simple continuation?
static bool Survives(const CTasGame &G, int N)
{
	if(G.Frozen() || G.EnteredFreeze())
		return false;
	vec2 aPos[128];
	N = std::min(N, 128);
	vec2 Gr = GradTo(G, G.Pos());
	int Toward = Gr.x < -0.2f ? 1 : Gr.x > 0.2f ? -1 : 0;
	const int aDirs[3] = {Toward, 0, -Toward};
	for(int d : aDirs)
	{
		if(G.Rollout(N, d, true, aPos) == N)
			return true;
		if(G.m_LastHook && G.Rollout(N, d, false, aPos) == N)
			return true;
		if(d == 0 && Toward == 0)
			break;
	}
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vHooks;
	HookTargets(G, 48, s_vHooks);
	for(auto [TX, TY] : s_vHooks)
		for(int d : aDirs)
		{
			if(G.RolloutHook(N, d, TX, TY, aPos) == N)
				return true;
			if(d == 0 && Toward == 0)
				break;
		}
	return false;
}

// coarse cell for the visited set: position, velocity, hook state, jumps, grenade reload
static int64_t VisitKey(const CTasGame &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	int64_t cx = (int64_t)std::floor(P.x / gs_P.m_VCell), cy = (int64_t)std::floor(P.y / gs_P.m_VCell);
	int64_t vx = (int64_t)std::floor(V.x / gs_P.m_VVel) + 64, vy = (int64_t)std::floor(V.y / gs_P.m_VVel) + 64;
	int hs = G.HookState();
	int64_t h = hs == 0 || hs == -1 ? 0 : (hs == 5 ? 2 : 1);
	int64_t j = G.Grounded() ? 2 : ((G.Jumped() & 2) ? 0 : 1);
	int64_t g = G.HasGrenade() ? 1 + (G.ReloadTimer() > 0 ? 1 : 0) + (G.NumProjectiles() > 0 ? 2 : 0) : 0;
	int64_t k = cx;
	k = k * 1024 + (cy & 1023);
	k = k * 128 + (vx & 127);
	k = k * 128 + (vy & 127);
	k = k * 3 + h;
	k = k * 3 + j;
	k = k * 5 + g;
	return k;
}

struct SCand
{
	float m_Score;
	float m_Speed; // speed along the route (px/tick)
	float m_Energy; // |v|^2 + 2 g * (height above the tee's position is lower) -> see below
	float m_Track; // distance to the ghost at the same race time (ghost share)
	int m_Parent;
	STasInput m_In;
	uint64_t m_Hash;
	int32_t m_Key;
	int64_t m_VKey;
	int m_Pending = 0; // ticks until a pending explosion beyond the fire lookahead (0 = none)
	float m_PendFit = 0; // how well the pending explosion matches the predicted future position along the route (0..1)
};

static int32_t CellKey(const CTasGame &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	int cx = (int)std::floor(P.x / gs_P.m_CellSize), cy = (int)std::floor(P.y / gs_P.m_CellSize);
	int vx = (int)std::floor(V.x / gs_P.m_VelCell), vy = (int)std::floor(V.y / gs_P.m_VelCell);
	int h = G.HookState() > 0 ? 1 : 0;
	if(gs_P.m_RichKey)
	{
		// hook idle / flying / attached, and the jumps still available
		int hs = G.HookState();
		h = hs == 0 || hs == -1 ? 0 : (hs == 5 ? 2 : 1) ;
		int Jumps = G.Grounded() ? 2 : ((G.Jumped() & 2) ? 0 : 1);
		h = h * 3 + Jumps;
	}
	uint32_t k = (uint32_t)cx * 73856093u ^ (uint32_t)cy * 19349663u ^ (uint32_t)(vx + 64) * 83492791u ^ (uint32_t)(vy + 64) * 2654435761u ^ (uint32_t)h * 97u;
	if(gs_P.m_ProjKey && G.NumProjectiles() > 0)
	{
		// a pending explosion is a boost waiting at a known place and time: keep such states apart
		vec2 E;
		int T;
		if(G.NextExplosion(E, T))
			k ^= ((uint32_t)(E.x / 48) * 2246822519u) ^ ((uint32_t)(E.y / 48) * 3266489917u) ^ ((uint32_t)((T - G.m_Tick) / 3 + 1) * 668265263u);
	}
	return (int32_t)k;
}

// pad aims: shots whose explosion lands near the route ahead 8..45 ticks later, refined to surface edges
// (narrow aim windows that hit e.g. the side face of a block end); returns (TX, TY) pairs
static void PadAims(const CTasGame &G, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	static thread_local CTasGame s_T;
	const int N = gs_P.m_PadAims;
	struct SShot
	{
		bool m_Ok;
		vec2 m_E;
		int m_T;
	};
	auto Shoot = [&](float Ang) {
		SShot R{false, vec2(0, 0), 0};
		s_T.CopyFrom(G);
		STasInput In;
		In.m_Dir = 0;
		In.m_Hook = G.m_LastHook;
		In.m_Jump = G.m_LastJump;
		In.m_Fire = 1;
		In.m_Weapon = 3;
		In.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
		In.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
		if(!In.m_TX && !In.m_TY)
			In.m_TY = -1;
		s_T.Step(In);
		R.m_Ok = s_T.NextExplosion(R.m_E, R.m_T);
		R.m_T -= G.m_Tick;
		return R;
	};
	const float DP = DistTo(G, G.Pos());
	auto Useful = [&](const SShot &S) {
		if(!S.m_Ok || S.m_T < 8 || S.m_T > 45)
			return false;
		float DE = DistTo(G, S.m_E);
		return DE < DP + 64.0f && DE > DP - 45.0f * 45.0f;
	};
	auto Emit = [&](float Ang) {
		int16_t TX = (int16_t)std::lround(std::cos(Ang) * 1000), TY = (int16_t)std::lround(std::sin(Ang) * 1000);
		if(!TX && !TY)
			TY = -1;
		for(auto &p : vOut)
			if(p.first == TX && p.second == TY)
				return;
		vOut.push_back({TX, TY});
	};
	std::vector<SShot> vS(N);
	for(int a = 0; a < N; a++)
		vS[a] = Shoot(2 * pi * a / N);
	for(int a = 0; a < N; a++)
	{
		const SShot &A = vS[a], &B = vS[(a + 1) % N];
		bool Jump = A.m_Ok != B.m_Ok || distance(A.m_E, B.m_E) > 48.0f || std::abs(A.m_T - B.m_T) > 3;
		if(!Jump)
			continue;
		if(!Useful(A) && !Useful(B))
			continue;
		float Lo = 2 * pi * a / N, Hi = 2 * pi * (a + 1) / N;
		SShot SL = A;
		for(int k = 0; k < 7; k++)
		{
			float Mid = 0.5f * (Lo + Hi);
			SShot M = Shoot(Mid);
			bool SameAsLo = M.m_Ok == SL.m_Ok && distance(M.m_E, SL.m_E) <= 48.0f && std::abs(M.m_T - SL.m_T) <= 3;
			if(SameAsLo)
			{
				Lo = Mid;
				SL = M;
			}
			else
				Hi = Mid;
		}
		SShot SH = Shoot(Hi);
		if(Useful(SL))
			Emit(Lo);
		if(Useful(SH))
			Emit(Hi);
	}
}

static void GenActions(const CTasGame &G, const STasInput &Prev, std::vector<STasInput> &vOut)
{
	vOut.clear();
	gs_vActCost.clear();
	struct SSync { std::vector<STasInput> &v; ~SSync() { gs_vActCost.resize(v.size(), 0.0f); } } Sync{vOut};
	if(G.m_CommitLeft > 0)
	{
		vOut.push_back(G.m_CommitIn);
		if(gs_P.m_CommitFire && G.HasGrenade() && gs_P.m_UseGrenade && G.ActiveWeapon() == 3 && G.ReloadTimer() == 0)
		{
			// shoot while the committed hook keeps pulling (the hook's direction is fixed at launch)
			static thread_local std::vector<std::pair<int16_t, int16_t>> s_vCF;
			FireTargets(G, gs_P.m_FireAngles, gs_P.m_FireRange, s_vCF);
			for(auto [TX, TY] : s_vCF)
			{
				STasInput F = G.m_CommitIn;
				if(G.HookState() == 0 || G.HookState() == -1)
					continue; // the hook hasn't launched yet: changing the aim would redirect it
				F.m_Fire = 1;
				F.m_TX = TX;
				F.m_TY = TY;
				vOut.push_back(F);
			}
		}
		return;
	}
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vHooks;
	static thread_local std::vector<float> s_vHookCost;
	if(!G.m_LastHook)
	{
		HookTargets(G, gs_P.m_Angles, s_vHooks);
		s_vHookCost.assign(s_vHooks.size(), 0.0f);
		if(gs_HasPrior)
		{
			std::vector<std::pair<float, int>> vOrd;
			for(int k = 0; k < (int)s_vHooks.size(); k++)
			{
				float p = PriorOf(gs_aHookPrior, vec2(s_vHooks[k].first, s_vHooks[k].second), G.Vel());
				vOrd.push_back({-std::log(p + 1e-4f), k});
			}
			std::sort(vOrd.begin(), vOrd.end());
			std::vector<std::pair<int16_t, int16_t>> vKeep;
			std::vector<float> vCost;
			for(int k = 0; k < (int)vOrd.size() && (gs_P.m_PriorTop <= 0 || k < gs_P.m_PriorTop); k++)
			{
				vKeep.push_back(s_vHooks[vOrd[k].second]);
				vCost.push_back(vOrd[k].first);
			}
			s_vHooks = vKeep;
			s_vHookCost = vCost;
		}
	}
	// a jump press does nothing without an available jump (then it only matters whether it's held)
	const bool CanJump = G.Grounded() || !(G.Jumped() & 2);
	const int Weapon = G.HasGrenade() ? 3 : -1; // WEAPON_GRENADE
	const bool CanNewHook = !G.m_LastHook;
	const bool Hooking = G.m_LastHook;
	const bool CanFire = G.HasGrenade() && gs_P.m_UseGrenade && G.ActiveWeapon() == 3 && G.ReloadTimer() == 0 && (!G.m_Started || G.m_Tick - G.m_StartTick >= gs_P.m_NoFireBefore);
	for(int Dir = -1; Dir <= 1; Dir++)
	{
		for(int Jump = 0; Jump <= 1; Jump++)
		{
			if(Jump && !CanJump && !G.m_LastJump)
				continue;
			if(Jump && !G.m_LastJump && !G.Grounded() && G.Vel().y < gs_P.m_JumpVyMin)
				continue;
			// jump only matters on a new press; keep it held otherwise to avoid duplicates
			STasInput In;
			In.m_Dir = Dir;
			In.m_Jump = Jump;
			In.m_Weapon = Weapon;
			In.m_TX = Prev.m_TX;
			In.m_TY = Prev.m_TY;
			// keep the current hook state
			In.m_Hook = Hooking;
			vOut.push_back(In);
			if(Hooking)
			{
				In.m_Hook = 0;
				vOut.push_back(In);
			}
			if(CanNewHook)
			{
				for(int hk = 0; hk < (int)s_vHooks.size(); hk++)
				{
					auto [TX, TY] = s_vHooks[hk];
					STasInput H = In;
					H.m_Hook = 1;
					H.m_TX = TX;
					H.m_TY = TY;
					gs_vActCost.resize(vOut.size(), 0.0f);
					if(!gs_P.m_MacroOnly)
					{
						vOut.push_back(H);
						gs_vActCost.resize(vOut.size(), 0.0f);
						gs_vActCost.back() = s_vHookCost[hk];
					}
					// macro moves: hold this hook (and direction) for a fixed time, judged at its end
					for(int N : gs_P.m_vMacro)
					{
						STasInput Mc = H;
						Mc.m_Commit = (uint8_t)N;
						vOut.push_back(Mc);
						gs_vActCost.resize(vOut.size(), 0.0f);
						gs_vActCost.back() = s_vHookCost[hk];
					}
				}
			}
			if(CanFire && (Jump == 0 || !gs_P.m_FireLookahead))
			{
				if(gs_P.m_FireLookahead)
				{
					static thread_local std::vector<std::pair<int16_t, int16_t>> s_vFire;
					if(Dir == -1)
					{
						FireTargets(G, gs_P.m_FireAngles, gs_P.m_FireRange, s_vFire);
						if(gs_P.m_PadAims > 0)
						{
							static thread_local std::vector<std::pair<int16_t, int16_t>> s_vPad;
							PadAims(G, s_vPad);
							s_vFire.insert(s_vFire.end(), s_vPad.begin(), s_vPad.end());
						}
						if(gs_HasPrior && gs_P.m_PriorShots > 0)
						{
							std::vector<std::pair<float, std::pair<int16_t, int16_t>>> vOrd;
							for(auto &t : s_vFire)
								vOrd.push_back({-std::log(PriorOf(gs_aShotPrior, vec2(t.first, t.second), G.Vel()) + 1e-4f), t});
							std::sort(vOrd.begin(), vOrd.end());
							s_vFire.clear();
							for(int k = 0; k < (int)vOrd.size() && k < gs_P.m_PriorShots; k++)
								s_vFire.push_back(vOrd[k].second);
						}
					}
					for(auto [TX, TY] : s_vFire)
					{
						STasInput F = In;
						F.m_Fire = 1;
						F.m_TX = TX;
						F.m_TY = TY;
						vOut.push_back(F);
						gs_vActCost.resize(vOut.size(), 0.0f);
						if(gs_HasPrior)
							gs_vActCost.back() = -std::log(PriorOf(gs_aShotPrior, vec2(TX, TY), G.Vel()) + 1e-4f);
					}
				}
				else
					for(int a = 0; a < gs_P.m_Angles; a++)
					{
						float Ang = 2 * pi * a / gs_P.m_Angles;
						STasInput F = In;
						F.m_Fire = 1;
						F.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
						F.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
						if(!F.m_TX && !F.m_TY)
							F.m_TY = -1;
						vOut.push_back(F);
					}
			}
		}
	}
}

static std::vector<STasInput> ExpandRepeat(const std::vector<STasInput> &v)
{
	std::vector<STasInput> Out;
	for(const auto &In : v)
		for(int r = 0; r < gs_P.m_Repeat; r++)
			Out.push_back(In);
	return Out;
}

struct SBeamState
{
	std::unique_ptr<CTasGame> m_pGame;
	STasInput m_Prev;
	int m_Clan = 0; // lineage: index of the ancestor at the last clan reset
};

static std::vector<STasInput> Search(std::vector<STasInput> vPrefix)
{
	auto t0 = std::chrono::steady_clock::now();
	std::vector<SBeamState> vBeam;
	{
		SBeamState S;
		S.m_pGame = std::make_unique<CTasGame>();
		S.m_pGame->Spawn(CTasGame::Map().m_vSpawns[0]);
		auto Teleport = [&]() {

			// experiment: start exactly at a given state just before the start line
			S.m_pGame->SetState(vec2(gs_P.m_TpX, gs_P.m_TpY), vec2(gs_P.m_TpVx, gs_P.m_TpVy));
			if(gs_P.m_TpJumped >= 0)
				S.m_pGame->Chr()->m_Core.m_Jumped = gs_P.m_TpJumped;
			if(gs_P.m_TpGren)
			{
				S.m_pGame->Chr()->GiveWeapon(WEAPON_GRENADE);
				S.m_pGame->Chr()->m_Core.m_ActiveWeapon = WEAPON_GRENADE;
			}

			S.m_pGame->m_Tick = 100;
			if(gs_P.m_TpStarted)
			{
				S.m_pGame->m_Started = true;
				S.m_pGame->m_StartTick = 100;
			}
		};
		if(gs_P.m_Teleport && gs_P.m_TpFirst)
			Teleport();
		for(const auto &In : vPrefix)
		{
			S.m_pGame->Step(In);
			UpdateRef(*S.m_pGame);
		}
		if(gs_P.m_PrefixJumped >= 0)
			S.m_pGame->Chr()->m_Core.m_Jumped = gs_P.m_PrefixJumped;
		if(gs_P.m_Teleport && !gs_P.m_TpFirst)
			Teleport();
		if(gs_Ref.Ghost())
		{
			// start on the ghost's line at our race time
			const CTasGame &G0 = *S.m_pGame;
			int RaceTick = G0.m_Started ? G0.m_Tick - G0.m_StartTick : 0;
			int T0 = (int)gs_Ref.m_vTime[0];
			S.m_pGame->m_RefIdx = std::clamp(RaceTick + gs_P.m_GhostShift - T0, 0, (int)gs_Ref.m_vPos.size() - 1);
			S.m_pGame->m_RefIdx = gs_Ref.Track(S.m_pGame->m_RefIdx, G0.Pos());
		}
		if(!vPrefix.empty())
			S.m_Prev = vPrefix.back();
		vBeam.push_back(std::move(S));
	}
	const int PrefixTicks = vPrefix.size();
	// history per tick: parent index and input of each beam slot
	std::vector<std::vector<std::pair<int, STasInput>>> vHist;

	int BestFinish = 1 << 30;
	gs_StopTicks = 1 << 30;
	std::unordered_set<int64_t> Visited; // coarse cells already reached at an earlier tick (visited mode)
	float GateBestEst = 1e30f;
	int GateDeadline = -1;
	std::vector<STasInput> vGateBest;
	std::vector<STasInput> vBest;
	const int NT = std::max(1, gs_P.m_Threads);

	for(int Step = 0; Step < gs_P.m_MaxTicks && !vBeam.empty(); Step++)
	{
		std::vector<std::vector<SCand>> vThreadCands(NT);
		std::vector<std::pair<int, std::vector<STasInput>>> vFinished(NT, {1 << 30, {}});
		std::atomic<int> Next{0};
		std::atomic<int> GrenTick{1 << 30};
		std::vector<std::pair<float, std::vector<STasInput>>> vGate(NT, {1e30f, {}});
		std::vector<std::vector<SCrossCand>> vCross(NT);
		auto Worker = [&](int T) {
			CTasGame Tmp, Look;
			std::vector<STasInput> vActs;
			auto &vC = vThreadCands[T];
			while(true)
			{
				int i = Next.fetch_add(1);
				if(i >= (int)vBeam.size())
					break;
				const CTasGame &G = *vBeam[i].m_pGame;
				GenActions(G, vBeam[i].m_Prev, vActs);
				static thread_local std::vector<float> vActCost;
				vActCost = gs_vActCost;
				for(size_t ai = 0; ai < vActs.size(); ai++)
				{
					const STasInput &In = vActs[ai];
					const float ActCost = ai < vActCost.size() ? vActCost[ai] : 0.0f;
					Tmp.CopyFrom(G);
					bool BadSub = false;
					for(int r = 0; r < gs_P.m_Repeat; r++)
					{
						StepIn(Tmp, In);
						if(Tmp.Frozen() || Tmp.m_StartTick == -2 || Tmp.EnteredFreeze() || Tmp.m_FinishTick >= 0)
						{
							BadSub = Tmp.m_FinishTick < 0;
							break;
						}
					}
					if(BadSub || Tmp.Frozen() || Tmp.m_StartTick == -2 || Tmp.EnteredFreeze())
					{
						if(getenv("TAS_DBG") && Step == 0)
							std::printf("reject frozen=%d start=%d pos %.0f %.0f\n", Tmp.Frozen(), Tmp.m_StartTick, Tmp.Pos().x, Tmp.Pos().y);
						continue;
					}
					if(!Tmp.m_Started && Tmp.m_Tick > PrefixTicks + gs_P.m_PreStartMax && PrefixTicks == 0)
						continue;
					if(gs_P.m_PrestartObj && Tmp.m_Started && !gs_P.m_SpeedObj)
					{
						// keep the best crossing (by horizontal speed) as a prefix for later searches
						{
							auto &GB = vGate[T];
							float E = -Tmp.Vel().x + 0.02f * (float)Tmp.m_Tick;
							int Alive = -1;
							if(E < GB.first && (Alive = Survives(Tmp, 25)))
							{
								GB.first = E;
								GB.second = {In};
								GB.second.push_back(STasInput{(int8_t)0, 0, 0, 0, (int16_t)(i & 0x7fff), (int16_t)(i >> 15), 0});
							}
							if(gs_P.m_PreCands > 0 && (Alive == 1 || (Alive < 0 && Survives(Tmp, 25))))
							{
								vec2 V = Tmp.Vel(), P = Tmp.Pos();
								if(P.y < gs_P.m_PreYMin || V.y > gs_P.m_PreVyMax)
									goto skipcand;
								int Jumps = Tmp.Grounded() ? 2 : ((Tmp.Jumped() & 2) ? 0 : 1);
								float En = dot(V, V) - gs_P.m_PreYW * P.y + gs_P.m_JumpEnergy * Jumps;
								int64_t Key = (int64_t)std::floor(P.y / 32);
								Key = Key * 64 + ((int)std::floor(V.y / 4) + 32);
								Key = Key * 64 + ((int)std::floor(V.x / 4) + 32);
								Key = Key * 3 + Jumps;
								vCross[T].push_back({En, Key, i, In, P, V, Jumps, Tmp.m_Tick});
							}
						skipcand:;
						}
						float Vx = Tmp.Vel().x;
						static std::atomic<int> s_BestVx{0};
						int Q = (int)(Vx * 100);
						int Old = s_BestVx.load();
						while(Q > Old && !s_BestVx.compare_exchange_weak(Old, Q))
							;
						if(Q > Old)
							std::printf("cross tick %d vel %.2f %.2f |v| %.2f pos %.0f %.0f\n", Tmp.m_Tick, Tmp.Vel().x, Tmp.Vel().y, length(Tmp.Vel()), Tmp.Pos().x, Tmp.Pos().y);
						continue;
					}
					if(gs_P.m_StopD > 0 && Tmp.m_Started && GeoDistTo(Tmp, Tmp.Pos()) < gs_P.m_StopD && (!gs_P.m_UseGrenade || Tmp.HasGrenade() || gs_Dist.Sample(gs_GrenadePos) < gs_P.m_StopD + 1.0f || true))
					{
						float E = Score(Tmp);
						auto &GB = vGate[T];
						if(E < GB.first && Survives(Tmp, std::max(gs_P.m_Survive, 25)))
						{
							GB.first = E;
							GB.second = {In};
							GB.second.push_back(STasInput{(int8_t)0, 0, 0, 0, (int16_t)(i & 0x7fff), (int16_t)(i >> 15), 0});
						}
						if(gs_P.m_GateCands > 0)
						{
							vec2 V = Tmp.Vel(), P = Tmp.Pos();
							int Jumps = Tmp.Grounded() ? 2 : ((Tmp.Jumped() & 2) ? 0 : 1);
							float Along = -dot(V, GradTo(Tmp, P));
							int RaceTick = Tmp.m_Tick - Tmp.m_StartTick;
							float Val = -(float)RaceTick + gs_P.m_GateLambda * Along + 2.0f * Jumps - (Tmp.ReloadTimer() > 0 ? 0.1f * Tmp.ReloadTimer() : 0.0f);
							int64_t Key = (int64_t)std::floor(P.x / 32) * 4096 + (int64_t)std::floor(P.y / 32);
							Key = Key * 64 + ((int)std::floor(V.x / 4) + 32);
							Key = Key * 64 + ((int)std::floor(V.y / 4) + 32);
							Key = Key * 3 + Jumps;
							bool Want = false;
							{
								std::lock_guard<std::mutex> L(gs_CrossMx);
								auto it = gs_Cross.find(Key);
								Want = it == gs_Cross.end() || Val > it->second.m_E;
							}
							if(Want && Survives(Tmp, 20))
								vCross[T].push_back({Val, Key, i, In, P, V, Jumps, RaceTick});
						}
						// keep it in the beam as well
					}
					const bool InBox = gs_P.m_StopBoxOn && Tmp.Grounded() && Tmp.Pos().x >= gs_P.m_StopBox[0] && Tmp.Pos().y >= gs_P.m_StopBox[1] && Tmp.Pos().x <= gs_P.m_StopBox[2] && Tmp.Pos().y <= gs_P.m_StopBox[3];
					const bool PastXLt = (gs_P.m_StopXLt > 0 && Tmp.Pos().x < gs_P.m_StopXLt) || (gs_P.m_StopX > 0 && Tmp.Pos().x > gs_P.m_StopX);
					const bool InRegion = gs_P.m_StopRegionOn && Tmp.Pos().x >= gs_P.m_StopRegion[0] && Tmp.Pos().y >= gs_P.m_StopRegion[1] && Tmp.Pos().x <= gs_P.m_StopRegion[2] && Tmp.Pos().y <= gs_P.m_StopRegion[3];
					if(((gs_P.m_StopAtGrenade && Tmp.HasGrenade()) || InBox || PastXLt || InRegion) && Tmp.m_Started)
					{
						GrenTick.store(std::min(GrenTick.load(), Tmp.m_Tick - Tmp.m_StartTick));
						// save the earliest pickup (then the best by score) as a segment result
						auto &GB = vGate[T];
						float E = (float)(Tmp.m_Tick - Tmp.m_StartTick) * 1000.0f + Score(Tmp);
						if(gs_P.m_GateSpeed)
							E = -length(Tmp.Vel()) * 100.0f + (float)(Tmp.m_Tick - Tmp.m_StartTick);
						if(E < GB.first && Survives(Tmp, 20))
						{
							GB.first = E;
							GB.second = {In};
							GB.second.push_back(STasInput{(int8_t)0, 0, 0, 0, (int16_t)(i & 0x7fff), (int16_t)(i >> 15), 0});
						}
						continue;
					}
					if(Tmp.m_FinishTick >= 0)
					{
						int T2 = Tmp.m_FinishTick - Tmp.m_StartTick;
						if(T2 < vFinished[T].first)
						{
							vFinished[T].first = T2;
							// reconstruct later from parent index
							vFinished[T].second = {In};
							vFinished[T].second.push_back(STasInput{(int8_t)0, 0, 0, 0, (int16_t)(i & 0x7fff), (int16_t)(i >> 15), 0});
						}
						continue;
					}
					const CTasGame *pEval = &Tmp;
					if(Tmp.m_CommitLeft > 0)
					{
						// judge a macro move by where it ends
						Look.CopyFrom(Tmp);
						bool Dead = false;
						while(Look.m_CommitLeft > 0)
						{
							StepIn(Look, Look.m_CommitIn);
							if(Look.Frozen() || Look.EnteredFreeze())
							{
								Dead = true;
								break;
							}
						}
						if(Dead)
							continue;
						pEval = &Look;
					}
					else if(gs_P.m_FireLookahead && Tmp.NumProjectiles() > 0)
					{
						// judge the state after the grenade has exploded
						Look.CopyFrom(Tmp);
						STasInput L = In;
						L.m_Fire = 0;
						bool Dead = false;
						for(int k = 0; k < gs_P.m_FireLookMax && Look.NumProjectiles() > 0; k++)
						{
							Look.Step(L);
							UpdateRef(Look);
							if(Look.Frozen())
							{
								Dead = true;
								break;
							}
						}
						if(Dead)
							continue;
						pEval = &Look;
					}
					float S = Score(*pEval);
					if(S > 1e8f)
						continue;
					S += gs_P.m_PriorKappa * ActCost;
					{
						const CTasGame &Tmp = *pEval;
						vec2 Gr = GradTo(Tmp, Tmp.Pos());
						float Sp = -dot(Tmp.Vel(), Gr);
						// kinetic + potential energy relative to the ceiling-free fall: v^2 - 2 g y (y grows downwards)
						int Jumps = Tmp.Grounded() ? 2 : ((Tmp.Jumped() & 2) ? 0 : 1);
						float En = dot(Tmp.Vel(), Tmp.Vel()) - 2.0f * 0.5f * Tmp.Pos().y + gs_P.m_JumpEnergy * Jumps;
						float Tr = gs_P.m_GhostShare > 0 && gs_Ref.Ghost() ? TrackScore(Tmp) : 0.0f;
						vC.push_back({S, Sp, En, Tr, i, In, Tmp.Hash(), CellKey(Tmp), gs_P.m_Visited ? VisitKey(Tmp) : 0});
						if(gs_P.m_PendShare > 0 && Tmp.NumProjectiles() > 0)
						{
							vec2 E;
							int T;
							if(Tmp.NextExplosion(E, T))
							{
								vC.back().m_Pending = T - Tmp.m_Tick;
								// where along the route will the tee be at the explosion, at its current speed?
								float DNow = DistTo(Tmp, Tmp.Pos()), DE = DistTo(Tmp, E);
								float DPred = DNow - std::max(Sp, 5.0f) * (T - Tmp.m_Tick) * 0.85f;
								vC.back().m_PendFit = (DE < 1e8f) ? std::max(0.0f, 1.0f - std::fabs(DE - DPred) / gs_P.m_PendWin) : 0.0f;
								if(gs_P.m_PendRoll && DE < DNow + 64.0f && T - Tmp.m_Tick <= 60)
								{
									// predicted miss distance at the explosion if the current input is held
									vec2 aP[64];
									int K = T - Tmp.m_Tick;
									int n = Tmp.Rollout(K, In.m_Dir, true, aP);
									float Miss = n == K ? distance(aP[K - 1], E) : 1e9f;
									vC.back().m_PendFit = std::max(0.0f, 1.0f - std::max(0.0f, Miss - 40.0f) / gs_P.m_PendWin);
								}
								else if(gs_P.m_PendRoll)
									vC.back().m_PendFit = 0;
							}
						}
					}
				}
			}
		};
		std::vector<std::thread> vThreads;
		for(int T = 1; T < NT; T++)
			vThreads.emplace_back(Worker, T);
		Worker(0);
		for(auto &Th : vThreads)
			Th.join();

		for(auto &vC : vCross)
			for(const auto &C : vC)
			{
				auto &Slot = gs_Cross[C.m_Key];
				if(C.m_E <= Slot.m_E)
					continue;
				std::vector<STasInput> vRun;
				vRun.push_back(C.m_In);
				int Idx = C.m_Parent;
				for(int s2 = (int)vHist.size() - 1; s2 >= 0; s2--)
				{
					vRun.push_back(vHist[s2][Idx].second);
					Idx = vHist[s2][Idx].first;
				}
				std::reverse(vRun.begin(), vRun.end());
				vRun = ExpandRepeat(vRun);
				Slot.m_E = C.m_E;
				Slot.m_Pos = C.m_Pos;
				Slot.m_Vel = C.m_Vel;
				Slot.m_Jumps = C.m_Jumps;
				Slot.m_Tick = C.m_Tick;
				Slot.m_vIn = vPrefix;
				Slot.m_vIn.insert(Slot.m_vIn.end(), vRun.begin(), vRun.end());
			}
		for(auto &GB : vGate)
		{
			if(GB.first < GateBestEst)
			{
				GateBestEst = GB.first;
				int Parent = (GB.second[1].m_TX & 0x7fff) | (GB.second[1].m_TY << 15);
				std::vector<STasInput> vRun;
				vRun.push_back(GB.second[0]);
				int Idx = Parent;
				for(int s2 = (int)vHist.size() - 1; s2 >= 0; s2--)
				{
					vRun.push_back(vHist[s2][Idx].second);
					Idx = vHist[s2][Idx].first;
				}
				std::reverse(vRun.begin(), vRun.end());
				vRun = ExpandRepeat(vRun);
				vGateBest = vPrefix;
				vGateBest.insert(vGateBest.end(), vRun.begin(), vRun.end());
				if(GateDeadline < 0)
					GateDeadline = Step + gs_P.m_GateSlack;
			}
		}
		if(gs_P.m_PrestartObj && !gs_P.m_SpeedObj)
			GateDeadline = -1;
		if(GateDeadline >= 0 && Step >= GateDeadline)
		{
			WriteInputs(gs_P.m_Out.c_str(), vGateBest);
			std::printf("GATE est %.2f len %zu\n", GateBestEst, vGateBest.size());
			if(gs_P.m_GateCands > 0)
			{
				std::vector<const SCrossBest *> vAll;
				for(const auto &[k, v] : gs_Cross)
					vAll.push_back(&v);
				std::sort(vAll.begin(), vAll.end(), [](const SCrossBest *a, const SCrossBest *b) { return a->m_E > b->m_E; });
				std::printf("GATECANDS %zu distinct crossings\n", vAll.size());
				for(int k = 0; k < (int)vAll.size() && k < gs_P.m_GateCands; k++)
				{
					char aBuf[512];
					std::snprintf(aBuf, sizeof(aBuf), "%s.g%d.txt", gs_P.m_Out.c_str(), k);
					WriteInputs(aBuf, vAll[k]->m_vIn);
					std::printf("GATECAND %d val %.1f tick %d pos %.1f %.1f vel %.2f %.2f |v| %.2f jumps %d -> %s\n", k, vAll[k]->m_E, vAll[k]->m_Tick, vAll[k]->m_Pos.x / 32, vAll[k]->m_Pos.y / 32,
						vAll[k]->m_Vel.x, vAll[k]->m_Vel.y, length(vAll[k]->m_Vel), vAll[k]->m_Jumps, aBuf);
				}
			}
			return vGateBest;
		}
		if(GrenTick.load() < (1 << 30))
		{
			gs_StopTicks = GrenTick.load();
			std::printf("GRENADE at %d ticks after start\n", GrenTick.load());
			if((gs_P.m_StopAtGrenade || gs_P.m_StopBoxOn || gs_P.m_StopXLt > 0 || gs_P.m_StopX > 0 || gs_P.m_StopRegionOn) && GateDeadline >= 0)
			{
				WriteInputs(gs_P.m_Out.c_str(), vGateBest);
				std::printf("GRENADE path written (%zu inputs, est %.0f)\n", vGateBest.size(), GateBestEst);
				{
					CTasGame Chk;
					Chk.Spawn(CTasGame::Map().m_vSpawns[0]);
					for(const auto &In : vGateBest)
						Chk.Step(In);
					std::printf("GATE STATE tick %d pos %.1f %.1f vel %.2f %.2f |v| %.2f\n", Chk.m_Tick - Chk.m_StartTick, Chk.Pos().x / 32, Chk.Pos().y / 32, Chk.Vel().x, Chk.Vel().y, length(Chk.Vel()));
				}
			}
			break;
		}
		// finished runs
		for(auto &F : vFinished)
		{
			if(F.first < BestFinish)
			{
				BestFinish = F.first;
				int Parent = (F.second[1].m_TX & 0x7fff) | (F.second[1].m_TY << 15);
				std::vector<STasInput> vRun;
				vRun.push_back(F.second[0]);
				int Idx = Parent;
				for(int s = (int)vHist.size() - 1; s >= 0; s--)
				{
					vRun.push_back(vHist[s][Idx].second);
					Idx = vHist[s][Idx].first;
				}
				std::reverse(vRun.begin(), vRun.end());
				vRun = ExpandRepeat(vRun);
				vBest = vPrefix;
				vBest.insert(vBest.end(), vRun.begin(), vRun.end());
				WriteInputs(gs_P.m_Out.c_str(), vBest);
				std::printf("FINISH %d ticks (%.2f s) at step %d, wrote %s\n", BestFinish, BestFinish / 50.0, Step, gs_P.m_Out.c_str());
				std::fflush(stdout);
			}
		}

		std::vector<SCand> vAll;
		for(auto &v : vThreadCands)
			vAll.insert(vAll.end(), v.begin(), v.end());
		std::sort(vAll.begin(), vAll.end(), [](const SCand &a, const SCand &b) { return a.m_Score < b.m_Score; });
		if(gs_P.m_OptVal > 0)
		{
			// option value of a loaded grenade: a ready state is worth its best immediate shot
			std::unordered_set<uint64_t> Seen0;
			std::vector<int> vIdx;
			for(int k = 0; k < (int)vAll.size() && (int)vIdx.size() < gs_P.m_OptVal * gs_P.m_Beam; k++)
				if(Seen0.insert(vAll[k].m_Hash).second)
					vIdx.push_back(k);
			std::atomic<int> NextO{0};
			auto W = [&]() {
				CTasGame Tmp, F;
				std::vector<std::pair<int16_t, int16_t>> vAims;
				while(true)
				{
					int q = NextO.fetch_add(1);
					if(q >= (int)vIdx.size())
						break;
					SCand &C = vAll[vIdx[q]];
					Tmp.CopyFrom(*vBeam[C.m_Parent].m_pGame);
					for(int r = 0; r < gs_P.m_Repeat; r++)
						StepIn(Tmp, C.m_In);
					if(!Tmp.HasGrenade() || Tmp.ReloadTimer() > 0 || Tmp.m_CommitLeft > 0 || Tmp.NumProjectiles() > 0)
						continue;
					FireTargets(Tmp, gs_P.m_FireAngles, gs_P.m_FireRange, vAims);
					float Best = C.m_Score;
					for(auto [TX, TY] : vAims)
					{
						F.CopyFrom(Tmp);
						STasInput In = C.m_In;
						In.m_Jump = Tmp.m_LastJump;
						In.m_Hook = Tmp.m_LastHook;
						In.m_Fire = 1;
						In.m_TX = TX;
						In.m_TY = TY;
						In.m_Commit = 0;
						StepIn(F, In);
						In.m_Fire = 0;
						bool Dead = F.Frozen() || F.EnteredFreeze();
						for(int k = 0; k < 12 && !Dead && F.NumProjectiles() > 0; k++)
						{
							StepIn(F, In);
							Dead = F.Frozen() || F.EnteredFreeze();
						}
						if(Dead || F.NumProjectiles() > 0)
							continue;
						Best = std::min(Best, Score(F));
					}
					C.m_Score = Best;
				}
			};
			std::vector<std::thread> vTh;
			for(int T = 1; T < NT; T++)
				vTh.emplace_back(W);
			W();
			for(auto &Th : vTh)
				Th.join();
			std::sort(vAll.begin(), vAll.end(), [](const SCand &a, const SCand &b) { return a.m_Score < b.m_Score; });
		}
		if(gs_P.m_Rollout > 0)
		{
			// keep the plain-score leaders (deduplicated), then rescore them with rollouts
			std::unordered_set<uint64_t> Seen0;
			std::unordered_map<int32_t, int> PerCell0;
			std::vector<SCand> vPre;
			for(const auto &C : vAll)
			{
				if((int)vPre.size() >= gs_P.m_Prefilter * gs_P.m_Beam)
					break;
				if(!Seen0.insert(C.m_Hash).second)
					continue;
				int &n = PerCell0[C.m_Key];
				if(n >= gs_P.m_PerCell * 2)
					continue;
				n++;
				vPre.push_back(C);
			}
			std::atomic<int> Next3{0};
			auto Re = [&]() {
				CTasGame Tmp;
				while(true)
				{
					int k = Next3.fetch_add(1);
					if(k >= (int)vPre.size())
						break;
					Tmp.CopyFrom(*vBeam[vPre[k].m_Parent].m_pGame);
					for(int r = 0; r < gs_P.m_Repeat; r++)
					{
						StepIn(Tmp, vPre[k].m_In);
					}
					vPre[k].m_Score = RolloutScore(Tmp);
				}
			};
			std::vector<std::thread> vTh;
			for(int T = 1; T < NT; T++)
				vTh.emplace_back(Re);
			Re();
			for(auto &Th : vTh)
				Th.join();
			vAll = std::move(vPre);
			std::sort(vAll.begin(), vAll.end(), [](const SCand &a, const SCand &b) { return a.m_Score < b.m_Score; });
		}
		std::unordered_set<uint64_t> Seen;
		std::unordered_map<int32_t, int> PerCell;
		std::vector<SCand> vSel;
		std::vector<int> vPerParent(vBeam.size(), 0);
		if(gs_P.m_Visited)
		{
			// time-ordered reachability: the first arrival in a coarse cell blocks later ones; no
			// ranking beyond the lag cutoff and the cap
			float Lim = vAll.empty() ? 0 : vAll[0].m_Score + gs_P.m_ShareSlack;
			// candidates that could claim a cell: new cell, within the lag cutoff
			std::vector<SCand> vPre;
			std::unordered_set<int64_t> Claim;
			for(const auto &C : vAll)
			{
				if((int)vPre.size() >= gs_P.m_Beam * 2 || C.m_Score > Lim)
					break;
				if(Visited.count(C.m_VKey) || !Seen.insert(C.m_Hash).second)
					continue;
				if(!Claim.insert(C.m_VKey).second)
					continue;
				vPre.push_back(C);
			}
			// doomed states must not claim cells (they would block the cell forever)
			std::vector<char> vOk(vPre.size(), 1);
			if(gs_P.m_Survive > 0)
			{
				std::atomic<int> NextV{0};
				auto W = [&]() {
					CTasGame Tmp;
					while(true)
					{
						int k = NextV.fetch_add(1);
						if(k >= (int)vPre.size())
							break;
						Tmp.CopyFrom(*vBeam[vPre[k].m_Parent].m_pGame);
						for(int r = 0; r < gs_P.m_Repeat; r++)
						{
							StepIn(Tmp, vPre[k].m_In);
						}
						vOk[k] = Survives(Tmp, gs_P.m_Survive);
					}
				};
				std::vector<std::thread> vTh;
				for(int T = 1; T < NT; T++)
					vTh.emplace_back(W);
				W();
				for(auto &Th : vTh)
					Th.join();
			}
			if(getenv("TAS_DBG"))
			{
				int nOk = 0;
				for(char c : vOk)
					nOk += c;
				std::printf("visited: all %zu pre %zu ok %d visited %zu\n", vAll.size(), vPre.size(), nOk, Visited.size());
			}
			for(size_t k = 0; k < vPre.size() && (int)vSel.size() < gs_P.m_Beam; k++)
				if(vOk[k])
				{
					Visited.insert(vPre[k].m_VKey);
					vSel.push_back(vPre[k]);
				}
		}
		else if(gs_P.m_SpeedShare > 0 || gs_P.m_EnergyShare > 0 || gs_P.m_RandShare > 0)
		{
			// portfolio: part of the beam by estimated time, part by speed along the route, part by energy.
			// the extra members are only taken among candidates that aren't too far behind.
			const int NEst = (int)(gs_P.m_Beam * (1.0f - gs_P.m_SpeedShare - gs_P.m_EnergyShare - gs_P.m_RandShare - gs_P.m_GhostShare - gs_P.m_PendShare - gs_P.m_BandShare));
			std::unordered_map<int, int> PerClan;
			auto Take = [&](const std::vector<const SCand *> &vOrder, int Quota) {
				int Taken = 0;
				for(const SCand *pC : vOrder)
				{
					if(Taken >= Quota)
						break;
					int Clan = vBeam[pC->m_Parent].m_Clan;
					if(gs_P.m_ClanCap > 0 && PerClan[Clan] >= gs_P.m_ClanCap)
						continue;
					if(Seen.count(pC->m_Hash))
						continue;
					int &n = PerCell[pC->m_Key];
					if(n >= gs_P.m_PerCell)
						continue;
					Seen.insert(pC->m_Hash);
					PerClan[Clan]++;
					n++;
					vSel.push_back(*pC);
					Taken++;
				}
			};
			std::vector<const SCand *> vOrd;
			for(const auto &C : vAll)
				vOrd.push_back(&C);
			Take(vOrd, NEst);
			float Lim = vAll.empty() ? 0 : vAll[0].m_Score + gs_P.m_ShareSlack;
			std::vector<const SCand *> vNear;
			for(const auto &C : vAll)
				if(C.m_Score <= Lim)
					vNear.push_back(&C);
			auto vSp = vNear;
			std::sort(vSp.begin(), vSp.end(), [](const SCand *a, const SCand *b) { return a->m_Speed > b->m_Speed; });
			Take(vSp, (int)(gs_P.m_Beam * gs_P.m_SpeedShare));
			auto vEn = vNear;
			std::sort(vEn.begin(), vEn.end(), [](const SCand *a, const SCand *b) { return a->m_Energy > b->m_Energy; });
			Take(vEn, (int)(gs_P.m_Beam * gs_P.m_EnergyShare));
			if(gs_P.m_BandShare > 0 && !vAll.empty())
			{
				// candidates a few ticks behind the leader, best first, one per cell: lines that pay off later
				std::vector<const SCand *> vBand;
				const float Best = vAll[0].m_Score;
				for(const auto &C : vAll)
					if(C.m_Score >= Best + gs_P.m_BandLo && C.m_Score <= Best + gs_P.m_BandHi)
						vBand.push_back(&C);
				std::unordered_set<int32_t> BandCells;
				std::vector<const SCand *> vB2;
				for(auto *p : vBand)
					if(BandCells.insert(p->m_Key).second)
						vB2.push_back(p);
				Take(vB2, (int)(gs_P.m_Beam * gs_P.m_BandShare));
			}
			if(gs_P.m_PendShare > 0)
			{
				// states carrying a grenade that explodes later: their payoff is invisible to the score
				std::vector<const SCand *> vPd;
				for(const auto *pC : vNear)
					if(pC->m_Pending > 0)
						vPd.push_back(pC);
				if(gs_P.m_PendRoll)
				{
					// rendezvous quota: rank only by how close the tee is headed to its pad
					std::vector<const SCand *> vF;
					for(auto *p : vPd)
						if(p->m_PendFit > 0)
							vF.push_back(p);
					vPd = vF;
					std::stable_sort(vPd.begin(), vPd.end(), [](const SCand *a, const SCand *b) { return a->m_PendFit > b->m_PendFit || (a->m_PendFit == b->m_PendFit && a->m_Score < b->m_Score); });
				}
				else
					std::stable_sort(vPd.begin(), vPd.end(), [](const SCand *a, const SCand *b) { return a->m_Score - gs_P.m_PendBonus * a->m_PendFit < b->m_Score - gs_P.m_PendBonus * b->m_PendFit; });
				Take(vPd, (int)(gs_P.m_Beam * gs_P.m_PendShare));
			}
			if(gs_P.m_GhostShare > 0)
			{
				// keep the states closest to the ghost's position and velocity (any score)
				std::vector<const SCand *> vGh;
				for(const auto &C : vAll)
					vGh.push_back(&C);
				std::sort(vGh.begin(), vGh.end(), [](const SCand *a, const SCand *b) { return a->m_Track < b->m_Track; });
				Take(vGh, (int)(gs_P.m_Beam * gs_P.m_GhostShare));
			}
			if(gs_P.m_RandShare > 0)
			{
				// pure exploration among candidates that aren't far behind
				auto vRnd = vNear;
				uint64_t Seed = 0x9e3779b97f4a7c15ull * (uint64_t)(Step + 1);
				for(size_t k = vRnd.size(); k > 1; k--)
				{
					Seed ^= Seed << 13;
					Seed ^= Seed >> 7;
					Seed ^= Seed << 17;
					std::swap(vRnd[k - 1], vRnd[Seed % k]);
				}
				Take(vRnd, (int)(gs_P.m_Beam * gs_P.m_RandShare));
			}
			std::sort(vSel.begin(), vSel.end(), [](const SCand &a, const SCand &b) { return a.m_Score < b.m_Score; });
			if(getenv("TAS_PEND"))
			{
				int nP = 0, nPL = 0, nA = 0;
				for(auto &C : vSel)
				{
					nP += C.m_Pending > 0;
					nPL += C.m_Pending > 12;
				}
				for(auto &C : vAll)
					nA += C.m_Pending > 12;
				std::printf("PEND step %d sel %zu pending %d long %d | cands long %d\n", Step, vSel.size(), nP, nPL, nA);
			}
		}
		for(const auto &C : vAll)
		{
			if(gs_P.m_SpeedShare > 0 || gs_P.m_EnergyShare > 0 || gs_P.m_RandShare > 0 || gs_P.m_GhostShare > 0 || gs_P.m_Visited)
				break;
			if((int)vSel.size() >= gs_P.m_Beam)
				break;
			if(gs_P.m_PerParent > 0 && vPerParent[C.m_Parent] >= gs_P.m_PerParent)
				continue;
			if(!Seen.insert(C.m_Hash).second)
				continue;
			int &n = PerCell[C.m_Key];
			if(n >= gs_P.m_PerCell)
				continue;
			n++;
			vPerParent[C.m_Parent]++;
			vSel.push_back(C);
		}
		if(gs_P.m_Survive > 0 && !vSel.empty() && !gs_P.m_Visited)
		{
			// drop doomed states, refill from the remaining candidates
			std::vector<char> vOk(vSel.size());
			auto CheckRange = [&](std::vector<SCand> &vC, std::vector<char> &vRes) {
				std::atomic<int> NextS{0};
				auto W = [&]() {
					CTasGame Tmp;
					while(true)
					{
						int k = NextS.fetch_add(1);
						if(k >= (int)vC.size())
							break;
						Tmp.CopyFrom(*vBeam[vC[k].m_Parent].m_pGame);
						for(int r = 0; r < gs_P.m_Repeat; r++)
						{
							StepIn(Tmp, vC[k].m_In);
						}
						vRes[k] = Survives(Tmp, gs_P.m_Survive);
					}
				};
				std::vector<std::thread> vTh;
				for(int T = 1; T < NT; T++)
					vTh.emplace_back(W);
				W();
				for(auto &Th : vTh)
					Th.join();
			};
			CheckRange(vSel, vOk);
			std::vector<SCand> vKeep;
			for(size_t k = 0; k < vSel.size(); k++)
				if(vOk[k])
					vKeep.push_back(vSel[k]);
			// refill (one pass) from candidates not yet taken
			if((int)vKeep.size() < gs_P.m_Beam)
			{
				std::vector<SCand> vMore;
				for(const auto &C : vAll)
				{
					if((int)(vKeep.size() + vMore.size()) >= gs_P.m_Beam * 2)
						break;
					if(!Seen.insert(C.m_Hash).second)
						continue;
					int &n = PerCell[C.m_Key];
					if(n >= gs_P.m_PerCell * 2)
						continue;
					n++;
					vMore.push_back(C);
				}
				std::vector<char> vOk2(vMore.size());
				CheckRange(vMore, vOk2);
				for(size_t k = 0; k < vMore.size() && (int)vKeep.size() < gs_P.m_Beam; k++)
					if(vOk2[k])
						vKeep.push_back(vMore[k]);
				std::sort(vKeep.begin(), vKeep.end(), [](const SCand &a, const SCand &b) { return a.m_Score < b.m_Score; });
			}
			if(!vKeep.empty())
				vSel = std::move(vKeep);
		}
		// stop when nothing left can beat the best finish
		if(vSel.empty())
			break;
		if(BestFinish < (1 << 30))
		{
			// all remaining states are already at or beyond the best time
			const CTasGame &G0 = *vBeam[vSel[0].m_Parent].m_pGame;
			if(G0.m_Started && G0.m_Tick + 1 - G0.m_StartTick >= BestFinish)
				break;
		}

		// materialize the new beam
		std::vector<SBeamState> vNew(vSel.size());
		std::vector<std::pair<int, STasInput>> vH(vSel.size());
		std::atomic<int> Next2{0};
		auto Mat = [&]() {
			while(true)
			{
				int k = Next2.fetch_add(1);
				if(k >= (int)vSel.size())
					break;
				vNew[k].m_pGame = std::make_unique<CTasGame>();
				vNew[k].m_pGame->CopyFrom(*vBeam[vSel[k].m_Parent].m_pGame);
				for(int r = 0; r < gs_P.m_Repeat; r++)
				{
					StepIn(*vNew[k].m_pGame, vSel[k].m_In);
				}
				vNew[k].m_Prev = vSel[k].m_In;
				vNew[k].m_Clan = (gs_P.m_ClanPeriod > 0 && Step % gs_P.m_ClanPeriod == 0) ? k : vBeam[vSel[k].m_Parent].m_Clan;
				vH[k] = {vSel[k].m_Parent, vSel[k].m_In};
			}
		};
		std::vector<std::thread> vThreads2;
		for(int T = 1; T < NT; T++)
			vThreads2.emplace_back(Mat);
		Mat();
		for(auto &Th : vThreads2)
			Th.join();
		vHist.push_back(std::move(vH));
		vBeam = std::move(vNew);

		if(Step % 50 == 0 || getenv("TAS_DBG"))
		{
			// save the current leader's inputs for inspection
			std::vector<STasInput> vRun;
			int Idx = 0;
			for(int s = (int)vHist.size() - 1; s >= 0; s--)
			{
				vRun.push_back(vHist[s][Idx].second);
				Idx = vHist[s][Idx].first;
			}
			std::reverse(vRun.begin(), vRun.end());
			vRun = ExpandRepeat(vRun);
			std::vector<STasInput> vAllIn = vPrefix;
			vAllIn.insert(vAllIn.end(), vRun.begin(), vRun.end());
			WriteInputs((gs_P.m_Out + ".partial").c_str(), vAllIn);
			const CTasGame &B = *vBeam[0].m_pGame;
			double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
			std::printf("step %d tick %d best est %.1f pos %.0f %.0f vel %.1f %.1f started %d(%d) gren %d D %.0f cands %zu beam %zu [%.0fs]\n", Step, B.m_Tick, vSel[0].m_Score, B.Pos().x, B.Pos().y,
				B.Vel().x, B.Vel().y, B.m_Started, B.m_StartTick, B.HasGrenade(), DistTo(B, B.Pos()), vAll.size(), vBeam.size(), Sec);
			std::fflush(stdout);
		}
	}
	if(gs_P.m_PrestartObj && !gs_P.m_SpeedObj && !vGateBest.empty())
	{
		WriteInputs(gs_P.m_Out.c_str(), vGateBest);
		std::printf("PRESTART best crossing written (%zu ticks)\n", vGateBest.size());
	}
	if(gs_P.m_PreCands > 0)
	{
		std::vector<const SCrossBest *> vAll;
		for(const auto &[k, v] : gs_Cross)
			vAll.push_back(&v);
		std::sort(vAll.begin(), vAll.end(), [](const SCrossBest *a, const SCrossBest *b) { return a->m_E > b->m_E; });
		std::printf("PRECANDS %zu distinct crossings\n", vAll.size());
		for(int k = 0; k < (int)vAll.size() && k < gs_P.m_PreCands; k++)
		{
			char aBuf[512];
			std::snprintf(aBuf, sizeof(aBuf), "%s.c%d.txt", gs_P.m_Out.c_str(), k);
			WriteInputs(aBuf, vAll[k]->m_vIn);
			std::printf("PRECAND %d E %.0f pos %.0f %.0f vel %.2f %.2f |v| %.2f jumps %d tick %d len %zu -> %s\n", k, vAll[k]->m_E, vAll[k]->m_Pos.x, vAll[k]->m_Pos.y,
				vAll[k]->m_Vel.x, vAll[k]->m_Vel.y, length(vAll[k]->m_Vel), vAll[k]->m_Jumps, vAll[k]->m_Tick, vAll[k]->m_vIn.size(), aBuf);
		}
		if(gs_P.m_PreEvalX > 0)
		{
			// rollout gate: a short corridor search from each crossing, ranked by ticks to reach x
			SParams Saved = gs_P;
			gs_P.m_PrestartObj = 0;
			gs_P.m_PrestartEnergy = 0;
			gs_P.m_PreCands = 0;
			gs_P.m_PreEvalX = 0;
			gs_P.m_StopX = Saved.m_PreEvalX;
			gs_P.m_Beam = 400;
			gs_P.m_MaxTicks = 200;
			gs_P.m_Out = Saved.m_Out + ".evaltmp";
			std::vector<std::pair<int, int>> vRank; // ticks, candidate index
			for(int k = 0; k < (int)vAll.size() && k < Saved.m_PreCands; k++)
			{
				std::fflush(stdout);
				Search(vAll[k]->m_vIn);
				int Ticks = gs_StopTicks;
				std::printf("PREEVAL %d E %.0f pos %.0f %.0f vel %.2f %.2f jumps %d -> %d ticks to x=%.0f\n", k, vAll[k]->m_E, vAll[k]->m_Pos.x, vAll[k]->m_Pos.y,
					vAll[k]->m_Vel.x, vAll[k]->m_Vel.y, vAll[k]->m_Jumps, Ticks, Saved.m_PreEvalX);
				vRank.push_back({Ticks, k});
			}
			std::sort(vRank.begin(), vRank.end());
			for(int r = 0; r < (int)vRank.size() && r < 6; r++)
			{
				char aBuf[512];
				std::snprintf(aBuf, sizeof(aBuf), "%s.r%d.txt", Saved.m_Out.c_str(), r);
				WriteInputs(aBuf, vAll[vRank[r].second]->m_vIn);
				std::printf("PRERANK %d: cand %d, %d ticks -> %s\n", r, vRank[r].second, vRank[r].first, aBuf);
			}
			gs_P = Saved;
		}
	}
	return vBest;
}

// ---------------------------------------------------------------------------
// local bruteforce: small targeted edits of a known input sequence, kept only when the true
// result (tick the target is reached, then the score there) improves
struct SBruteRes
{
	bool m_Ok = false;
	int m_Ticks = 1 << 30; // ticks after the start when the target is reached
	float m_Est = 1e30f;
	int m_ReachIdx = 0; // input index at which it was reached
};

static bool BruteTarget(const CTasGame &G)
{
	if(!G.m_Started || G.m_Tick - G.m_StartTick < gs_P.m_TargetMinTick)
		return false;
	if(gs_P.m_StopX > 0)
		return G.Pos().x > gs_P.m_StopX;
	if(gs_P.m_StopXLt > 0)
		return G.Pos().x < gs_P.m_StopXLt;
	if(gs_P.m_StopAtGrenade)
		return G.HasGrenade();
	return G.m_FinishTick >= 0;
}

static const int BRUTE_CP = 25;

static void BruteCheckpoints(const std::vector<STasInput> &v, std::vector<std::unique_ptr<CTasGame>> &vCp)
{
	vCp.clear();
	auto pG = std::make_unique<CTasGame>();
	pG->Spawn(CTasGame::Map().m_vSpawns[0]);
	for(size_t i = 0; i <= v.size(); i++)
	{
		if(i % BRUTE_CP == 0)
		{
			auto pC = std::make_unique<CTasGame>();
			pC->CopyFrom(*pG);
			vCp.push_back(std::move(pC));
		}
		if(i < v.size())
			pG->Step(v[i]);
	}
}

static SBruteRes BruteEval(const std::vector<STasInput> &v, int From, std::vector<std::unique_ptr<CTasGame>> &vCp, CTasGame &G)
{
	SBruteRes R;
	int c = std::min(From / BRUTE_CP, (int)vCp.size() - 1);
	G.CopyFrom(*vCp[c]);
	for(size_t i = (size_t)c * BRUTE_CP; i < v.size(); i++)
	{
		G.Step(v[i]);
		if(G.Frozen() || G.EnteredFreeze() || G.m_StartTick == -2)
			return R;
		if(BruteTarget(G))
		{
			R.m_Ok = true;
			R.m_Ticks = G.m_Tick - G.m_StartTick;
			R.m_Est = Score(G);
			R.m_ReachIdx = (int)i;
			return R;
		}
	}
	return R;
}

static bool BruteMutate(std::vector<STasInput> &v, int Limit, uint64_t &Seed, int &FirstChanged)
{
	auto Rnd = [&](int n) {
		Seed ^= Seed << 13;
		Seed ^= Seed >> 7;
		Seed ^= Seed << 17;
		return (int)(Seed % (uint64_t)std::max(1, n));
	};
	const int L = std::min((int)v.size(), Limit);
	if(L < 3)
		return false;
	int Kind = Rnd(5);
	if(Kind == 0 || Kind == 1)
	{
		// move a hook press or release by 1..3 ticks
		std::vector<int> vEdges;
		for(int t = 1; t < L; t++)
			if(v[t].m_Hook != v[t - 1].m_Hook)
				vEdges.push_back(t);
		if(vEdges.empty())
			return false;
		int t = vEdges[Rnd(vEdges.size())];
		int k = 1 + Rnd(3);
		bool Press = v[t].m_Hook;
		if(Rnd(2))
		{
			// earlier: the new state starts k ticks before
			for(int j = std::max(1, t - k); j < t; j++)
			{
				v[j].m_Hook = v[t].m_Hook;
				if(Press)
				{
					v[j].m_TX = v[t].m_TX;
					v[j].m_TY = v[t].m_TY;
				}
			}
			FirstChanged = std::max(0, t - k);
		}
		else
		{
			// later: the old state lasts k more ticks
			for(int j = t; j < std::min(L, t + k); j++)
				v[j].m_Hook = v[t - 1].m_Hook;
			if(Press && t + k < L)
			{
				v[t + k].m_TX = v[t].m_TX;
				v[t + k].m_TY = v[t].m_TY;
			}
			FirstChanged = t;
		}
		return true;
	}
	if(Kind == 2)
	{
		// nudge the aim of a hook press
		std::vector<int> vPress;
		for(int t = 1; t < L; t++)
			if(v[t].m_Hook && !v[t - 1].m_Hook)
				vPress.push_back(t);
		if(vPress.empty())
			return false;
		int t = vPress[Rnd(vPress.size())];
		float Ang = std::atan2((float)v[t].m_TY, (float)v[t].m_TX) + (Rnd(2) ? 1 : -1) * (0.5f + Rnd(12)) * pi / 180.0f;
		int16_t TX = (int16_t)std::lround(std::cos(Ang) * 1000), TY = (int16_t)std::lround(std::sin(Ang) * 1000);
		for(int j = t; j < L && v[j].m_Hook; j++)
		{
			v[j].m_TX = TX;
			v[j].m_TY = TY;
		}
		FirstChanged = t;
		return true;
	}
	if(Kind == 3)
	{
		// add, remove or move a jump press
		int t = 1 + Rnd(L - 2);
		int k = 1 + Rnd(3);
		uint8_t J = !v[t].m_Jump;
		for(int j = t; j < std::min(L, t + k); j++)
			v[j].m_Jump = J;
		FirstChanged = t;
		return true;
	}
	// direction over a short range
	int t = Rnd(L - 1);
	int k = 1 + Rnd(6);
	int8_t D = (int8_t)(Rnd(3) - 1);
	for(int j = t; j < std::min(L, t + k); j++)
		v[j].m_Dir = D;
	FirstChanged = t;
	return true;
}

static void Brute(std::vector<STasInput> vBest, double Seconds)
{
	// pad so slightly slower variants can still reach the target
	for(int k = 0; k < 60; k++)
		vBest.push_back(vBest.back());
	std::mutex Mutex;
	std::vector<std::unique_ptr<CTasGame>> vCp0;
	BruteCheckpoints(vBest, vCp0);
	CTasGame G0;
	SBruteRes Best = BruteEval(vBest, 0, vCp0, G0);
	if(!Best.m_Ok)
	{
		std::printf("BRUTE: initial inputs don't reach the target\n");
		return;
	}
	std::printf("BRUTE start: %d ticks (est %.2f)\n", Best.m_Ticks, Best.m_Est);
	std::fflush(stdout);
	int Version = 0;
	std::atomic<long> Evals{0};
	auto t0 = std::chrono::steady_clock::now();
	auto Worker = [&](int T) {
		uint64_t Seed = 0x9e3779b97f4a7c15ull * (T + 1) + (uint64_t)time(nullptr);
		std::vector<STasInput> vMine;
		std::vector<std::unique_ptr<CTasGame>> vCp;
		int MyVersion = -1;
		SBruteRes MyBest;
		CTasGame G;
		while(std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() < Seconds)
		{
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				if(MyVersion != Version)
				{
					vMine = vBest;
					MyBest = Best;
					MyVersion = Version;
					BruteCheckpoints(vMine, vCp);
				}
			}
			std::vector<STasInput> vTry = vMine;
			int First = 0;
			if(!BruteMutate(vTry, MyBest.m_ReachIdx + 1, Seed, First))
				continue;
			SBruteRes R = BruteEval(vTry, First, vCp, G);
			Evals++;
			if(R.m_Ok && (R.m_Ticks < MyBest.m_Ticks || (R.m_Ticks == MyBest.m_Ticks && R.m_Est < MyBest.m_Est - 1e-3f)))
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				if(R.m_Ticks < Best.m_Ticks || (R.m_Ticks == Best.m_Ticks && R.m_Est < Best.m_Est - 1e-3f))
				{
					Best = R;
					vBest = vTry;
					Version++;
					std::vector<STasInput> vOut(vBest.begin(), vBest.begin() + Best.m_ReachIdx + 1);
					WriteInputs(gs_P.m_Out.c_str(), vOut);
					std::printf("BRUTE %d ticks (est %.2f) after %ld evals, %.0fs\n", Best.m_Ticks, Best.m_Est, Evals.load(),
						std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
					std::fflush(stdout);
				}
			}
		}
	};
	std::vector<std::thread> vTh;
	for(int T = 1; T < gs_P.m_Threads; T++)
		vTh.emplace_back(Worker, T);
	Worker(0);
	for(auto &Th : vTh)
		Th.join();
	std::printf("BRUTE done: %d ticks, %ld evals\n", Best.m_Ticks, Evals.load());
}

int main(int argc, const char **argv)
{
	if(argc < 3)
	{
		std::printf("usage: tas <map> replay <inputs> | search [key=value...]\n");
		return 1;
	}
	if(!CTasGame::LoadMap(argv[1]))
	{
		std::printf("map load failed\n");
		return 1;
	}
	std::string Mode = argv[2];
	if(Mode == "replay")
	{
		auto vIn = ReadInputs(argv[3]);
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(size_t i = 0; i < vIn.size(); i++)
		{
			G.Step(vIn[i]);
			std::printf("%d %.3f %.3f %.3f %.3f frz=%d start=%d fin=%d\n", G.m_Tick, G.Pos().x, G.Pos().y, G.Vel().x, G.Vel().y, G.Frozen(), G.m_StartTick, G.m_FinishTick);
		}
		return 0;
	}
	if(Mode == "cut")
	{
		// cut <inputs> <D> <out>: keep the inputs up to the first started tick with distance to go below D
		const SMapInfo &M = CTasGame::Map();
		gs_Dist.Build(M, {TILE_FINISH});
		float BestD = -1;
		for(int y = 0; y < M.m_H; y++)
			for(int x = 0; x < M.m_W; x++)
				if(M.Tile(x, y) == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
				{
					vec2 P(x * 32 + 16, y * 32 + 16);
					float D = gs_Dist.Sample(P);
					if(D < 1e8f && D > BestD)
					{
						BestD = D;
						gs_GrenadePos = P;
					}
				}
		gs_DistGrenade.BuildFromPoint(M, gs_GrenadePos);
		auto vIn = ReadInputs(argv[3]);
		float Thr = std::stof(argv[4]);
		CTasGame G;
		G.Spawn(M.m_vSpawns[0]);
		size_t n = 0;
		for(; n < vIn.size(); n++)
		{
			G.Step(vIn[n]);
			if(G.m_Started && GeoDistTo(G, G.Pos()) < Thr)
			{
				n++;
				break;
			}
		}
		vIn.resize(n);
		WriteInputs(argv[5], vIn);
		std::printf("CUT %zu\n", n);
		return 0;
	}
	if(Mode == "bench")
	{
		CTasGame G, T;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		STasInput In;
		In.m_Dir = 1;
		for(int i = 0; i < 60; i++)
			G.Step(In);
		const int N = getenv("TAS_BENCH_N") ? atoi(getenv("TAS_BENCH_N")) : 200000;
		auto t0 = std::chrono::steady_clock::now();
		for(int i = 0; i < N; i++)
			T.CopyFrom(G);
		auto t1 = std::chrono::steady_clock::now();
		for(int i = 0; i < N; i++)
		{
			In.m_Hook = (i / 7) & 1;
			In.m_TX = 300;
			In.m_TY = -300;
			T.CopyFrom(G);
			T.Step(In);
		}
		auto t2 = std::chrono::steady_clock::now();
		for(int i = 0; i < N; i++)
			T.Hash();
		auto t3 = std::chrono::steady_clock::now();
		std::printf("copy %.3f us, copy+step %.3f us, hash %.3f us\n", std::chrono::duration<double>(t1 - t0).count() / N * 1e6,
			std::chrono::duration<double>(t2 - t1).count() / N * 1e6, std::chrono::duration<double>(t3 - t2).count() / N * 1e6);
		return 0;
	}



	if(Mode == "shots")
	{
		// exhaustive grenade-boost study: up to 3 shots (free aim/timing, reload-limited), dir -1, optional jump
		// shots <prefix> <prefixlen> <goal x (reach x < X tiles)> [maxticks] [threads] [nshots]
		auto vIn = ReadInputs(argv[3]);
		vIn.resize(std::min((size_t)atoi(argv[4]), vIn.size()));
		const float GX = atof(argv[5]) * 32;
		const int MaxT = argc > 6 ? atoi(argv[6]) : 90;
		const int NT = argc > 7 ? atoi(argv[7]) : 2;
		const int NS = argc > 8 ? atoi(argv[8]) : 3;
		CTasGame Base;
		Base.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(const auto &In : vIn)
			Base.Step(In);
		std::printf("base: pos %.1f %.1f vel %.2f %.2f |v| %.2f grenade %d reload %d\n", Base.Pos().x / 32, Base.Pos().y / 32, Base.Vel().x, Base.Vel().y, length(Base.Vel()), Base.HasGrenade(), Base.ReloadTimer());
		const int NA = 48; // aim angles
		struct SRes { int m_Ticks; float m_Speed; int m_A[3], m_T[3]; vec2 m_Pos, m_Vel; };
		std::vector<SRes> vRes;
		std::mutex Mx;
		std::atomic<int> Next{0};
		std::atomic<long> Sims{0};
		auto Aim = [&](int a, int16_t &TX, int16_t &TY) { float Ang = 2 * pi * a / NA; TX = (int16_t)std::lround(std::cos(Ang) * 1000); TY = (int16_t)std::lround(std::sin(Ang) * 1000); if(!TX && !TY) TY = -1; };
		auto Worker = [&]() {
			CTasGame G, G1, G2;
			while(true)
			{
				int a1 = Next.fetch_add(1);
				if(a1 >= NA)
					break;
				const bool HookGrid = getenv("SHOTS_HOOK") != nullptr;
				const int T1Min = getenv("SHOTS_T1MIN") ? atoi(getenv("SHOTS_T1MIN")) : 0;
				const int T1Max = getenv("SHOTS_T1MAX") ? atoi(getenv("SHOTS_T1MAX")) : 12;
				for(int hk = 0; hk < (HookGrid ? 1 + 12 * 3 : 1); hk++)
				for(int t1 = T1Min; t1 <= T1Max; t1 += 2)
				{
					// optional steering hook from t = 0: angle index and hold
					int16_t HX = 0, HY = -1; int HHold = 0;
					if(hk > 0)
					{
						float HA = 2 * pi * ((hk - 1) % 12) / 12; HX = (int16_t)std::lround(std::cos(HA) * 1000); HY = (int16_t)std::lround(std::sin(HA) * 1000); if(!HX && !HY) HY = -1;
						HHold = 3 + 3 * ((hk - 1) / 12);
					}
					// shot 1 phase
					int16_t X1, Y1; Aim(a1, X1, Y1);
					G1.CopyFrom(Base);
					bool Dead = false;
					for(int t = 0; t <= t1; t++)
					{
						STasInput In; In.m_Dir = -1; In.m_Weapon = 3;
						const bool Hk = t < HHold;
						In.m_Hook = Hk ? 1 : 0;
						In.m_TX = Hk ? HX : X1; In.m_TY = Hk ? HY : Y1;
						In.m_Fire = (t == t1) ? 1 : 0;
						G1.Step(In);
						if(G1.Frozen() || G1.EnteredFreeze()) { Dead = true; break; }
					}
					if(Dead) continue;
					for(int a2 = 0; a2 < (NS >= 2 ? NA : 1); a2++)
						for(int d2 = 25; d2 <= (NS >= 2 ? 33 : 25); d2 += 4)
						{
							int16_t X2, Y2; Aim(a2, X2, Y2);
							G2.CopyFrom(G1);
							Dead = false;
							int t2 = t1 + d2;
							for(int t = t1 + 1; t <= t2; t++)
							{
								STasInput In; In.m_Dir = -1; In.m_Weapon = 3; In.m_TX = X2; In.m_TY = Y2; In.m_Fire = (NS >= 2 && t == t2) ? 1 : 0;
								G2.Step(In);
								if(G2.Frozen() || G2.EnteredFreeze()) { Dead = true; break; }
								if(G2.Pos().x < GX) break;
							}
							if(Dead) continue;
							if(G2.Pos().x < GX)
							{
								std::lock_guard<std::mutex> L(Mx);
								vRes.push_back({G2.m_Tick - Base.m_Tick, length(G2.Vel()), {a1, a2, -1}, {t1, t2, -1}, G2.Pos(), G2.Vel()});
								continue;
							}
							for(int a3 = 0; a3 < (NS >= 3 ? NA : 1); a3++)
								for(int d3 = 25; d3 <= (NS >= 3 ? 29 : 25); d3 += 4)
								{
									int16_t X3, Y3; Aim(a3, X3, Y3);
									G.CopyFrom(G2);
									Sims++;
									int t3 = t2 + d3;
									int Reached = -1;
									for(int t = t2 + 1; t < MaxT; t++)
									{
										STasInput In; In.m_Dir = -1; In.m_Weapon = 3; In.m_TX = X3; In.m_TY = Y3; In.m_Fire = (NS >= 3 && t == t3) ? 1 : 0;
										G.Step(In);
										if(G.Frozen() || G.EnteredFreeze()) break;
										if(G.Pos().x < GX) { Reached = t + 1; break; }
									}
									if(Reached > 0)
									{
										std::lock_guard<std::mutex> L(Mx);
										vRes.push_back({Reached, length(G.Vel()), {a1, a2, a3}, {t1, t2, t3}, G.Pos(), G.Vel()});
									}
									if(NS < 3) break;
								}
							if(NS < 2) break;
						}
				}
			}
		};
		std::vector<std::thread> vTh;
		for(int T = 1; T < NT; T++)
			vTh.emplace_back(Worker);
		Worker();
		for(auto &Th : vTh)
			Th.join();
		std::printf("%ld simulations, %zu reach x < %.0f\n", Sims.load(), vRes.size(), GX / 32);
		std::sort(vRes.begin(), vRes.end(), [](const SRes &a, const SRes &b) { return a.m_Ticks < b.m_Ticks || (a.m_Ticks == b.m_Ticks && a.m_Speed > b.m_Speed); });
		for(int k = 0; k < (int)vRes.size() && k < 15; k++)
		{
			const auto &R = vRes[k];
			std::printf("ticks %3d |v| %5.1f  shots: t%d@%4.0f  t%d@%4.0f  t%d@%4.0f   -> (%.1f,%.1f) vel (%.1f,%.1f)\n", R.m_Ticks, R.m_Speed, R.m_T[0], 360.0f * R.m_A[0] / NA, R.m_T[1], R.m_A[1] >= 0 ? 360.0f * R.m_A[1] / NA : -1.f, R.m_T[2], R.m_A[2] >= 0 ? 360.0f * R.m_A[2] / NA : -1.f, R.m_Pos.x / 32, R.m_Pos.y / 32, R.m_Vel.x, R.m_Vel.y);
		}
		return 0;
	}
	if(Mode == "maneuver2")
	{
		// two hooks + one jump, exhaustive, from a prefix state; goal = region (x < GX, y < GY); ranked by speed there
		// maneuver2 <prefix> <prefixlen> <goal tile x> <goal tile y> [maxticks] [threads]
		auto vIn = ReadInputs(argv[3]);
		vIn.resize(std::min((size_t)atoi(argv[4]), vIn.size()));
		const float GX = atof(argv[5]) * 32, GY = atof(argv[6]) * 32;
		const int MaxT = argc > 7 ? atoi(argv[7]) : 70;
		const int NT = argc > 8 ? atoi(argv[8]) : 2;
		CTasGame Base;
		Base.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(const auto &In : vIn)
			Base.Step(In);
		if(getenv("TAS_TP"))
		{
			float x, y, vx, vy;
			int j = 0;
			std::sscanf(getenv("TAS_TP"), "%f,%f,%f,%f,%d", &x, &y, &vx, &vy, &j);
			Base.SetState(vec2(x, y), vec2(vx, vy));
			Base.Chr()->m_Core.m_Jumped = j;
			Base.Chr()->m_Core.m_HookState = HOOK_IDLE;
		}
		if(getenv("TAS_TP"))
		{
			float x, y, vx, vy;
			int j = 0;
			std::sscanf(getenv("TAS_TP"), "%f,%f,%f,%f,%d", &x, &y, &vx, &vy, &j);
			Base.SetState(vec2(x, y), vec2(vx, vy));
			Base.Chr()->m_Core.m_Jumped = j;
			Base.Chr()->m_Core.m_HookState = HOOK_IDLE;
		}
		std::printf("base: pos %.1f %.1f vel %.2f %.2f |v| %.2f jumped %d\n", Base.Pos().x / 32, Base.Pos().y / 32, Base.Vel().x, Base.Vel().y, length(Base.Vel()), Base.Jumped());
		struct SRes { int m_Ticks; float m_Speed; int m_A1, m_F1, m_H1, m_J, m_A2, m_F2, m_H2; vec2 m_Pos, m_Vel; };
		std::vector<SRes> vRes;
		std::mutex Mx;
		// angle sets in degrees (y down): hook 1 mostly up/back, hook 2 anything upward-ish
		std::vector<int> vA1, vA2;
		for(int a = -20; a >= -120; a -= 4) vA1.push_back(a);
		for(int a = 0; a >= -180; a -= 6) vA2.push_back(a);
		std::atomic<int> Next{0};
		std::atomic<long> Sims{0};
		auto Worker = [&]() {
			CTasGame G, G1;
			while(true)
			{
				int idx = Next.fetch_add(1);
				if(idx >= (int)vA1.size())
					break;
				float Ang1 = vA1[idx] * pi / 180.0f;
				int16_t TX1 = (int16_t)std::lround(std::cos(Ang1) * 1000), TY1 = (int16_t)std::lround(std::sin(Ang1) * 1000);
				for(int F1 = 0; F1 <= 10; F1 += 1)
					for(int H1 = 2; H1 <= 14; H1 += 2)
						for(int J = -1; J <= F1 + H1 + 2; J += 2)
						{
							// phase 1: hook 1 (+ jump), simulate up to its release; skip dead ones
							G1.CopyFrom(Base);
							bool Dead = false;
							int T1 = F1 + H1;
							for(int t = 0; t < T1; t++)
							{
								STasInput In;
								In.m_Dir = -1;
								In.m_TX = TX1; In.m_TY = TY1;
								In.m_Hook = (t >= F1) ? 1 : 0;
								In.m_Jump = (J >= 0 && t >= J && t < J + 2) ? 1 : 0;
								G1.Step(In);
								if(G1.Frozen() || G1.EnteredFreeze()) { Dead = true; break; }
							}
							if(Dead)
								continue;
							for(int ia2 = 0; ia2 < (int)vA2.size(); ia2++)
							{
								float Ang2 = vA2[ia2] * pi / 180.0f;
								int16_t TX2 = (int16_t)std::lround(std::cos(Ang2) * 1000), TY2 = (int16_t)std::lround(std::sin(Ang2) * 1000);
								for(int F2 = 1; F2 <= 9; F2 += 2)
									for(int H2 = 2; H2 <= 12; H2 += 2)
									{
										G.CopyFrom(G1);
										Sims++;
										int Reached = -1;
										for(int t = T1; t < MaxT; t++)
										{
											int u = t - T1;
											STasInput In;
											In.m_Dir = -1;
											In.m_TX = TX2; In.m_TY = TY2;
											In.m_Hook = (u >= F2 && u < F2 + H2) ? 1 : 0;
											In.m_Jump = (J >= 0 && t >= J && t < J + 2) ? 1 : 0;
											G.Step(In);
											if(G.Frozen() || G.EnteredFreeze())
												break;
											if(G.Pos().x < GX && G.Pos().y < GY) { Reached = t + 1; break; }
										}
										if(Reached > 0)
										{
											vec2 V = G.Vel();
											std::lock_guard<std::mutex> L(Mx);
											vRes.push_back({Reached, length(V), vA1[idx], F1, H1, J, vA2[ia2], F2, H2, G.Pos(), V});
										}
									}
							}
						}
			}
		};
		std::vector<std::thread> vTh;
		for(int T = 1; T < NT; T++)
			vTh.emplace_back(Worker);
		Worker();
		for(auto &Th : vTh)
			Th.join();
		std::printf("%ld simulations, %zu reach the goal\n", Sims.load(), vRes.size());
		std::sort(vRes.begin(), vRes.end(), [](const SRes &a, const SRes &b) { return a.m_Speed > b.m_Speed; });
		for(int k = 0; k < (int)vRes.size() && k < 25; k++)
		{
			const auto &R = vRes[k];
			std::printf("|v| %5.2f at tick %3d  hook1 ang %4d fire %2d hold %2d | jump %3d | hook2 ang %4d fire+%d hold %2d  -> (%.1f,%.1f) vel (%.1f,%.1f)\n", R.m_Speed, R.m_Ticks, R.m_A1, R.m_F1, R.m_H1, R.m_J, R.m_A2, R.m_F2, R.m_H2, R.m_Pos.x / 32, R.m_Pos.y / 32, R.m_Vel.x, R.m_Vel.y);
		}
		// also the earliest arrivals
		std::sort(vRes.begin(), vRes.end(), [](const SRes &a, const SRes &b) { return a.m_Ticks < b.m_Ticks || (a.m_Ticks == b.m_Ticks && a.m_Speed > b.m_Speed); });
		for(int k = 0; k < (int)vRes.size() && k < 5; k++)
		{
			const auto &R = vRes[k];
			std::printf("EARLY tick %3d |v| %5.2f  hook1 ang %4d fire %2d hold %2d | jump %3d | hook2 ang %4d fire+%d hold %2d\n", R.m_Ticks, R.m_Speed, R.m_A1, R.m_F1, R.m_H1, R.m_J, R.m_A2, R.m_F2, R.m_H2);
		}
		return 0;
	}
	if(Mode == "maneuver")
	{
		// exhaustive single-hook (+ optional jump) maneuver study from a prefix state:
		// maneuver <prefix> <prefixlen> <goal tile x (reach x < X)> <goal tile y (reach y < Y)> [maxticks]
		auto vIn = ReadInputs(argv[3]);
		vIn.resize(std::min((size_t)atoi(argv[4]), vIn.size()));
		const float GX = atof(argv[5]) * 32, GY = atof(argv[6]) * 32;
		const int MaxT = argc > 7 ? atoi(argv[7]) : 70;
		CTasGame Base;
		Base.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(const auto &In : vIn)
			Base.Step(In);
		if(getenv("TAS_TP"))
		{
			float x, y, vx, vy;
			int j = 0;
			std::sscanf(getenv("TAS_TP"), "%f,%f,%f,%f,%d", &x, &y, &vx, &vy, &j);
			Base.SetState(vec2(x, y), vec2(vx, vy));
			Base.Chr()->m_Core.m_Jumped = j;
			Base.Chr()->m_Core.m_HookState = HOOK_IDLE;
		}
		std::printf("base: pos %.1f %.1f vel %.2f %.2f |v| %.2f jumped %d\n", Base.Pos().x / 32, Base.Pos().y / 32, Base.Vel().x, Base.Vel().y, length(Base.Vel()), Base.Jumped());
		struct SRes { int m_Ticks; float m_Speed; float m_E; int m_Fire, m_Ang, m_Hold, m_Jump; vec2 m_Pos, m_Vel; };
		std::vector<SRes> vRes;
		std::mutex Mx;
		const int NA = 96;
		std::atomic<int> Next{0};
		auto Worker = [&]() {
			CTasGame G;
			while(true)
			{
				int a = Next.fetch_add(1);
				if(a >= NA)
					break;
				float Ang = 2 * pi * a / NA;
				int16_t TX = (int16_t)std::lround(std::cos(Ang) * 1000), TY = (int16_t)std::lround(std::sin(Ang) * 1000);
				for(int Fire = 0; Fire <= 14; Fire += 1)
					for(int Hold = 1; Hold <= 26; Hold += 1)
						for(int Jump = -1; Jump <= 44; Jump += 2)
						{
							G.CopyFrom(Base);
							int Reached = -1;
							for(int t = 0; t < MaxT; t++)
							{
								STasInput In;
								In.m_Dir = -1;
								In.m_TX = TX;
								In.m_TY = TY;
								In.m_Hook = (t >= Fire && t < Fire + Hold) ? 1 : 0;
								In.m_Jump = (Jump >= 0 && t >= Jump && t < Jump + 2) ? 1 : 0;
								G.Step(In);
								if(G.Frozen() || G.EnteredFreeze())
									break;
								if(G.Pos().x < GX && G.Pos().y < GY)
								{
									Reached = t + 1;
									break;
								}
							}
							if(Reached > 0)
							{
								vec2 V = G.Vel();
								std::lock_guard<std::mutex> L(Mx);
								vRes.push_back({Reached, length(V), dot(V, V) - G.Pos().y, Fire, a, Hold, Jump, G.Pos(), V});
							}
						}
			}
		};
		std::vector<std::thread> vTh;
		for(int T = 1; T < gs_P.m_Threads; T++)
			vTh.emplace_back(Worker);
		Worker();
		for(auto &Th : vTh)
			Th.join();
		std::printf("%zu maneuvers reach the goal\n", vRes.size());
		// Pareto front: earliest tick for each speed band, and fastest for each tick
		std::sort(vRes.begin(), vRes.end(), [](const SRes &a, const SRes &b) { return a.m_Ticks < b.m_Ticks || (a.m_Ticks == b.m_Ticks && a.m_Speed > b.m_Speed); });
		float BestSpeed = -1;
		for(const auto &R : vRes)
			if(R.m_Speed > BestSpeed + 0.5f)
			{
				BestSpeed = R.m_Speed;
				std::printf("ticks %3d |v| %5.2f E %6.0f  fire %2d ang %5.0f hold %2d jump %3d  at (%.1f,%.1f) vel (%.1f,%.1f)\n", R.m_Ticks, R.m_Speed, R.m_E, R.m_Fire, 360.0f * R.m_Ang / NA, R.m_Hold, R.m_Jump, R.m_Pos.x / 32, R.m_Pos.y / 32, R.m_Vel.x, R.m_Vel.y);
			}
		return 0;
	}
	if(Mode == "search")
	{
		for(int i = 3; i < argc; i++)
		{
			std::string A = argv[i];
			auto Eq = A.find('=');
			std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
			if(K == "beam")
				gs_P.m_Beam = std::stoi(V);
			else if(K == "angles")
				gs_P.m_Angles = std::stoi(V);
			else if(K == "maxticks")
				gs_P.m_MaxTicks = std::stoi(V);
			else if(K == "vref")
				gs_P.m_Vref = std::stof(V);
			else if(K == "alpha")
				gs_P.m_Alpha = std::stof(V);
			else if(K == "threads")
				gs_P.m_Threads = std::min(2, std::stoi(V));
			else if(K == "percell")
				gs_P.m_PerCell = std::stoi(V);
			else if(K == "cell")
				gs_P.m_CellSize = std::stof(V);
			else if(K == "velcell")
				gs_P.m_VelCell = std::stof(V);
			else if(K == "grenade")
				gs_P.m_UseGrenade = std::stoi(V);
			else if(K == "dynvref")
				gs_P.m_DynVref = std::stof(V);
			else if(K == "dynvmin")
				gs_P.m_DynVmin = std::stof(V);
			else if(K == "firelook")
				gs_P.m_FireLookahead = std::stoi(V);
			else if(K == "fireangles")
				gs_P.m_FireAngles = std::stoi(V);
			else if(K == "firerange")
				gs_P.m_FireRange = std::stof(V);
			else if(K == "jumpenergy")
				gs_P.m_JumpEnergy = std::stof(V);
			else if(K == "speedshare")
				gs_P.m_SpeedShare = std::stof(V);
			else if(K == "tp")
			{
				gs_P.m_Teleport = 1;
				std::sscanf(V.c_str(), "%f,%f,%f,%f,%d", &gs_P.m_TpX, &gs_P.m_TpY, &gs_P.m_TpVx, &gs_P.m_TpVy, &gs_P.m_TpJumped);
			}
			else if(K == "brute")
				gs_P.m_Brute = std::stod(V);
			else if(K == "macro")
			{
				gs_P.m_vMacro.clear();
				size_t p0 = 0;
				while(p0 < V.size())
				{
					size_t c = V.find(',', p0);
					gs_P.m_vMacro.push_back(std::stoi(V.substr(p0, c == std::string::npos ? std::string::npos : c - p0)));
					if(c == std::string::npos)
						break;
					p0 = c + 1;
				}
			}
			else if(K == "macroonly")
				gs_P.m_MacroOnly = std::stoi(V);
			else if(K == "ekappa")
				gs_P.m_EKappa = std::stof(V);
			else if(K == "repeat")
				gs_P.m_Repeat = std::max(1, std::stoi(V));
			else if(K == "track")
				gs_P.m_Track = std::stoi(V);
			else if(K == "trackvel")
				gs_P.m_TrackVel = std::stof(V);
			else if(K == "ghost")
				gs_P.m_GhostFile = V;
			else if(K == "ref")
				gs_P.m_RefFile = V;
			else if(K == "stencil16")
				gs_Stencil16 = std::stoi(V);
			else if(K == "visited")
				gs_P.m_Visited = std::stoi(V);
			else if(K == "vcell")
				gs_P.m_VCell = std::stof(V);
			else if(K == "vvel")
				gs_P.m_VVel = std::stof(V);
			else if(K == "ghostshare")
				gs_P.m_GhostShare = std::stof(V);
			else if(K == "randshare")
				gs_P.m_RandShare = std::stof(V);
			else if(K == "energyshare")
				gs_P.m_EnergyShare = std::stof(V);
			else if(K == "slack")
				gs_P.m_ShareSlack = std::stof(V);
			else if(K == "perparent")
				gs_P.m_PerParent = std::stoi(V);
			else if(K == "richkey")
				gs_P.m_RichKey = std::stoi(V);
			else if(K == "hookdedup")
				gs_P.m_HookDedup = std::stoi(V);
			else if(K == "jumpbonus")
				gs_P.m_JumpBonus = std::stof(V);
			else if(K == "upbonus")
				gs_P.m_HeightBonus = std::stof(V);
			else if(K == "progx")
				gs_P.m_ProgX = std::stoi(V);
			else if(K == "harmonic")
				gs_P.m_Harmonic = std::stoi(V);
			else if(K == "prenergy")
				gs_P.m_PrestartEnergy = std::stof(V);
			else if(K == "speedobj")
				gs_P.m_SpeedObj = std::stoi(V);
			else if(K == "stopd")
				gs_P.m_StopD = std::stof(V);
			else if(K == "gateslack")
				gs_P.m_GateSlack = std::stoi(V);
			else if(K == "stopx")
				gs_P.m_StopX = std::stof(V);
			else if(K == "prior")
				gs_P.m_Prior = V;
			else if(K == "priortop")
				gs_P.m_PriorTop = std::stoi(V);
			else if(K == "priorshots")
				gs_P.m_PriorShots = std::stoi(V);
			else if(K == "priorkappa")
				gs_P.m_PriorKappa = std::stof(V);
			else if(K == "speedfield")
				gs_P.m_SpeedField = V;
			else if(K == "speeddefault")
				gs_P.m_SpeedFieldDefault = std::stof(V);
			else if(K == "gatecands")
				gs_P.m_GateCands = std::stoi(V);
			else if(K == "gatelambda")
				gs_P.m_GateLambda = std::stof(V);
			else if(K == "ghostshift")
				gs_P.m_GhostShift = std::stoi(V);
			else if(K == "brutemin")
				gs_P.m_TargetMinTick = std::stoi(V);
			else if(K == "tmpgoal")
				std::sscanf(V.c_str(), "%f,%f", &gs_P.m_TmpGoalX, &gs_P.m_TmpGoalY);
			else if(K == "tpstarted")
				gs_P.m_TpStarted = std::stoi(V);
			else if(K == "prefixjumped")
				gs_P.m_PrefixJumped = std::stoi(V);
			else if(K == "stopregion")
			{
				gs_P.m_StopRegionOn = 1;
				std::sscanf(V.c_str(), "%f,%f,%f,%f", &gs_P.m_StopRegion[0], &gs_P.m_StopRegion[1], &gs_P.m_StopRegion[2], &gs_P.m_StopRegion[3]);
			}
			else if(K == "gatespeed")
				gs_P.m_GateSpeed = std::stoi(V);
			else if(K == "stopbox")
			{
				gs_P.m_StopBoxOn = 1;
				std::sscanf(V.c_str(), "%f,%f,%f,%f", &gs_P.m_StopBox[0], &gs_P.m_StopBox[1], &gs_P.m_StopBox[2], &gs_P.m_StopBox[3]);
			}
			else if(K == "speedgamma")
				gs_P.m_SpeedGamma = std::stof(V);
			else if(K == "stopxlt")
				gs_P.m_StopXLt = std::stof(V);
			else if(K == "jumpvymin")
				gs_P.m_JumpVyMin = std::stof(V);
			else if(K == "prevyw")
				gs_P.m_PreVyW = std::stof(V);
			else if(K == "preymin")
				gs_P.m_PreYMin = std::stof(V);
			else if(K == "prevymax")
				gs_P.m_PreVyMax = std::stof(V);
			else if(K == "preevalx")
				gs_P.m_PreEvalX = std::stof(V);
			else if(K == "preyw")
				gs_P.m_PreYW = std::stof(V);
			else if(K == "precands")
				gs_P.m_PreCands = std::stoi(V);
			else if(K == "prestartobj")
				gs_P.m_PrestartObj = std::stof(V);
			else if(K == "survive")
				gs_P.m_Survive = std::stoi(V);
			else if(K == "rollout")
				gs_P.m_Rollout = std::stoi(V);
			else if(K == "rdirs")
				gs_P.m_RolloutDirs = std::stoi(V);
			else if(K == "prefilter")
				gs_P.m_Prefilter = std::stoi(V);
			else if(K == "stopgren")
				gs_P.m_StopAtGrenade = std::stoi(V);
			else if(K == "out")
				gs_P.m_Out = V;
			else if(K == "commitfire")
				gs_P.m_CommitFire = atoi(V.c_str());
			else if(K == "bandshare")
				gs_P.m_BandShare = std::stof(V);
			else if(K == "bandlo")
				gs_P.m_BandLo = std::stof(V);
			else if(K == "bandhi")
				gs_P.m_BandHi = std::stof(V);
			else if(K == "crashpen")
				gs_P.m_CrashPen = std::stof(V);
			else if(K == "crashk")
				gs_P.m_CrashK = atoi(V.c_str());
			else if(K == "hookpen")
				gs_P.m_HookPen = std::stof(V);
			else if(K == "padaims")
				gs_P.m_PadAims = atoi(V.c_str());
			else if(K == "bendcredit")
				gs_P.m_BendCredit = std::stof(V);
			else if(K == "bendlen")
				gs_P.m_BendLen = std::stof(V);
			else if(K == "padopt")
				gs_P.m_PadOpt = std::stof(V);
			else if(K == "tpfirst")
				gs_P.m_TpFirst = atoi(V.c_str());
			else if(K == "padcredit")
				gs_P.m_PadCredit = std::stof(V);
			else if(K == "optval")
				gs_P.m_OptVal = atoi(V.c_str());
			else if(K == "kcredit")
				gs_P.m_KCredit = std::stof(V);
			else if(K == "kready")
				gs_P.m_KReady = atoi(V.c_str());
			else if(K == "clancap")
				gs_P.m_ClanCap = atoi(V.c_str());
			else if(K == "clanperiod")
				gs_P.m_ClanPeriod = atoi(V.c_str());
			else if(K == "gcredit")
				gs_P.m_GCredit = std::stof(V);
			else if(K == "tpgren")
				gs_P.m_TpGren = atoi(V.c_str());
			else if(K == "pendroll")
				gs_P.m_PendRoll = atoi(V.c_str());
			else if(K == "pendwin")
				gs_P.m_PendWin = std::stof(V);
			else if(K == "pendbonus")
				gs_P.m_PendBonus = std::stof(V);
			else if(K == "pendshare")
				gs_P.m_PendShare = std::stof(V);
			else if(K == "projkey")
				gs_P.m_ProjKey = atoi(V.c_str());
			else if(K == "firelookmax")
				gs_P.m_FireLookMax = atoi(V.c_str());
			else if(K == "nofirebefore")
				gs_P.m_NoFireBefore = atoi(V.c_str());
			else if(K == "prefix")
				gs_P.m_Prefix = V;
			else if(K == "prefixlen")
				gs_P.m_PrefixLen = std::stoi(V);
			else
			{
				std::printf("unknown option %s\n", K.c_str());
				return 1;
			}
		}
		const SMapInfo &M = CTasGame::Map();
		if(!gs_P.m_Prior.empty())
		{
			FILE *f = std::fopen(gs_P.m_Prior.c_str(), "r");
			bool Ok = f != nullptr;
			for(int b = 0; Ok && b < 3; b++)
				for(int k = 0; k < 24; k++)
					Ok &= std::fscanf(f, "%f", &gs_aHookPrior[b][k]) == 1;
			for(int b = 0; Ok && b < 3; b++)
				for(int k = 0; k < 24; k++)
					Ok &= std::fscanf(f, "%f", &gs_aShotPrior[b][k]) == 1;
			if(f)
				std::fclose(f);
			gs_HasPrior = Ok;
			std::printf("prior: %s\n", Ok ? "loaded" : "FAILED");
		}
		if(!gs_P.m_SpeedField.empty())
		{
			gs_vTileCost.assign(M.m_W * M.m_H, 1.0f);
			FILE *f = std::fopen(gs_P.m_SpeedField.c_str(), "r");
			int n = 0;
			for(int y = 0; f && y < M.m_H; y++)
				for(int x = 0; x < M.m_W; x++)
				{
					float v = 0;
					if(std::fscanf(f, "%f", &v) != 1)
						break;
					if(v > 0)
					{
						gs_vTileCost[y * M.m_W + x] = gs_P.m_Vref / v;
						n++;
					}
					else
						gs_vTileCost[y * M.m_W + x] = gs_P.m_Vref / gs_P.m_SpeedFieldDefault;
				}
			if(f)
				std::fclose(f);
			std::printf("speed field: %d tiles with data\n", n);
			gs_Dist.m_pTileCost = &gs_vTileCost;
			gs_DistGrenade.m_pTileCost = &gs_vTileCost;
		}
		gs_Dist.Build(M, {TILE_FINISH});
		if(gs_P.m_BendCredit > 0)
			BuildBend();
		// grenade pickup position
		// the grenade on the route is the one farthest from the finish
		float BestD = -1;
		for(int y = 0; y < M.m_H; y++)
			for(int x = 0; x < M.m_W; x++)
				if(M.Tile(x, y) == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
				{
					vec2 P(x * 32 + 16, y * 32 + 16);
					float D = gs_Dist.Sample(P);
					if(D < 1e8f && D > BestD)
					{
						BestD = D;
						gs_GrenadePos = P;
					}
				}
		gs_DistGrenade.BuildFromPoint(M, gs_GrenadePos);
		if(gs_P.m_TmpGoalX > 0)
			gs_DistGrenade.BuildFromPoint(M, vec2(gs_P.m_TmpGoalX, gs_P.m_TmpGoalY));
		if(gs_P.m_Harmonic)
		{
			SMapInfo Tmp = M;
			const uint8_t GOAL = 250;
			Tmp.m_vGame[(int)(gs_GrenadePos.y / 32) * Tmp.m_W + (int)(gs_GrenadePos.x / 32)] = GOAL;
			SDistField FromSpawn;
			FromSpawn.BuildFromPoint(M, M.m_vSpawns[0]);
			gs_HarmA.Build(Tmp, {M.m_vSpawns[0]}, 20.0f, {GOAL}, gs_DistGrenade, FromSpawn);
			gs_HarmB.Build(M, {gs_GrenadePos}, 20.0f, {TILE_FINISH}, gs_Dist, gs_DistGrenade);
			// match the fields on the start line
			float Sum = 0;
			int Num = 0;
			for(int y = 0; y < M.m_H; y++)
				if(M.Tile(29, y) == TILE_START)
				{
					vec2 P(29 * 32 + 16, y * 32 + 16);
					float A = gs_HarmA.Sample(P) + gs_Dist.Sample(gs_GrenadePos), B = gs_Dist.Sample(P);
					if(A < 1e8f && B < 1e8f)
					{
						Sum += A - B;
						Num++;
					}
				}
			gs_StartOffset = Num ? Sum / Num : 0;
			std::printf("start offset %.0f\n", gs_StartOffset);
		}
		std::printf("spawn D=%.0f, grenade at %.0f %.0f D=%.0f, spawn->grenade %.0f\n", gs_Dist.Sample(M.m_vSpawns[0]), gs_GrenadePos.x, gs_GrenadePos.y,
			gs_Dist.Sample(gs_GrenadePos), gs_DistGrenade.Sample(M.m_vSpawns[0]));
		if(getenv("TAS_DUMPFIELD"))
		{
			FILE *f = std::fopen(getenv("TAS_DUMPFIELD"), "w");
			for(int y = 0; y < M.m_H; y++)
			{
				for(int x = 0; x < M.m_W; x++)
				{
					vec2 P(x * 32 + 16, y * 32 + 16);
					std::fprintf(f, "%.0f %.0f %.0f %.0f ", std::min(gs_Dist.Sample(P), 1e6f), std::min(gs_DistGrenade.Sample(P), 1e6f),
						gs_P.m_Harmonic ? std::min(gs_HarmA.Sample(P), 1e6f) : 0.f, gs_P.m_Harmonic ? std::min(gs_HarmB.Sample(P), 1e6f) : 0.f);
				}
				std::fprintf(f, "\n");
			}
			std::fclose(f);
		}
		if(!gs_P.m_RefFile.empty())
			gs_Ref.Load(ReadInputs(gs_P.m_RefFile.c_str()));
		if(!gs_P.m_GhostFile.empty())
			gs_Ref.LoadGhost(gs_P.m_GhostFile.c_str(), gs_GrenadePos);
		std::vector<STasInput> vPrefix;
		if(!gs_P.m_Prefix.empty())
		{
			vPrefix = ReadInputs(gs_P.m_Prefix.c_str());
			if(gs_P.m_PrefixLen >= 0 && gs_P.m_PrefixLen < (int)vPrefix.size())
				vPrefix.resize(gs_P.m_PrefixLen);
		}
		if(gs_P.m_Brute > 0)
		{
			Brute(vPrefix, gs_P.m_Brute);
			return 0;
		}
		auto vBest = Search(vPrefix);
		std::printf("done, best %zu inputs\n", vBest.size());
		return 0;
	}
	return 1;
}
