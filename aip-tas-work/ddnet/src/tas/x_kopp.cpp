// x_kopp: kick opportunities along a run. At every tick after the grenade pickup, pretend the grenade is loaded and
// try N aims (exact CFastG step with the run's own input + fire at that aim); report the best same-tick kick
// (largest gain in speed |v|) and the best kick component along the run's own velocity.
// usage: x_kopp <map> <inputs> [N=720] [from_rt] [to_rt]
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include <game/collision.h>
#include "sim.h"

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

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_kopp <map> <inputs> [N] [from_rt] [to_rt]\n");
		return 1;
	}
	CFastG::Init();
	int N = argc > 3 ? std::atoi(argv[3]) : 720;
	int From = argc > 4 ? std::atoi(argv[4]) : -100000;
	int To = argc > 5 ? std::atoi(argv[5]) : 100000;
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i = 0;
	for(; i < vIn.size() && !G.HasGrenade(); i++)
		G.Step(vIn[i]);
	CFastG F;
	F.FromGame(G);
	std::vector<SExplLog> vLog;
	for(; i + 1 < vIn.size(); i++)
	{
		int rt = F.RaceTick() + 1;
		if(rt >= From && rt <= To && !(vIn[i].m_Hook && !vIn[i - 1].m_Hook))
		{
			// run's own next velocity (no shot) for reference
			CFastG A = F;
			A.m_NumProj = 0;
			A.m_ReloadTimer = 0;
			STasInput In0 = vIn[i];
			In0.m_Fire = 0;
			A.Step(In0);
			vec2 V0 = A.m_Core.m_Vel;
			float BestDv = -1e9, BestAl = -1e9, BestDvA = 0, BestAlA = 0, BestDvF = 0, BestAlF = 0;
			for(int a = 0; a < N; a++)
			{
				float Ang = 2 * pi * a / N;
				CFastG B = F;
				B.m_NumProj = 0;
				B.m_ReloadTimer = 0;
				STasInput In = vIn[i];
				In.m_Fire = 1;
				In.m_TX = (int)std::lround(std::cos(Ang) * 10000);
				In.m_TY = (int)std::lround(std::sin(Ang) * 10000);
				if(B.m_Fire & 1)
					B.m_Fire++; // make it a fresh press
				vLog.clear();
				CFastG::ms_pLog = &vLog;
				B.Step(In);
				CFastG::ms_pLog = nullptr;
				if(vLog.empty())
					continue;
				vec2 V1 = B.m_Core.m_Vel;
				float Dv = length(V1) - length(V0);
				float Al = length(V0) > 0.5f ? dot(V1 - V0, normalize(V0)) : 0;
				if(Dv > BestDv)
				{
					BestDv = Dv;
					BestDvA = Ang * 180 / pi;
					BestDvF = length(vLog[0].m_Force);
				}
				if(Al > BestAl)
				{
					BestAl = Al;
					BestAlA = Ang * 180 / pi;
					BestAlF = length(vLog[0].m_Force);
				}
			}
			std::printf("K %d %.0f %.0f |v| %.1f bestdv %.2f aim %.1f f %.1f | bestalong %.2f aim %.1f f %.1f | rl %d\n", rt, F.m_Pos.x, F.m_Pos.y, length(F.m_Core.m_Vel),
				BestDv > -1e8 ? BestDv : 0.0f, BestDvA, BestDvF, BestAl > -1e8 ? BestAl : 0.0f, BestAlA, BestAlF, F.m_ReloadTimer);
		}
		F.Step(vIn[i]);
		if(F.m_FinishTick >= 0)
			break;
	}
	return 0;
}
