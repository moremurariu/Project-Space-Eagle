// x_ds: deferred-shot beam search for the post-grenade part of the run (exact CFastG stepper).
//
// Ideas (vs seg):
//  - No speculative shots. A grenade's flight does not depend on the tee, so a shot can be decided when its explosion
//    happens: at every expansion the search looks back along the state's own history (up to `maxf` ticks) for ticks
//    with a free reload slot (25 ticks away from every other shot of the lineage, no hook press, no shot) from which an
//    exact aim makes the grenade hit a solid point right next to the tee in this very step ("retro" shots, also two at
//    once = stacked kicks). The shot is patched into the ancestor's input at output. Normal shots are only kept when
//    they explode in the same step (point-blank). So every kick is judged by its real effect, and the reload is never
//    wasted on approach kicks that get braked away: the grenade waits in the history until a kick is worth most.
//  - Progress is the geodesic distance to the finish (8 px grid, tee clearance), no reference line. The time model
//    is the lag behind the incumbent run at the same geodesic distance, with an energy credit; the beam is the union
//    of the best states under several energy weights (hedge) with per-cell dedup.
// usage: x_ds <map> inc=RUN cut=RT [key=value...]
//   inc: the incumbent run (inputs from spawn); cut: race tick to start searching from (its prefix is kept)
//   gate=finish | gate=G (geodesic px to the finish; the window ends there)  out=FILE
//   beam=3000 threads=4 maxf=95 retro=3 hookangles=32 fireangles=64 lam=0,0.004,0.01,0.02 incforce=1
//   brakew=W: a lineage's speed lost to the hook pull and to the direction key (vs pressing along vx) costs W ticks
//     per px/t on top of the energy credit (above 15 px/t a hook pull only applies if |v| does not grow, so a high-speed
//     hook only turns or brakes; NOTES "Why Teero is faster": we lose speed ~50% faster than he does between kicks)
//   hookref=FILE (Teero's hook timeline, teero/hooks/teero_hooks.csv) [htrack=teero_track.txt hrefw=0.5 hrefwin=2
//     hrefhard=0]: a state is matched to his track by position; hooking where he has no hook within hrefwin of his
//     ticks, or not hooking where he hooks throughout, costs hrefw ticks per tick (lineage total; hrefhard=1 drops
//     such children instead); where he hooks, aims at his anchor are added
//   beamdump=FILE: every kept state of every step (node, parent, pos, vel, hook, lag, retro kicks) for visualizing
//   rotfar=1: low-loss turning hooks at anchors up to the hook length (flight time included): per side the aim whose
//     pull at the grab is the strongest turn that does not raise |v|, and the farthest anchor turning >= 80% of that
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
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ------------------------------------------------------------------------------------------------ inputs
static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	if(!f)
		return v;
	int d, j, h, fi, tx, ty, w;
	char aLine[256];
	while(std::fgets(aLine, sizeof(aLine), f))
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
	std::fclose(f);
	return v;
}
static void WriteInputs(const char *pPath, const std::vector<STasInput> &v)
{
	FILE *f = std::fopen(pPath, "w");
	if(!f)
		return;
	for(const auto &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

// ------------------------------------------------------------------------------------------------ params
struct SPar
{
	std::string m_Inc, m_Prefix, m_Anc, m_Out = "runs/ds/out.txt", m_Gate = "finish";
	int m_Cut = 1030;
	int m_Beam = 3000, m_Threads = 4, m_MaxSteps = 2000;
	int m_MaxF = 95, m_Retro = 3, m_Retro2 = 1;
	int m_HookAngles = 32, m_FireAngles = 64, m_FireKeep = 6;
	std::vector<float> m_vLam = {0.0f, 0.004f, 0.01f, 0.02f};
	float m_OptW = 0; // energy credit per free shot slot in the lineage's last maxf ticks
	float m_BrakeW = 0; // ticks per px/t of speed lost to hook pull / direction braking (lineage total)
	int m_RotFar = 0; // far low-loss turning hook aims
	std::string m_HookRef, m_HTrack = "teero_track.txt";
	std::string m_BeamDump; // beamdump=FILE: every kept state of every step (for visualizing the search)
	float m_HRefW = 0.5f;
	int m_HRefWin = 2, m_HRefHard = 0;
	int m_IncForce = 1;
	int m_GateWait = 3;
	float m_CellPos = 4, m_CellVel = 0.5f;
	int m_Surv = 16; // survival check horizon (0 = off)
	float m_Over = 1.4f; // overselect factor before the survival check
	int m_Verbose = 1;
	int m_DirAll = 1;
	float m_KickMin = 0; // drop retro / fire kicks weaker than this
	float m_Jitter = 0;
	int m_Seed = 0;
	int m_Rot = 1;
	float m_CredW = 0;
	float m_KCred = 0;
	int m_KReady = 0;
	int m_Quota = 0;
	int m_RollH = 0;
	int m_ShH = 0, m_ShOff = 2; // shadow rollouts (lookahead by following the incumbent's inputs)
	int m_ShMix = 1; // 1: best of shadow and the cheap policies
	float m_LatPen = 0, m_LatDz = 32;
	float m_EGain = 0.004f;
	int m_GateSurv = 30;
	int m_CommitK = -1;
	std::string m_TRef;
	float m_TTrack = 0, m_TOff = -1e9f, m_TCap = 96, m_TTrackV = 2, m_TDecay = 1.0f, m_TLag = 0.05f;
	int m_TShift = 3, m_TSmooth = 2;
	float m_Boost = 0; // diagnostics: add this many px/t along the root velocity (a hypothetical extra kick)
	int m_NoFire0 = -1, m_NoFire1 = -1;
	int m_NoKick0 = -1, m_NoKick1 = -1; // no explosions at all in these race ticks (keeps the slots for later stacks) // no shots fired in these race ticks (shadow / point-blank); retro slots there stay usable
	float m_TrackFrac = 0, m_TrackV = 3, m_TrackLag = 2;
	int m_Shadow = 0; // add the incumbent's inputs at the matched progress point (this many, from the next one)
	int m_ShadowBack = 1;
	float m_RollPre = 3.0f, m_RollLam = 0.004f;
	float m_QPos = 64, m_QVel = 8;
	int m_CredH = 40;
};
static SPar gs_P;

// ------------------------------------------------------------------------------------------------ geodesic field
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

// ------------------------------------------------------------------------------------------------ reference profile
struct SRefProf
{
	std::vector<float> m_vG, m_vE; // per incumbent race tick (index = rt - m_Rt0): geodesic distance, energy
	int m_Rt0 = 0;
	std::vector<float> m_vEnv; // running min of g
	// fractional race tick at which the incumbent first gets to geodesic distance <= g
	float TimeAt(float g) const
	{
		if(m_vEnv.empty())
			return 0;
		if(g >= m_vEnv[0])
			return m_Rt0 - (g - m_vEnv[0]) / 30.0f;
		int lo = 0, hi = (int)m_vEnv.size() - 1;
		if(g < m_vEnv[hi])
			return m_Rt0 + hi + (m_vEnv[hi] - g) / 20.0f;
		while(hi - lo > 1)
		{
			int mid = (lo + hi) / 2;
			if(m_vEnv[mid] <= g)
				hi = mid;
			else
				lo = mid;
		}
		float a = m_vEnv[lo], b = m_vEnv[hi];
		float f = a > b ? (a - g) / (a - b) : 1.0f;
		return m_Rt0 + lo + f;
	}
	float EnergyAt(float g) const
	{
		float t = TimeAt(g) - m_Rt0;
		int i = std::clamp((int)std::floor(t), 0, (int)m_vE.size() - 1);
		return m_vE[i];
	}
};
static SRefProf gs_Ref;

// the incumbent's own path as the progress reference: progress K = fractional incumbent race tick of the closest
// point on its polyline (searched near a hint, backward moves penalized), so the incumbent's lag is exactly 0
struct SRefLine
{
	std::vector<vec2> m_vP, m_vV;
	std::vector<float> m_vE;
	int m_Rt0 = 0; // race tick of m_vP[0]
	vec2 VelAt(float K) const { return m_vV[std::clamp((int)std::lround(K - m_Rt0), 0, (int)m_vV.size() - 1)]; }
	// per step: progress moves at most a few incumbent ticks (Lo / Hi), wide only for a fresh start
	float Project(vec2 X, float Hint, float *pLat = nullptr, int Lo = 6, int Hi = 8) const
	{
		const int n = (int)m_vP.size();
		int h = (int)std::floor(Hint - m_Rt0);
		int i0 = std::clamp(h - Lo, 0, n - 2), i1 = std::clamp(h + Hi, 0, n - 2);
		float Best = 1e30f, BestK = Hint, BestD = 0;
		for(int i = i0; i <= i1; i++)
		{
			vec2 A = m_vP[i], B = m_vP[i + 1], AB = B - A;
			float L2 = dot(AB, AB);
			float u = L2 > 1e-6f ? std::clamp(dot(X - A, AB) / L2, 0.0f, 1.0f) : 0.0f;
			float d = distance(X, A + AB * u);
			float k = i + u;
			float Pen = (Lo > 50 || Hi > 50) ? 0.0f : (k < h ? (h - k) * 25.0f : std::max(0.0f, k - h - 4) * 25.0f);
			if(d + Pen < Best)
			{
				Best = d + Pen;
				BestK = m_Rt0 + k;
				BestD = d;
			}
		}
		if(pLat)
			*pLat = BestD;
		return BestK;
	}
	float EnergyAt(float K) const
	{
		int i = std::clamp((int)std::lround(K - m_Rt0), 0, (int)m_vE.size() - 1);
		return m_vE[i];
	}
	std::vector<float> m_vSinks; // incumbent race ticks of its speed minima (U-turn exits)
	// energy weight: energy just before a sink gets braked away; 1 far from it, down to m_SinkMin at the sink
	float m_SinkH = 0, m_SinkMin = 0.25f;
	float EWeight(float K) const
	{
		if(m_SinkH <= 0)
			return 1.0f;
		for(float Sk : m_vSinks)
			if(Sk >= K)
				return std::clamp((Sk - K) / m_SinkH, m_SinkMin, 1.0f);
		return 1.0f;
	}
	void FindSinks()
	{
		const int n = (int)m_vV.size();
		std::vector<float> Sp(n);
		for(int i = 0; i < n; i++)
			Sp[i] = length(m_vV[i]);
		for(int i = 15; i + 15 < n; i++)
		{
			bool Min = true;
			for(int j = i - 15; j <= i + 15 && Min; j++)
				Min = Sp[j] >= Sp[i];
			if(!Min)
				continue;
			float L = 0, R = 0;
			for(int j = std::max(0, i - 60); j < i; j++)
				L = std::max(L, Sp[j]);
			for(int j = i + 1; j < std::min(n, i + 61); j++)
				R = std::max(R, Sp[j]);
			if(std::min(L, R) - Sp[i] > 8 && (m_vSinks.empty() || m_Rt0 + i - m_vSinks.back() > 30))
				m_vSinks.push_back((float)(m_Rt0 + i));
		}
	}
};
static SRefLine gs_Line;
static SRefLine gs_TL; // Teero's track (hookref)
static std::vector<char> gs_vHkS; // his hook state per track tick (index k - gs_HkK0): 'G', 'F', '-'
static std::vector<vec2> gs_vHkA; // anchor of 'G' ticks
static int gs_HkK0 = 0;
static char HkAt(int k) { int i = k - gs_HkK0; return i >= 0 && i < (int)gs_vHkS.size() ? gs_vHkS[i] : 0; }
static SRefLine gs_IncLine; // the incumbent's own path (shadow inputs are indexed on it)
static bool gs_TRef = false;

// ------------------------------------------------------------------------------------------------ search state
enum
{
	MAXFIRE = 8,
};
struct SState
{
	CFastG m_G;
	int m_Node = -1; // node index in the history of its step (-1: root)
	int m_NFire = 0;
	int m_aFire[MAXFIRE]; // fire ticks (projectile start ticks) of the lineage, newest first
	STasInput m_Prev; // last input
	bool m_Inc = false; // on the incumbent's own path
	float m_G0 = 0; // progress on the reference line
	float m_KInc = 0; // progress on the incumbent's path (shadow index)
	float m_TCost = 0; // time-indexed tracking cost (ttrack)
	float m_Lag = 0, m_E = 0;
	float m_Brake = 0; // speed lost to hook pull / direction braking along the lineage (px/t)
	float m_TK = 0; // Teero's track tick at this position (hookref)
	float m_HDis = 0; // ticks of hook disagreement with Teero along the lineage
};

struct SRetroShot
{
	int m_Tau = -1; // fire tick (ancestor state tick)
	int m_Step = -1; // history step index of the patched input
	int16_t m_TX = 0, m_TY = 0;
	vec2 m_P0, m_Dir;
	float m_Val = 0;
	vec2 m_Col;
	vec2 m_Kick;
};

struct SNode
{
	int m_Parent; // node index in the previous step (-1: root)
	STasInput m_In;
	vec2 m_Pos; // m_Pos of the state after the step (a shot in the next step starts here)
	uint8_t m_HookPress; // the input of this step is a fresh hook press
	int8_t m_NPatch = 0;
	int m_aPatchStep[2];
	int16_t m_aPTX[2], m_aPTY[2];
};

static std::vector<std::vector<SNode>> gs_vHist; // per step
static long gs_NPre = 0, gs_NSel = 0, gs_NSurv = 0;
static std::atomic<long> gs_NStackTry{0}, gs_NStackEmit{0}, gs_NCanPB{0}, gs_NOldR{0}, gs_NPBAims{0};
static std::atomic<long> gs_NRetroFound{0}, gs_NRetroEmit{0}, gs_NFireEmit{0}, gs_NFireTry{0};
static int gs_T0 = 0; // tick of the root state
static vec2 gs_RootPos;
static int gs_RootRt = 0;
static int gs_IncStart = 0; // incumbent start tick: its input index for the step from race tick R is R + gs_IncStart

struct SCand
{
	int m_Parent;
	STasInput m_In;
	int m_NR = 0;
	SRetroShot m_aR[2];
	float m_G; // geodesic distance after the step
	float m_Lag; // ticks behind the incumbent (lower better)
	float m_E;
	float m_Opt;
	float m_Jit = 0;
	float m_Track = 0; // distance to the incumbent's state at the same progress
	int m_G0Rt = 0; // race tick after the step
	float m_KInc = 0;
	float m_TCost = 0;
	float m_Brake = 0;
	float m_TK = 0, m_HDis = 0;
	uint64_t m_Key;
	int64_t m_Cell;
	int64_t m_QCell;
	bool m_Inc = false;
	bool m_Gate = false;
	float m_GateT = 0;
};

static inline float Energy(const CFastG &G) { return dot(G.m_Core.m_Vel, G.m_Core.m_Vel) - G.m_Core.m_Pos.y; }

static uint64_t StateKey(const CFastG &G)
{
	uint64_t h = 1469598103934665603ull;
	auto Mix = [&](const void *p, size_t n) {
		const unsigned char *b = (const unsigned char *)p;
		for(size_t i = 0; i < n; i++)
			h = (h ^ b[i]) * 1099511628211ull;
	};
	const CCharacterCore &C = G.m_Core;
	Mix(&C.m_Pos, sizeof(C.m_Pos));
	Mix(&C.m_Vel, sizeof(C.m_Vel));
	Mix(&C.m_HookState, sizeof(C.m_HookState));
	Mix(&C.m_HookPos, sizeof(C.m_HookPos));
	Mix(&C.m_Jumped, sizeof(C.m_Jumped));
	Mix(&G.m_ReloadTimer, sizeof(int));
	Mix(&G.m_NumProj, sizeof(int));
	return h;
}


// reference line from a position track ("k x y" per race tick, k = race tick + shift): positions smoothed, velocities
// recovered from the per-tick displacement by inverting the horizontal speed ramp, energy = v^2 - y
static bool LoadTrackLine(const char *pPath, int Shift, int Smooth, SRefLine &L)
{
	FILE *f = std::fopen(pPath, "r");
	if(!f)
		return false;
	std::vector<int> vK;
	std::vector<vec2> vP;
	int k;
	float x, y;
	char aLine[256];
	while(std::fgets(aLine, sizeof(aLine), f))
		if(std::sscanf(aLine, "%d %f %f", &k, &x, &y) == 3)
		{
			if(!vK.empty() && k != vK.back() + 1)
				continue;
			vK.push_back(k);
			vP.emplace_back(x, y);
		}
	std::fclose(f);
	if(vP.size() < 10)
		return false;
	const int n = (int)vP.size();
	std::vector<vec2> vS(n);
	for(int i = 0; i < n; i++)
	{
		vec2 Sum(0, 0);
		int c = 0;
		for(int j = std::max(0, i - Smooth); j <= std::min(n - 1, i + Smooth); j++)
		{
			Sum += vP[j];
			c++;
		}
		vS[i] = Sum / (float)c;
	}
	auto Ramp = [](float v) { return v * 50 < 550 ? 1.0f : 1.0f / std::pow(1.4f, (v * 50 - 550) / 2000.0f); };
	L.m_vP = vS;
	L.m_vV.assign(n, vec2(0, 0));
	L.m_vE.assign(n, 0);
	for(int i = 0; i < n; i++)
	{
		vec2 D = (vS[std::min(n - 1, i + 1)] - vS[std::max(0, i - 1)]) / (float)(std::min(n - 1, i + 1) - std::max(0, i - 1));
		// vx with vx * ramp(|(vx, vy)|) = dx (monotone below ~119 px/t)
		float vy = D.y, lo = 0, hi = 119;
		float ax = std::fabs(D.x);
		for(int it = 0; it < 40; it++)
		{
			float m = 0.5f * (lo + hi);
			if(m * Ramp(std::sqrt(m * m + vy * vy)) < ax)
				lo = m;
			else
				hi = m;
		}
		vec2 V(D.x < 0 ? -lo : lo, vy);
		L.m_vV[i] = V;
		L.m_vE[i] = dot(V, V) - vS[i].y;
	}
	L.m_Rt0 = vK[0] - Shift;
	return true;
}

// reload slot availability: a shot at tick Tau is allowed if it is >= 25 ticks away from every shot of the lineage
static bool SlotFree(const SState &S, int Tau)
{
	for(int i = 0; i < S.m_NFire; i++)
		if(std::abs(Tau - S.m_aFire[i]) < 25)
			return false;
	return true;
}
static void AddFire(SState &S, int Tau)
{
	int n = std::min(S.m_NFire + 1, (int)MAXFIRE);
	for(int i = n - 1; i > 0; i--)
		S.m_aFire[i] = S.m_aFire[i - 1];
	S.m_aFire[0] = Tau;
	S.m_NFire = n;
	std::sort(S.m_aFire, S.m_aFire + S.m_NFire, [](int a, int b) { return a > b; });
}
static int ReloadFrom(const SState &S, int Tick)
{
	int R = 0;
	for(int i = 0; i < S.m_NFire; i++)
		R = std::max(R, 25 - (Tick - S.m_aFire[i]));
	return std::max(0, R);
}
// free shot slots in (Tick - maxf, Tick] that could still be used (greedy count)
static int FreeSlots(const SState &S, int Tick)
{
	int n = 0, Last = -1000000;
	int Lo = Tick - gs_P.m_MaxF + 1;
	std::vector<int> v(S.m_aFire, S.m_aFire + S.m_NFire);
	std::sort(v.begin(), v.end());
	int k = 0;
	for(int t = Lo; t <= Tick; t++)
	{
		while(k < (int)v.size() && v[k] + 25 <= t)
			k++;
		bool Ok = t - Last >= 25;
		for(int f : v)
			if(std::abs(t - f) < 25)
				Ok = false;
		if(Ok)
		{
			n++;
			Last = t;
		}
	}
	return n;
}

// option value of unused shot slots: newest-first greedy packing of free slots in (Tick - H, Tick], each worth a good
// kick at the current speed, fading linearly with its age (old slots are rarely reachable)
static float SlotCredit(const int *pFire, int NFire, int Tick, float Speed)
{
	if(gs_P.m_CredW <= 0)
		return 0;
	const int H = gs_P.m_CredH;
	float Kick = 24.0f * std::max(Speed, 10.0f) * 0.8f + 144.0f;
	float C = 0;
	int Last = 1 << 30;
	for(int Tau = Tick; Tau > Tick - H; Tau--)
	{
		if(Last - Tau < 25)
			continue;
		bool Ok = true;
		for(int i = 0; i < NFire; i++)
			if(std::abs(Tau - pFire[i]) < 25)
			{
				Ok = false;
				break;
			}
		if(!Ok)
			continue;
		C += Kick * (1.0f - (float)(Tick - Tau) / H);
		Last = Tau;
	}
	return C * gs_P.m_CredW;
}

// ancestor lookup: node of the state at tick T (T > gs_T0) along the lineage of node (Step, Idx)
static inline const SNode &NodeAt(int Step, int Idx) { return gs_vHist[Step][Idx]; }

// ------------------------------------------------------------------------------------------------ actions
static void HookAims(const CFastG &G, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.m_Pos;
	const float Range = 380.0f + 3.0f * length(G.m_Core.m_Vel);
	std::vector<int> vSeen;
	for(int a = 0; a < gs_P.m_HookAngles; a++)
	{
		float Ang = 2 * pi * (a + 0.5f) / gs_P.m_HookAngles;
		vec2 D(std::cos(Ang), std::sin(Ang));
		int Hit = -1;
		for(float r = 42.0f; r < Range; r += 4.0f)
		{
			vec2 Q = P + D * r;
			int tx = (int)std::floor(Q.x / 32), ty = (int)std::floor(Q.y / 32);
			int T = M.Tile(tx, ty);
			if(T == TILE_SOLID)
			{
				Hit = ty * M.m_W + tx;
				break;
			}
			if(T == TILE_NOHOOK)
				break;
		}
		if(Hit < 0 || std::find(vSeen.begin(), vSeen.end(), Hit) != vSeen.end())
			continue;
		vSeen.push_back(Hit);
		vOut.push_back({(int16_t)std::lround(D.x * 10000), (int16_t)std::lround(D.y * 10000)});
	}
}

// rotation pulses (as seg's RotHookAims, both turning directions): a hook fired at a solid tile 47..122 px away grabs
// and pulls in the same tick along the aim; above 15 px/t the pull only applies if |v| does not grow, so the best
// aims sit on that boundary (lossless turning). Per side: the boundary aim and 1 / 3 degrees inside it.
static vec2 PreHookVel(const CFastG &G, int Dir, int Jump)
{
	vec2 V = G.m_Core.m_Vel;
	V.y += 0.5f;
	const bool Gr = CTasGame::Collision()->IsOnGround(G.m_Pos, 28.0f);
	if(Jump)
	{
		if(Gr)
			V.y = -13.2f;
		else if(!(G.m_Core.m_Jumped & 2))
			V.y = -12.0f;
	}
	const float Acc = Gr ? 2.0f : 1.5f, Max = Gr ? 10.0f : 5.0f;
	if(Dir == 0)
		V.x *= Gr ? 0.5f : 0.95f;
	else if(Dir > 0 && V.x <= Max)
		V.x = std::min(V.x + Acc, Max);
	else if(Dir < 0 && V.x >= -Max)
		V.x = std::max(V.x - Acc, -Max);
	return V;
}
static vec2 HookPull(vec2 To, int Dir)
{
	vec2 H = normalize(To) * 3.0f;
	if(H.y > 0)
		H.y *= 0.3f;
	H.x *= ((H.x < 0 && Dir < 0) || (H.x > 0 && Dir > 0)) ? 0.95f : 0.75f;
	return H;
}
static void RotAims(const CFastG &G, int Dir, int Jump, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	const SMapInfo &M = CTasGame::Map();
	const vec2 P = G.m_Pos;
	if(!CTasGame::Collision()->FastAnySolid(P.x - 124, P.y - 124, P.x + 124, P.y + 124))
		return;
	const vec2 V = PreHookVel(G, Dir, Jump);
	const float L0 = length(V);
	if(L0 < 15.0f)
		return;
	static const std::vector<vec2> s_vDir = [] {
		std::vector<vec2> v;
		for(int i = 0; i < 1440; i++)
		{
			float Ang = i * pi / 720.0f;
			v.emplace_back(std::cos(Ang), std::sin(Ang));
		}
		return v;
	}();
	auto DirOf = [&](int i) { return s_vDir[((i % 1440) + 1440) % 1440]; };
	auto Reach = [&](int i) {
		vec2 D = DirOf(i);
		for(float r = 42.0f; r <= 122.0f; r += 1.0f)
		{
			vec2 Q = P + D * r;
			int T = M.Tile((int)std::floor(Q.x / 32), (int)std::floor(Q.y / 32));
			if(T == TILE_SOLID)
				return r > 47.0f;
			if(T == TILE_NOHOOK)
				return false;
		}
		return false;
	};
	auto NewV = [&](int i) { return V + HookPull(DirOf(i), Dir); };
	auto Ok = [&](int i) { float Ln = length(NewV(i)); return Ln < L0 - 0.004f; };
	for(int Side = -1; Side <= 1; Side += 2)
	{
		float Best = 0.0f;
		int BestI = -1;
		for(int i = 0; i < 1440; i += 2)
		{
			if(!Ok(i))
				continue;
			vec2 Nv = NewV(i);
			float Rot = Side * (V.x * Nv.y - V.y * Nv.x) / (L0 * length(Nv));
			if(Rot <= Best || !Reach(i))
				continue;
			Best = Rot;
			BestI = i;
		}
		if(BestI < 0)
			continue;
		for(int k = 0; k < 3; k++)
		{
			int i = BestI;
			if(k)
			{
				// 1 / 3 degrees towards less rotation
				int Off = k == 1 ? 4 : 12;
				int i1 = BestI + Off, i2 = BestI - Off;
				vec2 N1 = NewV(i1), N2 = NewV(i2);
				float R1 = Side * (V.x * N1.y - V.y * N1.x), R2 = Side * (V.x * N2.y - V.y * N2.x);
				bool Ok1 = Ok(i1) && R1 > 0 && Reach(i1), Ok2 = Ok(i2) && R2 > 0 && Reach(i2);
				if(Ok1 && (!Ok2 || length(N1) > length(N2)))
					i = i1;
				else if(Ok2)
					i = i2;
				else
					continue;
			}
			vec2 D = DirOf(i);
			std::pair<int16_t, int16_t> A{(int16_t)std::lround(D.x * 10000), (int16_t)std::lround(D.y * 10000)};
			if(std::find(vOut.begin(), vOut.end(), A) == vOut.end())
				vOut.push_back(A);
		}
	}
}

// far low-loss turning hooks: the hook flies 80 px/t from 42 px out, so an anchor r px away grabs in tick
// ceil((r - 42) / 80) and pulls from where the tee is then (an idle prediction of the next ticks). At the grab the pull
// should turn v as much as possible without raising |v| (it would not apply), as RotAims does for the same-tick grabs;
// a far anchor also drifts slower, so the farthest anchor turning >= 80% of the best is added too.
static void FarRays(const CFastG &G, std::vector<float> &vR)
{
	static const int N = 720;
	vR.assign(N, 0.0f);
	const SMapInfo &M = CTasGame::Map();
	const vec2 P = G.m_Pos;
	if(!CTasGame::Collision()->FastAnySolid(P.x - 380, P.y - 380, P.x + 380, P.y + 380))
		return;
	for(int i = 0; i < N; i++)
	{
		const float Ang = 2 * pi * i / N;
		const vec2 D(std::cos(Ang), std::sin(Ang));
		for(float r = 42.0f; r <= 362.0f; r += 3.0f)
		{
			vec2 Q = P + D * r;
			int T = M.Tile((int)std::floor(Q.x / 32), (int)std::floor(Q.y / 32));
			if(T == TILE_SOLID)
			{
				vR[i] = r;
				break;
			}
			if(T == TILE_NOHOOK)
				break;
		}
	}
}
static void RotFarAims(const CFastG &G, int Dir, const std::vector<float> &vR, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	if(length(G.m_Core.m_Vel) < 15.0f)
		return;
	const int N = (int)vR.size();
	// idle prediction: position / velocity at the start of ticks 1..4 (no hook, the same direction key)
	vec2 aP[5], aV[5];
	{
		CFastG F = G;
		STasInput In{};
		In.m_Dir = Dir;
		In.m_TX = 1000;
		In.m_Weapon = -1;
		aP[0] = F.m_Pos;
		aV[0] = F.m_Core.m_Vel;
		for(int k = 1; k < 5; k++)
		{
			F.Step(In);
			aP[k] = F.m_Pos;
			aV[k] = F.m_Core.m_Vel;
		}
	}
	for(int Side = -1; Side <= 1; Side += 2)
	{
		float Best = 0.0f, BestR = 0;
		int BestI = -1;
		std::vector<std::pair<float, int>> vOk; // (rotation, index)
		for(int i = 0; i < N; i++)
		{
			const float R = vR[i];
			if(R <= 122.0f) // same-tick grabs are RotAims'
				continue;
			const int k = std::min(4, (int)std::ceil((R - 42.0f) / 80.0f));
			const float Ang = 2 * pi * i / N;
			const vec2 A = G.m_Pos + vec2(std::cos(Ang), std::sin(Ang)) * R; // anchor (fired from the current position)
			const vec2 P = aP[k - 1];
			if(distance(P, A) > 380.0f || distance(P, A) < 46.0f)
				continue;
			vec2 V = aV[k - 1];
			V.y += 0.5f;
			const float L0 = length(V);
			if(L0 < 15.0f)
				continue;
			const vec2 Nv = V + HookPull(A - P, Dir);
			const float Ln = length(Nv);
			if(!(Ln < L0 - 0.004f))
				continue;
			const float Rot = Side * (V.x * Nv.y - V.y * Nv.x) / (L0 * Ln);
			if(Rot <= 0)
				continue;
			vOk.push_back({Rot, i});
			if(Rot > Best)
			{
				Best = Rot;
				BestI = i;
				BestR = R;
			}
		}
		if(BestI < 0)
			continue;
		int FarI = -1;
		float FarR = BestR;
		for(auto [Rot, i] : vOk)
			if(Rot >= 0.8f * Best && vR[i] > FarR + 32.0f)
			{
				FarR = vR[i];
				FarI = i;
			}
		for(int i : {BestI, FarI})
		{
			if(i < 0)
				continue;
			const float Ang = 2 * pi * i / N;
			std::pair<int16_t, int16_t> Aim{(int16_t)std::lround(std::cos(Ang) * 10000), (int16_t)std::lround(std::sin(Ang) * 10000)};
			if(std::find(vOut.begin(), vOut.end(), Aim) == vOut.end())
				vOut.push_back(Aim);
		}
	}
}

// speed lost in the step G -> (input In) to the direction key (vs pressing along vx) and to the hook pull (measured
// before the move; collisions, explosions and gravity are not counted)
static float BrakeLoss(const CFastG &G, const STasInput &In, const CFastG &After)
{
	const vec2 Va = PreHookVel(G, In.m_Dir, In.m_Jump);
	float Loss = 0;
	const float Vx = G.m_Core.m_Vel.x;
	if(std::fabs(Vx) > 0.5f)
	{
		const int Along = Vx > 0 ? 1 : -1;
		if(In.m_Dir != Along)
			Loss += std::max(0.0f, length(PreHookVel(G, Along, In.m_Jump)) - length(Va));
	}
	if(In.m_Hook && After.m_Core.m_HookState == HOOK_GRABBED && distance(After.m_Core.m_HookPos, G.m_Pos) > 46.0f)
	{
		const vec2 Nv = Va + HookPull(After.m_Core.m_HookPos - G.m_Pos, In.m_Dir);
		const float L0 = length(Va), Ln = length(Nv);
		if(Ln < 15.0f || Ln < L0)
			Loss += std::max(0.0f, L0 - Ln);
	}
	return Loss;
}

// ------------------------------------------------------------------------------------------------ survival
static bool SegFreeze(vec2 A, vec2 B)
{
	const SMapInfo &M = CTasGame::Map();
	float L = distance(A, B);
	int N = (int)(L / 4.0f) + 1;
	for(int i = 1; i <= N; i++)
	{
		vec2 P = mix(A, B, (float)i / N);
		int T = M.Tile((int)P.x / 32, (int)P.y / 32);
		if(T == TILE_FREEZE)
			return true;
	}
	return false;
}
// can the tee avoid freeze for N ticks? (core-only rollouts: dir x keep/release hook, then fresh hook presses)
static bool Survives(const CFastG &G, const STasInput &Prev, int N)
{
	if(G.m_Dead)
		return false;
	auto Roll = [&](int Dir, int Hook, int TX, int TY) {
		CCharacterCore Core = G.m_Core;
		Core.SetCoreWorld(nullptr, CTasGame::Collision(), &CFastG::ms_Teams);
		CNetObj_PlayerInput In = G.m_Input;
		In.m_Direction = Dir;
		In.m_Jump = 0;
		In.m_Fire = G.m_Input.m_Fire;
		bool Fresh = Hook && !Prev.m_Hook;
		if(Fresh)
		{
			In.m_TargetX = TX;
			In.m_TargetY = TY;
		}
		for(int t = 0; t < N; t++)
		{
			In.m_Hook = Hook;
			Core.m_Input = In;
			vec2 P0 = Core.m_Pos;
			Core.Tick(true);
			Core.Move();
			Core.Quantize();
			if(SegFreeze(P0, Core.m_Pos))
				return false;
		}
		return true;
	};
	for(int d : {(int)Prev.m_Dir, 1, 0, -1})
	{
		if(Roll(d, Prev.m_Hook, Prev.m_TX, Prev.m_TY))
			return true;
		if(Prev.m_Hook && Roll(d, 0, 0, -1))
			return true;
	}
	if(!Prev.m_Hook)
	{
		static thread_local std::vector<std::pair<int16_t, int16_t>> s_vH;
		HookAims(G, s_vH);
		for(auto [TX, TY] : s_vH)
			for(int d : {1, -1, 0})
				if(Roll(d, 1, TX, TY))
					return true;
	}
	return false;
}

// point-blank aims: a solid point within ~46 px along the ray (the grenade explodes in the same step)
static void FireAims(const CFastG &G, vec2 RouteDir, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.m_Pos;
	struct SA
	{
		float m_V;
		int16_t m_X, m_Y;
	};
	std::vector<SA> v;
	vec2 V = G.m_Core.m_Vel;
	for(int a = 0; a < gs_P.m_FireAngles; a++)
	{
		float Ang = 2 * pi * a / gs_P.m_FireAngles;
		vec2 D(std::cos(Ang), std::sin(Ang));
		bool Hit = false;
		for(float r = 4.0f; r <= 47.0f; r += 3.0f)
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
		vec2 K = -D * 12.0f;
		float Val = dot(K, RouteDir) * 0.5f + (length(V + K) - length(V));
		v.push_back({Val, (int16_t)std::lround(D.x * 10000), (int16_t)std::lround(D.y * 10000)});
	}
	std::sort(v.begin(), v.end(), [](const SA &a, const SA &b) { return a.m_V > b.m_V; });
	for(int i = 0; i < (int)v.size() && i < gs_P.m_FireKeep; i++)
		vOut.push_back({v[i].m_X, v[i].m_Y});
}

// ------------------------------------------------------------------------------------------------ retro shots
// For state S (tick t = S.m_G.m_Tick, history node (Step, Idx) for tick t), find shots fired from ancestors at
// ticks Tau in [t - maxf, t - 1] that explode in the next step next to the tee.
static void RetroFind(const SState &S, int Step, vec2 RouteDir, std::vector<SRetroShot> &vOut, int ExcludeTau = -100000)
{
	vOut.clear();
	const CFastG &G = S.m_G;
	if(G.m_NumProj >= CFastG::MAX_PROJ)
		return;
	const SMapInfo &M = CTasGame::Map();
	const vec2 Q = G.m_Pos, V = G.m_Core.m_Vel;
	const int t = G.m_Tick;
	// solid points around the tee
	vec2 aE[32];
	int NE = 0;
	for(int a = 0; a < 32; a++)
	{
		const float Ang = 2 * pi * a / 32;
		const vec2 D(std::cos(Ang), std::sin(Ang));
		for(float r = 6.0f; r < 100.0f; r += 3.0f)
		{
			vec2 P = Q + D * r;
			if(M.Tile((int)std::floor(P.x / 32), (int)std::floor(P.y / 32)) == TILE_SOLID)
			{
				aE[NE++] = P;
				break;
			}
		}
	}
	if(!NE)
		return;
	const float Curv = G.m_Core.m_Tuning.m_GrenadeCurvature, Speed = G.m_Core.m_Tuning.m_GrenadeSpeed;
	const float Vg = Speed / SERVER_TICK_SPEED, Cg = Curv / 10000.0f * Vg * Vg, R0 = CCharacterCore::PhysicalSize() * 0.75f;
	const float Strength = G.m_Core.m_Tuning.m_ExplosionStrength;
	static thread_local std::vector<SRetroShot> s_vC;
	s_vC.clear();
	// walk back the lineage: node at tick T is the input of the step T-1 -> T; the ancestor state at tick Tau = T - 1
	int St = Step, Ix = S.m_Node;
	for(int T = t; T > gs_T0 && t - (T - 1) <= gs_P.m_MaxF; T--)
	{
		if(St < 0 || Ix < 0)
			break;
		const SNode &N = gs_vHist[St][Ix];
		const int Tau = T - 1;
		const bool InputOk = !N.m_In.m_Fire && !N.m_HookPress;
		vec2 APos = N.m_Parent >= 0 ? gs_vHist[St - 1][N.m_Parent].m_Pos : gs_RootPos;
		const int TauT = t + 1 - Tau; // flight ticks until the explosion step
		if(InputOk && Tau != ExcludeTau && SlotFree(S, Tau) && TauT <= 99)
		{
			for(int e = 0; e < NE; e++)
			{
				const vec2 W = aE[e] - APos;
				auto F = [&](float Tau2) { return length(vec2(W.x, W.y - Cg * Tau2 * Tau2)) - R0 - Vg * Tau2; };
				float Lo = TauT - 1.0f, Hi = (float)TauT;
				if(!(F(Lo) > 0 && F(Hi) <= 0))
					continue;
				for(int it = 0; it < 22; it++)
				{
					float Mid = 0.5f * (Lo + Hi);
					if(F(Mid) > 0)
						Lo = Mid;
					else
						Hi = Mid;
				}
				vec2 Dn = normalize(vec2(W.x, W.y - Cg * Hi * Hi));
				int16_t TX = (int16_t)std::lround(Dn.x * 10000), TY = (int16_t)std::lround(Dn.y * 10000);
				if(!TX && !TY)
					TY = -1;
				const vec2 Dir = normalize(vec2(TX, TY));
				const vec2 P0 = APos + Dir * R0;
				bool Ok = false;
				vec2 Col;
				for(int Tau2 = 1; Tau2 <= TauT; Tau2++)
				{
					vec2 Prev = CalcPos(P0, Dir, Curv, Speed, (Tau2 - 1) / (float)SERVER_TICK_SPEED);
					vec2 Cur = CalcPos(P0, Dir, Curv, Speed, Tau2 / (float)SERVER_TICK_SPEED);
					if(Cur.x < 0 || Cur.y < 0 || Cur.x >= M.m_W * 32 || Cur.y >= M.m_H * 32)
						break;
					vec2 NewPos;
					int Collide = CTasGame::Collision()->IntersectLine(Prev, Cur, &Col, &NewPos);
					if(Collide)
					{
						Ok = Tau2 == TauT;
						break;
					}
				}
				if(!Ok)
					continue;
				if(!(distance(Q, Col) < 135.0f + CCharacterCore::PhysicalSize()))
					continue;
				vec2 Diff = Q - Col;
				float l = length(Diff);
				vec2 Fd = l ? normalize(Diff) : vec2(0, 1);
				l = 1 - std::clamp((l - 48.0f) / (135.0f - 48.0f), 0.0f, 1.0f);
				float Dmg = Strength * l;
				if(!(int)Dmg)
					continue;
				vec2 Fk = Fd * Dmg * 2;
				if(length(Fk) < gs_P.m_KickMin)
					continue;
				float Val = dot(Fk, RouteDir) * 0.5f + (length(V + Fk) - length(V));
				bool Dup = false;
				for(auto &C : s_vC)
					if(distance(C.m_Col, Col) < 6.0f && C.m_Tau == Tau)
					{
						Dup = true;
						break;
					}
				if(Dup)
					continue;
				SRetroShot R;
				R.m_Tau = Tau;
				R.m_Step = St;
				R.m_TX = TX;
				R.m_TY = TY;
				R.m_P0 = P0;
				R.m_Dir = Dir;
				R.m_Val = Val;
				R.m_Col = Col;
				R.m_Kick = Fk;
				s_vC.push_back(R);
			}
		}
		Ix = N.m_Parent;
		St--;
	}
	std::sort(s_vC.begin(), s_vC.end(), [](const SRetroShot &a, const SRetroShot &b) { return a.m_Val > b.m_Val; });
	// keep the best recent ones and the best old ones (25+ ticks: they can stack with a point-blank shot now)
	int NRecent = 0, NOld = 0;
	for(int k = 0; k < (int)s_vC.size(); k++)
	{
		bool Old = t - s_vC[k].m_Tau >= 25;
		if(Old ? NOld < gs_P.m_Retro : NRecent < gs_P.m_Retro)
		{
			vOut.push_back(s_vC[k]);
			(Old ? NOld : NRecent)++;
		}
	}
}

static void ApplyRetro(SState &S, const SRetroShot &R)
{
	CFastG &G = S.m_G;
	int At = 0;
	while(At < G.m_NumProj && G.m_aProj[At].m_StartTick > R.m_Tau)
		At++;
	for(int k = G.m_NumProj; k > At; k--)
		G.m_aProj[k] = G.m_aProj[k - 1];
	G.m_NumProj++;
	SFastProj &P = G.m_aProj[At];
	P.m_Pos = R.m_P0;
	P.m_Dir = R.m_Dir;
	P.m_StartTick = R.m_Tau;
	P.m_LifeSpan = 100 - (G.m_Tick - R.m_Tau);
	AddFire(S, R.m_Tau);
	G.m_ReloadTimer = ReloadFrom(S, G.m_Tick);
	// the patched input pressed fire at tick Tau+1 and released it at Tau+2 (if Tau+2 <= now)
	int Add = (R.m_Tau + 2 <= G.m_Tick) ? 2 : 1;
	G.m_Fire += Add;
	G.m_Input.m_Fire += Add;
	G.m_LatestInput.m_Fire += Add;
	G.m_LatestPrevInput.m_Fire += Add;
	G.m_Core.m_Input.m_Fire += Add;
	if(Add == 1)
		S.m_Prev.m_Fire = 1;
}

// ------------------------------------------------------------------------------------------------ scoring
// energy-equivalent value of the best point-blank kick available right now (loaded grenade, solid within ~45 px)
static float KickCredit(const CFastG &G)
{
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.m_Pos, V = G.m_Core.m_Vel;
	if(!CTasGame::Collision()->FastAnySolid(P.x - 48, P.y - 48, P.x + 48, P.y + 48))
		return 0;
	vec2 Rd = gs_Geo.Dir(P);
	float L = std::max(length(V), 10.0f);
	float Best = 0;
	for(int a = 0; a < 24; a++)
	{
		float Ang = 2 * pi * a / 24;
		vec2 D(std::cos(Ang), std::sin(Ang));
		bool Hit = false;
		for(float r = 6.0f; r <= 44.0f; r += 4.0f)
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
		float c = dot(-D, Rd);
		Best = std::max(Best, 24.0f * L * c + 144.0f);
	}
	return Best;
}

static void Score(const CFastG &G, SCand &C, const SState &Par)
{
	float Lat = 0;
	C.m_G = gs_Line.Project(G.m_Core.m_Pos, Par.m_G0, &Lat);
	C.m_Track = Lat + gs_P.m_TrackV * distance(G.m_Core.m_Vel, gs_Line.VelAt(C.m_G));
	C.m_KInc = gs_TRef ? gs_IncLine.Project(G.m_Core.m_Pos, Par.m_KInc) : C.m_G;
	C.m_G0Rt = G.RaceTick();
	float Rt = (float)G.RaceTick();
	C.m_Lag = Rt - C.m_G + gs_P.m_LatPen * std::max(0.0f, Lat - gs_P.m_LatDz);
	C.m_E = Energy(G);
	C.m_Opt = 0;
	C.m_TCost = Par.m_TCost;
	C.m_Lag += gs_P.m_BrakeW * C.m_Brake + gs_P.m_HRefW * C.m_HDis;
	if(gs_P.m_TTrack > 0)
	{
		// time-indexed: distance to the reference's position at the same (offset) race tick
		int i = std::clamp((int)std::lround(Rt - gs_P.m_TOff - gs_Line.m_Rt0), 0, (int)gs_Line.m_vP.size() - 1);
		float d = distance(G.m_Core.m_Pos, gs_Line.m_vP[i]);
		float dv = distance(G.m_Core.m_Vel, gs_Line.m_vV[i]);
		float c = std::min(d, gs_P.m_TCap) + gs_P.m_TTrackV * std::min(dv, 20.0f);
		C.m_TCost = Par.m_TCost * gs_P.m_TDecay + c * c / 1000.0f;
		C.m_Lag = gs_P.m_TTrack * C.m_TCost + gs_P.m_TLag * C.m_Lag;
	}
}

// ------------------------------------------------------------------------------------------------ rollout lookahead
// Score a state by short core-only rollouts with three cheap policies (coast / hold the current hook / steer towards
// the route with rotation pulses); returns the best end lag (ticks behind the incumbent at the same geodesic
// distance, judged H ticks later) and that rollout's end energy.
static void RollEval(const CFastG &G0, const STasInput &Prev, float K0, int H, float LamMid, float &LagOut, float &EOut)
{
	const int Rt0 = G0.RaceTick();
	float BestS = 1e30f, BestLag = 1e9f, BestE = 0;
	for(int Pol = 0; Pol < 3; Pol++)
	{
		if(Pol == 1 && !Prev.m_Hook)
			continue;
		CCharacterCore Core = G0.m_Core;
		Core.SetCoreWorld(nullptr, CTasGame::Collision(), &CFastG::ms_Teams);
		CNetObj_PlayerInput In = G0.m_Input;
		In.m_Jump = 0;
		int HookIn = Prev.m_Hook;
		bool Dead = false;
		int t = 0;
		float KR = K0;
		for(; t < H; t++)
		{
			vec2 P = Core.m_Pos, V = Core.m_Vel;
			int Dir = std::fabs(V.x) > 3.0f ? (V.x > 0 ? 1 : -1) : Prev.m_Dir;
			if(Pol == 0)
				HookIn = 0;
			else if(Pol == 1)
				In.m_Direction = Prev.m_Dir;
			else
			{
				vec2 U = gs_Geo.Dir(P + V * 3.0f);
				float L = length(V);
				float Phi = std::atan2(V.x * U.y - V.y * U.x, V.x * U.x + V.y * U.y);
				if(std::fabs(U.x) > 0.3f && L < 15.0f)
					Dir = U.x > 0 ? 1 : -1;
				if(L < 15.0f)
				{
					if(Core.m_HookState == HOOK_IDLE || !HookIn)
					{
						HookIn = 1;
						In.m_TargetX = (int)(U.x * 1000);
						In.m_TargetY = (int)(U.y * 1000);
					}
				}
				else if(std::fabs(Phi) > 0.1f)
				{
					float Side = Phi > 0 ? 1.0f : -1.0f;
					if(Core.m_HookState == HOOK_GRABBED && HookIn)
					{
						vec2 A = Core.m_HookPos - P;
						float c = dot(normalize(A), V / L);
						float sd = (V.x * A.y - V.y * A.x) * Side;
						if(!(c < 0.0f && c > -0.35f && sd > 0))
							HookIn = 0;
					}
					else if(Core.m_HookState == HOOK_FLYING && HookIn)
						;
					else if(!HookIn)
					{
						float Ang = std::atan2(V.y, V.x) + Side * (97.0f * pi / 180.0f);
						HookIn = 1;
						In.m_TargetX = (int)(std::cos(Ang) * 1000);
						In.m_TargetY = (int)(std::sin(Ang) * 1000);
					}
					else
						HookIn = 0;
				}
				else
					HookIn = 0;
			}
			if(Pol != 1)
				In.m_Direction = Dir;
			In.m_Hook = HookIn;
			Core.m_Input = In;
			vec2 P0 = Core.m_Pos;
			Core.Tick(true);
			Core.Move();
			Core.Quantize();
			if(SegFreeze(P0, Core.m_Pos))
			{
				Dead = true;
				break;
			}
			KR = gs_Line.Project(Core.m_Pos, KR);
		}
		float Lat = 0;
		float K = gs_Line.Project(Core.m_Pos, KR, &Lat);
		float Lag = (Rt0 + t + 1) - K + (Dead ? 10.0f + (H - t) : 0.0f) + gs_P.m_LatPen * std::max(0.0f, Lat - gs_P.m_LatDz);
		float E = dot(Core.m_Vel, Core.m_Vel) - Core.m_Pos.y;
		float S = Lag - LamMid * (E - gs_Line.EnergyAt(K));
		if(S < BestS)
		{
			BestS = S;
			BestLag = Lag;
			BestE = E;
		}
	}
	LagOut = BestLag;
	EOut = BestE;
}

static std::vector<STasInput> gs_vIncIn;
// anchor shadowing: per incumbent input index, where its hook press grabbed and where its shot exploded (world points),
// so a shadowing state aims at the same targets from its own position (robust to small offsets)
static std::vector<vec2> gs_vAnchor; // hook press -> grab point (x < -1e8: none)
static std::vector<vec2> gs_vShotX; // shot -> explosion point
static std::vector<int> gs_vShotTau; // shot -> flight ticks until the explosion step (1 = same step), -1 none
static int gs_ShadowMode = 2; // 1: raw inputs, 2: anchor targets

static int16_t Q16(float v) { return (int16_t)std::lround(std::clamp(v, -1.0f, 1.0f) * 10000); }

// aim from Pos so that a grenade explodes at X after Tau flight ticks (same solver as the retro shots)
static vec2 AimAt(const CFastG &G, vec2 Pos, vec2 X, int Tau)
{
	const float Curv = G.m_Core.m_Tuning.m_GrenadeCurvature, Speed = G.m_Core.m_Tuning.m_GrenadeSpeed;
	const float Vg = Speed / SERVER_TICK_SPEED, Cg = Curv / 10000.0f * Vg * Vg, R0 = CCharacterCore::PhysicalSize() * 0.75f;
	const vec2 W = X - Pos;
	if(Tau <= 1)
		return length(W) > 1e-3f ? normalize(W) : vec2(0, 1);
	auto F = [&](float T2) { return length(vec2(W.x, W.y - Cg * T2 * T2)) - R0 - Vg * T2; };
	float Lo = Tau - 1.0f, Hi = (float)Tau;
	if(!(F(Lo) > 0 && F(Hi) <= 0))
		return normalize(W);
	for(int it = 0; it < 22; it++)
	{
		float Mid = 0.5f * (Lo + Hi);
		if(F(Mid) > 0)
			Lo = Mid;
		else
			Hi = Mid;
	}
	return normalize(vec2(W.x, W.y - Cg * Hi * Hi));
}

static STasInput ShadowInput(long Idx, const CFastG &G, const STasInput &Prev)
{
	STasInput In = gs_vIncIn[Idx];
	In.m_Weapon = 3;
	if(gs_ShadowMode < 2)
		return In;
	const vec2 P = G.m_Pos;
	bool Press = In.m_Hook && Idx > 0 && !gs_vIncIn[Idx - 1].m_Hook;
	if(Press && !Prev.m_Hook && gs_vAnchor[Idx].x > -1e8f)
	{
		vec2 D = gs_vAnchor[Idx] - P;
		if(length(D) > 1e-3f)
		{
			D = normalize(D);
			In.m_TX = Q16(D.x);
			In.m_TY = Q16(D.y);
		}
	}
	else if(In.m_Fire && gs_vShotTau[Idx] >= 1)
	{
		vec2 D = AimAt(G, P, gs_vShotX[Idx], gs_vShotTau[Idx]);
		In.m_TX = Q16(D.x);
		In.m_TY = Q16(D.y);
		if(!In.m_TX && !In.m_TY)
			In.m_TY = -1;
	}
	return In;
}

// shadow rollouts: follow the incumbent's own inputs (from the matched progress point, offsets 0..n-1) for H ticks on
// the exact stepper; returns the best end lag and its energy (lag at H ticks: race tick - progress)
static void ShadowEval(const CFastG &G0, float K0, float KI0, int H, int NOff, float LamMid, float &LagOut, float &EOut, bool &Any)
{
	float BestS = 1e30f;
	Any = false;
	for(int o = 0; o < NOff; o++)
	{
		CFastG F = G0;
		int Base = (int)std::floor(KI0) + o - (NOff > 1 ? NOff / 2 : 0);
		bool Dead = false;
		int t = 0;
		float K = K0;
		for(; t < H; t++)
		{
			long Idx = (long)(Base + t) + gs_IncStart;
			if(Idx < 0 || Idx >= (long)gs_vIncIn.size())
				break;
			STasInput In = ShadowInput(Idx, F, t == 0 ? gs_vIncIn[std::max(0L, Idx - 1)] : gs_vIncIn[Idx - 1]);
			F.Step(In);
			if(F.m_Dead)
			{
				Dead = true;
				break;
			}
			if(F.m_FinishTick >= 0)
				break;
			K = gs_Line.Project(F.m_Core.m_Pos, K);
		}
		if(t == 0)
			continue;
		float Lag = (float)F.RaceTick() - K + (Dead ? 10.0f + (H - t) : 0.0f);
		float E = Energy(F);
		float S = Lag - LamMid * (E - gs_Line.EnergyAt(K));
		if(S < BestS)
		{
			BestS = S;
			LagOut = Lag;
			EOut = E;
			Any = true;
		}
	}
}

// ------------------------------------------------------------------------------------------------ main
int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_ds <map> inc=RUN cut=RT key=value...\n");
		return 1;
	}
	CFastG::Init();
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "inc") gs_P.m_Inc = V;
		else if(K == "prefix") gs_P.m_Prefix = V;
		else if(K == "anc") gs_P.m_Anc = V;
		else if(K == "out") gs_P.m_Out = V;
		else if(K == "beamdump") gs_P.m_BeamDump = V;
		else if(K == "gate") gs_P.m_Gate = V;
		else if(K == "cut") gs_P.m_Cut = std::stoi(V);
		else if(K == "beam") gs_P.m_Beam = std::stoi(V);
		else if(K == "threads") gs_P.m_Threads = std::stoi(V);
		else if(K == "maxsteps") gs_P.m_MaxSteps = std::stoi(V);
		else if(K == "maxf") gs_P.m_MaxF = std::stoi(V);
		else if(K == "retro") gs_P.m_Retro = std::stoi(V);
		else if(K == "retro2") gs_P.m_Retro2 = std::stoi(V);
		else if(K == "hookangles") gs_P.m_HookAngles = std::stoi(V);
		else if(K == "fireangles") gs_P.m_FireAngles = std::stoi(V);
		else if(K == "firekeep") gs_P.m_FireKeep = std::stoi(V);
		else if(K == "optw") gs_P.m_OptW = std::stof(V);
		else if(K == "brakew") gs_P.m_BrakeW = std::stof(V);
		else if(K == "rotfar") gs_P.m_RotFar = std::stoi(V);
		else if(K == "hookref") gs_P.m_HookRef = V;
		else if(K == "htrack") gs_P.m_HTrack = V;
		else if(K == "hrefw") gs_P.m_HRefW = std::stof(V);
		else if(K == "hrefwin") gs_P.m_HRefWin = std::stoi(V);
		else if(K == "hrefhard") gs_P.m_HRefHard = std::stoi(V);
		else if(K == "incforce") gs_P.m_IncForce = std::stoi(V);
		else if(K == "gatewait") gs_P.m_GateWait = std::stoi(V);
		else if(K == "cellpos") gs_P.m_CellPos = std::stof(V);
		else if(K == "cellvel") gs_P.m_CellVel = std::stof(V);
		else if(K == "verbose") gs_P.m_Verbose = std::stoi(V);
		else if(K == "dirall") gs_P.m_DirAll = std::stoi(V);
		else if(K == "kickmin") gs_P.m_KickMin = std::stof(V);
		else if(K == "surv") gs_P.m_Surv = std::stoi(V);
		else if(K == "rot") gs_P.m_Rot = std::stoi(V);
		else if(K == "jitter") gs_P.m_Jitter = std::stof(V);
		else if(K == "seed") gs_P.m_Seed = std::stoi(V);
		else if(K == "credw") gs_P.m_CredW = std::stof(V);
		else if(K == "kcred") gs_P.m_KCred = std::stof(V);
		else if(K == "kready") gs_P.m_KReady = std::stoi(V);
		else if(K == "quota") gs_P.m_Quota = std::stoi(V);
		else if(K == "rollh") gs_P.m_RollH = std::stoi(V);
		else if(K == "shh") gs_P.m_ShH = std::stoi(V);
		else if(K == "shmode") gs_ShadowMode = std::stoi(V);
		else if(K == "shoff") gs_P.m_ShOff = std::stoi(V);
		else if(K == "shmix") gs_P.m_ShMix = std::stoi(V);
		else if(K == "shadow") gs_P.m_Shadow = std::stoi(V);
		else if(K == "latpen") gs_P.m_LatPen = std::stof(V);
		else if(K == "egain") gs_P.m_EGain = std::stof(V);
		else if(K == "gatesurv") gs_P.m_GateSurv = std::stoi(V);
		else if(K == "commitk") gs_P.m_CommitK = std::stoi(V);
		else if(K == "tref") gs_P.m_TRef = V;
		else if(K == "ttrack") gs_P.m_TTrack = std::stof(V);
		else if(K == "toff") gs_P.m_TOff = std::stof(V);
		else if(K == "tcap") gs_P.m_TCap = std::stof(V);
		else if(K == "ttrackv") gs_P.m_TTrackV = std::stof(V);
		else if(K == "tdecay") gs_P.m_TDecay = std::stof(V);
		else if(K == "tlag") gs_P.m_TLag = std::stof(V);
		else if(K == "tshift") gs_P.m_TShift = std::stoi(V);
		else if(K == "tsmooth") gs_P.m_TSmooth = std::stoi(V);
		else if(K == "sinkh") gs_Line.m_SinkH = std::stof(V);
		else if(K == "boost") gs_P.m_Boost = std::stof(V);
		else if(K == "sinkmin") gs_Line.m_SinkMin = std::stof(V);
		else if(K == "nofire") std::sscanf(V.c_str(), "%d,%d", &gs_P.m_NoFire0, &gs_P.m_NoFire1);
		else if(K == "nokick") std::sscanf(V.c_str(), "%d,%d", &gs_P.m_NoKick0, &gs_P.m_NoKick1);
		else if(K == "trackfrac") gs_P.m_TrackFrac = std::stof(V);
		else if(K == "trackv") gs_P.m_TrackV = std::stof(V);
		else if(K == "tracklag") gs_P.m_TrackLag = std::stof(V);
		else if(K == "latdz") gs_P.m_LatDz = std::stof(V);
		else if(K == "shadowback") gs_P.m_ShadowBack = std::stoi(V);
		else if(K == "rollpre") gs_P.m_RollPre = std::stof(V);
		else if(K == "rolllam") gs_P.m_RollLam = std::stof(V);
		else if(K == "qpos") gs_P.m_QPos = std::stof(V);
		else if(K == "qvel") gs_P.m_QVel = std::stof(V);
		else if(K == "credh") gs_P.m_CredH = std::stoi(V);
		else if(K == "over") gs_P.m_Over = std::stof(V);
		else if(K == "lam")
		{
			gs_P.m_vLam.clear();
			size_t p = 0;
			while(p < V.size())
			{
				size_t q = V.find(',', p);
				if(q == std::string::npos)
					q = V.size();
				gs_P.m_vLam.push_back(std::stof(V.substr(p, q - p)));
				p = q + 1;
			}
		}
		else
		{
			std::printf("unknown option %s\n", K.c_str());
			return 1;
		}
	}
	auto t0 = std::chrono::steady_clock::now();
	gs_Geo.Build();

	// incumbent: replay, reference profile, prefix up to the cut
	std::vector<STasInput> vInc = ReadInputs(gs_P.m_Inc.c_str());
	gs_vIncIn = vInc;
	gs_vAnchor.assign(vInc.size(), vec2(-1e9f, -1e9f));
	gs_vShotX.assign(vInc.size(), vec2(0, 0));
	gs_vShotTau.assign(vInc.size(), -1);
	long PendPress = -1;
	if(vInc.empty())
	{
		std::printf("cannot read inc\n");
		return 1;
	}
	std::vector<STasInput> vPrefix;
	SState Root;
	std::vector<int> vIncFires; // fire ticks of the incumbent (projectile start ticks)
	int IncFinish = -1;
	{
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		size_t i = 0;
		for(; i < vInc.size() && !G.HasGrenade(); i++)
			G.Step(vInc[i]);
		CFastG F;
		F.FromGame(G);
		gs_Ref.m_Rt0 = F.RaceTick();
		gs_IncStart = F.m_StartTick;
		bool CutDone = !gs_P.m_Prefix.empty();
		for(; i < vInc.size(); i++)
		{
			if(!CutDone && F.RaceTick() >= gs_P.m_Cut)
			{
				CutDone = true;
				vPrefix.assign(vInc.begin(), vInc.begin() + i);
				Root.m_G = F;
				Root.m_Prev = vInc[i - 1];
			}
			int Rl = F.m_ReloadTimer;
			int Tick = F.m_Tick;
			if(vInc[i].m_Hook && i > 0 && !vInc[i - 1].m_Hook)
				PendPress = (long)i;
			if(!vInc[i].m_Hook)
				PendPress = -1;
			std::vector<SExplLog> vL;
			CFastG::ms_pLog = &vL;
			F.Step(vInc[i]);
			CFastG::ms_pLog = nullptr;
			if(PendPress >= 0 && F.m_Core.m_HookState == HOOK_GRABBED)
			{
				gs_vAnchor[PendPress] = F.m_Core.m_HookPos;
				PendPress = -1;
			}
			if(F.m_ReloadTimer > Rl)
			{
				vIncFires.push_back(Tick);
				// where does this shot explode? in flight: fly it; gone already: this step's explosion
				int k = -1;
				for(int q = 0; q < F.m_NumProj; q++)
					if(F.m_aProj[q].m_StartTick == Tick)
						k = q;
				if(k < 0 && !vL.empty())
				{
					gs_vShotX[i] = vL.back().m_E;
					gs_vShotTau[i] = 1;
				}
				else if(k >= 0)
				{
					const SFastProj &Pj = F.m_aProj[k];
					const float Curv = F.m_Core.m_Tuning.m_GrenadeCurvature, Speed = F.m_Core.m_Tuning.m_GrenadeSpeed;
					for(int T2 = F.m_Tick - Pj.m_StartTick + 1; T2 <= 100; T2++)
					{
						vec2 A = CalcPos(Pj.m_Pos, Pj.m_Dir, Curv, Speed, (T2 - 1) / (float)SERVER_TICK_SPEED);
						vec2 B = CalcPos(Pj.m_Pos, Pj.m_Dir, Curv, Speed, T2 / (float)SERVER_TICK_SPEED);
						vec2 Col, NP;
						if(CTasGame::Collision()->IntersectLine(A, B, &Col, &NP))
						{
							gs_vShotX[i] = Col;
							gs_vShotTau[i] = T2;
							break;
						}
					}
				}
			}
			gs_Ref.m_vG.push_back(gs_Geo.At(F.m_Core.m_Pos));
			gs_Ref.m_vE.push_back(Energy(F));
			gs_Line.m_vP.push_back(F.m_Core.m_Pos);
			gs_Line.m_vV.push_back(F.m_Core.m_Vel);
			gs_Line.m_vE.push_back(Energy(F));
			if(F.m_FinishTick >= 0)
			{
				IncFinish = F.RaceTick();
				break;
			}
		}
		// gs_Ref index 0 = race tick m_Rt0 + 1
		gs_Ref.m_Rt0 += 1;
		gs_Line.m_Rt0 = gs_Ref.m_Rt0;
		gs_IncLine = gs_Line;
		if(!gs_P.m_TRef.empty())
		{
			SRefLine T;
			if(!LoadTrackLine(gs_P.m_TRef.c_str(), gs_P.m_TShift, gs_P.m_TSmooth, T))
			{
				std::printf("cannot read %s\n", gs_P.m_TRef.c_str());
				return 1;
			}
			T.m_SinkH = gs_Line.m_SinkH;
			T.m_SinkMin = gs_Line.m_SinkMin;
			gs_Line = T;
			gs_TRef = true;
		}
		gs_Line.FindSinks();
		gs_Ref.m_vEnv.resize(gs_Ref.m_vG.size());
		float m = 1e9f;
		for(size_t k = 0; k < gs_Ref.m_vG.size(); k++)
		{
			m = std::min(m, gs_Ref.m_vG[k]);
			gs_Ref.m_vEnv[k] = m;
		}
		if(!CutDone)
		{
			std::printf("cut beyond the run\n");
			return 1;
		}
	}
	if(!gs_P.m_Prefix.empty())
	{
		// root from a separate prefix (a run that left the incumbent): the incumbent is only the reference / shadow
		vPrefix = ReadInputs(gs_P.m_Prefix.c_str());
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		size_t i = 0;
		for(; i < vPrefix.size() && !G.HasGrenade(); i++)
			G.Step(vPrefix[i]);
		CFastG F;
		F.FromGame(G);
		vIncFires.clear();
		for(; i < vPrefix.size(); i++)
		{
			int Rl = F.m_ReloadTimer, Tick = F.m_Tick;
			F.Step(vPrefix[i]);
			if(F.m_ReloadTimer > Rl)
				vIncFires.push_back(Tick);
		}
		Root.m_G = F;
		Root.m_Prev = vPrefix.back();
		if(F.m_Dead)
		{
			std::printf("prefix dies\n");
			return 1;
		}
	}
	for(int f : vIncFires)
		if(f < Root.m_G.m_Tick)
			AddFire(Root, f);
	if(gs_P.m_Boost != 0)
	{
		vec2 V = Root.m_G.m_Core.m_Vel;
		Root.m_G.m_Core.m_Vel = V + normalize(V) * gs_P.m_Boost;
		std::printf("boost: v %.1f %.1f -> %.1f %.1f\n", V.x, V.y, Root.m_G.m_Core.m_Vel.x, Root.m_G.m_Core.m_Vel.y);
	}
	gs_T0 = Root.m_G.m_Tick;
	Root.m_KInc = gs_P.m_Prefix.empty() ? (float)Root.m_G.RaceTick() : gs_IncLine.Project(Root.m_G.m_Core.m_Pos, (float)Root.m_G.RaceTick() - 60, nullptr, 60, 80);
	if(gs_TRef)
		Root.m_G0 = gs_Line.Project(Root.m_G.m_Core.m_Pos, (float)Root.m_G.RaceTick() - 60, nullptr, 160, 70);
	else
		Root.m_G0 = Root.m_KInc;
	if(gs_P.m_TOff < -1e8f)
		gs_P.m_TOff = std::round(Root.m_G.RaceTick() - Root.m_G0);
	std::printf("root progress %.1f (incumbent path %.1f), time offset vs reference %.0f\n", Root.m_G0, Root.m_KInc, gs_P.m_TOff);
	if(!gs_P.m_HookRef.empty())
	{
		if(!LoadTrackLine(gs_P.m_HTrack.c_str(), 0, 1, gs_TL))
		{
			std::printf("cannot read %s\n", gs_P.m_HTrack.c_str());
			return 1;
		}
		FILE *f = std::fopen(gs_P.m_HookRef.c_str(), "r");
		if(!f)
		{
			std::printf("cannot read %s\n", gs_P.m_HookRef.c_str());
			return 1;
		}
		char aLine[512];
		std::vector<std::tuple<int, char, vec2>> vR;
		std::fgets(aLine, sizeof(aLine), f);
		while(std::fgets(aLine, sizeof(aLine), f))
		{
			int k;
			char St = 0;
			float ax = 0, ay = 0;
			if(std::sscanf(aLine, "%d,%c,%f,%f", &k, &St, &ax, &ay) >= 2)
				vR.emplace_back(k, St, vec2(ax, ay));
		}
		std::fclose(f);
		if(vR.empty())
			return 1;
		gs_HkK0 = std::get<0>(vR.front());
		gs_vHkS.assign(std::get<0>(vR.back()) - gs_HkK0 + 1, 0);
		gs_vHkA.assign(gs_vHkS.size(), vec2(0, 0));
		for(auto &[k, St, A] : vR)
		{
			gs_vHkS[k - gs_HkK0] = St;
			gs_vHkA[k - gs_HkK0] = A;
		}
		Root.m_TK = gs_TL.Project(Root.m_G.m_Core.m_Pos, (float)Root.m_G.RaceTick(), nullptr, 150, 150);
		std::printf("hookref: %zu ticks from %s, root at Teero tick %.1f (race tick %d)\n", vR.size(), gs_P.m_HookRef.c_str(), Root.m_TK,
			Root.m_G.RaceTick());
	}
	gs_RootPos = Root.m_G.m_Pos;
	gs_RootRt = Root.m_G.RaceTick();
	Root.m_Inc = gs_P.m_Prefix.empty();
	// anchor: a run continuing the prefix (e.g. the previous window's best) is kept as the forced lineage
	std::vector<STasInput> vForced = vInc;
	if(!gs_P.m_Anc.empty())
	{
		vForced = ReadInputs(gs_P.m_Anc.c_str());
		bool Ok = vForced.size() > vPrefix.size();
		for(size_t k = 0; Ok && k < vPrefix.size(); k++)
			Ok = vForced[k].m_Dir == vPrefix[k].m_Dir && vForced[k].m_Jump == vPrefix[k].m_Jump && vForced[k].m_Hook == vPrefix[k].m_Hook &&
			     vForced[k].m_Fire == vPrefix[k].m_Fire && vForced[k].m_TX == vPrefix[k].m_TX && vForced[k].m_TY == vPrefix[k].m_TY;
		if(!Ok)
		{
			std::printf("anc does not continue the prefix, ignored\n");
			vForced.clear();
		}
		else
			Root.m_Inc = true;
	}
	float GateG = -1; // gate on progress: incumbent race tick
	if(gs_P.m_Gate.rfind("rt", 0) == 0)
		GateG = (float)std::stoi(gs_P.m_Gate.substr(2));
	else if(gs_P.m_Gate != "finish")
		GateG = std::stof(gs_P.m_Gate);
	std::printf("x_ds: cut rt %d (tick %d) pos %.0f %.0f v %.1f %.1f geo %.0f | incumbent finish rt %d, ref time at cut %.1f | geo field %.1fs\n",
		Root.m_G.RaceTick(), gs_T0, Root.m_G.m_Pos.x, Root.m_G.m_Pos.y, Root.m_G.m_Core.m_Vel.x, Root.m_G.m_Core.m_Vel.y,
		gs_Geo.At(Root.m_G.m_Core.m_Pos), IncFinish, gs_Ref.TimeAt(gs_Geo.At(Root.m_G.m_Core.m_Pos)),
		std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
	if(GateG >= 0)
		std::printf("gate: progress >= %.0f, incumbent reaches it at rt %.2f with E %.0f\n", GateG, GateG, gs_Line.EnergyAt(GateG));
	std::fflush(stdout);

	std::vector<SState> vBeam;
	vBeam.push_back(Root);
	const int NT = std::max(1, gs_P.m_Threads);
	const size_t IncBase = vPrefix.size();
	// best gate arrival
	struct SBest
	{
		float m_T = 1e30f;
		int m_Step = -1, m_Idx = -1;
		float m_E = 0;
		int m_Cand = -1;
	} Best;
	int FirstGate = -1;
	FILE *pDump = gs_P.m_BeamDump.empty() ? nullptr : std::fopen(gs_P.m_BeamDump.c_str(), "w");
	if(pDump)
		std::fprintf(pDump, "R %d %.0f %.0f %.3f %.3f\n", Root.m_G.RaceTick(), Root.m_G.m_Pos.x, Root.m_G.m_Pos.y,
			Root.m_G.m_Core.m_Vel.x, Root.m_G.m_Core.m_Vel.y);
	long DumpCand = 0;

	for(int Step = 0; Step < gs_P.m_MaxSteps && !vBeam.empty(); Step++)
	{
		if(FirstGate >= 0 && Step > FirstGate + gs_P.m_GateWait)
			break;
		std::vector<std::vector<SCand>> vTC(NT);
		std::atomic<int> Next{0};
		auto Worker = [&](int T) {
			std::vector<SCand> &vC = vTC[T];
			std::vector<std::pair<int16_t, int16_t>> vHook, vFire, vRot;
			std::vector<SRetroShot> vR, vR2;
			std::vector<SExplLog> vLog;
			SState Tmp;
			while(true)
			{
				int i = Next.fetch_add(1);
				if(i >= (int)vBeam.size())
					break;
				const SState &S = vBeam[i];
				const CFastG &G = S.m_G;
				const vec2 RouteDir = gs_Geo.Dir(G.m_Core.m_Pos);
				const STasInput &Prev = S.m_Prev;
				const bool Hooking = Prev.m_Hook;
				// Teero's hooks here (hookref): may hook (he hooks near), should hook (he hooks throughout), his anchor
				bool HrOn = true, HrOff = true, HrAnc = false;
				vec2 HrA(0, 0);
				if(!gs_vHkS.empty())
				{
					const int Tk = (int)std::lround(S.m_TK);
					HrOn = HrOff = false;
					for(int d = -gs_P.m_HRefWin; d <= gs_P.m_HRefWin; d++)
					{
						char c = HkAt(Tk + d);
						if(c == 'G' || c == 'F')
							HrOn = true;
						else if(c == '-')
							HrOff = true;
						else
							HrOn = HrOff = true;
					}
					for(int k = Tk - gs_P.m_HRefWin; k <= Tk + 8; k++)
						if(HkAt(k) == 'G')
						{
							HrA = gs_vHkA[k - gs_HkK0];
							HrAnc = true;
							break;
						}
				}
				const bool Grounded = CTasGame::Collision()->IsOnGround(G.m_Pos, 28.0f);
				const bool CanJump = !Prev.m_Jump && (Grounded || !(G.m_Core.m_Jumped & 2));
				auto Emit = [&](const STasInput &In, const SRetroShot *pR1, const SRetroShot *pR2, bool ImmFire, bool IsInc) {
					if(In.m_Fire && G.RaceTick() + 1 >= gs_P.m_NoFire0 && G.RaceTick() + 1 <= gs_P.m_NoFire1 && !IsInc)
						return;
					if((In.m_Fire || pR1) && G.RaceTick() + 1 >= gs_P.m_NoKick0 && G.RaceTick() + 1 <= gs_P.m_NoKick1 && !IsInc)
						return;
					Tmp = S;
					if(pR1)
						ApplyRetro(Tmp, *pR1);
					if(pR2)
						ApplyRetro(Tmp, *pR2);
					const int Tk = Tmp.m_G.m_Tick;
					vLog.clear();
					CFastG::ms_pLog = &vLog;
					Tmp.m_G.Step(In);
					CFastG::ms_pLog = nullptr;
					if(Tmp.m_G.m_Dead || Tmp.m_G.m_Bad)
						return;
					if(ImmFire)
					{
						// the new grenade must have exploded in this step
						for(int k = 0; k < Tmp.m_G.m_NumProj; k++)
							if(Tmp.m_G.m_aProj[k].m_StartTick == Tk)
								return;
						if(gs_P.m_KickMin > 0)
						{
							bool Strong = false;
							for(auto &L : vLog)
								Strong |= length(L.m_Force) >= gs_P.m_KickMin;
							if(!Strong)
								return;
						}
					}
					if((pR1 || pR2) && vLog.empty())
						return; // retro grenade did not explode (should not happen)
					if(pR1)
						gs_NRetroEmit++;
					if(ImmFire)
						gs_NFireEmit++;
					if(ImmFire && pR1)
						gs_NStackEmit++;
					SCand C;
					C.m_Parent = i;
					C.m_In = In;
					C.m_NR = 0;
					if(pR1)
						C.m_aR[C.m_NR++] = *pR1;
					if(pR2)
						C.m_aR[C.m_NR++] = *pR2;
					C.m_Brake = S.m_Brake + (gs_P.m_BrakeW > 0 ? BrakeLoss(G, In, Tmp.m_G) : 0.0f);
					C.m_TK = S.m_TK;
					C.m_HDis = S.m_HDis;
					if(!gs_vHkS.empty())
					{
						C.m_TK = gs_TL.Project(Tmp.m_G.m_Core.m_Pos, S.m_TK);
						const bool Dis = In.m_Hook ? !HrOn : !HrOff;
						if(Dis && !IsInc)
						{
							if(gs_P.m_HRefHard)
								return;
							C.m_HDis += 1;
						}
					}
					Score(Tmp.m_G, C, S);
					if(gs_P.m_KCred > 0 && Tmp.m_G.m_ReloadTimer <= gs_P.m_KReady)
						C.m_Opt += gs_P.m_KCred * KickCredit(Tmp.m_G);
					if(gs_P.m_CredW > 0)
					{
						int aF[MAXFIRE + 1];
						int NF = Tmp.m_NFire;
						for(int k = 0; k < NF; k++)
							aF[k] = Tmp.m_aFire[k];
						if(Tmp.m_G.m_ReloadTimer > 0 && ReloadFrom(Tmp, Tmp.m_G.m_Tick) == 0)
							aF[NF++] = Tk; // a shot of this step (point-blank or the incumbent's)
						C.m_Opt += SlotCredit(aF, NF, Tmp.m_G.m_Tick, length(Tmp.m_G.m_Core.m_Vel));
					}
					C.m_Key = StateKey(Tmp.m_G) ^ (uint64_t)(In.m_Hook * 7 + In.m_Jump * 13 + In.m_Fire * 31);
					if(gs_P.m_Jitter > 0)
					{
						uint64_t h = C.m_Key * 0x9E3779B97F4A7C15ull + (uint64_t)gs_P.m_Seed * 0xBF58476D1CE4E5B9ull;
						h ^= h >> 31;
						h *= 0x94D049BB133111EBull;
						h ^= h >> 29;
						C.m_Jit = ((h & 0xFFFFFF) / (float)0xFFFFFF * 2.0f - 1.0f) * gs_P.m_Jitter;
					}
					const CCharacterCore &Co = Tmp.m_G.m_Core;
					int64_t cx = (int64_t)std::floor(Co.m_Pos.x / gs_P.m_CellPos), cy = (int64_t)std::floor(Co.m_Pos.y / gs_P.m_CellPos);
					int64_t vx = (int64_t)std::floor(Co.m_Vel.x / gs_P.m_CellVel), vy = (int64_t)std::floor(Co.m_Vel.y / gs_P.m_CellVel);
					C.m_Cell = ((cx * 4096 + cy) * 1024 + (vx & 1023)) * 1024 + (vy & 1023);
					C.m_Cell = C.m_Cell * 8 + (Co.m_HookState == HOOK_GRABBED ? 1 : Co.m_HookState == HOOK_FLYING ? 2 : 0) + 3 * (Tmp.m_G.m_ReloadTimer > 0);
					{
						int64_t qx = (int64_t)std::floor(Co.m_Pos.x / gs_P.m_QPos), qy = (int64_t)std::floor(Co.m_Pos.y / gs_P.m_QPos);
						int64_t qvx = (int64_t)std::floor(Co.m_Vel.x / gs_P.m_QVel), qvy = (int64_t)std::floor(Co.m_Vel.y / gs_P.m_QVel);
						C.m_QCell = ((qx * 1024 + qy) * 64 + (qvx & 63)) * 64 + (qvy & 63);
					}
					C.m_Inc = IsInc;
					if(GateG >= 0)
					{
						if(C.m_G >= GateG)
						{
							float g0 = S.m_G0;
							float f = C.m_G > g0 ? (GateG - g0) / (C.m_G - g0) : 1.0f;
							C.m_Gate = true;
							C.m_GateT = Tmp.m_G.RaceTick() - 1 + std::clamp(f, 0.0f, 1.0f);
						}
					}
					else if(Tmp.m_G.m_FinishTick >= 0)
					{
						C.m_Gate = true;
						C.m_GateT = (float)Tmp.m_G.RaceTick();
					}
					vC.push_back(C);
				};
				// incumbent continuation
				bool IncHere = gs_P.m_IncForce >= 0 && S.m_Inc && IncBase + Step < vForced.size();
				if(IncHere)
					Emit(vForced[IncBase + Step], nullptr, nullptr, false, true);
				// shadow: the incumbent's own inputs where it was at this progress (time-shifted imitation)
				if(gs_P.m_Shadow > 0)
				{
					const float Kf = S.m_KInc; // incumbent race tick here
					const int K0 = (int)std::floor(Kf);
					const int RtRoot = gs_RootRt;
					for(int d = -gs_P.m_ShadowBack; d < gs_P.m_Shadow; d++)
					{
						// input of the incumbent's step from race tick K0 + d to K0 + d + 1
						long Idx = (long)(K0 + d) + gs_IncStart;
						if(Idx < 0 || Idx >= (long)vInc.size() || K0 + d < gs_Ref.m_Rt0)
							continue;
						(void)RtRoot;
						if(IncHere && gs_P.m_Anc.empty() && Idx == (long)(IncBase + Step))
							continue; // that one is the forced child already
						STasInput In = ShadowInput(Idx, G, Prev);
						Emit(In, nullptr, nullptr, false, false);
					}
				}
				// ordinary actions
				STasInput Base = Prev;
				Base.m_Fire = 0;
				Base.m_Weapon = 3;
				if(!Hooking)
				{
					HookAims(G, vHook);
					if(HrOn && HrAnc)
					{
						const float B = std::atan2(HrA.y - G.m_Pos.y, HrA.x - G.m_Pos.x);
						for(float o : {0.0f, 0.0175f, -0.0175f, 0.035f, -0.035f})
							vHook.insert(vHook.begin(), {(int16_t)std::lround(std::cos(B + o) * 10000), (int16_t)std::lround(std::sin(B + o) * 10000)});
					}
				}
				std::vector<float> vFarR;
				if(gs_P.m_RotFar && !Hooking && length(G.m_Core.m_Vel) >= 15.0f)
					FarRays(G, vFarR);
				for(int Dir = -1; Dir <= 1; Dir++)
				{
					if(!gs_P.m_DirAll && Dir == 0)
						continue;
					for(int Jump = 0; Jump <= (CanJump ? 1 : 0); Jump++)
					{
						STasInput In = Base;
						In.m_Dir = Dir;
						In.m_Jump = Jump;
						In.m_Hook = Hooking;
						Emit(In, nullptr, nullptr, false, false);
						if(Hooking)
						{
							STasInput R = In;
							R.m_Hook = 0;
							Emit(R, nullptr, nullptr, false, false);
						}
						else
						{
							for(auto [TX, TY] : vHook)
							{
								STasInput H = In;
								H.m_Hook = 1;
								H.m_TX = TX;
								H.m_TY = TY;
								Emit(H, nullptr, nullptr, false, false);
							}
							if(gs_P.m_Rot || !vFarR.empty())
							{
								vRot.clear();
								if(gs_P.m_Rot)
									RotAims(G, Dir, Jump, vRot);
								if(!vFarR.empty())
									RotFarAims(G, Dir, vFarR, vRot);
								for(auto [TX, TY] : vRot)
								{
									STasInput H = In;
									H.m_Hook = 1;
									H.m_TX = TX;
									H.m_TY = TY;
									Emit(H, nullptr, nullptr, false, false);
								}
							}
						}
					}
				}
				// point-blank shots (explode in this step)
				vFire.clear();
				const bool CanPB = G.m_ReloadTimer == 0 && ReloadFrom(S, G.m_Tick) == 0 && G.m_NumProj < CFastG::MAX_PROJ;
				if(CanPB)
				{
					FireAims(G, RouteDir, vFire);
					gs_NFireTry += (long)vFire.size();
					for(auto [TX, TY] : vFire)
						for(int Dir = -1; Dir <= 1; Dir++)
							for(int Hk = 0; Hk <= (Hooking ? 1 : 0); Hk++)
							{
								STasInput F = Base;
								F.m_Dir = Dir;
								F.m_Jump = 0;
								F.m_Hook = Hooking && Hk == 0;
								F.m_Fire = 1;
								F.m_TX = TX;
								F.m_TY = TY;
								Emit(F, nullptr, nullptr, true, false);
							}
				}
				// retro shots exploding in this step
				if(gs_P.m_Retro > 0 && Step > 0)
				{
					RetroFind(S, Step - 1, RouteDir, vR);
					gs_NRetroFound += (long)vR.size();
					if(CanPB)
					{
						gs_NCanPB++;
						if(!vFire.empty())
							gs_NPBAims++;
						for(auto &R : vR)
							if(G.m_Tick - R.m_Tau >= 25)
							{
								gs_NOldR++;
								break;
							}
					}
					// lob + point-blank double kicks: an old retro shot (25+ ticks, reload back) with a shot now
					if(CanPB)
						for(auto &R : vR)
							if(G.m_Tick - R.m_Tau >= 25)
								for(auto [TX, TY] : vFire)
								{
									gs_NStackTry++;
									STasInput F = Base;
									F.m_Dir = Prev.m_Dir;
									F.m_Jump = 0;
									F.m_Hook = Hooking;
									F.m_Fire = 1;
									F.m_TX = TX;
									F.m_TY = TY;
									Emit(F, &R, nullptr, true, false);
								}
					int Used = 0;
					for(int r = 0; r < (int)vR.size() && Used < gs_P.m_Retro; r++)
					{
						Used++;
						for(int Dir = -1; Dir <= 1; Dir++)
							for(int Hk = 0; Hk <= (Hooking ? 1 : 0); Hk++)
							{
								STasInput In = Base;
								In.m_Dir = Dir;
								In.m_Jump = 0;
								In.m_Hook = Hooking && Hk == 0;
								Emit(In, &vR[r], nullptr, false, false);
							}
						// stacked: a second retro shot from another slot for the same step
						if(gs_P.m_Retro2 > 0 && r < gs_P.m_Retro2)
						{
							SState T2 = S;
							ApplyRetro(T2, vR[r]);
							// T2's history is S's; RetroFind checks slots against T2's fire list
							T2.m_Node = S.m_Node;
							RetroFind(T2, Step - 1, RouteDir, vR2, vR[r].m_Tau);
							for(int r2 = 0; r2 < (int)vR2.size() && r2 < 2; r2++)
							{
								if(std::abs(vR2[r2].m_Tau - vR[r].m_Tau) < 25)
									continue;
								STasInput In = Base;
								In.m_Dir = Prev.m_Dir;
								In.m_Jump = 0;
								In.m_Hook = Hooking;
								Emit(In, &vR[r], &vR2[r2], false, false);
							}
						}
					}
				}
			}
		};
		std::vector<std::thread> vT;
		for(int T = 0; T < NT; T++)
			vT.emplace_back(Worker, T);
		for(auto &T : vT)
			T.join();
		std::vector<SCand> vAll;
		size_t N = 0;
		for(auto &v : vTC)
			N += v.size();
		vAll.reserve(N);
		DumpCand = N;
		for(auto &v : vTC)
			for(auto &c : v)
				vAll.push_back(c);
		if(vAll.empty())
		{
			std::printf("step %d: beam died\n", Step);
			break;
		}
		// gate arrivals: value = crossing time - egain x (energy - incumbent energy there); window gates also need a
		// state that can go on (survival check)
		{
			std::vector<std::pair<float, int>> vG;
			for(size_t k = 0; k < vAll.size(); k++)
				if(vAll[k].m_Gate)
				{
					const SCand &c = vAll[k];
					float Val = GateG >= 0 ? c.m_GateT - gs_P.m_EGain * (c.m_E - gs_Line.EnergyAt(GateG)) : c.m_GateT;
					vG.push_back({Val, (int)k});
				}
			std::sort(vG.begin(), vG.end());
			if(!vG.empty() && FirstGate < 0)
				FirstGate = Step;
			int Checked = 0;
			for(auto &[Val, k] : vG)
			{
				if(Val >= Best.m_T - 1e-4f || Checked > 200)
					break;
				const SCand &c = vAll[k];
				bool Ok = true;
				if(GateG >= 0)
				{
					SState S = vBeam[c.m_Parent];
					for(int r = 0; r < c.m_NR; r++)
						ApplyRetro(S, c.m_aR[r]);
					S.m_G.Step(c.m_In);
					Ok = Survives(S.m_G, c.m_In, gs_P.m_GateSurv);
					Checked++;
				}
				if(!Ok)
					continue;
				Best.m_T = Val;
				Best.m_Step = Step;
				Best.m_Idx = -1;
				Best.m_E = c.m_E;
				Best.m_Cand = k;
				break;
			}
		}
		// selection: union of the best per energy weight, per-cell dedup
		std::vector<int> vSel;
		std::vector<char> vTaken(vAll.size(), 0);
		std::unordered_set<uint64_t> Keys;
		Keys.reserve(gs_P.m_Beam * 2);
		std::unordered_map<int64_t, int> Cells, QCells;
		Cells.reserve(gs_P.m_Beam * 4);
		int IncSel = -1;
		for(size_t k = 0; k < vAll.size(); k++)
			if(vAll[k].m_Inc && !vAll[k].m_Gate)
			{
				IncSel = (int)k;
				break;
			}
		if(IncSel >= 0 && gs_P.m_IncForce)
		{
			vSel.push_back(IncSel);
			vTaken[IncSel] = 1;
			Keys.insert(vAll[IncSel].m_Key);
		}
		const int NL = (int)gs_P.m_vLam.size();
		std::vector<std::vector<int>> vOrder(NL);
		for(int l = 0; l < NL; l++)
		{
			const float Lam = gs_P.m_vLam[l];
			std::vector<std::pair<float, int>> v;
			v.reserve(vAll.size());
			for(size_t k = 0; k < vAll.size(); k++)
			{
				const SCand &c = vAll[k];
				if(c.m_Gate)
					continue;
				float Eref = gs_Line.EnergyAt(c.m_G);
				float S = c.m_Lag + c.m_Jit - Lam * (gs_Line.EWeight(c.m_G) * (c.m_E - Eref) + c.m_Opt);
				v.push_back({S, (int)k});
			}
			int Want = std::min((int)v.size(), gs_P.m_Beam * 20 / NL + 256);
			std::partial_sort(v.begin(), v.begin() + Want, v.end());
			v.resize(Want);
			for(auto &p : v)
				vOrder[l].push_back(p.second);
		}
		// rollout lookahead: re-score the preselected candidates by where short rollouts get them
		if(gs_P.m_RollH > 0 || gs_P.m_ShH > 0)
		{
			const int Pre = (int)(gs_P.m_Beam * gs_P.m_RollPre / NL) + 32;
			std::vector<int> vP;
			std::vector<char> vIn(vAll.size(), 0);
			for(int l = 0; l < NL; l++)
				for(int r = 0; r < (int)vOrder[l].size() && r < Pre; r++)
				{
					int k = vOrder[l][r];
					if(!vIn[k])
					{
						vIn[k] = 1;
						vP.push_back(k);
					}
				}
			if(IncSel >= 0 && !vIn[IncSel])
				vP.push_back(IncSel);
			gs_NPre = (long)vP.size();
			std::atomic<int> Nx{0};
			auto RW = [&]() {
				SState S;
				while(true)
				{
					int j = Nx.fetch_add(1);
					if(j >= (int)vP.size())
						break;
					SCand &c = vAll[vP[j]];
					S = vBeam[c.m_Parent];
					for(int r = 0; r < c.m_NR; r++)
						ApplyRetro(S, c.m_aR[r]);
					S.m_G.Step(c.m_In);
					float Lag = 1e9f, E = 0;
					if(gs_P.m_RollH > 0)
						RollEval(S.m_G, c.m_In, c.m_G, gs_P.m_RollH, gs_P.m_RollLam, Lag, E);
					if(gs_P.m_ShH > 0)
					{
						float L2 = 0, E2 = 0;
						bool Any = false;
						ShadowEval(S.m_G, c.m_G, c.m_KInc, gs_P.m_ShH, gs_P.m_ShOff, gs_P.m_RollLam, L2, E2, Any);
						if(Any && (gs_P.m_RollH <= 0 || L2 - gs_P.m_RollLam * E2 < Lag - gs_P.m_RollLam * E))
						{
							Lag = L2;
							E = E2;
						}
						if(!Any && gs_P.m_RollH <= 0)
						{
							Lag = c.m_Lag;
							E = c.m_E;
						}
					}
					c.m_Lag = Lag + gs_P.m_BrakeW * c.m_Brake + gs_P.m_HRefW * c.m_HDis;
					c.m_E = E;
				}
			};
			std::vector<std::thread> vR;
			for(int T = 0; T < NT; T++)
				vR.emplace_back(RW);
			for(auto &T : vR)
				T.join();
			for(int l = 0; l < NL; l++)
			{
				const float Lam = gs_P.m_vLam[l];
				std::vector<std::pair<float, int>> v;
				for(int k : vP)
				{
					const SCand &c = vAll[k];
					float Eref = gs_Line.EnergyAt(c.m_G);
					v.push_back({c.m_Lag + c.m_Jit - Lam * (gs_Line.EWeight(c.m_G) * (c.m_E - Eref) + c.m_Opt), k});
				}
				std::sort(v.begin(), v.end());
				vOrder[l].clear();
				for(auto &p : v)
					vOrder[l].push_back(p.second);
			}
		}
		// protected trackers: the states closest to the incumbent's own state at the same progress (they keep its line
		// alive, time-shifted), exact-duplicate check only
		if(gs_P.m_TrackFrac > 0)
		{
			std::vector<std::pair<float, int>> v;
			for(size_t k = 0; k < vAll.size(); k++)
				if(!vAll[k].m_Gate && !vTaken[k])
				{
					const SCand &c = vAll[k];
					v.push_back({c.m_Track + gs_P.m_TrackLag * (float)(vAll[k].m_G0Rt - c.m_G), (int)k});
				}
			int Want = std::min((int)v.size(), (int)(gs_P.m_Beam * gs_P.m_TrackFrac));
			std::partial_sort(v.begin(), v.begin() + Want, v.end());
			for(int r = 0; r < Want; r++)
			{
				int k = v[r].second;
				if(Keys.count(vAll[k].m_Key))
					continue;
				Keys.insert(vAll[k].m_Key);
				Cells[vAll[k].m_Cell] = 1;
				vTaken[k] = 1;
				vSel.push_back(k);
			}
		}
		// round robin over the weights
		std::vector<size_t> vPos(NL, 0);
		bool Any = true;
		const int SelMax = gs_P.m_Surv > 0 ? (int)(gs_P.m_Beam * gs_P.m_Over) : gs_P.m_Beam;
		while((int)vSel.size() < SelMax && Any)
		{
			Any = false;
			for(int l = 0; l < NL && (int)vSel.size() < SelMax; l++)
			{
				while(vPos[l] < vOrder[l].size())
				{
					int k = vOrder[l][vPos[l]++];
					if(vTaken[k])
						continue;
					const SCand &c = vAll[k];
					if(Keys.count(c.m_Key))
						continue;
					auto It = Cells.find(c.m_Cell);
					if(It != Cells.end())
						continue;
					if(gs_P.m_Quota > 0)
					{
						int &Q = QCells[c.m_QCell];
						if(Q >= gs_P.m_Quota)
							continue;
						Q++;
					}
					Cells[c.m_Cell] = 1;
					Keys.insert(c.m_Key);
					vTaken[k] = 1;
					vSel.push_back(k);
					Any = true;
					break;
				}
			}
		}
		gs_NSel = (long)vSel.size();
		// build the new beam (re-simulate the selected candidates) and the history step
		std::vector<SState> vNew(vSel.size());
		std::vector<SNode> vNodes(vSel.size());
		{
			std::atomic<int> Nx{0};
			auto Build = [&]() {
				while(true)
				{
					int j = Nx.fetch_add(1);
					if(j >= (int)vSel.size())
						break;
					const SCand &c = vAll[vSel[j]];
					const SState &Par = vBeam[c.m_Parent];
					SState S = Par;
					SNode Nd;
					Nd.m_Parent = Par.m_Node;
					Nd.m_NPatch = 0;
					for(int r = 0; r < c.m_NR; r++)
					{
						ApplyRetro(S, c.m_aR[r]);
						Nd.m_aPatchStep[Nd.m_NPatch] = c.m_aR[r].m_Step;
						Nd.m_aPTX[Nd.m_NPatch] = c.m_aR[r].m_TX;
						Nd.m_aPTY[Nd.m_NPatch] = c.m_aR[r].m_TY;
						Nd.m_NPatch++;
					}
					const int Tk = S.m_G.m_Tick;
					const int Rl0 = S.m_G.m_ReloadTimer;
					S.m_G.Step(c.m_In);
					if(S.m_G.m_ReloadTimer > Rl0)
						AddFire(S, Tk); // a real shot (incumbent pre-fires included)
					S.m_Prev = c.m_In;
					S.m_Inc = c.m_Inc;
					S.m_Node = j;
					S.m_G0 = c.m_G;
					S.m_KInc = c.m_KInc;
					S.m_TCost = c.m_TCost;
					S.m_Brake = c.m_Brake;
					S.m_TK = c.m_TK;
					S.m_HDis = c.m_HDis;
					S.m_Lag = c.m_Lag;
					S.m_E = c.m_E;
					Nd.m_In = c.m_In;
					Nd.m_Pos = S.m_G.m_Pos;
					Nd.m_HookPress = c.m_In.m_Hook && !Par.m_Prev.m_Hook;
					vNodes[j] = Nd;
					vNew[j] = std::move(S);
				}
			};
			std::vector<std::thread> vB;
			for(int T = 0; T < NT; T++)
				vB.emplace_back(Build);
			for(auto &T : vB)
				T.join();
		}
		// survival filter: keep the first Beam surviving states in selection order (node indices stay valid: the
		// history keeps all built nodes)
		if(gs_P.m_Surv > 0)
		{
			std::vector<char> vOk(vNew.size(), 1);
			std::atomic<int> Nx{0};
			auto Chk = [&]() {
				while(true)
				{
					int j = Nx.fetch_add(1);
					if(j >= (int)vNew.size())
						break;
					if(vNew[j].m_Inc)
						continue;
					vOk[j] = Survives(vNew[j].m_G, vNew[j].m_Prev, gs_P.m_Surv);
				}
			};
			std::vector<std::thread> vB;
			for(int T = 0; T < NT; T++)
				vB.emplace_back(Chk);
			for(auto &T : vB)
				T.join();
			std::vector<SState> vKeep;
			vKeep.reserve(gs_P.m_Beam);
			for(size_t j = 0; j < vNew.size() && (int)vKeep.size() < gs_P.m_Beam; j++)
				if(vOk[j])
					vKeep.push_back(std::move(vNew[j]));
			if(vKeep.empty() && !vNew.empty())
				vKeep.push_back(std::move(vNew[0]));
			gs_NSurv = (long)vKeep.size();
			vNew = std::move(vKeep);
		}
		// record the gate winner's lineage now (its node is a candidate, not kept): store as an extra node
		if(Best.m_Step == Step && Best.m_Idx == -1)
		{
			// the winning candidate
			for(size_t k = Best.m_Cand; k < vAll.size(); k++)
				if((int)k == Best.m_Cand)
				{
					const SCand &c = vAll[k];
					const SState &Par = vBeam[c.m_Parent];
					SNode Nd;
					Nd.m_Parent = Par.m_Node;
					Nd.m_NPatch = 0;
					for(int r = 0; r < c.m_NR; r++)
					{
						Nd.m_aPatchStep[Nd.m_NPatch] = c.m_aR[r].m_Step;
						Nd.m_aPTX[Nd.m_NPatch] = c.m_aR[r].m_TX;
						Nd.m_aPTY[Nd.m_NPatch] = c.m_aR[r].m_TY;
						Nd.m_NPatch++;
					}
					Nd.m_In = c.m_In;
					Nd.m_Pos = vec2(0, 0);
					Nd.m_HookPress = c.m_In.m_Hook && !Par.m_Prev.m_Hook;
					Best.m_Idx = (int)vNodes.size();
					vNodes.push_back(Nd);
					break;
				}
		}
		gs_vHist.push_back(std::move(vNodes));
		vBeam = std::move(vNew);
		if(pDump)
		{
			// step header: step, race tick, candidates generated, selected, kept; then one line per kept state:
			// node, parent node (previous step), pos, vel, hook state, hook pos, lag (ticks behind the incumbent), retro kicks
			std::fprintf(pDump, "S %d %d %ld %ld %zu\n", Step, vBeam.empty() ? -1 : vBeam[0].m_G.RaceTick(), DumpCand, gs_NSel,
				vBeam.size());
			for(const SState &S : vBeam)
			{
				const SNode &Nd = gs_vHist.back()[S.m_Node];
				std::fprintf(pDump, "%d %d %.0f %.0f %.2f %.2f %d %.0f %.0f %.3f %d\n", S.m_Node, Nd.m_Parent, S.m_G.m_Pos.x,
					S.m_G.m_Pos.y, S.m_G.m_Core.m_Vel.x, S.m_G.m_Core.m_Vel.y, S.m_G.m_Core.m_HookState, S.m_G.m_Core.m_HookPos.x,
					S.m_G.m_Core.m_HookPos.y, S.m_Lag, (int)Nd.m_NPatch);
			}
		}
		if(gs_P.m_Verbose && (Step % gs_P.m_Verbose == 0 || FirstGate == Step))
		{
			// best by lag (lam 0)
			float BL = 1e9f, BE = 0, BG = 0, BV = 0, IL = 0;
			vec2 BP(0, 0);
			int NMatch = 0;
			for(auto &S : vBeam)
			{
				int ri = S.m_G.RaceTick() - gs_Line.m_Rt0;
				if(ri >= 0 && ri < (int)gs_Line.m_vP.size() && distance(S.m_G.m_Core.m_Pos, gs_Line.m_vP[ri]) < 0.5f)
					NMatch++;
				if(S.m_Lag < BL)
				{
					BL = S.m_Lag;
					BE = S.m_E;
					BG = S.m_G0;
					BV = length(S.m_G.m_Core.m_Vel);
					BP = S.m_G.m_Pos;
				}
				if(S.m_Inc)
					IL = S.m_Lag;
			}
			std::printf("  retro found %ld emitted %ld | fire aims %ld emitted %ld | pre %ld sel %ld surv %ld\n", gs_NRetroFound.load(), gs_NRetroEmit.load(), gs_NFireTry.load(), gs_NFireEmit.load(), gs_NPre, gs_NSel, gs_NSurv);
			std::printf("  on-incumbent states: %d\n", NMatch);
			std::printf("step %4d rt %d beam %zu cand %zu | best lag %.2f geo %.0f E %.0f |v| %.1f pos %.0f %.0f | inc lag %.2f | %.0fs\n", Step,
				vBeam.empty() ? -1 : vBeam[0].m_G.RaceTick(), vBeam.size(), vAll.size(), BL, BG, BE, BV, BP.x, BP.y, IL,
				std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
			std::fflush(stdout);
		}
	}
	if(Best.m_Step < 0)
	{
		std::printf("NOGATE\n");
		return 0;
	}
	// reconstruct
	std::vector<STasInput> vOut(Best.m_Step + 1);
	std::vector<std::pair<int, std::pair<int16_t, int16_t>>> vPatches;
	{
		int St = Best.m_Step, Ix = Best.m_Idx;
		while(St >= 0)
		{
			const SNode &N = gs_vHist[St][Ix];
			vOut[St] = N.m_In;
			for(int p = 0; p < N.m_NPatch; p++)
				vPatches.push_back({N.m_aPatchStep[p], {N.m_aPTX[p], N.m_aPTY[p]}});
			Ix = N.m_Parent;
			St--;
		}
		for(auto &P : vPatches)
		{
			STasInput &In = vOut[P.first];
			In.m_Fire = 1;
			In.m_TX = P.second.first;
			In.m_TY = P.second.second;
		}
	}
	std::vector<STasInput> vFull = vPrefix;
	vFull.insert(vFull.end(), vOut.begin(), vOut.end());
	{
		// trace of the found path from the root state (as searched, boost included)
		std::string Tf = gs_P.m_Out + ".trace";
		FILE *f = std::fopen(Tf.c_str(), "w");
		if(f)
		{
			CFastG F = Root.m_G;
			std::vector<SExplLog> vL;
			for(size_t k = 0; k < vOut.size(); k++)
			{
				vL.clear();
				CFastG::ms_pLog = &vL;
				F.Step(vOut[k]);
				CFastG::ms_pLog = nullptr;
				std::fprintf(f, "T %d %d %.2f %.2f %.3f %.3f %.3f hs %d hp %.0f %.0f in %d %d %d %d %d %d rl %d np %d\n", (int)(vPrefix.size() + k), F.RaceTick(), F.m_Core.m_Pos.x,
					F.m_Core.m_Pos.y, F.m_Core.m_Vel.x, F.m_Core.m_Vel.y, length(F.m_Core.m_Vel), F.m_Core.m_HookState, F.m_Core.m_HookPos.x, F.m_Core.m_HookPos.y, vOut[k].m_Dir,
					vOut[k].m_Jump, vOut[k].m_Hook, vOut[k].m_Fire, vOut[k].m_TX, vOut[k].m_TY, F.m_ReloadTimer, F.m_NumProj);
				for(auto &E : vL)
					std::fprintf(f, "E %d %d ex %.1f %.1f tee %.1f %.1f dist %.1f |f| %.2f\n", (int)(vPrefix.size() + k), E.m_Tick - F.m_StartTick, E.m_E.x, E.m_E.y, E.m_Tee.x, E.m_Tee.y, E.m_Dist, length(E.m_Force));
			}
			std::fclose(f);
		}
	}
	// verify on the full prediction world
	int VerRt = -1;
	float VerG = 0;
	{
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		bool Dead = false;
		int GateRt = -1;
		float VK = Root.m_G0;
		int CommitAt = -1;
		for(size_t n = 0; n < vFull.size(); n++)
		{
			G.Step(vFull[n]);
			if(G.Frozen() || G.EnteredFreeze())
				Dead = true;
			if(G.HasGrenade() && G.m_Tick - G.m_StartTick >= gs_RootRt)
			{
				VK = gs_Line.Project(G.Pos(), VK);
				if(GateRt < 0 && GateG >= 0 && VK >= GateG)
					GateRt = G.m_Tick - G.m_StartTick;
				if(CommitAt < 0 && gs_P.m_CommitK >= 0 && VK >= gs_P.m_CommitK)
					CommitAt = (int)n + 1;
			}
		}
		if(CommitAt > 0)
		{
			std::vector<STasInput> vC(vFull.begin(), vFull.begin() + CommitAt);
			std::string Cf = gs_P.m_Out + ".c";
			WriteInputs(Cf.c_str(), vC);
			std::printf("commit: %d inputs (progress %d reached at rt %d) -> %s\n", CommitAt, gs_P.m_CommitK, CommitAt - G.m_StartTick, Cf.c_str());
		}
		VerRt = G.m_Tick - G.m_StartTick;
		VerG = gs_Geo.At(G.Pos());
		std::printf("verify (CTasGame): gate rt %d end rt %d geo %.0f finish %d dead %d |v| %.1f E %.0f\n", GateRt, VerRt, VerG, G.m_FinishTick >= 0 ? G.m_FinishTick - G.m_StartTick : -1, (int)Dead,
			length(G.Vel()), dot(G.Vel(), G.Vel()) - G.Pos().y);
	}
	WriteInputs(gs_P.m_Out.c_str(), vFull);
	std::printf("stacks: tried %ld emitted %ld | retro found %ld emitted %ld | canpb %ld with pb aims %ld with old retro %ld\n", gs_NStackTry.load(), gs_NStackEmit.load(), gs_NRetroFound.load(), gs_NRetroEmit.load(), gs_NCanPB.load(), gs_NPBAims.load(), gs_NOldR.load());
	std::printf("GATE t %.3f (incumbent %.3f) E %.0f retro patches %zu -> %s (%.0fs)\n", gs_vHist.empty() ? 0.0f : Best.m_T + (GateG >= 0 ? gs_P.m_EGain * (Best.m_E - gs_Line.EnergyAt(GateG)) : 0.0f),
		GateG >= 0 ? GateG : (float)IncFinish, Best.m_E, vPatches.size(), gs_P.m_Out.c_str(),
		std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
	return 0;
}
