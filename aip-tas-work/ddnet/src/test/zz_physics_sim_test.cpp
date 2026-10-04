// Physics experiments (killtile skipping, aleds) driving the real server code.
// Run: ./testrunner --gtest_filter='Sim.*'  (some tests take minutes; use one core)
#include "test.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cmath>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <numeric>

// access private state of characters for experiments
#define private public
#define protected public
#include <base/hash.h>
#include <base/io.h>
#include <base/logger.h>
#include <base/thread.h>
#include <base/types.h>
#include <engine/engine.h>
#include <engine/http.h>
#include <engine/kernel.h>
#include <engine/server/databases/connection.h>
#include <engine/server/databases/connection_pool.h>
#include <engine/server/register.h>
#include <engine/server/server.h>
#include <engine/server/server_logger.h>
#include <engine/shared/assertion_logger.h>
#include <engine/shared/config.h>
#include <engine/shared/datafile.h>
#include <engine/shared/network.h>
#include <engine/shared/snapshot.h>
#include <game/collision.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/gameworld.h>
#include <game/server/player.h>
#include <game/version.h>
#include <generated/protocol.h>
#include <zlib.h>
#undef private
#undef protected

// swallows all log output during simulation (the test logger keeps every line in memory)
class CNullLogger : public ILogger
{
public:
	void Log(const CLogMessage *pMessage) override {}
};
static CNullLogger gNullLogger;

// map loaded by the fixture; demo tests generate their own map into the test storage first
static const char *gpSimMapName = "coverage";
static void (*gpfnPrepareMap)(IStorage *pStorage) = nullptr;

class Sim : public ::testing::Test // NOLINT(readability-identifier-naming)
{
public:
	IGameServer *m_pGameServer = nullptr;
	CServer *m_pServer = nullptr;
	std::unique_ptr<IKernel> m_pKernel;
	CTestInfo m_TestInfo;
	std::unique_ptr<IStorage> m_pStorage;
	CConfig m_ConfigBackup;

	CGameContext *GameServer() { return (CGameContext *)m_pGameServer; }
	CCollision *Col() { return GameServer()->Collision(); }

	Sim()
	{
		m_ConfigBackup = g_Config;
		CServer *pServer = CreateServer();
		m_pServer = pServer;
		m_pKernel = std::unique_ptr<IKernel>(IKernel::Create());
		m_pKernel->RegisterInterface(m_pServer);
		IEngine *pEngine = CreateTestEngine(GAME_NAME);
		m_pKernel->RegisterInterface(pEngine);
		m_TestInfo.m_DeleteTestStorageFilesOnSuccess = true;
		m_pStorage = m_TestInfo.CreateTestStorage();
		m_pKernel->RegisterInterface(m_pStorage.get(), false);
		IConsole *pConsole = CreateConsole(CFGFLAG_SERVER | CFGFLAG_ECON).release();
		m_pKernel->RegisterInterface(pConsole);
		IConfigManager *pConfigManager = CreateConfigManager();
		m_pKernel->RegisterInterface(pConfigManager);
		IEngineHttp *pEngineHttp = CreateEngineHttp();
		m_pKernel->RegisterInterface(pEngineHttp);
		m_pKernel->RegisterInterface(static_cast<IHttp *>(pEngineHttp), false);
		IEngineAntibot *pEngineAntibot = CreateEngineAntibot();
		m_pKernel->RegisterInterface(pEngineAntibot);
		m_pKernel->RegisterInterface(static_cast<IAntibot *>(pEngineAntibot), false);
		m_pGameServer = CreateGameServer();
		m_pKernel->RegisterInterface(m_pGameServer);
		pEngine->Init();
		pConsole->Init();
		pConfigManager->Init();
		m_pServer->RegisterCommands();
		if(gpfnPrepareMap)
			gpfnPrepareMap(m_pStorage.get());
		EXPECT_NE(m_pServer->LoadMap(gpSimMapName), 0);
		m_pServer->m_RunServer = CServer::RUNNING;
		m_pServer->m_AuthManager.Init();
		{
			int Size = GameServer()->PersistentClientDataSize();
			for(auto &Client : m_pServer->m_aClients)
			{
				Client.m_HasPersistentData = false;
				Client.m_pPersistentData = malloc(Size);
			}
		}
		m_pServer->m_pPersistentData = malloc(GameServer()->PersistentDataSize());
		EXPECT_NE(m_pServer->LoadMap(gpSimMapName), 0);
		EXPECT_TRUE(pEngineHttp->Init(std::chrono::seconds{2}));
		pServer->m_NetServer.SetCallbacks(CServer::NewClientCallback, CServer::NewClientNoAuthCallback,
			CServer::ClientRejoinCallback, CServer::DelClientCallback, pServer);
		pServer->m_Econ.Init(pServer->Config(), pServer->Console(), &pServer->m_ServerBan);
		pServer->m_Fifo.Init(pServer->Console(), g_Config.m_SvInputFifo, CFGFLAG_SERVER);
		m_pServer->Antibot()->Init();
		GameServer()->OnInit(nullptr);
		g_Config.m_DbgDummies = 3;
		m_pServer->UpdateDebugDummies(false);
	}

	~Sim() override
	{
		m_pServer->m_Econ.Shutdown();
		m_pServer->m_Fifo.Shutdown();
		m_pGameServer->OnShutdown(nullptr);
		m_pServer->DbPool()->OnShutdown();
		g_Config = m_ConfigBackup;
	}

	// ---- map painting (tile coordinates) ----
	void ClearRect(int x0, int y0, int x1, int y1)
	{
		CCollision *c = Col();
		int W = c->m_Width, H = c->m_Height;
		for(int y = std::max(0, y0); y <= std::min(H - 1, y1); y++)
			for(int x = std::max(0, x0); x <= std::min(W - 1, x1); x++)
			{
				int i = y * W + x;
				c->m_pTiles[i].m_Index = TILE_AIR;
				if(c->m_pFront)
					c->m_pFront[i].m_Index = TILE_AIR;
				if(c->m_pTele)
					c->m_pTele[i].m_Type = 0;
				if(c->m_pSpeedup)
					c->m_pSpeedup[i].m_Type = 0;
				if(c->m_pSwitch)
					c->m_pSwitch[i].m_Type = 0;
				if(c->m_pTune)
					c->m_pTune[i].m_Type = 0;
			}
	}
	void SetTile(int x, int y, int Index) { Col()->m_pTiles[y * Col()->m_Width + x].m_Index = Index; }
	void FillRect(int x0, int y0, int x1, int y1, int Index)
	{
		for(int y = y0; y <= y1; y++)
			for(int x = x0; x <= x1; x++)
				SetTile(x, y, Index);
	}

	// ---- tees ----
	struct STee
	{
		int m_Cid;
		CNetObj_PlayerInput m_Input;
	};
	STee m_aTee[3];

	int CidOf(int t) { return m_pServer->MaxClients() - 1 - t; }
	CCharacter *Chr(int t) { return GameServer()->m_apPlayers[CidOf(t)]->GetCharacter(); }

	// freed snap ids normally wait for a real-time timeout; we respawn thousands of
	// times per second, so return them immediately (otherwise the pool runs dry and
	// every spawn logs "invalid id" into the in-memory test logger)
	void RecycleSnapIds()
	{
		m_pServer->m_IdPool.TimeoutIds();
	}

	// spawns tees; the tee spawned LAST ticks FIRST in the world
	void Spawn(int t, vec2 Pos)
	{
		CLogScope Quiet(&gNullLogger);
		CPlayer *p = GameServer()->m_apPlayers[CidOf(t)];
		p->KillCharacter(WEAPON_GAME, false);
		GameServer()->m_World.RemoveEntities();
		RecycleSnapIds();
		CCharacter *c = p->ForceSpawn(Pos);
		c->m_Core.m_Vel = vec2(0, 0);
		c->m_PrevPos = Pos;
		c->m_Core.m_ActiveWeapon = WEAPON_HAMMER;
		m_aTee[t].m_Cid = CidOf(t);
		mem_zero(&m_aTee[t].m_Input, sizeof(CNetObj_PlayerInput));
		m_aTee[t].m_Input.m_TargetX = 1;
		// prime m_NumInputs so that direct input may fire
		for(int k = 0; k < 3; k++)
			c->OnDirectInput(&m_aTee[t].m_Input);
	}
	void Kill(int t)
	{
		CLogScope Quiet(&gNullLogger);
		CPlayer *p = GameServer()->m_apPlayers[CidOf(t)];
		p->KillCharacter(WEAPON_GAME, false);
		GameServer()->m_World.RemoveEntities();
		RecycleSnapIds();
	}
	void PressFire(int t)
	{
		if(!(m_aTee[t].m_Input.m_Fire & 1))
			m_aTee[t].m_Input.m_Fire++;
	}
	void ReleaseFire(int t)
	{
		if(m_aTee[t].m_Input.m_Fire & 1)
			m_aTee[t].m_Input.m_Fire++;
	}
	void Aim(int t, vec2 Dir)
	{
		m_aTee[t].m_Input.m_TargetX = round_to_int(Dir.x * 256);
		m_aTee[t].m_Input.m_TargetY = round_to_int(Dir.y * 256);
		if(!m_aTee[t].m_Input.m_TargetX && !m_aTee[t].m_Input.m_TargetY)
			m_aTee[t].m_Input.m_TargetY = -1;
	}

	// one server tick, same order as CServer::Run: early input (direct input -> fire),
	// tick++, predicted input, game tick (world: Tick all, then TickDeferred all)
	void Step(int NumTees)
	{
		CLogScope Quiet(&gNullLogger);
		for(int t = 0; t < NumTees; t++)
			if(Chr(t))
				GameServer()->OnClientPredictedEarlyInput(m_aTee[t].m_Cid, &m_aTee[t].m_Input);
		m_pServer->m_CurrentGameTick++;
		for(int t = 0; t < NumTees; t++)
			if(Chr(t))
				GameServer()->OnClientPredictedInput(m_aTee[t].m_Cid, &m_aTee[t].m_Input);
		GameServer()->OnTick();
		if(m_Recording)
			SnapDemo();
		GameServer()->OnPostGlobalSnap(); // clear events (explosions, sounds) like a real snapshot would
	}

	bool m_Recording = false;
	void SnapDemo()
	{
		CSnapshotBuffer Data;
		m_pServer->m_SnapshotBuilder.Init();
		GameServer()->OnSnap(-1, true, true);
		int Size = m_pServer->m_SnapshotBuilder.Finish(&Data);
		if(getenv("SIM_DBG") && !Data.AsSnapshot()->IsValid(Size))
			printf("dbg invalid snapshot size=%d items=%d\n", Size, Data.AsSnapshot()->NumItems());
		else if(getenv("SIM_DBG"))
			printf("dbg valid snapshot size=%d items=%d\n", Size, Data.AsSnapshot()->NumItems());
		m_pServer->m_aDemoRecorder[CServer::RECORDER_MANUAL].RecordSnapshot(m_pServer->Tick(), Data.AsSnapshot(), Size);
	}
};

// replace the game layer with a big empty one and drop all special layers
static std::vector<CTile> gBigTiles;
static void UseBigMap(CCollision *c, int W, int H, CGameWorld *pWorld)
{
	// remove map entities (turrets, lasers, pickups...): they spam new entities every tick
	for(int Type = 0; Type < CGameWorld::NUM_ENTTYPES; Type++)
		if(Type != CGameWorld::ENTTYPE_CHARACTER)
			for(CEntity *e = pWorld->m_apFirstEntityTypes[Type]; e; e = e->m_pNextTypeEntity)
				e->m_MarkedForDestroy = true;
	pWorld->RemoveEntities();
	gBigTiles.assign((size_t)W * H, CTile{});
	c->m_pTiles = gBigTiles.data();
	c->m_Width = W;
	c->m_Height = H;
	c->m_pFront = nullptr;
	c->m_pTele = nullptr;
	c->m_pSpeedup = nullptr;
	c->m_pSwitch = nullptr;
	c->m_pTune = nullptr;
	c->m_pDoor = nullptr;
}

static void PrintMapInfo(CCollision *c)
{
	printf("map %dx%d front=%p tele=%p speedup=%p switch=%p tune=%p\n", c->m_Width, c->m_Height,
		(void *)c->m_pFront, (void *)c->m_pTele, (void *)c->m_pSpeedup, (void *)c->m_pSwitch, (void *)c->m_pTune);
}

TEST_F(Sim, MapInfo)
{
	PrintMapInfo(Col());
}

// Launch tee 0 at a band of death tiles N tiles thick.
// Dir: 0 = down, 1 = up, 2 = right. Returns true if the tee survived crossing.
// *pCrossD = displacement (px) on the tick the tee jumped past/into the band.
struct SCross
{
	bool m_Survived;
	int m_CrossD;
	int m_Ticks;
};

static const int BAND = 300; // band starts at tile 300 on the relevant axis
static const int W_BIG = 600, H_BIG = 600;

TEST_F(Sim, KillSkipSweep)
{
	UseBigMap(Col(), W_BIG, H_BIG, &GameServer()->m_World);
	auto Cross = [&](int Dir, int N, float V0, int Offset) -> SCross {
		for(auto &t : gBigTiles)
			t.m_Index = TILE_AIR;
		const int Lo = BAND * 32, Hi = (BAND + N) * 32; // band pixel range [Lo, Hi)
		vec2 Pos, Vel;
		if(Dir == 0)
		{
			FillRect(250, BAND, 350, BAND + N - 1, TILE_DEATH);
			Pos = vec2(300 * 32 + 16, Lo - 60 - Offset);
			Vel = vec2(0, V0);
		}
		else if(Dir == 1)
		{
			FillRect(250, BAND, 350, BAND + N - 1, TILE_DEATH);
			Pos = vec2(300 * 32 + 16, Hi + 60 + Offset);
			Vel = vec2(0, -V0);
		}
		else
		{
			FillRect(BAND, 250, BAND + N - 1, 350, TILE_DEATH);
			Pos = vec2(Lo - 60 - Offset, 300 * 32 + 16);
			Vel = vec2(V0, 0);
		}
		Spawn(0, Pos);
		Chr(0)->m_Core.m_Vel = Vel;
		SCross r{false, 0, 0};
		for(int i = 0; i < 40; i++)
		{
			float Before = Dir == 2 ? Chr(0)->m_Pos.x : Chr(0)->m_Pos.y;
			Step(1);
			r.m_Ticks++;
			if(!Chr(0))
			{
				return r;
			}
			float After = Dir == 2 ? Chr(0)->m_Pos.x : Chr(0)->m_Pos.y;
			bool Past = Dir == 1 ? After < Lo - 20 : After > Hi + 20;
			bool BeforeClear = Dir == 1 ? Before > Hi + 20 : Before < Lo - 20;
			if(BeforeClear && (Past || (Dir == 1 ? After <= Hi + 20 : After >= Lo - 20)))
				r.m_CrossD = (int)std::fabs(After - Before);
			if(Past)
			{
				// tick a few more to make sure
				Step(1);
				r.m_Survived = Chr(0) != nullptr;
				Kill(0);
				return r;
			}
		}
		Kill(0);
		return r;
	};

	const char *apDir[] = {"down", "up", "right"};
	for(int Dir = 0; Dir < 3; Dir++)
		for(int N = 1; N <= 3; N++)
		{
			// map: crossing displacement -> (survived, total)
			std::vector<std::pair<int, int>> vStat(400, {0, 0});
			float MinSurviveV = -1;
			for(float V = 1; V <= (Dir == 2 ? 400 : 200); V += (Dir == 2 ? 1.0f : 0.5f))
			{
				for(int Off = 0; Off < 64; Off++)
				{
					SCross r = Cross(Dir, N, V, Off);
					if(r.m_CrossD > 0 && r.m_CrossD < 400)
					{
						vStat[r.m_CrossD].second++;
						if(r.m_Survived)
							vStat[r.m_CrossD].first++;
					}
					if(r.m_Survived && MinSurviveV < 0)
						MinSurviveV = V;
				}
			}
			printf("== dir=%s thickness=%d tiles  (min launch speed with any survival: %.1f)\n", apDir[Dir], N, MinSurviveV);
			int MaxD = 0;
			for(int d = 0; d < 400; d++)
				if(vStat[d].second)
					MaxD = d;
			printf("   max per-tick displacement observed: %d px\n", MaxD);
			for(int d = 0; d < 400; d++)
				if(vStat[d].first)
					printf("   D=%3d px/tick: survived %d/%d\n", d, vStat[d].first, vStat[d].second);
		}
}

// Exact kill zone: move the tee in ONE tick from far outside to a chosen end position.
TEST_F(Sim, KillZoneProbe)
{
	UseBigMap(Col(), W_BIG, H_BIG, &GameServer()->m_World);
	for(int N = 1; N <= 3; N++)
	{
		for(auto &t : gBigTiles)
			t.m_Index = TILE_AIR;
		FillRect(250, BAND, 350, BAND + N - 1, TILE_DEATH);
		const int Lo = BAND * 32, Hi = (BAND + N) * 32;
		int First = 1 << 30, Last = -(1 << 30);
		for(int End = Lo - 40; End < Hi + 40; End++)
		{
			// start 300 px above, velocity 299.5 + gravity 0.5 = exactly 300 px this tick
			Spawn(0, vec2(300 * 32 + 16, End - 300));
			Chr(0)->m_Core.m_Vel = vec2(0, 299.5f);
			Step(1);
			int Y = Chr(0) ? (int)Chr(0)->m_Pos.y : -1;
			Step(1); // death is checked on the tick after the move
			bool Dead = !Chr(0);
			if(Y != End && Y != -1)
				printf("  unexpected end %d != %d\n", Y, End);
			if(Dead && Last != -(1 << 30) && End != Last + 1)
				printf("  non-contiguous at %d\n", End);
			if(Dead)
			{
				First = std::min(First, End);
				Last = std::max(Last, End);
			}
			Kill(0);
		}
		printf("vertical, %d tile(s) of death [%d,%d): center y kills in [%d, %d] -> zone %d px, need >= %d px in one tick\n",
			N, Lo, Hi, First, Last, Last - First + 1, Last - First + 2);
	}
}

// horizontal displacement per tick as a function of horizontal velocity (velocity ramp)
TEST_F(Sim, HorizontalRamp)
{
	UseBigMap(Col(), W_BIG, H_BIG, &GameServer()->m_World);
	float BestD = 0, BestV = 0;
	for(float V = 0; V <= 1000; V += 0.25f)
	{
		Spawn(0, vec2(100 * 32, 300 * 32));
		Chr(0)->m_Core.m_Vel = vec2(V, 0);
		float X0 = Chr(0)->m_Pos.x;
		Step(1);
		float D = Chr(0)->m_Pos.x - X0;
		if(D > BestD)
		{
			BestD = D;
			BestV = V;
		}
		if(std::fmod(V, 50.0f) == 0)
			printf("  vx=%6.1f -> %3.0f px/tick\n", V, D);
		Kill(0);
	}
	printf("max horizontal px/tick = %.0f at vx=%.2f\n", BestD, BestV);
	// with vertical velocity too: the ramp uses the full speed
	for(float Vy : {0.f, 20.f, 50.f, 100.f})
	{
		float Best = 0;
		for(float V = 0; V <= 1000; V += 0.5f)
		{
			Spawn(0, vec2(100 * 32, 300 * 32));
			Chr(0)->m_Core.m_Vel = vec2(V, Vy);
			float X0 = Chr(0)->m_Pos.x;
			Step(1);
			Best = std::max(Best, Chr(0)->m_Pos.x - X0);
			Kill(0);
		}
		printf("  with vy=%.0f: max horizontal px/tick = %.0f\n", Vy, Best);
	}
}

// Tee 0 dashes with ninja (to the right) through a 1-tile wall of death.
// Helper (optional tee 1) acts on it during the dash.
// Mode 0: alone, 1: helper hooks tee 0 from the far side, 2: helper hammers tee 0 from behind
TEST_F(Sim, NinjaThroughDeathWall)
{
	UseBigMap(Col(), W_BIG, H_BIG, &GameServer()->m_World);
	const int WallX = 300;
	const int Lo = WallX * 32;
	const char *apMode[] = {"ninja alone", "ninja + partner hook pull (partner older entity)", "ninja + partner hook pull (partner newer entity)", "ninja + partner hammer from behind (older)"};
	for(int Mode = 0; Mode < 4; Mode++)
	{
		int Survived = 0, Total = 0, MaxStep = 0;
		for(int Off = 0; Off < 64; Off++)
		{
			for(auto &t : gBigTiles)
				t.m_Index = TILE_AIR;
			FillRect(WallX, 200, WallX, 400, TILE_DEATH);
			const float Y = 300 * 32 + 16;
			bool Helper = Mode != 0;
			// entity order: spawned later => ticks earlier
			bool HelperOlder = Mode == 1 || Mode == 3;
			vec2 P0(Lo - 11 - 50 * 3 - Off, Y);
			vec2 HelperPos = Mode == 3 ? P0 + vec2(-30, 0) : vec2(Lo + 32 + 300, Y);
			if(Helper && HelperOlder)
				Spawn(1, HelperPos);
			Spawn(0, P0);
			if(Helper && !HelperOlder)
				Spawn(1, HelperPos);
			Chr(0)->GiveNinja();
			Aim(0, vec2(1, 0));
			if(Helper)
			{
				// keep the helper in place for the experiment
				Chr(1)->m_Core.m_Vel = vec2(0, 0);
				if(Mode == 1 || Mode == 2)
				{
					Aim(1, vec2(-1, 0));
					m_aTee[1].m_Input.m_Hook = 1;
				}
				else
					Aim(1, vec2(1, 0));
			}
			int NHelper = Helper ? 2 : 1;
			bool Dead = false;
			bool Passed = false;
			for(int i = 0; i < 25; i++)
			{
				if(i == 0)
					PressFire(0);
				else
					ReleaseFire(0);
				if(Mode == 3)
				{
					// hammer every time the helper can
					if(i % 2 == 0)
						PressFire(1);
					else
						ReleaseFire(1);
				}
				if(Helper && Chr(1))
				{
					Chr(1)->m_Core.m_Pos = Mode == 3 ? Chr(0)->m_Pos + vec2(-30, 0) : HelperPos; // pin helper
					Chr(1)->m_Pos = Chr(1)->m_Core.m_Pos;
					Chr(1)->m_Core.m_Vel = vec2(0, 0);
				}
				float X = Chr(0)->m_Pos.x;
				Step(NHelper);
				if(!Chr(0))
				{
					Dead = true;
					break;
				}
				MaxStep = std::max(MaxStep, (int)(Chr(0)->m_Pos.x - X));
				if(Chr(0)->m_Pos.x > Lo + 32 + 30)
				{
					Step(NHelper);
					Dead = !Chr(0);
					Passed = !Dead;
					break;
				}
			}
			Total++;
			if(Passed)
				Survived++;
			Kill(0);
			Kill(1);
		}
		printf("%-50s survived %2d/%d phases, max step %d px/tick\n", apMode[Mode], Survived, Total, MaxStep);
	}
}

// free fall from rest onto a band of N death tiles: which drop heights survive?
TEST_F(Sim, FreeFall)
{
	const int H = 1400;
	UseBigMap(Col(), 100, H, &GameServer()->m_World);
	const int Row = 1300;
	for(int N = 1; N <= 3; N++)
	{
		for(auto &t : gBigTiles)
			t.m_Index = TILE_AIR;
		FillRect(0, Row, 99, Row + N - 1, TILE_DEATH);
		const int Lo = Row * 32, Hi = (Row + N) * 32;
		int FirstSurvive = -1;
		int Buckets[400] = {0};
		int BucketsTotal[400] = {0};
		for(int Drop = 1000; Drop <= 36000; Drop += 3)
		{
			// Drop = distance from start center to the top of the band
			int StartY = Lo - Drop;
			if(StartY < 64)
				break;
			Spawn(0, vec2(50 * 32 + 16, StartY));
			bool Survived = false;
			float LastV = 0;
			for(int i = 0; i < 1000; i++)
			{
				Step(1);
				if(!Chr(0))
					break;
				LastV = Chr(0)->m_Core.m_Vel.y;
				if(Chr(0)->m_Pos.y > Hi + 20)
				{
					Step(1);
					Survived = Chr(0) != nullptr;
					break;
				}
			}
			(void)LastV;
			int b = Drop / 1024; // 32 tiles per bucket
			BucketsTotal[b]++;
			if(Survived)
			{
				Buckets[b]++;
				if(FirstSurvive < 0)
					FirstSurvive = Drop;
			}
			Kill(0);
			// the fall takes long: sample sparser once we know the start
			if(Drop > 4000 && N == 1 && Drop > 20000)
				break;
		}
		printf("== free fall onto %d tile(s) of death: lowest surviving drop = %d px (%.1f tiles)\n", N, FirstSurvive, FirstSurvive / 32.0);
		for(int b = 0; b < 400; b++)
			if(BucketsTotal[b])
				printf("   drop %4d-%4d tiles: survive %5.1f%%\n", b * 32, b * 32 + 31, 100.0 * Buckets[b] / BucketsTotal[b]);
	}
}

// fall, then ninja upward: the dash ends by restoring the old speed in the dash direction
TEST_F(Sim, NinjaRedirect)
{
	UseBigMap(Col(), 100, 1400, &GameServer()->m_World);
	Spawn(0, vec2(50 * 32 + 16, 100));
	for(int i = 0; i < 200; i++)
		Step(1);
	printf("after 200 ticks of free fall: vy=%.2f\n", Chr(0)->m_Core.m_Vel.y);
	Chr(0)->GiveNinja();
	Aim(0, vec2(0, -1));
	PressFire(0);
	for(int i = 0; i < 14; i++)
	{
		float Y = Chr(0)->m_Pos.y;
		Step(1);
		ReleaseFire(0);
		printf("  tick %2d: dy=%7.1f vel=(%.1f, %.1f)\n", i, Chr(0)->m_Pos.y - Y, Chr(0)->m_Core.m_Vel.x, Chr(0)->m_Core.m_Vel.y);
	}
}

// ---------- aled ----------
// Tee 1 (B) is frozen and flies right across a freeze wall W tiles thick.
// Tee 0 (A) starts left of the wall (optionally moving right) and presses hammer once at tick HammerTick,
// aiming at B. Success = B ends unfrozen, alive, right of the wall.
struct SAledCfg
{
	int m_W;
	float m_BX, m_BVx, m_BVy; // B start x (px left of wall), velocity
	float m_AX, m_AVx; // A start x (px left of wall), velocity x
	float m_ADy; // A start y relative to B
	bool m_AFirst; // A ticks before B (A spawned later)
	int m_HammerTick; // -1 = never
};
struct SAledRes
{
	bool m_BFree;
	bool m_AEverFrozen;
	bool m_Hit;
	vec2 m_AAt, m_BAt; // positions when the hammer fired
	int m_BCrossTick; // first tick B's center was right of the wall
	std::vector<std::pair<vec2, vec2>> m_vTraj; // (A,B) positions per tick
};

TEST_F(Sim, Aled)
{
	UseBigMap(Col(), 200, 200, &GameServer()->m_World);
	const int C = 100; // wall starts at tile column C
	const float WallL = C * 32;
	const float Y0 = 100 * 32 + 16;

	auto Run = [&](const SAledCfg &Cfg, bool Record) -> SAledRes {
		for(auto &t : gBigTiles)
			t.m_Index = TILE_AIR;
		FillRect(C, 20, C + Cfg.m_W - 1, 180, TILE_FREEZE);
		const float WallR = (C + Cfg.m_W) * 32;
		SAledRes r{false, false, false, vec2(0, 0), vec2(0, 0), -1, {}};
		if(Cfg.m_AFirst)
		{
			Spawn(1, vec2(WallL - Cfg.m_BX, Y0));
			Spawn(0, vec2(WallL - Cfg.m_AX, Y0 + Cfg.m_ADy));
		}
		else
		{
			Spawn(0, vec2(WallL - Cfg.m_AX, Y0 + Cfg.m_ADy));
			Spawn(1, vec2(WallL - Cfg.m_BX, Y0));
		}
		CCharacter *pA = Chr(0), *pB = Chr(1);
		pB->Freeze();
		pB->m_Core.m_Vel = vec2(Cfg.m_BVx, Cfg.m_BVy);
		pA->m_Core.m_Vel = vec2(Cfg.m_AVx, 0);
		for(int i = 0; i < 60; i++)
		{
			pA = Chr(0);
			pB = Chr(1);
			if(!pA || !pB)
				break;
			if(r.m_BCrossTick < 0 && pB->m_Pos.x >= WallR)
				r.m_BCrossTick = i;
			if(i == Cfg.m_HammerTick)
			{
				Aim(0, normalize(pB->m_Pos - pA->m_Pos));
				PressFire(0);
				r.m_AAt = pA->m_Pos;
				r.m_BAt = pB->m_Pos;
			}
			else
				ReleaseFire(0);
			bool BFrozenBefore = pB->m_FreezeTime > 0;
			Step(2);
			if(!Chr(0) || !Chr(1))
				break;
			if(i == Cfg.m_HammerTick && Chr(1)->m_Core.m_Vel != pB->m_Core.m_Vel)
				; // (hit detected below via reload timer)
			if(i == Cfg.m_HammerTick)
				r.m_Hit = Chr(0)->m_ReloadTimer > 5; // hit => long reload
			(void)BFrozenBefore;
			if(Chr(0)->m_FreezeTime > 0)
				r.m_AEverFrozen = true;
			if(Record)
				r.m_vTraj.emplace_back(Chr(0)->m_Pos, Chr(1)->m_Pos);
		}
		if(Chr(1) && Chr(0))
			r.m_BFree = Chr(1)->m_FreezeTime == 0 && Chr(1)->m_Pos.x >= WallR;
		Kill(0);
		Kill(1);
		return r;
	};

	for(int W = 1; W <= 2; W++)
	{
		long Sims = 0;
		int NSuccessClean = 0, NSuccessAFrozen = 0;
		float BestCleanMargin = -1;
		SAledCfg Example{}, ExampleDive{};
		SAledRes ExRes{}, ExDiveRes{};
		float MaxHitDist = 0;
		for(int AFirst = 0; AFirst < 2; AFirst++)
			for(float BVx = 4; BVx <= 30; BVx += 2)
				for(float BVy : {-6.f, -3.f, 0.f})
					for(float BX = 30; BX <= 60; BX += 6)
						for(float AX = 10; AX <= 90; AX += 4)
							for(float AVx : {0.f, 4.f, 8.f, 12.f})
								for(float ADy : {-20.f, 0.f, 20.f})
								{
									SAledCfg Cfg{W, BX, BVx, BVy, AX, AVx, ADy, (bool)AFirst, -1};
									// baseline to find candidate hammer ticks
									SAledRes Base = Run(Cfg, true);
									Sims++;
									const float WallR = (C + W) * 32;
									for(int t = 0; t < (int)Base.m_vTraj.size() && t < 40; t++)
									{
										// positions at the START of tick t+1 are traj[t]
										vec2 A = Base.m_vTraj[t].first, B = Base.m_vTraj[t].second;
										if(B.x < WallR - 40 || distance(A, B) >= 70)
											continue;
										Cfg.m_HammerTick = t + 1;
										SAledRes R = Run(Cfg, false);
										Sims++;
										if(R.m_Hit)
											MaxHitDist = std::max(MaxHitDist, distance(R.m_AAt, R.m_BAt));
										if(R.m_BFree)
										{
											if(!R.m_AEverFrozen)
											{
												if(!NSuccessClean)
												{
													Example = Cfg;
													ExRes = R;
												}
												NSuccessClean++;
											}
											else
											{
												if(!NSuccessAFrozen)
												{
													ExampleDive = Cfg;
													ExDiveRes = R;
												}
												NSuccessAFrozen++;
											}
										}
									}
								}
		(void)BestCleanMargin;
		printf("== %d-tile freeze wall: %ld sims, max hammer hit distance %.1f px\n", W, Sims, MaxHitDist);
		printf("   B freed, A never frozen: %d\n", NSuccessClean);
		printf("   B freed, A got frozen:   %d\n", NSuccessAFrozen);
		auto Show = [&](const char *pName, const SAledCfg &c, const SAledRes &r) {
			printf("   %s: A%s, B start %.0fpx left of wall v=(%.0f,%.0f); A start %.0fpx left, vx=%.0f, dy=%.0f; hammer tick %d\n",
				pName, c.m_AFirst ? " ticks first" : " ticks second", c.m_BX, c.m_BVx, c.m_BVy, c.m_AX, c.m_AVx, c.m_ADy, c.m_HammerTick);
			printf("      at hammer: A.x=%.0f (wall %.0f..%.0f) B.x=%.0f dist=%.1f\n", r.m_AAt.x, WallL, (C + W) * 32.f - 1, r.m_BAt.x, distance(r.m_AAt, r.m_BAt));
		};
		if(NSuccessClean)
			Show("example clean", Example, ExRes);
		if(NSuccessAFrozen)
			Show("example A-frozen", ExampleDive, ExDiveRes);
	}
}

