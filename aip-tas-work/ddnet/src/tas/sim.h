// Fast, copyable DDRace simulation for TAS search, built on the client prediction world.
#ifndef TAS_SIM_H
#define TAS_SIM_H

#include <base/vmath.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class CGameWorld;
class CCharacter;
class CCollision;

struct STasInput
{
	int8_t m_Dir = 0;
	uint8_t m_Jump = 0;
	uint8_t m_Hook = 0;
	uint8_t m_Fire = 0; // held
	int16_t m_TX = 0;
	int16_t m_TY = -1;
	int8_t m_Weapon = -1; // wanted weapon (0 based), -1 = keep
	uint8_t m_Commit = 0; // search only: hold this input for this many ticks (macro move)
};

struct SMapInfo
{
	int m_W = 0, m_H = 0;
	std::vector<uint8_t> m_vGame; // game layer tile index
	std::vector<uint8_t> m_vFront;
	std::vector<vec2> m_vSpawns;
	int Tile(int x, int y) const
	{
		if(x < 0 || y < 0 || x >= m_W || y >= m_H)
			return 1;
		return m_vGame[y * m_W + x];
	}
	int Front(int x, int y) const
	{
		if(x < 0 || y < 0 || x >= m_W || y >= m_H || m_vFront.empty())
			return 0;
		return m_vFront[y * m_W + x];
	}
};

// One simulated world with a single tee.
class CTasGame
{
public:
	static bool LoadMap(const char *pPath);
	static const SMapInfo &Map();
	static CCollision *Collision();

	CTasGame();
	~CTasGame();
	CTasGame(const CTasGame &) = delete;
	CTasGame &operator=(const CTasGame &) = delete;

	void Spawn(vec2 Pos);
	void CopyFrom(const CTasGame &Other);
	void SetState(vec2 Pos, vec2 Vel); // experiments only: teleport the tee
	void Step(const STasInput &In);

	CCharacter *Chr() const;
	vec2 Pos() const;
	vec2 Vel() const;
	bool Frozen() const;
	int HookState() const;
	vec2 HookPos() const;
	int ReloadTimer() const;
	int ActiveWeapon() const;
	bool HasGrenade() const;
	int Jumped() const;
	bool Grounded() const;
	uint64_t Hash() const;
	int NumProjectiles() const;
	// earliest upcoming explosion of a projectile in flight (deterministic in solo: projectiles ignore the tee)
	bool NextExplosion(vec2 &Pos, int &Tick) const;
	// the last move entered freeze (the freeze itself only applies on the next tick)
	bool EnteredFreeze() const;
	// cheap core-only continuation (no weapons/entities); writes positions, returns number of
	// ticks before the tee would touch freeze (== Ticks if never)
	int Rollout(int Ticks, int Dir, bool KeepHook, vec2 *pPositions) const;
	// v^2 lost to collisions with solid tiles over a short continuation (current direction, hook released)
	float CrashLoss(int Ticks) const;
	// same, but (re)fires the hook towards (TX, TY) as early as possible
	int RolloutHook(int Ticks, int Dir, int TX, int TY, vec2 *pPositions) const;
	int m_LastHook = 0;
	int m_RefIdx = 0; // index on the search's reference path (tracked by the search)
	int m_CommitLeft = 0; // search only: ticks left of a committed macro move
	STasInput m_CommitIn;
	int m_LastJump = 0;
	float m_HookLoss = 0; // search only: accumulated v^2 lost while hooked above the hook speed limit
	float m_TrackCost = 0; // search only: accumulated deviation from a reference trajectory
	float m_Bonus = 0; // search only: accumulated bonus (ticks)
	int m_LastShot = -1; // search only: last matched reference shot
	vec2 m_PendE = vec2(0, 0); // search only: cached next explosion of a projectile in flight
	int m_PendT = -1, m_PendFire = -1;
	int m_ReadyTicks = 0; // search only: consecutive ticks with a loaded grenade

	int m_Tick = 0; // server tick of the last simulated step
	int m_StartTick = -1;
	int m_FinishTick = -1;
	bool m_Started = false;
	int m_Fire = 0; // input fire counter

private:
	void CheckRace();
	std::unique_ptr<CGameWorld> m_pWorld;
};

#endif
