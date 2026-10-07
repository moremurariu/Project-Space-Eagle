// hp: hairpin beam search (physlab3 experiment tool, single thread).
// Starts from the state after `cut` lines of `prefix` and beam-searches per-tick inputs until the tee crosses
// x <= gatex moving left (the gate). Ranking = estimated race tick at the gate, ghost model on Teero's track:
//   T = rt + (K_gate - k(s)) - tT(s,H) + H / v_along
// k(s) = Teero's (fractional) tick at the projection s of our position on his smoothed track, tT(s,H) = his time
// over the next H px, v_along = our displacement speed along his tangent (+ credits), clamped to [vmin, vcap*H/tT].
// Credits: pending grenades (in flight) count with the kick they would give at the ballistic prediction of our
// position on the tick before they explode (x kg / (1 + dt/kdecay)); reload 0 counts cr px/t.
// Shots: fireang aims for the best `firetop` nodes with reload 0; a shot is kept if it exploded within pbmax ticks
// (point-blank) or its explosion lies in the ROI box within fmax ticks (pre-fire). Different pending explosions are
// kept apart by the dedup key and get a reserved share (minper nodes for each of the best maxkeys explosion keys).
// usage: hp <map> prefix=FILE cut=N [beam=3000 maxt=100 gatex=7700 out=PREFIX nout=5 H=300 vcap=1.3 vmin=4 cr=3
//        kg=1 kdecay=8 hookang=32 fireang=72 firetop=400 pbmax=2 fmax=40 roi=x0,y0,x1,y1 survive=25 survevery=1
//        cpos=8 cvel=1.5 minper=8 maxkeys=150 jump=1 lat=0 latdz=40 fixed=FILE fixedn=M nofire=0 ref=PATH]
//   fixed=FILE fixedn=M: the first M steps after the cut are forced to FILE's lines cut..cut+M-1
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/entities/projectile.h>
#include <game/client/prediction/gameworld.h>
#include "../../ddnet/src/tas/sim.h"
#undef private
#undef protected

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct SPar
{
	std::string m_Prefix, m_Out = "hp_out", m_Ref = "../teero_track.txt", m_Fixed;
	int m_Cut = 0, m_Beam = 3000, m_MaxT = 100, m_NOut = 5, m_HookAng = 32, m_FireAng = 72, m_FireTop = 400, m_PbMax = 2,
	    m_FMax = 40, m_Survive = 25, m_SurvEvery = 1, m_MinPer = 8, m_MaxKeys = 150, m_Jump = 1, m_FixedN = 0, m_NoFire = 0,
	    m_FireFrom = 0, m_FireTo = 1 << 30, m_Fire2 = 1 << 30;
	float m_GateX = 7700, m_GateY1 = 2098, m_GateY0 = 2040, m_GateVx = 15, m_H = 300, m_VCap = 1.3f, m_VMin = 4, m_Cr = 3, m_Kg = 1, m_KDecay = 8, m_CPos = 8, m_CVel = 1.5f,
	      m_HookRange = 400, m_Lat = 0, m_LatDz = 40, m_CapK = 1e9f, m_Eta = 0.8f, m_YExit = 2150, m_KTurn = 1168, m_H2 = 800, m_KShaft = 1150, m_V0 = 23, m_Kp = 4, m_Kr = 6, m_Krt = 10, m_EtaB = 0.3f;
	int m_Mode = 1, m_Chord = 0, m_ForceRt = -1, m_Switch = 1 << 30;
	float m_ForceDeg = 0, m_LagT = 5, m_TrackCap = 96, m_RvDx = -38, m_RvDy = 5, m_VNom = 13, m_VMax = 25, m_RvW = 0.5f, m_RvVx = 1, m_RvVy = 16, m_AMax = 3.5f, m_TgtX = -1, m_TgtY = 0;
	float m_Roi[4] = {9150, 1700, 9700, 2300};
};
static SPar P;

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

