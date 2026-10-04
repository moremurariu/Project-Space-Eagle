// The hookfly/aled bot's brain, shared by the server-side bot (src/game/server/hookbot.h, a debug dummy) and the
// client-side one (drives your own dummy, see CGameClient::HookBotInput). It only sees plain tee state, so both can
// feed it (see docs/HOOKBOT.md).
#ifndef GAME_HOOKBOT_H
#define GAME_HOOKBOT_H

#include <base/vmath.h>

#include <game/gamecore.h>

#include <generated/protocol.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <string>
#include <vector>

class CCollision;
class CTeamsCore;

// a tee at the end of a tick, as the server sees it (m_Core.m_Id must be the client id)
struct SHookBotTee
{
	CCharacterCore m_Core;
	vec2 m_PrevPos; // position before the last move
	int m_FreezeTime = 0;
	int m_Reload = 0;
	bool m_Grounded = false;
	CNetObj_PlayerInput m_Input = {}; // last applied input
};

// Two tees stepped with the shared core physics plus freeze, in the server's tick order.
// Used for short lookaheads; weapons and most tiles are not simulated.
class CHookBotSim
{
public:
	struct STee
	{
		CCharacterCore m_Core;
		int m_FreezeTime = 0;
		vec2 m_PrevPos;
		bool m_EnteredFreeze = false; // the last move touched freeze
		vec2 m_EnteredFrom = vec2(-1e9f, -1e9f), m_EnteredTo = vec2(-1e9f, -1e9f); // the move m_EnteredFreeze is for (the next Step checks that move again, unless it changed)
		bool m_Dead = false; // a kill tile got it (checked like CCharacter::HandleSkippableTiles)
		CNetObj_PlayerInput m_PrevInput = {};
		int m_Reload = 0;
	};
	CWorldCore m_World;
	STee m_aTee[2];
	int m_aOrder[2] = {0, 1};
	// the game tick the next Step plays (the freeze in a freeze tile is renewed once a second, CCharacter::Freeze: out of
	// it, a tee that lay there thaws 100-150 ticks later, not whenever its first freeze would have run out)
	int m_Tick = 0;
	CCollision *m_pCollision = nullptr;
	CTeamsCore *m_pTeams = nullptr;

	CHookBotSim() = default;
	CHookBotSim(const CHookBotSim &Other) { *this = Other; }
	CHookBotSim &operator=(const CHookBotSim &Other);
	// AFirst: A's character ticks before B's (the server ticks the newest character first)
	void Init(CCollision *pCollision, CTeamsCore *pTeams, const SHookBotTee &A, const SHookBotTee &B, bool AFirst);
	// Only >= 0: step just that tee (the other stays where it is, still in the world); for cheap lookaheads where
	// the other one does nothing
	void Step(const CNetObj_PlayerInput &InA, const CNetObj_PlayerInput &InB, int Only = -1);
	bool PathTouchesFreeze(vec2 From, vec2 To) const;
	// the two tees trade places (a planner that wants the other one as tee 0)
	void Swap();
	bool InFreeze(vec2 Pos) const;
	bool TouchesDeath(vec2 Pos) const;

private:
	void Link();
};

// walking distance (px) from every tile to a goal: through air, and through freeze at a high cost, never through
// walls or kill tiles. Tells a planner how much closer to the goal a position is, around corners and down shafts.
class CHookBotGoalField
{
public:
	// FreezeCost: per-tile cost multiplier through freeze; 0: freeze is impassable (only open air counts)
	void Build(const CCollision *pCollision, vec2 GoalTile, float FreezeCost = 8.0f);
	// the same, keeping to a line (tiles): tiles farther than Radius tiles from it cost FarCost times as much
	void BuildAlong(const CCollision *pCollision, vec2 GoalTile, float FreezeCost, const std::vector<vec2> &vLine, float Radius, float FarCost);
	float Dist(vec2 Pos) const; // interpolated; large if unreachable
	// of the tiles reachable in this field, the one closest to the goal of By (tiles)
	vec2 BestReachable(const CHookBotGoalField &By) const;
	const void *DistData() const { return m_pDist.get(); }
	vec2 m_GoalTile = vec2(0, 0);

private:
	int m_W = 0, m_H = 0;
	// shared with every field built from the same goal tile, costs and line (they're cached: a full-map search each)
	std::shared_ptr<const std::vector<float>> m_pDist;
};

// the map-wide caches (goal fields, tile kinds, open-air regions) are per map: drops them when the map changed (by
// its tiles), e.g. a server's map change with the same CCollision
void HookBotMapCheck(const CCollision *pCollision);

class CHookBotBrain
{
public:
	enum
	{
		MODE_IDLE,
		MODE_HOOKFLY,
		MODE_ALED,
		MODE_PSEUDOFLY, // I hammer, the partner drives (hooks me and steers)
		MODE_PSEUDODRIVE, // I drive (hook the partner and steer), the partner hammers me
		MODE_HAMMERHIT, // over a freeze floor: whoever is free hooks the frozen one out and hammers it (m_DriveDir)
		MODE_PLAY, // get both of us to m_pGoal, picking the technique: rescues (hammerhit, aleds) or a pseudofly up
	};

