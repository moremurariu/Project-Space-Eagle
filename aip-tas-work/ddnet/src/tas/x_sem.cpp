// x_sem: semantic replay of a run (plan transfer). The run's inputs are replayed from another state, but every new
// hook press is re-aimed at the anchor the run's hook grabbed, and every shot is re-aimed so that its grenade hits the
// point where the run's grenade exploded; direction and jump keys are the run's own, optionally time-shifted.
//
// Why: the polished runs are knife-edge (NOTES: +-1 px at a cut kills ~73% of raw continuations within 30-100 ticks),
// so a section that a planner makes faster can only be kept by an exact rejoin (x_graft). If the run's plan
// (anchors, explosion points, keys) transfers to nearby states, a lead found anywhere can be carried to the finish.
//
// usage: x_sem MAP run=RUN cut=RT [pert=dx,dy,dvx,dvy] [prefix=FILE] [shift=D] [raw=1] [out=FILE] [v=1]
//   cut: start from RUN's own state at race tick RT (perturbed by pert), or from the end of prefix=FILE
//   shift: RUN's tick used for our tick t is t + D (D > 0: we are D ticks ahead of RUN)
//   raw=1: replay RUN's raw inputs (no re-aiming) for comparison
// prints: death / finish race tick and the lead over RUN (projection on RUN's path) every 25 ticks
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include "tasio.h"

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

struct SPlanTick
{
	STasInput m_In;
	bool m_Press = false, m_HasAnchor = false;
	vec2 m_Anchor;
	bool m_Shot = false, m_HasExpl = false;
	vec2 m_Expl;
	int m_ExplT = 0; // flight ticks
	vec2 m_Pos; // run's position after this step
	vec2 m_PressPos, m_FirePos; // run's core / character position at the press / shot (before the step)
};

static const float R0 = 28.0f * 0.75f;

// grenade flight from the tee position P (CCharacter::m_Pos at fire time) along Dir: first collision (tick offset, point)
static bool Flight(vec2 P, vec2 Dir, const CTuningParams &Tu, int &T, vec2 &Col)
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
		{
			T = k;
			return true;
		}
	}
	return false;
}

// aim from P so that the grenade's straight-plus-drop path passes through E (the first collision should be there)
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