// ---------------------------------------------------------------- reference (Teero's track)
struct SRef
{
	int m_K0 = 0;
	std::vector<vec2> m_P, m_T;
	std::vector<float> m_S;
	float m_KGate = 0;
	int N() const { return (int)m_P.size(); }
	void Load(const char *pPath, int K0, int K1)
	{
		std::vector<vec2> Raw;
		FILE *f = std::fopen(pPath, "r");
		int k;
		float x, y;
		std::vector<std::pair<int, vec2>> All;
		while(f && std::fscanf(f, "%d %f %f", &k, &x, &y) == 3)
			All.push_back({k, vec2(x, y)});
		if(f)
			std::fclose(f);
		m_K0 = K0;
		for(int kk = K0; kk <= K1; kk++)
		{
			vec2 S(0, 0);
			int n = 0;
			for(auto &[kx, p] : All)
				if(kx >= kk - 2 && kx <= kk + 2)
				{
					S += p;
					n++;
				}
			m_P.push_back(S / (float)std::max(n, 1));
		}
		m_S.assign(N(), 0);
		for(int i = 1; i < N(); i++)
			m_S[i] = m_S[i - 1] + distance(m_P[i], m_P[i - 1]);
		m_T.assign(N(), vec2(1, 0));
		for(int i = 0; i < N(); i++)
		{
			vec2 d = m_P[std::min(i + 1, N() - 1)] - m_P[std::max(i - 1, 0)];
			if(length(d) > 0)
				m_T[i] = normalize(d);
		}
		m_KGate = (float)K1;
		for(int i = 1; i < N(); i++)
			if(m_P[i].x <= P.m_GateX && m_P[i - 1].x > P.m_GateX)
			{
				float a = (m_P[i - 1].x - P.m_GateX) / (m_P[i - 1].x - m_P[i].x);
				m_KGate = K0 + i - 1 + a;
				break;
			}
	}
	// fractional index of the projection near hint
	float Project(vec2 p, int Hint, float *pDist) const
	{
		int lo = std::max(0, Hint - 15), hi = std::min(N() - 1, Hint + 40);
		int Best = lo;
		float BD = 1e18f;
		for(int i = lo; i <= hi; i++)
		{
			float d = distance(p, m_P[i]);
			if(d < BD)
			{
				BD = d;
				Best = i;
			}
		}
		float F = (float)Best;
		float BestD = BD;
		for(int s = -1; s <= 0; s++)
		{
			int a = Best + s, b = a + 1;
			if(a < 0 || b >= N())
				continue;
			vec2 ab = m_P[b] - m_P[a];
			float L2 = dot(ab, ab);
			if(L2 <= 0)
				continue;
			float t = std::clamp(dot(p - m_P[a], ab) / L2, 0.0f, 1.0f);
			float d = distance(p, m_P[a] + ab * t);
			if(d < BestD)
			{
				BestD = d;
				F = a + t;
			}
		}
		if(pDist)
			*pDist = BestD;
		return F;
	}
	float SAt(float F) const
	{
		int a = std::clamp((int)std::floor(F), 0, N() - 1), b = std::min(a + 1, N() - 1);
		float t = F - a;
		return m_S[a] + (m_S[b] - m_S[a]) * t;
	}
	float FAtS(float S) const
	{
		if(S >= m_S.back())
		{
			float v = (m_S.back() - m_S[N() - 6]) / 5.0f;
			return (N() - 1) + (S - m_S.back()) / std::max(v, 1.0f);
		}
		int i = (int)(std::upper_bound(m_S.begin(), m_S.end(), S) - m_S.begin());
		i = std::clamp(i, 1, N() - 1);
		float a = m_S[i - 1], b = m_S[i];
		return (i - 1) + (b > a ? (S - a) / (b - a) : 0.0f);
	}
	vec2 Tangent(float F) const { return m_T[std::clamp((int)std::lround(F), 0, N() - 1)]; }
};
static SRef R;

static float Ramp(float V)
{
	// velocity ramp: start 550, range 2000, curvature 1.4 (units per second)
	float v = V * 50;
	if(v < 550)
		return 1.0f;
	return 1.0f / std::pow(1.4f, (v - 550) / 2000.0f);
}

// ---------------------------------------------------------------- explosions of projectiles in flight
struct SExp
{
	int m_T; // server tick of the explosion step
	vec2 m_P;
};
static int PendingExplosions(const CTasGame &G, SExp *pOut, int Max)
{
	int n = 0;
	for(CEntity *pEnt = G.m_pWorld->FindFirst(CGameWorld::ENTTYPE_PROJECTILE); pEnt && n < Max; pEnt = pEnt->TypeNext())
	{
		CProjectile *pProj = (CProjectile *)pEnt;
		int Life = pProj->m_LifeSpan;
		for(int T = G.m_Tick + 1; T < G.m_Tick + 200; T++)
		{
			float Pt = (T - pProj->m_StartTick - 1) / (float)SERVER_TICK_SPEED;
			float Ct = (T - pProj->m_StartTick) / (float)SERVER_TICK_SPEED;
			vec2 PrevPos = pProj->GetPos(Pt), CurPos = pProj->GetPos(Ct), ColPos, NewPos;
			int Collide = CTasGame::Collision()->IntersectLine(PrevPos, CurPos, &ColPos, &NewPos);
			if(Life > -1)
				Life--;
			if(Collide || Life == -1)
			{
				pOut[n++] = SExp{T, ColPos};
				break;
			}
		}
	}
	return n;
}

static vec2 KickAt(vec2 Tee, vec2 E)
{
	vec2 D = Tee - E;
	float l = length(D);
	vec2 Dir = l > 0 ? D / l : vec2(0, 1);
	float f = 1 - std::clamp((l - 48.0f) / 87.0f, 0.0f, 1.0f);
	float Dmg = 6 * f;
	if((int)Dmg == 0)
		return vec2(0, 0);
	return Dir * Dmg * 2;
}

// ---------------------------------------------------------------- value
struct SEval
{
	float m_V;
	float m_F;
	uint64_t m_EKey;
	int m_NE;
};