	CHookBotBrain() = default;
	CHookBotBrain(const CHookBotBrain &) = delete;
	CHookBotBrain &operator=(const CHookBotBrain &) = delete;
	~CHookBotBrain() { FinishJob(); }
	void Init(CCollision *pCollision, CTeamsCore *pTeams)
	{
		m_pCollision = pCollision;
		m_pTeams = pTeams;
		HookBotMapCheck(pCollision);
	}
	void Start(int Mode);
	// the input for the coming tick, from both tees at the end of the previous one (GameTick: the coming tick)
	CNetObj_PlayerInput Tick(int GameTick, const SHookBotTee &Bot, const SHookBotTee &Partner, bool BotFirst);
	// parses a chat line that pinged the bot; false if it wasn't a command
	static int ParseCommand(const char *pMessage, const char *pName);
	// "left" / "right" / "up" in a chat line that pinged the bot: the direction it drives a pseudofly in
	// (-1, 1, 0), or 2 if the line has none
	static int ParseDirection(const char *pMessage);
	// "goal here" (-> *pHere) or "goal <x> <y>" in tiles; false if the line sets no goal
	static bool ParseGoal(const char *pMessage, bool *pHere, vec2 *pTile);
	// hammerhit towards a goal instead of m_DriveDir (built once; shared with the planning thread)
	void SetGoal(vec2 Tile);
	std::shared_ptr<const CHookBotGoalField> m_pGoal;
	std::shared_ptr<const CHookBotGoalField> m_pGoalAir; // the same without passing freeze: where flying can get us
	// for moves on my own: further along the route (m_SoloLead waypoints on; with a goal nearby, hovering at it scored as
	// well as flying through it fast, and a long freeze floor further on needed the speed)
	std::shared_ptr<const CHookBotGoalField> m_pGoalSolo;
	// further on still (2 x m_SoloLead waypoints): for the team moves that cross freeze for good, which only pay off well
	// beyond it (a fall caught onto the ledge past Stronghold's freeze shaft is further from the pool beside it, the
	// goal 7 waypoints on)
	std::shared_ptr<const CHookBotGoalField> m_pGoalFar;
	int m_SoloLead = 7;

	// a route (waypoints in tiles, e.g. from scripts/demo_route.py over a rank 1 run): in play mode the goal moves on to
	// the next waypoint once we're both near the current one; the next goal's fields are built on a thread
	static bool ParseRoute(const char *pText, std::vector<vec2> *pOut); // the first path only
	// every path in a route file: one per tee of the run, each after a "# path" line (scripts/demo_route.py --cid 0,1)
	static bool ParseRoutes(const char *pText, std::vector<std::vector<vec2>> *pOut);
	// hookbot/routes/<map>.txt for a loaded map file: without the "_<sha256>" / "_<crc>" a downloaded map's file name has
	static void RoutePath(const char *pMapBaseName, char *pOut, int OutSize);
	// starts at the waypoint ahead of us (or at Index)
	void StartRoute(const std::vector<vec2> &vRoute, vec2 Me, vec2 Partner, int Index = -1);
	// a route file's text, all its paths: I follow the one I'm on, and switch where the team split up and I'm on the
	// other tee's line (Stronghold after the fly: one goes down the freeze column right of the ledge, the other into the
	// shaft). False if it has no waypoints; *pMsg: what to say
	bool StartRouteText(const char *pText, vec2 Me, vec2 Partner, char *pMsg, int MsgSize);
	std::vector<vec2> m_vRoute; // the path I follow
	std::vector<std::vector<vec2>> m_vvRoutes; // all paths (empty: just m_vRoute)
	int m_RoutePath = 0; // which of them m_vRoute is
	int m_NextPathCheck = 0;
	int m_RouteIndex = -1;
	float m_RouteNear = 6 * 32, m_RouteFar = 14 * 32; // advance once one of us is within Near and both within Far
	// each of us on our own along the route ([0] me, [1] the partner; from 12 before m_RouteIndex to 6 past it): my solo
	// moves aim m_SoloLead waypoints past mine (the shared index moves on with the one of us ahead, and for the other a
	// solo goal on from there was 14 waypoints off: it only hovered, Stronghold's hookable blocks after the zig-zag's
	// room)
	int m_aOwnIndex[2] = {-1, -1};
	int m_OwnPath = -1;
	struct SOwnJob
	{
		std::thread m_Thread;
		std::atomic<bool> m_Done{false};
		vec2 m_Tile;
		std::vector<vec2> m_vLine; // the route from my own index to the tile
		std::shared_ptr<CHookBotGoalField> m_pField;
	};
	std::unique_ptr<SOwnJob> m_pOwnJob;
	std::shared_ptr<const CHookBotGoalField> m_pOwnSolo; // for m_OwnSoloTile
	vec2 m_OwnSoloTile = vec2(-1, -1);
	void UpdateOwnRoute(bool Check);
	// the goal fields aim this many waypoints further on (the waypoints of a fast run often hang in mid-air: a goal
	// right there would have the rescues fling the partner up towards it instead of on along the way)
	int m_RouteLead = 3;
	vec2 m_aRouteLastPos[2] = {vec2(0, 0), vec2(0, 0)}; // to notice a kill / respawn / teleport
	vec2 RouteGoal(int Index) const { return m_vRoute[std::min(Index + m_RouteLead, (int)m_vRoute.size() - 1)]; }
	std::function<void(const char *)> m_Say = [](const char *) {};
	// play mode: kill my tee (a restart from the spawn), when we're stuck for good: both of us lying frozen in freeze
	// for a moment, or one of us for long with no rescue coming. Both brains see it on the same tick
	std::function<void()> m_Kill = [] {};
	int m_ProgressIndex = -1, m_ProgressSince = 0; // the route's waypoint, and since when it hasn't moved on
	int m_StuckSince = -1, m_BothStuckSince = -1;
	int m_BothIdleSince = -1; // both of us free and standing still since
	int m_Restarts = 0;

