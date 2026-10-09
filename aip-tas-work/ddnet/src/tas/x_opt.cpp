// x_opt: long-horizon plan optimizer. A window of a run is rewritten as a semantic plan - hook holds (press tick,
// release tick, the anchor its hook grabbed), shots (fire tick, the point where the grenade exploded), the direction
// and jump keys per tick - and improved by simulated annealing over joint mutations of that plan (shift a press /
// release / shot, move an anchor or an explosion point, merge / split / drop / add hook holds, change direction keys
// and jumps), every candidate simulated exactly (CFastG) from the cut to the window end.
//
// Why: x_ds ranks per-tick input choices by the progress a few dozen ticks ahead; it reproduces the polished run's
// own hooks and cannot move several kicks and hooks together toward a speed that only pays off hundreds of ticks
// later (kick-phase coupling, NOTES "Speed session"). Here the objective is measured at the end of a long window:
// the lead over the run (projection on its path) plus kv x the speed difference to the run at that place, and a
// candidate keeps or loses all its changes at once.
//
// usage: x_opt MAP run=RUN cut=A end=B [iters=N] [threads=T] [seed=S] [kv=0.3] [lw=0.05] [lat0=12] [t0=0.3]
//              [field=FILE fw=1] [out=FILE] [v=1]
//   cut / end: race ticks of the window; the plan is everything the run does in [A, B)
//   kv: ticks per px/t of speed above the run's speed at the same place (end of the window)
//   lw, lat0: penalty lw x (distance to the run's path - lat0) at the end
//   field=FILE: score by the time-to-go field instead (fw x (T_run - T_ours) at the end tick)
//   out: prefix (the run's inputs before A + the best plan) for x_ds / x_graft continuations
//   kvalt=K1,K2 outalt=P: also keep the best plan seen for each other kv (e.g. 0 = lead only) and write it to P_k<i>.txt
//     (a high exit speed often cannot be kept through the next turn)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include "tasio.h"
#include "tfield.h"

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

static const float R0 = 28.0f * 0.75f;

static vec2 AimAt(vec2 P, vec2 E, const CTuningParams &Tu)
{
	const float Vg = Tu.m_GrenadeSpeed / SERVER_TICK_SPEED, Cg = Tu.m_GrenadeCurvature / 10000.0f * Vg * Vg;
	const vec2 W = E - P;
	auto F = [&](float t) { return length(vec2(W.x, W.y - Cg * t * t)) - R0 - Vg * t; };
	float Lo = 0, Hi = 1;
	while(F(Hi) > 0 && Hi < 120)
		Hi *= 1.5f;
	for(int it = 0; it < 40; it++)
	{
		float Mid = 0.5f * (Lo + Hi);
		if(F(Mid) > 0)
			Lo = Mid;
		else
			Hi = Mid;
	}
	return normalize(vec2(W.x, W.y - Cg * Hi * Hi));
}

static bool Flight(vec2 P, vec2 Dir, const CTuningParams &Tu, vec2 &Col)
{
	const vec2 P0 = P + Dir * R0;
	const float Curv = Tu.m_GrenadeCurvature, Speed = Tu.m_GrenadeSpeed;
	const SMapInfo &M = CTasGame::Map();
	for(int k = 1; k <= 100; k++)
	{
		vec2 Prev = CalcPos(P0, Dir, Curv, Speed, (k - 1) / (float)SERVER_TICK_SPEED);
		vec2 Cur = CalcPos(P0, Dir, Curv, Speed, k / (float)SERVER_TICK_SPEED);
		if(Cur.x < 0 || Cur.y < 0 || Cur.x >= M.m_W * 32 || Cur.y >= M.m_H * 32)
			return false;
		vec2 NewPos;
		if(CTasGame::Collision()->IntersectLine(Prev, Cur, &Col, &NewPos))
			return true;
	}
	return false;
}

static void SetAim(STasInput &In, vec2 D)
{
	In.m_TX = (int16_t)std::lround(D.x * 10000);
	In.m_TY = (int16_t)std::lround(D.y * 10000);
	if(!In.m_TX && !In.m_TY)
		In.m_TY = -1;
}

struct SHookSeg
{
	int m_P, m_R; // window ticks [P, R); P < 0: already held at the cut
	bool m_Anch = false;
	vec2 m_A = vec2(0, 0);
	float m_Ang = 0;
	bool m_Orig = false; // use the run's own aim when the press state matches
	int16_t m_TX = 0, m_TY = 0;
	vec2 m_Ref = vec2(0, 0);
};

struct SShot
{
	int m_F;
	bool m_HasX = false;
	vec2 m_X = vec2(0, 0);
	float m_Ang = 0;
	bool m_Orig = false;
	int16_t m_TX = 0, m_TY = 0;
	vec2 m_Ref = vec2(0, 0);
	int m_RefTick = 0;
};

struct SPlan
{
	std::vector<int8_t> m_D, m_J;
	std::vector<SHookSeg> m_H;
	std::vector<SShot> m_S;
};

struct SCtx
{
	CFastG m_Cut;
	int m_N = 0, m_NT = 0; // window ticks (scored at the end), + the survival tail (the run's own plan)
	std::vector<STasInput> m_vBase; // the run's inputs in the window (weapon, neutral aims)
	std::vector<vec2> m_vIncP; // the run's core position after each window tick ... to the end of the run
	std::vector<float> m_vIncS;
	std::vector<int> m_vIncRt;
	std::vector<vec2> m_vIncV;
	int m_RtA = 0, m_IncFinish = -1;
	float m_Kv = 0.3f, m_Lw = 0.05f, m_Lat0 = 12, m_Fw = 0, m_LatMax = 100;
	const STField *m_pField = nullptr;
	float m_IncTEnd = 0;
	// close-kick spots (mutation 16): window ticks where the run passes within kickr px of a solid tile, and the
	// nearest point of that tile (Teero's kicks: 44 of 49 within 48 px, many pushing sideways to turn)
	std::vector<int> m_vOppK;
	std::vector<vec2> m_vOppW;
};
static SCtx g;

