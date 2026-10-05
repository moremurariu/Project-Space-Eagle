#define private public
#define protected public
#include <game/client/pickup_data.h>
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/entities/pickup.h>
#include <game/client/prediction/entities/projectile.h>
#include <game/client/prediction/gameworld.h>
#undef private
#undef protected

#include "sim.h"

#include <cstdlib>

#include <base/str.h>
#include <engine/shared/config.h>
#include <engine/shared/map.h>
#include <engine/storage.h>

#include <game/collision.h>
#include <game/layers.h>
#include <game/mapbugs.h>
#include <game/mapitems.h>
#include <game/tuning.h>

#include <generated/protocol.h>

static CMap *gs_pMap;
static CLayers *gs_pLayers;
static CCollision *gs_pCollision;
static CTuningParams gs_aTuning[TuneZone::NUM];
static CMapBugs gs_MapBugs;
static SMapInfo gs_MapInfo;
static std::vector<CPickupData> gs_vPickups;

static void SetConfigDefaults()
{
	// this tool has no config manager, so set the variables' defaults directly
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Flags, Desc) g_Config.m_##Name = Def;
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Flags, Desc) g_Config.m_##Name = Def;
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Flags, Desc) str_copy(g_Config.m_##Name, Def, Len);
#include <engine/shared/config_variables.h>
#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_COL
#undef MACRO_CONFIG_STR
}