	int m_Mode = MODE_IDLE;
	int m_CoastDir = 0; // in the air between swing plans on my own: the way I keep steering
	bool m_Coasting = false; // in the air between swing plans on my own

	// hookfly timing (tuned in SimBot.TuneHookfly against scripted partners)
	struct SFlyParams
	{
		float m_Wait = 12; // hook the partner below once my vy > -m_Wait
		float m_StartWait = 1; // same, while the partner still stands on the ground
		int m_Release = 0; // let go once the partner is above me by m_Release px
		int m_HookerDir = 1; // 1: move away from the partner while hooking it
		int m_Dodge = 48; // while being pulled: step aside if horizontally closer than this
		int m_MaxDx = 120; // move towards the partner beyond this
	};
	SFlyParams m_Fly;

	// pseudofly (tuned in SimPseudo.Search / SimBot.Pseudofly against scripted partners)
	struct SPseudoParams
	{
		// hammerer: hammer when the partner is in reach and stops coming closer, is about to bump into me or to
		// leave my reach, or is closer than m_FireDist
		float m_FireDist = 50;
		float m_MinDy = 10; // the partner must be at least this much higher
		float m_Lead = 4; // steer under the partner: position error plus this many ticks of velocity difference
		int m_Dead = 6; // don't steer within this many px
		// driver: let go once the hammerer is closer than m_ReleaseDist and still closing in (the rope hardly pulls
		// there, and letting go resets the hook timeout), hook again once it is farther than m_RehookDist and the gap
		// opens, or after m_MaxFree ticks
		float m_ReleaseDist = 50;
		float m_RehookDist = 45;
		int m_MaxFree = 3;
		// steering: stop pushing once I'm this fast, or the hammerer lags this far behind (the rope drags it at
		// most 15 px/tick, a faster driver leaves it behind and the fly falls apart)
		float m_MaxVx = 12;
		float m_MaxBehind = 30;
		// climbing (PlanClimb): hammer only once the partner is closer than m_FireDist (or about to bump into me): the
		// rope drags me up at 15 px/tick only while it's longer than about 42 px; and the driver air-jumps once
		// falling faster than m_AirJumpVy
		bool m_CloseOnly = false;
		float m_AirJumpVy = 1000;
	};
	SPseudoParams m_Pseudo;
	int m_DriveDir = 0; // pseudofly direction when I drive
	int m_FlyStyle = 0; // play mode's fly: 0 pseudofly, 1 hookfly (the freed one, rising above, hooks the falling
			    // rescuer up; got back up from Stronghold's lower floor in 0 of 4 runs, pseudofly in 3 of 4)

