#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/entities/projectile.h>
#include <game/client/prediction/gameworld.h>
#undef private
#undef protected

#include "fastg.h"

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>

CTeamsCore CFastG::ms_Teams;
thread_local SStepAudit *CFastG::ms_pAudit = nullptr;
thread_local std::vector<SExplLog> *CFastG::ms_pLog = nullptr;

enum
{
	FL_FREEZE = 1,
	FL_START = 2,
	FL_FINISH = 4,
};
static std::vector<uint8_t> gs_vFlags;
static std::vector<vec2> gs_vGrenades;
static int gs_W, gs_H;

void CFastG::Init()
{
	const SMapInfo &M = CTasGame::Map();
	gs_W = M.m_W;
	gs_H = M.m_H;
	gs_vFlags.assign(gs_W * gs_H, 0);
	gs_vGrenades.clear();
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
			if(T != 0 && T != TILE_SOLID && T != TILE_NOHOOK && T != TILE_FREEZE && T != TILE_START && T != TILE_FINISH && T < ENTITY_OFFSET)
			{
				std::printf("CFastG: unsupported tile %d at %d,%d\n", T, x, y);
				std::exit(1);
			}
		}
	if(!M.m_vFront.empty())
	{
		std::printf("CFastG: maps with a front layer are not supported\n");
		std::exit(1);
	}
}

void CFastG::FromGame(const CTasGame &G)
{
	CCharacter *pChr = G.Chr();
	m_Core = pChr->m_Core;
	m_Core.SetCoreWorld(nullptr, CTasGame::Collision(), &ms_Teams);
	m_Pos = pChr->m_Pos;
	m_PrevPos = pChr->m_PrevPos;
	m_Tick = G.m_Tick;
	m_StartTick = G.m_StartTick;
	m_FinishTick = G.m_FinishTick;
	m_Started = G.m_Started;
	m_Dead = G.Frozen();
	m_Bad = false;
	m_Fire = G.m_Fire;
	m_ReloadTimer = pChr->m_ReloadTimer;
	m_QueuedWeapon = pChr->m_QueuedWeapon;
	m_NumInputs = pChr->m_NumInputs;
	m_Input = pChr->m_Input;
	m_LatestInput = pChr->m_LatestInput;
	m_LatestPrevInput = pChr->m_LatestPrevInput;
	m_NumProj = 0;
	for(CEntity *pEnt = pChr->GameWorld()->FindFirst(CGameWorld::ENTTYPE_PROJECTILE); pEnt; pEnt = pEnt->TypeNext())
	{
		CProjectile *pProj = (CProjectile *)pEnt;
		if(pProj->m_MarkedForDestroy)
			continue;
		if(pProj->m_Type != WEAPON_GRENADE || !pProj->m_Explosive || pProj->m_Bouncing || pProj->m_Freeze || m_NumProj >= MAX_PROJ)
		{
			m_Bad = true;
			continue;
		}
		SFastProj &P = m_aProj[m_NumProj++];
		P.m_Pos = pProj->m_Pos;
		P.m_Dir = pProj->m_Direction;
		P.m_StartTick = pProj->m_StartTick;
		P.m_LifeSpan = pProj->m_LifeSpan;
	}
}

static inline int TileAt(float x, float y)
{
	int Nx = std::clamp((int)x / 32, 0, gs_W - 1);
	int Ny = std::clamp((int)y / 32, 0, gs_H - 1);
	return Ny * gs_W + Nx;
}

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

void CFastG::CheckRace()
{
	// as CFast::CheckRace (start) plus the finish line (CTasGame::CheckRace)
	uint8_t Move = MoveFlags(m_PrevPos, m_Pos);
	float R = CCharacterCore::PhysicalSize() / 3.f;
	vec2 P = m_Pos;
	uint8_t Sens = gs_vFlags[PureIndex(P.x + R, P.y - R)] | gs_vFlags[PureIndex(P.x + R, P.y + R)] |
		       gs_vFlags[PureIndex(P.x - R, P.y - R)] | gs_vFlags[PureIndex(P.x - R, P.y + R)];
	uint8_t Here = gs_vFlags[TileAt(P.x, P.y)];
	bool Any = Move != 0 || Here != 0;
	uint8_t Iter = Move != 0 ? Move : Here;
	bool Start = Any && ((Iter & FL_START) || (Sens & FL_START));
	bool Finish = Any && ((Iter & FL_FINISH) || (Sens & FL_FINISH));
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
	if(Finish && m_Started && m_FinishTick < 0 && m_StartTick >= 0 && m_Tick > m_StartTick)
		m_FinishTick = m_Tick;
}

void CFastG::DoWeaponSwitch()
{
	if(m_ReloadTimer != 0 || m_QueuedWeapon == -1)
		return;
	if(m_Core.m_aWeapons[WEAPON_NINJA].m_Got || !m_Core.m_aWeapons[m_QueuedWeapon].m_Got)
		return;
	// SetWeapon
	if(m_QueuedWeapon == m_Core.m_ActiveWeapon)
		return;
	int W = m_QueuedWeapon;
	m_QueuedWeapon = -1;
	m_Core.m_ActiveWeapon = (W < WEAPON_HAMMER || W >= NUM_WEAPONS) ? -1 : W;
}

