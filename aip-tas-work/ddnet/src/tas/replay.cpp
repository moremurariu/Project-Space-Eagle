// Replay an input file (one line per tick from spawn: dir jump hook fire tx ty weapon) on DDNet's prediction code.
#include "sim.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

int main(int argc, const char **argv)
{
	if(argc < 3)
	{
		std::printf("usage: replay MAP INPUTS [every]\n");
		return 1;
	}
	int Every = argc > 3 ? std::atoi(argv[3]) : 1;
	if(!CTasGame::LoadMap(argv[1]))
	{
		std::printf("map load failed\n");
		return 1;
	}
	const SMapInfo &M = CTasGame::Map();
	std::printf("map %dx%d spawns %d\n", M.m_W, M.m_H, (int)M.m_vSpawns.size());
	if(std::string(argv[2]) == "dump")
	{
		// game layer as text: one row per tile row, tile index as a char ('0'+index for small ones)
		for(int y = 0; y < M.m_H; y++)
		{
			for(int x = 0; x < M.m_W; x++)
			{
				int T = M.Tile(x, y), F = M.Front(x, y);
				char c = '.';
				if(T == 1) c = '#';
				else if(T == 3) c = 'N';
				else if(T == 9 || F == 9) c = 'f';
				else if(T == 12 || F == 12) c = 'D';
				else if(T == 11 || F == 11) c = 'u';
				else if(T == 33 || F == 33) c = 'S';
				else if(T == 34 || F == 34) c = 'F';
				else if(T >= 192) c = 'E';
				else if(T != 0) c = 'o';
				std::putchar(c);
			}
			std::putchar('\n');
		}
		return 0;
	}
	CTasGame G;
	G.Spawn(M.m_vSpawns[0]);
	FILE *f = std::fopen(argv[2], "r");
	int d, j, h, fi, tx, ty, w, t = 0;
	bool Got = false;
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
		G.Step(In);
		t++;
		if(G.HasGrenade() && !Got)
			std::printf("GRENADE at input %d (tick %d, start %d, race tick %d)\n", t, G.m_Tick, G.m_StartTick, G.m_Tick - G.m_StartTick);
		Got = G.HasGrenade();
		if(t % Every == 0 || G.Frozen())
			std::printf("t %d pos %.2f %.2f vel %.5f %.5f hook %d jumped %d frz %d start %d hp %.0f %.0f\n", t, G.Pos().x, G.Pos().y, G.Vel().x, G.Vel().y, G.HookState(), G.Jumped(), G.Frozen(), G.m_StartTick, G.HookPos().x, G.HookPos().y);
	}
	std::printf("end t %d start %d\n", t, G.m_StartTick);
	return 0;
}
