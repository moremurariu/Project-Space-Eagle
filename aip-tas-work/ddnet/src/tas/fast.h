// Fast exact single-tee stepper for the segment before the grenade pickup (no weapons fired, no other entities).
// Idea from ddnet_physics: keep the whole state in one small copyable struct and replace per-tick
// allocations / entity scans by precomputed per-tile flags. The physics itself is DDNet's CCharacterCore,
// so results are identical to CTasGame (checked by `fastcheck`).
#ifndef TAS_FAST_H
#define TAS_FAST_H

#include "sim.h"

#include <game/gamecore.h>

#include <cstdint>
#include <vector>

class CTasGame;

class CFast
{
public:
	// call after CTasGame::LoadMap
	static void Init();
	static CTeamsCore ms_Teams;

	CCharacterCore m_Core;
	vec2 m_Pos = vec2(0, 0); // CCharacter::m_Pos (position after the last move)
	vec2 m_PrevPos = vec2(0, 0); // CCharacter::m_PrevPos (position before the last move)
	int m_Tick = 0;
	int m_StartTick = -1;
	bool m_Started = false;
	bool m_Got = false; // grenade picked up
	bool m_Dead = false; // last move touched freeze (or double start)
	int m_Fire = 0;

	void FromGame(const CTasGame &G);
	void Step(const STasInput &In);
	// race tick; before the start line it counts from ms_AssumedStart (a reference run's start tick)
	static int ms_AssumedStart;
	int RaceTick() const { return m_StartTick >= 0 ? m_Tick - m_StartTick : m_Tick - ms_AssumedStart; }

private:
	void CheckRace();
};

#endif