void CFastG::HandleWeaponSwitch()
{
	if(m_NumInputs < 2)
		return;
	int WantedWeapon = m_Core.m_ActiveWeapon;
	if(m_QueuedWeapon != -1)
		WantedWeapon = m_QueuedWeapon;
	bool Anything = false;
	for(int i = 0; i < NUM_WEAPONS - 1; ++i)
		if(m_Core.m_aWeapons[i].m_Got)
			Anything = true;
	if(!Anything)
		return;
	// next / prev weapon presses are never used by the TAS inputs (always 0)
	if(m_LatestInput.m_WantedWeapon)
		WantedWeapon = m_Input.m_WantedWeapon - 1;
	if(WantedWeapon >= 0 && WantedWeapon < NUM_WEAPONS && WantedWeapon != m_Core.m_ActiveWeapon && m_Core.m_aWeapons[WantedWeapon].m_Got)
		m_QueuedWeapon = WantedWeapon;
	DoWeaponSwitch();
}

void CFastG::FireWeapon(int GameTick)
{
	if(m_NumInputs < 2)
		return;
	if(m_ReloadTimer != 0)
		return;
	DoWeaponSwitch();
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
	const int W = m_Core.m_ActiveWeapon;
	bool FullAuto = W == WEAPON_GRENADE || W == WEAPON_SHOTGUN || W == WEAPON_LASER;
	bool WillFire = CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses;
	if(FullAuto && (m_LatestInput.m_Fire & 1) && W >= 0 && m_Core.m_aWeapons[W].m_Ammo)
		WillFire = true;
	if(!WillFire)
		return;
	if(W < 0 || !m_Core.m_aWeapons[W].m_Ammo)
		return;
	if(W != WEAPON_GRENADE)
	{
		// hammer / gun shots are not modelled (the search only fires the grenade)
		m_Bad = true;
		return;
	}
	vec2 ProjStartPos = m_Pos + Direction * CCharacterCore::PhysicalSize() * 0.75f;
	if(m_NumProj >= MAX_PROJ)
	{
		m_Bad = true;
		return;
	}
	// new entities go to the front of the type list
	for(int i = m_NumProj; i > 0; i--)
		m_aProj[i] = m_aProj[i - 1];
	m_NumProj++;
	SFastProj &P = m_aProj[0];
	P.m_Pos = ProjStartPos;
	P.m_Dir = Direction;
	P.m_StartTick = GameTick;
	P.m_LifeSpan = (int)(SERVER_TICK_SPEED * m_Core.m_Tuning.m_GrenadeLifetime);
	m_ReloadTimer = m_Core.m_Tuning.GetWeaponFireDelay(W) * SERVER_TICK_SPEED;
}

void CFastG::Explode(vec2 Pos)
{
	const float Radius = 135.0f, InnerRadius = 48.0f;
	if(!(distance(m_Pos, Pos) < Radius + CCharacterCore::PhysicalSize()))
		return;
	vec2 Diff = m_Pos - Pos;
	vec2 ForceDir(0, 1);
	float l = length(Diff);
	if(l)
		ForceDir = normalize(Diff);
	l = 1 - std::clamp((l - InnerRadius) / (Radius - InnerRadius), 0.0f, 1.0f);
	float Strength = m_Core.m_Tuning.m_ExplosionStrength;
	float Dmg = Strength * l;
	if(ms_pLog)
		ms_pLog->push_back({m_Tick, Pos, m_Pos, m_Core.m_Vel, (int)Dmg ? ForceDir * Dmg * 2 : vec2(0, 0), length(Diff)});
	if((int)Dmg)
	{
		// TakeDamage: no move restrictions on this map (ClampVel is the identity)
		m_Core.m_Vel = m_Core.m_Vel + ForceDir * Dmg * 2;
	}
}

static inline bool Clipped(vec2 P)
{
	return round_to_int(P.x) / 32 < -200 || round_to_int(P.x) / 32 > gs_W + 200 || round_to_int(P.y) / 32 < -200 || round_to_int(P.y) / 32 > gs_H + 200;
}

void CFastG::TickProjectiles()
{
	const float Curvature = m_Core.m_Tuning.m_GrenadeCurvature, Speed = m_Core.m_Tuning.m_GrenadeSpeed;
	int Keep = 0;
	SFastProj aKeep[MAX_PROJ];
	for(int i = 0; i < m_NumProj; i++)
	{
		SFastProj &P = m_aProj[i];
		float Pt = (m_Tick - P.m_StartTick - 1) / (float)SERVER_TICK_SPEED;
		float Ct = (m_Tick - P.m_StartTick) / (float)SERVER_TICK_SPEED;
		vec2 PrevPos = CalcPos(P.m_Pos, P.m_Dir, Curvature, Speed, Pt);
		vec2 CurPos = CalcPos(P.m_Pos, P.m_Dir, Curvature, Speed, Ct);
		vec2 ColPos, NewPos;
		int Collide = CTasGame::Collision()->IntersectLine(PrevPos, CurPos, &ColPos, &NewPos);
		if(P.m_LifeSpan > -1)
			P.m_LifeSpan--;
		bool Destroy = false;
		if(Collide || Clipped(CurPos))
		{
			// server CProjectile::Tick explodes once and returns; the client prediction world would also run the
			// lifetime explosion when the hit falls on the last lifetime tick (nadelab: server-checked)
			Explode(ColPos);
			Destroy = true;
		}
		else if(P.m_LifeSpan == -1)
		{
			Explode(ColPos);
			Destroy = true;
		}
		if(!Destroy)
			aKeep[Keep++] = P;
	}
	for(int i = 0; i < Keep; i++)
		m_aProj[i] = aKeep[i];
	m_NumProj = Keep;
}