// 2-tile "dive" aled: B (frozen) already just past the wall; A runs/flies into the wall and hammers
// on the first tick its center is inside the freeze (its own freeze check only runs after firing).
TEST_F(Sim, AledDive)
{
	UseBigMap(Col(), 200, 200, &GameServer()->m_World);
	const int C = 100;
	const float WallL = C * 32;
	const float Y0 = 100 * 32 + 16;
	for(int W = 1; W <= 3; W++)
	{
		const float WallR = (C + W) * 32;
		int Tried = 0, BFreed = 0, BFreedAClean = 0;
		float MaxDepth = 0, MinDepth = 1e9;
		for(int AFirst = 0; AFirst < 2; AFirst++)
			for(float D = 0; D <= 30; D += 1) // B center this far right of the wall
				for(float V = 2; V <= 40; V += 1) // A horizontal speed into the wall
					for(float K = 1; K <= 45; K += 1) // A start distance from wall
						for(int Extra = 0; Extra <= 1; Extra++) // hammer on entry tick or one later
						{
							auto Setup = [&]() {
								for(auto &t : gBigTiles)
									t.m_Index = TILE_AIR;
								FillRect(C, 20, C + W - 1, 180, TILE_FREEZE);
								if(AFirst)
								{
									Spawn(1, vec2(WallR + D, Y0));
									Spawn(0, vec2(WallL - K, Y0));
								}
								else
								{
									Spawn(0, vec2(WallL - K, Y0));
									Spawn(1, vec2(WallR + D, Y0));
								}
								Chr(1)->Freeze();
								Chr(1)->m_Core.m_Vel = vec2(0.5f, -0.5f); // drifting away slowly
								Chr(0)->m_Core.m_Vel = vec2(V, -0.5f);
							};
							Setup();
							// find the tick at whose start A's center is first inside the freeze
							int Entry = -1;
							for(int i = 0; i < 20 && Chr(0); i++)
							{
								if(Chr(0)->m_Pos.x >= WallL)
								{
									Entry = i;
									break;
								}
								Step(2);
							}
							Kill(0);
							Kill(1);
							if(Entry < 0)
								continue;
							Setup();
							int HT = Entry + Extra;
							bool AFrozenAtFire = false;
							float Depth = 0;
							bool Hit = false;
							for(int i = 0; i < 40 && Chr(0) && Chr(1); i++)
							{
								if(i == HT)
								{
									Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
									PressFire(0);
									AFrozenAtFire = Chr(0)->m_FreezeTime > 0;
									Depth = Chr(0)->m_Pos.x - WallL;
								}
								else
									ReleaseFire(0);
								Step(2);
								if(i == HT && Chr(0))
									Hit = Chr(0)->m_ReloadTimer > 5;
							}
							Tried++;
							bool Freed = Chr(1) && Chr(1)->m_FreezeTime == 0 && Chr(1)->m_Pos.x >= WallR;
							bool AFree = Chr(0) && Chr(0)->m_FreezeTime == 0;
							(void)AFrozenAtFire;
							if(Freed && Hit)
							{
								BFreed++;
								MaxDepth = std::max(MaxDepth, Depth);
								MinDepth = std::min(MinDepth, Depth);
								if(AFree)
									BFreedAClean++;
							}
							Kill(0);
							Kill(1);
						}
		printf("== %d-tile wall, hammerer dives in: %d tries, B freed in %d (hammerer center %.0f..%.0f px deep in freeze when firing), hammerer also free: %d\n",
			W, Tried, BFreed, MinDepth, MaxDepth, BFreedAClean);
	}
}

// Tolerance map for the 2-tile dive aled: success vs (hammerer depth in freeze, partner distance past wall, vertical offset)
TEST_F(Sim, AledTolerance)
{
	UseBigMap(Col(), 200, 200, &GameServer()->m_World);
	const int C = 100, W = 2;
	const float WallL = C * 32, WallR = (C + W) * 32;
	const float Y0 = 100 * 32 + 16;
	// Res[dy][D][depth]: 0 = untested, 1 = fail, 2 = success
	const int NDy = 5, ND = 25, NE = 40;
	static int Res[NDy][ND][NE];
	int aDy[NDy] = {0, 10, 20, 30, 40};
	for(int iy = 0; iy < NDy; iy++)
		for(int D = 0; D < ND; D++)
			for(int AFirst = 0; AFirst < 2; AFirst++)
				for(float V = 2; V <= 42; V += 1)
					for(int K = 1; K <= 12; K++)
					{
						auto Setup = [&]() {
							for(auto &t : gBigTiles)
								t.m_Index = TILE_AIR;
							FillRect(C, 20, C + W - 1, 180, TILE_FREEZE);
							vec2 PB(WallR + D, Y0 - aDy[iy]);
							vec2 PA(WallL - K, Y0);
							if(AFirst)
							{
								Spawn(1, PB);
								Spawn(0, PA);
							}
							else
							{
								Spawn(0, PA);
								Spawn(1, PB);
							}
							Chr(1)->Freeze();
							Chr(1)->m_Core.m_Vel = vec2(0, -0.5f);
							Chr(0)->m_Core.m_Vel = vec2(V, -0.5f);
						};
						Setup();
						Step(2); // A moves once: enters the wall (or not)
						int Depth = Chr(0) ? (int)(Chr(0)->m_Pos.x - WallL) : -1;
						Kill(0);
						Kill(1);
						if(Depth < 0 || Depth >= NE)
							continue;
						Setup();
						Step(2);
						Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
						PressFire(0);
						for(int i = 0; i < 30 && Chr(0) && Chr(1); i++)
						{
							Step(2);
							ReleaseFire(0);
						}
						bool Ok = Chr(1) && Chr(1)->m_FreezeTime == 0 && Chr(1)->m_Pos.x >= WallR;
						int &Cell = Res[iy][D][Depth];
						Cell = std::max(Cell, Ok ? 2 : 1);
						Kill(0);
						Kill(1);
					}
	for(int iy = 0; iy < NDy; iy++)
	{
		printf("== partner %d px higher. rows: partner center px past wall; cols: hammerer depth 0..%d ('#'=works '.'=fails ' '=not reached)\n", aDy[iy], NE - 1);
		for(int D = 0; D < ND; D++)
		{
			printf("  %2d |", D);
			int Min = -1, Max = -1;
			for(int E = 0; E < NE; E++)
			{
				int c = Res[iy][D][E];
				putchar(c == 2 ? '#' : c == 1 ? '.' : ' ');
				if(c == 2)
				{
					if(Min < 0)
						Min = E;
					Max = E;
				}
			}
			if(Min >= 0)
				printf("| works at depth %d..%d", Min, Max);
			printf("\n");
		}
	}
}

// ---------- real DDNet demos ----------
// Generates maps/simphysics.map (game layer only), runs the scenes on it and records server demos.
// Demos are copied to $SIM_DEMO_DIR. View them with cl_overlay_entities 100 to see the game tiles.
namespace SimMap {
const int W = 260, H = 390;
// S1 free fall
const int FALL_ROW = 370, FALL_COL_A = 19, FALL_COL_B = 23;
// S2 horizontal
const int HOR_WALL = 120, HOR_LANE[3] = {40, 60, 80}, HOR_HELPER_DIST = 150;
// S3 ninja redirect
const int RED_BAND = 290, RED_COL = 220;
// S4 1-tile aled
const int A1_WALL = 75, A1_ROW = 170;
// S5 2-tile aled
const int A2_WALL = 120, A2_ROW = 170;
// vertical: 2-deep freeze pool in a floor
const int POOL_L = 80, POOL_W = 4, POOL_GROUND = 300;

static std::vector<std::array<int, 5>> Rects()
{
	std::vector<std::array<int, 5>> v;
	v.push_back({2, FALL_ROW, 40, FALL_ROW, TILE_DEATH});
	v.push_back({HOR_WALL, 30, HOR_WALL, 90, TILE_DEATH});
	int HelperCol = (HOR_WALL * 32 + 32 + HOR_HELPER_DIST) / 32;
	v.push_back({HelperCol - 2, HOR_LANE[2] + 1, HelperCol + 2, HOR_LANE[2] + 1, TILE_SOLID});
	v.push_back({205, RED_BAND, 235, RED_BAND, TILE_DEATH});
	v.push_back({A1_WALL, 120, A1_WALL, 215, TILE_FREEZE});
	v.push_back({A2_WALL, 150, A2_WALL + 1, A2_ROW, TILE_FREEZE});
	v.push_back({105, A2_ROW + 1, 140, A2_ROW + 1, TILE_SOLID});
	v.push_back({55, POOL_GROUND, 115, POOL_GROUND, TILE_SOLID});
	v.push_back({POOL_L, POOL_GROUND - 2, POOL_L + POOL_W - 1, POOL_GROUND - 1, TILE_FREEZE});
	return v;
}

static std::vector<std::array<int, 5>> gvExtraRects; // added by tests that design parts of the map at runtime

static void WriteTo(IStorage *pStorage, const char *pPath)
{
	std::vector<CTile> vTiles((size_t)W * H, CTile{});
	std::vector<std::array<int, 5>> vRects = Rects();
	vRects.insert(vRects.end(), gvExtraRects.begin(), gvExtraRects.end());
	for(auto &r : vRects)
		for(int y = r[1]; y <= r[3]; y++)
			for(int x = r[0]; x <= r[2]; x++)
				vTiles[y * W + x].m_Index = r[4];
	pStorage->CreateFolder("maps", IStorage::TYPE_SAVE);
	CDataFileWriter Writer;
	ASSERT_TRUE(Writer.Open(pStorage, pPath));
	CMapItemVersion Version;
	Version.m_Version = 1;
	Writer.AddItem(MAPITEMTYPE_VERSION, 0, sizeof(Version), &Version);
	CMapItemGroup_v1 Group;
	Group.m_Version = 1;
	Group.m_OffsetX = 0;
	Group.m_OffsetY = 0;
	Group.m_ParallaxX = 100;
	Group.m_ParallaxY = 100;
	Group.m_StartLayer = 0;
	Group.m_NumLayers = 1;
	Writer.AddItem(MAPITEMTYPE_GROUP, 0, sizeof(Group), &Group);
	CMapItemLayerTilemap_v2 GameLayer;
	GameLayer.m_Layer.m_Version = 0;
	GameLayer.m_Layer.m_Type = LAYERTYPE_TILES;
	GameLayer.m_Layer.m_Flags = 0;
	GameLayer.m_Version = 2;
	GameLayer.m_Width = W;
	GameLayer.m_Height = H;
	GameLayer.m_Flags = TILESLAYERFLAG_GAME;
	GameLayer.m_Color = {255, 255, 255, 255};
	GameLayer.m_ColorEnv = -1;
	GameLayer.m_ColorEnvOffset = 0;
	GameLayer.m_Image = -1;
	GameLayer.m_Data = Writer.AddData(vTiles.size() * sizeof(CTile), vTiles.data());
	Writer.AddItem(MAPITEMTYPE_LAYER, 0, sizeof(GameLayer), &GameLayer);
	Writer.Finish();
}

static void Write(IStorage *pStorage) { WriteTo(pStorage, "maps/simphysics.map"); }
} // namespace SimMap

class SimDemo : public Sim
{
public:
	static void SetUpTestSuite()
	{
		CNetBase::Init(); // huffman table used to compress demo chunks (normally set up with the network)
		gpSimMapName = "simphysics";
		gpfnPrepareMap = SimMap::Write;
	}
	static void TearDownTestSuite()
	{
		gpSimMapName = "coverage";
		gpfnPrepareMap = nullptr;
	}
	SimDemo()
	{
		g_Config.m_SvMaxAfkTime = 0; // no "zz" bubbles on idle tees
	}
	void Name(int t, const char *pName) { m_pServer->SetClientName(CidOf(t), pName); }
	// big centered text in the demo (recorded only, not sent)
	void Caption(const char *pText)
	{
		CNetMsg_Sv_Broadcast Msg;
		Msg.m_pMessage = pText;
		m_pServer->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NOSEND, -1);
	}
	void StartRec(const char *pName)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "demos/%s.demo", pName);
		m_pStorage->CreateFolder("demos", IStorage::TYPE_SAVE);
		CServer *s = m_pServer;
		s->m_aDemoRecorder[CServer::RECORDER_MANUAL].Start(s->Storage(), s->Console(), aBuf, GameServer()->NetVersion(),
			GameServer()->Map()->BaseName(), s->m_aCurrentMapSha256[CServer::MAP_TYPE_SIX], s->m_aCurrentMapCrc[CServer::MAP_TYPE_SIX], "server",
			s->m_aCurrentMapSize[CServer::MAP_TYPE_SIX], s->m_apCurrentMapData[CServer::MAP_TYPE_SIX], nullptr, nullptr, nullptr);
		ASSERT_TRUE(m_pServer->m_aDemoRecorder[CServer::RECORDER_MANUAL].IsRecording());
		m_Recording = true;
		m_aRecName = pName;
		Hold(1); // the client ignores messages that arrive before the first snapshot
	}
	// advance the clock and record the scene without simulating (a pause in the video)
	void Hold(int Ticks)
	{
		for(int i = 0; i < Ticks; i++)
		{
			m_pServer->m_CurrentGameTick++;
			SnapDemo();
			GameServer()->OnPostGlobalSnap();
		}
	}
	void StopRec()
	{
		m_pServer->m_aDemoRecorder[CServer::RECORDER_MANUAL].Stop(IDemoRecorder::EStopMode::KEEP_FILE);
		m_Recording = false;
		char aPath[256];
		str_format(aPath, sizeof(aPath), "demos/%s.demo", m_aRecName.c_str());
		void *pData;
		unsigned Size;
		ASSERT_TRUE(m_pStorage->ReadFile(aPath, IStorage::TYPE_SAVE, &pData, &Size));
		const char *pDir = getenv("SIM_DEMO_DIR");
		std::string Out = std::string(pDir ? pDir : ".") + "/" + m_aRecName + ".demo";
		FILE *f = fopen(Out.c_str(), "wb");
		ASSERT_TRUE(f);
		fwrite(pData, 1, Size, f);
		fclose(f);
		free(pData);
		printf("wrote %s (%u bytes, cids: tee0=%d tee1=%d)\n", Out.c_str(), Size, CidOf(0), CidOf(1));
		if(getenv("SIM_DBG"))
		{
			// play the demo back with the engine's demo player
			struct CListener : CDemoPlayer::IListener
			{
				int m_Snaps = 0;
				void OnDemoPlayerSnapshot(void *pData, int Size) override { m_Snaps++; }
				void OnDemoPlayerMessage(void *pData, int Size) override {}
			} Listener;
			CDemoPlayer Player(&m_pServer->m_SnapshotDelta, &m_pServer->m_SnapshotDeltaSixup, false);
			Player.SetListener(&Listener);
			int Res = Player.Load(m_pStorage.get(), m_pServer->Console(), aPath, IStorage::TYPE_SAVE);
			printf("dbg load=%d err='%s' first=%d last=%d\n", Res, Player.ErrorMessage(), Player.Info()->m_Info.m_FirstTick, Player.Info()->m_Info.m_LastTick);
			Player.Play();
			Player.SetSpeed(64);
			for(int i = 0; i < 400 && Player.IsPlaying(); i++)
			{
				Player.Update();
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
			}
			printf("dbg snaps=%d err='%s'\n", Listener.m_Snaps, Player.ErrorMessage());
		}
	}
	std::string m_aRecName;
	// swap the map the demo recorder embeds for one that also contains SimMap::gvExtraRects
	void UseExtendedMapForDemo()
	{
		const char *pPath = "maps/simphysics_ext.map";
		SimMap::WriteTo(m_pStorage.get(), pPath);
		void *pData;
		unsigned Size;
		ASSERT_TRUE(m_pStorage->ReadFile(pPath, IStorage::TYPE_SAVE, &pData, &Size));
		CServer *s = m_pServer;
		s->m_apCurrentMapData[CServer::MAP_TYPE_SIX] = (unsigned char *)pData;
		s->m_aCurrentMapSize[CServer::MAP_TYPE_SIX] = Size;
		s->m_aCurrentMapSha256[CServer::MAP_TYPE_SIX] = sha256(pData, Size);
		s->m_aCurrentMapCrc[CServer::MAP_TYPE_SIX] = crc32(0, (const unsigned char *)pData, Size);
		for(auto &r : SimMap::gvExtraRects)
			FillRect(r[0], r[1], r[2], r[3], r[4]);
	}
};

TEST_F(SimDemo, KilltileFall)
{
	using namespace SimMap;
	const float Lo = FALL_ROW * 32;
	auto Fall = [&](int Drop) {
		Spawn(0, vec2(FALL_COL_A * 32 + 16, Lo - Drop));
		for(int i = 0; i < 400; i++)
		{
			Step(1);
			if(!Chr(0))
				return false;
			if(Chr(0)->m_Pos.y > Lo + 32 + 20)
			{
				Step(1);
				bool Ok = Chr(0) != nullptr;
				Kill(0);
				return Ok;
			}
		}
		Kill(0);
		return false;
	};
	int S = -1, D = -1;
	for(int Drop = 3000; Drop < 3900 && D < 0; Drop++)
	{
		bool Ok = Fall(Drop);
		if(Ok && S < 0)
			S = Drop;
		else if(!Ok && S > 0)
			D = Drop;
	}
	ASSERT_GT(S, 0);
	ASSERT_GT(D, 0);
	printf("fall: survive drop %d, die drop %d\n", S, D);
	Spawn(0, vec2(FALL_COL_A * 32 + 16, Lo - D));
	Spawn(1, vec2(FALL_COL_B * 32 + 16, Lo - S));
	Name(0, "dies");
	Name(1, "skips");
	StartRec("sim1_killtile_fall");
	Caption("Both tees fall ~94 tiles onto ONE row of death tiles.\nDeath is only checked where you are at the END of each tick.");
	Hold(50);
	bool Said = false;
	for(int i = 0; i < 160; i++)
	{
		Step(2);
		if(!Said && Chr(1) && Chr(1)->m_Pos.y > Lo - 600)
		{
			Caption("Now moving ~50 px per tick (needs 51+ to jump the 50 px kill zone)");
			Said = true;
		}
		if(!Chr(1) || Chr(1)->m_Pos.y > Lo + 32 + 400)
			break;
	}
	Caption("'skips' jumped over the kill zone between two ticks.\n'dies' landed a tick inside it.");
	Hold(40);
	StopRec();
}

TEST_F(SimDemo, Horizontal)
{
	using namespace SimMap;
	const float Lo = HOR_WALL * 32;
	auto LaneY = [&](int l) { return HOR_LANE[l] * 32 + 16.f; };
	// ninja run in lane l; Helper: partner standing on the platform hooks the ninja
	auto RunNinja = [&](int l, int Off, int Dash, bool Helper, bool Rec) {
		float Y = LaneY(l);
		if(Helper)
		{
			Spawn(1, vec2(Lo + 32 + HOR_HELPER_DIST, Y));
			Aim(1, vec2(-1, 0));
		}
		Spawn(0, vec2(Lo - 11 - 150 - Off, Y));
		Chr(0)->GiveNinja();
		Chr(0)->m_Core.m_Vel = vec2(0, -0.5f);
		Aim(0, vec2(1, 0));
		int N = Helper ? 2 : 1;
		if(Rec)
		{
			Name(0, "ninja");
			Name(1, "partner");
			Caption(Helper ? "Ninja dash + partner hooking you forward: 54 px/tick -> skips the wall" : "Ninja dash: exactly 50 px/tick (no velocity ramp) -> still 1 px short");
			Hold(40);
		}
		bool Passed = false;
		for(int i = 0; i < 30; i++)
		{
			if(Helper)
				m_aTee[1].m_Input.m_Hook = 1;
			if(i == Dash)
				PressFire(0);
			else
				ReleaseFire(0);
			Step(N);
			if(!Chr(0))
				break;
			if(Chr(0)->m_Pos.x > Lo + 32 + 30 && i > Dash + 10)
			{
				Passed = true;
				break;
			}
		}
		if(Rec)
			Hold(40);
		Passed = Passed && Chr(0);
		Kill(0);
		Kill(1);
		m_aTee[1].m_Input.m_Hook = 0;
		return Passed;
	};
	int FOff = -1, FDash = -1;
	for(int Dash = 2; Dash <= 8 && FOff < 0; Dash++)
		for(int Off = 0; Off < 64 && FOff < 0; Off++)
			if(RunNinja(2, Off, Dash, true, false))
			{
				FOff = Off;
				FDash = Dash;
			}
	ASSERT_GE(FOff, 0);
	printf("horizontal: ninja+hook passes with off=%d dash=%d\n", FOff, FDash);

	// lane 0: plain movement at the best speed
	Spawn(0, vec2(Lo - 300, LaneY(0)));
	Name(0, "tee");
	StartRec("sim2_horizontal");
	Caption("Normal movement at the best possible speed: 48 px/tick.\nFaster = the velocity ramp shrinks horizontal movement. Needs 51.");
	Hold(40);
	Chr(0)->m_Core.m_Vel = vec2(108.5f, -2.0f);
	m_aTee[0].m_Input.m_Direction = 1;
	for(int i = 0; i < 16 && Chr(0); i++)
		Step(1);
	Hold(40);
	Kill(0);
	RunNinja(1, FOff, FDash, false, true);
	bool Ok = RunNinja(2, FOff, FDash, true, true);
	EXPECT_TRUE(Ok);
	StopRec();
}

TEST_F(SimDemo, NinjaRedirect)
{
	using namespace SimMap;
	const float X = RED_COL * 32 + 16;
	auto Run = [&](float Y0, bool Rec) {
		Spawn(0, vec2(X, Y0));
		Aim(0, vec2(0, -1));
		if(Rec)
		{
			Name(0, "tee");
			Caption("Tee is falling at 100 px/tick (like after a 200 tile drop).\nIt dashes ninja UP. When the dash ends, ninja gives the old speed back - upward.");
			Hold(40);
		}
		Chr(0)->m_Core.m_Vel = vec2(0, 99.5f); // falling at 100 px/tick
		bool Passed = false;
		for(int i = 0; i < 40; i++)
		{
			if(i == 1)
			{
				Chr(0)->GiveNinja();
				PressFire(0);
			}
			else
				ReleaseFire(0);
			Step(1);
			if(!Chr(0))
				break;
			if(Chr(0)->m_Pos.y < RED_BAND * 32 - 200)
			{
				Passed = true;
				break;
			}
		}
		if(Rec)
		{
			Caption("Launched up at 100 px/tick: skipped the death row");
			Hold(40);
		}
		Passed = Passed && Chr(0);
		Kill(0);
		return Passed;
	};
	float Found = -1;
	for(float Y0 = 9760; Y0 < 9900 && Found < 0; Y0 += 1)
		if(Run(Y0, false))
			Found = Y0;
	ASSERT_GE(Found, 0);
	StartRec("sim3_ninja_redirect");
	EXPECT_TRUE(Run(Found, true));
	StopRec();
}

TEST_F(SimDemo, Aled1Tile)
{
	using namespace SimMap;
	const float WallL = A1_WALL * 32, Y0 = A1_ROW * 32 + 16;
	auto Run = [&](int HammerTick, bool Rec) {
		Spawn(0, vec2(WallL - 70, Y0 - 20)); // A: older entity, ticks second
		Spawn(1, vec2(WallL - 36, Y0));
		Chr(1)->Freeze();
		if(Rec)
		{
			Name(0, "A");
			Name(1, "B");
			Caption("1-tile aled: frozen B drifts across the freeze.\nA stays outside and hammers B once B is fully across (reach < 63 px).");
			Hold(40);
		}
		Chr(1)->m_Core.m_Vel = vec2(4, -3);
		Chr(0)->m_Core.m_Vel = vec2(8, 0);
		for(int i = 0; i < 70 && Chr(0) && Chr(1); i++)
		{
			if(i == HammerTick)
			{
				Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
				PressFire(0);
			}
			else
				ReleaseFire(0);
			Step(2);
		}
		bool Ok = Chr(1) && Chr(1)->m_FreezeTime == 0 && Chr(0) && Chr(0)->m_FreezeTime == 0;
		if(Rec)
			Hold(25);
		Kill(0);
		Kill(1);
		return Ok;
	};
	int Tick = -1;
	for(int t = 30; t < 45 && Tick < 0; t++)
		if(Run(t, false))
			Tick = t;
	ASSERT_GE(Tick, 0);
	StartRec("sim4_aled_1tile");
	EXPECT_TRUE(Run(Tick, true));
	StopRec();
}

TEST_F(SimDemo, Aled2Tile)
{
	using namespace SimMap;
	const float WallL = A2_WALL * 32, WallR = (A2_WALL + 2) * 32, Y0 = A2_ROW * 32 + 16;
	// returns B freed; *pDepth = hammerer depth in the freeze when firing
	auto Run = [&](float K, float V, int *pDepth, int Rec) {
		Spawn(0, vec2(WallL - K, Y0));
		Spawn(1, vec2(WallR, Y0));
		Chr(1)->Freeze();
		if(Rec)
		{
			Name(0, "A");
			Name(1, "B");
			Caption(Rec == 1 ? "2-tile aled: A runs INTO the freeze and hammers on that same tick.\nThe hammer fires before A's own freeze check." : "Same, but A is only 1 px deep when hammering: B is out of reach");
			Hold(40);
		}
		m_aTee[0].m_Input.m_Direction = 1;
		Chr(0)->m_Core.m_Vel = vec2(V, 0);
		bool Fired = false;
		*pDepth = -999;
		for(int i = 0; i < 60 && Chr(0) && Chr(1); i++)
		{
			if(!Fired && Chr(0)->m_Pos.x >= WallL)
			{
				*pDepth = (int)(Chr(0)->m_Pos.x - WallL);
				Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
				PressFire(0);
				Fired = true;
				m_aTee[0].m_Input.m_Direction = 0;
			}
			else
				ReleaseFire(0);
			Step(2);
		}
		bool Ok = Chr(1) && Chr(1)->m_FreezeTime == 0;
		if(Rec)
		{
			char aBuf[160];
			if(Ok)
				str_format(aBuf, sizeof(aBuf), "A was %d px deep: B is free. A is frozen (it is in the freeze).", *pDepth);
			else
				str_format(aBuf, sizeof(aBuf), "A was %d px deep: hammer missed, B stays frozen.", *pDepth);
			Caption(aBuf);
			Hold(40);
		}
		Kill(0);
		Kill(1);
		return Ok;
	};
	float SK = -1, SV = 0, FK = -1, FV = 0;
	for(float V = 6; V <= 10 && (SK < 0 || FK < 0); V += 0.5f)
		for(float K = 120; K >= 1; K -= 1) // prefer a long run-up
		{
			int Depth;
			bool Ok = Run(K, V, &Depth, 0);
			if(Ok && Depth >= 5 && Depth <= 7 && SK < 0)
			{
				SK = K;
				SV = V;
			}
			if(!Ok && Depth == 1 && FK < 0)
			{
				FK = K;
				FV = V;
			}
		}
	ASSERT_GE(SK, 0);
	ASSERT_GE(FK, 0);
	int D1, D2;
	StartRec("sim5_aled_2tile");
	bool Ok1 = Run(SK, SV, &D1, 1);
	bool Ok2 = Run(FK, FV, &D2, 2);
	StopRec();
	printf("2-tile: success depth %d (freed=%d), fail depth %d (freed=%d)\n", D1, Ok1, D2, Ok2);
	EXPECT_TRUE(Ok1);
	EXPECT_FALSE(Ok2);
}

// Dynamic 2-tile aled: B sits frozen INSIDE the 2-tile freeze holding keys. A hammers B once from outside
// (B gets one tick of input and launches through the freeze, refrozen), then A backs off, runs/jumps in and
// does the second hammer on the first tick its own center is inside the freeze.
struct SDynCfg
{
	int m_BDepth; // B center px right of the wall's left edge (inside the freeze: 0..63)
	int m_AGap; // A center px left of the wall at the first hammer
	int m_BDir, m_BJump; // keys B holds
	int m_Back; // ticks A holds left after the first hammer (then right)
	int m_Jump; // tick (after the first hammer) at which A presses jump, -1 = never
};
struct SDynRes
{
	bool m_Ok = false;
	bool m_Hit1 = false, m_Hit2 = false;
	int m_Depth2 = -999, m_BPast = 0, m_Dy = 0, m_Tick2 = -1;
};

