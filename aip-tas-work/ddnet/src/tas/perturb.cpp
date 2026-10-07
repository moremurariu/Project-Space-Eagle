// perturb: robustness of a run's continuation: replay to a cut, perturb the state (pos / vel), replay the rest of the
// inputs on the fast stepper; report finish tick (or death tick). Many perturbations per call.
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

// usage: perturb MAP INPUTS CUT_RT [n=200] [dpos=2] [dvel=0.5] [seed=1]
//   prints one line per perturbation: dx dy dvx dvy -> finish rt (or DEAD rt)
int main(int argc, const char **argv)
{
	if(argc < 4 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: perturb MAP INPUTS CUT_RT [n] [dpos] [dvel] [seed]\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const int CutRt = std::atoi(argv[3]);
	const int N = argc > 4 ? std::atoi(argv[4]) : 200;
	const float Dp = argc > 5 ? std::atof(argv[5]) : 2.0f, Dv = argc > 6 ? std::atof(argv[6]) : 0.5f;
	unsigned Seed = argc > 7 ? std::atoi(argv[7]) : 1;
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	const int Cut = CutRt + 68;
	for(int i = 0; i < Cut && i < (int)vIn.size(); i++)
		G.Step(vIn[i]);
	static CTasFast F0, F;
	F0.FromGameFull(G);
	auto Rnd = [&]() { Seed = Seed * 1103515245u + 12345u; return ((Seed >> 8) & 0xffff) / 65535.0f * 2.0f - 1.0f; };
	int Fin0 = -1;
	int nFin = 0, nBetter = 0, nSame = 0;
	double SumD = 0;
	for(int t = 0; t <= N; t++)
	{
		F = F0;
		float dx = 0, dy = 0, dvx = 0, dvy = 0;
		if(t > 0)
		{
			dx = std::round(Rnd() * Dp);
			dy = std::round(Rnd() * Dp);
			dvx = std::round(Rnd() * Dv * 256) / 256.0f;
			dvy = std::round(Rnd() * Dv * 256) / 256.0f;
			F.SetState(F.Pos() + vec2(dx, dy), F.Vel() + vec2(dvx, dvy));
		}
		int End = -1;
		bool Dead = false;
		for(int i = Cut; i < (int)vIn.size(); i++)
		{
			F.Step(vIn[i]);
			if(F.m_Dead)
			{
				Dead = true;
				End = F.m_Tick - F.m_StartTick;
				break;
			}
			if(F.m_FinishTick >= 0)
			{
				End = F.m_FinishTick - F.m_StartTick;
				break;
			}
		}
		if(t == 0)
		{
			Fin0 = End;
			std::printf("unperturbed: %s %d\n", Dead ? "DEAD" : (End >= 0 ? "finish" : "no finish"), End);
			continue;
		}
		if(!Dead && End >= 0)
		{
			nFin++;
			SumD += End - Fin0;
			if(End < Fin0)
				nBetter++;
			if(End == Fin0)
				nSame++;
		}
		if(getenv("PERT_ALL"))
			std::printf("%+.0f %+.0f %+.3f %+.3f -> %s %d\n", dx, dy, dvx, dvy, Dead ? "DEAD" : "finish", End);
	}
	std::printf("cut %d: %d/%d finish (same %d, better %d, mean d %.2f)\n", CutRt, nFin, N, nSame, nBetter, nFin ? SumD / nFin : 0.0);
	return 0;
}
