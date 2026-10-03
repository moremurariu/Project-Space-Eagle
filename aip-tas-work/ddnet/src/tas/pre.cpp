// Energy beam search. The clock only starts at the start line, so the time before it is free: before the
// start, states are ranked by energy E = v^2 - y (+ credit for unused jumps); every state keeps per-cell
// diversity and a global dominance table prunes cells reached earlier with a better value.
//
// usage: pre <map> key=value...        (see tas-work/NOTES.md for the experiments behind the defaults)
//   common:  beam=20000 angles=64 maxticks=200 threads=4 out=PREFIX top=N prefix=FILE (start after replaying it)
//            cpos=16 cvel=2 (cell size)  dom=1 (dominance)  survive=25 (survival check horizon)
// 1. pre-start crossings (no gatex): every start-line crossing is recorded per bucket (height band, vertical
//    speed band, jumps left) and the top ones are written as prefixes OUT<k>.txt.
//    jw=100 gw=150 (credit for an unused air / ground jump)  ymin=Y (only crossings with y >= Y)
//    rank=E|v|cont (raw energy + 144 per unused jump | speed | energy after a short surviving continuation)
// 2. polish=FILE seconds=S [ymin=]: hill-climb a pre-start prefix for the continuation value; eval=FILE prints it.
// 3. post-start / corridor: gatex=X keeps searching after the start and records states at x > X, ranked by
//    -race_tick + gatelambda*E_eff (gatelambda=0: pure time). Post-start states are ranked by
//    lambda*E_eff + x/vref - race_tick (E_eff = E + pjc*air jump + pgc*grounded; lamdecay=1 fades lambda),
//    or with trref=TRACK by -(race_tick + T_rem(x, E_eff)), T_rem = time along the reference line at the speed
//    the energy gives there; hnow=H covers the next H px at the current speed (best so far: hnow=300).
//    postbeam=N postdir1=1 (only hold right after the start)  postymin/postymax (crossing height band)
//    ghostshare=F ghostmu=M (part of the beam kept near the reference height)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "sim.h"