TEST_F(SimDemo, Aled2TileDynamic)
{
	using namespace SimMap;
	const float WallL = A2_WALL * 32, WallR = (A2_WALL + 2) * 32, Y0 = A2_ROW * 32 + 16;
	auto Run = [&](const SDynCfg &c, bool Rec) {
		SDynRes r;
		Spawn(0, vec2(WallL - c.m_AGap, Y0)); // A (older: ticks second)
		Spawn(1, vec2(WallL + c.m_BDepth, Y0)); // B
		Chr(1)->Freeze();
		m_aTee[1].m_Input.m_Direction = c.m_BDir;
		m_aTee[1].m_Input.m_Jump = c.m_BJump;
		if(Rec)
		{
			Name(0, "A");
			Name(1, "B");
			char aBuf[200];
			str_format(aBuf, sizeof(aBuf), "B is frozen INSIDE the 2-tile freeze, holding %s%s%s.\nA hammers B: B gets 1 tick of input and launches through the freeze.",
				c.m_BDir < 0 ? "left" : c.m_BDir > 0 ? "right" : "", c.m_BDir && c.m_BJump ? " + " : "", c.m_BJump ? "jump" : (c.m_BDir ? "" : "nothing"));
			Caption(aBuf);
			Hold(60);
		}
		for(int i = 0; i < 3; i++)
			Step(2); // settle on the floor
		Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
		PressFire(0);
		Step(2);
		r.m_Hit1 = Chr(0) && Chr(0)->m_ReloadTimer > 5;
		if(!r.m_Hit1)
		{
			Kill(0);
			Kill(1);
			return r;
		}
		bool Fired2 = false;
		for(int i = 1; i < 90 && Chr(0) && Chr(1); i++)
		{
			CCharacter *pA = Chr(0), *pB = Chr(1);
			if(!Fired2)
			{
				m_aTee[0].m_Input.m_Direction = i <= c.m_Back ? -1 : 1;
				m_aTee[0].m_Input.m_Jump = c.m_Jump >= 0 && i >= c.m_Jump;
			}
			if(!Fired2 && pA->m_Pos.x >= WallL)
			{
				Fired2 = true;
				r.m_Tick2 = i;
				r.m_Depth2 = (int)(pA->m_Pos.x - WallL);
				r.m_BPast = (int)(pB->m_Pos.x - WallR);
				r.m_Dy = (int)(pB->m_Pos.y - pA->m_Pos.y);
				Aim(0, normalize(pB->m_Pos - pA->m_Pos));
				PressFire(0);
				m_aTee[0].m_Input.m_Direction = 0;
				m_aTee[0].m_Input.m_Jump = 0;
				Step(2);
				r.m_Hit2 = Chr(0) && Chr(0)->m_ReloadTimer > 5;
				if(Rec)
				{
					char aBuf[200];
					str_format(aBuf, sizeof(aBuf), "2nd hammer: A's center is %d px inside the freeze, B is %d px past it.\nA fires before its own freeze check.", r.m_Depth2, r.m_BPast);
					Caption(aBuf);
				}
				continue;
			}
			ReleaseFire(0);
			Step(2);
		}
		r.m_Ok = Fired2 && r.m_Hit2 && Chr(1) && Chr(1)->m_FreezeTime == 0 && Chr(1)->m_Pos.x >= WallR;
		if(Rec)
		{
			Caption(r.m_Ok ? "B is free on the other side. A is frozen in the freeze." : "failed");
			Hold(60);
		}
		Kill(0);
		Kill(1);
		m_aTee[0].m_Input = {};
		m_aTee[1].m_Input = {};
		return r;
	};

	std::vector<std::pair<SDynCfg, SDynRes>> vOk;
	long Sims = 0;
	for(int BDepth = 4; BDepth <= 60; BDepth += 4)
		for(int AGap = 2; AGap + BDepth < 63; AGap += 4)
			for(int BDir = -1; BDir <= 1; BDir++)
				for(int BJump = 0; BJump <= 1; BJump++)
					for(int Back = 0; Back <= 14; Back += 2)
						for(int Jump = -1; Jump <= 36; Jump += (Jump < 0 ? 1 : 3))
						{
							SDynCfg c{BDepth, AGap, BDir, BJump, Back, Jump};
							SDynRes r = Run(c, false);
							Sims++;
							if(r.m_Ok)
								vOk.emplace_back(c, r);
						}
	printf("dynamic 2-tile aled: %ld sims, %zu successes\n", Sims, vOk.size());
	int aCount[2][3][2] = {};
	for(auto &[c, r] : vOk)
		aCount[c.m_Jump >= 0][c.m_BDir + 1][c.m_BJump]++;
	for(int j = 0; j < 2; j++)
		for(int d = 0; d < 3; d++)
			for(int b = 0; b < 2; b++)
				if(aCount[j][d][b])
					printf("  A %s, B holds dir=%d jump=%d: %d\n", j ? "jumps in" : "runs in", d - 1, b, aCount[j][d][b]);
	ASSERT_FALSE(vOk.empty());
	// prefer: A jumps in, B starts deep in the freeze, big vertical offset between them is fine
	auto Score = [](const std::pair<SDynCfg, SDynRes> &p) { return (p.first.m_Jump >= 0) * 1000 + p.first.m_BDepth * 10 - p.first.m_Back; };
	std::sort(vOk.begin(), vOk.end(), [&](auto &a, auto &b) { return Score(a) > Score(b); });
	for(size_t i = 0; i < std::min<size_t>(8, vOk.size()); i++)
	{
		auto &[c, r] = vOk[i];
		printf("  B depth %d, A gap %d, B keys dir=%d jump=%d, A back %d jump@%d -> 2nd hammer tick %d: A %d px deep, B %d px past, dy %d\n",
			c.m_BDepth, c.m_AGap, c.m_BDir, c.m_BJump, c.m_Back, c.m_Jump, r.m_Tick2, r.m_Depth2, r.m_BPast, r.m_Dy);
	}
	StartRec("sim6_aled_2tile_dynamic");
	SDynRes r = Run(vOk[0].first, true);
	EXPECT_TRUE(r.m_Ok);
	// second segment: the best setup where B holds jump
	for(auto &[c, Res] : vOk)
		if(c.m_BJump && c.m_Jump >= 0)
		{
			printf("  jump variant: B depth %d, A gap %d, A back %d jump@%d -> A %d px deep, B %d px past, dy %d\n",
				c.m_BDepth, c.m_AGap, c.m_Back, c.m_Jump, Res.m_Depth2, Res.m_BPast, Res.m_Dy);
			EXPECT_TRUE(Run(c, true).m_Ok);
			break;
		}
	StopRec();
}

// exact minimum free-fall drop (from rest) that can skip N rows of death tiles
TEST_F(Sim, MinFallHeight)
{
	UseBigMap(Col(), 100, 1400, &GameServer()->m_World);
	const int Row = 1300;
	const float Lo = Row * 32;
	for(int N = 1; N <= 3; N++)
	{
		for(auto &t : gBigTiles)
			t.m_Index = TILE_AIR;
		FillRect(0, Row, 99, Row + N - 1, TILE_DEATH);
		const float Hi = (Row + N) * 32;
		std::vector<int> vSurvive;
		for(int Drop = 32 * N * 70; Drop <= 32 * (80 + 150 * (N - 1)) + 32 * 40 && vSurvive.size() < 12; Drop++)
		{
			Spawn(0, vec2(50 * 32 + 16, Lo - Drop));
			bool Ok = false;
			for(int i = 0; i < 1000; i++)
			{
				Step(1);
				if(!Chr(0))
					break;
				if(Chr(0)->m_Pos.y > Hi + 20)
				{
					Step(1);
					Ok = Chr(0) != nullptr;
					break;
				}
			}
			Kill(0);

			if(Ok)
				vSurvive.push_back(Drop);
		}
		printf("%d row(s): first surviving drops (px from tee center to top of the death row):", N);
		for(int d : vSurvive)
			printf(" %d", d);
		if(!vSurvive.empty())
			printf("  -> minimum %.2f blocks\n", vSurvive[0] / 32.0);
		else
			printf(" none\n");
	}
}

// ---------- consistency research: dynamic 2-tile aleds ----------
// 2 tees. Same scene as Aled2TileDynamic, but the dive hammer can be pressed PressOffset ticks
// after the tick where A's center is first inside the freeze (0 = the only tick that can work).
struct SDyn2Res
{
	bool m_Ok = false;
	int m_Entry = -1;
};

class SimAled : public SimDemo
{
public:
	float WallL() const { return SimMap::A2_WALL * 32; }
	float WallR() const { return (SimMap::A2_WALL + 2) * 32; }
	float Y0() const { return SimMap::A2_ROW * 32 + 16; }

	// Entry: if >= 0, the tick at which A stops its inputs and (with PressOffset) fires; -1 = detect
	SDyn2Res RunDyn2(const SDynCfg &c, int PressOffset, int Entry = -1)
	{
		SDyn2Res r;
		Spawn(0, vec2(WallL() - c.m_AGap, Y0()));
		Spawn(1, vec2(WallL() + c.m_BDepth, Y0()));
		Chr(1)->Freeze();
		m_aTee[1].m_Input.m_Direction = c.m_BDir;
		m_aTee[1].m_Input.m_Jump = c.m_BJump;
		for(int i = 0; i < 3; i++)
			Step(2);
		Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
		PressFire(0);
		Step(2);
		if(!Chr(0) || Chr(0)->m_ReloadTimer <= 5)
			return Finish(r);
		int Stop = Entry;
		for(int i = 1; i < 90 && Chr(0) && Chr(1); i++)
		{
			if(Stop < 0 && Chr(0)->m_Pos.x >= WallL())
				Stop = i;
			bool Moving = Stop < 0 || i < Stop;
			m_aTee[0].m_Input.m_Direction = Moving ? (i <= c.m_Back ? -1 : 1) : 0;
			m_aTee[0].m_Input.m_Jump = Moving && c.m_Jump >= 0 && i >= c.m_Jump;
			if(Stop >= 0 && i == Stop + PressOffset)
			{
				Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
				PressFire(0);
			}
			else
				ReleaseFire(0);
			Step(2);
		}
		r.m_Entry = Stop;
		r.m_Ok = Chr(1) && Chr(1)->m_FreezeTime == 0 && Chr(1)->m_Pos.x >= WallR();
		return Finish(r);
	}
	template<class T>
	T Finish(T r)
	{
		Kill(0);
		Kill(1);
		Kill(2);
		for(auto &t : m_aTee)
			t.m_Input = {};
		return r;
	}
};

TEST_F(SimAled, Dynamic2TeeConsistency)
{
	// 1) find all working setups (dive hammer on the exact tick)
	std::vector<SDynCfg> vOk;
	for(int BDepth = 4; BDepth <= 60; BDepth += 4)
		for(int AGap = 2; AGap + BDepth < 63; AGap += 4)
			for(int BDir = -1; BDir <= 1; BDir++)
				for(int BJump = 0; BJump <= 1; BJump++)
					for(int Back = 0; Back <= 14; Back += 2)
						for(int Jump = -1; Jump <= 36; Jump += (Jump < 0 ? 1 : 3))
						{
							SDynCfg c{BDepth, AGap, BDir, BJump, Back, Jump};
							if(RunDyn2(c, 0).m_Ok)
								vOk.push_back(c);
						}
	printf("working setups: %zu\n", vOk.size());
	ASSERT_FALSE(vOk.empty());

	// 2) the dive press window: press 2 ticks early .. 2 ticks late
	int aPress[5] = {};
	for(auto &c : vOk)
	{
		int Entry = RunDyn2(c, 0).m_Entry;
		for(int k = -2; k <= 2; k++)
			aPress[k + 2] += RunDyn2(c, k, Entry).m_Ok;
	}
	printf("dive press offset -2..+2 ticks: %d %d %d %d %d (of %zu)\n", aPress[0], aPress[1], aPress[2], aPress[3], aPress[4], vOk.size());

	// 3) robustness of everything else (dive press always on the exact tick)
	struct SScored
	{
		SDynCfg m_Cfg;
		int m_Ok, m_Total;
	};
	std::vector<SScored> vScored;
	for(auto &c : vOk)
	{
		int Ok = 0, Total = 0;
		for(int dB : {-4, -2, 0, 2, 4})
			for(int dA : {-4, -2, 0, 2, 4})
				for(int dBack : {-1, 0, 1})
					for(int dJ : {-2, -1, 0, 1, 2})
					{
						if(c.m_Jump < 0 && dJ)
							continue;
						SDynCfg n = c;
						n.m_BDepth += dB;
						n.m_AGap += dA;
						n.m_Back = std::max(0, n.m_Back + dBack);
						if(n.m_Jump >= 0)
							n.m_Jump = std::max(0, n.m_Jump + dJ);
						if(n.m_BDepth < 0 || n.m_BDepth > 63 || n.m_AGap < 1)
							continue;
						Total++;
						Ok += RunDyn2(n, 0).m_Ok;
					}
		vScored.push_back({c, Ok, Total});
	}
	std::sort(vScored.begin(), vScored.end(), [](auto &a, auto &b) { return a.m_Ok * b.m_Total > b.m_Ok * a.m_Total; });
	printf("most forgiving 2-tee setups (fraction of +-4px / +-1 tick back-off / +-2 tick jump neighbours that still work):\n");
	for(size_t i = 0; i < std::min<size_t>(10, vScored.size()); i++)
	{
		auto &s = vScored[i];
		printf("  %5.1f%%  B %d px deep keys dir=%d jump=%d | A %d px from wall, back %d ticks, jump@%d\n", 100.0 * s.m_Ok / s.m_Total,
			s.m_Cfg.m_BDepth, s.m_Cfg.m_BDir, s.m_Cfg.m_BJump, s.m_Cfg.m_AGap, s.m_Cfg.m_Back, s.m_Cfg.m_Jump);
	}
}

// 3 tees, "relay": A hammers B (inside the freeze) once, then walks into the freeze itself and gets frozen,
// HOLDING FIRE aimed at B. C (outside, left) hammers A at tick TC. Being unfrozen gives A full-auto for that
// tick (CCharacter::m_FrozenLastTick), so A's held fire swings from inside the freeze at B.
struct SRelayCfg
{
	int m_BDepth, m_AGap, m_BDir, m_BJump;
	int m_Walk; // ticks A holds right after the first hammer
	int m_CGap; // C center px left of the wall where C stops (C starts further left and walks there)
	bool m_AimRight; // A just aims straight right instead of tracking B
};