// aim (deg) whose shot from G (fired on the next step) explodes closest to (TgtX, TgtY)
static float AimForTarget(const CTasGame &G, const STasInput &Base, float *pMiss)
{
	static CTasGame S;
	vec2 p = G.Pos();
	float Direct = std::atan2(P.m_TgtY - p.y, P.m_TgtX - p.x) * 180 / pi;
	float BestA = Direct, BestD = 1e9f;
	auto Try = [&](float A) {
		S.CopyFrom(G);
		STasInput F = Base;
		F.m_Fire = 1;
		F.m_TX = (int16_t)std::lround(std::cos(A * pi / 180) * 10000);
		F.m_TY = (int16_t)std::lround(std::sin(A * pi / 180) * 10000);
		S.Step(F);
		SExp aE[4];
		int ne = PendingExplosions(S, aE, 4);
		float D = ne ? distance(aE[ne - 1].m_P, vec2(P.m_TgtX, P.m_TgtY)) : 1e8f;
		if(D < BestD)
		{
			BestD = D;
			BestA = A;
		}
	};
	for(float A = Direct - 15; A <= Direct + 15; A += 0.5f)
		Try(A);
	float C = BestA;
	for(float A = C - 0.5f; A <= C + 0.5f; A += 0.05f)
		Try(A);
	if(pMiss)
		*pMiss = BestD;
	return BestA;
}

static void UpdateTrack(CTasGame &G)
{
	if(P.m_Mode != 5)
		return;
	int Rt = G.m_Tick - G.m_StartTick;
	if(Rt > P.m_Switch)
		return;
	float k = Rt - P.m_LagT;
	int i = std::clamp((int)std::lround(k) - R.m_K0, 0, R.N() - 1);
	float d = distance(G.Pos(), R.m_P[i]);
	d = std::min(d, P.m_TrackCap);
	G.m_TrackCost += d * d / 1000.0f;
}

static SEval Evaluate(const CTasGame &G, float HintF)
{
	SEval E;
	vec2 p = G.Pos(), v = G.Vel();
	float Dist;
	E.m_F = R.Project(p, (int)std::lround(HintF), &Dist);
	SExp aE[4];
	int ne = PendingExplosions(G, aE, 4);
	E.m_NE = ne;
	E.m_EKey = 0;
	const int Rt = G.m_Tick - G.m_StartTick;
	vec2 vk = v;
	for(int i = 0; i < ne; i++)
	{
		int dt = aE[i].m_T - 1 - G.m_Tick; // ticks until the state the kick sees
		vec2 ph = p;
		vec2 vv = v;
		for(int t = 0; t < dt; t++)
		{
			vv.y += 0.5f;
			ph += vec2(vv.x * Ramp(length(vv)), vv.y);
		}
		vec2 K = KickAt(ph, aE[i].m_P);
		vk += K * (P.m_Kg / (1.0f + dt / P.m_KDecay));
		E.m_EKey = E.m_EKey * 1000003ull + (uint64_t)(aE[i].m_T * 7919 + (int)(aE[i].m_P.x / 16) * 131 + (int)(aE[i].m_P.y / 16));
	}
	if(P.m_Mode == 4)
	{
		float KF = R.m_K0 + E.m_F;
		float Vx;
		if(KF < P.m_KShaft)
			Vx = P.m_V0 + (ne ? P.m_Kp : 0.0f);
		else
		{
			float Ux = std::max(0.0f, -vk.x);
			float Uy = KF < P.m_KTurn ? std::max(0.0f, vk.y) * P.m_Eta : std::fabs(vk.y) * P.m_EtaB;
			Vx = std::sqrt(Ux * Ux + Uy * Uy + std::max(0.0f, P.m_YExit - p.y));
		}
		if(ne == 0)
			Vx += P.m_Kr * std::max(0.0f, 1.0f - G.ReloadTimer() / P.m_Krt);
		Vx = std::max(Vx, P.m_VMin);
		E.m_V = Rt + (R.m_KGate - KF) + P.m_H2 / (Vx * Ramp(Vx)) + P.m_Lat * std::max(0.0f, Dist - P.m_LatDz);
		return E;
	}
	if(P.m_Mode == 2)
	{
		float KF = R.m_K0 + E.m_F;
		float Ux = KF < P.m_KShaft ? 0.0f : std::max(0.0f, -vk.x), Uy;
		if(KF < P.m_KTurn)
			Uy = std::max(0.0f, vk.y) * P.m_Eta;
		else
			Uy = std::fabs(vk.y) * P.m_EtaB;
		float E2 = Ux * Ux + Uy * Uy + std::max(0.0f, P.m_YExit - p.y);
		float Vx = std::sqrt(E2);
		if(ne == 0)
			Vx += P.m_Kr * std::max(0.0f, 1.0f - G.ReloadTimer() / P.m_Krt);
		Vx = std::max(Vx, P.m_VMin);
		E.m_V = Rt + (R.m_KGate - KF) + P.m_H2 / (Vx * Ramp(Vx)) + P.m_Lat * std::max(0.0f, Dist - P.m_LatDz);
		return E;
	}
	if(P.m_Mode == 6 && ne > 0)
	{
		int Rem = aE[0].m_T - 1 - G.m_Tick;
		if(Rem >= 0)
		{
			vec2 Tg = aE[0].m_P + vec2(P.m_RvDx, P.m_RvDy);
			// ballistic prediction; the tee can deviate by ~amax/2 * Rem^2 (hook + air control)
			vec2 ph = p, vv = v;
			for(int t = 0; t < Rem; t++)
			{
				vv.y += 0.5f;
				ph += vec2(vv.x * Ramp(length(vv)), vv.y);
			}
			float Miss = distance(ph, Tg);
			float Reach = 0.5f * P.m_AMax * Rem * Rem;
			float Dv = std::max(0.0f, distance(vv, vec2(P.m_RvVx, P.m_RvVy)) - P.m_AMax * Rem);
			float Val = std::max(0.0f, Miss - Reach) + 0.05f * Miss + P.m_RvW * Dv;
			E.m_V = 10000 + Val;
			return E;
		}
	}
	float Sp = length(vk);
	vec2 Disp(vk.x * Ramp(Sp), vk.y);
	vec2 Dir = R.Tangent(E.m_F);
	if(P.m_Chord)
	{
		float Fc = R.FAtS(R.SAt(E.m_F) + P.m_H);
		int ic = std::clamp((int)std::lround(Fc), 0, R.N() - 1);
		vec2 C = R.m_P[ic] - p;
		if(length(C) > 1)
			Dir = normalize(C);
	}
	float Along = dot(Disp, Dir);
	if(ne == 0)
		Along += P.m_Cr * std::max(0.0f, 1.0f - G.ReloadTimer() / 5.0f);
	float S0 = R.SAt(E.m_F);
	float F2 = R.FAtS(S0 + P.m_H);
	float tT = std::max(F2 - E.m_F, 0.5f);
	float VT = P.m_H / tT;
	float KF = R.m_K0 + E.m_F;
	Along = KF < P.m_CapK ? std::clamp(Along, P.m_VMin, P.m_VCap * VT) : std::max(Along, P.m_VMin);
	E.m_V = Rt + (R.m_KGate - KF) - tT + P.m_H / Along + P.m_Lat * std::max(0.0f, Dist - P.m_LatDz);
	if(P.m_Mode == 5 && Rt <= P.m_Switch)
		E.m_V = G.m_TrackCost + 0.001f * E.m_V;
	return E;
}