struct SRes
{
	float m_Score = -1e9f;
	bool m_Dead = false;
	int m_DeadK = -1, m_Fin = -1;
	float m_Lead = 0, m_Lat = 0, m_Sp = 0, m_IncSp = 0;
	std::vector<STasInput> m_vIn;
	std::vector<float> m_vLead; // every 10 ticks
};

// the window score with another kv (archive of best plans for kv values other than the chain's)
static float AltScore(const SRes &R, float Kv)
{
	if(R.m_Dead || R.m_Fin >= 0 || g.m_pField)
		return R.m_Score;
	return R.m_Lead + Kv * (R.m_Sp - R.m_IncSp) - g.m_Lw * std::max(0.0f, R.m_Lat - g.m_Lat0);
}

// projection of P on the run's path near window tick k: (label race tick, lateral distance)
static void Project(vec2 P, int k, float &Label, float &Lat, float &IncSp, vec2 *pIncV = nullptr)
{
	float Best = 1e9f;
	Label = 0;
	IncSp = 0;
	const int n = (int)g.m_vIncP.size();
	for(int d = -60; d <= 60; d++)
	{
		int a = k + d;
		if(a < 0 || a + 1 >= n)
			continue;
		vec2 A = g.m_vIncP[a], B = g.m_vIncP[a + 1], AB = B - A;
		float L2 = dot(AB, AB);
		float u = L2 > 1e-6f ? std::clamp(dot(P - A, AB) / L2, 0.0f, 1.0f) : 0.0f;
		float dd = distance(P, A + AB * u);
		if(dd < Best)
		{
			Best = dd;
			Label = g.m_vIncRt[a] + u * (g.m_vIncRt[a + 1] - g.m_vIncRt[a]);
			IncSp = g.m_vIncS[a] + u * (g.m_vIncS[a + 1] - g.m_vIncS[a]);
			if(pIncV)
				*pIncV = g.m_vIncV[a] + (g.m_vIncV[a + 1] - g.m_vIncV[a]) * u;
		}
	}
	Lat = Best;
}

// checkpoints of an evaluation (every CK window ticks): a candidate that differs from its parent plan only from
// tick f on is simulated from the parent's last checkpoint <= f (inputs before f are the same function of the same
// states), ~2x fewer simulated ticks per candidate
enum { CK = 8 };
struct SCkpt
{
	CFastG m_S;
	size_t m_Hi, m_Si;
	float m_Win, m_Lead, m_Lat, m_Sp, m_IncSp;
};

// first window tick where plans A and B can behave differently (g.m_NT if they are the same)
static int FirstDiff(const SPlan &A, const SPlan &B)
{
	int f = g.m_NT;
	for(int k = 0; k < g.m_NT && k < f; k++)
		if(A.m_D[k] != B.m_D[k] || A.m_J[k] != B.m_J[k])
		{
			f = k;
			break;
		}
	const size_t nh = std::max(A.m_H.size(), B.m_H.size());
	for(size_t i = 0; i < nh; i++)
	{
		if(i >= A.m_H.size() || i >= B.m_H.size())
		{
			const SHookSeg &H = i < A.m_H.size() ? A.m_H[i] : B.m_H[i];
			f = std::min(f, std::max(0, H.m_P));
			break;
		}
		const SHookSeg &a = A.m_H[i], &b = B.m_H[i];
		const bool SamePress = a.m_P == b.m_P && a.m_Anch == b.m_Anch && a.m_A == b.m_A && a.m_Ang == b.m_Ang && a.m_Orig == b.m_Orig &&
				       a.m_TX == b.m_TX && a.m_TY == b.m_TY && a.m_Ref == b.m_Ref;
		if(!SamePress)
		{
			f = std::min(f, std::max(0, std::min(a.m_P, b.m_P)));
			break;
		}
		if(a.m_R != b.m_R)
		{
			f = std::min(f, std::max(0, std::min(a.m_R, b.m_R)));
			break;
		}
	}
	const size_t ns = std::max(A.m_S.size(), B.m_S.size());
	for(size_t i = 0; i < ns; i++)
	{
		if(i >= A.m_S.size() || i >= B.m_S.size())
		{
			f = std::min(f, std::max(0, (i < A.m_S.size() ? A.m_S[i] : B.m_S[i]).m_F));
			break;
		}
		const SShot &a = A.m_S[i], &b = B.m_S[i];
		if(!(a.m_F == b.m_F && a.m_HasX == b.m_HasX && a.m_X == b.m_X && a.m_Ang == b.m_Ang && a.m_Orig == b.m_Orig && a.m_TX == b.m_TX &&
			   a.m_TY == b.m_TY && a.m_Ref == b.m_Ref && a.m_RefTick == b.m_RefTick))
		{
			f = std::min(f, std::max(0, std::min(a.m_F, b.m_F)));
			break;
		}
	}
	return f;
}