TEST_F(SimAled, RelayHorizontal)
{
	auto Run = [&](const SRelayCfg &c, int TC, bool Rec, int *pDepth = nullptr, int *pPast = nullptr) {
		Spawn(2, vec2(WallL() - c.m_AGap - 40, Y0())); // C, walks to WallL - CGap after the first hammer
		Spawn(0, vec2(WallL() - c.m_AGap, Y0())); // A
		Spawn(1, vec2(WallL() + c.m_BDepth, Y0())); // B
		Chr(1)->Freeze();
		m_aTee[1].m_Input.m_Direction = c.m_BDir;
		m_aTee[1].m_Input.m_Jump = c.m_BJump;
		if(Rec)
		{
			Name(0, "A");
			Name(1, "B");
			Name(2, "C");
			Caption("Relay 2-tile aled. B is frozen INSIDE the freeze.\nA hammers B through, then walks into the freeze HOLDING FIRE.");
			Hold(60);
		}
		for(int i = 0; i < 3; i++)
			Step(3);
		Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
		PressFire(0); // ... and keep holding it
		Step(3);
		bool Hit1 = Chr(0) && Chr(0)->m_ReloadTimer > 5;
		bool Said = false;
		for(int i = 1; i < 140 && Hit1 && Chr(0) && Chr(1) && Chr(2); i++)
		{
			m_aTee[0].m_Input.m_Direction = i <= c.m_Walk ? 1 : 0;
			{
				// C walks right and brakes before its stop point
				const CCharacter *pC = Chr(2);
				m_aTee[2].m_Input.m_Direction = pC->m_Pos.x + pC->m_Core.m_Vel.x * 3 < WallL() - c.m_CGap ? 1 : 0;
			}
			Aim(0, c.m_AimRight ? vec2(1, 0) : normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
			if(i == TC)
			{
				if(pDepth)
					*pDepth = (int)(Chr(0)->m_Pos.x - WallL());
				if(pPast)
					*pPast = (int)(Chr(1)->m_Pos.x - WallR());
				Aim(2, normalize(Chr(0)->m_Pos - Chr(2)->m_Pos));
				PressFire(2);
				if(Rec)
					Caption("C hammers A (any time). Being unfrozen gives A full-auto for one tick:\nA's HELD fire swings from inside the freeze and frees B.");
			}
			else
				ReleaseFire(2);
			if(Rec && !Said && i == TC - 25)
			{
				Caption("A is frozen in the freeze, still holding fire. B rests past the wall.");
				Said = true;
			}
			Step(3);
		}
		bool Ok = Hit1 && Chr(1) && Chr(1)->m_FreezeTime == 0 && Chr(1)->m_Pos.x >= WallR();
		if(Rec)
		{
			Caption(Ok ? "B is free. No frame-perfect input was needed." : "failed");
			Hold(60);
		}
		return Finish(Ok);
	};

	struct SRes
	{
		SRelayCfg m_Cfg;
		int m_Window, m_First, m_Last;
	};
	std::vector<SRes> vRes;
	for(int BDepth = 4; BDepth <= 52; BDepth += 8)
		for(int AGap = 4; AGap <= 28 && AGap + BDepth < 63; AGap += 8)
			for(int BDir = -1; BDir <= 1; BDir++)
				for(int BJump = 0; BJump <= 1; BJump++)
					for(int Walk = 4; Walk <= 16; Walk += 4)
						for(int CGap : {4, 14, 24})
						{
							SRelayCfg c{BDepth, AGap, BDir, BJump, Walk, CGap, false};
							int Window = 0, First = -1, Last = -1;
							for(int TC = 10; TC <= 130; TC += 4)
								if(Run(c, TC, false))
								{
									Window++;
									if(First < 0)
										First = TC;
									Last = TC;
								}
							vRes.push_back({c, Window, First, Last});
						}
	std::sort(vRes.begin(), vRes.end(), [](auto &a, auto &b) { return a.m_Window > b.m_Window; });
	int NWorking = 0;
	for(auto &r : vRes)
		NWorking += r.m_Window > 0;
	printf("relay: %zu setups, %d work for at least one C timing\n", vRes.size(), NWorking);
	for(size_t i = 0; i < std::min<size_t>(10, vRes.size()); i++)
	{
		auto &r = vRes[i];
		printf("  C may hammer in %2d/31 sampled ticks (%d..%d) | B %d px deep dir=%d jump=%d | A %d px from wall, walks %d | C stops %d px from wall\n",
			r.m_Window, r.m_First, r.m_Last, r.m_Cfg.m_BDepth, r.m_Cfg.m_BDir, r.m_Cfg.m_BJump, r.m_Cfg.m_AGap, r.m_Cfg.m_Walk, r.m_Cfg.m_CGap);
	}
	ASSERT_GT(vRes[0].m_Window, 0);
	// does it still work if A doesn't track B but just aims right?
	SRelayCfg Best = vRes[0].m_Cfg;
	Best.m_AimRight = true;
	int WindowRight = 0;
	for(int TC = 10; TC <= 130; TC += 4)
		WindowRight += Run(Best, TC, false);
	printf("  best setup with A simply aiming right: %d/31\n", WindowRight);
	// every single tick in the window
	int Exact = 0, D = 0, P = 0;
	for(int TC = vRes[0].m_First; TC <= vRes[0].m_Last; TC++)
		Exact += Run(vRes[0].m_Cfg, TC, false);
	Run(vRes[0].m_Cfg, (vRes[0].m_First + vRes[0].m_Last) / 2, false, &D, &P);
	printf("  best setup, every tick %d..%d: %d work. At the middle: A %d px deep, B %d px past the wall\n", vRes[0].m_First, vRes[0].m_Last, Exact, D, P);
	// sloppiness: neighbours of the best setup (positions +-4 px, walk +-2 ticks, B's keys), C hammering "late" at 60 ticks
	{
		int Ok = 0, Total = 0;
		for(int dB : {-4, 0, 4})
			for(int dA : {-4, 0, 4})
				for(int dW : {-2, 0, 2})
					for(int dC : {-6, 0, 6})
						for(int Keys = 0; Keys < 2; Keys++)
						{
							SRelayCfg n = vRes[0].m_Cfg;
							n.m_BDepth += dB;
							n.m_AGap += dA;
							n.m_Walk += dW;
							n.m_CGap = std::max(2, n.m_CGap + dC);
							n.m_BJump = Keys;
							Total++;
							Ok += Run(n, 60, false);
						}
		printf("  sloppy neighbours of the best setup (C hammers at 60): %d/%d work\n", Ok, Total);
	}
	StartRec("sim7_aled_2tile_relay");
	EXPECT_TRUE(Run(vRes[0].m_Cfg, (vRes[0].m_First + vRes[0].m_Last) / 2, true));
	StopRec();
}

// Vertical relay: B sits frozen at the bottom of a 2-deep freeze pool. A (on normal floor next to the pool) knocks
// B up and out, then steps into the pool edge (frozen) HOLDING FIRE aimed at B. C hammers A while B is above the pool.
// When B is free it releases/presses jump and holds right to escape.
struct SVertCfg
{
	int m_BX, m_AX; // B center px right of the pool's left edge, A center px left of it
	int m_BDir, m_BJump;
	int m_Walk; // ticks A walks right after the first hammer
	int m_CGap; // where C stops, px left of the pool edge
	int m_HammerDelay = 0; // A jumps and does the first hammer this many ticks later (0 = from the ground)
	int m_JumpIn = 0; // A jumps while walking into the pool
};

class SimVert : public SimAled
{
public:
	float PoolL() const { return SimMap::POOL_L * 32; }
	float PoolTop() const { return (SimMap::POOL_GROUND - 2) * 32; }
	float GroundY() const { return (SimMap::POOL_GROUND - 1) * 32 + 16; }

	// returns true if B ends free and outside the pool; *pBAbove = B px above the pool top when C fired
	bool RunVert(const SVertCfg &c, int TC, bool Rec, int *pBAbove = nullptr, int *pADepth = nullptr)
	{
		Spawn(2, vec2(PoolL() - c.m_AX - 40, GroundY())); // C
		Spawn(0, vec2(PoolL() - c.m_AX, GroundY())); // A
		Spawn(1, vec2(PoolL() + c.m_BX, GroundY())); // B
		Chr(1)->Freeze();
		m_aTee[1].m_Input.m_Direction = c.m_BDir;
		m_aTee[1].m_Input.m_Jump = c.m_BJump;
		if(Rec)
		{
			Name(0, "A");
			Name(1, "B");
			Name(2, "C");
			Caption("Vertical relay aled. B is frozen at the bottom of a 2-deep freeze pool.\nA knocks B up, then steps into the pool HOLDING FIRE.");
			Hold(60);
		}
		for(int i = 0; i < 3; i++)
			Step(3);
		if(c.m_HammerDelay > 0)
		{
			m_aTee[0].m_Input.m_Jump = 1;
			for(int i = 0; i < c.m_HammerDelay; i++)
				Step(3);
			m_aTee[0].m_Input.m_Jump = 0;
		}
		Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
		PressFire(0);
		Step(3);
		if(!Chr(0) || Chr(0)->m_ReloadTimer <= 5)
			return Finish(false);
		int Freed = -1;
		for(int i = 1; i < 130 && Chr(0) && Chr(1) && Chr(2); i++)
		{
			m_aTee[0].m_Input.m_Direction = i <= c.m_Walk ? 1 : 0;
			m_aTee[0].m_Input.m_Jump = c.m_JumpIn && i >= 2 && i <= 4;
			Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
			const CCharacter *pC = Chr(2);
			m_aTee[2].m_Input.m_Direction = pC->m_Pos.x + pC->m_Core.m_Vel.x * 3 < PoolL() - c.m_CGap ? 1 : 0;
			if(i == TC)
			{
				if(pBAbove)
					*pBAbove = (int)(PoolTop() - Chr(1)->m_Pos.y);
				if(pADepth)
					*pADepth = (int)(Chr(0)->m_Pos.x - PoolL());
				Aim(2, normalize(Chr(0)->m_Pos - Chr(2)->m_Pos));
				PressFire(2);
				if(Rec)
					Caption("C hammers A while B is above the pool:\nA's held fire swings from inside the freeze and frees B.");
			}
			else
				ReleaseFire(2);
			// B escapes once free: jump again (release first) and hold right
			if(Freed < 0 && i > TC && Chr(1)->m_FreezeTime == 0)
				Freed = i;
			if(Freed >= 0)
			{
				m_aTee[1].m_Input.m_Direction = 1;
				m_aTee[1].m_Input.m_Jump = i > Freed;
			}
			Step(3);
		}
		bool Ok = Freed >= 0 && Chr(1) && Chr(1)->m_FreezeTime == 0;
		if(Rec)
		{
			Caption(Ok ? "B is free and jumps out. C only had to hit a window of several ticks." : "failed");
			Hold(60);
		}
		return Finish(Ok);
	}
};

TEST_F(SimVert, RelayVertical)
{
	struct SRes
	{
		SVertCfg m_Cfg;
		int m_Window, m_First, m_Last;
	};
	std::vector<SRes> vRes;
	for(int BX = 2; BX <= 34; BX += 4)
		for(int AX = 14; AX <= 60; AX += 6)
			for(int BDir = -1; BDir <= 1; BDir++)
				for(int HammerDelay : {0, 1})
					for(int Walk : {6, 10, 14, 18})
						for(int JumpIn = 0; JumpIn <= 1; JumpIn++)
						{
							SVertCfg c{BX, AX, BDir, 0, Walk, 4, HammerDelay, JumpIn};
							int Window = 0, First = -1, Last = -1;
							for(int TC = 4; TC <= 45; TC++)
								if(RunVert(c, TC, false))
								{
									Window++;
									if(First < 0)
										First = TC;
									Last = TC;
								}
							vRes.push_back({c, Window, First, Last});
						}
	std::sort(vRes.begin(), vRes.end(), [](auto &a, auto &b) { return a.m_Window > b.m_Window; });
	int NWorking = 0;
	for(auto &r : vRes)
		NWorking += r.m_Window > 0;
	printf("vertical relay: %zu setups, %d work for at least one C timing\n", vRes.size(), NWorking);
	for(size_t i = 0; i < std::min<size_t>(10, vRes.size()); i++)
	{
		auto &r = vRes[i];
		printf("  C window %2d ticks (%d..%d) | B %d px into pool dir=%d | A %d px left of pool, hammers %d ticks into a jump, walks %d%s\n",
			r.m_Window, r.m_First, r.m_Last, r.m_Cfg.m_BX, r.m_Cfg.m_BDir, r.m_Cfg.m_AX, r.m_Cfg.m_HammerDelay, r.m_Cfg.m_Walk, r.m_Cfg.m_JumpIn ? " + jumps in" : "");
	}
	ASSERT_GT(vRes[0].m_Window, 0);
	int Above = 0, Depth = 0;
	RunVert(vRes[0].m_Cfg, (vRes[0].m_First + vRes[0].m_Last) / 2, false, &Above, &Depth);
	printf("  best, middle of window: B %d px above the pool, A %d px into the pool\n", Above, Depth);
	StartRec("sim8_aled_2tile_vertical_relay");
	EXPECT_TRUE(RunVert(vRes[0].m_Cfg, (vRes[0].m_First + vRes[0].m_Last) / 2, true));
	StopRec();
}

// ---------- flashy: both tees through a 2-tile wall ----------
// The dynamic 2-tile aled, but A keeps flying through the wall (frozen), and once A is out, the freshly freed B
// hammers A back. Both end up unfrozen on the far side.
struct SBothCfg
{
	SDynCfg m_Dyn;
	int m_BSteer; // B's direction once free
	int m_BJump; // B jumps once free
	int m_PressDelay; // ticks after the first valid tick that B presses hammer
};
struct SBothRes
{
	bool m_Ok = false;
	bool m_BFreed = false;
	int m_ValidTicks = 0; // ticks in which B could have hammered A successfully (A out, clear, in reach)
};

class SimBoth : public SimAled
{
public:
	SBothRes RunBoth(const SBothCfg &c, bool Rec)
	{
		SBothRes r;
		const SDynCfg &d = c.m_Dyn;
		Spawn(0, vec2(WallL() - d.m_AGap, Y0()));
		Spawn(1, vec2(WallL() + d.m_BDepth, Y0()));
		Chr(1)->Freeze();
		m_aTee[1].m_Input.m_Direction = d.m_BDir;
		m_aTee[1].m_Input.m_Jump = d.m_BJump;
		if(Rec)
		{
			Name(0, "A");
			Name(1, "B");
			Caption("B is frozen inside a 2-tile freeze wall.");
			Hold(50);
		}
		for(int i = 0; i < 3; i++)
			Step(2);
		Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
		PressFire(0);
		Step(2);
		if(!Chr(0) || Chr(0)->m_ReloadTimer <= 5)
			return Finish(r);
		int Stop = -1, Freed = -1, FirstValid = -1, Pressed = -1;
		for(int i = 1; i < 110 && Chr(0) && Chr(1); i++)
		{
			CCharacter *pA = Chr(0), *pB = Chr(1);
			// A's program until it is inside the freeze
			if(Stop < 0 && pA->m_Pos.x >= WallL())
				Stop = i;
			bool Moving = Stop < 0;
			m_aTee[0].m_Input.m_Direction = Moving ? (i <= d.m_Back ? -1 : 1) : 0;
			m_aTee[0].m_Input.m_Jump = Moving && d.m_Jump >= 0 && i >= d.m_Jump;
			if(i == Stop)
			{
				Aim(0, normalize(pB->m_Pos - pA->m_Pos));
				PressFire(0);
				if(Rec)
					Caption("A dives in: hammer on its first tick inside the freeze frees B...");
			}
			else
				ReleaseFire(0);
			// B once free
			if(Freed < 0 && Stop >= 0 && i > Stop && pB->m_FreezeTime == 0)
			{
				Freed = i;
				r.m_BFreed = true;
			}
			ReleaseFire(1);
			if(Freed >= 0)
			{
				m_aTee[1].m_Input.m_Direction = c.m_BSteer;
				m_aTee[1].m_Input.m_Jump = c.m_BJump && i > Freed;
				// can B hammer A now? A out of the wall (last move too), frozen, in reach
				vec2 APrev = pA->m_Pos - pA->m_Core.m_Vel; // approximate previous position
				bool Valid = pA->m_FreezeTime > 0 && pA->m_Pos.x >= WallR() && APrev.x >= WallR() + 1 && distance(pA->m_Pos, pB->m_Pos) < 60;
				if(Valid)
				{
					r.m_ValidTicks++;
					if(FirstValid < 0)
						FirstValid = i;
				}
				if(Pressed < 0 && FirstValid >= 0 && i == FirstValid + c.m_PressDelay)
				{
					Pressed = i;
					Aim(1, normalize(pA->m_Pos - pB->m_Pos));
					PressFire(1);
					if(Rec)
						Caption("...A flies through frozen, and B hammers A back. Both are through.");
				}
			}
			Step(2);
		}
		r.m_Ok = Chr(0) && Chr(1) && Chr(0)->m_FreezeTime == 0 && Chr(1)->m_FreezeTime == 0 && Chr(0)->m_Pos.x >= WallR() && Chr(1)->m_Pos.x >= WallR();
		if(Rec)
			Hold(50);
		return Finish(r);
	}
};

TEST_F(SimBoth, Search)
{
	// working dynamic aleds (as in Dynamic2TeeConsistency)
	std::vector<SDynCfg> vDyn;
	for(int BDepth = 4; BDepth <= 60; BDepth += 4)
		for(int AGap = 2; AGap + BDepth < 63; AGap += 4)
			for(int BDir = -1; BDir <= 1; BDir++)
				for(int BJump = 0; BJump <= 1; BJump++)
					for(int Back = 0; Back <= 14; Back += 2)
						for(int Jump = -1; Jump <= 36; Jump += (Jump < 0 ? 1 : 3))
						{
							SDynCfg c{BDepth, AGap, BDir, BJump, Back, Jump};
							if(RunDyn2(c, 0).m_Ok)
								vDyn.push_back(c);
						}
	printf("dynamic aleds: %zu\n", vDyn.size());
	struct SHit
	{
		SBothCfg m_Cfg;
		int m_Valid;
	};
	std::vector<SHit> vHits;
	for(auto &d : vDyn)
		for(int Steer = -1; Steer <= 1; Steer++)
			for(int J = 0; J <= 1; J++)
			{
				SBothCfg c{d, Steer, J, 0};
				SBothRes r = RunBoth(c, false);
				if(r.m_Ok)
					vHits.push_back({c, r.m_ValidTicks});
			}
	printf("both-through: %zu (config, B steering) combos work\n", vHits.size());
	// hammer-back window for every hit, then prefer B starting deep in the freeze with a comfortable window
	std::vector<int> vWindow;
	for(auto &h : vHits)
	{
		int Delays = 0;
		for(int k = 0; k < 20; k++)
		{
			SBothCfg c = h.m_Cfg;
			c.m_PressDelay = k;
			Delays += RunBoth(c, false).m_Ok;
		}
		h.m_Valid = Delays;
	}
	std::sort(vHits.begin(), vHits.end(), [](auto &a, auto &b) {
		int Sa = a.m_Cfg.m_Dyn.m_BDepth * 10 + std::min(a.m_Valid, 8) * 3;
		int Sb = b.m_Cfg.m_Dyn.m_BDepth * 10 + std::min(b.m_Valid, 8) * 3;
		return Sa > Sb;
	});
	for(size_t i = 0; i < std::min<size_t>(10, vHits.size()); i++)
	{
		auto &h = vHits[i];
		const SDynCfg &d = h.m_Cfg.m_Dyn;
		int Delays = h.m_Valid;
		printf("  B's hammer-back window %2d ticks | B %d deep keys %d/%d | A gap %d back %d jump@%d | B steers %d jump %d\n", Delays,
			d.m_BDepth, d.m_BDir, d.m_BJump, d.m_AGap, d.m_Back, d.m_Jump, h.m_Cfg.m_BSteer, h.m_Cfg.m_BJump);
	}
	ASSERT_FALSE(vHits.empty());
	StartRec("sim9_both_through");
	SBothCfg Show = vHits[0].m_Cfg;
	Show.m_PressDelay = std::min(2, vHits[0].m_Valid - 1);
	EXPECT_TRUE(RunBoth(Show, true).m_Ok);
	StopRec();
}


// ---------- flashy: killtile gauntlet, down and back up ----------
// A tee falls through several rows of death tiles, dashes ninja UP at the bottom (the dash gives its speed back,
// upward) and flies back up through the same rows. Death tiles don't affect movement, so the trajectory is simulated
// first and rows are placed only where both passes jump over them. A second tee dropped a few px off dies.
class SimGauntlet : public SimAled
{
public:
};

TEST_F(SimGauntlet, DownAndUp)
{
	const float X0 = 162 * 32 + 16, X1 = 168 * 32 + 16, Y0 = 30 * 32 + 16, NinjaY = 340 * 32;
	// fly tee t from (X, Y) down, ninja up at NinjaY; returns the ninja tick; pTraj gets y after every tick
	auto Fly = [&](int t, float X, float Y, int NinjaTick, std::vector<int> *pTraj, bool Rec) {
		Spawn(t, vec2(X, Y));
		Aim(t, vec2(0, -1));
		if(pTraj)
			pTraj->push_back((int)Chr(t)->m_Pos.y);
		for(int i = 0; i < 600 && Chr(t); i++)
		{
			bool Now = NinjaTick < 0 ? Chr(t)->m_Pos.y >= NinjaY : i == NinjaTick;
			if(Now)
			{
				NinjaTick = i;
				Chr(t)->GiveNinja();
				PressFire(t);
			}
			else
				ReleaseFire(t);
			Step(2);
			if(!Chr(t))
				break;
			if(pTraj)
				pTraj->push_back((int)Chr(t)->m_Pos.y);
			if(NinjaTick >= 0 && i > NinjaTick + 12 && Chr(t)->m_Core.m_Vel.y >= 0)
				break; // apex after the launch
		}
		return NinjaTick;
	};
	// 1) trajectory without rows
	std::vector<int> vTraj;
	int NinjaTick = Fly(0, X0, Y0, -1, &vTraj, false);
	Finish(0);
	int Apex = *std::min_element(vTraj.begin(), vTraj.end());
	printf("gauntlet: ninja at tick %d, %zu ticks, apex y=%d (start %d)\n", NinjaTick, vTraj.size(), Apex, (int)Y0);
	// 2) rows skipped on the way down AND up
	std::vector<int> vRows;
	for(int r = 40; r < 330; r++)
	{
		int Lo = 32 * r - 9, Hi = 32 * r + 40;
		if(Apex >= Lo)
			continue; // must be crossed on the way up too
		bool Hit = false;
		for(int y : vTraj)
			Hit |= y >= Lo && y <= Hi;
		if(!Hit && (vRows.empty() || r - vRows.back() >= 24))
			vRows.push_back(r);
	}
	while(vRows.size() > 5)
		vRows.erase(vRows.begin() + vRows.size() / 2);
	printf("  rows skipped both ways:");
	for(int r : vRows)
		printf(" %d", r);
	printf("\n");
	ASSERT_GE(vRows.size(), 2u);
	SimMap::gvExtraRects.clear();
	for(int r : vRows)
		SimMap::gvExtraRects.push_back({150, r, 180, r, TILE_DEATH});
	UseExtendedMapForDemo();
	// 3) a second tee dropped a few px off that dies on the first row
	int Off = -1;
	for(int d = 1; d < 40 && Off < 0; d++)
	{
		Fly(1, X1, Y0 + d, NinjaTick, nullptr, false);
		bool Died = !Chr(1);
		Finish(0);
		if(Died)
			Off = d;
	}
	ASSERT_GE(Off, 0);
	printf("  second tee dies when dropped %d px lower\n", Off);
	// 4) record both
	Spawn(1, vec2(X1, Y0 + Off));
	Spawn(0, vec2(X0, Y0));
	Name(0, "threads it");
	char aName[32];
	str_format(aName, sizeof(aName), "%d px lower", Off);
	Name(1, aName);
	Aim(0, vec2(0, -1));
	bool Said = false, Alive = true, Recording = false;
	for(int i = 0; i < 600 && Chr(0); i++)
	{
		// start the demo shortly before the first row (the first 100 ticks are just falling)
		if(!Recording && Chr(0)->m_Pos.y >= vRows[0] * 32 - 900)
		{
			StartRec("sim11_killtile_gauntlet");
			char aBuf[200];
			str_format(aBuf, sizeof(aBuf), "Two tees dropped from %d tiles up, one of them %d px lower.\n%d rows of death tiles below. Moving ~%d px per tick.", (vRows[0] * 32 - (int)Y0) / 32, Off,
				(int)vRows.size(), (int)Chr(0)->m_Core.m_Vel.y);
			Caption(aBuf);
			Hold(30);
			Recording = true;
		}
		if(i == NinjaTick)
		{
			Chr(0)->GiveNinja();
			PressFire(0);
			Caption("Ninja dash UP: when it ends, the tee gets its full speed back - upward.");
		}
		else
			ReleaseFire(0);
		if(!Said && !Chr(1))
		{
			Caption("One tick lands inside a kill zone: dead. The other steps over every row.");
			Said = true;
		}
		Step(2);
		if(!Chr(0))
		{
			Alive = false;
			break;
		}
		if(i > NinjaTick + 12 && Chr(0)->m_Core.m_Vel.y >= 0)
			break;
	}
	Caption(Alive ? "Down through every row and back up again - alive." : "died");
	Hold(50);
	StopRec();
	EXPECT_TRUE(Alive);
}

// ---------- flying aleds ----------
// Hammerfly: H (tee 0, bottom) hammers T (tee 1, top) whenever it can; T hooks H and re-hooks when the hook runs out.
class SimFly : public SimAled
{
public:
	int m_HookReleaseTicks = 0;
	// one tick of hammerfly inputs (does not step)
	void HammerflyInputs(int Dir)
	{
		CCharacter *pH = Chr(0), *pT = Chr(1);
		// T: hook H
		Aim(1, normalize(pH->m_Pos - pT->m_Pos));
		int HookState = pT->m_Core.m_HookState;
		if(m_HookReleaseTicks > 0)
		{
			m_aTee[1].m_Input.m_Hook = 0;
			m_HookReleaseTicks--;
		}
		else if(m_aTee[1].m_Input.m_Hook && HookState != HOOK_GRABBED && HookState != HOOK_FLYING)
		{
			m_aTee[1].m_Input.m_Hook = 0; // re-hook next tick
			m_HookReleaseTicks = 1;
		}
		else
			m_aTee[1].m_Input.m_Hook = 1;
		// H: hammer T whenever ready and T is above
		Aim(0, normalize(pT->m_Pos - pH->m_Pos));
		if(pH->m_ReloadTimer == 0 && !(m_aTee[0].m_Input.m_Fire & 1) && pT->m_Pos.y < pH->m_Pos.y)
			PressFire(0);
		else
			ReleaseFire(0);
		m_aTee[0].m_Input.m_Direction = Dir;
		m_aTee[1].m_Input.m_Direction = Dir;
	}
};

TEST_F(SimFly, HammerflyWorks)
{
	UseBigMap(Col(), 100, 400, &GameServer()->m_World);
	FillRect(0, 380, 99, 380, TILE_SOLID);
	Spawn(0, vec2(50 * 32 + 16, 379 * 32 + 16));
	Spawn(1, vec2(50 * 32 + 16, 379 * 32 + 16 - 40));
	for(int i = 0; i < 400 && Chr(0) && Chr(1); i++)
	{
		HammerflyInputs(0);
		Step(2);
		if(i % 25 == 0)
			printf("dbg t=%3d H y=%.0f T y=%.0f (height above floor %.0f / %.0f) H vy=%.1f hook=%d\n", i, Chr(0)->m_Pos.y, Chr(1)->m_Pos.y, 380 * 32 - Chr(0)->m_Pos.y, 380 * 32 - Chr(1)->m_Pos.y, Chr(0)->m_Core.m_Vel.y, Chr(1)->m_Core.m_HookState);
	}
	Finish(0);
}

// Vertical 2-tile aled out of a hammerfly: T goes up through a 2-tile freeze ceiling (frozen), H air-jumps in
// behind it and hammers on its first tick inside the freeze, flies through itself, T hammers H back, and they
// keep hammerflying above the ceiling.
struct SHfCfg
{
	int m_Ceil; // ceiling top row
	int m_TOff; // T spawns this many px above H
	int m_HJump; // ticks after T froze that H jumps (-1 = no jump)
	int m_BackDelay = 0; // ticks after the first valid tick that T hammers H back
};
struct SHfRes
{
	bool m_Ok = false, m_TFreed = false;
	int m_Valid = 0, m_Depth = 0, m_Past = 0;
};

class SimHammerfly : public SimFly
{
public:
	static constexpr int X0 = 240, X1 = 258, FLOOR = 380;
	float CX() const { return 249 * 32 + 16; }
	void Arena(int Ceil)
	{
		FillRect(X0, 5, X1, FLOOR - 1, TILE_AIR);
		FillRect(X0, FLOOR, X1, FLOOR, TILE_SOLID);
		FillRect(X0, Ceil, X1, Ceil + 1, TILE_FREEZE);
	}
	SHfRes Run(const SHfCfg &c, bool Rec)
	{
		SHfRes r;
		const float CeilTop = c.m_Ceil * 32, CeilBot = (c.m_Ceil + 2) * 32;
		Arena(c.m_Ceil);
		Spawn(0, vec2(CX(), (FLOOR - 1) * 32 + 16));
		Spawn(1, vec2(CX(), (FLOOR - 1) * 32 + 16 - c.m_TOff));
		m_HookReleaseTicks = 0;
		if(Rec)
		{
			Name(0, "H");
			Name(1, "T");
			Caption("Hammerfly: H (bottom) hammers T up, T hooks H.\nAbove them: a 2-tile freeze ceiling.");
			Hold(40);
		}
		int Phase = 0, TFroze = -1, HFired = -1, FirstValid = -1, BackPressed = -1, Resume = -1;
		for(int i = 0; i < 900 && Chr(0) && Chr(1); i++)
		{
			CCharacter *pH = Chr(0), *pT = Chr(1);
			if(Phase == 0)
			{
				HammerflyInputs(0);
				if(pT->m_FreezeTime > 0)
				{
					Phase = 1;
					TFroze = i;
					if(Rec)
						Caption("T is kicked up through the freeze ceiling (frozen). H air-jumps in behind it...");
				}
			}
			if(Phase == 1)
			{
				m_aTee[1].m_Input.m_Hook = 0;
				m_aTee[0].m_Input.m_Jump = c.m_HJump >= 0 && i >= TFroze + c.m_HJump;
				if(pH->m_Pos.y < CeilBot && pH->m_Pos.y >= CeilTop && pH->m_FreezeTime == 0)
				{
					r.m_Depth = (int)(CeilBot - pH->m_Pos.y);
					r.m_Past = (int)(CeilTop - pT->m_Pos.y);
					Aim(0, normalize(pT->m_Pos - pH->m_Pos));
					PressFire(0);
					HFired = i;
					Phase = 2;
					if(Rec)
					{
						char aBuf[160];
						str_format(aBuf, sizeof(aBuf), "...and hammers on its first tick inside: %d px into the freeze, T %d px above it.", r.m_Depth, r.m_Past);
						Caption(aBuf);
					}
				}
				else
					ReleaseFire(0);
				if(pH->m_FreezeTime > 0 || (TFroze >= 0 && i > TFroze + 80))
					break; // H froze without firing, or never got there
			}
			else if(Phase == 2)
			{
				if(i > HFired)
					ReleaseFire(0);
				m_aTee[0].m_Input = {m_aTee[0].m_Input.m_Direction, m_aTee[0].m_Input.m_TargetX, m_aTee[0].m_Input.m_TargetY, 0, m_aTee[0].m_Input.m_Fire, 0, 0, 0, 0, 0};
				if(pT->m_FreezeTime == 0)
					r.m_TFreed = true;
				// T hammers H back once H is out above the ceiling
				vec2 HPrev = pH->m_Pos - pH->m_Core.m_Vel;
				bool V = r.m_TFreed && pH->m_FreezeTime > 0 && pH->m_Pos.y < CeilTop && HPrev.y < CeilTop - 1 && distance(pH->m_Pos, pT->m_Pos) < 60;
				if(V)
				{
					r.m_Valid++;
					if(FirstValid < 0)
						FirstValid = i;
				}
				if(BackPressed < 0 && FirstValid >= 0 && i == FirstValid + c.m_BackDelay)
				{
					BackPressed = i;
					Aim(1, normalize(pH->m_Pos - pT->m_Pos));
					PressFire(1);
					if(Rec)
						Caption("H flies through frozen, T hammers it back - and they keep flying.");
				}
				else
					ReleaseFire(1);
				if(BackPressed >= 0 && i > BackPressed + 1 && pH->m_FreezeTime == 0)
				{
					Phase = 3;
					Resume = i;
				}
				if(i > HFired + 80)
					break;
			}
			else if(Phase == 3)
			{
				HammerflyInputs(0);
				if(i > Resume + (Rec ? 150 : 100))
					break;
			}
			Step(2);
		}
		r.m_Ok = Phase == 3 && Chr(0) && Chr(1) && Chr(0)->m_FreezeTime == 0 && Chr(1)->m_FreezeTime == 0 && Chr(0)->m_Pos.y < CeilTop && Chr(1)->m_Pos.y < CeilTop;
		if(Rec)
			Hold(30);
		for(auto &t : m_aTee)
			t.m_Input = {};
		return Finish(r);
	}
};

TEST_F(SimHammerfly, Search)
{
	struct SHit
	{
		SHfCfg m_Cfg;
		SHfRes m_Res;
		int m_Window = 0;
	};
	std::vector<SHit> vHits;
	long Sims = 0, TFreed = 0;
	for(int Ceil = FLOOR - 40; Ceil <= FLOOR - 8; Ceil++)
		for(int TOff = 36; TOff <= 70; TOff += 2)
			for(int HJump = -1; HJump <= 24; HJump++)
			{
				SHfCfg c{Ceil, TOff, HJump};
				SHfRes r = Run(c, false);
				Sims++;
				TFreed += r.m_TFreed;
				if(r.m_Ok)
					vHits.push_back({c, r});
			}
	printf("hammerfly 2-tile aled: %ld sims, T freed in %ld, full success %zu\n", Sims, TFreed, vHits.size());
	ASSERT_FALSE(vHits.empty());
	for(auto &h : vHits)
		for(int k = 0; k < 15; k++)
		{
			SHfCfg c = h.m_Cfg;
			c.m_BackDelay = k;
			h.m_Window += Run(c, false).m_Ok;
		}
	// best window first; among equal windows, the lowest ceiling (shorter clip)
	std::sort(vHits.begin(), vHits.end(), [](auto &a, auto &b) { return a.m_Window != b.m_Window ? a.m_Window > b.m_Window : a.m_Cfg.m_Ceil > b.m_Cfg.m_Ceil; });
	for(size_t i = 0; i < std::min<size_t>(10, vHits.size()); i++)
	{
		auto &h = vHits[i];
		printf("  back window %2d | ceiling %d tiles up, T starts %d px up, H jumps %d ticks after T froze | H %d px in, T %d px above\n", h.m_Window,
			FLOOR - h.m_Cfg.m_Ceil, h.m_Cfg.m_TOff, h.m_Cfg.m_HJump, h.m_Res.m_Depth, h.m_Res.m_Past);
	}
	SimMap::gvExtraRects.clear();
	SHfCfg Show = vHits[0].m_Cfg;
	Show.m_BackDelay = std::min(2, vHits[0].m_Window - 1);
	SimMap::gvExtraRects.push_back({X0, FLOOR, X1, FLOOR, TILE_SOLID});
	SimMap::gvExtraRects.push_back({X0, Show.m_Ceil, X1, Show.m_Ceil + 1, TILE_FREEZE});
	UseExtendedMapForDemo();
	StartRec("sim12_hammerfly_aled");
	EXPECT_TRUE(Run(Show, true).m_Ok);
	StopRec();
}

// Hookfly as players describe it: the launched tee passes its partner, keeps rising, and near the top of its rise
// (vy > -Wait) hooks the partner below. The previous hooker lets go once it is passed.
struct SHookflyPolicy
{
	float m_Wait; // hook when own vy > -m_Wait (rising slower than this)
	int m_HookerDir; // 0 none, 1 away from partner, -1 toward partner
	int m_RiderDir;
	int m_Release = -10; // hooker lets go once the rider is above it by -m_Release px (negative = still below)
	int m_FlingX = -1; // if >= 0: a hooker left of this x keeps hooking until the rider is past it (fling it through a wall)
	int m_Flight = 0; // horizontal hookfly: the hooker moves this way ...
	int m_MoveDelay = 0; // ... this many ticks after it started hooking
	int m_RiderFlight = 0; // the rider also holds the flight direction
};
struct SHookflyState
{
	int m_Hooker = 1; // tee currently hooking (starts with the one that jumped)
	bool m_Started = false;
	int m_Tick = 0, m_HookStart = 0;
};
void HookflyStep(SimFly *p, SHookflyState &St, const SHookflyPolicy &Pol)
{
	int Hk = St.m_Hooker, R = 1 - Hk;
	CCharacter *pHk = p->Chr(Hk), *pR = p->Chr(R);
	auto &HkIn = p->m_aTee[Hk].m_Input;
	auto &RIn = p->m_aTee[R].m_Input;
	// the rider takes over once it is above the hooker and near the top of its rise
	St.m_Tick++;
	bool FlingNow = Pol.m_FlingX >= 0 && pHk->m_Pos.x >= Pol.m_FlingX && pR->m_Pos.x < pHk->m_Pos.x + 20;
	if(!FlingNow && pR->m_Pos.y < pHk->m_Pos.y - 10 && pR->m_Core.m_Vel.y > -Pol.m_Wait)
	{
		St.m_Hooker = R;
		St.m_HookStart = St.m_Tick;
		std::swap(Hk, R);
		std::swap(pHk, pR);
	}
	auto &In1 = p->m_aTee[Hk].m_Input;
	auto &In2 = p->m_aTee[R].m_Input;
	(void)HkIn;
	(void)RIn;
	p->Aim(Hk, normalize(pR->m_Pos - pHk->m_Pos));
	int State = pHk->m_Core.m_HookState;
	bool Fling = Pol.m_FlingX >= 0 && pHk->m_Pos.x >= Pol.m_FlingX && pR->m_Pos.x < pHk->m_Pos.x + 20;
	if(!Fling && pR->m_Pos.y < pHk->m_Pos.y + Pol.m_Release)
		In1.m_Hook = 0; // (nearly) passed: let go (hooking now would pull it back down)
	else if(In1.m_Hook && State != HOOK_GRABBED && State != HOOK_FLYING)
		In1.m_Hook = 0;
	else
		In1.m_Hook = 1;
	// the old hooker lets go (it is being pulled now)
	In2.m_Hook = 0;
	int Away = pHk->m_Pos.x >= pR->m_Pos.x ? 1 : -1;
	In1.m_Direction = Pol.m_HookerDir * Away;
	In2.m_Direction = Pol.m_RiderDir * -Away;
	if(Pol.m_Flight)
	{
		In1.m_Direction = St.m_Tick - St.m_HookStart >= Pol.m_MoveDelay ? Pol.m_Flight : 0;
		In2.m_Direction = Pol.m_RiderFlight ? Pol.m_Flight : 0;
	}
	In1.m_Jump = 0;
	In2.m_Jump = 0;
}

TEST_F(SimFly, HookflySearch)
{
	UseBigMap(Col(), 200, 400, &GameServer()->m_World);
	FillRect(0, 380, 199, 380, TILE_SOLID);
	struct SBest
	{
		SHookflyPolicy m_Pol;
		int m_Dx;
		float m_Height;
	};
	std::vector<SBest> v;
	for(float Wait : {0.f, 2.f, 4.f, 6.f, 8.f, 10.f, 12.f})
		for(int HD = -1; HD <= 1; HD++)
			for(int RD = -1; RD <= 1; RD++)
				for(int Dx : {40, 80})
					for(int Release : {-80, -40, -20, 0, 20})
				{
					SHookflyPolicy Pol{Wait, HD, RD, Release};
					Spawn(0, vec2(100 * 32 + 16, 379 * 32 + 16));
					Spawn(1, vec2(100 * 32 + 16 + Dx, 379 * 32 + 16));
					m_aTee[1].m_Input.m_Jump = 1;
					for(int i = 0; i < 15; i++)
						Step(2);
					m_aTee[1].m_Input.m_Jump = 0;
					SHookflyState St;
					float MinLow = 1e9;
					for(int i = 0; i < 600 && Chr(0) && Chr(1); i++)
					{
						HookflyStep(this, St, Pol);
						Step(2);
						if(i > 100)
							MinLow = std::min(MinLow, 380 * 32 - std::max(Chr(0)->m_Pos.y, Chr(1)->m_Pos.y));
					}
					float H = Chr(0) && Chr(1) ? 380 * 32 - std::max(Chr(0)->m_Pos.y, Chr(1)->m_Pos.y) : 0;
					v.push_back({Pol, Dx, std::min(H, MinLow)});
					Finish(0);
				}
	std::sort(v.begin(), v.end(), [](auto &a, auto &b) { return a.m_Height > b.m_Height; });
	for(int i = 0; i < 8; i++)
		printf("dbg hookfly: lowest point after 2 s %.0f px | wait %.0f hooker dir %d rider dir %d release %d dx %d\n", v[i].m_Height, v[i].m_Pol.m_Wait, v[i].m_Pol.m_HookerDir, v[i].m_Pol.m_RiderDir, v[i].m_Pol.m_Release, v[i].m_Dx);
}

// Vertical 2-tile aled out of a hookfly. The tee that gets launched through the 2-tile freeze ceiling freezes (F);
// the other one (D, below) keeps hooking F or not, jumps, and hammers on its first tick inside the freeze, flies
// through, F hammers D back, and the hookfly continues above.
struct SHkCfg
{
	int m_Ceil;
	int m_Dx; // horizontal spacing at the start
	bool m_Hold; // D keeps hooking the frozen F
	int m_DJump; // ticks after F froze that D jumps (-1 = never)
	int m_BackDelay = 0;
	int m_FJump = -1; // ticks after being freed that F air-jumps (-1 = never); F also hooks D up through
};
struct SHkRes
{
	bool m_Ok = false, m_FFreed = false;
	int m_Depth = 0, m_Past = 0;
};

class SimHookfly : public SimFly
{
public:
	static constexpr int X0 = 240, X1 = 258, FLOOR = 380;
	void Arena(int Ceil)
	{
		FillRect(X0, 5, X1, FLOOR - 1, TILE_AIR);
		FillRect(X0, FLOOR, X1, FLOOR, TILE_SOLID);
		FillRect(X0, Ceil, X1, Ceil + 1, TILE_FREEZE);
	}
	SHkRes Run(const SHkCfg &c, bool Rec)
	{
		SHkRes r;
		const float CeilTop = c.m_Ceil * 32, CeilBot = (c.m_Ceil + 2) * 32;
		const SHookflyPolicy Pol{12, 1, 0, 0};
		Arena(c.m_Ceil);
		Spawn(0, vec2(246 * 32 + 16, (FLOOR - 1) * 32 + 16));
		Spawn(1, vec2(246 * 32 + 16 + c.m_Dx, (FLOOR - 1) * 32 + 16));
		if(Rec)
		{
			Name(0, "A");
			Name(1, "B");
			Caption("Hookfly: the higher tee hooks the lower one up, then they swap.\nAbove them: a 2-tile freeze ceiling.");
		}
		m_aTee[1].m_Input.m_Jump = 1;
		for(int i = 0; i < 15; i++)
			Step(2);
		m_aTee[1].m_Input.m_Jump = 0;
		SHookflyState St;
		int Phase = 0, FT = -1, F = -1, D = -1, Fired = -1, FirstValid = -1, Back = -1, Resume = -1, FreedAt = -1;
		for(int i = 0; i < 1000 && Chr(0) && Chr(1); i++)
		{
			if(Phase == 0)
			{
				HookflyStep(this, St, Pol);
				for(int t = 0; t < 2; t++)
					if(Chr(t)->m_FreezeTime > 0)
					{
						F = t;
						D = 1 - t;
						Phase = 1;
						FT = i;
					}
				if(Phase == 1)
				{
					if(Chr(D)->m_Pos.y < Chr(F)->m_Pos.y)
						break; // the one below must dive
					if(Rec)
						Caption("One tee is flung through the freeze (frozen). The other keeps hooking it and jumps in behind...");
				}
			}
			if(Phase == 1)
			{
				CCharacter *pD = Chr(D), *pF = Chr(F);
				m_aTee[F].m_Input = {};
				Aim(D, normalize(pF->m_Pos - pD->m_Pos));
				m_aTee[D].m_Input.m_Direction = 0;
				m_aTee[D].m_Input.m_Hook = c.m_Hold;
				m_aTee[D].m_Input.m_Jump = c.m_DJump >= 0 && i >= FT + c.m_DJump;
				if(pD->m_Pos.y < CeilBot && pD->m_Pos.y >= CeilTop && pD->m_FreezeTime == 0)
				{
					r.m_Depth = (int)(CeilBot - pD->m_Pos.y);
					r.m_Past = (int)(CeilTop - pF->m_Pos.y);
					PressFire(D);
					Fired = i;
					Phase = 2;
					if(Rec)
					{
						char aBuf[160];
						str_format(aBuf, sizeof(aBuf), "...and hammers on its first tick inside: %d px into the freeze, partner %d px above it.", r.m_Depth, r.m_Past);
						Caption(aBuf);
					}
				}
				if(pD->m_FreezeTime > 0 || i > FT + 90)
					break;
			}
			else if(Phase == 2)
			{
				CCharacter *pD = Chr(D), *pF = Chr(F);
				m_aTee[D].m_Input = {};
				m_aTee[D].m_Input.m_Fire = i > Fired ? 2 : 1;
				if(pF->m_FreezeTime == 0 && !r.m_FFreed)
				{
					r.m_FFreed = true;
					FreedAt = i;
				}
				if(r.m_FFreed && Back < 0)
				{
					// F hooks D up through the ceiling and may air-jump to stay up
					Aim(F, normalize(pD->m_Pos - pF->m_Pos));
					m_aTee[F].m_Input.m_Hook = pD->m_Pos.y >= CeilTop - 4;
					m_aTee[F].m_Input.m_Jump = c.m_FJump >= 0 && i >= FreedAt + c.m_FJump;
				}
				vec2 DPrev = pD->m_Pos - pD->m_Core.m_Vel;
				bool V = r.m_FFreed && pD->m_FreezeTime > 0 && pD->m_Pos.y < CeilTop && DPrev.y < CeilTop - 1 && distance(pD->m_Pos, pF->m_Pos) < 60;
				if(V && FirstValid < 0)
					FirstValid = i;
				if(Back < 0 && FirstValid >= 0 && i == FirstValid + c.m_BackDelay)
				{
					Back = i;
					Aim(F, normalize(pD->m_Pos - pF->m_Pos));
					PressFire(F);
					m_aTee[F].m_Input.m_Hook = 0;
					if(Rec)
						Caption("The freed tee hooks its partner up through the freeze, hammers it - and the hookfly goes on.");
				}
				else
					ReleaseFire(F);
				if(Back >= 0 && i > Back + 1 && pD->m_FreezeTime == 0)
				{
					Phase = 3;
					Resume = i;
					St.m_Hooker = pF->m_Pos.y < pD->m_Pos.y ? F : D;
					for(auto &t : m_aTee)
						t.m_Input.m_Hook = 0;
				}
				if(i > Fired + 80)
					break;
			}
			else if(Phase == 3)
			{
				HookflyStep(this, St, Pol);
				if(i > Resume + 120)
					break;
			}
			Step(2);
		}
		r.m_Ok = Phase == 3 && Chr(0) && Chr(1) && Chr(0)->m_FreezeTime == 0 && Chr(1)->m_FreezeTime == 0 && Chr(0)->m_Pos.y < CeilTop && Chr(1)->m_Pos.y < CeilTop;
		if(Rec)
			Hold(30);
		for(auto &t : m_aTee)
			t.m_Input = {};
		return Finish(r);
	}
};

TEST_F(SimHookfly, Search)
{
	struct SHit
	{
		SHkCfg m_Cfg;
		SHkRes m_Res;
		int m_Window = 0;
	};
	std::vector<SHit> vHits;
	long Sims = 0, Freed = 0;
	for(int Ceil = FLOOR - 70; Ceil <= FLOOR - 10; Ceil++)
		for(int Dx : {40, 60, 80})
			for(int Hold = 0; Hold <= 1; Hold++)
				for(int DJump = -1; DJump <= 30; DJump++)
					for(int FJump : {-1, 0, 4, 8, 12})
				{
					SHkCfg c{Ceil, Dx, (bool)Hold, DJump, 0, FJump};
					SHkRes r = Run(c, false);
					Sims++;
					Freed += r.m_FFreed;
					if(r.m_Ok)
						vHits.push_back({c, r});
				}
	printf("hookfly 2-tile aled: %ld sims, partner freed in %ld, full success %zu\n", Sims, Freed, vHits.size());
	ASSERT_FALSE(vHits.empty());
	for(auto &h : vHits)
		for(int k = 0; k < 15; k++)
		{
			SHkCfg c = h.m_Cfg;
			c.m_BackDelay = k;
			h.m_Window += Run(c, false).m_Ok;
		}
	std::sort(vHits.begin(), vHits.end(), [](auto &a, auto &b) { return a.m_Window != b.m_Window ? a.m_Window > b.m_Window : a.m_Cfg.m_Ceil > b.m_Cfg.m_Ceil; });
	int NHold = 0;
	for(auto &h : vHits)
		NHold += h.m_Cfg.m_Hold;
	printf("  %d of them keep hooking the frozen partner\n", NHold);
	for(size_t i = 0; i < std::min<size_t>(10, vHits.size()); i++)
	{
		auto &h = vHits[i];
		printf("  back window %2d | ceiling %d tiles up, dx %d, %s, jump %d ticks after the freeze, freed tee jumps @%d | %d px in, partner %d px above\n", h.m_Window,
			FLOOR - h.m_Cfg.m_Ceil, h.m_Cfg.m_Dx, h.m_Cfg.m_Hold ? "keeps hooking" : "lets go", h.m_Cfg.m_DJump, h.m_Cfg.m_FJump, h.m_Res.m_Depth, h.m_Res.m_Past);
	}
	SimMap::gvExtraRects.clear();
	SHkCfg Show = vHits[0].m_Cfg;
	Show.m_BackDelay = std::min(2, vHits[0].m_Window - 1);
	SimMap::gvExtraRects.push_back({X0, FLOOR, X1, FLOOR, TILE_SOLID});
	SimMap::gvExtraRects.push_back({X0, Show.m_Ceil, X1, Show.m_Ceil + 1, TILE_FREEZE});
	UseExtendedMapForDemo();
	StartRec("sim13_hookfly_aled");
	EXPECT_TRUE(Run(Show, true).m_Ok);
	StopRec();
}

TEST_F(SimFly, HorizontalHookflySearch)
{
	UseBigMap(Col(), 600, 400, &GameServer()->m_World);
	FillRect(0, 380, 599, 380, TILE_SOLID);
	struct SRes
	{
		SHookflyPolicy m_Pol;
		int m_Dx;
		float m_Dist, m_MinLow, m_Rise;
	};
	std::vector<SRes> v;
	for(float Wait : {4.f, 8.f, 12.f})
		for(int Release : {-40, 0, 20})
			for(int MoveDelay : {0, 2, 4, 6, 8, 12, 16})
				for(int RiderFlight = 0; RiderFlight <= 1; RiderFlight++)
					for(int Dx : {40, 80})
					{
						SHookflyPolicy Pol{Wait, 0, 0, Release, 1, MoveDelay, RiderFlight};
						// the tee in the flight direction jumps and hooks first
						Spawn(0, vec2(100 * 32 + 16, 379 * 32 + 16));
						Spawn(1, vec2(100 * 32 + 16 + Dx, 379 * 32 + 16));
						m_aTee[1].m_Input.m_Jump = 1;
						for(int i = 0; i < 15; i++)
							Step(2);
						m_aTee[1].m_Input.m_Jump = 0;
						SHookflyState St;
						float MinLow = 1e9;
						for(int i = 0; i < 600 && Chr(0) && Chr(1); i++)
						{
							HookflyStep(this, St, Pol);
							Step(2);
							if(i > 100)
								MinLow = std::min(MinLow, 380 * 32 - std::max(Chr(0)->m_Pos.y, Chr(1)->m_Pos.y));
						}
						if(Chr(0) && Chr(1))
							v.push_back({Pol, Dx, (Chr(0)->m_Pos.x + Chr(1)->m_Pos.x) / 2 - 100 * 32, MinLow, 380 * 32 - (Chr(0)->m_Pos.y + Chr(1)->m_Pos.y) / 2});
						Finish(0);
					}
	// airborne the whole time, most horizontal distance
	std::sort(v.begin(), v.end(), [](auto &a, auto &b) {
		bool Fa = a.m_MinLow > 60, Fb = b.m_MinLow > 60;
		return Fa != Fb ? Fa : a.m_Dist > b.m_Dist;
	});
	for(int i = 0; i < 8; i++)
		printf("dbg horizontal hookfly: %.0f px right in 12 s, lowest %.0f px, now %.0f px up | wait %.0f release %d movedelay %d riderflight %d dx %d\n", v[i].m_Dist, v[i].m_MinLow, v[i].m_Rise,
			v[i].m_Pol.m_Wait, v[i].m_Pol.m_Release, v[i].m_Pol.m_MoveDelay, v[i].m_Pol.m_RiderFlight, v[i].m_Dx);
}

// Horizontal 2-tile aled out of a horizontal hookfly: hookfly right into a tall 2-tile freeze wall. The front tee
// freezes (F); D (behind) keeps hooking it or not, steers right, maybe jumps, and hammers on its first tick inside the
// wall; F hooks D through, hammers it back, and the hookfly continues on the far side.
struct SHzCfg
{
	int m_Wall; // wall left column
	int m_StartOff; // px added to both start positions (tick phase)
	bool m_Hold;
	int m_DJump;
	int m_FDir; // freed F's direction while bringing D through
	int m_FJump;
	int m_BackDelay = 0;
	int m_FlingDist = -1; // fling mode starts this many px before the wall (-1 = off)
	int m_HookDelay = 0; // D (re)hooks the frozen F only this many ticks after F froze
	int m_FloorHook = -1; // if >= 0: instead of F, D hooks the floor this many px past the wall to gain speed
	bool m_FHook = true; // freed F hooks D through the wall
};
struct SHzRes
{
	bool m_Ok = false, m_FFreed = false;
	int m_Depth = 0, m_Past = 0, m_Dy = 0;
	int m_AirJumps = 0; // air ("double") jumps the engine registered for either tee
};

static long gHz[6];
class SimHookflyH : public SimFly
{
public:
	static constexpr int X0 = 145, X1 = 259, FLOOR = 280, TOP = 175;
	void Arena(int Wall)
	{
		FillRect(X0, TOP, X1, FLOOR - 1, TILE_AIR);
		FillRect(X0, FLOOR, X1, FLOOR, TILE_SOLID);
		FillRect(Wall, TOP, Wall + 1, FLOOR - 1, TILE_FREEZE);
	}
	SHzRes Run(const SHzCfg &c, bool Rec)
	{
		SHzRes r;
		const float WallL = c.m_Wall * 32, WallR = (c.m_Wall + 2) * 32;
		SHookflyPolicy Pol{12, 0, 0, -40, 1, 2, 0};
		Pol.m_FlingX = c.m_FlingDist >= 0 ? (int)WallL - c.m_FlingDist : -1;
		Arena(c.m_Wall);
		const float SX = 150 * 32 + 16 + c.m_StartOff;
		Spawn(0, vec2(SX, (FLOOR - 1) * 32 + 16));
		Spawn(1, vec2(SX + 40, (FLOOR - 1) * 32 + 16));
		if(Rec)
		{
			Name(0, "A");
			Name(1, "B");
			Caption("Horizontal hookfly: whoever is on top hooks the other and flies right.\nAhead: a 2-tile freeze wall.");
		}
		m_aTee[1].m_Input.m_Jump = 1;
		for(int i = 0; i < 15; i++)
			Step(2);
		m_aTee[1].m_Input.m_Jump = 0;
		SHookflyState St;
		int Phase = 0, FT = -1, F = -1, D = -1, Fired = -1, FirstValid = -1, Back = -1, Resume = -1, FreedAt = -1;
		bool FExited = false;
		for(int i = 0; i < 1200 && Chr(0) && Chr(1); i++)
		{
			if(Phase == 0)
			{
				HookflyStep(this, St, Pol);
				for(int t = 0; t < 2; t++)
					if(Chr(t)->m_FreezeTime > 0)
					{
						F = t;
						D = 1 - t;
						Phase = 1;
						FT = i;
					}
				if(Phase == 1)
				{
					if(Chr(D)->m_Pos.x >= WallL || Chr(D)->m_FreezeTime > 0)
						break;
					if(Rec)
						Caption(c.m_FloorHook >= 0 ? "One tee is flung into the freeze. The other hooks the floor BEHIND the wall (hooks pass through freeze) to dive in fast..." :
									     "One tee is flung into the freeze. The other keeps hooking it and flies in behind...");
				}
				if(Phase == 0 && Chr(0)->m_Pos.x > WallR + 200)
					break; // flew past without anyone freezing?
			}
			if(Phase == 1)
			{
				CCharacter *pD = Chr(D), *pF = Chr(F);
				m_aTee[F].m_Input = {};
				m_aTee[D].m_Input.m_Direction = 1;
				if(c.m_FloorHook >= 0)
				{
					// hook through the freeze wall into the floor behind it
					if(!(m_aTee[D].m_Input.m_Hook))
						Aim(D, normalize(vec2(WallR + c.m_FloorHook, FLOOR * 32) - pD->m_Pos));
					m_aTee[D].m_Input.m_Hook = i >= FT + c.m_HookDelay;
				}
				else
				{
					Aim(D, normalize(pF->m_Pos - pD->m_Pos));
					m_aTee[D].m_Input.m_Hook = c.m_Hold && i >= FT + c.m_HookDelay;
				}
				if(pF->m_Pos.x >= WallR && !FExited)
					FExited = true;
				m_aTee[D].m_Input.m_Jump = c.m_DJump >= 0 && i >= FT + c.m_DJump;
				if(pD->m_Pos.x >= WallL && pD->m_FreezeTime == 0)
				{
					r.m_Depth = (int)(pD->m_Pos.x - WallL);
					r.m_Past = (int)(pF->m_Pos.x - WallR);
					r.m_Dy = (int)(pF->m_Pos.y - pD->m_Pos.y);
					Aim(D, normalize(pF->m_Pos - pD->m_Pos));
					PressFire(D);
					Fired = i;
					Phase = 2;
					if(Rec)
					{
						char aBuf[160];
						str_format(aBuf, sizeof(aBuf), "...and hammers on its first tick inside the wall: %d px in, partner %d px past it.", r.m_Depth, r.m_Past);
						Caption(aBuf);
					}
				}
				if(pD->m_FreezeTime > 0 || i > FT + 90)
					break;
			}
			else if(Phase == 2)
			{
				CCharacter *pD = Chr(D), *pF = Chr(F);
				m_aTee[D].m_Input = {};
				m_aTee[D].m_Input.m_Fire = i > Fired ? 2 : 1;
				if(pF->m_FreezeTime == 0 && !r.m_FFreed)
				{
					r.m_FFreed = true;
					FreedAt = i;
				}
				if(getenv("SIM_TRACE") && r.m_FFreed)
					printf("dbg t+%2d D=(%+.0f vs wall, %.0f) v(%.1f,%.1f) frz=%d | F=(%+.0f past, %.0f) v(%.1f,%.1f) frz=%d | dist %.0f\n", i - Fired, pD->m_Pos.x - WallL, pD->m_Pos.y, pD->m_Core.m_Vel.x, pD->m_Core.m_Vel.y, pD->m_FreezeTime > 0,
						pF->m_Pos.x - WallR, pF->m_Pos.y, pF->m_Core.m_Vel.x, pF->m_Core.m_Vel.y, pF->m_FreezeTime > 0, distance(pD->m_Pos, pF->m_Pos));
				if(r.m_FFreed && Back < 0)
				{
					Aim(F, normalize(pD->m_Pos - pF->m_Pos));
					m_aTee[F].m_Input.m_Hook = c.m_FHook && pD->m_Pos.x < WallR + 4;
					m_aTee[F].m_Input.m_Direction = c.m_FDir;
					m_aTee[F].m_Input.m_Jump = c.m_FJump >= 0 && i >= FreedAt + c.m_FJump;
				}
				vec2 DPrev = pD->m_Pos - pD->m_Core.m_Vel;
				bool V = r.m_FFreed && pD->m_FreezeTime > 0 && pD->m_Pos.x >= WallR && DPrev.x >= WallR + 1 && distance(pD->m_Pos, pF->m_Pos) < 60;
				if(V && FirstValid < 0)
					FirstValid = i;
				if(Back < 0 && FirstValid >= 0 && i == FirstValid + c.m_BackDelay)
				{
					Back = i;
					Aim(F, normalize(pD->m_Pos - pF->m_Pos));
					PressFire(F);
					m_aTee[F].m_Input.m_Hook = 0;
					if(Rec)
						Caption(c.m_FHook ? "The freed tee hooks its partner through the wall and hammers it. Both through." :
								    "The diver flies through frozen and the freed tee hammers it back. Both through.");
				}
				else
					ReleaseFire(F);
				if(Back >= 0 && i > Back + 1 && pD->m_FreezeTime == 0)
				{
					Phase = 3;
					Resume = i;
					St.m_Hooker = pF->m_Pos.y < pD->m_Pos.y ? F : D;
					St.m_HookStart = St.m_Tick;
					for(auto &t : m_aTee)
						t.m_Input.m_Hook = 0;
				}
				if(i > Fired + 90)
					break;
			}
			else if(Phase == 3)
			{
				// restart the hookfly like at the start: the tee in front jumps first (only off the ground), then they hook each other again
				int Front = Chr(0)->m_Pos.x > Chr(1)->m_Pos.x ? 0 : 1;
				if(i < Resume + 16)
				{
					for(auto &t : m_aTee)
						t.m_Input = {};
					m_aTee[Front].m_Input.m_Jump = Chr(Front)->IsGrounded();
					St.m_Hooker = Front;
					St.m_HookStart = St.m_Tick;
				}
				else
					HookflyStep(this, St, Pol);
				if(i > Resume + (Rec ? 40 : 120))
					break;
			}
			Step(2);
			for(int t = 0; t < 2; t++)
				if(Chr(t) && (Chr(t)->m_Core.m_TriggeredEvents & COREEVENT_AIR_JUMP))
					r.m_AirJumps++;
		}
		gHz[0]++;
		gHz[1] += FT >= 0;
		gHz[2] += Fired >= 0;
		gHz[3] += r.m_FFreed;
		gHz[4] += FExited;
		if(getenv("SIM_DBG") && Fired >= 0 && r.m_Past >= 0 && (gHz[5]++ % 40) == 0)
			printf("dbg fired with F through: depth %d, F past %d, dy %d (F froze at %d, fired at %d)\n", r.m_Depth, r.m_Past, r.m_Dy, FT, Fired);
		r.m_Ok = Phase == 3 && Chr(0) && Chr(1) && Chr(0)->m_FreezeTime == 0 && Chr(1)->m_FreezeTime == 0 && Chr(0)->m_Pos.x >= WallR && Chr(1)->m_Pos.x >= WallR;
		if(Rec)
			Hold(30);
		for(auto &t : m_aTee)
			t.m_Input = {};
		return Finish(r);
	}
};

TEST_F(SimHookflyH, Search)
{
	struct SHit
	{
		SHzCfg m_Cfg;
		SHzRes m_Res;
		int m_Window = 0;
	};
	std::vector<SHit> vHits;
	long Sims = 0, Freed = 0;
	const int WallStep = getenv("SIM_DBG") ? 6 : 1;
	for(int Wall = 162; Wall <= 200; Wall += WallStep)
		for(int Off = 0; Off < 32; Off += 4)
			for(int Mode = 0; Mode < 5; Mode++) // 0 let go, 1 keep hooking F, 2..4 hook the floor behind the wall
				for(int DJump = -1; DJump <= 24; DJump += 2)
					for(int FlingDist : {40, 100, 200})
						for(int HookDelay : {0, 4, 8})
						{
							if(Mode == 0 && HookDelay)
								continue;
							int FloorHook = Mode == 2 ? 0 : Mode == 3 ? 96 : Mode == 4 ? 200 : -1;
							SHzCfg c{Wall, Off, Mode == 1, DJump, 0, -1, 0, FlingDist, HookDelay, FloorHook};
							SHzRes r = Run(c, false);
							Sims++;
							Freed += r.m_FFreed;
							if(!r.m_FFreed)
								continue;
							// stage 2: how the freed tee brings the diver through
							for(int FDir = -1; FDir <= 1; FDir++)
								for(int FJump : {-1, 0, 4, 8})
									for(int FHook = 0; FHook <= 1; FHook++)
									{
										SHzCfg c2 = c;
										c2.m_FDir = FDir;
										c2.m_FJump = FJump;
										c2.m_FHook = FHook;
										SHzRes r2 = Run(c2, false);
										if(r2.m_Ok)
											vHits.push_back({c2, r2});
									}
						}
	printf("horizontal hookfly 2-tile aled: %ld sims, partner freed in %ld, full success %zu (someone froze %ld, frozen tee got through %ld, D fired %ld)\n", Sims, Freed, vHits.size(), gHz[1], gHz[4], gHz[2]);
	ASSERT_FALSE(vHits.empty());
	for(auto &h : vHits)
		for(int k = 0; k < 15; k++)
		{
			SHzCfg c = h.m_Cfg;
			c.m_BackDelay = k;
			h.m_Window += Run(c, false).m_Ok;
		}
	// comfortable hammer-back first, then the longest hookfly before the wall
	std::sort(vHits.begin(), vHits.end(), [](auto &a, auto &b) {
		int Wa = std::min(a.m_Window, 10), Wb = std::min(b.m_Window, 10);
		return Wa != Wb ? Wa > Wb : a.m_Cfg.m_Wall > b.m_Cfg.m_Wall;
	});
	std::map<int, int> Walls;
	for(auto &h : vHits)
		Walls[h.m_Cfg.m_Wall]++;
	printf("  walls that work:");
	for(auto &[w, n] : Walls)
		printf(" col%d(%d)", w, n);
	printf("\n");
	for(size_t i = 0; i < std::min<size_t>(10, vHits.size()); i++)
	{
		auto &h = vHits[i];
		printf("  back window %2d | wall col %d, start +%d, fling %d, D: %s delay %d jump %d | F dir %d jump %d hook %d | %d px in, partner %d past, dy %d\n", h.m_Window, h.m_Cfg.m_Wall,
			h.m_Cfg.m_StartOff, h.m_Cfg.m_FlingDist, h.m_Cfg.m_FloorHook >= 0 ? "hooks floor" : h.m_Cfg.m_Hold ? "hooks partner" : "no hook", h.m_Cfg.m_HookDelay, h.m_Cfg.m_DJump, h.m_Cfg.m_FDir, h.m_Cfg.m_FJump,
			h.m_Cfg.m_FHook, h.m_Res.m_Depth, h.m_Res.m_Past, h.m_Res.m_Dy);
	}
	SimMap::gvExtraRects.clear();
	SHzCfg Show = vHits[0].m_Cfg;
	Show.m_BackDelay = std::min(2, vHits[0].m_Window - 1);
	SimMap::gvExtraRects.push_back({X0, FLOOR, X1, FLOOR, TILE_SOLID});
	SimMap::gvExtraRects.push_back({Show.m_Wall, TOP, Show.m_Wall + 1, FLOOR - 1, TILE_FREEZE});
	UseExtendedMapForDemo();
	StartRec("sim14_hookfly_horizontal_aled");
	EXPECT_TRUE(Run(Show, true).m_Ok);
	StopRec();
}

// the same move without any double (air) jump: only the ground jump that starts the hookfly
TEST_F(SimHookflyH, NoDoubleJump)
{
	struct SHit
	{
		SHzCfg m_Cfg;
		SHzRes m_Res;
		int m_Window = 0;
	};
	std::vector<SHit> vHits;
	long Sims = 0, Freed = 0;
	for(int Wall = 162; Wall <= 200; Wall++)
		for(int Off = 0; Off < 32; Off += 2)
			for(int Mode = 0; Mode < 5; Mode++)
				for(int FlingDist : {20, 40, 70, 100, 150, 200})
					for(int HookDelay : {0, 2, 4, 6, 8})
					{
						if(Mode == 0 && HookDelay)
							continue;
						int FloorHook = Mode == 2 ? 0 : Mode == 3 ? 96 : Mode == 4 ? 200 : -1;
						SHzCfg c{Wall, Off, Mode == 1, -1, 0, -1, 0, FlingDist, HookDelay, FloorHook};
						SHzRes r = Run(c, false);
						Sims++;
						Freed += r.m_FFreed;
						if(!r.m_FFreed)
							continue;
						for(int FDir = -1; FDir <= 1; FDir++)
							for(int FHook = 0; FHook <= 1; FHook++)
							{
								SHzCfg c2 = c;
								c2.m_FDir = FDir;
								c2.m_FHook = FHook;
								SHzRes r2 = Run(c2, false);
								if(r2.m_Ok && r2.m_AirJumps == 0)
									vHits.push_back({c2, r2});
							}
					}
	printf("no double jump: %ld sims, partner freed in %ld, complete without any air jump %zu\n", Sims, Freed, vHits.size());
	ASSERT_FALSE(vHits.empty());
	for(auto &h : vHits)
		for(int k = 0; k < 15; k++)
		{
			SHzCfg c = h.m_Cfg;
			c.m_BackDelay = k;
			SHzRes r = Run(c, false);
			h.m_Window += r.m_Ok && r.m_AirJumps == 0;
		}
	std::sort(vHits.begin(), vHits.end(), [](auto &a, auto &b) {
		int Wa = std::min(a.m_Window, 10), Wb = std::min(b.m_Window, 10);
		return Wa != Wb ? Wa > Wb : a.m_Cfg.m_Wall > b.m_Cfg.m_Wall;
	});
	for(size_t i = 0; i < std::min<size_t>(8, vHits.size()); i++)
	{
		auto &h = vHits[i];
		printf("  back window %2d | wall col %d, start +%d, fling %d, D: %s delay %d | F dir %d hook %d | %d px in, partner %d past, dy %d\n", h.m_Window, h.m_Cfg.m_Wall, h.m_Cfg.m_StartOff,
			h.m_Cfg.m_FlingDist, h.m_Cfg.m_FloorHook >= 0 ? "hooks floor" : h.m_Cfg.m_Hold ? "hooks partner" : "no hook", h.m_Cfg.m_HookDelay, h.m_Cfg.m_FDir, h.m_Cfg.m_FHook,
			h.m_Res.m_Depth, h.m_Res.m_Past, h.m_Res.m_Dy);
	}
	SimMap::gvExtraRects.clear();
	SHzCfg Show = vHits[0].m_Cfg;
	Show.m_BackDelay = std::min(2, vHits[0].m_Window - 1);
	SimMap::gvExtraRects.push_back({X0, FLOOR, X1, FLOOR, TILE_SOLID});
	SimMap::gvExtraRects.push_back({Show.m_Wall, TOP, Show.m_Wall + 1, FLOOR - 1, TILE_FREEZE});
	UseExtendedMapForDemo();
	StartRec("sim15_hookfly_horizontal_aled_nodj");
	SHzRes r = Run(Show, true);
	StopRec();
	printf("  recorded: ok %d, air jumps %d\n", r.m_Ok, r.m_AirJumps);
	EXPECT_TRUE(r.m_Ok);
	EXPECT_EQ(r.m_AirJumps, 0);
}

// ---- TAS replay on the real server ----
// TAS_MAP=<map file> TAS_INPUTS=<inputs file> [TAS_TRACE=1] [TAS_DEMO=<name>]
// inputs: one line per tick "dir jump hook fire tx ty weapon" (fire = held, weapon 0-based or -1)
static void TasPrepareMap(IStorage *pStorage)
{
	const char *pMap = getenv("TAS_MAP");
	ASSERT_TRUE(pMap);
	FILE *f = fopen(pMap, "rb");
	ASSERT_TRUE(f);
	std::vector<unsigned char> vData;
	unsigned char aBuf[4096];
	size_t n;
	while((n = fread(aBuf, 1, sizeof(aBuf), f)) > 0)
		vData.insert(vData.end(), aBuf, aBuf + n);
	fclose(f);
	pStorage->CreateFolder("maps", IStorage::TYPE_SAVE);
	IOHANDLE File = pStorage->OpenFile("maps/tasmap.map", IOFLAG_WRITE, IStorage::TYPE_SAVE);
	ASSERT_TRUE(File);
	io_write(File, vData.data(), vData.size());
	io_close(File);
}

class TasReplay : public SimDemo // NOLINT(readability-identifier-naming)
{
public:
	static void SetUpTestSuite()
	{
		CNetBase::Init();
		gpSimMapName = "tasmap";
		gpfnPrepareMap = TasPrepareMap;
	}
	static void TearDownTestSuite()
	{
		gpSimMapName = "coverage";
		gpfnPrepareMap = nullptr;
	}
};

TEST_F(TasReplay, Run)
{
	const char *pInputs = getenv("TAS_INPUTS");
	ASSERT_TRUE(pInputs);
	FILE *f = fopen(pInputs, "r");
	ASSERT_TRUE(f);
	std::vector<std::array<int, 7>> vIn;
	std::array<int, 7> a;
	while(fscanf(f, "%d %d %d %d %d %d %d", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6]) == 7)
		vIn.push_back(a);
	fclose(f);
	const bool Trace = getenv("TAS_TRACE") != nullptr;
	const char *pDemo = getenv("TAS_DEMO");

	// the first spawn point in map order is where a lone player spawns
	vec2 SpawnPos = GameServer()->m_pController->m_avSpawnPoints[0][0];
	printf("spawn %.1f %.1f\n", SpawnPos.x, SpawnPos.y);
	CPlayer *p = GameServer()->m_apPlayers[CidOf(0)];
	{
		CLogScope Quiet(&gNullLogger);
		p->KillCharacter(WEAPON_GAME, false);
		GameServer()->m_World.RemoveEntities();
		RecycleSnapIds();
		p->ForceSpawn(SpawnPos);
	}
	// keep the other debug dummies out of the way: spectators never respawn (a killed player respawns at a free spawn
	// point, which on the KoG map version is right in the spawn room's flight path and can be hooked)
	for(int t = 1; t < 3; t++)
	{
		Kill(t);
		CLogScope Quiet(&gNullLogger);
		GameServer()->m_apPlayers[CidOf(t)]->SetTeam(TEAM_SPECTATORS, false);
	}
	m_aTee[0].m_Cid = CidOf(0);
	mem_zero(&m_aTee[0].m_Input, sizeof(CNetObj_PlayerInput));
	if(pDemo)
	{
		Name(0, "c29");
		StartRec(pDemo);
	}
	int Fire = 0;
	int StartTick = -1, FinishTick = -1;
	bool DoubleStart = false;
	int Tick0 = m_pServer->Tick();
	for(size_t i = 0; i < vIn.size(); i++)
	{
		CNetObj_PlayerInput &In = m_aTee[0].m_Input;
		In.m_Direction = vIn[i][0];
		In.m_Jump = vIn[i][1];
		In.m_Hook = vIn[i][2];
		if((Fire & 1) != (vIn[i][3] ? 1 : 0))
			Fire++;
		In.m_Fire = Fire;
		In.m_TargetX = vIn[i][4];
		In.m_TargetY = vIn[i][5];
		In.m_WantedWeapon = vIn[i][6] >= 0 ? vIn[i][6] + 1 : 0;
		In.m_PlayerFlags = PLAYERFLAG_PLAYING;
		Step(1);
		CCharacter *c = Chr(0);
		if(!c)
		{
			printf("tee died at tick %d\n", (int)i + 1);
			break;
		}
		if(c->m_DDRaceState == ERaceState::STARTED)
		{
			// the timer restarts every tick the tee touches the start line; that is only
			// double start abuse when it comes back after having left the line
			int Now = c->m_StartTime - Tick0;
			if(StartTick >= 0 && Now != StartTick && Now - StartTick > 1)
			{
				printf("DOUBLE START: timer restarted at tick %d (previous start %d)\n", Now, StartTick);
				DoubleStart = true;
			}
			StartTick = Now;
		}
		if(FinishTick < 0 && c->m_DDRaceState == ERaceState::FINISHED)
		{
			FinishTick = m_pServer->Tick() - Tick0;
			StartTick = c->m_StartTime - Tick0;
		}
		if(Trace)
			printf("%d %.3f %.3f %.3f %.3f frz=%d dir=%d/%d hook=%d hs=%d jumped=%d\n", (int)i + 1, c->m_Core.m_Pos.x, c->m_Core.m_Pos.y, c->m_Core.m_Vel.x, c->m_Core.m_Vel.y, c->m_FreezeTime > 0,
				c->m_Input.m_Direction, c->m_Core.m_Direction, c->m_Input.m_Hook, c->m_Core.m_HookState, c->m_Core.m_Jumped);
	}
	if(pDemo)
	{
		Hold(100);
		StopRec();
	}
	printf("start tick %d finish tick %d -> %d ticks = %.2f s%s\n", StartTick, FinishTick, FinishTick - StartTick, (FinishTick - StartTick) / 50.0, DoubleStart ? " (INVALID: double start)" : "");
	EXPECT_FALSE(DoubleStart);
	EXPECT_GE(FinishTick, 0);
}

