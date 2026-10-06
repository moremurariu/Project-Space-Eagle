// nadecheck: replay an input file on CTasGame (client prediction world) and CNadeG (server projectile semantics)
// side by side; report mismatches, every explosion and the height reached.
// usage: nadecheck MAP INPUTS [trace=0] [extra=200]
//   extra: idle ticks appended after the inputs (fire released) so the apex is reached
#include "nade.h"

#include <base/logger.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, const char **argv)
{
	if(getenv("NADE_LOG"))
		log_set_global_logger_default();
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: nadecheck MAP INPUTS [trace=0] [extra=200]\n");
		return 1;
	}
	CNadeG::Init();
	const int Trace = argc > 3 ? std::atoi(argv[3]) : 0;
	const int Extra = argc > 4 ? std::atoi(argv[4]) : 200;
	std::vector<STasInput> vIn = ReadInputFile(argv[2]);
	if(vIn.empty())
	{
		std::printf("no inputs\n");
		return 1;
	}
	STasInput Idle = vIn.back();
	Idle.m_Fire = 0;
	Idle.m_Jump = 0;
	Idle.m_Dir = 0;
	for(int i = 0; i < Extra; i++)
		vIn.push_back(Idle);

	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	CNadeG F;
	F.FromGame(G);
	std::vector<SNadeExpl> vLog;
	F.m_pLog = &vLog;
	const float Y0 = G.Pos().y;
	float MinY = Y0, MinYF = Y0;
	int MinTick = 0, MinTickF = 0, FirstMismatch = -1;
	int Fires = 0;
	std::printf("spawn %.1f %.1f\n", G.Pos().x, G.Pos().y);
	for(size_t i = 0; i < vIn.size(); i++)
	{
		int NP = G.NumProjectiles();
		G.Step(vIn[i]);
		size_t L0 = vLog.size();
		F.Step(vIn[i]);
		if(G.NumProjectiles() > NP)
			Fires++;
		bool Same = G.Pos() == F.m_Pos && G.Vel() == F.m_Core.m_Vel;
		if(!Same && FirstMismatch < 0)
			FirstMismatch = (int)i + 1;
		if(G.Pos().y < MinY)
		{
			MinY = G.Pos().y;
			MinTick = (int)i + 1;
		}
		if(F.m_Pos.y < MinYF)
		{
			MinYF = F.m_Pos.y;
			MinTickF = (int)i + 1;
		}
		for(size_t k = L0; k < vLog.size(); k++)
		{
			const SNadeExpl &E = vLog[k];
			std::printf("tick %4d explosion at %.1f %.1f (%s) tee %.0f %.0f dist %.1f kick %.2f %.2f\n", (int)i + 1, E.m_E.x, E.m_E.y, E.m_Lifetime ? "lifetime" : "hit",
				E.m_Tee.x, E.m_Tee.y, E.m_Dist, E.m_Force.x, E.m_Force.y);
		}
		if(Trace)
			std::printf("%4d G %.0f %.0f %.3f %.3f | F %.0f %.0f %.3f %.3f grenade=%d weapon=%d reload=%d proj=%d %s\n", (int)i + 1, G.Pos().x, G.Pos().y, G.Vel().x, G.Vel().y,
				F.m_Pos.x, F.m_Pos.y, F.m_Core.m_Vel.x, F.m_Core.m_Vel.y, G.HasGrenade(), G.ActiveWeapon(), G.ReloadTimer(), G.NumProjectiles(), Same ? "" : "MISMATCH");
	}
	std::printf("CTasGame: start y %.1f, min y %.1f at tick %d -> height %.1f px = %.2f tiles\n", Y0, MinY, MinTick, Y0 - MinY, (Y0 - MinY) / 32);
	std::printf("CNadeG  : min y %.1f at tick %d -> height %.1f px = %.2f tiles; grenades fired %d, double-explosion risk %d, bad %d\n", MinYF, MinTickF, Y0 - MinYF,
		(Y0 - MinYF) / 32, Fires, F.m_DoubleRisk, F.m_Bad);
	std::printf("first mismatch: %d\n", FirstMismatch);
	return 0;
}