#include <game/mapitems.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
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
	if(!f)
		return;
	for(const auto &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

struct SPreParams
{
	int m_Beam = 20000;
	int m_Angles = 64;
	int m_MaxTicks = 200;
	int m_Threads = 4;
	float m_JW = 100;
	float m_GW = 150;
	float m_CPos = 16;
	float m_CVel = 2;
	int m_Dom = 1;
	float m_YMin = 0;
	int m_Top = 40;
	std::string m_Rank = "E";
	std::string m_Out = "runs/pre/c";
	std::string m_Prefix;
	float m_HookRange = 420;
	int m_Survive = 25;
	float m_ContSlack = 120;
	// post-start segment: keep searching after the start line up to a gate
	float m_GateX = 0;
	float m_Lambda = 0.09f; // ticks per unit of energy at the gate
	float m_VRef = 25;
	int m_PostBeam = 20000;
	float m_PJC = 130, m_PGC = 160;
	float m_PostYMin = -1e9f, m_PostYMax = 1e9f;
	float m_GateLambda = -1; // energy weight in the gate value (-1 = lambda)
	float m_GateSlack = 2; // gate states this far below the best gate value are not recorded
	int m_GateWait = 2; // stop this many steps after the first gate arrival
	int m_LamDecay = 0; // lambda falls linearly to 0 at the gate (energy late in the segment is worth less)
	std::string m_TrRef; // reference line for the remaining-time model (replaces lambda/vref)
	int m_TrUse = 1; // rank by the time model (0: only load the reference line for the ghost share)
	float m_HNow = 0; // time model: the next HNow px are covered at the current horizontal speed
	int m_PostDir1 = 0; // after the start only hold right (all our corridor runs do)
	float m_GhostShare = 0; // part of the post beam ranked with a penalty for leaving the reference height
	float m_GhostMu = 1; // ticks per tile of height difference
};
static std::atomic<float> s_BestCont{-1e30f};
static std::atomic<float> s_BestGate{-1e30f};
static SPreParams gs_P;

static int JumpsLeft(const CTasGame &G)
{
	return G.Grounded() ? 2 : ((G.Jumped() & 2) ? 0 : 1);
}

static float Energy(const CTasGame &G)
{
	vec2 V = G.Vel();
	float E = dot(V, V) - G.Pos().y;
	int J = JumpsLeft(G);
	if(J >= 1)
		E += gs_P.m_JW;
	if(J == 2)
		E += gs_P.m_GW;
	return E;
}

static int64_t CellKey(const CTasGame &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	int64_t k = (int64_t)std::floor(P.x / gs_P.m_CPos) + 64;
	k = k * 1024 + ((int64_t)std::floor(P.y / gs_P.m_CPos) + 64);
	k = k * 128 + ((int64_t)std::floor(V.x / gs_P.m_CVel) + 64);
	k = k * 128 + ((int64_t)std::floor(V.y / gs_P.m_CVel) + 64);
	int hs = G.HookState();
	int h = hs == 0 || hs == -1 ? 0 : (hs == 5 ? 2 : 1);
	k = k * 3 + h;
	k = k * 3 + JumpsLeft(G);
	k = k * 2 + (G.m_LastJump ? 1 : 0);
	if(h)
	{
		vec2 A = G.HookPos();
		k = k * 64 + ((int64_t)std::floor(A.x / 64) & 63);
		k = k * 64 + ((int64_t)std::floor(A.y / 64) & 63);
	}
	return k;
}

static void HookTargets(const CTasGame &G, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.Pos();
	const float Range = gs_P.m_HookRange + 4.0f * length(G.Vel());
	std::vector<int> vSeen;
	for(int a = 0; a < gs_P.m_Angles; a++)
	{
		float Ang = 2 * pi * a / gs_P.m_Angles;
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
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

static void GenActions(const CTasGame &G, const STasInput &Prev, std::vector<STasInput> &vOut)
{
	vOut.clear();
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vHooks;
	const bool Hooking = G.m_LastHook;
	if(!Hooking)
		HookTargets(G, s_vHooks);
	const bool CanJump = !G.m_LastJump && (G.Grounded() || !(G.Jumped() & 2));
	const int DirLo = gs_P.m_PostDir1 && G.m_Started ? 1 : -1;
	for(int Dir = DirLo; Dir <= 1; Dir++)
		for(int Jump = 0; Jump <= (CanJump ? 1 : 0); Jump++)
		{
			STasInput In;
			In.m_Dir = Dir;
			In.m_Jump = Jump;
			In.m_Weapon = -1;
			In.m_TX = Prev.m_TX;
			In.m_TY = Prev.m_TY;
			In.m_Hook = Hooking;
			vOut.push_back(In);
			if(Hooking)
			{
				In.m_Hook = 0;
				vOut.push_back(In);
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
		}
}

// can the tee stay out of freeze for N ticks with a simple continuation (hold a direction, keep/release
// the hook, or fire a new hook towards a solid tile)?
static bool Survives(const CTasGame &G, int N)
{
	if(G.Frozen() || G.EnteredFreeze())
		return false;
	vec2 aPos[128];
	N = std::min(N, 128);
	for(int d = 1; d >= -1; d--)
	{
		if(G.Rollout(N, d, true, aPos) == N)
			return true;
		if(G.m_LastHook && G.Rollout(N, d, false, aPos) == N)
			return true;
	}
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vHooks;
	HookTargets(G, s_vHooks);
	for(auto [TX, TY] : s_vHooks)
		for(int d = 1; d >= -1; d--)
			if(G.RolloutHook(N, d, TX, TY, aPos) == N)
				return true;
	return false;
}

struct SCand
{
	float m_E;
	int m_Parent;
	STasInput m_In;
	int64_t m_Key;
	uint64_t m_Hash;
	bool m_Post = false;
	float m_X = 0, m_Ee = 0;
	int m_Rt = 0;
	float m_G = 0; // ghost-share ranking value
};

// remaining-time model: time to the gate along a reference line (x -> y, arc length) at the speed the
// energy gives there, v = sqrt(E + y_ref); a table over (x, E)
static const float TR_X0 = 900, TR_DX = 8, TR_E0 = -300, TR_DE = 10;
static int gs_TrNX = 0, gs_TrNE = 0;
static std::vector<float> gs_vTr;
static std::vector<float> gs_vRefY; // reference height per x step
static float RefY(float X)
{
	int i = std::clamp((int)((X - TR_X0) / TR_DX), 0, (int)gs_vRefY.size() - 1);
	return gs_vRefY[i];
}
static float Disp(float v)
{
	return v > 11 ? v * std::pow(1.4f, -(50 * v - 550) / 2000.0f) : v;
}
static bool BuildTr(const char *pRef, float GateX)
{
	std::vector<std::pair<float, float>> vP;
	FILE *f = std::fopen(pRef, "r");
	if(!f)
		return false;
	char aLine[256];
	while(std::fgets(aLine, sizeof(aLine), f))
	{
		int t;
		float x, y;
		if(std::sscanf(aLine, "%d %f %f", &t, &x, &y) == 3 && t >= -5)
			vP.push_back({x, y});
	}
	std::fclose(f);
	// smooth (the track is ~10 px noisy)
	std::vector<std::pair<float, float>> vS(vP.size());
	for(size_t i = 0; i < vP.size(); i++)
	{
		float sx = 0, sy = 0;
		int n = 0;
		for(int k = -3; k <= 3; k++)
		{
			int j = std::clamp((int)i + k, 0, (int)vP.size() - 1);
			sx += vP[j].first;
			sy += vP[j].second;
			n++;
		}
		vS[i] = {sx / n, sy / n};
	}
	gs_TrNX = (int)((GateX - TR_X0) / TR_DX) + 2;
	gs_TrNE = 260;
	std::vector<float> vY(gs_TrNX, 0), vDs(gs_TrNX, TR_DX);
	for(int i = 0; i < gs_TrNX; i++)
	{
		float X = TR_X0 + i * TR_DX;
		// first track segment covering X
		size_t k = 0;
		while(k + 1 < vS.size() && vS[k + 1].first < X)
			k++;
		if(k + 1 >= vS.size())
			k = vS.size() - 2;
		auto A = vS[k], B = vS[k + 1];
		float dx = B.first - A.first;
		float t = dx > 1e-3f ? std::clamp((X - A.first) / dx, 0.0f, 1.0f) : 0.0f;
		vY[i] = A.second + t * (B.second - A.second);
		float slope = dx > 1e-3f ? (B.second - A.second) / dx : 0.0f;
		vDs[i] = TR_DX * std::sqrt(1 + slope * slope);
	}
	gs_vRefY = vY;
	gs_vTr.assign((size_t)gs_TrNX * gs_TrNE, 0);
	for(int e = 0; e < gs_TrNE; e++)
	{
		float E = TR_E0 + e * TR_DE;
		float Acc = 0;
		for(int i = gs_TrNX - 1; i >= 0; i--)
		{
			float X = TR_X0 + i * TR_DX;
			if(X < GateX)
				Acc += vDs[i] / Disp(std::sqrt(std::max(E + vY[i], 25.0f)));
			gs_vTr[(size_t)i * gs_TrNE + e] = Acc;
		}
	}
	return true;
}
static float TRem(float X, float E)
{
	float fi = std::clamp((X - TR_X0) / TR_DX, 0.0f, (float)gs_TrNX - 1.001f);
	float fe = std::clamp((E - TR_E0) / TR_DE, 0.0f, (float)gs_TrNE - 1.001f);
	int i = (int)fi, e = (int)fe;
	float a = fi - i, b = fe - e;
	auto At = [&](int ii, int ee) { return gs_vTr[(size_t)ii * gs_TrNE + ee]; };
	return (1 - a) * ((1 - b) * At(i, e) + b * At(i, e + 1)) + a * ((1 - b) * At(i + 1, e) + b * At(i + 1, e + 1));
}

// post-start mode: energy with credits for the jumps still available
static float EffEnergy(const CTasGame &G)
{
	vec2 V = G.Vel();
	float E = dot(V, V) - G.Pos().y;
	int J = JumpsLeft(G);
	if(J >= 1)
		E += gs_P.m_PJC;
	if(J == 2)
		E += gs_P.m_PGC;
	return E;
}

struct SCross
{
	float m_E = -1e30f; // ranking value
	int m_Step = -1, m_Parent = -1;
	STasInput m_In;
	vec2 m_Pos, m_Vel;
	int m_Jumps = 0, m_Hook = 0;
	float m_Raw = 0; // v^2 - y
};

// ---- polish: local search on a pre-start prefix, maximizing the crossing value ----
// value = best (v^2 - y + jv*jumps_left) after a short surviving continuation from the crossing
static float gs_JV = 144;
static int gs_ContTicks = 20;
static int gs_ContAngles = 32;

static float ContinueValue(const CTasGame &G0)
{
	static thread_local CTasGame T;
	float Best = -1e30f;
	auto Val = [](const CTasGame &G) {
		vec2 V = G.Vel();
		return dot(V, V) - G.Pos().y + gs_JV * (JumpsLeft(G) >= 1 ? 1 : 0);
	};
	for(int a = -1; a < gs_ContAngles; a++)
		for(int Rel = 0; Rel <= (a >= 0 ? 1 : 0); Rel++)
		{
			T.CopyFrom(G0);
			STasInput In;
			In.m_Dir = 1;
			In.m_Weapon = -1;
			if(a >= 0)
			{
				float Ang = 2 * pi * a / gs_ContAngles;
				In.m_Hook = 1;
				In.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
				In.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
				if(!In.m_TX && !In.m_TY)
					In.m_TY = -1;
			}
			bool Dead = false;
			int Hold = Rel ? 6 : gs_ContTicks;
			for(int k = 0; k < gs_ContTicks; k++)
			{
				if(k == 0 && G0.m_LastHook && a >= 0)
				{
					// re-press: release for one tick first
					STasInput R = In;
					R.m_Hook = 0;
					T.Step(R);
				}
				else
				{
					STasInput I2 = In;
					if(k >= Hold)
						I2.m_Hook = 0;
					T.Step(I2);
				}
				if(T.Frozen() || T.EnteredFreeze() || T.m_StartTick == -2)
				{
					Dead = true;
					break;
				}
			}
			if(!Dead)
				Best = std::max(Best, Val(T));
		}
	return Best;
}

struct SPolishRes
{
	float m_Val = -1e30f;
	int m_CrossLen = -1;
	vec2 m_Pos, m_Vel;
	int m_Jumps = 0;
	float m_Raw = 0;
};

static SPolishRes EvalPrefix(const std::vector<STasInput> &v, int Extra)
{
	SPolishRes R;
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	int n = (int)v.size();
	for(int i = 0; i < n + Extra; i++)
	{
		STasInput In = v[std::min(i, n - 1)];
		G.Step(In);
		if(G.Frozen() || G.EnteredFreeze() || G.m_StartTick == -2)
			return R;
		if(G.m_Started)
		{
			R.m_CrossLen = i + 1;
			R.m_Pos = G.Pos();
			R.m_Vel = G.Vel();
			R.m_Jumps = JumpsLeft(G);
			R.m_Raw = dot(R.m_Vel, R.m_Vel) - R.m_Pos.y;
			if(R.m_Pos.y < gs_P.m_YMin)
				return R;
			R.m_Val = ContinueValue(G);
			return R;
		}
	}
	return R;
}

static uint64_t gs_Rng = 88172645463325252ull;
static uint64_t Rnd()
{
	gs_Rng ^= gs_Rng << 13;
	gs_Rng ^= gs_Rng >> 7;
	gs_Rng ^= gs_Rng << 17;
	return gs_Rng;
}
static int RndInt(int n) { return (int)(Rnd() % (uint64_t)n); }

static void Mutate(std::vector<STasInput> &v)
{
	int n = (int)v.size();
	int Kind = RndInt(6);
	int t = RndInt(n);
	if(Kind == 0 || Kind == 1)
	{
		// rotate the aim of the hook press run containing t
		if(!v[t].m_Hook)
			return;
		int a = t, b = t;
		while(a > 0 && v[a - 1].m_Hook && v[a - 1].m_TX == v[t].m_TX && v[a - 1].m_TY == v[t].m_TY)
			a--;
		while(b + 1 < n && v[b + 1].m_Hook && v[b + 1].m_TX == v[t].m_TX && v[b + 1].m_TY == v[t].m_TY)
			b++;
		float Ang = std::atan2((float)v[t].m_TY, (float)v[t].m_TX);
		float d = (Kind == 0 ? 0.5f : 4.0f) * (pi / 180) * ((float)RndInt(2001) / 1000.0f - 1.0f);
		Ang += d;
		int16_t TX = (int16_t)std::lround(std::cos(Ang) * 1000), TY = (int16_t)std::lround(std::sin(Ang) * 1000);
		// keep the aim on the following non-hook ticks consistent (it only matters while hooking)
		for(int k = a; k <= b; k++)
		{
			v[k].m_TX = TX;
			v[k].m_TY = TY;
		}
	}
	else if(Kind == 2)
	{
		// move a hook press/release boundary by one tick
		if(t == 0 || v[t].m_Hook == v[t - 1].m_Hook)
			return;
		if(RndInt(2))
			v[t - 1] = v[t]; // boundary earlier
		else
			v[t] = v[t - 1]; // boundary later
	}
	else if(Kind == 3)
	{
		// change the direction on a short run
		int L = 1 + RndInt(4);
		int d = RndInt(3) - 1;
		for(int k = t; k < std::min(n, t + L); k++)
			v[k].m_Dir = d;
	}
	else if(Kind == 4)
	{
		// move a jump press by one tick
		if(t == 0 || v[t].m_Jump == v[t - 1].m_Jump)
			return;
		if(RndInt(2))
			v[t - 1].m_Jump = v[t].m_Jump;
		else
			v[t].m_Jump = v[t - 1].m_Jump;
	}
	else
	{
		// release the hook for one tick and re-press with a nearby aim
		if(!v[t].m_Hook || t + 1 >= n)
			return;
		v[t].m_Hook = 0;
		float Ang = std::atan2((float)v[t + 1].m_TY, (float)v[t + 1].m_TX) + (pi / 180) * (float)(RndInt(41) - 20);
		int16_t TX = (int16_t)std::lround(std::cos(Ang) * 1000), TY = (int16_t)std::lround(std::sin(Ang) * 1000);
		for(int k = t + 1; k < n && v[k].m_Hook; k++)
		{
			v[k].m_TX = TX;
			v[k].m_TY = TY;
		}
	}
}

static int Polish(const char *pIn, const char *pOut, double Seconds, int Threads)
{
	std::vector<STasInput> vBest = ReadInputs(pIn);
	SPolishRes Best = EvalPrefix(vBest, 0);
	if(Best.m_CrossLen < 0)
	{
		std::printf("input does not cross\n");
		return 1;
	}
	vBest.resize(Best.m_CrossLen);
	std::printf("start: val %.1f raw %.1f len %d pos %.1f %.1f vel %.2f %.2f jumps %d\n", Best.m_Val, Best.m_Raw, Best.m_CrossLen, Best.m_Pos.x, Best.m_Pos.y, Best.m_Vel.x, Best.m_Vel.y, Best.m_Jumps);
	auto t0 = std::chrono::steady_clock::now();
	std::mutex Mx;
	std::atomic<long> Evals{0};
	auto W = [&](int Seed) {
		uint64_t Local = 0x9E3779B97F4A7C15ull * (Seed + 1);
		while(std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() < Seconds)
		{
			std::vector<STasInput> v;
			float BestVal;
			{
				std::lock_guard<std::mutex> L(Mx);
				v = vBest;
				BestVal = Best.m_Val;
				gs_Rng ^= Local;
				Local = Rnd();
			}
			int NM = 1 + (int)(Local % 3);
			for(int m = 0; m < NM; m++)
			{
				std::lock_guard<std::mutex> L(Mx);
				Mutate(v);
			}
			SPolishRes R = EvalPrefix(v, 30);
			Evals++;
			if(R.m_CrossLen < 0 || R.m_Val < BestVal)
				continue;
			std::lock_guard<std::mutex> L(Mx);
			if(R.m_Val > Best.m_Val || (R.m_Val == Best.m_Val && R.m_Raw > Best.m_Raw))
			{
				bool Better = R.m_Val > Best.m_Val + 0.05f;
				// extend with the repeated last input if the crossing moved later
				while((int)v.size() < R.m_CrossLen)
					v.push_back(v.back());
				v.resize(R.m_CrossLen);
				vBest = v;
				Best = R;
				if(Better)
				{
					WriteInputs(pOut, vBest);
					std::printf("val %.1f raw %.1f len %d pos %.1f %.1f vel %.2f %.2f jumps %d (%ld evals, %.0fs)\n", R.m_Val, R.m_Raw, R.m_CrossLen, R.m_Pos.x, R.m_Pos.y, R.m_Vel.x, R.m_Vel.y, R.m_Jumps, Evals.load(),
						std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
					std::fflush(stdout);
				}
			}
		}
	};
	std::vector<std::thread> vTh;
	for(int T = 1; T < Threads; T++)
		vTh.emplace_back(W, T);
	W(0);
	for(auto &Th : vTh)
		Th.join();
	WriteInputs(pOut, vBest);
	std::printf("done: val %.1f raw %.1f len %d (%ld evals)\n", Best.m_Val, Best.m_Raw, Best.m_CrossLen, Evals.load());
	return 0;
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: pre <map> key=value...\n");
		return 1;
	}
	std::string PolishIn, EvalIn;
	double PolishSec = 60;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
		{
			std::printf("bad arg %s\n", argv[i]);
			return 1;
		}
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "beam") gs_P.m_Beam = std::stoi(V);
		else if(K == "angles") gs_P.m_Angles = std::stoi(V);
		else if(K == "maxticks") gs_P.m_MaxTicks = std::stoi(V);
		else if(K == "threads") gs_P.m_Threads = std::stoi(V);
		else if(K == "jw") gs_P.m_JW = std::stof(V);
		else if(K == "gw") gs_P.m_GW = std::stof(V);
		else if(K == "cpos") gs_P.m_CPos = std::stof(V);
		else if(K == "cvel") gs_P.m_CVel = std::stof(V);
		else if(K == "dom") gs_P.m_Dom = std::stoi(V);
		else if(K == "ymin") gs_P.m_YMin = std::stof(V);
		else if(K == "top") gs_P.m_Top = std::stoi(V);
		else if(K == "rank") gs_P.m_Rank = V;
		else if(K == "out") gs_P.m_Out = V;
		else if(K == "prefix") gs_P.m_Prefix = V;
		else if(K == "hookrange") gs_P.m_HookRange = std::stof(V);
		else if(K == "survive") gs_P.m_Survive = std::stoi(V);
		else if(K == "contslack") gs_P.m_ContSlack = std::stof(V);
		else if(K == "gatex") gs_P.m_GateX = std::stof(V);
		else if(K == "lambda") gs_P.m_Lambda = std::stof(V);
		else if(K == "vref") gs_P.m_VRef = std::stof(V);
		else if(K == "postbeam") gs_P.m_PostBeam = std::stoi(V);
		else if(K == "pjc") gs_P.m_PJC = std::stof(V);
		else if(K == "pgc") gs_P.m_PGC = std::stof(V);
		else if(K == "postymin") gs_P.m_PostYMin = std::stof(V);
		else if(K == "gatelambda") gs_P.m_GateLambda = std::stof(V);
		else if(K == "gateslack") gs_P.m_GateSlack = std::stof(V);
		else if(K == "gatewait") gs_P.m_GateWait = std::stoi(V);
		else if(K == "lamdecay") gs_P.m_LamDecay = std::stoi(V);
		else if(K == "trref") gs_P.m_TrRef = V;
		else if(K == "truse") gs_P.m_TrUse = std::stoi(V);
		else if(K == "hnow") gs_P.m_HNow = std::stof(V);
		else if(K == "postdir1") gs_P.m_PostDir1 = std::stoi(V);
		else if(K == "ghostshare") gs_P.m_GhostShare = std::stof(V);
		else if(K == "ghostmu") gs_P.m_GhostMu = std::stof(V);
		else if(K == "postymax") gs_P.m_PostYMax = std::stof(V);
		else if(K == "polish") PolishIn = V;
		else if(K == "seconds") PolishSec = std::stod(V);
		else if(K == "jv") gs_JV = std::stof(V);
		else if(K == "cont") gs_ContTicks = std::stoi(V);
		else if(K == "eval") EvalIn = V;
		else
		{
			std::printf("unknown option %s\n", K.c_str());
			return 1;
		}
	}
	if(gs_P.m_GateLambda < 0)
		gs_P.m_GateLambda = gs_P.m_Lambda;
	if(!gs_P.m_TrRef.empty())
	{
		if(!BuildTr(gs_P.m_TrRef.c_str(), gs_P.m_GateX))
		{
			std::printf("cannot read %s\n", gs_P.m_TrRef.c_str());
			return 1;
		}
		std::printf("time model: E=300 -> %.1f ticks, E=600 -> %.1f, E=900 -> %.1f from x=960\n", TRem(960, 300), TRem(960, 600), TRem(960, 900));
	}
	if(!EvalIn.empty())
	{
		auto v = ReadInputs(EvalIn.c_str());
		SPolishRes R = EvalPrefix(v, 0);
		std::printf("EVAL val %.1f raw %.1f len %d pos %.1f %.1f vel %.2f %.2f jumps %d\n", R.m_Val, R.m_Raw, R.m_CrossLen, R.m_Pos.x, R.m_Pos.y, R.m_Vel.x, R.m_Vel.y, R.m_Jumps);
		return 0;
	}
	if(!PolishIn.empty())
		return Polish(PolishIn.c_str(), gs_P.m_Out.c_str(), PolishSec, gs_P.m_Threads);
	auto t0 = std::chrono::steady_clock::now();
	std::vector<STasInput> vPrefix;
	if(!gs_P.m_Prefix.empty())
		vPrefix = ReadInputs(gs_P.m_Prefix.c_str());

	std::vector<std::unique_ptr<CTasGame>> vBeam;
	std::vector<STasInput> vPrev;
	{
		auto G = std::make_unique<CTasGame>();
		G->Spawn(CTasGame::Map().m_vSpawns[0]);
		for(const auto &In : vPrefix)
			G->Step(In);
		vPrev.push_back(vPrefix.empty() ? STasInput{} : vPrefix.back());
		vBeam.push_back(std::move(G));
	}
	std::vector<std::vector<std::pair<int, STasInput>>> vHist;
	std::unordered_map<int64_t, float> Dom;
	// best crossing per bucket
	std::unordered_map<int64_t, SCross> Cross;
	const int NT = std::max(1, gs_P.m_Threads);

	int FirstGateStep = -1;
	for(int Step = 0; Step < gs_P.m_MaxTicks && !vBeam.empty(); Step++)
	{
		if(gs_P.m_GateX > 0 && FirstGateStep >= 0 && Step > FirstGateStep + gs_P.m_GateWait)
			break;
		std::vector<std::vector<SCand>> vTC(NT);
		std::vector<std::vector<SCross>> vTX(NT);
		std::atomic<int> Next{0};
		auto Worker = [&](int T) {
			CTasGame Tmp;
			std::vector<STasInput> vActs;
			while(true)
			{
				int i = Next.fetch_add(1);
				if(i >= (int)vBeam.size())
					break;
				const CTasGame &G = *vBeam[i];
				GenActions(G, vPrev[i], vActs);
				for(const auto &In : vActs)
				{
					Tmp.CopyFrom(G);
					Tmp.Step(In);
					if(Tmp.Frozen() || Tmp.EnteredFreeze() || Tmp.m_StartTick == -2)
						continue;
					if(Tmp.m_Started && gs_P.m_GateX > 0)
					{
						const int Rt = Tmp.m_Tick - Tmp.m_StartTick;
						const float Ee = EffEnergy(Tmp);
						vec2 P = Tmp.Pos(), V = Tmp.Vel();
						if(!G.m_Started && (P.y < gs_P.m_PostYMin || P.y > gs_P.m_PostYMax))
							continue; // crossing outside the wanted height band
						if(P.x > gs_P.m_GateX)
						{
							SCross C;
							C.m_Pos = P;
							C.m_Vel = V;
							C.m_Jumps = JumpsLeft(Tmp);
							C.m_Hook = Tmp.HookState();
							C.m_Raw = Ee;
							float Over = V.x > 1 ? (P.x - gs_P.m_GateX) / V.x : 0.0f;
							C.m_E = -((float)Rt - Over) + gs_P.m_GateLambda * Ee;
							C.m_Step = Step;
							C.m_Parent = i;
							C.m_In = In;
							// only near-best gate states are worth the (costly) survival check
							if(C.m_E >= s_BestGate.load() - gs_P.m_GateSlack && Survives(Tmp, gs_P.m_Survive))
							{
								vTX[T].push_back(C);
								float Old = s_BestGate.load();
								while(C.m_E > Old && !s_BestGate.compare_exchange_weak(Old, C.m_E))
									;
							}
							continue;
						}
						float Lam = gs_P.m_Lambda;
						if(gs_P.m_LamDecay)
							Lam *= std::clamp((gs_P.m_GateX - P.x) / (gs_P.m_GateX - 960.0f), 0.0f, 1.0f);
						float Sc = Lam * Ee + P.x / gs_P.m_VRef - (float)Rt;
						if(!gs_vTr.empty() && gs_P.m_TrUse)
						{
							if(gs_P.m_HNow > 0)
							{
								// the next HNow px at the current speed, the rest at the speed the energy gives
								float Vn = std::max(Disp(length(V)) * (V.x > 0 ? V.x / std::max(length(V), 1e-3f) : 0.0f), 5.0f);
								float H = std::min(gs_P.m_HNow, std::max(gs_P.m_GateX - P.x, 0.0f));
								Sc = -(float)Rt - (H / Vn + TRem(P.x + H, Ee));
							}
							else
								Sc = -(float)Rt - TRem(P.x, Ee);
						}
						SCand Cd{Sc, i, In, CellKey(Tmp) * 2 + 1, Tmp.Hash()};
						Cd.m_Post = true;
						Cd.m_X = P.x;
						Cd.m_G = Sc - (gs_vRefY.empty() ? 0.0f : gs_P.m_GhostMu * std::fabs(P.y - RefY(P.x)) / 32.0f);
						Cd.m_Ee = Ee;
						Cd.m_Rt = Rt;
						vTC[T].push_back(Cd);
						continue;
					}
					if(Tmp.m_Started)
					{
						SCross C;
						C.m_Pos = Tmp.Pos();
						C.m_Vel = Tmp.Vel();
						C.m_Jumps = JumpsLeft(Tmp);
						C.m_Hook = Tmp.HookState();
						C.m_Raw = dot(C.m_Vel, C.m_Vel) - C.m_Pos.y;
						C.m_E = gs_P.m_Rank == "v" ? length(C.m_Vel) : C.m_Raw + (C.m_Jumps >= 1 ? 144.0f : 0.0f);
						C.m_Step = Step;
						C.m_Parent = i;
						C.m_In = In;
						if(gs_P.m_Rank == "cont")
						{
							// the continuation value is at most the raw value: only evaluate promising crossings
							if(C.m_Pos.y < gs_P.m_YMin || C.m_E < s_BestCont.load() - gs_P.m_ContSlack)
								continue;
							C.m_E = ContinueValue(Tmp);
							if(C.m_E > -1e29f)
							{
								vTX[T].push_back(C);
								float Old = s_BestCont.load();
								while(C.m_E > Old && !s_BestCont.compare_exchange_weak(Old, C.m_E))
									;
							}
							continue;
						}
						if(C.m_Pos.y >= gs_P.m_YMin && Survives(Tmp, gs_P.m_Survive))
							vTX[T].push_back(C);
						continue;
					}
					vTC[T].push_back({Energy(Tmp), i, In, CellKey(Tmp) * 2, Tmp.Hash()});
				}
			}
		};
		std::vector<std::thread> vTh;
		for(int T = 1; T < NT; T++)
			vTh.emplace_back(Worker, T);
		Worker(0);
		for(auto &Th : vTh)
			Th.join();

		// crossings: keep the best per bucket
		int NewCross = 0;
		if(gs_P.m_GateX > 0 && FirstGateStep < 0)
			for(auto &v : vTX)
				if(!v.empty())
					FirstGateStep = Step;
		for(auto &v : vTX)
			for(auto &C : v)
			{
				int64_t Key = (int64_t)std::floor(C.m_Pos.y / 16);
				Key = Key * 64 + ((int)std::floor(C.m_Vel.y / 3) + 32);
				Key = Key * 3 + C.m_Jumps;
				auto &S = Cross[Key];
				if(C.m_E > S.m_E)
				{
					S = C;
					NewCross++;
				}
			}

		std::vector<SCand> vAll;
		for(auto &v : vTC)
			vAll.insert(vAll.end(), v.begin(), v.end());
		std::sort(vAll.begin(), vAll.end(), [](const SCand &a, const SCand &b) { return a.m_E != b.m_E ? a.m_E > b.m_E : a.m_Hash < b.m_Hash; }); // hash tie-break: same order whatever the thread scheduling
		std::unordered_set<uint64_t> Seen;
		std::unordered_set<int64_t> Cells;
		std::vector<SCand> vSel;
		int NPre = 0, NPost = 0;
		for(const auto &C : vAll)
		{
			if(NPre >= gs_P.m_Beam && NPost >= gs_P.m_PostBeam)
				break;
			if(C.m_Post ? NPost >= gs_P.m_PostBeam : NPre >= gs_P.m_Beam)
				continue;
			if(!Seen.insert(C.m_Hash).second)
				continue;
			if(!Cells.insert(C.m_Key).second)
				continue;
			if(gs_P.m_Dom)
			{
				auto it = Dom.find(C.m_Key);
				if(it != Dom.end() && it->second >= C.m_E - 0.5f)
					continue;
			}
			vSel.push_back(C);
			(C.m_Post ? NPost : NPre)++;
		}
		if(gs_P.m_GhostShare > 0)
		{
			// reserve part of the post beam for the reference-height ranking: drop the weakest post
			// states picked by score and refill by the ghost value
			int Want = (int)(gs_P.m_GhostShare * gs_P.m_PostBeam);
			std::vector<SCand> vKeep;
			int Post = 0, Lim = gs_P.m_PostBeam - Want;
			for(const auto &C : vSel)
			{
				if(C.m_Post && Post++ >= Lim)
				{
					Cells.erase(C.m_Key);
					Seen.erase(C.m_Hash);
					continue;
				}
				vKeep.push_back(C);
			}
			vSel = std::move(vKeep);
			std::vector<const SCand *> vG;
			for(const auto &C : vAll)
				if(C.m_Post)
					vG.push_back(&C);
			std::sort(vG.begin(), vG.end(), [](const SCand *a, const SCand *b) { return a->m_G != b->m_G ? a->m_G > b->m_G : a->m_Hash < b->m_Hash; });
			int Added = 0;
			for(const SCand *pC : vG)
			{
				if(Added >= Want)
					break;
				if(Seen.count(pC->m_Hash) || Cells.count(pC->m_Key))
					continue;
				if(gs_P.m_Dom)
				{
					auto it = Dom.find(pC->m_Key);
					if(it != Dom.end() && it->second >= pC->m_E - 0.5f)
						continue;
				}
				Seen.insert(pC->m_Hash);
				Cells.insert(pC->m_Key);
				vSel.push_back(*pC);
				Added++;
			}
		}
		for(const auto &C : vSel)
		{
			float &D = Dom.try_emplace(C.m_Key, -1e30f).first->second;
			D = std::max(D, C.m_E);
		}
		// materialize the new beam
		std::vector<std::unique_ptr<CTasGame>> vNew(vSel.size());
		std::vector<STasInput> vNewPrev(vSel.size());
		std::atomic<int> Next2{0};
		auto Mat = [&]() {
			while(true)
			{
				int k = Next2.fetch_add(1);
				if(k >= (int)vSel.size())
					break;
				vNew[k] = std::make_unique<CTasGame>();
				vNew[k]->CopyFrom(*vBeam[vSel[k].m_Parent]);
				vNew[k]->Step(vSel[k].m_In);
				vNewPrev[k] = vSel[k].m_In;
			}
		};
		std::vector<std::thread> vTh2;
		for(int T = 1; T < NT; T++)
			vTh2.emplace_back(Mat);
		Mat();
		for(auto &Th : vTh2)
			Th.join();
		std::vector<std::pair<int, STasInput>> H(vSel.size());
		for(size_t k = 0; k < vSel.size(); k++)
			H[k] = {vSel[k].m_Parent, vSel[k].m_In};
		vHist.push_back(std::move(H));
		vBeam = std::move(vNew);
		vPrev = std::move(vNewPrev);

		float BestCross = -1e30f;
		for(auto &[k, C] : Cross)
			BestCross = std::max(BestCross, C.m_E);
		double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
		{
			const SCand *pBestX = nullptr, *pBestS = nullptr;
			for(const auto &C : vSel)
				if(C.m_Post)
				{
					if(!pBestX || C.m_X > pBestX->m_X)
						pBestX = &C;
					if(!pBestS || C.m_E > pBestS->m_E)
						pBestS = &C;
				}
			if(pBestX && Step % 5 == 0)
				std::printf("  post: maxx %.0f (rt %d Ee %.0f)  best S %.1f at x %.0f rt %d Ee %.0f\n", pBestX->m_X, pBestX->m_Rt, pBestX->m_Ee, pBestS->m_E, pBestS->m_X, pBestS->m_Rt, pBestS->m_Ee);
		}
		if(Step % 5 == 0 || NewCross)
			std::printf("step %d beam %zu (post %d) cands %zu bestE %.1f cross buckets %zu new %d bestcross %.1f (%.0fs)\n", Step, vBeam.size(), NPost, vAll.size(),
				vSel.empty() ? 0.0f : vSel[0].m_E, Cross.size(), NewCross, BestCross, Sec);
		std::fflush(stdout);
	}

	// write the top crossings
	std::vector<const SCross *> vC;
	for(auto &[k, C] : Cross)
		vC.push_back(&C);
	std::sort(vC.begin(), vC.end(), [](const SCross *a, const SCross *b) { return a->m_E > b->m_E; });
	for(int r = 0; r < (int)vC.size() && r < gs_P.m_Top; r++)
	{
		const SCross &C = *vC[r];
		std::vector<STasInput> vRun;
		vRun.push_back(C.m_In);
		int Idx = C.m_Parent;
		for(int s = C.m_Step - 1; s >= 0; s--)
		{
			vRun.push_back(vHist[s][Idx].second);
			Idx = vHist[s][Idx].first;
		}
		std::reverse(vRun.begin(), vRun.end());
		std::vector<STasInput> vAllIn = vPrefix;
		vAllIn.insert(vAllIn.end(), vRun.begin(), vRun.end());
		char aBuf[512];
		std::snprintf(aBuf, sizeof(aBuf), "%s%d.txt", gs_P.m_Out.c_str(), r);
		WriteInputs(aBuf, vAllIn);
		// verify by replay
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(const auto &In : vAllIn)
			G.Step(In);
		std::printf("CROSS %d rank %.1f E %.1f tick %d pos %.1f %.1f vel %.2f %.2f |v| %.2f jumps %d hook %d -> %s [replay pos %.1f %.1f started %d]\n", r, C.m_E, C.m_Raw, (int)vAllIn.size(), C.m_Pos.x, C.m_Pos.y,
			C.m_Vel.x, C.m_Vel.y, length(C.m_Vel), C.m_Jumps, C.m_Hook, aBuf, G.Pos().x, G.Pos().y, G.m_Started);
	}
	return 0;
}