// ---------------------------------------------------------------- actions
static void HookTargets(const CTasGame &G, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 Pp = G.Pos();
	const float Range = P.m_HookRange + 4.0f * length(G.Vel());
	std::vector<int> vSeen;
	for(int a = 0; a < P.m_HookAng; a++)
	{
		float Ang = 2 * pi * a / P.m_HookAng;
		vec2 D(std::cos(Ang), std::sin(Ang));
		int Hit = -1;
		for(float r = 42.0f; r < Range; r += 4.0f)
		{
			vec2 Q = Pp + D * r;
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
		if(Hit < 0)
			continue;
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

static bool Survives(const CTasGame &G, int N)
{
	if(G.Frozen() || G.EnteredFreeze())
		return false;
	static vec2 aPos[256];
	N = std::min(N, 256);
	for(int d = 1; d >= -1; d--)
	{
		if(G.Rollout(N, d, true, aPos) == N)
			return true;
		if(G.m_LastHook && G.Rollout(N, d, false, aPos) == N)
			return true;
	}
	static std::vector<std::pair<int16_t, int16_t>> s_vHooks;
	HookTargets(G, s_vHooks);
	for(auto [TX, TY] : s_vHooks)
		for(int d = 1; d >= -1; d--)
			if(G.RolloutHook(N, d, TX, TY, aPos) == N)
				return true;
	return false;
}

struct SNode
{
	std::unique_ptr<CTasGame> m_pG;
	float m_V, m_F;
	uint64_t m_EKey;
	int m_Hist; // index into history of this layer
};
struct SHist
{
	int m_Parent;
	STasInput m_In;
};
struct SCand
{
	float m_V, m_F;
	int m_Parent;
	STasInput m_In;
	uint64_t m_Key, m_EKey;
	int m_PNode; // index of the parent node in the current beam
};
struct SGateHit
{
	int m_Rt;
	float m_Sp;
	vec2 m_Pos, m_Vel;
	int m_Layer, m_Parent;
	STasInput m_In;
	bool m_Surv;
};

static uint64_t CellKey(const CTasGame &G, uint64_t EKey, const STasInput &In)
{
	vec2 p = G.Pos(), v = G.Vel();
	uint64_t K = 1469598103934665603ull;
	auto Mix = [&](int64_t x) { K = (K ^ (uint64_t)x) * 1099511628211ull; };
	Mix((int)std::floor(p.x / P.m_CPos));
	Mix((int)std::floor(p.y / P.m_CPos));
	Mix((int)std::floor(v.x / P.m_CVel));
	Mix((int)std::floor(v.y / P.m_CVel));
	Mix(G.HookState() * 2 + (In.m_Hook ? 1 : 0));
	Mix(G.HookState() == HOOK_GRABBED ? (int)(G.HookPos().x / 32) * 1000 + (int)(G.HookPos().y / 32) : 0);
	Mix(G.ReloadTimer() == 0 ? 0 : 1 + G.ReloadTimer() / 6);
	Mix(G.Jumped());
	Mix((int64_t)EKey);
	return K;
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: hp <map> prefix=FILE cut=N ...\n");
		return 1;
	}
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "prefix") P.m_Prefix = V;
		else if(K == "out") P.m_Out = V;
		else if(K == "ref") P.m_Ref = V;
		else if(K == "fixed") P.m_Fixed = V;
		else if(K == "fixedn") P.m_FixedN = std::stoi(V);
		else if(K == "cut") P.m_Cut = std::stoi(V);
		else if(K == "beam") P.m_Beam = std::stoi(V);
		else if(K == "maxt") P.m_MaxT = std::stoi(V);
		else if(K == "nout") P.m_NOut = std::stoi(V);
		else if(K == "hookang") P.m_HookAng = std::stoi(V);
		else if(K == "fireang") P.m_FireAng = std::stoi(V);
		else if(K == "firetop") P.m_FireTop = std::stoi(V);
		else if(K == "pbmax") P.m_PbMax = std::stoi(V);
		else if(K == "fmax") P.m_FMax = std::stoi(V);
		else if(K == "survive") P.m_Survive = std::stoi(V);
		else if(K == "survevery") P.m_SurvEvery = std::stoi(V);
		else if(K == "minper") P.m_MinPer = std::stoi(V);
		else if(K == "maxkeys") P.m_MaxKeys = std::stoi(V);
		else if(K == "jump") P.m_Jump = std::stoi(V);
		else if(K == "nofire") P.m_NoFire = std::stoi(V);
		else if(K == "firefrom") P.m_FireFrom = std::stoi(V);
		else if(K == "fireto") P.m_FireTo = std::stoi(V);
		else if(K == "fire2") P.m_Fire2 = std::stoi(V);
		else if(K == "gatex") P.m_GateX = std::stof(V);
		else if(K == "gatey") P.m_GateY1 = std::stof(V);
		else if(K == "gatey0") P.m_GateY0 = std::stof(V);
		else if(K == "gatevx") P.m_GateVx = std::stof(V);
		else if(K == "H") P.m_H = std::stof(V);
		else if(K == "vcap") P.m_VCap = std::stof(V);
		else if(K == "capk") P.m_CapK = std::stof(V);
		else if(K == "mode") P.m_Mode = std::stoi(V);
		else if(K == "chord") P.m_Chord = std::stoi(V);
		else if(K == "forcefire") std::sscanf(V.c_str(), "%d:%f", &P.m_ForceRt, &P.m_ForceDeg);
		else if(K == "forcetgt") std::sscanf(V.c_str(), "%d:%f,%f", &P.m_ForceRt, &P.m_TgtX, &P.m_TgtY);
		else if(K == "lagt") P.m_LagT = std::stof(V);
		else if(K == "rvoff") std::sscanf(V.c_str(), "%f,%f", &P.m_RvDx, &P.m_RvDy);
		else if(K == "rvv") std::sscanf(V.c_str(), "%f,%f", &P.m_RvVx, &P.m_RvVy);
		else if(K == "vnom") P.m_VNom = std::stof(V);
		else if(K == "vmax") P.m_VMax = std::stof(V);
		else if(K == "rvw") P.m_RvW = std::stof(V);
		else if(K == "amax") P.m_AMax = std::stof(V);
		else if(K == "trackcap") P.m_TrackCap = std::stof(V);
		else if(K == "switch") P.m_Switch = std::stoi(V);
		else if(K == "eta") P.m_Eta = std::stof(V);
		else if(K == "etab") P.m_EtaB = std::stof(V);
		else if(K == "yexit") P.m_YExit = std::stof(V);
		else if(K == "kturn") P.m_KTurn = std::stof(V);
		else if(K == "kshaft") P.m_KShaft = std::stof(V);
		else if(K == "v0") P.m_V0 = std::stof(V);
		else if(K == "kp") P.m_Kp = std::stof(V);
		else if(K == "H2") P.m_H2 = std::stof(V);
		else if(K == "kr") P.m_Kr = std::stof(V);
		else if(K == "krt") P.m_Krt = std::stof(V);
		else if(K == "vmin") P.m_VMin = std::stof(V);
		else if(K == "cr") P.m_Cr = std::stof(V);
		else if(K == "kg") P.m_Kg = std::stof(V);
		else if(K == "kdecay") P.m_KDecay = std::stof(V);
		else if(K == "cpos") P.m_CPos = std::stof(V);
		else if(K == "cvel") P.m_CVel = std::stof(V);
		else if(K == "hookrange") P.m_HookRange = std::stof(V);
		else if(K == "lat") P.m_Lat = std::stof(V);
		else if(K == "latdz") P.m_LatDz = std::stof(V);
		else if(K == "roi")
			std::sscanf(V.c_str(), "%f,%f,%f,%f", &P.m_Roi[0], &P.m_Roi[1], &P.m_Roi[2], &P.m_Roi[3]);
		else
			std::printf("unknown key %s\n", K.c_str());
	}
	R.Load(P.m_Ref.c_str(), 1060, 1260);
	std::printf("ref: %d pts, K_gate %.2f\n", R.N(), R.m_KGate);
	std::vector<STasInput> vPre = ReadInputs(P.m_Prefix.c_str());
	std::vector<STasInput> vFixed;
	if(!P.m_Fixed.empty())
	{
		auto vF = ReadInputs(P.m_Fixed.c_str());
		for(int i = P.m_Cut; i < P.m_Cut + P.m_FixedN && i < (int)vF.size(); i++)
			vFixed.push_back(vF[i]);
	}
	auto pStart = std::make_unique<CTasGame>();
	pStart->Spawn(CTasGame::Map().m_vSpawns[0]);
	for(int i = 0; i < P.m_Cut && i < (int)vPre.size(); i++)
		pStart->Step(vPre[i]);
	STasInput LastIn = vPre[P.m_Cut - 1];
	{
		vec2 p = pStart->Pos(), v = pStart->Vel();
		std::printf("start rt %d pos %.1f %.1f vel %.3f %.3f reload %d hook %d proj %d\n", pStart->m_Tick - pStart->m_StartTick, p.x, p.y, v.x, v.y,
			pStart->ReloadTimer(), pStart->HookState(), pStart->NumProjectiles());
	}
	std::vector<std::vector<SHist>> vHist;
	std::vector<SNode> vBeam;
	{
		SEval E = Evaluate(*pStart, R.Project(pStart->Pos(), 0, nullptr) + 0.0f);
		// better hint: global nearest
		float bd = 1e18f;
		int bi = 0;
		for(int i = 0; i < R.N(); i++)
			if(distance(R.m_P[i], pStart->Pos()) < bd)
			{
				bd = distance(R.m_P[i], pStart->Pos());
				bi = i;
			}
		E = Evaluate(*pStart, (float)bi);
		vHist.push_back({SHist{-1, LastIn}});
		vBeam.push_back(SNode{std::move(pStart), E.m_V, E.m_F, E.m_EKey, 0});
	}
	std::vector<SGateHit> vGate;
	std::vector<std::pair<int16_t, int16_t>> vHooks;
	auto T0 = std::chrono::steady_clock::now();
	long long Steps = 0;
	for(int L = 1; L <= P.m_MaxT && !vBeam.empty(); L++)
	{
		std::vector<SCand> vC;
		std::unordered_map<uint64_t, int> mKey;
		// fire permission: best firetop nodes only
		std::vector<float> vVals;
		for(auto &N : vBeam)
			vVals.push_back(N.m_V);
		std::vector<float> vSorted = vVals;
		std::sort(vSorted.begin(), vSorted.end());
		float FireThr = vSorted[std::min((int)vSorted.size() - 1, P.m_FireTop - 1)];
		auto Add = [&](SCand &&C) {
			auto It = mKey.find(C.m_Key);
			if(It != mKey.end())
			{
				if(vC[It->second].m_V <= C.m_V)
					return;
				vC[It->second] = std::move(C);
				return;
			}
			mKey[C.m_Key] = (int)vC.size();
			vC.push_back(std::move(C));
		};
		CTasGame Scratch;
		for(int ni = 0; ni < (int)vBeam.size(); ni++)
		{
			const CTasGame &G = *vBeam[ni].m_pG;
			const STasInput &Prev = vHist[L - 1][vBeam[ni].m_Hist].m_In;
			std::vector<STasInput> vAct;
			if(L - 1 < (int)vFixed.size())
				vAct.push_back(vFixed[L - 1]);
			else
			{
				const bool Hooking = G.m_LastHook;
				if(!Hooking)
					HookTargets(G, vHooks);
				const bool CanJump = P.m_Jump && !G.m_LastJump && (G.Grounded() || !(G.Jumped() & 2));
				const int Rt = G.m_Tick - G.m_StartTick + 1; // race tick after this step
				const bool CanFire = !P.m_NoFire && G.HasGrenade() && G.ReloadTimer() == 0 && !Prev.m_Fire && vBeam[ni].m_V <= FireThr &&
						     ((Rt >= P.m_FireFrom && Rt <= P.m_FireTo) || Rt >= P.m_Fire2);
				float NodeAim = 1e9f, NodeMiss = 0;
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int Jump = 0; Jump <= (CanJump ? 1 : 0); Jump++)
					{
						STasInput In;
						In.m_Dir = Dir;
						In.m_Jump = Jump;
						In.m_Weapon = 3;
						In.m_TX = Prev.m_TX;
						In.m_TY = Prev.m_TY;
						In.m_Hook = Hooking;
						In.m_Fire = 0;
						vAct.push_back(In);
						if(Hooking)
						{
							STasInput Rl = In;
							Rl.m_Hook = 0;
							vAct.push_back(Rl);
						}
						else
							for(auto [TX, TY] : vHooks)
							{
								STasInput H = In;
								H.m_Hook = 1;
								H.m_TX = TX;
								H.m_TY = TY;
								vAct.push_back(H);
							}
						if(Rt == P.m_ForceRt && G.ReloadTimer() == 0 && !Jump)
						{
							STasInput F = In;
							F.m_Fire = 1;
							float Deg = P.m_ForceDeg;
							if(P.m_TgtX > 0)
							{
								if(NodeAim > 1e8f)
									NodeAim = AimForTarget(G, In, &NodeMiss);
								Deg = NodeAim;
							}
							F.m_TX = (int16_t)std::lround(std::cos(Deg * pi / 180) * 10000);
							F.m_TY = (int16_t)std::lround(std::sin(Deg * pi / 180) * 10000);
							if(NodeMiss < 40)
								vAct.push_back(F);
						}
						else if(CanFire && !Jump)
							for(int a = 0; a < P.m_FireAng; a++)
							{
								float Ang = 2 * pi * (a + 0.5f) / P.m_FireAng;
								STasInput F = In;
								F.m_Fire = 1;
								F.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
								F.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
								vAct.push_back(F);
							}
					}
			}
			if(P.m_ForceRt == G.m_Tick - G.m_StartTick + 1 && G.ReloadTimer() == 0 && L - 1 >= (int)vFixed.size())
			{
				std::vector<STasInput> vF;
				for(auto &In : vAct)
					if(In.m_Fire)
						vF.push_back(In);
				vAct = vF;
			}
			for(const STasInput &In : vAct)
			{
				CTasGame *pC = &Scratch;
				pC->CopyFrom(G);
				pC->Step(In);
				UpdateTrack(*pC);
				Steps++;
				if(pC->Frozen() || pC->EnteredFreeze())
					continue;
				if(In.m_Fire && pC->NumProjectiles() > G.NumProjectiles() && L - 1 >= (int)vFixed.size() &&
					pC->m_Tick - pC->m_StartTick != P.m_ForceRt)
				{
					// a new projectile in flight: keep point-blank-ish or ROI pre-fires only
					SExp aE[4];
					int ne = PendingExplosions(*pC, aE, 4);
					bool Ok = true;
					for(int e = 0; e < ne; e++)
					{
						int dt = aE[e].m_T - pC->m_Tick;
						bool Pb = dt <= P.m_PbMax;
						bool Roi = aE[e].m_P.x >= P.m_Roi[0] && aE[e].m_P.x <= P.m_Roi[2] && aE[e].m_P.y >= P.m_Roi[1] &&
							   aE[e].m_P.y <= P.m_Roi[3] && dt <= P.m_FMax;
						if(!Pb && !Roi)
							Ok = false;
					}
					if(!Ok)
						continue;
				}
				vec2 p = pC->Pos(), v = pC->Vel();
				if(p.x <= P.m_GateX && v.x <= -P.m_GateVx && p.y <= P.m_GateY1 && p.y >= P.m_GateY0)
				{
					SGateHit Gh;
					Gh.m_Rt = pC->m_Tick - pC->m_StartTick;
					Gh.m_Sp = length(v);
					Gh.m_Pos = p;
					Gh.m_Vel = v;
					Gh.m_Layer = L;
					Gh.m_Parent = vBeam[ni].m_Hist;
					Gh.m_In = In;
					Gh.m_Surv = Survives(*pC, 30);
					vGate.push_back(Gh);
					continue;
				}
				SEval E = Evaluate(*pC, vBeam[ni].m_F);
				SCand C;
				C.m_V = E.m_V;
				C.m_F = E.m_F;
				C.m_EKey = E.m_EKey;
				C.m_Parent = vBeam[ni].m_Hist;
				C.m_In = In;
				C.m_Key = CellKey(*pC, E.m_EKey, In);
				C.m_PNode = ni;
				Add(std::move(C));
			}
		}
		// selection: reserved share per explosion key, then global rank; survival check
		std::vector<int> vOrder(vC.size());
		for(int i = 0; i < (int)vC.size(); i++)
			vOrder[i] = i;
		std::sort(vOrder.begin(), vOrder.end(), [&](int a, int b) { return vC[a].m_V < vC[b].m_V; });
		std::vector<char> vTaken(vC.size(), 0);
		std::vector<int> vSel;
		std::vector<std::unique_ptr<CTasGame>> vGames(vC.size());
		const bool Check = P.m_SurvEvery > 0 && (L % P.m_SurvEvery) == 0;
		auto TryTake = [&](int i) -> bool {
			if(vTaken[i])
				return false;
			vTaken[i] = 1;
			auto pG = std::make_unique<CTasGame>();
			pG->CopyFrom(*vBeam[vC[i].m_PNode].m_pG);
			pG->Step(vC[i].m_In);
			UpdateTrack(*pG);
			if(Check && !Survives(*pG, P.m_Survive))
				return false;
			vGames[i] = std::move(pG);
			vSel.push_back(i);
			return true;
		};
		{
			std::unordered_map<uint64_t, int> mCnt;
			int Keys = 0;
			for(int i : vOrder)
			{
				uint64_t EK = vC[i].m_EKey;
				if(!EK)
					continue;
				auto It = mCnt.find(EK);
				if(It == mCnt.end())
				{
					if(Keys >= P.m_MaxKeys)
						continue;
					Keys++;
					It = mCnt.emplace(EK, 0).first;
				}
				if(It->second >= P.m_MinPer)
					continue;
				if(TryTake(i))
					It->second++;
			}
		}
		if(P.m_Mode == 6 && vBeam[0].m_pG->m_Tick - vBeam[0].m_pG->m_StartTick + 1 <= P.m_ForceRt + 24)
		{
			// two pools: pending (rendezvous value) and the rest (mode 1 value), half the beam each
			int nP = 0, nN = 0;
			for(int i : vOrder)
			{
				bool Pend = vC[i].m_EKey != 0;
				if(Pend ? nP >= P.m_Beam / 2 : nN >= P.m_Beam / 2)
					continue;
				if(TryTake(i))
					(Pend ? nP : nN)++;
			}
		}
		for(int i : vOrder)
		{
			if((int)vSel.size() >= P.m_Beam)
				break;
			TryTake(i);
		}
		if((int)vSel.size() < P.m_Beam / 10)
		{
			// too few survivors (rollouts ignore pending explosions and multi-phase escapes): keep unchecked ones
			int Need = P.m_Beam / 10 - (int)vSel.size();
			for(int i : vOrder)
			{
				if(Need <= 0)
					break;
				if(vGames[i])
					continue;
				auto pG = std::make_unique<CTasGame>();
				pG->CopyFrom(*vBeam[vC[i].m_PNode].m_pG);
				pG->Step(vC[i].m_In);
				UpdateTrack(*pG);
				vGames[i] = std::move(pG);
				vSel.push_back(i);
				Need--;
			}
		}
		std::sort(vSel.begin(), vSel.end(), [&](int a, int b) { return vC[a].m_V < vC[b].m_V; });
		std::vector<SHist> vH;
		std::vector<SNode> vNext;
		for(int i : vSel)
		{
			vH.push_back(SHist{vC[i].m_Parent, vC[i].m_In});
			vNext.push_back(SNode{std::move(vGames[i]), vC[i].m_V, vC[i].m_F, vC[i].m_EKey, (int)vH.size() - 1});
		}
		vHist.push_back(std::move(vH));
		vBeam = std::move(vNext);
		if(!vBeam.empty())
		{
			const CTasGame &B = *vBeam[0].m_pG;
			int npend = 0;
			for(auto &N : vBeam)
				npend += N.m_EKey != 0;
			double El = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
			std::printf("L %3d rt %d cand %zu beam %zu pend %d best %.2f pos %.0f %.0f vel %.2f %.2f rl %d k %.1f | gate hits %zu | %.1fs %.1fM steps\n", L,
				B.m_Tick - B.m_StartTick, vC.size(), vBeam.size(), npend, vBeam[0].m_V, B.Pos().x, B.Pos().y, B.Vel().x, B.Vel().y, B.ReloadTimer(),
				R.m_K0 + vBeam[0].m_F, vGate.size(), El, Steps / 1e6);
			std::fflush(stdout);
		}
		// stop when the gate was hit and the beam can't do better any more (all later)
		if(!vGate.empty())
		{
			int BestRt = 1 << 30;
			for(auto &g : vGate)
				if(g.m_Surv)
					BestRt = std::min(BestRt, g.m_Rt);
			if(BestRt < (1 << 30) && L >= 1 && (vHist.size() > 0))
			{
				if(!vBeam.empty() && vBeam[0].m_pG->m_Tick - vBeam[0].m_pG->m_StartTick >= BestRt + 2)
					break;
			}
		}
	}
	// results
	std::sort(vGate.begin(), vGate.end(), [](const SGateHit &a, const SGateHit &b) {
		if(a.m_Surv != b.m_Surv)
			return a.m_Surv > b.m_Surv;
		if(a.m_Rt != b.m_Rt)
			return a.m_Rt < b.m_Rt;
		return a.m_Sp > b.m_Sp;
	});
	int NOut = 0;
	for(auto &g : vGate)
	{
		if(NOut >= P.m_NOut)
			break;
		std::vector<STasInput> vPath;
		vPath.push_back(g.m_In);
		int Idx = g.m_Parent;
		for(int L = g.m_Layer - 1; L >= 1; L--)
		{
			vPath.push_back(vHist[L][Idx].m_In);
			Idx = vHist[L][Idx].m_Parent;
		}
		std::reverse(vPath.begin(), vPath.end());
		char aName[512];
		std::snprintf(aName, sizeof(aName), "%s_%d.txt", P.m_Out.c_str(), NOut);
		FILE *f = std::fopen(aName, "w");
		for(int i = 0; i < P.m_Cut; i++)
		{
			const STasInput &In = vPre[i];
			std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
		}
		for(auto &In : vPath)
			std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
		std::fclose(f);
		std::printf("GATE rt %d |v| %.2f pos %.0f %.0f vel %.2f %.2f surv %d -> %s\n", g.m_Rt, g.m_Sp, g.m_Pos.x, g.m_Pos.y, g.m_Vel.x, g.m_Vel.y, g.m_Surv,
			aName);
		NOut++;
	}
	if(vGate.empty())
		std::printf("no gate hit\n");
	return 0;
}
