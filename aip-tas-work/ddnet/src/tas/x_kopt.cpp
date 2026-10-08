// x_kopt: how much more speed could each of a run's grenade kicks give? For every real shot (post-pickup), the shot is
// removed and re-fired from every tick within +-W of its fire tick where the reload is free (25 ticks from the run's
// other shots, no fresh hook press that tick: the hook shares the aim), at N aims. A grenade's flight does not depend
// on the tee, and until it explodes the tee moves exactly as in the run, so each candidate is exact: the run's inputs
// with the shot moved / re-aimed, stepped (CFastG) until that grenade explodes (at most M ticks after the original
// explosion). Reports per shot: the run's own kick (tick, distance, |f|, |v| after) and the candidate with the largest
// |v| after its explosion, plus the best one that explodes no later than the original.
// usage: x_kopt <map> <inputs> [W=6] [N=1440] [M=6] [from_rt] [to_rt]
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include <game/collision.h>

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

struct SKick
{
	int m_Fire = -1, m_Expl = -1; // race ticks (after the step)
	float m_Dist = 0, m_F = 0, m_V = 0, m_Cos = 0, m_VB = 0, m_DE = -1e9f; // |v| before the kick, energy gain (v^2/2)
	vec2 m_Force = vec2(0, 0), m_VAfter = vec2(0, 0);
	int m_TX = 0, m_TY = 0, m_Idx = -1; // aim and input index of the shot
};

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_kopt <map> <inputs> [W=6] [N=1440] [M=6] [from_rt] [to_rt]\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const int W = argc > 3 ? std::atoi(argv[3]) : 6;
	const int N = argc > 4 ? std::atoi(argv[4]) : 1440;
	const int M = argc > 5 ? std::atoi(argv[5]) : 6;
	const int From = argc > 6 ? std::atoi(argv[6]) : -100000, To = argc > 7 ? std::atoi(argv[7]) : 100000;

	// replay to the pickup on CTasGame, then CFastG; record per-tick states (index = input index of the next step)
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i0 = 0;
	for(; i0 < vIn.size() && !G.HasGrenade(); i0++)
		G.Step(vIn[i0]);
	CFastG F0;
	F0.FromGame(G);
	std::vector<CFastG> vS; // vS[k] = state before input i0 + k
	std::vector<SExplLog> vLog;
	CFastG::ms_pLog = &vLog;
	std::vector<int> vShotIdx; // input indices whose step fired a grenade
	std::vector<std::vector<SExplLog>> vExpl;
	{
		CFastG F = F0;
		for(size_t i = i0; i < vIn.size(); i++)
		{
			vS.push_back(F);
			int Rl = F.m_ReloadTimer;
			vLog.clear();
			F.Step(vIn[i]);
			vExpl.push_back(vLog);
			if(F.m_ReloadTimer > Rl)
				vShotIdx.push_back((int)i);
			if(F.m_FinishTick >= 0)
				break;
		}
	}
	auto Rt = [&](int Idx) { return vS[Idx - i0].RaceTick() + 1; }; // race tick after the step of input Idx
	auto Ramp = [](float v) { return v * 50 < 550 ? 1.0f : 1.0f / std::pow(1.4f, (v * 50 - 550) / 2000.0f); };
	std::printf("# shot: fire_rt expl_rt dist |f| |v|before |v|after | best: fire_rt expl_rt dist |f| |v|after gain | best expl<=orig: ...\n");
	float SumGain = 0, SumGainE = 0;
	for(size_t s = 0; s < vShotIdx.size(); s++)
	{
		const int Idx = vShotIdx[s];
		if(Rt(Idx) < From || Rt(Idx) > To)
			continue;
		// the run without this shot
		std::vector<STasInput> vStrip = vIn;
		vStrip[Idx].m_Fire = 0;
		// the original explosion of this shot: replay the stripped run and the real one, the first tick where they differ
		// in projectile count after the fire is the explosion; simpler: step the real run from Idx and watch the
		// projectile started at that tick
		SKick Orig;
		// the stripped run's explosions per step (from Idx); the original shot's explosion is the one the real run has extra
		std::vector<std::vector<SExplLog>> vSE;
		float VBefore = 0;
		{
			CFastG F = vS[Idx - i0], R = vS[Idx - i0];
			for(int i = Idx; i < (int)vIn.size() && i - Idx < 110; i++)
			{
				vLog.clear();
				F.Step(vStrip[i]);
				vSE.push_back(vLog);
				vLog.clear();
				R.Step(vIn[i]);
				if(Orig.m_Expl < 0 && vLog.size() > vSE.back().size())
				{
					Orig.m_Fire = Rt(Idx);
					Orig.m_Expl = R.RaceTick();
					const SExplLog &L = vLog.back();
					Orig.m_Force = L.m_Force;
					Orig.m_F = length(L.m_Force);
					Orig.m_Dist = L.m_Dist;
					Orig.m_V = length(R.m_Core.m_Vel);
					Orig.m_VAfter = R.m_Core.m_Vel;
					Orig.m_VB = length(L.m_VelBefore);
					Orig.m_DE = 0.5f * (dot(L.m_VelBefore + L.m_Force, L.m_VelBefore + L.m_Force) - dot(L.m_VelBefore, L.m_VelBefore));
					VBefore = length(F.m_Core.m_Vel);
				}
				if(F.m_Dead)
					break;
			}
		}
		if(Orig.m_Expl < 0)
			continue;
		// neighbouring shots of the run constrain the reload
		int Prev = s > 0 ? Rt(vShotIdx[s - 1]) : -1000, Next = s + 1 < vShotIdx.size() ? Rt(vShotIdx[s + 1]) : 100000;
		SKick Best, BestE;
		for(int dT = -W; dT <= W; dT++)
		{
			const int CIdx = Idx + dT;
			if(CIdx < (int)i0 + 1 || CIdx >= (int)vIn.size())
				continue;
			const int CRt = Rt(CIdx);
			if(CRt - Prev < 25 || Next - CRt < 25)
				continue;
			const bool FreshHook = vStrip[CIdx].m_Hook && !vStrip[CIdx - 1].m_Hook;
			if(FreshHook || vStrip[CIdx].m_Fire || vStrip[CIdx - 1].m_Fire)
				continue;
			const CFastG &Base = vS[CIdx - i0];
			if(Base.m_ReloadTimer > 0)
				continue;
			for(int a = 0; a < N; a++)
			{
				float Ang = a * 2.0f * pi / N;
				STasInput In = vStrip[CIdx];
				In.m_Fire = 1;
				In.m_TX = (int)std::lround(std::cos(Ang) * 10000.0f);
				In.m_TY = (int)std::lround(std::sin(Ang) * 10000.0f);
				CFastG F = Base;
				bool Found = false;
				const SExplLog *pL = nullptr;
				for(int k = CIdx; k < (int)vIn.size() && !F.m_Dead; k++)
				{
					vLog.clear();
					F.Step(k == CIdx ? In : vStrip[k]);
					const int j = k - Idx; // step index in vSE (stripped run from Idx)
					const size_t NS = j >= 0 && j < (int)vSE.size() ? vSE[j].size() : 0;
					if(vLog.size() > NS)
					{
						Found = true;
						pL = &vLog.back();
						break;
					}
					if(F.RaceTick() >= Orig.m_Expl + M)
						break;
				}
				if(!Found)
					continue;
				SKick K;
				K.m_Fire = CRt;
				K.m_TX = In.m_TX;
				K.m_TY = In.m_TY;
				K.m_Idx = CIdx;
				K.m_Expl = F.RaceTick();
				K.m_Force = pL->m_Force;
				K.m_F = length(pL->m_Force);
				K.m_Dist = pL->m_Dist;
				K.m_V = length(F.m_Core.m_Vel);
				K.m_VAfter = F.m_Core.m_Vel;
				K.m_VB = length(pL->m_VelBefore);
				K.m_DE = 0.5f * (dot(pL->m_VelBefore + pL->m_Force, pL->m_VelBefore + pL->m_Force) - dot(pL->m_VelBefore, pL->m_VelBefore));
				if(K.m_DE > Best.m_DE)
					Best = K;
				if(K.m_Expl <= Orig.m_Expl && K.m_DE > BestE.m_DE)
					BestE = K;
			}
		}
		(void)Ramp;
		auto Cos = [](vec2 F, vec2 V) { return length(F) > 0 && length(V) > 0 ? dot(F, V) / length(F) / length(V) : 0.0f; };
		std::printf("shot %4d -> %4d d %5.1f |f| %5.2f cos %5.2f |v| %5.1f -> %5.1f dE %5.0f | best %4d -> %4d d %5.1f |f| %5.2f cos %5.2f |v| %5.1f -> %5.1f dE %5.0f (%+5.0f) | <=orig %4d -> %4d |f| %5.2f dE %5.0f (%+5.0f)\n",
			Orig.m_Fire, Orig.m_Expl, Orig.m_Dist, Orig.m_F, Cos(Orig.m_Force, Orig.m_VAfter - Orig.m_Force), Orig.m_VB, Orig.m_V, Orig.m_DE,
			Best.m_Fire, Best.m_Expl, Best.m_Dist, Best.m_F, Cos(Best.m_Force, Best.m_VAfter - Best.m_Force), Best.m_VB, Best.m_V, Best.m_DE,
			Best.m_DE - Orig.m_DE, BestE.m_Fire, BestE.m_Expl, BestE.m_F, BestE.m_DE, BestE.m_DE - Orig.m_DE);
		if(std::getenv("KOPT_AIMS"))
			std::printf("AIM orig_idx %d best_idx %d %d %d bestE_idx %d %d %d\n", Idx, Best.m_Idx, Best.m_TX, Best.m_TY, BestE.m_Idx, BestE.m_TX, BestE.m_TY);
		if(Best.m_DE > -1e8f)
			SumGain += Best.m_DE - Orig.m_DE;
		if(BestE.m_DE > -1e8f)
			SumGainE += BestE.m_DE - Orig.m_DE;
	}
	std::printf("TOTAL energy gain (v^2/2) if every kick were re-aimed: %+.0f (explosion no later than the original: %+.0f)\n", SumGain,
		SumGainE);
	return 0;
}
