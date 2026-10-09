// Fast exact single-tee stepper with the grenade (post-pickup segment): CFast's character physics plus DDNet's
// weapon handling (direct-input and full-auto fire, weapon switch, reload), grenade projectiles (flight, collision,
// lifetime) and explosions (force on the owner), in the same order as the prediction world that CTasGame runs.
// Checked tick by tick against CTasGame by `fastgcheck`.
#ifndef TAS_FASTG_H
#define TAS_FASTG_H

#include "sim.h"

#include <game/gamecore.h>
#include <generated/protocol.h>

#include <cstdint>
#include <vector>

class CTasGame;

struct SFastProj
{
	vec2 m_Pos; // start position
	vec2 m_Dir;
	int m_StartTick;
	int m_LifeSpan;
};

struct SExplLog
{
	int m_Tick;
	vec2 m_E, m_Tee, m_VelBefore, m_Force;
	float m_Dist;
	int m_FireTick = -1; // server tick the grenade was fired
};

struct SStepAudit
{
	vec2 m_V0, m_VExpl, m_VTick, m_VMove; // velocity at the step start, after explosions, after the core tick, after the move
	CCharacterCore m_CoreBeforeTick; // for counterfactual core ticks (hook / dir / jump attribution)
};

class CFastG
{
public:
	static thread_local std::vector<SExplLog> *ms_pLog;
	static thread_local SStepAudit *ms_pAudit; // analysis only: per-stage velocities of the last Step // analysis only (single-threaded tools): explosions are appended here
	enum
	{
		MAX_PROJ = 4,
	};
	// call after CTasGame::LoadMap
	static void Init();
	static CTeamsCore ms_Teams;

	CCharacterCore m_Core;
	vec2 m_Pos = vec2(0, 0); // CCharacter::m_Pos (after the last move)
	vec2 m_PrevPos = vec2(0, 0); // CCharacter::m_PrevPos
	int m_Tick = 0;
	int m_StartTick = -1;
	int m_FinishTick = -1;
	bool m_Started = false;
	bool m_Dead = false; // touched freeze (or double start)
	bool m_Bad = false; // something this stepper does not model happened (e.g. a gun shot)
	int m_Fire = 0; // input fire counter

	// CCharacter weapon state
	int m_ReloadTimer = 0;
	int m_QueuedWeapon = -1;
	int m_NumInputs = 0;
	CNetObj_PlayerInput m_Input = {}; // m_Input (== m_SavedInput)
	CNetObj_PlayerInput m_LatestInput = {};
	CNetObj_PlayerInput m_LatestPrevInput = {};

	int m_NumProj = 0;
	SFastProj m_aProj[MAX_PROJ]; // newest first (the world's entity list order)

	void FromGame(const CTasGame &G);
	void Step(const STasInput &In);
	bool HasGrenade() const { return m_Core.m_aWeapons[WEAPON_GRENADE].m_Got; }
	int RaceTick() const { return m_Tick - m_StartTick; }
	// earliest upcoming explosion of a grenade in flight (point, server tick); false if none
	bool NextExplosion(vec2 &Pos, int &Tick) const;

private:
	void CheckRace();
	void HandleWeaponSwitch();
	void DoWeaponSwitch();
	void FireWeapon(int GameTick);
	void Explode(vec2 Pos, int FireTick = -1);
	void TickProjectiles();
};

#endif