// two runs in one demo: TAS_INPUTS (tee 0) and TAS_INPUTS2 (tee 1), both solo so they never touch each other
// (no collision, hooking or explosions between them). TAS_NAME/TAS_NAME2 name the tees, TAS_DEMO records,
// TAS_DELAY/TAS_DELAY2 hold a tee's spawn back.
TEST_F(TasReplay, Two)
{
	const char *apInputs[2] = {getenv("TAS_INPUTS"), getenv("TAS_INPUTS2")};
	const char *apNames[2] = {getenv("TAS_NAME") ? getenv("TAS_NAME") : "run1", getenv("TAS_NAME2") ? getenv("TAS_NAME2") : "run2"};
	std::vector<std::array<int, 7>> avIn[2];
	for(int t = 0; t < 2; t++)
	{
		ASSERT_TRUE(apInputs[t]);
		FILE *f = fopen(apInputs[t], "r");
		ASSERT_TRUE(f);
		std::array<int, 7> a;
		while(fscanf(f, "%d %d %d %d %d %d %d", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6]) == 7)
			avIn[t].push_back(a);
		fclose(f);
	}
	const char *pDemo = getenv("TAS_DEMO");

	vec2 SpawnPos = GameServer()->m_pController->m_avSpawnPoints[0][0];
	{
		CLogScope Quiet(&gNullLogger);
		for(int t = 0; t < 3; t++)
			GameServer()->m_apPlayers[CidOf(t)]->KillCharacter(WEAPON_GAME, false);
		GameServer()->m_World.RemoveEntities();
		RecycleSnapIds();
		for(int t = 0; t < 2; t++)
		{
			m_aTee[t].m_Cid = CidOf(t);
			mem_zero(&m_aTee[t].m_Input, sizeof(CNetObj_PlayerInput));
		}
	}
	// TAS_DELAY/TAS_DELAY2: spawn that tee this many ticks later (to line both runs up at the start line)
	const int aDelay[2] = {getenv("TAS_DELAY") ? atoi(getenv("TAS_DELAY")) : 0, getenv("TAS_DELAY2") ? atoi(getenv("TAS_DELAY2")) : 0};
	auto SpawnTee = [&](int t) {
		CLogScope Quiet(&gNullLogger);
		CPlayer *p = GameServer()->m_apPlayers[CidOf(t)];
		if(p->GetTeam() == TEAM_SPECTATORS)
			p->SetTeam(TEAM_RED, false);
		p->ForceSpawn(SpawnPos)->SetSolo(true);
	};
	GameServer()->m_apPlayers[CidOf(2)]->SetTeam(TEAM_SPECTATORS, false); // the unused dummy would respawn too
	for(int t = 0; t < 2; t++)
		if(aDelay[t] == 0)
			SpawnTee(t);
		else // a spectator until then, so the server does not respawn it on its own
			GameServer()->m_apPlayers[CidOf(t)]->SetTeam(TEAM_SPECTATORS, false);
	for(int t = 0; t < 2; t++)
		Name(t, apNames[t]);
	if(pDemo)
		StartRec(pDemo);
	int aFire[2] = {0, 0}, aStart[2] = {-1, -1}, aFinish[2] = {-1, -1};
	const int Tick0 = m_pServer->Tick();
	const size_t Len = std::max(avIn[0].size() + aDelay[0], avIn[1].size() + aDelay[1]);
	for(size_t Tick = 0; Tick < Len; Tick++)
	{
		for(int t = 0; t < 2; t++)
		{
			if((int)Tick == aDelay[t] && aDelay[t] > 0)
			{
				ASSERT_FALSE(Chr(t));
				SpawnTee(t);
			}
			if((int)Tick < aDelay[t])
				continue;
			const size_t i = Tick - aDelay[t];
			CNetObj_PlayerInput &In = m_aTee[t].m_Input;
			if(i >= avIn[t].size())
			{
				// out of inputs: let go of everything, keep the aim
				In.m_Direction = 0;
				In.m_Jump = 0;
				In.m_Hook = 0;
				if(aFire[t] & 1)
					aFire[t]++;
				In.m_Fire = aFire[t];
				continue;
			}
			const auto &v = avIn[t][i];
			In.m_Direction = v[0];
			In.m_Jump = v[1];
			In.m_Hook = v[2];
			if((aFire[t] & 1) != (v[3] ? 1 : 0))
				aFire[t]++;
			In.m_Fire = aFire[t];
			In.m_TargetX = v[4];
			In.m_TargetY = v[5];
			In.m_WantedWeapon = v[6] >= 0 ? v[6] + 1 : 0;
			In.m_PlayerFlags = PLAYERFLAG_PLAYING;
		}
		Step(2);
		for(int t = 0; t < 2; t++)
		{
			CCharacter *c = Chr(t);
			if(!c)
				continue;
			if(aFinish[t] < 0 && c->m_DDRaceState == ERaceState::STARTED)
				aStart[t] = c->m_StartTime - Tick0;
			if(aFinish[t] < 0 && c->m_DDRaceState == ERaceState::FINISHED)
			{
				aFinish[t] = m_pServer->Tick() - Tick0;
				aStart[t] = c->m_StartTime - Tick0;
			}
		}
	}
	if(pDemo)
	{
		Hold(100);
		StopRec();
	}
	for(int t = 0; t < 2; t++)
	{
		printf("%s: spawn tick %d start tick %d finish tick %d -> %d ticks = %.2f s\n", apNames[t], aDelay[t], aStart[t], aFinish[t], aFinish[t] - aStart[t], (aFinish[t] - aStart[t]) / 50.0);
		EXPECT_GE(aFinish[t], 0);
	}
}

