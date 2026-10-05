// mapdump <map>: print the game layer as text (# solid, n nohook, f freeze, d death, S start, F finish,
// E entity, . air), one row per tile row.
#include "sim.h"

#include <game/mapitems.h>

#include <cstdio>

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: mapdump <map>\n");
		return 1;
	}
	const SMapInfo &M = CTasGame::Map();
	if(argc > 2)
	{
		// mapdump <map> ents: entity tiles "x y type" (type = index - ENTITY_OFFSET)
		for(int y = 0; y < M.m_H; y++)
			for(int x = 0; x < M.m_W; x++)
				if(M.Tile(x, y) >= ENTITY_OFFSET)
					std::printf("%d %d %d\n", x, y, M.Tile(x, y) - ENTITY_OFFSET);
		return 0;
	}
	for(int y = 0; y < M.m_H; y++)
	{
		for(int x = 0; x < M.m_W; x++)
		{
			int T = M.Tile(x, y), F = M.Front(x, y);
			char c = '.';
			if(T == TILE_SOLID)
				c = '#';
			else if(T == TILE_NOHOOK)
				c = 'n';
			else if(T == TILE_FREEZE || F == TILE_FREEZE)
				c = 'f';
			else if(T == TILE_DEATH || F == TILE_DEATH)
				c = 'd';
			else if(T == TILE_START || F == TILE_START)
				c = 'S';
			else if(T == TILE_FINISH || F == TILE_FINISH)
				c = 'F';
			else if(T >= ENTITY_OFFSET)
				c = 'E';
			else if(T != 0)
				c = '?';
			std::putchar(c);
		}
		std::putchar('\n');
	}
	return 0;
}