static void Eval(const SPlan &P, SRes &Res, bool Keep, const std::vector<SCkpt> *pFrom = nullptr, int From = 0, std::vector<SCkpt> *pRec = nullptr)
{
	CFastG S = g.m_Cut;
	size_t hi = 0, si = 0;
	int k0 = 0;
	float WinScore = 0;
	if(pRec)
		pRec->clear();
	if(pFrom && !Keep && From >= CK && !pFrom->empty())
	{
		const int c = std::min(From / CK, (int)pFrom->size() - 1);
		const SCkpt &C = (*pFrom)[c];
		S = C.m_S;
		hi = C.m_Hi;
		si = C.m_Si;
		WinScore = C.m_Win;
		Res.m_Lead = C.m_Lead;
		Res.m_Lat = C.m_Lat;
		Res.m_Sp = C.m_Sp;
		Res.m_IncSp = C.m_IncSp;
		k0 = c * CK;
		if(pRec)
			pRec->assign(pFrom->begin(), pFrom->begin() + c);
	}
	if(Keep)
	{
		Res.m_vIn.clear();
		Res.m_vLead.clear();
	}
	Res.m_Dead = false;
	Res.m_Fin = -1;
	for(int k = k0; k < g.m_NT; k++)
	{
		if(pRec && k % CK == 0)
			pRec->push_back({S, hi, si, WinScore, Res.m_Lead, Res.m_Lat, Res.m_Sp, Res.m_IncSp});
		STasInput In = g.m_vBase[k];
		In.m_Dir = P.m_D[k];
		In.m_Jump = P.m_J[k];
		while(hi < P.m_H.size() && P.m_H[hi].m_R <= k)
			hi++;
		bool Hook = hi < P.m_H.size() && P.m_H[hi].m_P <= k;
		const SHookSeg *pPress = Hook && P.m_H[hi].m_P == k ? &P.m_H[hi] : nullptr;
		In.m_Hook = Hook;
		In.m_Fire = 0;
		const SShot *pShot = nullptr;
		if(si < P.m_S.size() && P.m_S[si].m_F <= k && S.m_ReloadTimer == 0)
		{
			pShot = &P.m_S[si];
			// a press and a shot share the aim: only where the run itself did both on this tick
			if(pPress && !(pShot->m_Orig && pPress->m_Orig && k == pShot->m_RefTick && distance(S.m_Core.m_Pos, pPress->m_Ref) < 1e-3f))
				pShot = nullptr;
		}
		if(pPress)
		{
			if(pPress->m_Orig && distance(S.m_Core.m_Pos, pPress->m_Ref) < 1e-3f)
			{
				In.m_TX = pPress->m_TX;
				In.m_TY = pPress->m_TY;
			}
			else if(pPress->m_Anch)
				SetAim(In, normalize(pPress->m_A - S.m_Core.m_Pos));
			else
				SetAim(In, vec2(std::cos(pPress->m_Ang), std::sin(pPress->m_Ang)));
		}
		if(pShot)
		{
			In.m_Fire = 1;
			if(pShot->m_Orig && distance(S.m_Pos, pShot->m_Ref) < 1e-3f && k == pShot->m_RefTick)
			{
				In.m_TX = pShot->m_TX;
				In.m_TY = pShot->m_TY;
			}
			else if(pShot->m_HasX)
				SetAim(In, AimAt(S.m_Pos, pShot->m_X, S.m_Core.m_Tuning));
			else
				SetAim(In, vec2(std::cos(pShot->m_Ang), std::sin(pShot->m_Ang)));
		}
		S.Step(In);
		if(pShot && S.m_ReloadTimer > 0)
			si++;
		if(Keep)
			Res.m_vIn.push_back(In);
		bool OffRoute = false;
		float DLab = 0;
		if(k % 5 == 4 || S.m_Dead)
		{
			float Lab, Lat, IncSp;
			Project(S.m_Core.m_Pos, k, Lab, Lat, IncSp);
			DLab = Lab - g.m_RtA;
			OffRoute = Lat > g.m_LatMax;
		}
		if(S.m_Dead || OffRoute)
		{
			// dead (or off the route): ranked below every live plan, by how far along the run's path it got
			Res.m_Dead = true;
			Res.m_DeadK = k;
			Res.m_Score = (k < g.m_N ? -1000.0f : -500.0f) + DLab;
			return;
		}
		if(S.m_FinishTick >= 0)
		{
			Res.m_Fin = S.RaceTick();
			Res.m_Score = 100.0f + (float)(g.m_IncFinish - Res.m_Fin);
			Res.m_Lead = (float)(g.m_IncFinish - Res.m_Fin);
			return;
		}
		if(Keep && (k + 1) % 10 == 0)
		{
			float Lab, Lat, IncSp;
			Project(S.m_Core.m_Pos, k, Lab, Lat, IncSp);
			Res.m_vLead.push_back(Lab - S.RaceTick());
		}
		if(k == g.m_N - 1)
		{
			// the window's score: lead + speed along the run's direction at that place - lateral distance
			float Lab, Lat, IncSp;
			vec2 IncV(0, 0);
			Project(S.m_Core.m_Pos, k, Lab, Lat, IncSp, &IncV);
			Res.m_Lead = Lab - S.RaceTick();
			Res.m_Lat = Lat;
			Res.m_Sp = IncSp > 1e-3f ? dot(S.m_Core.m_Vel, IncV) / IncSp : length(S.m_Core.m_Vel);
			Res.m_IncSp = IncSp;
			if(g.m_pField)
			{
				float T = g.m_pField->Value(S.m_Core.m_Pos, S.m_Core.m_Vel);
				WinScore = g.m_Fw * (g.m_IncTEnd - T) - g.m_Lw * std::max(0.0f, Lat - g.m_Lat0);
			}
			else
				WinScore = Res.m_Lead + g.m_Kv * (Res.m_Sp - IncSp) - g.m_Lw * std::max(0.0f, Lat - g.m_Lat0);
		}
	}
	Res.m_Score = WinScore;
}

// ---- mutations
static bool Valid(const SPlan &P)
{
	for(size_t i = 0; i < P.m_H.size(); i++)
	{
		const SHookSeg &H = P.m_H[i];
		if(H.m_R <= H.m_P || H.m_R <= 0 || H.m_P >= g.m_NT)
			return false;
		if(i > 0 && H.m_P < P.m_H[i - 1].m_R + 1)
			return false;
		if(H.m_P < 0 && i > 0)
			return false;
	}
	for(size_t i = 1; i < P.m_S.size(); i++)
		if(P.m_S[i].m_F < P.m_S[i - 1].m_F + 1)
			return false;
	for(const SShot &S : P.m_S)
		if(S.m_F < 0 || S.m_F >= g.m_NT)
			return false;
	return true;
}

static float Gauss(std::mt19937 &R) { return std::normal_distribution<float>(0, 1)(R); }

