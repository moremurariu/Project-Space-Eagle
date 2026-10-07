// x_trace: replay a run (CTasGame up to the grenade pickup, then the exact CFastG stepper) and print one line per
// tick plus every explosion with its distance, force, alignment and v^2 gain.
// usage: x_trace <map> <inputs> [from_rt]
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
		std::printf("usage: x_trace <map> <inputs> [from_rt]\n");
		return 1;
	}
	CFastG::Init();
	int From = argc > 3 ? std::atoi(argv[3]) : -100000;
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i = 0;
	for(; i < vIn.size() && !G.HasGrenade(); i++)
		G.Step(vIn[i]);
	CFastG F;
	F.FromGame(G);
	std::vector<SExplLog> vLog;
	CFastG::ms_pLog = &vLog;
	for(; i < vIn.size(); i++)
	{
		vec2 V0 = F.m_Core.m_Vel;
		vLog.clear();
		F.Step(vIn[i]);
		const STasInput &In = vIn[i];
		int rt = F.RaceTick();
		if(rt < From)
			continue;
		vec2 P = F.m_Core.m_Pos, V = F.m_Core.m_Vel;
		std::printf("T %d %d %.2f %.2f %.3f %.3f %.3f hs %d hp %.0f %.0f in %d %d %d %d %d %d rl %d np %d gr %d j %d%s%s\n", (int)i, rt, P.x, P.y, V.x, V.y,
			length(V), F.m_Core.m_HookState, F.m_Core.m_HookPos.x, F.m_Core.m_HookPos.y, In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY,
			F.m_ReloadTimer, F.m_NumProj, (int)CTasGame::Collision()->IsOnGround(F.m_Pos, 28.0f), F.m_Core.m_Jumped, F.m_Dead ? " DEAD" : "",
			F.m_FinishTick >= 0 ? " FINISH" : "");
		for(const SExplLog &E : vLog)
		{
			vec2 Vb = E.m_VelBefore;
			vec2 Va = Vb + E.m_Force;
			float c = length(Vb) > 0.01f && length(E.m_Force) > 0.01f ? dot(normalize(Vb), normalize(E.m_Force)) : 0;
			std::printf("E %d %d ex %.1f %.1f tee %.1f %.1f dist %.1f force %.2f %.2f |f| %.2f vb %.2f %.2f |vb| %.2f |va| %.2f cos %.2f dv2 %.0f\n", (int)i,
				E.m_Tick - F.m_StartTick, E.m_E.x, E.m_E.y, E.m_Tee.x, E.m_Tee.y, E.m_Dist, E.m_Force.x, E.m_Force.y, length(E.m_Force), Vb.x, Vb.y, length(Vb),
				length(Va), c, dot(Va, Va) - dot(Vb, Vb));
		}
		if(F.m_FinishTick >= 0)
			break;
		(void)V0;
	}
	return 0;
}