// ---- pseudofly (docs/PHYSICS-NOTES.md): the upper tee (driver D) hooks the lower one (hammerer H), H hammers D up
// whenever its reload allows. D lets go around its apex and hooks again after the kick; D steers left/right, H mostly
// decides how high. Tee 0 = H, tee 1 = D.
struct SPseudoPolicy
{
	// driver: hold the hook while the rope is long, let go when the hammerer has come close (the rope hardly pulls
	// there, and it resets the 1.2 s hook timeout), hook again once the kick opens the gap
	int m_Dir = 0; // where D flies (-1, 0, 1)
	float m_MaxVx = 1000, m_MaxBehind = 1000; // D stops pushing once this fast / H lags this far behind
	float m_ReleaseDist = 50; // let go once H is closer than this and still closing in
	float m_RehookDist = 50; // hook again once H is farther than this
	int m_MaxFree = 8; // hook again anyway after this many ticks
	// hammerer: hammer at the closest point (or before D leaves reach)
	float m_FireDist = 0; // also hammer whenever D is closer than this
	float m_MinDy = 10; // D must be at least this much higher
	float m_Offset = 0; // stay this far behind D (against its flight direction)
	float m_Lead = 4; // ticks of velocity difference added to the position error when steering
	int m_Dead = 6; // don't steer within this many px of the target spot
	int m_DelayD = 0, m_DelayH = 0; // reaction delays in ticks
	bool m_AirStart = false; // start already flying (D 50 px above H, both rising) instead of from the floor
	int m_TeeH = 0, m_TeeD = 1; // which tee plays which role
	int m_BotRole = -1; // 0 / 1: the bot (tee 0) plays H / D instead of the script
};

struct SPseudoState
{
	bool m_Free = false; // D has let go and waits for the kick
	int m_FreeTicks = 0;
	struct SSeen
	{
		vec2 m_H, m_D, m_VH, m_VD;
	};
	std::vector<SSeen> m_vSeen;
};

void PseudoflyStep(SimFly *p, SPseudoState &St, const SPseudoPolicy &Pol)
{
	CCharacter *pH = p->Chr(Pol.m_TeeH), *pD = p->Chr(Pol.m_TeeD);
	St.m_vSeen.push_back({pH->m_Pos, pD->m_Pos, pH->m_Core.m_Vel, pD->m_Core.m_Vel});
	for(int Role = 0; Role < 2; Role++)
	{
	// a human sees the state a few ticks late and extrapolates it (without gravity or hook)
	const int Delay = Role ? Pol.m_DelayH : Pol.m_DelayD;
	SPseudoState::SSeen S = St.m_vSeen[std::max<int>(0, (int)St.m_vSeen.size() - 1 - Delay)];
	S.m_D += S.m_VD * Delay;
	S.m_H += S.m_VH * Delay;
	const float Dist = distance(S.m_D, S.m_H), NextDist = distance(S.m_D + S.m_VD, S.m_H + S.m_VH);
	if(Role == 1 - Pol.m_BotRole)
		continue;
	// driver
	if(Role == 0)
	{
		auto &In = p->m_aTee[Pol.m_TeeD].m_Input;
		p->Aim(Pol.m_TeeD, normalize(S.m_H - S.m_D));
		const int State = pD->m_Core.m_HookState;
		const bool Below = S.m_H.y > S.m_D.y + 10;
		if(!St.m_Free && State == HOOK_GRABBED && Dist < Pol.m_ReleaseDist && NextDist < Dist)
		{
			St.m_Free = true;
			St.m_FreeTicks = 0;
		}
		else if(St.m_Free && (++St.m_FreeTicks > Pol.m_MaxFree || (Dist > Pol.m_RehookDist && NextDist > Dist)))
			St.m_Free = false;
		const bool Want = !St.m_Free && Below;
		In.m_Hook = HookBotHookInput(Want, In.m_Hook, State);
		In.m_Direction = Pol.m_Dir;
		if(S.m_VD.x * Pol.m_Dir > Pol.m_MaxVx || (S.m_D.x - S.m_H.x) * Pol.m_Dir > Pol.m_MaxBehind)
			In.m_Direction = 0;
		In.m_Jump = 0;
	}
	// hammerer
	else
	{
		auto &In = p->m_aTee[Pol.m_TeeH].m_Input;
		p->Aim(Pol.m_TeeH, normalize(S.m_D - S.m_H));
		const bool Ready = pH->m_ReloadTimer == 0 && !(In.m_Fire & 1);
		// D stops coming closer, is about to bump into me (player collision below 35 px) or to leave my reach
		const bool Closest = NextDist >= Dist || NextDist < 38;
		if(Ready && Dist < 60 && S.m_H.y - S.m_D.y > Pol.m_MinDy && (Closest || Dist < Pol.m_FireDist || NextDist >= 60))
			p->PressFire(Pol.m_TeeH);
		else
			p->ReleaseFire(Pol.m_TeeH);
		const float Err = (S.m_D.x - Pol.m_Dir * Pol.m_Offset) - S.m_H.x + (S.m_VD.x - S.m_VH.x) * Pol.m_Lead;
		In.m_Direction = Err > Pol.m_Dead ? 1 : Err < -Pol.m_Dead ? -1 : 0;
		In.m_Jump = 0;
	}
	}
}

class SimPseudo : public SimFly
{
public:
	static constexpr int FLOOR = 380, W = 600;
	struct SRes
	{
		bool m_Ok = false; // still flying together at the end
		int m_Ticks = 0; // how long it lasted
		float m_Up = 0, m_Side = 0; // px/s gained (up, in the flight direction)
	};
	// both stand on the floor, D jumps, then the policies take over
	SRes Run(SPseudoPolicy Pol, int Ticks = 500, bool Print = false)
	{
		if(Pol.m_BotRole >= 0)
		{
			// the bot is tee 0
			Pol.m_TeeH = Pol.m_BotRole == 0 ? 0 : 1;
			Pol.m_TeeD = 1 - Pol.m_TeeH;
		}
		const int iH = Pol.m_TeeH, iD = Pol.m_TeeD;
		const float X = (Pol.m_Dir > 0 ? 60 : Pol.m_Dir < 0 ? W - 60 : W / 2) * 32 + 16;
		const float Y = Pol.m_AirStart ? (FLOOR - 40) * 32 : (FLOOR - 1) * 32 + 16;
		Spawn(iH, vec2(X - (Pol.m_Dir ? Pol.m_Dir : 1) * (Pol.m_AirStart ? 10 : 32), Y + (Pol.m_AirStart ? 50 : 0)));
		Spawn(iD, vec2(X, Y));
		SPseudoState St;
		SRes R;
		CHookBot &Bot = GameServer()->m_HookBot;
		if(Pol.m_BotRole >= 0)
		{
			g_Config.m_SvHookbot = 1;
			Bot.OnTick();
			Bot.Start(Pol.m_BotRole == 0 ? CHookBotBrain::MODE_PSEUDOFLY : CHookBotBrain::MODE_PSEUDODRIVE, CidOf(1));
			Bot.m_Brain.m_DriveDir = Pol.m_Dir;
		}
		auto StepAll = [&]() {
			if(Pol.m_BotRole >= 0)
				Bot.GetInput(CidOf(0), &m_aTee[0].m_Input);
			Step(2);
		};
		if(Pol.m_AirStart)
		{
			Chr(iH)->m_Core.m_Vel = vec2(0, -14);
			Chr(iD)->m_Core.m_Vel = vec2(0, -12);
		}
		else if(Pol.m_BotRole != 1)
		{
			// the scripted driver jumps (a driving bot jumps by itself)
			for(int i = 0; i < 5; i++)
				StepAll();
			m_aTee[iD].m_Input.m_Jump = 1;
			StepAll();
		}
		const vec2 Start = Chr(iD)->m_Pos;
		int Lost = 0;
		for(R.m_Ticks = 0; R.m_Ticks < Ticks && Chr(0) && Chr(1); R.m_Ticks++)
		{
			PseudoflyStep(this, St, Pol);
			StepAll();
			CCharacter *pH = Chr(iH), *pD = Chr(iD);
			if(Print && R.m_Ticks % (getenv("PF_EVERY") ? atoi(getenv("PF_EVERY")) : 10) == 0)
				printf("dbg t=%3d D %6.0f %5.0f v %5.1f %5.1f hook %d | H rel %4.0f %4.0f v %5.1f %5.1f reload %2d\n", R.m_Ticks, pD->m_Pos.x - Start.x, Start.y - pD->m_Pos.y,
					pD->m_Core.m_Vel.x, pD->m_Core.m_Vel.y, pD->m_Core.m_HookState, pH->m_Pos.x - pD->m_Pos.x, pH->m_Pos.y - pD->m_Pos.y, pH->m_Core.m_Vel.x,
					pH->m_Core.m_Vel.y, pH->m_ReloadTimer);
			// lost: apart, or back on the floor after the start
			bool Bad = distance(pH->m_Pos, pD->m_Pos) > 200 || (R.m_Ticks > 100 && (pH->IsGrounded() || pD->IsGrounded()));
			Lost = Bad ? Lost + 1 : 0;
			if(Lost > 25)
				break;
		}
		if(Chr(iD))
		{
			R.m_Ok = R.m_Ticks >= Ticks;
			R.m_Up = (Start.y - Chr(iD)->m_Pos.y) * SERVER_TICK_SPEED / std::max(R.m_Ticks, 1);
			R.m_Side = (Chr(iD)->m_Pos.x - Start.x) * (Pol.m_Dir ? Pol.m_Dir : 1) * SERVER_TICK_SPEED / std::max(R.m_Ticks, 1);
		}
		if(Pol.m_BotRole >= 0)
			Bot.Start(CHookBotBrain::MODE_IDLE, -1);
		return R;
	}
	void SetUpMap()
	{
		UseBigMap(Col(), W, 400, &GameServer()->m_World);
		FillRect(0, FLOOR, W - 1, FLOOR, TILE_SOLID);
	}
};

TEST_F(SimPseudo, Trace)
{
	SetUpMap();
	SPseudoPolicy Pol;
	Pol.m_Dir = getenv("PF_DIR") ? atoi(getenv("PF_DIR")) : 0;
	Pol.m_Offset = getenv("PF_OFF") ? atof(getenv("PF_OFF")) : 0;
	Pol.m_AirStart = (getenv("PF_AIR") && atoi(getenv("PF_AIR")));
	if(getenv("PF_REL"))
		Pol.m_ReleaseDist = atof(getenv("PF_REL"));
	if(getenv("PF_FIRE"))
		Pol.m_FireDist = atof(getenv("PF_FIRE"));
	if(getenv("PF_BOT"))
		Pol.m_BotRole = atoi(getenv("PF_BOT"));
	SRes R = Run(Pol, 500, true);
	printf("pseudofly dir %d: ok %d after %d ticks, up %.0f px/s, side %.0f px/s\n", Pol.m_Dir, R.m_Ok, R.m_Ticks, R.m_Up, R.m_Side);
}

// sweeps both players' rules; PF_DIR picks the flight direction, PF_BOT=0/1 whether the bot hammers or drives
// (the other role is a human with a reaction delay of 0, 3 or 6 ticks), PF_AIR starts already flying.
TEST_F(SimPseudo, Search)
{
	SetUpMap();
	const int Dir = getenv("PF_DIR") ? atoi(getenv("PF_DIR")) : 0;
	struct SScored
	{
		SPseudoPolicy m_P;
		int m_Ok;
		float m_Up, m_Side;
	};
	std::vector<SScored> v;
	for(float ReleaseDist : {0.f, 40.f, 50.f, 60.f, 70.f})
		for(float RehookDist : {45.f, 55.f, 70.f})
			for(int MaxFree : {3, 8, 15})
				for(float FireDist : {0.f, 40.f, 50.f})
					for(float MinDy : {10.f, 25.f})
						for(float Offset : Dir ? std::vector<float>{0, 20, 40} : std::vector<float>{0})
							for(float Lead : {0.f, 4.f})
							{
								SPseudoPolicy P;
								P.m_Dir = Dir;
								P.m_ReleaseDist = ReleaseDist;
								P.m_RehookDist = RehookDist;
								P.m_MaxFree = MaxFree;
								P.m_FireDist = FireDist;
								P.m_MinDy = MinDy;
								P.m_Offset = Offset;
								P.m_Lead = Lead;
								P.m_AirStart = (getenv("PF_AIR") && atoi(getenv("PF_AIR")));
								SScored Sc{P, 0, 0, 0};
								for(int Delay : {0, 3, 6})
								{
									// the human's reaction delay; the bot reacts at once
									(getenv("PF_BOT") && atoi(getenv("PF_BOT")) == 1 ? P.m_DelayH : P.m_DelayD) = Delay;
									SRes R = Run(P);
									Sc.m_Ok += R.m_Ok;
									Sc.m_Up += R.m_Ok ? R.m_Up / 3 : 0;
									Sc.m_Side += R.m_Ok ? R.m_Side / 3 : 0;
								}
								v.push_back(Sc);
							}
	auto Score = [&](const SScored &a) { return a.m_Ok * 10000 + a.m_Up + a.m_Side; };
	std::sort(v.begin(), v.end(), [&](auto &a, auto &b) { return Score(a) > Score(b); });
	for(int i = 0; i < 15 && i < (int)v.size(); i++)
	{
		const auto &P = v[i].m_P;
		printf("pf: ok %d/3 up %4.0f side %4.0f px/s | release <%2.0f rehook >%2.0f maxfree %2d | fire <%2.0f mindy %2.0f offset %2.0f lead %.0f\n", v[i].m_Ok, v[i].m_Up, v[i].m_Side,
			P.m_ReleaseDist, P.m_RehookDist, P.m_MaxFree, P.m_FireDist, P.m_MinDy, P.m_Offset, P.m_Lead);
	}
	int Ok3 = 0;
	for(auto &Sc : v)
		Ok3 += Sc.m_Ok == 3;
	printf("pf: %d of %zu policies kept flying with all three delays\n", Ok3, v.size());
}

// the real bot (src/game/hookbot.cpp) in either role, against scripted partners with reaction delays 0-6 ticks and
// the partner's parameters varied around the ones that work
TEST_F(SimPseudo, Bot)
{
	SetUpMap();
	if(getenv("PF_MAXVX"))
		GameServer()->m_HookBot.m_Brain.m_Pseudo.m_MaxVx = atof(getenv("PF_MAXVX"));
	if(getenv("PF_BEHIND"))
		GameServer()->m_HookBot.m_Brain.m_Pseudo.m_MaxBehind = atof(getenv("PF_BEHIND"));
	for(int BotRole = 0; BotRole < 2; BotRole++)
		for(int Dir : {0, 1, -1})
			for(int Air = 0; Air < 2; Air++)
			{
				int Ok = 0, N = 0;
				float Up = 0, Side = 0;
				for(int Delay : {0, 2, 4, 6})
					for(int k = 0; k < 3; k++)
					{
						SPseudoPolicy P;
						P.m_BotRole = BotRole;
						P.m_Dir = Dir;
						P.m_AirStart = Air;
						if(getenv("PF_HUMANCAP"))
							P.m_MaxVx = 12, P.m_MaxBehind = 30;
						(BotRole == 0 ? P.m_DelayD : P.m_DelayH) = Delay;
						// the human driver lets go at different distances; the human hammerer waits more or less
						P.m_ReleaseDist = std::array<float, 3>{0, 40, 60}[k];
						P.m_FireDist = std::array<float, 3>{0, 40, 50}[k];
						SRes R = Run(P);
						Ok += R.m_Ok;
						N++;
						if(R.m_Ok)
							Up += R.m_Up, Side += R.m_Side;
					}
				printf("bot %s, fly %-8s %s: kept flying %2d/%d, avg up %4.0f px/s, avg side %4.0f px/s\n", BotRole ? "drives " : "hammers", Dir < 0 ? "left" : Dir > 0 ? "right" : "up",
					Air ? "from the air   " : "from the ground", Ok, N, Ok ? Up / Ok : 0, Ok ? Side / Ok : 0);
			}
}

// ---- a real map with the bot on both tees (TAS_MAP=<map file>), e.g. Stronghold's opening: fall into the shaft,
// hammerhit right along the freeze floor, 1-tile aled through the wall at x 105 ----
class SimMapBots : public SimDemo
{
public:
	static void SetUpTestSuite()
	{
		CNetBase::Init();
		gpSimMapName = "tasmap";
		gpfnPrepareMap = TasPrepareMap;
	}
	static void TearDownTestSuite()
	{
		gpSimMapName = "coverage";
		gpfnPrepareMap = nullptr;
	}
	CHookBotBrain m_aBrain[2];
	int m_RouteT0 = 0;
	// both brains decide, then one server tick (HH_SOLO: only tee 0 is a bot, tee 1 stands still)
	void BotsStep()
	{
		for(int t = 0; t < (getenv("HH_SOLO") ? 1 : 2); t++)
		{
			CCharacter *pMe = Chr(t), *pOther = Chr(1 - t);
			if(!pMe || !pOther)
				continue;
			m_aTee[t].m_Input = m_aBrain[t].Tick(m_pServer->Tick() + 1, CHookBot::TeeState(pMe), CHookBot::TeeState(pOther), CHookBot::TicksFirst(GameServer(), pMe, pOther));
			m_aTee[t].m_Input.m_PlayerFlags = PLAYERFLAG_PLAYING;
		}
		Step(2);
	}
};

TEST_F(SimMapBots, Hammerhit)
{
	const float X0 = getenv("HH_X0") ? atof(getenv("HH_X0")) : 33, X1 = getenv("HH_X1") ? atof(getenv("HH_X1")) : 30;
	const float Y = getenv("HH_Y") ? atof(getenv("HH_Y")) : 60;
	const int Goal = getenv("HH_GOAL") ? atoi(getenv("HH_GOAL")) : 106; // done once a free tee is past this tile
	const int Seconds = getenv("HH_SECONDS") ? atoi(getenv("HH_SECONDS")) : 60;
	Kill(2);
	// HH_Y1: tee 1 starts at another height; HH_FRZ1: tee 1 starts frozen (let it settle first)
	const float Y1 = getenv("HH_Y1") ? atof(getenv("HH_Y1")) : Y;
	Spawn(1, vec2(X1 * 32 + 16, Y1 * 32 + 16));
	Spawn(0, vec2(X0 * 32 + 16, Y * 32 + 16));
	if(getenv("HH_FRZ1"))
	{
		for(int i = 0; i < 30; i++)
			Step(2);
		Chr(0)->m_Core.m_Pos = Chr(0)->m_PrevPos = vec2(X0 * 32 + 16, Y * 32 + 16);
		Chr(0)->m_Core.m_Vel = vec2(0, 0);
		// HH_V0=vx,vy: tee 0's velocity (e.g. just hammered out of the freeze)
		if(getenv("HH_V0"))
		{
			sscanf(getenv("HH_V0"), "%f,%f", &Chr(0)->m_Core.m_Vel.x, &Chr(0)->m_Core.m_Vel.y);
			Chr(0)->m_Pos = Chr(0)->m_Core.m_Pos; // else the next tick checks the tiles from where it was (in the freeze)
			Chr(0)->Unfreeze();
		}
		Chr(1)->Freeze();
	}
	for(int t = 0; t < 2; t++)
	{
		m_aBrain[t].Init(Col(), &GameServer()->m_pController->Teams().m_Core);
		m_aBrain[t].Start(CHookBotBrain::MODE_HAMMERHIT);
		m_aBrain[t].m_DriveDir = 1;
		if(getenv("HH_BUDGET"))
			m_aBrain[t].m_RescueBudgetUs = atoi(getenv("HH_BUDGET"));
		// planning inline keeps the run repeatable; the plans are only used from the tick they were planned for, as live
		m_aBrain[t].m_AsyncPlanning = getenv("HH_ASYNC") != nullptr;
		if(getenv("HH_FLY"))
			m_aBrain[t].m_FlyStyle = atoi(getenv("HH_FLY"));
	}
	// HH_GOALTILE=x,y: hammerhit to a goal (e.g. Simple Down's spawn segment) instead of right
	vec2 GoalTile(0, 0);
	const bool HasGoal = getenv("HH_GOALTILE") && sscanf(getenv("HH_GOALTILE"), "%f,%f", &GoalTile.x, &GoalTile.y) == 2;
	// HH_ROUTE=<route file>: follow its waypoints in play mode (HH_ROUTE_UNTIL: done once past this waypoint)
	std::vector<vec2> vRoute;
	const bool HasRoute = getenv("HH_ROUTE") != nullptr;
	if(HasRoute)
	{
		FILE *f = fopen(getenv("HH_ROUTE"), "r");
		ASSERT_TRUE(f);
		std::string Text;
		char aLine[256];
		while(fgets(aLine, sizeof(aLine), f))
			Text += aLine;
		fclose(f);
		ASSERT_TRUE(CHookBotBrain::ParseRoute(Text.c_str(), &vRoute));
		for(int t = 0; t < 2; t++)
		{
			m_aBrain[t].Start(CHookBotBrain::MODE_PLAY);
			m_aBrain[t].StartRoute(vRoute, Chr(t)->m_Pos, Chr(1 - t)->m_Pos, getenv("HH_ROUTE_AT") ? atoi(getenv("HH_ROUTE_AT")) : -1);
			if(t == 0)
				m_aBrain[t].m_Say = [&](const char *pText) { printf("route %5.2f s: %s\n", (m_pServer->Tick() - m_RouteT0) / 50.0, pText); };
		}
	}
	if(HasGoal)
		for(auto &Brain : m_aBrain)
		{
			Brain.SetGoal(GoalTile);
			if(getenv("HH_PLAY"))
				Brain.Start(CHookBotBrain::MODE_PLAY);
		}
	const char *pDemo = getenv("HH_DEMO");
	if(pDemo)
	{
		Name(0, "Claude");
		Name(1, "Claude 2");
		for(int t = 0; t < 2; t++)
			GameServer()->m_apPlayers[CidOf(t)]->SetTeeInfos("claude", false, 0, 0);
		StartRec(pDemo);
	}
	int Done = -1, MaxX = 0;
	// HH_REALTIME: one tick per 20 ms like a real server (threaded planning has to keep up with that)
	const bool RealTime = getenv("HH_REALTIME") != nullptr;
	const auto T0 = std::chrono::steady_clock::now();
	m_RouteT0 = m_pServer->Tick();
	for(int i = 0; i < Seconds * SERVER_TICK_SPEED && Chr(0) && Chr(1); i++)
	{
		if(RealTime)
			std::this_thread::sleep_until(T0 + std::chrono::milliseconds(20 * i));
		if(getenv("HH_SOLO") && getenv("HH_JUMP1"))
		{
			// the idle "human" jumps (and air-jumps a moment later) at these ticks, e.g. to be pulled up
			const int J = atoi(getenv("HH_JUMP1"));
			m_aTee[1].m_Input.m_Jump = (i >= J && i < J + 2) || (i >= J + 14 && i < J + 16);
		}
		BotsStep();
		if(!Chr(0) || !Chr(1))
			break;
		for(int t = 0; t < 2; t++)
			MaxX = std::max(MaxX, (int)(Chr(t)->m_Pos.x / 32));
		// done: both through (or near the goal), and one of them free (it can rescue the other on open floor)
		const int RouteUntil = getenv("HH_ROUTE_UNTIL") ? atoi(getenv("HH_ROUTE_UNTIL")) : 1 << 20;
		const bool Through = HasRoute ? m_aBrain[0].m_RouteIndex >= RouteUntil :
				     HasGoal  ? m_aBrain[0].m_pGoal->Dist(Chr(0)->m_Pos) < 9 * 32 && m_aBrain[0].m_pGoal->Dist(Chr(1)->m_Pos) < 9 * 32 :
						Chr(0)->m_Pos.x > Goal * 32 && Chr(1)->m_Pos.x > Goal * 32;
		if(Done < 0 && Through && (Chr(0)->m_FreezeTime == 0 || Chr(1)->m_FreezeTime == 0))
			Done = i;
		if(getenv("HH_EVERY") ? i % atoi(getenv("HH_EVERY")) == 0 : i % 10 == 0)
			printf("dbg %5.2f | A dir %2d goal d %.0f | A %6.1f %6.1f v %5.1f %5.1f %s hook %d | B %6.1f %6.1f v %5.1f %5.1f %s hook %d\n", i / 50.0, m_aTee[0].m_Input.m_Direction,
				m_aBrain[0].m_pGoal ? m_aBrain[0].m_pGoal->Dist(Chr(0)->m_Pos) : -1.0f, Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32,
				Chr(0)->m_Core.m_Vel.x, Chr(0)->m_Core.m_Vel.y, Chr(0)->m_FreezeTime ? "FRZ" : "   ", Chr(0)->m_Core.m_HookState, Chr(1)->m_Pos.x / 32,
				Chr(1)->m_Pos.y / 32, Chr(1)->m_Core.m_Vel.x, Chr(1)->m_Core.m_Vel.y, Chr(1)->m_FreezeTime ? "FRZ" : "   ", Chr(1)->m_Core.m_HookState);
		if(Done >= 0 && i > Done + 100)
			break;
	}
	if(HasRoute)
		printf("route: reached waypoint %d of %zu\n", m_aBrain[0].m_RouteIndex, vRoute.size());
	for(int t = 0; t < 2; t++)
		if(!Chr(t))
			printf("tee %d DIED\n", t);
		else
			printf("tee %d ends at tile %.1f %.1f%s\n", t, Chr(t)->m_Pos.x / 32, Chr(t)->m_Pos.y / 32, Chr(t)->m_FreezeTime ? " frozen" : "");
	if(pDemo)
	{
		Hold(50);
		StopRec();
	}
	for(int t = 0; t < 2; t++)
	{
		const auto &St = m_aBrain[t].m_RescueStats;
		printf("brain %d: plans %d (found %d), hammers %d, last plan %d us\n", t, St.m_Plans, St.m_Found, St.m_Fired, m_aBrain[t].m_LastPlanUs);
	}
	printf("hammerhit: %s, furthest tile x %d\n", Done >= 0 ? "both made it past the goal" : "stuck", MaxX);
	if(Done >= 0)
		printf("both past tile %d after %.2f s\n", Goal, Done / 50.0);
}

