// nadelab core: an exact single-tee + grenade stepper with the REAL SERVER's projectile semantics, grenade flight
// prediction and an aim solver. Based on src/tas/fastg.cpp (CFastG), with two differences:
//  * any number of grenades in flight (MAX_PROJ = 16),
//  * a grenade that hits a solid tile on its very last lifetime tick explodes ONCE, like the server's
//    CProjectile::Tick (it returns after the collision). The client prediction code that CTasGame and CFastG run
//    explodes it twice in that case (no return after the collision, then the lifetime branch fires as well).
//    Plans built here never rely on that, and CNadeG::m_DoubleRisk counts such grenades.
#ifndef NADELAB_NADE_H
#define NADELAB_NADE_H

#include "sim.h"

#include <game/gamecore.h>
#include <generated/protocol.h>

#include <cstdint>
#include <vector>

struct SNadeProj
{
	vec2 m_Pos; // start position
	vec2 m_Dir;
	int m_StartTick;
	int m_LifeSpan;
};

struct SNadeExpl
{
	int m_Tick;
	vec2 m_E, m_Tee, m_VelBefore, m_Force;
	float m_Dist;
	bool m_Lifetime; // mid-air lifetime explosion (no collision)
};

class CNadeG
{
public:
	enum
	{
		MAX_PROJ = 16,
	};
	static void Init(); // after CTasGame::LoadMap
	static CTeamsCore ms_Teams;

	CCharacterCore m_Core;
	vec2 m_Pos = vec2(0, 0); // CCharacter::m_Pos (after the last move)
	vec2 m_PrevPos = vec2(0, 0);
	int m_Tick = 0;
	bool m_Dead = false; // touched freeze
	bool m_Bad = false; // something not modelled happened (gun/hammer shot, too many grenades)
	int m_DoubleRisk = 0; // grenades that collided on their last lifetime tick (prediction != server there)
	int m_Fire = 0; // input fire counter

	int m_ReloadTimer = 0;
	int m_QueuedWeapon = -1;
	int m_NumInputs = 0;
	CNetObj_PlayerInput m_Input = {};
	CNetObj_PlayerInput m_LatestInput = {};
	CNetObj_PlayerInput m_LatestPrevInput = {};

	int m_NumProj = 0;
	SNadeProj m_aProj[MAX_PROJ]; // newest first (the world's entity list order)

	std::vector<SNadeExpl> *m_pLog = nullptr; // optional explosion log
	int m_LastFireStep = -1000; // step (tick after the step) of the last grenade fired (bookkeeping only)

	void FromGame(const CTasGame &G);
	void Step(const STasInput &In);
	bool HasGrenade() const { return m_Core.m_aWeapons[WEAPON_GRENADE].m_Got; }
	bool Grounded() const;

private:
	void HandleWeaponSwitch();
	void DoWeaponSwitch();
	void FireWeapon(int GameTick);
	void Explode(vec2 Pos, bool Lifetime);
	void TickProjectiles();
};

// ---- grenade flight ----
struct SNadeFlight
{
	int m_Tau = 0; // explosion happens in the world tick StartTick + Tau (1..101)
	vec2 m_E = vec2(0, 0); // explosion point
	bool m_Collide = false; // hit a tile (else: lifetime end or clipped)
};
// exact flight of a grenade fired from tee position TeePos (CCharacter::m_Pos) in direction Dir (already normalized)
SNadeFlight FlyGrenade(vec2 TeePos, vec2 Dir);
vec2 AimDir(int TX, int TY);
// kick (velocity change) an explosion at E gives a tee at TeePos (zero if out of range)
vec2 ExplosionKick(vec2 TeePos, vec2 E);
// any solid tile touched by the box [x0,x1]x[y0,y1] (pixels)?
bool AnySolid(float x0, float y0, float x1, float y1);
int MapW();
int MapH();

// ---- aim solver ----
// Find an integer aim (TX,TY) so that a grenade fired from FirePos explodes exactly Tau ticks later as close as possible
// to directly below Tee (largest upward kick on a tee at Tee). Tries tile hits (any Tau) and, for Tau = 101, mid-air
// lifetime explosions. AllowLastTickHit: allow a tile hit on the last lifetime tick (server explodes once, prediction twice).
struct SAim
{
	bool m_Ok = false;
	int m_TX = 0, m_TY = 0;
	SNadeFlight m_F;
	vec2 m_Kick = vec2(0, 0);
};
SAim SolveAim(vec2 FirePos, int Tau, vec2 Tee, bool AllowLastTickHit = false, vec2 Want = vec2(0, -1), bool OnlyHits = false);
// cheap necessary-condition filter for SolveAim: can a grenade from FirePos be within reach of Tee exactly Tau ticks later?
bool AimPossible(vec2 FirePos, int Tau, vec2 Tee, float Slack = 4.0f);

std::vector<STasInput> ReadInputFile(const char *pPath);
bool WriteInputFile(const char *pPath, const std::vector<STasInput> &v);

#endif
