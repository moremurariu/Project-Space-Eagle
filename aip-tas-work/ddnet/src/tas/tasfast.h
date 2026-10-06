// CTasFast: the exact single-tee grenade stepper (CFastG) with the CTasGame interface the searches use, so seg can
// run on it (segf, -DSEG_FAST). Prefixes are still replayed on CTasGame and converted with FromGameFull.
#ifndef TAS_TASFAST_H
#define TAS_TASFAST_H

#include "fastg.h"
#include "sim.h"

#include <game/collision.h>
#include <game/mapitems.h>

class CTasFast : public CFastG
{
public:
	int m_LastHook = 0;
	int m_RefIdx = 0;
	int m_CommitLeft = 0;
	STasInput m_CommitIn;
	int m_LastJump = 0;
	float m_HookLoss = 0;
	float m_TrackCost = 0;
	float m_Bonus = 0;
	int m_LastShot = -1;
	vec2 m_PendE = vec2(0, 0);
	int m_PendT = -1, m_PendFire = -1;
	int m_ReadyTicks = 0;
	float m_PendCredit = 0;
	int m_RetroMin = 0; // retro: ancestors before this tick can't fire a retro shot (a retro shot changed their reload) // pfcred: credit (ticks) for the grenade in flight, until it explodes

	void CopyFrom(const CTasFast &Other) { *this = Other; }
	void FromGameFull(const CTasGame &G)
	{
		FromGame(G);
		m_LastHook = G.m_LastHook;
		m_RefIdx = G.m_RefIdx;
		m_CommitLeft = G.m_CommitLeft;
		m_CommitIn = G.m_CommitIn;
		m_LastJump = G.m_LastJump;
		m_HookLoss = G.m_HookLoss;
		m_TrackCost = G.m_TrackCost;
		m_Bonus = G.m_Bonus;
		m_LastShot = G.m_LastShot;
		m_PendE = G.m_PendE;
		m_PendT = G.m_PendT;
		m_PendFire = G.m_PendFire;
		m_ReadyTicks = G.m_ReadyTicks;
	}
	void Step(const STasInput &In)
	{
		CFastG::Step(In);
		m_LastHook = In.m_Hook;
		m_LastJump = In.m_Jump;
	}
	void SetState(vec2 Pos, vec2 Vel)
	{
		m_Core.m_Pos = Pos;
		m_Core.m_Vel = Vel;
		m_Pos = m_PrevPos = Pos;
	}
	const CCharacterCore &Core() const { return m_Core; }
	vec2 Pos() const { return m_Core.m_Pos; }
	vec2 Vel() const { return m_Core.m_Vel; }
	// the dead flag covers both (sticky: set by the move that touched freeze)
	bool Frozen() const { return m_Dead; }
	bool EnteredFreeze() const { return m_Dead; }
	int HookState() const { return m_Core.m_HookState; }
	vec2 HookPos() const { return m_Core.m_HookPos; }
	int ReloadTimer() const { return m_ReloadTimer; }
	int ActiveWeapon() const { return m_Core.m_ActiveWeapon; }
	int Jumped() const { return m_Core.m_Jumped; }
	bool Grounded() const { return CTasGame::Collision()->IsOnGround(m_Pos, CCharacterCore::PhysicalSize()); }
	int NumProjectiles() const { return m_NumProj; }

	uint64_t Hash() const
	{
		// same bytes in the same order as CTasGame::Hash
		const CCharacterCore &C = m_Core;
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
		Mix(&m_ReloadTimer, sizeof(int));
		Mix(&m_LastHook, sizeof(int));
		Mix(&m_LastJump, sizeof(int));
		Mix(&m_StartTick, sizeof(int));
		Mix(&m_Fire, sizeof(int));
		Mix(&m_CommitLeft, sizeof(int));
		bool G = C.m_aWeapons[WEAPON_GRENADE].m_Got;
		Mix(&G, 1);
		for(int i = 0; i < m_NumProj; i++)
		{
			Mix(&m_aProj[i].m_Pos, sizeof(vec2));
			Mix(&m_aProj[i].m_StartTick, sizeof(int));
			Mix(&m_aProj[i].m_Dir, sizeof(vec2));
		}
		return h;
	}

	// as CTasGame::Rollout / RolloutHook / CrashLoss
	int Rollout(int Ticks, int Dir, bool KeepHook, vec2 *pPositions) const
	{
		CCharacterCore Core = m_Core;
		Core.SetCoreWorld(nullptr, CTasGame::Collision(), &ms_Teams);
		CNetObj_PlayerInput In = m_Input;
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
	int RolloutHook(int Ticks, int Dir, int TX, int TY, vec2 *pPositions) const
	{
		CCharacterCore Core = m_Core;
		Core.SetCoreWorld(nullptr, CTasGame::Collision(), &ms_Teams);
		CNetObj_PlayerInput In = m_Input;
		In.m_Direction = Dir;
		In.m_Jump = m_LastJump;
		In.m_Fire = 0;
		In.m_TargetX = TX;
		In.m_TargetY = TY;
		int Hook = m_LastHook;
		for(int t = 0; t < Ticks; t++)
		{
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
	float CrashLoss(int Ticks) const
	{
		CCharacterCore Core = m_Core;
		Core.SetCoreWorld(nullptr, CTasGame::Collision(), &ms_Teams);
		CNetObj_PlayerInput In = m_Input;
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

private:
	static bool SegmentHitsFreeze(vec2 A, vec2 B)
	{
		const SMapInfo &M = CTasGame::Map();
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
};

#endif
