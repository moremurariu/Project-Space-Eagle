#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/gameworld.h>
#undef private
#undef protected

#include "fast.h"

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>

CTeamsCore CFast::ms_Teams;
int CFast::ms_AssumedStart = 0;

enum
{
	FL_FREEZE = 1,
	FL_START = 2,
	FL_FINISH = 4,
};
static std::vector<uint8_t> gs_vFlags;
static std::vector<vec2> gs_vGrenades;
static int gs_W, gs_H;

void CFast::Init()
{
	const SMapInfo &M = CTasGame::Map();
	gs_W = M.m_W;
	gs_H = M.m_H;
	gs_vFlags.assign(gs_W * gs_H, 0);
	for(int y = 0; y < gs_H; y++)
		for(int x = 0; x < gs_W; x++)
		{
			int T = M.Tile(x, y), F = M.Front(x, y);
			uint8_t &Fl = gs_vFlags[y * gs_W + x];
			if(T == TILE_FREEZE || F == TILE_FREEZE)
				Fl |= FL_FREEZE;
			if(T == TILE_START || F == TILE_START)
				Fl |= FL_START;
			if(T == TILE_FINISH || F == TILE_FINISH)
				Fl |= FL_FINISH;
			if(T == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
				gs_vGrenades.emplace_back(x * 32.0f + 16.0f, y * 32.0f + 16.0f);
			// the fast stepper only knows these tiles; anything else must not exist in the game layer
			if(T != 0 && T != TILE_SOLID && T != TILE_NOHOOK && T != TILE_FREEZE && T != TILE_START && T != TILE_FINISH && T < ENTITY_OFFSET)
			{
				std::printf("CFast: unsupported tile %d at %d,%d\n", T, x, y);
				std::exit(1);
			}
		}
	if(!M.m_vFront.empty())
	{
		std::printf("CFast: maps with a front layer are not supported\n");
		std::exit(1);
	}
}

void CFast::FromGame(const CTasGame &G)
{
	m_Core = G.Chr()->m_Core;
	m_Core.SetCoreWorld(nullptr, CTasGame::Collision(), &ms_Teams);
	m_Pos = G.Chr()->m_Pos;
	m_PrevPos = G.Chr()->m_PrevPos;
	m_Tick = G.m_Tick;
	m_StartTick = G.m_StartTick;
	m_Started = G.m_Started;
	m_Got = G.HasGrenade();
	m_Dead = G.Frozen();
	m_Fire = G.m_Fire;
}

static inline int TileAt(float x, float y)
{
	int Nx = std::clamp((int)x / 32, 0, gs_W - 1);
	int Ny = std::clamp((int)y / 32, 0, gs_H - 1);
	return Ny * gs_W + Nx;
}

// flags of all tiles CCollision::GetMapIndices(PrevPos, Pos) visits (same sampling), or of the tile at Pos if none
static inline uint8_t MoveFlags(vec2 PrevPos, vec2 Pos)
{
	float d = distance(PrevPos, Pos);
	if(!d)
		return gs_vFlags[TileAt(Pos.x, Pos.y)];
	int End(d + 1);
	uint8_t Fl = 0;
	for(int i = 0; i < End; i++)
	{
		float a = i / d;
		vec2 Tmp = mix(PrevPos, Pos, a);
		Fl |= gs_vFlags[TileAt(Tmp.x, Tmp.y)];
	}
	return Fl;
}

static inline int PureIndex(float x, float y)
{
	int Nx = std::clamp(round_to_int(x) / 32, 0, gs_W - 1);
	int Ny = std::clamp(round_to_int(y) / 32, 0, gs_H - 1);
	return Ny * gs_W + Nx;
}

void CFast::CheckRace()
{
	// same as CTasGame::CheckRace (start line along the previous move + the four sensitivity points)
	uint8_t Move = MoveFlags(m_PrevPos, m_Pos);
	float R = CCharacterCore::PhysicalSize() / 3.f;
	vec2 P = m_Pos;
	uint8_t Sens = gs_vFlags[PureIndex(P.x + R, P.y - R)] | gs_vFlags[PureIndex(P.x + R, P.y + R)] |
		       gs_vFlags[PureIndex(P.x - R, P.y - R)] | gs_vFlags[PureIndex(P.x - R, P.y + R)];
	// CTasGame::CheckRace iterates over the existing tiles along the move (on this map: exactly the flagged
	// ones), or the tile at the position if there are none; the sensitivity points only count inside that loop
	uint8_t Here = gs_vFlags[TileAt(P.x, P.y)];
	bool Any = Move != 0 || Here != 0;
	uint8_t Iter = Move != 0 ? Move : Here;
	bool Start = Any && ((Iter & FL_START) || (Sens & FL_START));
	if(Start)
	{
		if(m_Started && m_StartTick >= 0 && m_Tick - m_StartTick > 1)
			m_StartTick = -2;
		else if(m_StartTick != -2)
		{
			m_Started = true;
			m_StartTick = m_Tick;
		}
	}
}

void CFast::Step(const STasInput &In)
{
	// OnDirectInput: weapons are not handled here (the segment before the grenade never fires)
	if((m_Fire & 1) != (In.m_Fire ? 1 : 0))
		m_Fire++;
	m_Tick++;
	CNetObj_PlayerInput Input = {};
	Input.m_Direction = In.m_Dir;
	Input.m_TargetX = In.m_TX;
	Input.m_TargetY = In.m_TY;
	Input.m_Jump = In.m_Jump;
	Input.m_Fire = m_Fire;
	Input.m_Hook = In.m_Hook;
	Input.m_WantedWeapon = In.m_Weapon >= 0 ? In.m_Weapon + 1 : 0;
	Input.m_PlayerFlags = PLAYERFLAG_PLAYING;
	if(Input.m_TargetX == 0 && Input.m_TargetY == 0)
		Input.m_TargetY = -1;
	CheckRace();

	// pickups tick before the character: distance to the tee's position after its last move
	for(const vec2 &G : gs_vGrenades)
		if(distance(G, m_Pos) < 20.0f + CCharacterCore::PhysicalSize())
			m_Got = true;

	// CCharacter::Tick -> PreTick
	m_Core.m_Input = Input;
	m_Core.Tick(true, true);
	// DDRacePostCoreTick: jump rules
	if(m_Core.m_Jumps == -1 || m_Core.m_Jumps == 0)
		m_Core.m_Jumped |= 2;
	else if(m_Core.m_Jumps == 1 && m_Core.m_Jumped > 0)
		m_Core.m_Jumped |= 2;
	else if(m_Core.m_JumpedTotal < m_Core.m_Jumps - 1 && m_Core.m_Jumped > 1)
		m_Core.m_Jumped = 1;
	if((m_Core.m_Super || m_Core.m_EndlessJump) && m_Core.m_Jumped > 1)
		m_Core.m_Jumped = 1;
	m_PrevPos = m_Core.m_Pos;
	// TickDeferred
	m_Core.Move();
	m_Core.Quantize();
	m_Pos = m_Core.m_Pos;
	// HandleTiles: the existing tiles along the move, or the tile at the new position if there are none
	uint8_t Move = MoveFlags(m_PrevPos, m_Pos);
	if(!Move)
		Move = gs_vFlags[TileAt(m_Pos.x, m_Pos.y)];
	if(Move & FL_FREEZE)
		m_Dead = true;
	if(m_StartTick == -2)
		m_Dead = true;
}
