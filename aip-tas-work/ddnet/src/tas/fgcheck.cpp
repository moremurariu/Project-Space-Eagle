// fgcheck: CTasGame vs CTasFast (CFastG) tick by tick from a cut of a run; first mismatch and the explosions around it.
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

// usage: fgcheck MAP INPUTS CUT_LINES
int main(int argc, const char **argv)
{
	if(argc < 4 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: fgcheck MAP INPUTS CUT_LINES\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const int Cut = std::atoi(argv[3]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	for(int i = 0; i < Cut && i < (int)vIn.size(); i++)
		G.Step(vIn[i]);
	static CTasFast F;
	F.FromGameFull(G);
	std::vector<SExplLog> vLog;
	CFastG::ms_pLog = &vLog;
	for(int i = Cut; i < (int)vIn.size(); i++)
	{
		vLog.clear();
		G.Step(vIn[i]);
		F.Step(vIn[i]);
		for(const auto &L : vLog)
			std::printf("fast expl tick %d at %.1f %.1f tee %.1f %.1f force %.2f %.2f\n", L.m_Tick, L.m_E.x, L.m_E.y, L.m_Tee.x, L.m_Tee.y, L.m_Force.x, L.m_Force.y);
		vec2 Pg = G.Pos(), Vg = G.Vel(), Pf = F.Pos(), Vf = F.Vel();
		if(F.Frozen() || F.EnteredFreeze() || F.m_StartTick == -2 || G.Frozen() || G.EnteredFreeze())
			std::printf("FLAG input %d: fast frozen %d entered %d start %d | game frozen %d entered %d\n", i, (int)F.Frozen(), (int)F.EnteredFreeze(), F.m_StartTick, (int)G.Frozen(), (int)G.EnteredFreeze());
		if(Pg != Pf || Vg != Vf || G.ReloadTimer() != F.ReloadTimer() || G.NumProjectiles() != F.NumProjectiles() || G.HookState() != F.HookState())
		{
			std::printf("MISMATCH input %d (rt %d): game pos %.2f %.2f vel %.3f %.3f reload %d proj %d hook %d | fast pos %.2f %.2f vel %.3f %.3f reload %d proj %d hook %d bad %d\n", i,
				G.m_Tick - G.m_StartTick, Pg.x, Pg.y, Vg.x, Vg.y, G.ReloadTimer(), G.NumProjectiles(), G.HookState(), Pf.x, Pf.y, Vf.x, Vf.y, F.ReloadTimer(), F.NumProjectiles(), F.HookState(), (int)F.m_Bad);
			std::printf("input: %d %d %d %d %d %d %d\n", vIn[i].m_Dir, vIn[i].m_Jump, vIn[i].m_Hook, vIn[i].m_Fire, vIn[i].m_TX, vIn[i].m_TY, vIn[i].m_Weapon);
			return 0;
		}
	}
	std::printf("no mismatch from input %d to %zu (finish game %d fast %d)\n", Cut, vIn.size(), G.m_FinishTick - G.m_StartTick, F.m_FinishTick - F.m_StartTick);
	return 0;
}