// one tee on the frozen other's head over a 1-tile freeze floor (Simple Down tunnel 3, floor row 78): the frozen
// one holds hammer, the top one hammers it (HS_X tile, HS_DX top offset px)
TEST_F(SimMapBots, HeadStand)
{
	Kill(2);
	const float X = (getenv("HS_X") ? atof(getenv("HS_X")) : 150) * 32 + 16, Dx = getenv("HS_DX") ? atof(getenv("HS_DX")) : 0;
	const float FloorY = 78 * 32 + 16;
	Spawn(1, vec2(X, FloorY));
	Spawn(0, vec2(X + Dx, FloorY - 40));
	for(int i = 0; i < 30; i++)
		Step(2);
	Chr(1)->Freeze();
	for(int i = 0; i < 10; i++)
		Step(2);
	printf("hs before: top %.1f %.1f bottom %.1f %.1f frozen %d/%d top grounded %d\n", Chr(0)->m_Pos.x, Chr(0)->m_Pos.y, Chr(1)->m_Pos.x, Chr(1)->m_Pos.y,
		Chr(0)->m_FreezeTime > 0, Chr(1)->m_FreezeTime > 0, Chr(0)->IsGrounded());
	PressFire(1);
	Aim(1, vec2(0, -1));
	Step(2);
	Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
	PressFire(0);
	for(int i = 0; i < 20; i++)
	{
		Step(2);
		ReleaseFire(0);
		printf("hs t=%2d top %6.1f %6.1f v %5.1f %5.1f frz %d | bottom %6.1f %6.1f v %5.1f %5.1f frz %d\n", i, Chr(0)->m_Pos.x, Chr(0)->m_Pos.y, Chr(0)->m_Core.m_Vel.x,
			Chr(0)->m_Core.m_Vel.y, Chr(0)->m_FreezeTime > 0, Chr(1)->m_Pos.x, Chr(1)->m_Pos.y, Chr(1)->m_Core.m_Vel.x, Chr(1)->m_Core.m_Vel.y, Chr(1)->m_FreezeTime > 0);
	}
}

// frozen, lying on a ledge in the freeze, and the partner hammers me from FP_FROM="x,y" (px): does holding the way the
// brain picks (FrozenHoldDir) take me somewhere the plain hammer doesn't? FP_BOT / FP_HUMAN="x,y" (px): where we
// start (default: on top of Stronghold's 3-tile post right of the ledge after the fly, the partner at the ledge's
// end), FP_ROUTE: route file (all its paths), FP_FORCE0: hold nothing instead
TEST_F(SimMapBots, FrozenPost)
{
	vec2 BotPos(10644, 2513), HumanPos(10593, 2705), From(10605, 2548);
	if(getenv("FP_BOT"))
		sscanf(getenv("FP_BOT"), "%f,%f", &BotPos.x, &BotPos.y);
	if(getenv("FP_HUMAN"))
		sscanf(getenv("FP_HUMAN"), "%f,%f", &HumanPos.x, &HumanPos.y);
	if(getenv("FP_FROM"))
		sscanf(getenv("FP_FROM"), "%f,%f", &From.x, &From.y);
	Kill(2);
	Spawn(1, HumanPos);
	Spawn(0, BotPos);
	for(int i = 0; i < 10; i++)
		Step(2);
	CHookBotBrain &Brain = m_aBrain[0];
	Brain.Init(Col(), &GameServer()->m_pController->Teams().m_Core);
	Brain.m_AsyncPlanning = false;
	Brain.Start(CHookBotBrain::MODE_PLAY);
	Brain.m_Say = [&](const char *pText) { printf("fp %5.2f s: %s\n", (m_pServer->Tick() - m_RouteT0) / 50.0, pText); };
	m_RouteT0 = m_pServer->Tick();
	if(getenv("FP_ROUTE"))
	{
		FILE *f = fopen(getenv("FP_ROUTE"), "r");
		ASSERT_TRUE(f);
		std::string Text;
		char aLine[256];
		while(fgets(aLine, sizeof(aLine), f))
			Text += aLine;
		fclose(f);
		char aMsg[256];
		ASSERT_TRUE(Brain.StartRouteText(Text.c_str(), Chr(0)->m_Pos, Chr(1)->m_Pos, aMsg, sizeof(aMsg)));
		printf("fp: %s\n", aMsg);
	}
	auto BotTick = [&]() {
		m_aTee[0].m_Input = Brain.Tick(m_pServer->Tick() + 1, CHookBot::TeeState(Chr(0)), CHookBot::TeeState(Chr(1)), CHookBot::TicksFirst(GameServer(), Chr(0), Chr(1)));
		m_aTee[0].m_Input.m_PlayerFlags = PLAYERFLAG_PLAYING;
		if(getenv("FP_FORCE0"))
			m_aTee[0].m_Input.m_Direction = 0;
	};
	for(int i = 0; i < 60; i++)
	{
		BotTick();
		Step(2);
	}
	printf("fp before: bot %.1f %.1f frozen %d grounded %d, holding %d, path %d waypoint %d (tile %.1f %.1f)\n", Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32,
		Chr(0)->m_FreezeTime > 0, Chr(0)->IsGrounded(), m_aTee[0].m_Input.m_Direction, Brain.m_RoutePath, Brain.m_RouteIndex,
		Brain.m_vRoute.empty() ? 0.0f : Brain.m_vRoute[Brain.m_RouteIndex].x, Brain.m_vRoute.empty() ? 0.0f : Brain.m_vRoute[Brain.m_RouteIndex].y);
	// the partner in the nook below-left, swinging at me
	Chr(1)->m_Core.m_Pos = Chr(1)->m_Pos = Chr(1)->m_PrevPos = From;
	Chr(1)->m_Core.m_Vel = vec2(0, 0);
	Aim(1, normalize(Chr(0)->m_Pos - From));
	PressFire(1);
	float MaxX = 0;
	for(int i = 0; i < 120 && Chr(0); i++)
	{
		BotTick();
		Step(2);
		ReleaseFire(1);
		MaxX = std::max(MaxX, Chr(0)->m_Pos.x);
		if(i % 5 == 0)
			printf("fp t=%3d bot %6.2f %6.2f v %5.1f %5.1f frz %d in %d\n", i, Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32, Chr(0)->m_Core.m_Vel.x, Chr(0)->m_Core.m_Vel.y,
				Chr(0)->m_FreezeTime > 0, m_aTee[0].m_Input.m_Direction);
	}
	printf("fp after: bot %.2f %.2f, max x %.2f\n", Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32, MaxX / 32);
}

// the partner frozen for good somewhere I can't rescue it from (AL_HUMAN, px; default: the bottom of Stronghold's
// shaft), me free (AL_BOT; default: on the platform below the ledge): do I find my way to where I can get it out, and
// do I? AL_ROUTE: route file, AL_SECONDS
TEST_F(SimMapBots, Alone)
{
	vec2 BotPos(10254, 3569), HumanPos(9934, 4145);
	if(getenv("AL_BOT"))
		sscanf(getenv("AL_BOT"), "%f,%f", &BotPos.x, &BotPos.y);
	if(getenv("AL_HUMAN"))
		sscanf(getenv("AL_HUMAN"), "%f,%f", &HumanPos.x, &HumanPos.y);
	const int Seconds = getenv("AL_SECONDS") ? atoi(getenv("AL_SECONDS")) : 30;
	Kill(2);
	Spawn(1, HumanPos);
	Spawn(0, BotPos);
	for(int i = 0; i < 20; i++)
		Step(2);
	if(!getenv("AL_FREE")) // AL_FREE: the partner stands there free (a part the bot does on its own)
		Chr(1)->Freeze();
	if(getenv("AL_HV")) // AL_HV="vx,vy": the partner starts there flying
	{
		Chr(1)->m_Core.m_Pos = Chr(1)->m_Pos = Chr(1)->m_PrevPos = HumanPos;
		sscanf(getenv("AL_HV"), "%f,%f", &Chr(1)->m_Core.m_Vel.x, &Chr(1)->m_Core.m_Vel.y);
	}
	if(getenv("AL_V")) // AL_V="vx,vy": the bot starts there flying (e.g. a moment of a recorded run)
	{
		Chr(0)->m_Core.m_Pos = Chr(0)->m_Pos = Chr(0)->m_PrevPos = BotPos;
		sscanf(getenv("AL_V"), "%f,%f", &Chr(0)->m_Core.m_Vel.x, &Chr(0)->m_Core.m_Vel.y);
	}
	CHookBotBrain &Brain = m_aBrain[0];
	Brain.Init(Col(), &GameServer()->m_pController->Teams().m_Core);
	Brain.m_AsyncPlanning = getenv("AL_ASYNC") != nullptr; // AL_ASYNC: plan on threads as live (AL_REALTIME: 20 ms ticks)
	Brain.Start(CHookBotBrain::MODE_PLAY);
	m_RouteT0 = m_pServer->Tick();
	Brain.m_Say = [&](const char *pText) { printf("al %5.2f s: %s\n", (m_pServer->Tick() - m_RouteT0) / 50.0, pText); };
	std::vector<double> vTickMs; // how long each Tick() took on this thread
	if(getenv("AL_ROUTE"))
	{
		FILE *f = fopen(getenv("AL_ROUTE"), "r");
		ASSERT_TRUE(f);
		std::string Text;
		char aLine[256];
		while(fgets(aLine, sizeof(aLine), f))
			Text += aLine;
		fclose(f);
		char aMsg[256];
		ASSERT_TRUE(Brain.StartRouteText(Text.c_str(), Chr(0)->m_Pos, Chr(1)->m_Pos, aMsg, sizeof(aMsg)));
		printf("al: %s (line %d)\n", aMsg, Brain.m_RoutePath);
	}
	if(getenv("AL_LEAD")) // how many waypoints ahead the goal is
		Brain.m_RouteLead = atoi(getenv("AL_LEAD"));
	if(getenv("AL_V")) // flying: in the middle of swinging on my own
		Brain.m_CoastDir = Chr(0)->m_Core.m_Vel.x < 0 ? -1 : 1;
	const auto T0 = std::chrono::steady_clock::now();
	for(int i = 0; i < Seconds * SERVER_TICK_SPEED && Chr(0) && Chr(1); i++)
	{
		if(getenv("AL_REALTIME"))
			std::this_thread::sleep_until(T0 + std::chrono::milliseconds(20 * i));
		const auto TickStart = std::chrono::steady_clock::now();
		m_aTee[0].m_Input = Brain.Tick(m_pServer->Tick() + 1, CHookBot::TeeState(Chr(0)), CHookBot::TeeState(Chr(1)), CHookBot::TicksFirst(GameServer(), Chr(0), Chr(1)));
		vTickMs.push_back(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - TickStart).count());
		if(vTickMs.back() > 5 && getenv("AL_ASYNC"))
			printf("al slow tick %.2f s: %.1f ms\n", i / 50.0, vTickMs.back());
		m_aTee[0].m_Input.m_PlayerFlags = PLAYERFLAG_PLAYING;
		Step(2);
		if(i % (getenv("AL_EVERY") ? atoi(getenv("AL_EVERY")) : 25) == 0)
			printf("al %5.2f s: bot %6.2f %6.2f v %5.1f %5.1f frz %d in %2d %d %d | partner %6.2f %6.2f frz %d | waypoint %d\n", i / 50.0, Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32,
				Chr(0)->m_Core.m_Vel.x, Chr(0)->m_Core.m_Vel.y, Chr(0)->m_FreezeTime > 0, m_aTee[0].m_Input.m_Direction, m_aTee[0].m_Input.m_Jump, m_aTee[0].m_Input.m_Hook,
				Chr(1)->m_Pos.x / 32, Chr(1)->m_Pos.y / 32, Chr(1)->m_FreezeTime > 0, Brain.m_RouteIndex);
		if(getenv("AL_PLAN") && i % (getenv("AL_EVERY") ? atoi(getenv("AL_EVERY")) : 25) == 0) // the rescue plan's state
			printf("al   plan valid %d hook-only %d post %d dirafter %d jump %d air %d hook %d fire %d | plans %d found %d fired %d\n", Brain.m_Plan.m_Valid, Brain.m_Plan.m_HookOnly, Brain.m_Plan.m_PostDir, Brain.m_Plan.m_DirAfter, Brain.m_Plan.m_JumpAt,
				Brain.m_Plan.m_AirJumpAt, Brain.m_Plan.m_HookAt, Brain.m_Plan.m_FireAt, Brain.m_RescueStats.m_Plans, Brain.m_RescueStats.m_Found, Brain.m_RescueStats.m_Fired);
	}
	std::vector<double> vSorted = vTickMs;
	std::sort(vSorted.begin(), vSorted.end());
	if(!vSorted.empty())
		printf("al tick ms: median %.2f, 99%% %.2f, max %.1f, total %.0f over %d ticks\n", vSorted[vSorted.size() / 2], vSorted[vSorted.size() * 99 / 100], vSorted.back(),
			std::accumulate(vSorted.begin(), vSorted.end(), 0.0), (int)vSorted.size());
	printf("al end: bot %.2f %.2f frozen %d, partner %.2f %.2f frozen %d\n", Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32, Chr(0)->m_FreezeTime > 0, Chr(1)->m_Pos.x / 32,
		Chr(1)->m_Pos.y / 32, Chr(1)->m_FreezeTime > 0);
}

// the special layers mapdump doesn't show, in a region (TL_RECT="x0,y0,x1,y1", tiles): tele, speedup, switch, tune
TEST_F(SimMapBots, SpecialTiles)
{
	int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	ASSERT_TRUE(getenv("TL_RECT") && sscanf(getenv("TL_RECT"), "%d,%d,%d,%d", &x0, &y0, &x1, &y1) == 4);
	CCollision *c = Col();
	const int W = c->m_Width;
	for(int y = y0; y <= y1; y++)
		for(int x = x0; x <= x1; x++)
		{
			const int i = y * W + x;
			if(c->m_pTele && c->m_pTele[i].m_Type)
				printf("tl %d,%d tele type %d number %d\n", x, y, c->m_pTele[i].m_Type, c->m_pTele[i].m_Number);
			if(c->m_pSpeedup && c->m_pSpeedup[i].m_Type)
				printf("tl %d,%d speedup force %d angle %d max %d\n", x, y, c->m_pSpeedup[i].m_Force, c->m_pSpeedup[i].m_Angle, c->m_pSpeedup[i].m_MaxSpeed);
			if(c->m_pSwitch && c->m_pSwitch[i].m_Type)
				printf("tl %d,%d switch type %d number %d delay %d\n", x, y, c->m_pSwitch[i].m_Type, c->m_pSwitch[i].m_Number, c->m_pSwitch[i].m_Delay);
			if(c->m_pTune && c->m_pTune[i].m_Type)
				printf("tl %d,%d tune zone %d\n", x, y, c->m_pTune[i].m_Number);
			if(c->m_pFront && c->m_pFront[i].m_Index)
				printf("tl %d,%d front %d\n", x, y, c->m_pFront[i].m_Index);
		}
}

// a scripted rescue: tee 0 stands at JH_BOT (px), tee 1 lies frozen at JH_PARTNER (px); tee 0 jumps and fires its hook
// at tee 1 JH_AT ticks later (a list: "3,5,8") and holds it for JH_HOLD ticks: does it catch it, where does it pull it?
TEST_F(SimMapBots, JumpHook)
{
	vec2 BotPos, PartnerPos;
	ASSERT_TRUE(getenv("JH_BOT") && sscanf(getenv("JH_BOT"), "%f,%f", &BotPos.x, &BotPos.y) == 2);
	ASSERT_TRUE(getenv("JH_PARTNER") && sscanf(getenv("JH_PARTNER"), "%f,%f", &PartnerPos.x, &PartnerPos.y) == 2);
	const int Hold = getenv("JH_HOLD") ? atoi(getenv("JH_HOLD")) : 30;
	for(const char *p = getenv("JH_AT") ? getenv("JH_AT") : "4,6,8,10,12"; *p;)
	{
		const int At = atoi(p);
		while(*p && *p != ',')
			p++;
		if(*p)
			p++;
		Kill(2);
		Spawn(1, PartnerPos);
		Spawn(0, BotPos);
		for(int i = 0; i < 5; i++)
			Step(2);
		Chr(1)->Freeze();
		bool Caught = false;
		for(int t = 0; t < At + Hold + 30 && Chr(0) && Chr(1); t++)
		{
			// JH_AIRJUMP: tick of an air jump too
			m_aTee[0].m_Input.m_Jump = t == 0 || (getenv("JH_AIRJUMP") && t == atoi(getenv("JH_AIRJUMP")));
			// JH_DIR: steering while it holds the hook
			// (JH_DIR0: before it)
			m_aTee[0].m_Input.m_Direction = t >= At ? (getenv("JH_DIR") ? atoi(getenv("JH_DIR")) : 0) : (getenv("JH_DIR0") ? atoi(getenv("JH_DIR0")) : 0);
			// JH_AIMDY: aim this many px above (-) / below (+) its center
			Aim(0, normalize(Chr(1)->m_Pos + vec2(0, getenv("JH_AIMDY") ? atof(getenv("JH_AIMDY")) : 0) - Chr(0)->m_Pos));
			m_aTee[0].m_Input.m_Hook = t >= At && t < At + Hold;
			Step(2);
			Caught |= Chr(0)->m_Core.m_HookedPlayer == CidOf(1);
			if(getenv("JH_V")) // every tick
				printf("jh   t %2d bot %.2f %.2f v %.1f %.1f hook %d at %.2f %.2f hooked %d | partner %.2f %.2f\n", t, Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32, Chr(0)->m_Core.m_Vel.x,
					Chr(0)->m_Core.m_Vel.y, Chr(0)->m_Core.m_HookState, Chr(0)->m_Core.m_HookPos.x / 32, Chr(0)->m_Core.m_HookPos.y / 32, Chr(0)->m_Core.m_HookedPlayer, Chr(1)->m_Pos.x / 32,
					Chr(1)->m_Pos.y / 32);
			if(t == At)
				printf("jh at %2d: fired from %.2f %.2f\n", At, Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32);
		}
		printf("jh at %2d: caught %d, partner ends %.2f %.2f frozen %d, bot ends %.2f %.2f\n", At, Caught, Chr(1)->m_Pos.x / 32, Chr(1)->m_Pos.y / 32, Chr(1)->m_FreezeTime > 0,
			Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32);
	}
}

// a hammer from standing (SH_BOT, px) at a partner flying in (SH_PARTNER px, SH_V "vx,vy"), at tick N (SH_AT, a list):
// where does it come to rest, in the freeze or not? (the partner freezes wherever the map freezes it)
TEST_F(SimMapBots, StandHammer)
{
	vec2 BotPos, PartnerPos, V;
	ASSERT_TRUE(getenv("SH_BOT") && sscanf(getenv("SH_BOT"), "%f,%f", &BotPos.x, &BotPos.y) == 2);
	ASSERT_TRUE(getenv("SH_PARTNER") && sscanf(getenv("SH_PARTNER"), "%f,%f", &PartnerPos.x, &PartnerPos.y) == 2);
	ASSERT_TRUE(getenv("SH_V") && sscanf(getenv("SH_V"), "%f,%f", &V.x, &V.y) == 2);
	for(const char *p = getenv("SH_AT") ? getenv("SH_AT") : "10"; *p;)
	{
		const int At = atoi(p);
		while(*p && *p != ',')
			p++;
		if(*p)
			p++;
		Kill(2);
		Spawn(1, PartnerPos);
		Spawn(0, BotPos);
		Chr(1)->m_Core.m_Vel = V;
		bool Hit = false;
		for(int t = 0; t < At + 150 && Chr(0) && Chr(1); t++)
		{
			Aim(0, normalize(Chr(1)->m_Pos - Chr(0)->m_Pos));
			if(t == At)
			{
				PressFire(0);
				Hit = distance(Chr(0)->m_Pos, Chr(1)->m_Pos) < 61.5f;
			}
			else
				ReleaseFire(0);
			Step(2);
		}
		CHookBotSim Probe;
		Probe.m_pCollision = Col();
		printf("sh at %2d: in reach %d, partner rests at %.2f %.2f %s, bot %.2f %.2f %s\n", At, Hit, Chr(1)->m_Pos.x / 32, Chr(1)->m_Pos.y / 32, Probe.InFreeze(Chr(1)->m_Pos) ? "IN FREEZE" : "free of freeze",
			Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32, Chr(0)->m_FreezeTime ? "frozen" : "free");
	}
}

static bool HammerReachesTest(vec2 From, vec2 To) { return distance(From, To) < 61.5f; }
static void HammerPushTest(CCharacterCore &Target, vec2 From)
{
	vec2 Dir = distance(From, Target.m_Pos) > 0 ? normalize(Target.m_Pos - From) : vec2(0, -1);
	Target.m_Vel += vec2(0, -1) + normalize(Dir + vec2(0, -1.1f)) * 10.0f;
}

// a rescue from a recorded state: FS_BOT="x,y,vx,vy" (free, air jump left), FS_PARTNER="x,y" (frozen, lying),
// FS_GOAL="x,y" (tiles); prints the best plan at several budgets (FS_MS="30,1000")
TEST_F(SimMapBots, FromState)
{
	Kill(2);
	float ax, ay, avx, avy, bx, by, gx, gy;
	ASSERT_TRUE(getenv("FS_BOT") && sscanf(getenv("FS_BOT"), "%f,%f,%f,%f", &ax, &ay, &avx, &avy) == 4);
	ASSERT_TRUE(getenv("FS_PARTNER") && sscanf(getenv("FS_PARTNER"), "%f,%f", &bx, &by) == 2);
	ASSERT_TRUE(getenv("FS_GOAL") && sscanf(getenv("FS_GOAL"), "%f,%f", &gx, &gy) == 2);
	CHookBotBrain Brain;
	Brain.Init(Col(), &GameServer()->m_pController->Teams().m_Core);
	Brain.SetGoal(vec2(gx, gy));
	std::vector<int> vMs;
	for(const char *p = getenv("FS_MS") ? getenv("FS_MS") : "30,1000"; *p;)
	{
		vMs.push_back(atoi(p));
		while(*p && *p != ',')
			p++;
		if(*p)
			p++;
	}
	for(int Ms : vMs)
	{
		Spawn(1, vec2(bx, by - 2));
		Spawn(0, vec2(ax, ay));
		Step(2);
		Chr(1)->Freeze();
		// FS_SETTLE=n: the partner lies there n ticks first (in freeze it stays frozen)
		for(int i = 0; i < (getenv("FS_SETTLE") ? atoi(getenv("FS_SETTLE")) : 0); i++)
			Step(2);
		Chr(0)->m_Core.m_Pos = vec2(ax, ay);
		Chr(0)->m_Core.m_Vel = vec2(avx, avy);
		Chr(0)->m_PrevPos = vec2(ax, ay);
		Chr(0)->m_Core.m_Jumped = getenv("FS_JUMPED") ? atoi(getenv("FS_JUMPED")) : 0;
		Chr(0)->m_ReloadTimer = 0;
		CHookBotSim Base;
		CHookBot::InitSim(Base, GameServer(), Chr(0), Chr(1));
		int Cursor = 0, HookCursor = 0;
		const int64_t T0 = time_get();
		// FS_FIRST: as the live bot's first half second (no rescues by the hook alone yet)
		auto Plan = CHookBotBrain::PlanRescue(Base, 0, &Cursor, CHookBotBrain::SPlan(), 0, Brain.m_pGoal.get(), T0 + (int64_t)Ms * time_freq() / 1000, !getenv("FS_FIRST"), &HookCursor);
		printf("fs %5d ms (searched %d plan families): %s score %.0f partner ends %.1f %.1f", Ms, Cursor, Plan.m_Valid ? (Plan.m_Aled ? "ALED" : Plan.m_HookOnly ? "hook-only delivery" : Plan.m_Delivered ? "delivery" : "save") : "none", Plan.m_Score,
			Plan.m_PartnerEnd.x / 32, Plan.m_PartnerEnd.y / 32);
		if(Plan.m_Valid)
			printf(" | air jump %d aim %.0f dir after %d me frozen %d", Plan.m_AirJumpAt, Plan.m_AimDy, Plan.m_DirAfter, Plan.m_MeFrozen);
		if(Plan.m_Valid)
			printf(" | hook %d-%d then %d jump %d dir %d to x %.1f fire %d prehammer %d | at the hammer: bot %.1f %.1f partner %.1f %.1f", Plan.m_HookAt,
				Plan.m_HookEnd > 100000 ? -1 : Plan.m_HookEnd, Plan.m_Hook2At, Plan.m_JumpAt, Plan.m_Dir, Plan.m_TargetX / 32, Plan.m_FireAt, Plan.m_PreHammer, Plan.m_vPath.back().x / 32, Plan.m_vPath.back().y / 32, Plan.m_vUserPath.back().x / 32,
				Plan.m_vUserPath.back().y / 32);
		printf("\n");
		// FS_EXEC: play the plan's inputs in the real game from the same state; where it first parts from the plan
		if(Plan.m_Valid && getenv("FS_EXEC"))
		{
			Spawn(1, vec2(bx, by - 2));
			Spawn(0, vec2(ax, ay));
			Step(2);
			Chr(1)->Freeze();
			for(int i = 0; i < (getenv("FS_SETTLE") ? atoi(getenv("FS_SETTLE")) : 0); i++)
				Step(2);
			Chr(0)->m_Core.m_Pos = vec2(ax, ay);
			Chr(0)->m_Core.m_Vel = vec2(avx, avy);
			Chr(0)->m_PrevPos = vec2(ax, ay);
			Chr(0)->m_Core.m_Jumped = getenv("FS_JUMPED") ? atoi(getenv("FS_JUMPED")) : 0;
			Chr(0)->m_ReloadTimer = 0;
			int PrevHook = 0;
			// FS_SPLIT=n: at tick n, copy the game's state into a new sim (as the live bot does before every plan) and
			// go on in both: do they part?
			const int Split = getenv("FS_SPLIT") ? atoi(getenv("FS_SPLIT")) : -1;
			CHookBotSim Copy;
			int CopyPrevHook = 0;
			for(int t = 0; t < Plan.m_FireAt && Chr(0) && Chr(1); t++)
			{
				if(t == Split)
				{
					CHookBot::InitSim(Copy, GameServer(), Chr(0), Chr(1));
					CopyPrevHook = PrevHook;
				}
				if(Split >= 0 && t >= Split)
				{
					const auto &Cb = Copy.m_aTee[0], &Cu = Copy.m_aTee[1];
					CNetObj_PlayerInput Ci = CHookBotBrain::RescueInput(Plan, t, CopyPrevHook, Cb.m_Core.m_HookState, Cu.m_Core.m_Pos - Cb.m_Core.m_Pos, Cb.m_Core.m_Pos, Cb.m_Core.m_Vel);
					CopyPrevHook = Ci.m_Hook;
					Copy.Step(Ci, CNetObj_PlayerInput{});
				}
				CNetObj_PlayerInput In = CHookBotBrain::RescueInput(Plan, t, PrevHook, Chr(0)->m_Core.m_HookState, Chr(1)->m_Pos - Chr(0)->m_Pos, Chr(0)->m_Pos, Chr(0)->m_Core.m_Vel);
				PrevHook = In.m_Hook;
				In.m_PlayerFlags = PLAYERFLAG_PLAYING;
				m_aTee[0].m_Input = In;
				mem_zero(&m_aTee[1].m_Input, sizeof(m_aTee[1].m_Input));
				Step(2);
				if(Split >= 0 && t >= Split && (distance(Copy.m_aTee[0].m_Core.m_Pos, Chr(0)->m_Pos) > 0.5f || distance(Copy.m_aTee[1].m_Core.m_Pos, Chr(1)->m_Pos) > 0.5f))
				{
					printf("fs split t %d: game bot %.2f %.2f hook %d hooked %d | copy bot %.2f %.2f hook %d hooked %d | game partner %.2f %.2f copy %.2f %.2f\n", t, Chr(0)->m_Pos.x / 32,
						Chr(0)->m_Pos.y / 32, Chr(0)->m_Core.m_HookState, Chr(0)->m_Core.m_HookedPlayer, Copy.m_aTee[0].m_Core.m_Pos.x / 32, Copy.m_aTee[0].m_Core.m_Pos.y / 32,
						Copy.m_aTee[0].m_Core.m_HookState, Copy.m_aTee[0].m_Core.m_HookedPlayer, Chr(1)->m_Pos.x / 32, Chr(1)->m_Pos.y / 32, Copy.m_aTee[1].m_Core.m_Pos.x / 32,
						Copy.m_aTee[1].m_Core.m_Pos.y / 32);
					break;
				}
				const vec2 Pb = Plan.m_vPath[std::min(t + 1, (int)Plan.m_vPath.size() - 1)], Pu = Plan.m_vUserPath[std::min(t + 1, (int)Plan.m_vUserPath.size() - 1)];
				if(distance(Pb, Chr(0)->m_Pos) > 0.5f || distance(Pu, Chr(1)->m_Pos) > 0.5f || getenv("FS_EXEC_ALL"))
				{
					printf("fs exec t %d: bot %.2f %.2f planned %.2f %.2f | partner %.2f %.2f planned %.2f %.2f | input dir %d jump %d hook %d | hook state %d hooked %d\n", t,
						Chr(0)->m_Pos.x / 32, Chr(0)->m_Pos.y / 32, Pb.x / 32, Pb.y / 32, Chr(1)->m_Pos.x / 32, Chr(1)->m_Pos.y / 32, Pu.x / 32, Pu.y / 32, In.m_Direction, In.m_Jump,
						In.m_Hook, Chr(0)->m_Core.m_HookState, Chr(0)->m_Core.m_HookedPlayer);
					if(!getenv("FS_EXEC_ALL"))
						break;
				}
			}
		}
	}
}