	// dive plan (aled), relative to the tick it starts: hook the partner from m_HookAt on, air-jump at m_JumpAt,
	// hold m_Dir; the hammer fires by itself on the first tick my last move touched freeze
	struct SPlan
	{
		bool m_Valid = false;
		int m_HookAt = -1;
		int m_HookEnd = 1 << 20; // rescue: let go here (a fling), otherwise at the hammer
		int m_Hook2At = -1; // rescue: hook again from here until the hammer (lift first, then a long swing)
		float m_TargetX = -1; // rescue: >= 0: steer to this x and hold there instead of holding m_Dir
		int m_JumpAt = -1;
		int m_Dir = 0;
		int m_FireAt = -1;
		int m_PostDir = 0; // rescue: where I steer after the hammer (my own fall must not end on a kill tile)
		int m_PostJump = 0; // rescue: and my air jump then (2: once I'm falling, 1: right away)
		bool m_PreHammer = false; // rescue: start by hammering the frozen partner I stand on (it answers with its held hammer)
		bool m_Aled = false; // rescue: the partner comes out beyond a freeze wall I stay behind
		bool m_Delivered = false; // rescue: not freed, but brought to rest outside the freeze (it thaws by itself)
		bool m_HookOnly = false; // rescue: no hammer: at m_FireAt I let go of the hook, and it comes to rest outside the freeze
		int m_AirJumpAt = -1; // rescue: an air jump too
		int m_DirAfter = 2; // rescue: from m_HookAt on, hold this direction instead (2: no)
		bool m_MeFrozen = false; // rescue: I end up in the freeze myself (for tests)
		bool m_Grounded = false; // rescue: I never leave the ground
		float m_AimDy = 0; // rescue: aim the hook this far below (+) / above (-) the partner's center
		vec2 m_PartnerEnd = vec2(0, 0); // rescue: where the partner ends up (for tests)
		float m_Dist = 0;
		float m_Score = -1e9f;
		std::vector<vec2> m_vPath; // my position after each planned tick
		std::vector<vec2> m_vUserPath; // the partner's
	};
	SPlan PlanDive(const CHookBotSim &Base, int PrevHook) const;
	// hammerhit rescue: the partner is frozen (or about to be) and I'm free. Searches direction, jump and hook
	// timings and when to hammer; the partner has to come out free (not refrozen for a while), as far in
	// m_DriveDir and as high as possible. Uses m_HookAt / m_JumpAt / m_Dir / m_FireAt of SPlan.
	// It is an anytime search: it enumerates plan families from *pCursor on until Deadline, starting from Seed (the
	// plan being followed, re-anchored to Base), and leaves *pCursor where it stopped. Thread-safe (reads only its
	// arguments and the map).
	// HookOnly: rescues by the hook alone count too (SPlan::m_HookOnly; a fallback, see m_NoRescueSince)
	// the rescue plan's direction / jump / hook at plan tick t
	static CNetObj_PlayerInput RescueInput(const SPlan &P, int t, int PrevHook, int HookState, vec2 AimAt, vec2 Pos, vec2 Vel);
	// pHookCursor: where the search of hook-only rescues from a jump (tried first, when HookOnly) goes on from
	static SPlan PlanRescue(const CHookBotSim &Base, int PrevHook, int *pCursor, const SPlan &Seed, int Goal, const CHookBotGoalField *pField, int64_t Deadline, bool HookOnly = true, int *pHookCursor = nullptr);
	struct SRescueStats
	{
		int m_Plans = 0, m_Found = 0, m_Fired = 0, m_Freed = 0;
	} m_RescueStats;
	int m_LastPlanUs = 0; // cost of the last plan (for tests)
	int m_PlanBudgetUs = 30000; // planning time per tick; the search stops early and keeps its best plan so far
	// hammerhit planning runs on a worker thread (m_AsyncPlanning; tests can run it inline): a job plans from the
	// state m_RescueLead ticks ahead (predicted by playing the bridge inputs meanwhile) for m_RescueBudgetUs, so the
	// game never waits for it. While a plan is followed, the next job searches on for a better one.
	bool m_AsyncPlanning = true;
	int m_RescueLead = 2;
	int m_RescueBudgetUs = 30000;
	int m_RescueCursor = 0;
	int64_t m_PlanDeadline = 0;
	struct SStats
	{
		int m_Entered = 0, m_FarReach = 0, m_Reloading = 0, m_NotBeyond = 0, m_Refrozen = 0, m_LaunchChecks = 0, m_Through = 0, m_Launches = 0, m_HammerLaunches = 0, m_Replans = 0, m_ReplanFails = 0, m_Fired = 0, m_Aborts = 0;
	} m_Stats;
	int m_Phase = 0; // aled: 0 fly, 1 launched/diving, 2 after the hammer
	SPlan m_Plan;
	int m_PlanStart = 0;