bool CTasGame::LoadMap(const char *pPath)
{
	SetConfigDefaults();
	const char *apArgs[] = {"tas"};
	IStorage *pStorage = CreateStorage(IStorage::EInitializationType::BASIC, 1, apArgs);
	gs_pMap = new CMap();
	if(!gs_pMap->Load(pPath, pStorage, pPath, IStorage::TYPE_ABSOLUTE))
		return false;
	gs_pLayers = new CLayers();
	gs_pLayers->Init(gs_pMap, false, false);
	gs_pCollision = new CCollision();
	gs_pCollision->Init(gs_pLayers);
	CCollision::ms_TileExistsCache = getenv("TAS_NOCACHE") == nullptr;
	CCollision::ms_FastPaths = getenv("TAS_NOFAST") == nullptr;
	CCharacterCore::ms_TasSolo = getenv("TAS_NOFAST") == nullptr;

	// same as CGameContext::OnInit with default settings
	for(auto &Tuning : gs_aTuning)
	{
		Tuning = CTuningParams::DEFAULT;
		Tuning.Set("gun_curvature", 0);
		Tuning.Set("gun_speed", 1400);
		Tuning.Set("shotgun_curvature", 0);
		Tuning.Set("shotgun_speed", 500);
		Tuning.Set("shotgun_speeddiff", 0);
		// a lone tee never meets another player; this only skips CCharacterCore's per-pixel
		// loops over all player slots (identical results, much faster)
		Tuning.Set("player_collision", 0);
		Tuning.Set("player_hooking", 0);
	}

	SMapInfo &M = gs_MapInfo;
	M.m_W = gs_pCollision->GetWidth();
	M.m_H = gs_pCollision->GetHeight();
	M.m_vGame.resize(M.m_W * M.m_H);
	if(gs_pCollision->FrontLayer())
		M.m_vFront.resize(M.m_W * M.m_H);
	for(int i = 0; i < M.m_W * M.m_H; i++)
	{
		int T = gs_pCollision->GameLayer()[i].m_Index;
		M.m_vGame[i] = T;
		if(!M.m_vFront.empty())
			M.m_vFront[i] = gs_pCollision->FrontLayer()[i].m_Index;
		vec2 Center((i % M.m_W) * 32.0f + 16.0f, (i / M.m_W) * 32.0f + 16.0f);
		if(T == ENTITY_OFFSET + ENTITY_SPAWN)
			M.m_vSpawns.push_back(Center);
		if(T == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
		{
			CPickupData Data;
			Data.m_Pos = Center;
			Data.m_Type = POWERUP_WEAPON;
			Data.m_Subtype = WEAPON_GRENADE;
			Data.m_SwitchNumber = 0;
			gs_vPickups.push_back(Data);
		}
	}
	return true;
}

const SMapInfo &CTasGame::Map() { return gs_MapInfo; }
CCollision *CTasGame::Collision() { return gs_pCollision; }

CTasGame::CTasGame() :
	m_pWorld(std::make_unique<CGameWorld>())
{
	m_pWorld->Init(gs_pCollision, gs_aTuning, &gs_MapBugs);
	auto &C = m_pWorld->m_WorldConfig;
	C.m_IsDDRace = true;
	C.m_IsVanilla = false;
	C.m_IsFNG = false;
	C.m_InfiniteAmmo = true;
	C.m_PredictTiles = true;
	C.m_PredictFreeze = 1;
	C.m_PredictWeapons = true;
	C.m_PredictDDRace = true;
	C.m_IsSolo = false;
	C.m_UseTuneZones = false;
	C.m_BugDDRaceInput = false;
	C.m_NoWeakHookAndBounce = false;
	C.m_PredictEvents = false;
	C.m_OldLaser = false;
	m_pWorld->m_IsValidCopy = true;
	m_pWorld->m_LocalClientId = 0;
}

CTasGame::~CTasGame() = default;

void CTasGame::Spawn(vec2 Pos)
{
	CNetObj_Character Char = {};
	Char.m_X = round_to_int(Pos.x);
	Char.m_Y = round_to_int(Pos.y);
	Char.m_Weapon = WEAPON_GUN;
	Char.m_HookState = HOOK_IDLE;
	Char.m_HookedPlayer = -1;
	Char.m_Direction = 0;
	Char.m_Tick = m_Tick;
	CNetObj_DDNetCharacter Ext = {};
	Ext.m_Flags = CHARACTERFLAG_WEAPON_HAMMER | CHARACTERFLAG_WEAPON_GUN;
	Ext.m_FreezeEnd = 0;
	Ext.m_Jumps = 2;
	Ext.m_TeleCheckpoint = 0;
	Ext.m_StrongWeakId = 0;
	Ext.m_JumpedTotal = 0;
	Ext.m_NinjaActivationTick = -500;
	Ext.m_FreezeStart = 0;
	Ext.m_TargetX = 0;
	Ext.m_TargetY = -1;
	Ext.m_TuneZoneOverride = TuneZone::OVERRIDE_NONE;
	CCharacter *pChr = new CCharacter(m_pWorld.get(), 0, &Char, &Ext);
	pChr->m_Core.m_Pos = Pos;
	pChr->m_Pos = pChr->m_PrevPos = pChr->m_PrevPrevPos = Pos;
	pChr->m_Core.m_Vel = vec2(0, 0);
	pChr->m_Core.m_ActiveWeapon = WEAPON_GUN;
	pChr->m_Core.m_aWeapons[WEAPON_HAMMER].m_Ammo = -1;
	pChr->m_Core.m_aWeapons[WEAPON_GUN].m_Ammo = -1;
	pChr->m_LastWeapon = WEAPON_HAMMER;
	pChr->m_Core.m_Tuning = gs_aTuning[0];
	pChr->m_IsLocal = true;
	m_pWorld->InsertEntity(pChr);
	m_pWorld->m_apCharacters[0] = pChr;
	m_pWorld->m_Core.m_apCharacters[0] = &pChr->m_Core;

	int Id = 1;
	for(const auto &Data : gs_vPickups)
		m_pWorld->InsertEntity(new CPickup(m_pWorld.get(), Id++, &Data));
}

static void Unlink(CGameWorld *pWorld)
{
	pWorld->m_pParent = nullptr;
	pWorld->m_pChild = nullptr;
	for(int Type = 0; Type < CGameWorld::NUM_ENTTYPES; Type++)
		for(CEntity *pEnt = pWorld->FindFirst(Type); pEnt; pEnt = pEnt->TypeNext())
		{
			pEnt->m_pParent = nullptr;
			pEnt->m_pChild = nullptr;
		}
}

static int CountEntities(CGameWorld *pWorld, int Type)
{
	int n = 0;
	for(CEntity *pEnt = pWorld->FindFirst(Type); pEnt; pEnt = pEnt->TypeNext())
		n++;
	return n;
}

// fast copy: same entity layout (one character, the static pickups, nothing else) -> copy the character in place
static bool FastCopy(CGameWorld *pTo, CGameWorld *pFrom)
{
	static const bool s_Off = getenv("TAS_NOFASTCOPY") != nullptr;
	if(s_Off)
		return false;
	for(int Type = 0; Type < CGameWorld::NUM_ENTTYPES; Type++)
	{
		if(Type == CGameWorld::ENTTYPE_CHARACTER || Type == CGameWorld::ENTTYPE_PICKUP)
			continue;
		if(pFrom->FindFirst(Type) || pTo->FindFirst(Type))
			return false;
	}
	if(CountEntities(pFrom, CGameWorld::ENTTYPE_CHARACTER) != 1 || CountEntities(pTo, CGameWorld::ENTTYPE_CHARACTER) != 1)
		return false;
	if(CountEntities(pFrom, CGameWorld::ENTTYPE_PICKUP) != CountEntities(pTo, CGameWorld::ENTTYPE_PICKUP))
		return false;
	CCharacter *pSrc = (CCharacter *)pFrom->FindFirst(CGameWorld::ENTTYPE_CHARACTER);
	CCharacter *pDst = (CCharacter *)pTo->FindFirst(CGameWorld::ENTTYPE_CHARACTER);
	if(pSrc->GetCid() != 0 || pDst->GetCid() != 0)
		return false;
	pTo->m_GameTick = pFrom->m_GameTick;
	pTo->m_pCollision = pFrom->m_pCollision;
	pTo->m_WorldConfig = pFrom->m_WorldConfig;
	pTo->m_pTuningList = pFrom->m_pTuningList;
	pTo->m_pMapBugs = pFrom->m_pMapBugs;
	pTo->m_Teams = pFrom->m_Teams;
	if(!pFrom->m_Core.m_vSwitchers.empty() || !pTo->m_Core.m_vSwitchers.empty())
		pTo->m_Core.m_vSwitchers = pFrom->m_Core.m_vSwitchers;
	pTo->m_PredictedEvents = pFrom->m_PredictedEvents;
	CEntity *pPrev = pDst->m_pPrevTypeEntity, *pNext = pDst->m_pNextTypeEntity;
	*pDst = *pSrc;
	pDst->m_pPrevTypeEntity = pPrev;
	pDst->m_pNextTypeEntity = pNext;
	pDst->m_pParent = nullptr;
	pDst->m_pChild = nullptr;
	pDst->m_pGameWorld = pTo;
	pTo->m_apCharacters[0] = pDst;
	pTo->m_Core.m_apCharacters[0] = &pDst->m_Core;
	pDst->SetCoreWorld(pTo);
	return true;
}

void CTasGame::CopyFrom(const CTasGame &Other)
{
	CGameWorld *pFrom = Other.m_pWorld.get();
	if(!FastCopy(m_pWorld.get(), pFrom))
	{
		m_pWorld->CopyWorld(pFrom);
		Unlink(pFrom);
		Unlink(m_pWorld.get());
	}
	m_pWorld->m_IsValidCopy = true;
	m_pWorld->m_LocalClientId = 0;
	m_Tick = Other.m_Tick;
	m_StartTick = Other.m_StartTick;
	m_FinishTick = Other.m_FinishTick;
	m_Started = Other.m_Started;
	m_Fire = Other.m_Fire;
	m_LastHook = Other.m_LastHook;
	m_RefIdx = Other.m_RefIdx;
	m_CommitLeft = Other.m_CommitLeft;
	m_CommitIn = Other.m_CommitIn;
	m_LastJump = Other.m_LastJump;
	m_HookLoss = Other.m_HookLoss;
	m_TrackCost = Other.m_TrackCost;
	m_Bonus = Other.m_Bonus;
	m_LastShot = Other.m_LastShot;
	m_PendE = Other.m_PendE;
	m_PendT = Other.m_PendT;
	m_PendFire = Other.m_PendFire;
}

void CTasGame::SetState(vec2 Pos, vec2 Vel)
{
	CCharacter *pChr = Chr();
	pChr->m_Core.m_Pos = Pos;
	pChr->m_Core.m_Vel = Vel;
	pChr->m_Pos = pChr->m_PrevPos = pChr->m_PrevPrevPos = Pos;
}

CCharacter *CTasGame::Chr() const { return m_pWorld->m_apCharacters[0]; }
vec2 CTasGame::Pos() const { return Chr()->m_Core.m_Pos; }
vec2 CTasGame::Vel() const { return Chr()->m_Core.m_Vel; }
bool CTasGame::Frozen() const { return Chr()->m_FreezeTime > 0 || Chr()->m_Core.m_DeepFrozen; }

int CTasGame::HookState() const { return Chr()->m_Core.m_HookState; }
vec2 CTasGame::HookPos() const { return Chr()->m_Core.m_HookPos; }
int CTasGame::ReloadTimer() const { return Chr()->m_ReloadTimer; }
int CTasGame::ActiveWeapon() const { return Chr()->m_Core.m_ActiveWeapon; }
bool CTasGame::HasGrenade() const { return Chr()->m_Core.m_aWeapons[WEAPON_GRENADE].m_Got; }
int CTasGame::Jumped() const { return Chr()->m_Core.m_Jumped; }
bool CTasGame::Grounded() const { return Chr()->IsGrounded(); }

bool CTasGame::EnteredFreeze() const
{
	CCharacter *pChr = Chr();
	std::vector<int> vIndices = gs_pCollision->GetMapIndices(pChr->m_PrevPos, pChr->m_Pos);
	if(vIndices.empty())
		vIndices.push_back(gs_pCollision->GetMapIndex(pChr->m_Pos));
	for(int Index : vIndices)
	{
		if(Index < 0)
			continue;
		if(gs_pCollision->GetTileIndex(Index) == TILE_FREEZE || gs_pCollision->GetFrontTileIndex(Index) == TILE_FREEZE)
			return true;
	}
	return false;
}

int CTasGame::NumProjectiles() const
{
	int n = 0;
	for(CEntity *pEnt = m_pWorld->FindFirst(CGameWorld::ENTTYPE_PROJECTILE); pEnt; pEnt = pEnt->TypeNext())
		n++;
	return n;
}

bool CTasGame::NextExplosion(vec2 &Pos, int &Tick) const
{
	bool Found = false;
	for(CEntity *pEnt = m_pWorld->FindFirst(CGameWorld::ENTTYPE_PROJECTILE); pEnt; pEnt = pEnt->TypeNext())
	{
		CProjectile *pProj = (CProjectile *)pEnt;
		if(!pProj->m_Explosive)
			continue;
		int Life = pProj->m_LifeSpan;
		for(int T = m_Tick + 1; T < m_Tick + 200; T++)
		{
			float Pt = (T - pProj->m_StartTick - 1) / (float)SERVER_TICK_SPEED;
			float Ct = (T - pProj->m_StartTick) / (float)SERVER_TICK_SPEED;
			vec2 PrevPos = pProj->GetPos(Pt), CurPos = pProj->GetPos(Ct), ColPos, NewPos;
			int Collide = gs_pCollision->IntersectLine(PrevPos, CurPos, &ColPos, &NewPos);
			if(Life > -1)
				Life--;
			bool Clipped = CurPos.x < -200 || CurPos.y < -200 || CurPos.x > gs_MapInfo.m_W * 32 + 200 || CurPos.y > gs_MapInfo.m_H * 32 + 200;
			if(Collide || Clipped || Life == -1)
			{
				if(!Found || T < Tick)
				{
					Found = true;
					Tick = T;
					Pos = ColPos;
				}
				break;
			}
		}
	}
	return Found;
}

uint64_t CTasGame::Hash() const
{
	// exact physical state (for removing duplicate branches)
	const CCharacterCore &C = Chr()->m_Core;
	uint64_t h = 1469598103934665603ull;
	auto Mix = [&](const void *p, size_t n) {
		const unsigned char *b = (const unsigned char *)p;
		for(size_t i = 0; i < n; i++)
			h = (h ^ b[i]) * 1099511628211ull;
	};
	Mix(&C.m_Pos, sizeof(C.m_Pos));
	Mix(&C.m_Vel, sizeof(C.m_Vel));
	Mix(&C.m_HookPos, sizeof(C.m_HookPos));
	Mix(&C.m_HookDir, sizeof(C.m_HookDir));
	Mix(&C.m_HookState, sizeof(C.m_HookState));
	Mix(&C.m_HookTick, sizeof(C.m_HookTick));
	Mix(&C.m_Jumped, sizeof(C.m_Jumped));
	Mix(&C.m_JumpedTotal, sizeof(C.m_JumpedTotal));
	Mix(&C.m_ActiveWeapon, sizeof(C.m_ActiveWeapon));
	Mix(&Chr()->m_ReloadTimer, sizeof(int));
	Mix(&m_LastHook, sizeof(int));
	Mix(&m_LastJump, sizeof(int));
	Mix(&m_StartTick, sizeof(int));
	Mix(&m_Fire, sizeof(int));
	Mix(&m_CommitLeft, sizeof(int));
	bool G = C.m_aWeapons[WEAPON_GRENADE].m_Got;
	Mix(&G, 1);
	for(CEntity *pEnt = m_pWorld->FindFirst(CGameWorld::ENTTYPE_PROJECTILE); pEnt; pEnt = pEnt->TypeNext())
	{
		Mix(&pEnt->m_Pos, sizeof(vec2));
		Mix(&((CProjectile *)pEnt)->m_StartTick, sizeof(int));
		Mix(&((CProjectile *)pEnt)->m_Direction, sizeof(vec2));
	}
	return h;
}

static bool SegmentHitsFreeze(vec2 A, vec2 B)
{
	const SMapInfo &M = gs_MapInfo;
	float L = distance(A, B);
	int N = (int)(L / 4.0f) + 1;
	for(int i = 1; i <= N; i++)
	{
		vec2 P = mix(A, B, (float)i / N);
		int x = (int)P.x / 32, y = (int)P.y / 32;
		if(M.Tile(x, y) == TILE_FREEZE || M.Front(x, y) == TILE_FREEZE)
			return true;
	}
	return false;
}

int CTasGame::RolloutHook(int Ticks, int Dir, int TX, int TY, vec2 *pPositions) const
{
	CCharacterCore Core = Chr()->m_Core;
	Core.SetCoreWorld(nullptr, gs_pCollision, &Chr()->GameWorld()->m_Teams);
	CNetObj_PlayerInput In = Chr()->m_Input;
	In.m_Direction = Dir;
	In.m_Jump = m_LastJump;
	In.m_Fire = 0;
	In.m_TargetX = TX;
	In.m_TargetY = TY;
	int Hook = m_LastHook;
	for(int t = 0; t < Ticks; t++)
	{
		// release first if the hook is held, then press
		Hook = Hook ? (t == 0 ? 0 : 1) : 1;
		In.m_Hook = Hook;
		Core.m_Input = In;
		vec2 Prev = Core.m_Pos;
		Core.Tick(true);
		Core.Move();
		Core.Quantize();
		if(SegmentHitsFreeze(Prev, Core.m_Pos))
			return t;
		pPositions[t] = Core.m_Pos;
	}
	return Ticks;
}

float CTasGame::CrashLoss(int Ticks) const
{
	CCharacterCore Core = Chr()->m_Core;
	Core.SetCoreWorld(nullptr, gs_pCollision, &Chr()->GameWorld()->m_Teams);
	CNetObj_PlayerInput In = Chr()->m_Input;
	In.m_Jump = m_LastJump;
	In.m_Hook = 0;
	In.m_Fire = 0;
	float Loss = 0;
	for(int t = 0; t < Ticks; t++)
	{
		Core.m_Input = In;
		Core.Tick(true);
		vec2 Before = Core.m_Vel;
		Core.Move();
		vec2 After = Core.m_Vel;
		Loss += std::max(0.0f, dot(Before, Before) - dot(After, After));
		Core.Quantize();
	}
	return Loss;
}

int CTasGame::Rollout(int Ticks, int Dir, bool KeepHook, vec2 *pPositions) const
{
	CCharacterCore Core = Chr()->m_Core;
	Core.SetCoreWorld(nullptr, gs_pCollision, &Chr()->GameWorld()->m_Teams);
	CNetObj_PlayerInput In = Chr()->m_Input;
	In.m_Direction = Dir;
	In.m_Jump = m_LastJump;
	In.m_Hook = KeepHook ? m_LastHook : 0;
	In.m_Fire = 0;
	for(int t = 0; t < Ticks; t++)
	{
		Core.m_Input = In;
		vec2 Prev = Core.m_Pos;
		Core.Tick(true);
		Core.Move();
		Core.Quantize();
		if(SegmentHitsFreeze(Prev, Core.m_Pos))
			return t;
		pPositions[t] = Core.m_Pos;
	}
	return Ticks;
}

void CTasGame::CheckRace()
{
	// Mirrors CCharacter::DDRacePostCoreTick + CGameControllerDDNet::HandleCharacterTiles on the server:
	// the tiles along the previous move plus the four sensitivity points around the current position.
	CCharacter *pChr = Chr();
	CCollision *pCol = gs_pCollision;
	std::vector<int> vIndices = pCol->GetMapIndices(pChr->m_PrevPos, pChr->m_Pos);
	if(vIndices.empty())
		vIndices.push_back(pCol->GetMapIndex(pChr->m_Pos));
	float R = pChr->GetProximityRadius() / 3.f;
	vec2 P = pChr->m_Pos;
	int aS[4] = {
		pCol->GetPureMapIndex(vec2(P.x + R, P.y - R)),
		pCol->GetPureMapIndex(vec2(P.x + R, P.y + R)),
		pCol->GetPureMapIndex(vec2(P.x - R, P.y - R)),
		pCol->GetPureMapIndex(vec2(P.x - R, P.y + R))};
	bool Sens[2] = {false, false};
	for(int S : aS)
	{
		int T = pCol->GetTileIndex(S), F = pCol->GetFrontTileIndex(S);
		Sens[0] |= T == TILE_START || F == TILE_START;
		Sens[1] |= T == TILE_FINISH || F == TILE_FINISH;
	}
	for(int Index : vIndices)
	{
		if(Index < 0)
			continue;
		int T = pCol->GetTileIndex(Index), F = pCol->GetFrontTileIndex(Index);
		bool Start = Sens[0] || T == TILE_START || F == TILE_START;
		bool Finish = Sens[1] || T == TILE_FINISH || F == TILE_FINISH;
		if(Start)
		{
			// staying on the start line keeps restarting the timer (normal); coming back to it later is
			// double start abuse, which we don't allow
			if(m_Started && m_StartTick >= 0 && m_Tick - m_StartTick > 1)
				m_StartTick = -2;
			else if(m_StartTick != -2)
			{
				m_Started = true;
				m_StartTick = m_Tick;
			}
		}
		if(Finish && m_Started && m_FinishTick < 0 && m_StartTick >= 0 && m_Tick > m_StartTick)
			m_FinishTick = m_Tick;
	}
}

void CTasGame::Step(const STasInput &In)
{
	CCharacter *pChr = Chr();
	CNetObj_PlayerInput Input = {};
	Input.m_Direction = In.m_Dir;
	Input.m_TargetX = In.m_TX;
	Input.m_TargetY = In.m_TY;
	Input.m_Jump = In.m_Jump;
	if((m_Fire & 1) != (In.m_Fire ? 1 : 0))
		m_Fire++;
	Input.m_Fire = m_Fire;
	Input.m_Hook = In.m_Hook;
	Input.m_WantedWeapon = In.m_Weapon >= 0 ? In.m_Weapon + 1 : 0;
	Input.m_PlayerFlags = PLAYERFLAG_PLAYING;

	// server order: early input (weapon fire) at the old tick, then the new tick
	m_pWorld->m_GameTick = m_Tick;
	pChr->OnDirectInput(&Input);
	m_Tick++;
	m_pWorld->m_GameTick = m_Tick;
	pChr->OnPredictedInput(&Input);
	// the server's DDRacePostCoreTick checks the previous move during this tick
	CheckRace();
	m_pWorld->Tick();
	m_LastHook = In.m_Hook;
	m_LastJump = In.m_Jump;
}