static bool Mutate(SPlan &P, std::mt19937 &R, const std::vector<int> &vW)
{
	std::uniform_real_distribution<float> U(0, 1);
	int Tot = 0;
	for(int w : vW)
		Tot += w;
	int x = (int)(U(R) * Tot), Op = 0;
	while(Op < (int)vW.size() - 1 && x >= vW[Op])
		x -= vW[Op++];
	auto RandSeg = [&](bool PressOnly) -> int {
		std::vector<int> v;
		for(int i = 0; i < (int)P.m_H.size(); i++)
			if((!PressOnly || P.m_H[i].m_P >= 0) && P.m_H[i].m_P < g.m_N)
				v.push_back(i);
		if(v.empty())
			return -1;
		return v[(size_t)(U(R) * v.size()) % v.size()];
	};
	const int N = g.m_N;
	auto RandShot = [&]() -> int {
		int n = 0;
		while(n < (int)P.m_S.size() && P.m_S[n].m_F < N)
			n++;
		if(n == 0)
			return -1;
		return (int)(U(R) * n) % n;
	};
	switch(Op)
	{
	case 0: // press shift
	{
		int i = RandSeg(true);
		if(i < 0)
			return false;
		int d = U(R) < 0.5f ? -1 : 1;
		if(U(R) < 0.25f)
			d *= 2;
		P.m_H[i].m_P += d;
		if(P.m_H[i].m_P < 0)
			return false;
		if(U(R) < 0.5f)
			P.m_H[i].m_R += d; // keep the duration
		P.m_H[i].m_Orig = false;
		return true;
	}
	case 1: // release shift
	{
		int i = RandSeg(false);
		if(i < 0)
			return false;
		int d = U(R) < 0.5f ? -1 : 1;
		if(U(R) < 0.3f)
			d *= 1 + (int)(U(R) * 4);
		P.m_H[i].m_R = std::min(g.m_NT, P.m_H[i].m_R + d);
		return true;
	}
	case 2: // anchor / angle jitter
	{
		int i = RandSeg(true);
		if(i < 0)
			return false;
		SHookSeg &H = P.m_H[i];
		float Sc = U(R) < 0.7f ? 1.0f : 5.0f;
		if(H.m_Anch)
			H.m_A += vec2(Gauss(R), Gauss(R)) * (3.0f * Sc);
		else
			H.m_Ang += Gauss(R) * 0.01f * Sc;
		H.m_Orig = false;
		return true;
	}
	case 3: // drop a hold
	{
		int i = RandSeg(true);
		if(i < 0)
			return false;
		P.m_H.erase(P.m_H.begin() + i);
		return true;
	}
	case 4: // merge with the next hold
	{
		int i = RandSeg(false);
		if(i < 0 || i + 1 >= (int)P.m_H.size())
			return false;
		P.m_H[i].m_R = P.m_H[i + 1].m_R;
		P.m_H.erase(P.m_H.begin() + i + 1);
		return true;
	}
	case 5: // split a hold (re-press the same anchor)
	{
		int i = RandSeg(false);
		if(i < 0)
			return false;
		SHookSeg H = P.m_H[i];
		int Lo = std::max(H.m_P, 0) + 1, Hi = H.m_R - 2;
		if(Hi < Lo)
			return false;
		int k = Lo + (int)(U(R) * (Hi - Lo + 1));
		SHookSeg H2 = H;
		H2.m_P = k + 1;
		H2.m_Orig = false;
		if(H.m_P < 0)
		{
			// held at the cut: the anchor is the current hook position (unknown here) - re-press along the old aim
			H2.m_Anch = false;
			H2.m_Ang = std::atan2((float)H.m_TY, (float)H.m_TX);
		}
		P.m_H[i].m_R = k;
		P.m_H.insert(P.m_H.begin() + i + 1, H2);
		return true;
	}
	case 6: // add a tap in a gap
	{
		int k = (int)(U(R) * N);
		int Len = 1 + (int)(U(R) * 4);
		SHookSeg H;
		H.m_P = k;
		H.m_R = std::min(N, k + Len);
		H.m_Anch = false;
		H.m_Ang = U(R) * 2 * pi;
		// insert sorted
		auto It = std::lower_bound(P.m_H.begin(), P.m_H.end(), H, [](const SHookSeg &a, const SHookSeg &b) { return a.m_P < b.m_P; });
		P.m_H.insert(It, H);
		return true;
	}
	case 7: // shot shift
	{
		int i = RandShot();
		if(i < 0)
			return false;
		int d = (U(R) < 0.5f ? -1 : 1) * (1 + (int)(U(R) * (U(R) < 0.2f ? 8 : 2)));
		P.m_S[i].m_F += d;
		P.m_S[i].m_Orig = false;
		return P.m_S[i].m_F < N;
	}
	case 8: // explosion point jitter
	{
		int i = RandShot();
		if(i < 0)
			return false;
		SShot &S = P.m_S[i];
		float Sc = U(R) < 0.7f ? 1.0f : 6.0f;
		if(S.m_HasX)
			S.m_X += vec2(Gauss(R), Gauss(R)) * (4.0f * Sc);
		else
			S.m_Ang += Gauss(R) * 0.01f * Sc;
		S.m_Orig = false;
		return true;
	}
	case 9: // direction keys
	{
		int k = (int)(U(R) * N);
		int Len = 1 + (int)(U(R) * 6);
		int v = (int)(U(R) * 3) - 1;
		if(v == P.m_D[k])
			v = v == 1 ? -1 : v + 1;
		for(int j = k; j < std::min(N, k + Len); j++)
			P.m_D[j] = v;
		return true;
	}
	case 10: // jump: move a press or toggle
	{
		int k = (int)(U(R) * N);
		if(U(R) < 0.5f)
		{
			// find a jump rising edge near k and move it
			for(int d = 0; d < 30; d++)
				for(int s : {-1, 1})
				{
					int j = k + s * d;
					if(j > 0 && j < N && P.m_J[j] && !P.m_J[j - 1])
					{
						int m = U(R) < 0.5f ? -1 : 1;
						if(j + m <= 0 || j + m >= N)
							return false;
						if(m < 0)
							P.m_J[j - 1] = 1;
						else
							P.m_J[j] = 0;
						return true;
					}
				}
			return false;
		}
		P.m_J[k] = !P.m_J[k];
		return true;
	}
	case 11: // drop a shot
	{
		int i = RandShot();
		if(i < 0)
			return false;
		P.m_S.erase(P.m_S.begin() + i);
		return true;
	}
	case 12: // re-aim a shot (absolute angle, wide)
	{
		int i = RandShot();
		if(i < 0)
			return false;
		SShot &S = P.m_S[i];
		float A0 = S.m_HasX ? std::atan2((float)S.m_TY, (float)S.m_TX) : S.m_Ang;
		S.m_HasX = false;
		S.m_Ang = U(R) < 0.3f ? U(R) * 2 * pi : A0 + Gauss(R) * 0.6f;
		S.m_Orig = false;
		return true;
	}
	case 13: // re-aim a hook press (absolute angle, wide)
	{
		int i = RandSeg(true);
		if(i < 0)
			return false;
		SHookSeg &H = P.m_H[i];
		float A0 = std::atan2((float)H.m_TY, (float)H.m_TX);
		if(!H.m_Orig && !H.m_Anch)
			A0 = H.m_Ang;
		H.m_Anch = false;
		H.m_Ang = U(R) < 0.3f ? U(R) * 2 * pi : A0 + Gauss(R) * 0.5f;
		H.m_Orig = false;
		return true;
	}
	case 14: // shift the shot schedule from a shot on (the reload couples the kicks)
	{
		int i = RandShot();
		if(i < 0)
			return false;
		int d = (U(R) < 0.5f ? -1 : 1) * (1 + (int)(U(R) * 6));
		for(size_t j = i; j < P.m_S.size() && P.m_S[j].m_F < N; j++)
		{
			P.m_S[j].m_F += d;
			P.m_S[j].m_Orig = false;
		}
		return true;
	}
	case 16: // close kick: a shot aimed at a wall point the run passes within kickr px of, fired 1-18 ticks before
	{
		if(g.m_vOppK.empty())
			return false;
		const int o = (int)(U(R) * g.m_vOppK.size()) % (int)g.m_vOppK.size();
		const int F = std::max(0, g.m_vOppK[o] - 1 - (int)(U(R) * 18));
		if(F >= N)
			return false;
		// free the reload around it: drop shots fired within 12 ticks of F
		for(size_t j = 0; j < P.m_S.size();)
			if(std::abs(P.m_S[j].m_F - F) < 12 && P.m_S[j].m_F < N)
				P.m_S.erase(P.m_S.begin() + j);
			else
				j++;
		SShot S;
		S.m_F = F;
		S.m_HasX = true;
		S.m_X = g.m_vOppW[o] + vec2(Gauss(R), Gauss(R)) * 3.0f;
		auto It = std::lower_bound(P.m_S.begin(), P.m_S.end(), S, [](const SShot &a, const SShot &b) { return a.m_F < b.m_F; });
		P.m_S.insert(It, S);
		return true;
	}
	case 15: // add a shot (absolute angle)
	{
		SShot S;
		S.m_F = (int)(U(R) * N);
		S.m_HasX = false;
		S.m_Ang = U(R) * 2 * pi;
		auto It = std::lower_bound(P.m_S.begin(), P.m_S.end(), S, [](const SShot &a, const SShot &b) { return a.m_F < b.m_F; });
		P.m_S.insert(It, S);
		return true;
	}
	}
	return false;
}

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_opt MAP run=RUN cut=A end=B [iters=N] [threads=T] [seed=S] [kv=0.3] [lw=0.05] [lat0=12] [t0=0.3] [field=FILE fw=1] [out=FILE]\n");
		return 1;
	}
	CFastG::Init();
	std::string Run, Out, Field, WStr, Ref, Prefix, Shots, OutAlt;
	std::vector<float> vKvAlt;
	int Cut = -1, End = -1, Threads = 3, Seed = 1, Verbose = 0, Tail = 30, Shift = 0;
	long long Iters = 200000;
	float T0 = 0.3f;
	float KickR = 45;
	bool UseCk = true;
	int CloseW = 3;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "run") Run = V;
		else if(K == "ref") Ref = V;
		else if(K == "prefix") Prefix = V;
		else if(K == "shift") Shift = std::atoi(V.c_str());
		else if(K == "cut") Cut = std::atoi(V.c_str());
		else if(K == "end") End = std::atoi(V.c_str());
		else if(K == "tail") Tail = std::atoi(V.c_str());
		else if(K == "iters") Iters = std::atoll(V.c_str());
		else if(K == "threads") Threads = std::atoi(V.c_str());
		else if(K == "seed") Seed = std::atoi(V.c_str());
		else if(K == "kv") g.m_Kv = std::atof(V.c_str());
		else if(K == "lw") g.m_Lw = std::atof(V.c_str());
		else if(K == "lat0") g.m_Lat0 = std::atof(V.c_str());
		else if(K == "latmax") g.m_LatMax = std::atof(V.c_str());
		else if(K == "shots") Shots = V;
		else if(K == "t0") T0 = std::atof(V.c_str());
		else if(K == "field") Field = V;
		else if(K == "fw") g.m_Fw = std::atof(V.c_str());
		else if(K == "out") Out = V;
		else if(K == "w") WStr = V;
		else if(K == "kvalt")
		{
			for(size_t a = 0; a < V.size();)
			{
				size_t b = V.find(',', a);
				if(b == std::string::npos)
					b = V.size();
				vKvAlt.push_back(std::atof(V.substr(a, b - a).c_str()));
				a = b + 1;
			}
		}
		else if(K == "outalt") OutAlt = V;
		else if(K == "ck") UseCk = std::atoi(V.c_str()) != 0;
		else if(K == "kickr") KickR = std::atof(V.c_str());
		else if(K == "closew") CloseW = std::atoi(V.c_str());
		else if(K == "v") Verbose = std::atoi(V.c_str());
	}
	std::vector<STasInput> vIn = ReadInputs(Run.c_str());
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i0 = 0;
	for(; i0 < vIn.size() && !G.HasGrenade(); i0++)
		G.Step(vIn[i0]);
	CFastG F;
	F.FromGame(G);
	// run once: race tick per index, positions, the cut
	std::vector<int> vRt(vIn.size(), -1);
	std::vector<vec2> vP(vIn.size()), vV(vIn.size()), vFirePos(vIn.size()), vPressPos(vIn.size());
	std::vector<char> vShot(vIn.size(), 0), vGrab(vIn.size(), 0);
	std::vector<vec2> vGrabPos(vIn.size());
	size_t iA = 0, iB = 0;
	{
		CFastG R = F;
		for(size_t i = i0; i < vIn.size(); i++)
		{
			vFirePos[i] = R.m_Pos;
			vPressPos[i] = R.m_Core.m_Pos;
			int Np0 = R.m_NumProj, Rl0 = R.m_ReloadTimer;
			R.Step(vIn[i]);
			vRt[i] = R.RaceTick();
			vP[i] = R.m_Core.m_Pos;
			vV[i] = R.m_Core.m_Vel;
			vShot[i] = R.m_NumProj > Np0 || (vIn[i].m_Fire && Rl0 == 0 && R.m_ReloadTimer > 0);
			vGrab[i] = R.m_Core.m_HookState == HOOK_GRABBED;
			vGrabPos[i] = R.m_Core.m_HookPos;
			if(R.m_FinishTick >= 0)
			{
				g.m_IncFinish = R.RaceTick();
				vIn.resize(i + 1);
				break;
			}
		}
	}
	// window: inputs iA..iB-1; the cut state has race tick Cut - 1, the window's last step gives race tick End
	std::vector<STasInput> vPre;
	if(!Prefix.empty())
	{
		// start from the end of another line (e.g. a lead found by a previous window), the run's plan shifted by
		// Shift ticks (Shift > 0: we are Shift ticks ahead of the run)
		vPre = ReadInputs(Prefix.c_str());
		CTasGame G2;
		G2.Spawn(CTasGame::Map().m_vSpawns[0]);
		size_t j = 0;
		for(; j < vPre.size() && !G2.HasGrenade(); j++)
			G2.Step(vPre[j]);
		CFastG C;
		C.FromGame(G2);
		for(; j < vPre.size(); j++)
			C.Step(vPre[j]);
		g.m_Cut = C;
		Cut = C.RaceTick() + 1;
		for(iA = i0; iA < vIn.size() && vRt[iA] < Cut + Shift; iA++)
			;
		iB = std::min(vIn.size(), iA + std::max(1, End - C.RaceTick()));
	}
	else
	{
		for(iA = i0; iA < vIn.size() && vRt[iA] < Cut; iA++)
			;
		for(iB = iA; iB < vIn.size() && vRt[iB] < End; iB++)
			;
		iB = std::min(iB + 1, vIn.size());
		CFastG R = F;
		for(size_t i = i0; i < iA; i++)
			R.Step(vIn[i]);
		g.m_Cut = R;
		vPre.assign(vIn.begin(), vIn.begin() + iA);
	}
	g.m_RtA = g.m_Cut.RaceTick();
	g.m_N = (int)(iB - iA);
	g.m_NT = std::min((int)(vIn.size() - iA), g.m_N + Tail);
	std::printf("x_opt: run %s finish %d, window rt %d..%d (%d ticks, inputs %zu..%zu), survival tail %d\n", Run.c_str(), g.m_IncFinish, g.m_RtA, g.m_RtA + g.m_N, g.m_N, iA, iB, g.m_NT - g.m_N);
	if(Ref.empty())
	{
		for(size_t i = i0; i < vIn.size(); i++)
			if(vRt[i] > g.m_RtA)
			{
				g.m_vIncP.push_back(vP[i]);
				g.m_vIncRt.push_back(vRt[i]);
				g.m_vIncS.push_back(length(vV[i]));
				g.m_vIncV.push_back(vV[i]);
			}
	}
	else
	{
		// the reference path (lead, speed, finish) is another run, aligned by race tick
		std::vector<STasInput> vR = ReadInputs(Ref.c_str());
		CTasGame G2;
		G2.Spawn(CTasGame::Map().m_vSpawns[0]);
		size_t j = 0;
		for(; j < vR.size() && !G2.HasGrenade(); j++)
			G2.Step(vR[j]);
		CFastG Q;
		Q.FromGame(G2);
		g.m_IncFinish = -1;
		for(; j < vR.size(); j++)
		{
			Q.Step(vR[j]);
			if(Q.RaceTick() > g.m_Cut.RaceTick())
			{
				g.m_vIncP.push_back(Q.m_Core.m_Pos);
				g.m_vIncRt.push_back(Q.RaceTick());
				g.m_vIncS.push_back(length(Q.m_Core.m_Vel));
				g.m_vIncV.push_back(Q.m_Core.m_Vel);
			}
			if(Q.m_FinishTick >= 0)
			{
				g.m_IncFinish = Q.RaceTick();
				break;
			}
		}
		std::printf("reference %s: finish %d\n", Ref.c_str(), g.m_IncFinish);
	}
	g.m_vBase.assign(vIn.begin() + iA, vIn.begin() + iA + g.m_NT);
	// the plan
	SPlan P0;
	for(int k = 0; k < g.m_NT; k++)
	{
		P0.m_D.push_back((int8_t)vIn[iA + k].m_Dir);
		P0.m_J.push_back((int8_t)vIn[iA + k].m_Jump);
	}
	for(int k = 0; k < g.m_NT; k++)
	{
		size_t i = iA + k;
		if(!vIn[i].m_Hook)
			continue;
		bool Press = i == 0 || !vIn[i - 1].m_Hook;
		if(!Press && k > 0)
			continue;
		SHookSeg H;
		H.m_P = Press ? k : -1;
		int r = k;
		while(r < g.m_NT && vIn[iA + r].m_Hook)
			r++;
		H.m_R = r;
		H.m_TX = vIn[i].m_TX;
		H.m_TY = vIn[i].m_TY;
		H.m_Ang = std::atan2((float)H.m_TY, (float)H.m_TX);
		if(Press)
		{
			H.m_Orig = true;
			H.m_Ref = vPressPos[i];
			for(int j = k; j < r; j++)
				if(vGrab[iA + j])
				{
					vec2 D0 = normalize(vec2(H.m_TX, H.m_TY));
					H.m_Anch = true;
					H.m_A = H.m_Ref + D0 * dot(vGrabPos[iA + j] - H.m_Ref, D0);
					break;
				}
		}
		P0.m_H.push_back(H);
	}
	for(int k = 0; k < g.m_NT; k++)
	{
		size_t i = iA + k;
		if(!vShot[i])
			continue;
		SShot S;
		S.m_F = k;
		S.m_RefTick = k;
		S.m_TX = vIn[i].m_TX;
		S.m_TY = vIn[i].m_TY;
		S.m_Ang = std::atan2((float)S.m_TY, (float)S.m_TX);
		S.m_Orig = true;
		S.m_Ref = vFirePos[i];
		vec2 Col;
		if(Flight(vFirePos[i], normalize(vec2(S.m_TX, S.m_TY)), g.m_Cut.m_Core.m_Tuning, Col))
		{
			S.m_HasX = true;
			S.m_X = Col;
		}
		P0.m_S.push_back(S);
	}
	if(!Shots.empty())
	{
		// shots=RT:DEG,RT:DEG,...: the window's shots replaced by these (fired at race tick RT, absolute aim)
		std::vector<SShot> vS;
		for(const SShot &S : P0.m_S)
			if(S.m_F >= g.m_N)
				vS.push_back(S);
		for(size_t a = 0; a < Shots.size();)
		{
			size_t b = Shots.find(',', a);
			if(b == std::string::npos)
				b = Shots.size();
			int Rt;
			float Deg;
			if(std::sscanf(Shots.substr(a, b - a).c_str(), "%d:%f", &Rt, &Deg) == 2)
			{
				SShot S;
				S.m_F = Rt - g.m_RtA - 1;
				S.m_HasX = false;
				S.m_Ang = Deg * pi / 180.0f;
				if(S.m_F >= 0 && S.m_F < g.m_N)
					vS.push_back(S);
			}
			a = b + 1;
		}
		std::sort(vS.begin(), vS.end(), [](const SShot &x, const SShot &y) { return x.m_F < y.m_F; });
		P0.m_S = vS;
	}
	STField Fld;
	if(!Field.empty())
	{
		if(!Fld.Load(Field.c_str()))
		{
			std::printf("cannot load field %s\n", Field.c_str());
			return 1;
		}
		g.m_pField = &Fld;
		if(g.m_Fw == 0)
			g.m_Fw = 1;
		g.m_IncTEnd = Fld.Value(g.m_vIncP[g.m_N - 1], g.m_vIncV[g.m_N - 1]);
	}
	{
		// close-kick spots along the run's path in the window
		const SMapInfo &Mi = CTasGame::Map();
		for(int k = 0; k < g.m_N && k < (int)g.m_vIncP.size(); k++)
		{
			const vec2 Q = g.m_vIncP[k];
			float Bd = KickR;
			vec2 Bw(0, 0);
			for(int ty = (int)std::floor((Q.y - KickR) / 32); ty <= (int)std::floor((Q.y + KickR) / 32); ty++)
				for(int tx = (int)std::floor((Q.x - KickR) / 32); tx <= (int)std::floor((Q.x + KickR) / 32); tx++)
				{
					if(Mi.Tile(tx, ty) != TILE_SOLID)
						continue;
					const vec2 C(std::clamp(Q.x, tx * 32.0f, tx * 32.0f + 32.0f), std::clamp(Q.y, ty * 32.0f, ty * 32.0f + 32.0f));
					const float d = distance(Q, C);
					if(d < Bd && d > 0.5f)
					{
						Bd = d;
						Bw = C;
					}
				}
			if(Bd < KickR)
			{
				g.m_vOppK.push_back(k);
				g.m_vOppW.push_back(Bw + normalize(Bw - Q) * 4.0f); // a bit inside the tile
			}
		}
		std::printf("close-kick spots: %zu window ticks within %.0f px of a solid tile\n", g.m_vOppK.size(), KickR);
	}
	SRes Res0;
	Eval(P0, Res0, true);
	std::printf("plan: %zu hook holds, %zu shots; replay: score %.3f lead %.3f lat %.1f |v| %.2f (run %.2f)%s\n", P0.m_H.size(), P0.m_S.size(),
		Res0.m_Score, Res0.m_Lead, Res0.m_Lat, Res0.m_Sp, Res0.m_IncSp, Res0.m_Dead ? " DEAD" : "");
	std::vector<int> vW = {10, 10, 12, 3, 3, 3, 2, 8, 10, 6, 3, 1, 2, 2, 2, 1, CloseW};
	if(!WStr.empty())
	{
		vW.clear();
		for(size_t a = 0; a < WStr.size();)
		{
			size_t b = WStr.find(',', a);
			if(b == std::string::npos)
				b = WStr.size();
			vW.push_back(std::atoi(WStr.substr(a, b - a).c_str()));
			a = b + 1;
		}
	}
	// annealing, one chain per thread, the global best shared
	std::mutex Mu;
	SPlan Best = P0;
	float BestScore = Res0.m_Score;
	std::atomic<long long> Done{0};
	const int NA = (int)vKvAlt.size();
	std::vector<SPlan> vAltBest(NA, P0);
	std::vector<float> vAltScore(NA);
	for(int a = 0; a < NA; a++)
		vAltScore[a] = AltScore(Res0, vKvAlt[a]);
	auto Work = [&](int Tid) {
		std::vector<SPlan> vMyAlt(NA, P0);
		std::vector<float> vMyAltSc(vAltScore);
		std::mt19937 Rng(Seed * 1000 + Tid);
		std::uniform_real_distribution<float> U(0, 1);
		SPlan Cur = P0;
		SRes Rs;
		std::vector<SCkpt> vCkCur, vCkC;
		Eval(Cur, Rs, false, nullptr, 0, UseCk ? &vCkCur : nullptr);
		float Sc = Rs.m_Score;
		long long PerThread = Iters / Threads;
		for(long long it = 0; it < PerThread; it++)
		{
			float T = T0 * (1.0f - (float)it / PerThread);
			SPlan C = Cur;
			int Nm = 1 + (U(Rng) < 0.35f) + (U(Rng) < 0.12f);
			bool Ok = true;
			for(int m = 0; m < Nm && Ok; m++)
				Ok = Mutate(C, Rng, vW);
			if(!Ok || !Valid(C))
				continue;
			SRes Rc;
			if(UseCk)
				Eval(C, Rc, false, &vCkCur, FirstDiff(Cur, C), &vCkC);
			else
				Eval(C, Rc, false);
			Done++;
			for(int a = 0; a < NA; a++)
			{
				const float As = AltScore(Rc, vKvAlt[a]);
				if(As > vMyAltSc[a])
				{
					vMyAltSc[a] = As;
					vMyAlt[a] = C;
				}
			}
			if(Rc.m_Score >= Sc || (T > 0 && U(Rng) < std::exp((Rc.m_Score - Sc) / T)))
			{
				Cur = std::move(C);
				Sc = Rc.m_Score;
				if(UseCk)
					vCkCur.swap(vCkC);
				if(Sc > BestScore)
				{
					std::lock_guard<std::mutex> L(Mu);
					if(Sc > BestScore)
					{
						BestScore = Sc;
						Best = Cur;
						if(Verbose)
							std::printf("  t%d it %lld: score %.3f (lead %.3f lat %.1f |v| %.2f vs %.2f)\n", Tid, it, Sc, Rc.m_Lead, Rc.m_Lat, Rc.m_Sp, Rc.m_IncSp);
						std::fflush(stdout);
					}
				}
			}
			if(it % 20000 == 19999)
			{
				std::lock_guard<std::mutex> L(Mu);
				if(Sc < BestScore - 2 * T - 0.05f)
				{
					Cur = Best;
					Sc = BestScore;
					if(UseCk)
					{
						SRes Rr;
						Eval(Cur, Rr, false, nullptr, 0, &vCkCur);
					}
				}
				if(Tid == 0)
				{
					std::printf("  it %lld/%lld: best %.3f, chain %.3f, T %.3f\n", it + 1, PerThread, BestScore, Sc, T);
					std::fflush(stdout);
				}
			}
		}
		std::lock_guard<std::mutex> L(Mu);
		for(int a = 0; a < NA; a++)
			if(vMyAltSc[a] > vAltScore[a])
			{
				vAltScore[a] = vMyAltSc[a];
				vAltBest[a] = vMyAlt[a];
			}
	};
	std::vector<std::thread> vT;
	for(int t = 0; t < Threads; t++)
		vT.emplace_back(Work, t);
	for(auto &t : vT)
		t.join();
	SRes Rb;
	Eval(Best, Rb, true);
	std::printf("RESULT score %.3f lead %.3f lat %.1f |v| %.2f (run %.2f)%s fin %d, %lld evals\n", Rb.m_Score, Rb.m_Lead, Rb.m_Lat, Rb.m_Sp, Rb.m_IncSp,
		Rb.m_Dead ? " DEAD" : "", Rb.m_Fin, (long long)Done);
	std::printf("lead every 10:");
	for(float l : Rb.m_vLead)
		std::printf(" %.2f", l);
	std::printf("\n");
	std::printf("best plan: %zu hook holds, %zu shots (run %zu, %zu); shots at", Best.m_H.size(), Best.m_S.size(), P0.m_H.size(), P0.m_S.size());
	for(const SShot &S : Best.m_S)
		std::printf(" %d", g.m_RtA + S.m_F + 1);
	std::printf(" (run");
	for(const SShot &S : P0.m_S)
		std::printf(" %d", g.m_RtA + S.m_F + 1);
	std::printf(")\n");
	if(!Out.empty())
	{
		std::vector<STasInput> vO = vPre;
		vO.insert(vO.end(), Rb.m_vIn.begin(), Rb.m_vIn.begin() + std::min((int)Rb.m_vIn.size(), g.m_N));
		WriteInputs(Out.c_str(), vO);
		std::printf("wrote %s (%zu inputs, ends at rt %d)\n", Out.c_str(), vO.size(), g.m_RtA + g.m_N);
	}
	for(int a = 0; a < NA; a++)
	{
		SRes Ra;
		Eval(vAltBest[a], Ra, true);
		const bool Same = Ra.m_vIn.size() == Rb.m_vIn.size() && std::equal(Ra.m_vIn.begin(), Ra.m_vIn.begin() + std::min((int)Ra.m_vIn.size(), g.m_N), Rb.m_vIn.begin(),
			[](const STasInput &x, const STasInput &y) { return std::memcmp(&x, &y, sizeof(STasInput)) == 0; });
		std::printf("ALT kv %.2f score %.3f lead %.3f lat %.1f |v| %.2f (run %.2f)%s%s\n", vKvAlt[a], AltScore(Ra, vKvAlt[a]), Ra.m_Lead, Ra.m_Lat, Ra.m_Sp, Ra.m_IncSp,
			Ra.m_Dead ? " DEAD" : "", Same ? " same" : "");
		if(!OutAlt.empty() && !Same && !Ra.m_Dead)
		{
			char aBuf[512];
			std::snprintf(aBuf, sizeof(aBuf), "%s_k%d.txt", OutAlt.c_str(), a);
			std::vector<STasInput> vO = vPre;
			vO.insert(vO.end(), Ra.m_vIn.begin(), Ra.m_vIn.begin() + std::min((int)Ra.m_vIn.size(), g.m_N));
			WriteInputs(aBuf, vO);
			std::printf("wrote %s\n", aBuf);
		}
	}
	return 0;
}