	// team moves: a move both of us make together, where each one's part only works with the other's (Stronghold's
	// freeze ceiling: from both standing below it, one jumps and the other jumps after it and hammers it up through the
	// freeze, it lands frozen above and thaws; then the one below jumps and the one above hooks it up through). Both
	// brains plan it the same way from the same state: the tees in client id order, a search budgeted by simulated
	// ticks instead of time. So both find the same plan without talking, and each plays its own half. Only with a
	// partner that is a brain too (m_PartnerIsBot): a person can't know the plan
	enum
	{
		TEAM_NONE,
		TEAM_THROW,
		TEAM_CATCH,
		TEAM_GATHER,
		TEAM_LEAP,
		TEAM_JOINT,
		TEAM_DROP,
		TEAM_FALL,
		TEAM_DASH,
		TEAM_HOP,
		TEAM_CLIMB,
		TEAM_DRAG,
		TEAM_COLUMN,
		TEAM_FLING,
		TEAM_FINISH,
	};
	struct STeamPlan
	{
		bool m_Valid = false;
		int m_Kind = TEAM_NONE;
		int m_aId[2] = {-1, -1}; // client ids of the plan's tees 0 and 1 (the lower id first)
		int m_Start = 0; // the game tick its tick 0 is played in
		int m_Up = -1; // which of its tees goes up through the freeze (thrown, or caught)
		std::vector<CNetObj_PlayerInput> m_avIn[2]; // [k]: what each does in tick k (m_Fire 1: hammer then)
		std::vector<vec2> m_avPath[2]; // [k]: where each is when deciding tick k
		vec2 m_aEnd[2] = {vec2(0, 0), vec2(0, 0)}; // where each comes to rest
		float m_Score = -1e9f;
		int m_Steps = 0; // ticks simulated to find it
	};
	const char *m_pWhy = ""; // what decided this tick's input (for the tests' traces)
	bool m_PartnerIsBot = false;
	bool m_DebugFly = false;
	int m_TeamBudget = 1200000; // simulated ticks per team search (deterministic, both brains must find the same plan)
	STeamPlan m_Team;
	// both tees in client id order: the same sim whichever of us builds it
	void InitTeamSim(CHookBotSim &S) const;
	// the throw from both standing: the thrown one jumps (and air-jumps near the top), the thrower jumps after it and
	// hammers it up through the freeze above; it has to come to rest outside the freeze well along (the frozen fall
	// counts, it thaws there), the thrower back on its feet outside the freeze. Thread-safe, deterministic
	static STeamPlan PlanThrow(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps);
	// the catch: the one above (closer to the goal) walks to the edge, the one below walks under the gap and jumps (and
	// air-jumps), the one above hooks it and holds a direction, which drags it up through the freeze; both have to come
	// to rest outside the freeze above it (the caught one frozen: it thaws). Thread-safe, deterministic
	static STeamPlan PlanCatch(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps);
	// before a throw: both of us onto a floor below the freeze ceiling the way goes up through (one with FreezeColumns
	// above it), next to each other: each steers to a spot on it, maybe with its air jump, letting go of any hook; both
	// have to come to rest there outside the freeze. Thread-safe, deterministic
	static STeamPlan PlanGather(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps);
	// both of us standing together, the way on beyond freeze to the side: run that way (the one in front maybe hooking
	// the one behind, which drags it faster), each jumps near the end of the floor and maybe air-jumps, and we fly through
	// the freeze frozen. Both have to come to rest, one of us outside the freeze; the other may lie in it within 8 tiles
	// of the first with nothing in between (it pulls it out once it has thawed: rank 1 at Stronghold's first freeze walls
	// after the ceiling). Thread-safe, deterministic
	static STeamPlan PlanLeap(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps);
	bool m_GoalByAir = true; // the route's goal can be reached through open air from where we were when it was set
	bool m_SoloByAir = true; // the solo goal (further on) too
	// the general one: a beam search over both of us at once, in steps of 4 ticks, each of us steering left / right /
	// neither, jumping or not, holding the hook on the other or not; a step's end is judged by where we both come to
	// rest if we coast on from there (holding the direction in the air, the air jump once falling or not). The same rules
	// for the end as a leap. For whatever the other team moves don't cover (Stronghold's corridor of freeze walls after
	// the ledge: the one in front drags the other through). Thread-safe, deterministic
	// a freeze floor the way goes down through, with a long drop onto freeze below it: one of us walks into it and falls
	// through frozen, the other holds it on the hook just below the freeze (re-hooking: a hook lets go of a player after
	// 1.2 s) until its freeze is about to run out, then follows it; the first thaws in the air before it lands and gets
	// the other out of the freeze floor below (a rescue plan, PlanRescue, played as part of this one). Stronghold after the
	// corridor, the pit at x 275-289: rank 1's way, 3.7 s. Thread-safe, deterministic
	static STeamPlan PlanDrop(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps);
	// a fall caught on the hook: maybe a pseudofly first (the upper one driving one way or the other, the lower one
	// hammering it up), then one of us (the diver) lets itself fall into freeze while the other (the catcher) gets ready
	// beside it, hooks it as it falls past frozen and drags it, and lets go where its fall lands it outside the freeze
	// further on, where it thaws. The catcher has to end outside the freeze too. Stronghold after the pit: up over the
	// freeze shaft at x 332-340, one falls down it, the other on the platform beside it hooks it through the corner under
	// the block and drags it under the freeze room's ceiling, so it falls through the hole onto the ledge (rank 1's way).
	// Thread-safe, deterministic
	static STeamPlan PlanFall(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps, const SPseudoParams &Pseudo);
	// a dash across a freeze floor with nothing to hook but each other: both run and jump, one hooks the other past it
	// (and hammers it as it comes by), then the one ahead hooks the one behind up and on whenever it sinks; each holds
	// the way on throughout (in the air that keeps speed above 5) and air-jumps over the freeze. A grid of these, judged
	// where we come to rest
	static STeamPlan PlanDash(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps);
	// a hop up through a freeze ceiling right above us: one jumps into it, the other jumps as it comes back down and
	// hammers it free, the freed one hooks the other up through and hammers it free above, then a pseudofly on
	static STeamPlan PlanHop(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps, const SPseudoParams &Pseudo);
	// a climb from both of us in the air: a pseudofly along Air (the open-air field) from here, to where we come to rest,
	// or to where each of us can swing on by itself (free, clear of the freeze, something hookable in reach, well on)
	// a drag from standing: one of us hooks the other (through freeze too) and holds a direction, both maybe jump, it lets
	// go, both fly on to where we come to rest. A grid, judged by TeamEndValue
	static STeamPlan PlanDrag(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps);
	static STeamPlan PlanColumnDrop(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps);
	static STeamPlan PlanLaunch(const CHookBotSim &Base, const CHookBotGoalField &Field, const CHookBotGoalField &Air, int MaxSteps, const SPseudoParams &Pseudo);
	static STeamPlan PlanFling(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps);
	// the finish drop, both of us standing near the finish: one of us (the diver) walks into the freeze beside the floor
	// the other stands on (or above it, and falls past), the other (the holder) hooks it below that freeze and holds it
	// there out of the freeze until it thaws, then walks in after it; the diver, free in the air, hooks it and steers to
	// a column a straight fall down goes to the finish from, dragging the holder along (Stronghold's last freeze layers,
	// a 3-wide gap under the block: frozen beside the block, nobody gets under it by itself; rank 1 so, 208-215 s).
	// Judged by the finish: each of us crosses a finish tile, or rests outside the freeze where a way of its own (the
	// finish search's first stage) goes there. Thread-safe, deterministic
	static STeamPlan PlanFinishDrop(const CHookBotSim &Base, const std::vector<vec2> &vFinish, int MaxSteps);
	static STeamPlan PlanClimb(const CHookBotSim &Base, const CHookBotGoalField &Field, const CHookBotGoalField &Air, int MaxSteps, const SPseudoParams &Pseudo, int MaxTicks = 300);
	int m_NoFlyUntil = 0;
	int m_ClimbAgainAt = -1; // a climb ended in the air: plan the next one from here at once // a climb just handed over to swinging: no fly again yet
	// the team-move judge's checks, for tests: can a free tee get from From to To by itself; can Free get Stuck out
	static bool TestCanReach(const CHookBotSim &S, vec2 From, vec2 To);
	static bool TestCanRescue(const CHookBotSim &S, vec2 Free, vec2 Stuck);
	// Effort 1: a wider beam and all four passes (when the plain one found nothing: a restart costs far more)
	static STeamPlan PlanJoint(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps, int Effort = 0);
	// a team search (TEAM_CALL), shared between the two brains: both plan the same one at the same tick
	template<typename F>
	STeamPlan TeamCall(const CHookBotSim &S, int Line, F &&Search);
	std::string TeamCallKey(const CHookBotSim &S, int Line) const;
	static STeamPlan PlanJointPass(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps, int K, int MaxDepth, int Width);
	// its beam: kept nodes per level, ticks per step, steps deep, nodes per level whose rest is simulated (tests tune these)
	struct SJointParams
	{
		int m_Width = 10, m_K = 4, m_MaxDepth = 24, m_NumCoast = 30, m_Passes = 2;
	};
	static SJointParams ms_Joint;
	static bool ms_DebugTeam; // tests: the team planners print what they found
	int m_TeamEnd = -1; // when the last team move ended (we wait for the one it left frozen to thaw)
	int m_TeamEndKind = TEAM_NONE;
	vec2 m_aTeamFailPos[2] = {vec2(-1e9f, -1e9f), vec2(-1e9f, -1e9f)}; // nothing found from here: not again until we move

private:
	CCollision *m_pCollision = nullptr;
	CTeamsCore *m_pTeams = nullptr;
	CNetObj_PlayerInput m_Input = {};
	int m_Fire = 0;