// does ANY aled exist from a recorded state? Random piecewise plans (direction changes, hook segments, a jump),
// FS_* as in FromState, FS_N rollouts; prints the best few aleds (partner freed beyond the wall at FS_WALL tiles)
TEST_F(SimMapBots, AledSearch)
{
	Kill(2);
	float ax, ay, avx, avy, bx, by;
	ASSERT_TRUE(getenv("FS_BOT") && sscanf(getenv("FS_BOT"), "%f,%f,%f,%f", &ax, &ay, &avx, &avy) == 4);
	ASSERT_TRUE(getenv("FS_PARTNER") && sscanf(getenv("FS_PARTNER"), "%f,%f", &bx, &by) == 2);
	const float WallL = atof(getenv("FS_WALL") ? getenv("FS_WALL") : "108") * 32, WallR = WallL + 32;
	const int N = getenv("FS_N") ? atoi(getenv("FS_N")) : 200000;
	Spawn(1, vec2(bx, by - 2));
	Spawn(0, vec2(ax, ay));
	Step(2);
	Chr(1)->Freeze();
	Chr(0)->m_Core.m_Pos = vec2(ax, ay);
	Chr(0)->m_Core.m_Vel = vec2(avx, avy);
	Chr(0)->m_PrevPos = vec2(ax, ay);
	Chr(0)->m_Core.m_Jumped = 0;
	Chr(0)->m_ReloadTimer = 0;
	CHookBotSim Base;
	CHookBot::InitSim(Base, GameServer(), Chr(0), Chr(1));
	const CNetObj_PlayerInput None = {};
	unsigned Seed = 12345;
	auto Rnd = [&](int n) { Seed = Seed * 1103515245u + 12345u; return (int)((Seed >> 8) % (unsigned)n); };
	struct SFound
	{
		float m_Score;
		std::string m_Desc;
	};
	std::vector<SFound> vFound;
	int Aleds = 0;
	for(int r = 0; r < N; r++)
	{
		// a plan: up to 4 segments of (length, direction, hook), one jump tick
		int aLen[4], aDir[4], aHook[4];
		int Total = 0;
		for(int k = 0; k < 4; k++)
		{
			aLen[k] = 4 + Rnd(30);
			aDir[k] = Rnd(3) - 1;
			aHook[k] = Rnd(2);
			Total += aLen[k];
		}
		const int JumpAt = Rnd(3) ? Rnd(std::min(Total, 60)) : -1;
		CHookBotSim S = Base;
		int PrevHook = 0, t = 0;
		std::vector<int> vHookTicks;
		for(int k = 0; k < 4; k++)
			for(int i = 0; i < aLen[k]; i++, t++)
			{
				auto &Bot = S.m_aTee[0], &Usr = S.m_aTee[1];
				if(Bot.m_Dead || Usr.m_Dead || Bot.m_FreezeTime > 0 || Bot.m_EnteredFreeze)
					goto Next;
				// fire here? the partner beyond the wall, me before it, in reach, partner clean
				if(t > 0 && Usr.m_Core.m_Pos.x < WallL - 2 && Bot.m_Core.m_Pos.x > WallL && HammerReachesTest(Bot.m_Core.m_Pos, Usr.m_Core.m_Pos) && !Usr.m_EnteredFreeze &&
					!S.InFreeze(Usr.m_Core.m_Pos))
				{
					CHookBotSim A = S;
					HammerPushTest(A.m_aTee[1].m_Core, Bot.m_Core.m_Pos);
					A.m_aTee[1].m_FreezeTime = 0;
					int Free = 0;
					for(; Free < 30; Free++)
					{
						A.Step(None, None);
						if(A.m_aTee[1].m_Dead || A.m_aTee[1].m_EnteredFreeze || A.m_aTee[1].m_FreezeTime > 0)
							break;
					}
					if(Free >= 12)
					{
						Aleds++;
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "fire t=%d free %d | segs %d:%d/%d %d:%d/%d %d:%d/%d %d:%d/%d jump %d | bot %.1f %.1f partner %.1f %.1f", t, Free, aLen[0], aDir[0],
							aHook[0], aLen[1], aDir[1], aHook[1], aLen[2], aDir[2], aHook[2], aLen[3], aDir[3], aHook[3], JumpAt, Bot.m_Core.m_Pos.x / 32, Bot.m_Core.m_Pos.y / 32,
							Usr.m_Core.m_Pos.x / 32, Usr.m_Core.m_Pos.y / 32);
						vFound.push_back({(float)Free - t * 0.5f, aBuf});
						goto Next;
					}
				}
				CNetObj_PlayerInput In = {};
				In.m_TargetX = round_to_int(Usr.m_Core.m_Pos.x - Bot.m_Core.m_Pos.x);
				In.m_TargetY = round_to_int(Usr.m_Core.m_Pos.y - Bot.m_Core.m_Pos.y);
				In.m_Hook = HookBotHookInput(aHook[k], PrevHook, Bot.m_Core.m_HookState);
				In.m_Jump = t == JumpAt;
				In.m_Direction = aDir[k];
				PrevHook = In.m_Hook;
				S.Step(In, None);
			}
	Next:;
	}
	std::sort(vFound.begin(), vFound.end(), [](auto &a, auto &b) { return a.m_Score > b.m_Score; });
	printf("as: %d of %d random plans aled the partner through the wall\n", Aleds, N);
	for(int i = 0; i < 8 && i < (int)vFound.size(); i++)
		printf("as: %s\n", vFound[i].m_Desc.c_str());
}

// where must the frozen partner lie on the floor for the rescuer to aled it through the 1-tile wall at x 105?
// B frozen on the floor at x, A in the air at dx / height; the rescue planner gets a generous budget
TEST_F(SimMapBots, AledSetups)
{
	Kill(2);
	CHookBotBrain Brain;
	Brain.Init(Col(), &GameServer()->m_pController->Teams().m_Core);
	Brain.Start(CHookBotBrain::MODE_HAMMERHIT);
	Brain.m_DriveDir = 1;
	const float FloorY = 96 * 32 + 16; // a frozen tee lying on the freeze floor (settles at 3090)
	for(float Bx = getenv("AS_X0") ? atof(getenv("AS_X0")) : 94; Bx <= 104.6f; Bx += getenv("AS_STEP") ? atof(getenv("AS_STEP")) : 0.5f)
	{
		int Aleds = 0, Plans = 0, N = 0;
		std::string Line;
		for(float Dx : {-3.f, -1.5f, 0.f, 1.5f, 3.f})
			for(float H : {3.f, 5.f, 7.f, 9.f})
				for(float Vy : {-8.f, 0.f, 8.f})
				{
					const float Ax = std::min(Bx + Dx, 104.4f);
					Spawn(1, vec2(Bx * 32, FloorY));
					Spawn(0, vec2(Ax * 32, FloorY - H * 32));
					Chr(1)->Freeze();
					Chr(0)->m_Core.m_Vel = vec2(0, Vy);
					Chr(0)->m_Core.m_Jumped = 0;
					CHookBotSim Base;
					CHookBot::InitSim(Base, GameServer(), Chr(0), Chr(1));
					int Cursor = 0;
					const int64_t PlanStart = time_get();
					auto Plan = CHookBotBrain::PlanRescue(Base, 0, &Cursor, CHookBotBrain::SPlan(), 1, nullptr, time_get() + time_freq() / 2);
					Brain.m_LastPlanUs = (int)((time_get() - PlanStart) * 1000000 / time_freq());
					if(getenv("AS_DBG") && N == 0)
					{
						printf("dbg B frz %d pos %.0f %.0f infreeze %d | A pos %.0f %.0f reload %d frz %d jumped %d\n", Base.m_aTee[1].m_FreezeTime, Base.m_aTee[1].m_Core.m_Pos.x,
							Base.m_aTee[1].m_Core.m_Pos.y, Base.InFreeze(Base.m_aTee[1].m_Core.m_Pos), Base.m_aTee[0].m_Core.m_Pos.x, Base.m_aTee[0].m_Core.m_Pos.y,
							Base.m_aTee[0].m_Reload, Base.m_aTee[0].m_FreezeTime, Base.m_aTee[0].m_Core.m_Jumped);
						CHookBotSim S = Base;
						CNetObj_PlayerInput In = {}, None = {};
						for(int t = 0; t < 40; t++)
						{
							In.m_TargetX = round_to_int(S.m_aTee[1].m_Core.m_Pos.x - S.m_aTee[0].m_Core.m_Pos.x);
							In.m_TargetY = round_to_int(S.m_aTee[1].m_Core.m_Pos.y - S.m_aTee[0].m_Core.m_Pos.y);
							In.m_Hook = 1;
							S.Step(In, None);
							if(t % 4 == 0)
								printf("dbg t=%d A %.0f %.0f hs %d hp %d | B %.0f %.0f frz %d\n", t, S.m_aTee[0].m_Core.m_Pos.x, S.m_aTee[0].m_Core.m_Pos.y, S.m_aTee[0].m_Core.m_HookState,
									S.m_aTee[0].m_Core.HookedPlayer(), S.m_aTee[1].m_Core.m_Pos.x, S.m_aTee[1].m_Core.m_Pos.y, S.m_aTee[1].m_FreezeTime);
						}
					}
					N++;
					Plans += Plan.m_Valid;
					Aleds += Plan.m_Valid && Plan.m_Score > 300;
					if(Plan.m_Valid && Plan.m_Score > 300 && getenv("AS_PARAMS"))
						printf("aled plan: hookat %d hookfor %d jump %d dir %d fire %d plan %d us\n", Plan.m_HookAt, Plan.m_HookEnd > 100000 ? -1 : Plan.m_HookEnd - Plan.m_HookAt,
							Plan.m_JumpAt, Plan.m_Dir, Plan.m_FireAt, Brain.m_LastPlanUs);
				}
		printf("aled setups: B frozen at x %5.1f: rescue found %2d/%d, aled %2d/%d\n", Bx, Plans, N, Aleds, N);
	}
}

// ---- the chat-controlled hookfly bot (src/game/server/hookbot.cpp) with a scripted "human" partner ----
// tee 0 is the first debug dummy (the bot's slot), tee 1 plays the human with a reaction delay
struct SHuman
{
	int m_Delay = 2; // reaction time in ticks
	float m_Wait = 12; // hook back when own vy > -m_Wait
	int m_Release = 0; // let go once the bot is above me by -m_Release px
	int m_Dir = 0; // 1: step away from the bot while hooking
};

class SimBot : public SimHookfly
{
public:
	CHookBot &Bot() { return GameServer()->m_HookBot; }
	struct SSeen
	{
		vec2 m_B, m_U, m_VU;
		bool m_BotHooksMe;
	};
	std::vector<SSeen> m_vSeen;
	void Start(int Mode)
	{
		g_Config.m_SvHookbot = 1;
		Bot().OnTick();
		ASSERT_TRUE(Bot().IsBot(CidOf(0)));
		EXPECT_STREQ(m_pServer->ClientName(CidOf(0)), "Claude");
		EXPECT_STREQ(GameServer()->m_apPlayers[CidOf(0)]->TeeInfos().m_aSkinName, "claude");
		Bot().Start(Mode, CidOf(1));
		m_vSeen.clear();
	}
	void HumanInputs(const SHuman &H)
	{
		CCharacter *pB = Chr(0), *pU = Chr(1);
		m_vSeen.push_back({pB->m_Pos, pU->m_Pos, pU->m_Core.m_Vel, pB->m_Core.HookedPlayer() == CidOf(1) && pB->m_Core.m_HookState == HOOK_GRABBED});
		const SSeen &S = m_vSeen[std::max<int>(0, (int)m_vSeen.size() - 1 - H.m_Delay)];
		auto &In = m_aTee[1].m_Input;
		int State = pU->m_Core.m_HookState;
		bool Grabbed = pU->m_Core.HookedPlayer() == CidOf(0) && State == HOOK_GRABBED;
		bool Want;
		if(Grabbed)
			Want = S.m_B.y > S.m_U.y - H.m_Release;
		else if(State == HOOK_FLYING && In.m_Hook)
			Want = true;
		else
			Want = !S.m_BotHooksMe && S.m_B.y > S.m_U.y + 10 && S.m_VU.y > -H.m_Wait && !pU->IsGrounded();
		In.m_Hook = HookBotHookInput(Want, In.m_Hook, State);
		Aim(1, normalize(S.m_B - S.m_U));
		int Away = S.m_U.x >= S.m_B.x ? 1 : -1;
		In.m_Direction = Want ? Away * H.m_Dir : 0;
		In.m_Jump = 0;
	}
	void BotStep()
	{
		Bot().GetInput(CidOf(0), &m_aTee[0].m_Input);
		Step(2);
	}
	static std::vector<SHuman> Humans()
	{
		std::vector<SHuman> v;
		for(int Delay : {0, 3, 6})
			for(float Wait : {6.f, 12.f})
				for(int Release : {0, 20})
					for(int Dir : {0, 1})
						v.push_back({Delay, Wait, Release, Dir});
		return v;
	}
	// partners that play their half of the hookfly properly (hook back right away, step aside), with reaction delays
	static std::vector<SHuman> GoodHumans()
	{
		std::vector<SHuman> v;
		for(int Delay : {0, 1, 2, 3, 4})
			for(int Release : {0, 20})
				v.push_back({Delay, 12, Release, 1});
		return v;
	}
};

// one hookfly with a scripted human; returns the lowest height (of the lower tee) after 3 s, and the final top height
std::pair<float, float> BotHookfly(SimBot *p, const SHuman &H, int Ticks = 600)
{
	p->Spawn(0, vec2(248 * 32 + 16, (SimBot::FLOOR - 1) * 32 + 16));
	p->Spawn(1, vec2(248 * 32 + 16 + 40, (SimBot::FLOOR - 1) * 32 + 16));
	p->Start(CHookBotBrain::MODE_HOOKFLY);
	float Low = 1e9f;
	for(int i = 0; i < Ticks && p->Chr(0) && p->Chr(1); i++)
	{
		p->HumanInputs(H);
		p->BotStep();
		if(i > 150 && p->Chr(0) && p->Chr(1))
			Low = std::min(Low, SimBot::FLOOR * 32 - std::max(p->Chr(0)->m_Pos.y, p->Chr(1)->m_Pos.y));
	}
	if(!p->Chr(0) || !p->Chr(1))
		return {0, 0};
	float Top = SimBot::FLOOR * 32 - std::min(p->Chr(0)->m_Pos.y, p->Chr(1)->m_Pos.y);
	return {Low, Top};
}

TEST_F(SimBot, TuneHookfly)
{
	UseBigMap(Col(), 500, 400, &GameServer()->m_World);
	FillRect(0, FLOOR, 499, FLOOR, TILE_SOLID);
	struct SRes
	{
		CHookBotBrain::SFlyParams m_P;
		int m_Good;
		float m_Sum;
	};
	std::vector<SRes> v;
	auto vHumans = Humans();
	for(float Wait : {4.f, 8.f, 12.f, 16.f})
		for(float StartWait : {1.f, 4.f, 8.f})
			for(int Release : {-20, 0, 20})
				for(int HookerDir : {0, 1})
					for(int Dodge : {0, 48})
					{
						CHookBotBrain::SFlyParams P;
						P.m_Wait = Wait;
						P.m_StartWait = StartWait;
						P.m_Release = Release;
						P.m_HookerDir = HookerDir;
						P.m_Dodge = Dodge;
						int Good = 0;
						float Sum = 0;
						for(const SHuman &H : vHumans)
						{
							Bot().m_Brain.m_Fly = P;
							auto [Low, Top] = BotHookfly(this, H);
							Good += Low > 200;
							Sum += std::min(Top, 2500.f);
							Finish(0);
						}
						v.push_back({P, Good, Sum});
					}
	std::sort(v.begin(), v.end(), [](auto &a, auto &b) { return a.m_Good != b.m_Good ? a.m_Good > b.m_Good : a.m_Sum > b.m_Sum; });
	for(int i = 0; i < 12; i++)
		printf("tune: %2d/%zu kept flying, avg top %4.0f | wait %2.0f start %2.0f release %3d hooker dir %d dodge %d\n", v[i].m_Good, vHumans.size(), v[i].m_Sum / vHumans.size(),
			v[i].m_P.m_Wait, v[i].m_P.m_StartWait, v[i].m_P.m_Release, v[i].m_P.m_HookerDir, v[i].m_P.m_Dodge);
}

TEST_F(SimBot, Hookfly)
{
	UseBigMap(Col(), 500, 400, &GameServer()->m_World);
	FillRect(0, FLOOR, 499, FLOOR, TILE_SOLID);
	int Good = 0, N = 0;
	for(const SHuman &H : GoodHumans())
	{
		auto [Low, Top] = BotHookfly(this, H);
		printf("hookfly human delay %d release %2d: lowest after 3 s %5.0f px, highest at 12 s %5.0f px\n", H.m_Delay, H.m_Release, Low, Top);
		Good += Low > 200;
		N++;
		Finish(0);
	}
	printf("hookfly kept going (never back under 200 px) with %d of %d scripted partners\n", Good, N);
	EXPECT_GE(Good * 10, N * 6);
}

TEST_F(SimBot, Aled)
{
	int Total = 0, Ok = 0, MaxPlanUs = 0;
	for(int Ceil = FLOOR - 40; Ceil <= FLOOR - 8; Ceil += 2)
		if(!getenv("SIM_CEIL") || Ceil == FLOOR - atoi(getenv("SIM_CEIL")))
	{
		int CeilOk = 0, CeilN = 0, Stuck = 0, BotFirst = 0, Dives = 0;
		const float CeilTop = Ceil * 32;
		for(const SHuman &H : GoodHumans())
			if(!getenv("SIM_ONE") || (H.m_Delay == 0 && H.m_Release == 0))
		{
			UseBigMap(Col(), 500, 400, &GameServer()->m_World);
			FillRect(0, FLOOR, 499, FLOOR, TILE_SOLID);
			FillRect(0, Ceil, 499, Ceil + 1, TILE_FREEZE);
			Spawn(0, vec2(248 * 32 + 16, (FLOOR - 1) * 32 + 16));
			Spawn(1, vec2(248 * 32 + 16 + 40, (FLOOR - 1) * 32 + 16));
			Start(CHookBotBrain::MODE_ALED);
			bool Success = false, Dived = false;
			int EndTick = 0;
			for(int i = 0; i < 1500 && Chr(0) && Chr(1) && !Success; i++)
			{
				EndTick = i;
				HumanInputs(H);
				if(getenv("SIM_DBG") && Bot().m_Brain.m_Phase == 1)
				{
					auto &P = Bot().m_Brain.m_Plan;
					int k = m_pServer->Tick() - Bot().m_Brain.m_PlanStart;
					bool In = k >= 0 && k < (int)P.m_vPath.size();
					printf("dbg   plan valid %d hook@%d jump@%d dir %d fire@%d d %.1f | k %d expect %.1f,%.1f user %.1f,%.1f\n", P.m_Valid, P.m_HookAt, P.m_JumpAt, P.m_Dir, P.m_FireAt, P.m_Dist, k,
						In ? P.m_vPath[k].x : 0.f, In ? P.m_vPath[k].y : 0.f, In ? P.m_vUserPath[k].x : 0.f, In ? P.m_vUserPath[k].y : 0.f);
				}
				CHookBotSim Pred;
				bool Check = getenv("SIM_DBG") && Bot().m_Brain.m_Phase == 1 && Chr(1)->m_FreezeTime > 0;
				if(Check)
					CHookBot::InitSim(Pred, GameServer(), Chr(0), Chr(1));
				Bot().GetInput(CidOf(0), &m_aTee[0].m_Input);
				if(Check)
				{
					CNetObj_PlayerInput None = {};
					Pred.Step(m_aTee[0].m_Input, None);
				}
				Step(2);
				if(Check && Chr(0) && Chr(1))
					printf("dbg   predicted bot %.2f,%.2f human %.2f,%.2f v %.2f,%.2f | actual bot %.2f,%.2f human %.2f,%.2f v %.2f,%.2f\n", Pred.m_aTee[0].m_Core.m_Pos.x, Pred.m_aTee[0].m_Core.m_Pos.y, Pred.m_aTee[1].m_Core.m_Pos.x, Pred.m_aTee[1].m_Core.m_Pos.y, Pred.m_aTee[1].m_Core.m_Vel.x, Pred.m_aTee[1].m_Core.m_Vel.y,
						Chr(0)->m_Pos.x, Chr(0)->m_Pos.y, Chr(1)->m_Pos.x, Chr(1)->m_Pos.y, Chr(1)->m_Core.m_Vel.x, Chr(1)->m_Core.m_Vel.y);
				if(getenv("SIM_DBG") && Chr(0) && Chr(1))
					printf("dbg t=%d phase %d bot %.0f,%.0f v %.1f,%.1f hook %d/%d frz %d fire %d | human %.0f,%.0f v %.1f,%.1f hook %d frz %d | ceil %.0f\n", i, Bot().m_Brain.m_Phase, Chr(0)->m_Pos.x, Chr(0)->m_Pos.y, Chr(0)->m_Core.m_Vel.x, Chr(0)->m_Core.m_Vel.y, m_aTee[0].m_Input.m_Hook, Chr(0)->m_Core.m_HookState, Chr(0)->m_FreezeTime, m_aTee[0].m_Input.m_Fire, Chr(1)->m_Pos.x, Chr(1)->m_Pos.y, Chr(1)->m_Core.m_Vel.x, Chr(1)->m_Core.m_Vel.y, Chr(1)->m_Core.m_HookState, Chr(1)->m_FreezeTime, CeilTop);
				Dived |= Bot().m_Brain.m_Mode == CHookBotBrain::MODE_ALED && Bot().m_Brain.m_Phase >= 1;
				Success = Chr(1)->m_Pos.y < CeilTop && Chr(1)->m_FreezeTime == 0;
				if(!Success && i > 60 && Chr(1)->m_FreezeTime > 0 && Chr(1)->m_Pos.y > CeilTop && Chr(1)->m_Pos.y < CeilTop + 64 && Chr(1)->m_Core.m_Vel.y >= 0)
					Stuck++, i = 1500;
				if(!Success && Chr(0)->m_FreezeTime > 0 && Chr(0)->m_Pos.y < CeilTop + 64 && Chr(1)->m_FreezeTime == 0 && Bot().m_Brain.m_Phase != 2)
					BotFirst++, i = 1500;
			}
			if(getenv("SIM_STATS"))
			{
				printf("  ended at %d: bot %s human %s\n", EndTick, Chr(0) ? "alive" : "dead", Chr(1) ? "alive" : "dead");
				auto &St = Bot().m_Brain.m_Stats;
				printf("  dives into freeze %d: out of reach %d reloading %d partner not beyond %d refrozen %d\n", St.m_Entered, St.m_FarReach, St.m_Reloading, St.m_NotBeyond, St.m_Refrozen);
				printf("  run delay %d release %d: %s | checks %d through %d launches %d (hammer %d) replans %d (failed %d) fired %d aborts %d\n", H.m_Delay, H.m_Release, Success ? "OK" : "--", St.m_LaunchChecks, St.m_Through, St.m_Launches, St.m_HammerLaunches, St.m_Replans, St.m_ReplanFails, St.m_Fired, St.m_Aborts);
			}
			Bot().m_Brain.m_Stats = {};
			Dives += Dived;
			CeilOk += Success;
			MaxPlanUs = std::max(MaxPlanUs, Bot().m_Brain.m_LastPlanUs);
			CeilN++;
			Finish(0);
		}
		printf("aled: ceiling %2d tiles up: %2d/%d freed (dive started %d, partner stuck in the freeze %d, bot frozen first %d)\n", FLOOR - Ceil, CeilOk, CeilN, Dives, Stuck, BotFirst);
		Ok += CeilOk;
		Total += CeilN;
	}
	printf("aled total %d/%d (slowest plan %d us)\n", Ok, Total, MaxPlanUs);
	EXPECT_GT(Ok, 0);
}

TEST_F(SimFly, HookflyClimbTrace)
{
	UseBigMap(Col(), 200, 400, &GameServer()->m_World);
	FillRect(0, 380, 199, 380, TILE_SOLID);
	Spawn(0, vec2(100 * 32 + 16, 379 * 32 + 16));
	Spawn(1, vec2(100 * 32 + 16 + 40, 379 * 32 + 16));
	m_aTee[1].m_Input.m_Jump = 1;
	for(int i = 0; i < 15; i++)
		Step(2);
	m_aTee[1].m_Input.m_Jump = 0;
	SHookflyState St;
	SHookflyPolicy Pol{12, 1, 0, 0};
	for(int i = 0; i < 600 && Chr(0) && Chr(1); i++)
	{
		HookflyStep(this, St, Pol);
		Step(2);
		if(i % 50 == 0)
			printf("dbg t=%d heights %.0f %.0f dx %.0f\n", i, 380 * 32 - Chr(0)->m_Pos.y, 380 * 32 - Chr(1)->m_Pos.y, Chr(1)->m_Pos.x - Chr(0)->m_Pos.x);
	}
}

TEST_F(SimBot, StepCost)
{
	UseBigMap(Col(), 500, 400, &GameServer()->m_World);
	FillRect(0, FLOOR, 499, FLOOR, TILE_SOLID);
	FillRect(0, FLOOR - 20, 499, FLOOR - 19, TILE_FREEZE);
	Spawn(0, vec2(248 * 32 + 16, (FLOOR - 5) * 32 + 16));
	Spawn(1, vec2(248 * 32 + 16 + 40, (FLOOR - 8) * 32 + 16));
	CHookBotSim Base;
	CHookBot::InitSim(Base, GameServer(), Chr(0), Chr(1));
	CNetObj_PlayerInput In = {}, None = {};
	In.m_Hook = 1;
	In.m_TargetX = 40;
	In.m_TargetY = -96;
	int64_t Start = time_get();
	int N = 0;
	for(int r = 0; r < 2000; r++)
	{
		CHookBotSim S = Base;
		for(int t = 0; t < 50; t++, N++)
			S.Step(In, None);
	}
	printf("step: %.3f us per two-tee step\n", (time_get() - Start) * 1e6 / time_freq() / N);
}

// the test map for playing with the bot: HOOKBOT_MAP_OUT=<file> ./testrunner --gtest_filter=SimBot.WriteMap
// an open shaft (plain hookfly), three shafts with a 2-tile freeze ceiling 10 / 20 / 30 tiles above the floor, and a
// 200 tile wide hall
TEST_F(SimBot, WriteMap)
{
	const int MW = 364, MH = 150, Floor = 140;
	std::vector<CTile> vTiles((size_t)MW * MH, CTile{});
	auto Rect = [&](int x0, int y0, int x1, int y1, int Index) {
		for(int y = y0; y <= y1; y++)
			for(int x = x0; x <= x1; x++)
				vTiles[y * MW + x].m_Index = Index;
	};
	Rect(0, 0, MW - 1, 0, TILE_SOLID);
	Rect(0, 0, 0, MH - 1, TILE_SOLID);
	Rect(MW - 1, 0, MW - 1, MH - 1, TILE_SOLID);
	Rect(0, Floor, MW - 1, MH - 1, TILE_SOLID);
	// shafts 40 tiles wide, walls with a 3-tile gap at the floor to walk through
	// then a 200 tile wide hall for flying sideways (pseudofly)
	for(int Wall : {41, 81, 121, 161})
		Rect(Wall, 1, Wall, Floor - 4, TILE_SOLID);
	int Ceil[3] = {10, 20, 30};
	for(int i = 0; i < 3; i++)
	{
		int x0 = 42 + i * 40, y = Floor - Ceil[i] - 2;
		Rect(x0, y, x0 + 38, y + 1, TILE_FREEZE);
	}
	for(int x : {15, 20, 25})
		Rect(x, Floor - 1, x, Floor - 1, ENTITY_OFFSET + ENTITY_SPAWN);

	const char *pTmp = "maps/hookbot.map";
	m_pStorage->CreateFolder("maps", IStorage::TYPE_SAVE);
	{
		CDataFileWriter Writer;
		ASSERT_TRUE(Writer.Open(m_pStorage.get(), pTmp));
		CMapItemVersion Version;
		Version.m_Version = 1;
		Writer.AddItem(MAPITEMTYPE_VERSION, 0, sizeof(Version), &Version);
		CMapItemGroup_v1 Group;
		Group.m_Version = 1;
		Group.m_OffsetX = 0;
		Group.m_OffsetY = 0;
		Group.m_ParallaxX = 100;
		Group.m_ParallaxY = 100;
		Group.m_StartLayer = 0;
		Group.m_NumLayers = 1;
		Writer.AddItem(MAPITEMTYPE_GROUP, 0, sizeof(Group), &Group);
		CMapItemLayerTilemap_v2 GameLayer;
		GameLayer.m_Layer.m_Version = 0;
		GameLayer.m_Layer.m_Type = LAYERTYPE_TILES;
		GameLayer.m_Layer.m_Flags = 0;
		GameLayer.m_Version = 2;
		GameLayer.m_Width = MW;
		GameLayer.m_Height = MH;
		GameLayer.m_Flags = TILESLAYERFLAG_GAME;
		GameLayer.m_Color = {255, 255, 255, 255};
		GameLayer.m_ColorEnv = -1;
		GameLayer.m_ColorEnvOffset = 0;
		GameLayer.m_Image = -1;
		GameLayer.m_Data = Writer.AddData(vTiles.size() * sizeof(CTile), vTiles.data());
		Writer.AddItem(MAPITEMTYPE_LAYER, 0, sizeof(GameLayer), &GameLayer);
		Writer.Finish();
	}
	void *pData;
	unsigned Size;
	ASSERT_TRUE(m_pStorage->ReadFile(pTmp, IStorage::TYPE_SAVE, &pData, &Size));
	const char *pOut = getenv("HOOKBOT_MAP_OUT");
	FILE *f = fopen(pOut ? pOut : "hookbot.map", "wb");
	ASSERT_TRUE(f);
	fwrite(pData, 1, Size, f);
	fclose(f);
	free(pData);
	printf("wrote %s (%u bytes)\n", pOut ? pOut : "hookbot.map", Size);
}

// save_replay slices the demo on a job thread; that used to overflow the 512 KiB default thread stack on macOS
// SLICE_DEMO=<absolute path to a demo> ./testrunner --gtest_filter=SimDemo.SliceOnThread
TEST_F(SimDemo, SliceOnThread)
{
	const char *pIn = getenv("SLICE_DEMO");
	if(!pIn)
		GTEST_SKIP() << "set SLICE_DEMO";
	static CDemoEditor s_Editor;
	s_Editor.Init(&m_pServer->m_SnapshotDelta, &m_pServer->m_SnapshotDeltaSixup, m_pServer->Console(), m_pStorage.get());
	struct SJob
	{
		const char *m_pIn;
		bool m_Ok = false;
	} Job{pIn};
	void *pThread = thread_init(
		[](void *pUser) {
			auto *p = (SJob *)pUser;
			p->m_Ok = s_Editor.Slice(p->m_pIn, "sliced.demo", 0, 1 << 30, nullptr, nullptr);
		},
		&Job, "slice");
	thread_wait(pThread);
	EXPECT_TRUE(Job.m_Ok);
}