bool CFastG::NextExplosion(vec2 &Pos, int &Tick) const
{
	const float Curvature = m_Core.m_Tuning.m_GrenadeCurvature, Speed = m_Core.m_Tuning.m_GrenadeSpeed;
	bool Found = false;
	for(int i = 0; i < m_NumProj; i++)
	{
		const SFastProj &P = m_aProj[i];
		int Life = P.m_LifeSpan;
		for(int T = m_Tick + 1; T < m_Tick + 200; T++)
		{
			float Pt = (T - P.m_StartTick - 1) / (float)SERVER_TICK_SPEED;
			float Ct = (T - P.m_StartTick) / (float)SERVER_TICK_SPEED;
			vec2 PrevPos = CalcPos(P.m_Pos, P.m_Dir, Curvature, Speed, Pt);
			vec2 CurPos = CalcPos(P.m_Pos, P.m_Dir, Curvature, Speed, Ct);
			vec2 ColPos, NewPos;
			int Collide = CTasGame::Collision()->IntersectLine(PrevPos, CurPos, &ColPos, &NewPos);
			if(Life > -1)
				Life--;
			if(Collide || Clipped(CurPos) || Life == -1)
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

void CFastG::Step(const STasInput &In)
{
	if((m_Fire & 1) != (In.m_Fire ? 1 : 0))
		m_Fire++;
	CNetObj_PlayerInput Input = {};
	Input.m_Direction = In.m_Dir;
	Input.m_TargetX = In.m_TX;
	Input.m_TargetY = In.m_TY;
	Input.m_Jump = In.m_Jump;
	Input.m_Fire = m_Fire;
	Input.m_Hook = In.m_Hook;
	Input.m_WantedWeapon = In.m_Weapon >= 0 ? In.m_Weapon + 1 : 0;
	Input.m_PlayerFlags = PLAYERFLAG_PLAYING;

	// OnDirectInput at the old tick
	m_NumInputs++;
	m_LatestPrevInput = m_LatestInput;
	m_LatestInput = Input;
	if(m_LatestInput.m_TargetX == 0 && m_LatestInput.m_TargetY == 0)
		m_LatestInput.m_TargetY = -1;
	if(m_NumInputs > 1)
	{
		HandleWeaponSwitch();
		FireWeapon(m_Tick);
	}
	m_LatestPrevInput = m_LatestInput;

	m_Tick++;
	// OnPredictedInput
	m_Input = Input;
	if(m_Input.m_TargetX == 0 && m_Input.m_TargetY == 0)
		m_Input.m_TargetY = -1;
	CheckRace();

	// world tick: projectiles, then pickups, then the character
	if(ms_pAudit)
		ms_pAudit->m_V0 = m_Core.m_Vel;
	if(m_NumProj)
		TickProjectiles();
	if(ms_pAudit)
		ms_pAudit->m_VExpl = m_Core.m_Vel;
	if(!m_Core.m_aWeapons[WEAPON_GRENADE].m_Got)
		for(const vec2 &G : gs_vGrenades)
			if(distance(G, m_Pos) < 20.0f + CCharacterCore::PhysicalSize())
			{
				m_Core.m_aWeapons[WEAPON_GRENADE].m_Got = true;
				m_Core.m_aWeapons[WEAPON_GRENADE].m_Ammo = -1;
			}

	// CCharacter::Tick: PreTick (DDRaceTick restores m_Input from m_SavedInput == m_Input)
	m_Core.m_Input = m_Input;
	if(ms_pAudit)
		ms_pAudit->m_CoreBeforeTick = m_Core;
	m_Core.Tick(true, true);
	if(ms_pAudit)
		ms_pAudit->m_VTick = m_Core.m_Vel;
	// HandleWeapons
	if(m_ReloadTimer)
		m_ReloadTimer--;
	else
		FireWeapon(m_Tick);
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
	if(ms_pAudit)
		ms_pAudit->m_VMove = m_Core.m_Vel;
	m_Core.Quantize();
	m_Pos = m_Core.m_Pos;
	uint8_t Move = MoveFlags(m_PrevPos, m_Pos);
	if(!Move)
		Move = gs_vFlags[TileAt(m_Pos.x, m_Pos.y)];
	if(Move & FL_FREEZE)
		m_Dead = true;
	if(m_StartTick == -2)
		m_Dead = true;
}