	// routes: where on the path we are (Index >= 0: there), and switching to another tee's path when I'm on it
	void PickRouteIndex(vec2 Me, vec2 Partner, int Index = -1);
	// how far Me is from waypoints From..To of a path, counting only the parts it can see (not through walls); *pSeg:
	// the nearest segment
	// AirOnly: nor through freeze
	float PathDist(const std::vector<vec2> &vPath, int From, int To, vec2 Me, int *pSeg, bool AirOnly = false) const;
	void UpdateRoutePath();

	// frozen and lying still: which way to hold, so that when the partner hammers me (unfreezing me for a tick),
	// that tick of input takes me the right way (Stronghold: off the 3-tile post right of the ledge and down the
	// freeze column, instead of landing back on it)
	int FrozenHoldDir();
	int m_FrozenDir = 0;
	int m_NextFrozenDirCheck = 0;
	int m_FrozenDirSaid = 0; // what I last told the partner I'm holding

	// the current tick's view
	const SHookBotTee *m_pB = nullptr;
	const SHookBotTee *m_pU = nullptr;
	bool m_BotFirst = true;
	int m_Now = 0;

	// aled
	int m_NextPlan = 0;
	int m_FiredTick = -1;
	bool m_HoldBlocked = false;

	// hammerhit planning job
	struct SRescueJob
	{
		std::thread m_Thread;
		std::atomic<bool> m_Done{false};
		CHookBotSim m_Base;
		int m_PrevHook = 0, m_Cursor = 0, m_Anchor = 0, m_Goal = 0, m_Epoch = 0, m_Us = 0;
		std::shared_ptr<const CHookBotGoalField> m_pField;
		bool m_Fresh = false;
		bool m_HookOnly = false;
		int m_HookCursor = 0;
		int64_t m_Deadline = 0;
		SPlan m_Seed, m_Result;
	};
	std::unique_ptr<SRescueJob> m_pJob;
	// since when the rescue plans find nothing (-1: they do, or there's nobody to rescue). Rescues by the hook alone only
	// after half a second of that: they leave the partner frozen for a while, and where a hammer rescue comes up a moment
	// later (Stronghold's opening), taking them at once made it slower and sometimes stuck
	int m_NoRescueSince = -1;
	int m_HookCursor = 0; // PlanRescue's pHookCursor, from job to job
	struct SGoalJob
	{
		std::thread m_Thread;
		std::atomic<bool> m_Done{false};
		vec2 m_Tile;
		std::vector<vec2> m_vAirTiles; // air field candidates, farthest first; the first one reachable from m_From wins
		bool m_GoalByAir = false; // the first one (the goal) was
		bool m_SoloByAir = true; // the solo goal can be reached through open air from m_From (or we aren't in open air)
		vec2 m_From;
		vec2 m_SoloTile, m_FarTile;
		std::shared_ptr<CHookBotGoalField> m_pField, m_pAir, m_pSolo, m_pFar;
	};
	std::unique_ptr<SGoalJob> m_pGoalJob;
	// the goal fields for route waypoint Index (on a thread unless Sync)
	void SetRouteGoal(int Index, bool Sync, vec2 PairMid);
	void UpdateRoute();
	int m_RescueEpoch = 0; // bumped whenever a rescue ends; older jobs are discarded
	int m_NextRescueJob = 0; // after a plan that found nothing
	std::vector<CNetObj_PlayerInput> m_vBridge; // inputs from m_BridgeStart on, played while a job plans ahead
	int m_BridgeStart = 0;
	void FinishJob()
	{
		if(m_pJob && m_pJob->m_Thread.joinable())
			m_pJob->m_Thread.join();
		if(m_pGoalJob && m_pGoalJob->m_Thread.joinable())
			m_pGoalJob->m_Thread.join();
		if(m_pSoloJob && m_pSoloJob->m_Thread.joinable())
			m_pSoloJob->m_Thread.join();
		if(m_pOwnJob && m_pOwnJob->m_Thread.joinable())
			m_pOwnJob->m_Thread.join();
	}
	void CollectJob();
	void StartJob(const SHookBotTee &Bot, const SHookBotTee &Partner);


