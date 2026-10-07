// nademap: writes a minimal DDNet map for grenade experiments.
// usage: nademap OUT.map W H [spec...]
//   spec "b:X,Y,T"   tile T (default 3 = TILE_NOHOOK) at tile X,Y   (also "r:X0,Y0,X1,Y1,T" for a rectangle)
//   spec "s:X,Y"     spawn entity at tile X,Y
//   spec "g:X,Y"     grenade pickup entity at tile X,Y
// The whole map is air except what the specs place, so out-of-map coordinates (clamped to the border tiles) are air too.
#include <base/logger.h>
#include <base/os.h>

#include <engine/shared/datafile.h>
#include <engine/storage.h>

#include <game/gamecore.h>
#include <game/mapitems.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

int main(int argc, const char **argv)
{
	CCmdlineFix CmdlineFix(&argc, &argv);
	log_set_global_logger_default();
	if(argc < 4)
	{
		std::printf("usage: nademap OUT.map W H [b:X,Y[,T]] [r:X0,Y0,X1,Y1[,T]] [s:X,Y] [g:X,Y] ...\n");
		return 1;
	}
	const char *pOut = argv[1];
	const int W = std::atoi(argv[2]), H = std::atoi(argv[3]);
	std::vector<CTile> vTiles(W * H);
	for(auto &T : vTiles)
		T = CTile{.m_Index = TILE_AIR, .m_Flags = 0, .m_Skip = 0, .m_MustBe0 = 0};
	auto Set = [&](int x, int y, int Index) {
		if(x < 0 || y < 0 || x >= W || y >= H)
		{
			std::printf("tile %d,%d outside the map\n", x, y);
			std::exit(1);
		}
		vTiles[y * W + x].m_Index = Index;
	};
	for(int i = 4; i < argc; i++)
	{
		int a[5] = {0, 0, 0, 0, TILE_NOHOOK};
		if(!std::strncmp(argv[i], "b:", 2))
		{
			a[2] = TILE_NOHOOK;
			int n = std::sscanf(argv[i] + 2, "%d,%d,%d", &a[0], &a[1], &a[2]);
			if(n < 2)
				return 1;
			Set(a[0], a[1], a[2]);
		}
		else if(!std::strncmp(argv[i], "r:", 2))
		{
			int n = std::sscanf(argv[i] + 2, "%d,%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3], &a[4]);
			if(n < 4)
				return 1;
			for(int y = a[1]; y <= a[3]; y++)
				for(int x = a[0]; x <= a[2]; x++)
					Set(x, y, a[4]);
		}
		else if(!std::strncmp(argv[i], "s:", 2))
		{
			if(std::sscanf(argv[i] + 2, "%d,%d", &a[0], &a[1]) != 2)
				return 1;
			Set(a[0], a[1], ENTITY_OFFSET + ENTITY_SPAWN);
		}
		else if(!std::strncmp(argv[i], "g:", 2))
		{
			if(std::sscanf(argv[i] + 2, "%d,%d", &a[0], &a[1]) != 2)
				return 1;
			Set(a[0], a[1], ENTITY_OFFSET + ENTITY_WEAPON_GRENADE);
		}
		else
		{
			std::printf("bad spec %s\n", argv[i]);
			return 1;
		}
	}

	std::unique_ptr<IStorage> pStorage(CreateStorage(IStorage::EInitializationType::BASIC, argc, argv));
	CDataFileWriter Writer;
	if(!Writer.Open(pStorage.get(), pOut, IStorage::TYPE_ABSOLUTE))
	{
		std::printf("cannot write %s\n", pOut);
		return 1;
	}
	CMapItemVersion Version;
	Version.m_Version = 1;
	Writer.AddItem(MAPITEMTYPE_VERSION, 0, sizeof(Version), &Version);

	CMapItemGroup Group = {};
	Group.m_Version = 3;
	Group.m_OffsetX = 0;
	Group.m_OffsetY = 0;
	Group.m_ParallaxX = 100;
	Group.m_ParallaxY = 100;
	Group.m_StartLayer = 0;
	Group.m_NumLayers = 1;
	Group.m_UseClipping = 0;
	StrToInts(Group.m_aName, std::size(Group.m_aName), "Game");
	Writer.AddItem(MAPITEMTYPE_GROUP, 0, sizeof(Group), &Group);

	CMapItemLayerTilemap Layer = {};
	Layer.m_Layer.m_Version = 0;
	Layer.m_Layer.m_Type = LAYERTYPE_TILES;
	Layer.m_Layer.m_Flags = 0;
	Layer.m_Version = 3;
	Layer.m_Width = W;
	Layer.m_Height = H;
	Layer.m_Flags = TILESLAYERFLAG_GAME;
	Layer.m_Color.r = Layer.m_Color.g = Layer.m_Color.b = Layer.m_Color.a = 255;
	Layer.m_ColorEnv = -1;
	Layer.m_ColorEnvOffset = 0;
	Layer.m_Image = -1;
	Layer.m_Data = Writer.AddData(vTiles.size() * sizeof(CTile), vTiles.data());
	StrToInts(Layer.m_aName, std::size(Layer.m_aName), "Game");
	Layer.m_Tele = Layer.m_Speedup = Layer.m_Front = Layer.m_Switch = Layer.m_Tune = -1;
	Writer.AddItem(MAPITEMTYPE_LAYER, 0, sizeof(Layer), &Layer);
	Writer.Finish();
	std::printf("wrote %s (%dx%d tiles)\n", pOut, W, H);
	return 0;
}
