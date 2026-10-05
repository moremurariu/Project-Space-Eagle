// perturb <map> <inputs>: sensitivity of replaying the run's own inputs to small state perturbations at tick T
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "sim.h"

#include <game/mapitems.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	int d, j, h, fi, tx, ty, w;
	while(f && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
	{
		STasInput In;
		In.m_Dir = d; In.m_Jump = j; In.m_Hook = h; In.m_Fire = fi; In.m_TX = tx; In.m_TY = ty; In.m_Weapon = w;
		v.push_back(In);
	}
	if(f)
		std::fclose(f);
	return v;
}

int main(int argc, const char **argv)
{
	CTasGame::LoadMap(argv[1]);
	auto vIn = ReadInputs(argv[2]);
	for(int k = 0; k < 8; k++)
		vIn.push_back(vIn.back());
	vec2 Gren(170 * 32 + 16, 77 * 32 + 16);
	auto Run = [&](CTasGame &G, int From, float &TStar) {
		for(int i = From; i < (int)vIn.size(); i++)
		{
			vec2 P = G.Pos(), Q = G.Chr()->m_PrevPos;
			int Rt = G.m_Tick - G.m_StartTick;
			G.Step(vIn[i]);
			if(G.Frozen() || G.EnteredFreeze())
				return -i;
			if(G.HasGrenade())
			{
				float Dc = distance(P, Gren), Dp = distance(Q, Gren);
				float F = Dp > Dc + 1e-4f ? std::clamp((Dp - 48.0f) / (Dp - Dc), 0.0f, 1.0f) : 1.0f;
				TStar = (float)(Rt - 1) + F;
				return i;
			}
		}
		return 0;
	};
	for(int T : {300, 450, 600, 700, 800, 880, 930})
	{
		CTasGame Base;
		Base.Spawn(CTasGame::Map().m_vSpawns[0]);
		int Idx = 0;
		for(; Idx < (int)vIn.size(); Idx++)
		{
			Base.Step(vIn[Idx]);
			if(Base.m_Started && Base.m_Tick - Base.m_StartTick >= T)
			{
				Idx++;
				break;
			}
		}
		int Ok = 0, Dead = 0, NoPick = 0;
		float Sum = 0, Worst = -1e9f, Best = 1e9f;
		float T0 = 0;
		{
			CTasGame G;
			G.CopyFrom(Base);
			Run(G, Idx, T0);
		}
		for(int dx = -2; dx <= 2; dx++)
			for(int dy = -2; dy <= 2; dy++)
				for(float dv : {-0.1f, 0.0f, 0.1f})
				{
					if(!dx && !dy && dv == 0)
						continue;
					CTasGame G;
					G.CopyFrom(Base);
					G.SetState(Base.Pos() + vec2(dx, dy), Base.Vel() + vec2(dv, -dv));
					float Ts = 0;
					int r = Run(G, Idx, Ts);
					if(r > 0)
					{
						Ok++;
						Sum += Ts - T0;
						Worst = std::max(Worst, Ts - T0);
						Best = std::min(Best, Ts - T0);
					}
					else if(r < 0)
						Dead++;
					else
						NoPick++;
				}
		std::printf("T %d: base t* %.3f | ok %d dead %d nopickup %d | dt* mean %+.3f min %+.3f max %+.3f\n", T, T0, Ok, Dead, NoPick, Ok ? Sum / Ok : 0, Best, Worst);
	}
}