	int m_PostDir = 0, m_PostEnd = -1; // steering after a rescue hammer
	int m_PostJump = 0; // and the air jump then (SafeFallDir)

	// pseudofly driver
	bool m_PfFree = false;
	int m_PfFreeTicks = 0;

	void InitSim(CHookBotSim &S) const
	{
		S.Init(m_pCollision, m_pTeams, *m_pB, *m_pU, m_BotFirst);
		S.m_Tick = m_Now;
	}
	void Hookfly(CNetObj_PlayerInput &In);
	void PseudoHammer(CNetObj_PlayerInput &In);
	// ForceRescue: treat the partner as needing me even though it isn't frozen (it's cut off from the goal by freeze)
	void Hammerhit(CNetObj_PlayerInput &In, bool ForceRescue = false);
	void Play(CNetObj_PlayerInput &In);
	// a team move: playing my half of one, waiting for the partner it left frozen to thaw, or starting one (false: none)
	bool TeamMove(CNetObj_PlayerInput &In);
	bool TryFall(CNetObj_PlayerInput &In, int Budget = 0);
	// a move on my own (both of us standing safely, nobody to rescue): jump / air-jump / steer to a spot further along
	// the goal where I can stand without freezing, e.g. the very edge of a platform next to a freeze column
	struct SSoloPlan
	{
		bool m_Valid = false;
		float m_TargetX = 0;
		int m_JumpAt = -1, m_AirJumpAt = -1;
		float m_Score = -1e9f;
		std::vector<vec2> m_vPath; // [k]: where I am when deciding tick k
		std::vector<CNetObj_PlayerInput> m_vInputs; // [k]: what I do then
		bool m_Swing = false; // with the hook (m_vInputs' hook and aim count too), planned on from the air
		bool m_EndsGrounded = false; // a swing plan that ends standing (not planned on from there)
		bool m_Safe = true; // a swing plan: letting go at its end, I land without freezing
		// a swing plan cut short (I plan on from its end): the rest of what the search found, which goes on from there
		// out of the freeze for that long
		std::vector<vec2> m_vRestPath;
		std::vector<CNetObj_PlayerInput> m_vRestInputs;
	};
	SSoloPlan m_Solo;
	// the rest of the last swing plan that was cut short: followed when the search from its cut finds nothing (from right
	// under a freeze band every move came near the freeze at once, and I coasted into it)
	SSoloPlan m_SoloRest;
	// a swing plan I left in the air (a few px off its path): its inputs from there on, played while the next search runs
	std::vector<CNetObj_PlayerInput> m_vDropRest;
	int m_DropRestAt = 0; // the tick its first input is for
	int m_SoloStart = 0;
	// the solo search runs on a thread (on the client's main thread it made the game stutter before each jump)
	struct SSoloJob
	{
		std::thread m_Thread;
		std::atomic<bool> m_Done{false};
		CHookBotSim m_Base;
		vec2 m_From;
		std::shared_ptr<const CHookBotGoalField> m_pField;
		std::shared_ptr<const CHookBotGoalField> m_pAltField; // searched by when m_pField finds nothing
		bool m_AllowFrozen = false;
		bool m_Air = false; // planned from where the current swing plan ends, in the air (only swings from there)
		bool m_Settle = false; // land at the first safe place that gets me somewhere (the partner is far behind)
		int m_StartTick = 0; // the tick the plan starts at
		bool m_LongSwing = false; // 400 ms for the swing search (planning inline); on threads, live, 200 ms
		SSoloPlan m_Result;
	};
	std::unique_ptr<SSoloJob> m_pSoloJob;
	int m_NextSoloSearch = 0;
	// AllowFrozen: also through freeze, onto a spot outside it where I thaw (the frozen ticks count against it)
	static SSoloPlan SoloSearch(const CHookBotSim &Base, const CHookBotGoalField &Field, int64_t Deadline, bool AllowFrozen = false);
	// on my own with the hook, from the ground or the air: a few swings ahead (hook a hookable tile in reach while
	// steering, let go, the next one), the best few kept after each (a beam search), never into freeze or a kill tile,
	// ending where I'm safe (letting go, I land without freezing) whenever that's possible.
	// The plan is followed until it ends; the next one is planned from where it ends while I fly (Stronghold after the
	// shaft: across a 30-tile freeze band on the hookable blocks in the ceiling)
	// Settle: a safe plan is followed on to where I land (and not planned on from there)
	// Relaxed: only touching the freeze rules a move out, not coming within 4 px of it (a last resort in the air)
	static SSoloPlan SwingSearch(const CHookBotSim &Base, const CHookBotGoalField &Field, int64_t Deadline, bool Settle = false, bool Relaxed = false);
	static SSoloPlan TestSwingSearch(const CHookBotSim &S, const CHookBotGoalField &Field, int Ms) { return SwingSearch(S, Field, TestDeadline(Ms)); }
	static int64_t TestDeadline(int Ms);
	static SSoloPlan TestSoloSearch(const CHookBotSim &S, const CHookBotGoalField &Field, int Ms, bool AllowFrozen = false) { return SoloSearch(S, Field, TestDeadline(Ms), AllowFrozen); }
	// one tick of the solo move (false: none going on); starts/continues the search while standing
	bool SoloMove(CNetObj_PlayerInput &In, bool AllowFrozen = false);
	// the partner is frozen for good and no rescue works from where I stand: I go on by myself along the route to
	// where one does (each new spot gets a second of rescue planning first)
	bool PartnerStuckForGood() const; // frozen, lying still in freeze
	// falling from here: a direction to hold that lands me outside the freeze (0 if none does); *pJump: with my air jump
	// (1: right away, 2: once I'm falling) or not (0)
	int SafeFallDir(int *pJump = nullptr) const;
	bool PartnerFarBehind() const; // free, and 10 tiles further from the goal than me
	// the finish (FinishMove): falls by myself that cross a finish tile, in up to 3 stages (each to rest outside the freeze,
	// thawing there, then on); the first stage is played, and searched again from where it ends
	bool FinishMove(CNetObj_PlayerInput &In);
	void ScanFinish();
	bool m_Finished = false, m_PartnerFinished = false; // crossed a finish tile
	// the route's goal 14 waypoints on is at the finish (both of us see the same route)
	bool NearFinish();
	static bool TestFinishSearch(const CHookBotSim &S, const std::vector<vec2> &vFinish, int MaxSteps, std::vector<vec2> *pvPath, int *pStages, int *pSteps);
	std::vector<vec2> m_vFinishTiles;
	bool m_FinishScanned = false;
	std::vector<CNetObj_PlayerInput> m_vFinishPlan;
	std::vector<vec2> m_vFinishPath;
	int m_FinishPlanAt = -1, m_FinishTriedTick = -1000;
	vec2 m_FinishTriedAt = vec2(-1e9f, -1e9f);
	bool AirWayClimbs(vec2 P, int Tiles, const CHookBotGoalField *pField = nullptr) const; // the way from P to the air goal (or down
		// pField) goes this many tiles above it
	vec2 m_RescueTryPos = vec2(-1e9f, -1e9f);
	vec2 m_AloneMoveFrom = vec2(-1e9f, -1e9f); // where the alone move I'm on started
	// the partner stuck for good: a field to where it lies, for my alone moves (the route's goal ran on ahead of me as I
	// went on alone, and I went the other way, along the route past Stronghold's post, the partner lying at the bottom of
	// the freeze column behind me)
	std::shared_ptr<const CHookBotGoalField> m_pPartnerField;
	vec2 m_PartnerFieldAt = vec2(-1e9f, -1e9f);
	// where I wait for the partner's fall through freeze to hook it out (FindCatchSpot), for the partner standing at m_CatchFor
	bool FindCatchSpot();
	bool SwingReach(const CHookBotSim &S, vec2 From, vec2 To) const; // a catch spot I get to swinging
	vec2 m_CatchSpot = vec2(-1e9f, -1e9f), m_CatchFor = vec2(-1e9f, -1e9f);
	int m_CatchCheckAt = -1;
	bool m_CatchGo = false; // on my way there (the solo moves go to it)
	std::shared_ptr<const CHookBotGoalField> m_pCatchField;
	vec2 m_CatchFieldAt = vec2(-1e9f, -1e9f);
	vec2 m_AloneStuckPos = vec2(-1e9f, -1e9f); // an alone move from here got me nowhere: none from here until
	int m_AloneStuckUntil = 0;
	int m_RescueTrySince = 0;
	void PseudoDrive(CNetObj_PlayerInput &In);
	bool HoldIsRisky();
	void Aled(CNetObj_PlayerInput &In);
	bool TryLaunch(CNetObj_PlayerInput &In);
	bool FreezeBetween(vec2 A, vec2 B) const;
	// can a tee get from A to B through open air (no walls, freeze or kill tiles), within Radius tiles of A? (true
	// if B is further away than that: can't tell)
	bool AirConnected(vec2 A, vec2 B, int Radius = 24) const;
	bool m_CutOff = false;
	int m_NextCutOffCheck = 0;
	int m_NextAirRebuild = 0;
	// the partner's gestures: a hammer swing aimed up that hits nothing = "let's fly" (m_FlyWishUntil); I answer with a
	// nod (my aim up and down, m_NodUntil)
	int m_FlyWishUntil = 0, m_NodUntil = 0, m_PrevPartnerFire = 0;
	void ReadGestures();
};

// tests: the planners' deadlines go by the ticks they simulate (StepNs each, 0: the wall clock) instead of the wall
// clock, so runs repeat exactly (needs m_AsyncPlanning off)
void HookBotDeterministic(int StepNs);
// prints the wall time, simulated ticks and calls spent in each planner so far
void HookBotProfDump();

// the hook input rule shared by the live bot and its lookahead: holding the hook re-fires it once it is
// neither flying nor grabbed (needs one released tick)
int HookBotHookInput(bool Want, int PrevHook, int HookState);
// a team sim's whole state as text and back (HH_SIMDUMP prints it where a team planner starts; SimMapBots.JointBench
// plans from it with JB_SIM): to rerun a live search exactly, hooks and spent jumps included
std::string HookBotSimDump(const CHookBotSim &S);
bool HookBotSimLoad(CHookBotSim &S, const char *pText);

#endif