static void SetAim(STasInput &In, vec2 D)
{
	In.m_TX = (int16_t)std::lround(D.x * 10000);
	In.m_TY = (int16_t)std::lround(D.y * 10000);
	if(!In.m_TX && !In.m_TY)
		In.m_TY = -1;
}

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_sem MAP run=RUN cut=RT [pert=dx,dy,dvx,dvy] [prefix=FILE] [shift=D] [raw=1] [out=FILE]\n");
		return 1;
	}
	CFastG::Init();
	std::string Run, Prefix, Out;
	int Cut = -1, Shift = 0, Raw = 0, Verbose = 0;
	float Pert[4] = {0, 0, 0, 0};
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "run") Run = V;
		else if(K == "cut") Cut = std::atoi(V.c_str());
		else if(K == "prefix") Prefix = V;
		else if(K == "shift") Shift = std::atoi(V.c_str());
		else if(K == "raw") Raw = std::atoi(V.c_str());
		else if(K == "out") Out = V;
		else if(K == "v") Verbose = std::atoi(V.c_str());
		else if(K == "pert") std::sscanf(V.c_str(), "%f,%f,%f,%f", &Pert[0], &Pert[1], &Pert[2], &Pert[3]);
	}
	std::vector<STasInput> vIn = ReadInputs(Run.c_str());
	// 1. the run's plan: per input index, its hook anchors and shot explosion points
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i0 = 0;
	for(; i0 < vIn.size() && !G.HasGrenade(); i0++)
		G.Step(vIn[i0]);
	CFastG F;
	F.FromGame(G);
	const int StartTick = F.m_StartTick; // race tick of input index i = i + 1 - StartTick... use F.RaceTick() after a step
	std::vector<SPlanTick> vPlan(vIn.size());
	std::vector<int> vRt(vIn.size(), -1);
	int RunFinish = -1;
	{
		CFastG R = F;
		int PendPress = -1;
		for(size_t i = i0; i < vIn.size(); i++)
		{
			SPlanTick &P = vPlan[i];
			P.m_In = vIn[i];
			P.m_Press = vIn[i].m_Hook && (i == 0 || !vIn[i - 1].m_Hook);
			int Np0 = R.m_NumProj;
			int Rl0 = R.m_ReloadTimer;
			vec2 FirePos = R.m_Pos;
			P.m_PressPos = R.m_Core.m_Pos;
			P.m_FirePos = FirePos;
			R.Step(vIn[i]);
			vRt[i] = R.RaceTick();
			P.m_Pos = R.m_Core.m_Pos;
			if(P.m_Press)
				PendPress = (int)i;
			if(!vIn[i].m_Hook)
				PendPress = -1;
			if(PendPress >= 0 && R.m_Core.m_HookState == HOOK_GRABBED)
			{
				// the grab point lies on the original ray; the stored hook position is quantized, so rebuild it exactly
				SPlanTick &Q = vPlan[PendPress];
				vec2 D0 = normalize(vec2(Q.m_In.m_TX, Q.m_In.m_TY));
				Q.m_HasAnchor = true;
				Q.m_Anchor = Q.m_PressPos + D0 * dot(R.m_Core.m_HookPos - Q.m_PressPos, D0);
				PendPress = -1;
			}
			if(R.m_NumProj > Np0 || (vIn[i].m_Fire && Rl0 == 0 && R.m_ReloadTimer > 0))
			{
				P.m_Shot = true;
				vec2 D = normalize(vec2(vIn[i].m_TX, vIn[i].m_TY));
				int T;
				vec2 Col;
				if(Flight(FirePos, D, R.m_Core.m_Tuning, T, Col))
				{
					P.m_HasExpl = true;
					P.m_Expl = Col;
					P.m_ExplT = T;
				}
			}
			if(R.m_FinishTick >= 0)
			{
				RunFinish = R.RaceTick();
				break;
			}
		}
	}
	// 2. start state
	CFastG S = F;
	std::vector<STasInput> vOut;
	size_t iStart;
	if(!Prefix.empty())
	{
		std::vector<STasInput> vP = ReadInputs(Prefix.c_str());
		CTasGame G2;
		G2.Spawn(CTasGame::Map().m_vSpawns[0]);
		size_t k = 0;
		for(; k < vP.size() && !G2.HasGrenade(); k++)
			G2.Step(vP[k]);
		S.FromGame(G2);
		for(; k < vP.size(); k++)
			S.Step(vP[k]);
		vOut = vP;
		iStart = vP.size();
	}
	else
	{
		size_t k = i0;
		for(; k < vIn.size() && vRt[k] < Cut; k++)
			S.Step(vIn[k]);
		vOut.assign(vIn.begin(), vIn.begin() + k);
		iStart = k;
		S.m_Core.m_Pos += vec2(Pert[0], Pert[1]);
		S.m_Pos = S.m_Core.m_Pos;
		S.m_Core.m_Vel += vec2(Pert[2], Pert[3]);
	}
	std::printf("x_sem: run finish %d, start at rt %d pos %.1f %.1f v %.2f %.2f, shift %d, %s\n", RunFinish, S.RaceTick(), S.m_Core.m_Pos.x,
		S.m_Core.m_Pos.y, S.m_Core.m_Vel.x, S.m_Core.m_Vel.y, Shift, Raw ? "raw replay" : "semantic replay");
	// 3. replay
	int Fin = -1, Dead = -1;
	float Hint = 0;
	for(size_t k = iStart;; k++)
	{
		size_t j = k + Shift; // the run's input index for our step
		if(j >= vIn.size() || j < i0)
			break;
		STasInput In = vIn[j];
		const SPlanTick &P = vPlan[j];
		if(!Raw)
		{
			if(P.m_Press && P.m_HasAnchor && distance(S.m_Core.m_Pos, P.m_PressPos) > 1e-3f)
				SetAim(In, normalize(P.m_Anchor - S.m_Core.m_Pos));
			if(P.m_Shot && P.m_HasExpl && S.m_ReloadTimer == 0 && distance(S.m_Pos, P.m_FirePos) > 1e-3f)
				SetAim(In, AimAt(S.m_Pos, P.m_Expl, S.m_Core.m_Tuning));
		}
		S.Step(In);
		vOut.push_back(In);
		if(S.m_Dead)
		{
			Dead = S.RaceTick();
			break;
		}
		if(S.m_FinishTick >= 0)
		{
			Fin = S.RaceTick();
			break;
		}
		int Rt = S.RaceTick();
		if(Rt % 25 == 0 || Verbose)
		{
			// lead: projection on the run's path near index j
			float Best = 1e9f, BestK = 0;
			for(int d = -40; d <= 40; d++)
			{
				int a = (int)j + d;
				if(a < (int)i0 + 1 || a + 1 >= (int)vIn.size() || vRt[a] < 0 || vRt[a + 1] < 0)
					continue;
				vec2 A = vPlan[a].m_Pos, B = vPlan[a + 1].m_Pos, AB = B - A;
				float L2 = dot(AB, AB);
				float u = L2 > 1e-6f ? std::clamp(dot(S.m_Core.m_Pos - A, AB) / L2, 0.0f, 1.0f) : 0.0f;
				float dd = distance(S.m_Core.m_Pos, A + AB * u);
				if(dd < Best)
				{
					Best = dd;
					BestK = vRt[a] + u;
				}
			}
			Hint = BestK;
			std::printf("rt %d lead %.2f lat %.1f |v| %.1f\n", Rt, Hint - Rt, Best, length(S.m_Core.m_Vel));
		}
	}
	std::printf("RESULT %s dead %d finish %d (run %d)\n", Raw ? "raw" : "sem", Dead, Fin, RunFinish);
	if(!Out.empty())
		WriteInputs(Out.c_str(), vOut);
	return 0;
}
