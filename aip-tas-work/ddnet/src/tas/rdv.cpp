// rdv: rendezvous (pre-fire) opportunities along a run. From the state at every tick, a grenade fired with each of
// N aims is flown exactly (CFastG, projectiles ignore the tee in solo); if it explodes 'minflight'..'maxflight' ticks
// later next to where the run itself is at that tick, the kick it would have given (strength, d(v^2) with the run's
// velocity) is reported. Shows where long pre-fires / double kicks are geometrically available on our own line.
// usage: rdv MAP INPUTS [from_rt=1000] [to_rt=99999] [aims=180] [minflight=10] [maxflight=30] [mingain=300] [loaded=0]
//   loaded=1: only ticks where the run itself holds a loaded grenade (a lob there displaces none of its shots)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "tasfast.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
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

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: rdv MAP INPUTS [from_rt] [to_rt] [aims] [minflight] [maxflight] [mingain]\n");
		return 1;
	}
	CFastG::Init();
	const int R0 = argc > 3 ? std::atoi(argv[3]) : 1000, R1 = argc > 4 ? std::atoi(argv[4]) : 99999;
	const int NA = argc > 5 ? std::atoi(argv[5]) : 180, F0 = argc > 6 ? std::atoi(argv[6]) : 10, F1 = argc > 7 ? std::atoi(argv[7]) : 30;
	const float MinGain = argc > 8 ? std::atof(argv[8]) : 300;
	const bool Loaded = argc > 9 && std::atoi(argv[9]);
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	std::vector<CTasFast> vS; // state before input i
	std::vector<vec2> vP, vV; // after input i
	std::vector<int> vRt;
	// RDV_TP="lines,x,y,vx,vy,tpk,reload": teleport after the first `lines` inputs like seg's tp= (diagnostics)
	int TpLines = -1;
	float TpX = 0, TpY = 0, TpVx = 0, TpVy = 0;
	int TpK = 0, TpRl = -1;
	if(getenv("RDV_TP"))
		std::sscanf(getenv("RDV_TP"), "%d,%f,%f,%f,%f,%d,%d", &TpLines, &TpX, &TpY, &TpVx, &TpVy, &TpK, &TpRl);
	for(size_t n = 0; n < vIn.size(); n++)
	{
		const STasInput &In = vIn[n];
		if((int)n == TpLines)
		{
			G.SetState(vec2(TpX, TpY), vec2(TpVx, TpVy));
			G.m_Tick += 4000;
			G.Chr()->m_Core.m_HookState = HOOK_IDLE;
			G.Chr()->m_Core.m_HookTick = 0;
			G.m_LastHook = 0;
			G.m_StartTick = G.m_Tick - TpK;
			if(TpRl >= 0)
				G.Chr()->m_ReloadTimer = TpRl;
		}
		CTasFast F;
		F.FromGameFull(G);
		vS.push_back(F);
		G.Step(In);
		vP.push_back(G.Pos());
		vV.push_back(G.Vel());
		vRt.push_back(G.m_Started && G.m_StartTick >= 0 ? G.m_Tick - G.m_StartTick : -1000);
	}
	if(getenv("RDV_OWN"))
	{
		// every explosion of the run (exact, logged by CFastG): tick, point, distance to the tee, kick, alignment
		size_t i = 0;
		while(i < vIn.size() && (!vS[i].HasGrenade() || (TpLines >= 0 && (int)i < TpLines)))
			i++;
		if(i >= vIn.size())
			return 0;
		CTasFast F = vS[i];
		std::vector<SExplLog> vLog;
		CFastG::ms_pLog = &vLog;
		for(; i < vIn.size(); i++)
			F.Step(vIn[i]);
		CFastG::ms_pLog = nullptr;
		float SumK = 0, SumGain = 0;
		int N = 0;
		for(const auto &L : vLog)
		{
			int Rt = L.m_Tick - F.m_StartTick;
			if(Rt < R0 || Rt > R1)
				continue;
			vec2 V = L.m_VelBefore, K = L.m_Force;
			float Sp = length(V), Kl = length(K);
			float Cos = Sp > 0 && Kl > 0 ? dot(K, V) / (Kl * Sp) : 0;
			std::printf("expl rt %d at %.0f %.0f: tee %.0f %.0f dist %.0f kick %.1f px/t cos %.2f |v| %.1f -> %.1f d(v2) %+.0f\n", Rt, L.m_E.x, L.m_E.y, L.m_Tee.x, L.m_Tee.y, L.m_Dist,
				Kl, Cos, Sp, length(V + K), dot(V + K, V + K) - dot(V, V));
			SumK += Kl;
			SumGain += dot(V + K, V + K) - dot(V, V);
			N++;
		}
		std::printf("%d explosions, mean kick %.2f px/t, total d(v2) %+.0f\n", N, N ? SumK / N : 0, SumGain);
		return 0;
	}
	if(getenv("RDV_SCHED"))
	{
		// shot-schedule planner on a fixed path: every (fire tick, aim) flown exactly (immediate explosions via the
		// CFastG explosion log), valued by how long its speed gain lasts on this path (until the next speed minimum,
		// capped at SCHED_H ticks), then the best set of fire ticks with >= 25 ticks between them (dynamic programming).
		// Writes a seg plan (SCHED_OUT): "t t ex ey r te tol" per selected shot.
		const int H = getenv("SCHED_H") ? std::atoi(getenv("SCHED_H")) : 60;
		const float R = getenv("SCHED_R") ? std::atof(getenv("SCHED_R")) : 32;
		const char *pOut = getenv("SCHED_OUT");
		const int N = vIn.size();
		std::vector<float> vSp(N);
		for(int i = 0; i < N; i++)
			vSp[i] = length(vV[i]);
		auto NextMin = [&](int j) {
			for(int k = j + 1; k < N - 1; k++)
			{
				bool Min = true;
				for(int d = -8; d <= 8 && Min; d++)
					if(k + d >= 0 && k + d < N && vSp[k + d] < vSp[k] - 1e-3f)
						Min = false;
				if(Min)
					return k;
			}
			return N - 1;
		};
		struct SShot { float m_Val = -1; int m_Aim = 0, m_Te = 0; vec2 m_E; float m_Kick = 0, m_Dv = 0; };
		std::vector<SShot> vBest(N);
		int I0 = -1, I1 = -1;
		for(int i = 1; i < N; i++)
		{
			if(vRt[i] < R0 || vRt[i] > R1 || !vS[i].HasGrenade() || (TpLines >= 0 && i < TpLines))
				continue;
			if(I0 < 0)
				I0 = i;
			I1 = i;
			if(vIn[i].m_Hook && !vIn[i - 1].m_Hook)
				continue; // a fresh hook press uses the same aim
			for(int a = 0; a < NA; a++)
			{
				float Ang = 2 * pi * a / NA;
				CTasFast F = vS[i];
				F.m_ReloadTimer = 0;
				STasInput In = vIn[i];
				In.m_Fire = 1;
				In.m_Weapon = 3;
				In.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
				In.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
				if(F.m_Fire & 1)
					F.m_Fire++;
				std::vector<SExplLog> vLog;
				CFastG::ms_pLog = &vLog;
				F.Step(In);
				CFastG::ms_pLog = nullptr;
				vec2 E, K;
				int j;
				if(!vLog.empty())
				{
					// exploded in the fire step itself (point-blank): the force was applied in this step
					E = vLog.back().m_E;
					K = vLog.back().m_Force;
					j = i;
				}
				else
				{
					int Te;
					if(!F.NextExplosion(E, Te))
						continue;
					int Fl = Te - F.m_Tick;
					j = i + Fl;
					if(j >= N)
						continue;
					vec2 P = vP[j - 1];
					float d = distance(P, E);
					float l = 1 - std::clamp((d - 48.0f) / 87.0f, 0.0f, 1.0f);
					float Dmg = d < 135 + 28 ? 6 * l : 0;
					if(!(int)Dmg)
						continue;
					K = (d > 0 ? normalize(P - E) : vec2(0, 1)) * Dmg * 2;
				}
				vec2 V = j > 0 ? vV[j - 1] : vV[0];
				float Dv = length(V + K) - length(V);
				if(Dv <= 0)
					continue;
				int Last = std::min(NextMin(j) - j, H);
				float Val = Dv / std::max(length(V), 10.0f) * std::max(Last, 1);
				if(Val > vBest[i].m_Val)
				{
					vBest[i].m_Val = Val;
					vBest[i].m_Aim = a;
					vBest[i].m_Te = vRt[i] + (j - i);
					vBest[i].m_E = E;
					vBest[i].m_Kick = length(K);
					vBest[i].m_Dv = Dv;
				}
			}
		}
		if(I0 < 0)
			return 0;
		// DP over fire ticks: the first shot after the reload the path has at I0
		const int Rl0 = vS[I0].m_ReloadTimer;
		std::vector<float> vF(N, -1);
		std::vector<int> vPrev(N, -1);
		float BestTot = 0;
		int BestEnd = -1;
		for(int i = I0; i <= I1; i++)
		{
			if(vBest[i].m_Val <= 0 || i < I0 + Rl0)
				continue;
			float Bp = 0;
			int Pi = -1;
			for(int k = I0; k <= i - 25; k++)
				if(vF[k] > Bp)
				{
					Bp = vF[k];
					Pi = k;
				}
			vF[i] = vBest[i].m_Val + Bp;
			vPrev[i] = Pi;
			if(vF[i] > BestTot)
			{
				BestTot = vF[i];
				BestEnd = i;
			}
		}
		std::vector<int> vSel;
		for(int i = BestEnd; i >= 0; i = vPrev[i])
			vSel.push_back(i);
		std::reverse(vSel.begin(), vSel.end());
		FILE *f = pOut ? std::fopen(pOut, "w") : nullptr;
		std::printf("schedule (value %.1f, reload %d at rt %d):\n", BestTot, Rl0, vRt[I0]);
		for(int i : vSel)
		{
			const SShot &S = vBest[i];
			std::printf("  fire rt %d aim %.1f -> expl rt %d at %.0f %.0f kick %.1f dv %+.1f value %.1f\n", vRt[i], 360.0f * S.m_Aim / NA, S.m_Te, S.m_E.x, S.m_E.y, S.m_Kick, S.m_Dv, S.m_Val);
			if(f)
				std::fprintf(f, "%d %d %.0f %.0f %.0f %d 1\n", vRt[i], vRt[i], S.m_E.x, S.m_E.y, R, S.m_Te);
		}
		if(f)
			std::fclose(f);
		// the path's own shots for comparison
		std::printf("path's own shots:");
		for(int i = std::max(I0, 1); i <= I1; i++)
			if(vS[i + 1 < N ? i + 1 : i].m_ReloadTimer > vS[i].m_ReloadTimer)
				std::printf(" %d", vRt[i]);
		std::printf("\n");
		return 0;
	}
	int Count = 0;
	for(size_t i = 0; i < vIn.size(); i++)
	{
		const int Rt = vRt[i];
		if(Rt < R0 || Rt > R1 || !vS[i].HasGrenade())
			continue;
		if(Loaded && (vS[i].m_ReloadTimer != 0 || vIn[i].m_Fire))
			continue;
		float BestG = -1e9f;
		int BestA = -1, BestT = 0;
		vec2 BestE;
		for(int a = 0; a < NA; a++)
		{
			float Ang = 2 * pi * a / NA;
			CTasFast F = vS[i];
			F.m_ReloadTimer = 0;
			STasInput In = vIn[i];
			In.m_Fire = 1;
			In.m_Weapon = 3;
			In.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
			In.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
			if(F.m_Fire & 1)
				F.m_Fire++; // make it a fresh press
			F.Step(In);
			vec2 E;
			int Te;
			if(!F.NextExplosion(E, Te))
				continue;
			int Fl = Te - F.m_Tick;
			if(Fl < F0 || Fl > F1)
				continue;
			size_t j = i + Fl + 1; // the run's state after the step that ends at server tick Te ... index of that tick
			if(j >= vP.size())
				continue;
			// the explosion applies before the tee moves in tick Te: compare with the position after tick Te-1
			vec2 P = vP[j - 1], V = vV[j - 1];
			float d = distance(P, E);
			if(d >= 135 + 28)
			{
				if(getenv("RDV_ALL") && getenv("RDV_MISS"))
					std::printf("  rt %d aim %.1f flight %d expl %.0f %.0f at rt %d: tee %.0f %.0f dist %.0f (miss)\n", Rt, 360.0f * a / NA, Fl, E.x, E.y, Rt + Fl, P.x, P.y, d);
				continue;
			}
			float l = 1 - std::clamp((d - 48.0f) / 87.0f, 0.0f, 1.0f);
			float Dmg = 6 * l;
			if(!(int)Dmg)
				continue;
			vec2 K = (d > 0 ? normalize(P - E) : vec2(0, 1)) * Dmg * 2;
			float Gain = dot(V + K, V + K) - dot(V, V);
			if(getenv("RDV_ALL"))
				std::printf("  rt %d aim %.1f flight %d expl %.0f %.0f at rt %d: tee %.0f %.0f v %.1f %.1f dist %.0f kick %.1f gain %+.0f\n", Rt, 360.0f * a / NA, Fl, E.x, E.y,
					Rt + Fl, P.x, P.y, V.x, V.y, d, length(K), Gain);
			if(Gain > BestG)
			{
				BestG = Gain;
				BestA = a;
				BestT = Fl;
				BestE = E;
			}
		}
		if(BestA >= 0 && BestG >= MinGain)
		{
			Count++;
			// the run's own next shot after i (a point-blank there stacks with the lob if it comes >= 25 ticks later)
			int Next = -1;
			for(size_t k = i + 1; k < vIn.size() && k < i + 60; k++)
				if(vIn[k].m_Fire && !vIn[k - 1].m_Fire)
				{
					Next = vRt[k];
					break;
				}
			std::printf("rt %d pos %.0f %.0f |v| %.1f: aim %.1f flight %d -> expl %.0f %.0f at rt %d, tee |v| %.1f, gain %+.0f (run's next shot rt %d)\n", Rt, vP[i].x, vP[i].y,
				length(vV[i]), 360.0f * BestA / NA, BestT, BestE.x, BestE.y, Rt + BestT, length(vV[i + BestT]), BestG, Next);
		}
	}
	std::printf("%d ticks with a rendezvous kick >= %.0f\n", Count, MinGain);
	return 0;
}
