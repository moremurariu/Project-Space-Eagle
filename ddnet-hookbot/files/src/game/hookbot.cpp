#include "hookbot.h"

#include <base/str.h>
#include <base/time.h>
#include <base/types.h>

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <vector>

// the planners' clock for their deadlines: wall-clock time, or (HookBotDeterministic, for tests) one that only moves
// with the ticks they simulate (StepNs each; a tick of both tees costs about 2000 ns), so a run plays the same every
// time (the searches stop where the time is up, and with the wall clock no two runs did; it needs the planning inline).
// Other StepNs give other runs, each one repeatable
static int gs_HookBotStepNs = 0;
static thread_local int64_t ts_HookBotSimSteps = 0;
void HookBotDeterministic(int StepNs) { gs_HookBotStepNs = StepNs; }
// a planner called inside a team plan's search has to find the same as in the other brain: its budget is simulated
// ticks then (CStepBudget; its deadline must be 0: the clock reads -1 until the ticks are used up, then 1)
static thread_local int64_t ts_StepLimit = -1;
static int64_t PlanClock()
{
	if(ts_StepLimit >= 0)
		return ts_HookBotSimSteps < ts_StepLimit ? -1 : 1;
	return gs_HookBotStepNs ? ts_HookBotSimSteps * gs_HookBotStepNs * time_freq() / 1000000000 : time_get();
}
class CStepBudget
{
	int64_t m_Old;

public:
	CStepBudget(int64_t Steps) :
		m_Old(ts_StepLimit) { ts_StepLimit = ts_HookBotSimSteps + Steps; }
	~CStepBudget() { ts_StepLimit = m_Old; }
};

static void Aim(CNetObj_PlayerInput &In, vec2 Dir)
{
	In.m_TargetX = round_to_int(Dir.x);
	In.m_TargetY = round_to_int(Dir.y);
	if(In.m_TargetX == 0 && In.m_TargetY == 0)
		In.m_TargetY = -1;
}

// ---- lookahead ----

CHookBotSim &CHookBotSim::operator=(const CHookBotSim &Other)
{
	m_aTee[0] = Other.m_aTee[0];
	m_aTee[1] = Other.m_aTee[1];
	m_aOrder[0] = Other.m_aOrder[0];
	m_aOrder[1] = Other.m_aOrder[1];
	m_Tick = Other.m_Tick;
	m_pCollision = Other.m_pCollision;
	m_pTeams = Other.m_pTeams;
	m_World.m_pPrng = nullptr;
	Link();
	return *this;
}

void CHookBotSim::Link()
{
	for(auto &pChr : m_World.m_apCharacters)
		pChr = nullptr;
	for(auto &Tee : m_aTee)
	{
		Tee.m_Core.SetCoreWorld(&m_World, m_pCollision, m_pTeams);
		if(Tee.m_Core.m_Id >= 0 && Tee.m_Core.m_Id < MAX_CLIENTS)
			m_World.m_apCharacters[Tee.m_Core.m_Id] = &Tee.m_Core;
	}
}

void CHookBotSim::Init(CCollision *pCollision, CTeamsCore *pTeams, const SHookBotTee &A, const SHookBotTee &B, bool AFirst)
{
	m_pCollision = pCollision;
	m_pTeams = pTeams;
	const SHookBotTee *apTee[2] = {&A, &B};
	for(int i = 0; i < 2; i++)
	{
		STee &T = m_aTee[i];
		T.m_Core = apTee[i]->m_Core;
		T.m_Core.SetCoreWorld(&m_World, m_pCollision, m_pTeams);
		T.m_Core.SetAntiPingInterfereCallback([](int, bool) {});
		T.m_FreezeTime = apTee[i]->m_FreezeTime;
		T.m_PrevPos = apTee[i]->m_PrevPos;
		T.m_EnteredFreeze = PathTouchesFreeze(apTee[i]->m_PrevPos, apTee[i]->m_Core.m_Pos);
		T.m_PrevInput = apTee[i]->m_Input;
		T.m_Reload = apTee[i]->m_Reload;
	}
	m_aOrder[0] = AFirst ? 0 : 1;
	m_aOrder[1] = AFirst ? 1 : 0;
	Link();
}

void CHookBotSim::Swap()
{
	std::swap(m_aTee[0], m_aTee[1]);
	m_aOrder[0] = 1 - m_aOrder[0];
	m_aOrder[1] = 1 - m_aOrder[1];
	Link();
}

bool CHookBotSim::InFreeze(vec2 Pos) const
{
	int Index = m_pCollision->GetPureMapIndex(Pos);
	int T = m_pCollision->GetTileIndex(Index), F = m_pCollision->GetFrontTileIndex(Index);
	return T == TILE_FREEZE || T == TILE_DFREEZE || F == TILE_FREEZE || F == TILE_DFREEZE;
}

bool CHookBotSim::TouchesDeath(vec2 Pos) const
{
	const float r = 28.0f / 3.0f;
	for(vec2 d : {vec2(r, -r), vec2(r, r), vec2(-r, -r), vec2(-r, r)})
		if(m_pCollision->GetCollisionAt(Pos.x + d.x, Pos.y + d.y) == TILE_DEATH || m_pCollision->GetFrontCollisionAt(Pos.x + d.x, Pos.y + d.y) == TILE_DEATH)
			return true;
	return false;
}

bool CHookBotSim::PathTouchesFreeze(vec2 From, vec2 To) const
{
	// like CCollision::GetMapIndices (the center, every pixel), without the allocation
	float Len = distance(From, To);
	int Steps = (int)Len + 1;
	for(int i = 0; i <= Steps; i++)
		if(InFreeze(mix(From, To, i / (float)Steps)))
			return true;
	return false;
}

// same order as CGameWorld::Tick: every character runs Tick() (freeze input, core tick, tile checks for the
// previous move), then every character moves
void CHookBotSim::Step(const CNetObj_PlayerInput &InA, const CNetObj_PlayerInput &InB, int Only)
{
	const CNetObj_PlayerInput *apIn[2] = {&InA, &InB};
	ts_HookBotSimSteps++;
	for(int i : m_aOrder)
	{
		if(Only >= 0 && i != Only)
			continue;
		STee &T = m_aTee[i];
		CNetObj_PlayerInput In = *apIn[i];
		if(T.m_FreezeTime > 0)
		{
			T.m_FreezeTime--;
			In.m_Direction = 0;
			In.m_Jump = 0;
			In.m_Hook = 0;
			if(T.m_FreezeTime == 1)
			{
				T.m_FreezeTime = 0;
				T.m_Core.m_FreezeStart = 0;
			}
		}
		T.m_Core.m_Input = In;
		T.m_Core.Tick(true);
		if(PathTouchesFreeze(T.m_PrevPos, T.m_Core.m_Pos) && (T.m_FreezeTime == 0 || T.m_Core.m_FreezeStart < m_Tick - SERVER_TICK_SPEED))
		{
			T.m_FreezeTime = 3 * SERVER_TICK_SPEED;
			T.m_Core.m_FreezeStart = m_Tick;
		}
		T.m_PrevPos = T.m_Core.m_Pos;
		T.m_PrevInput = In;
		if(T.m_Reload > 0)
			T.m_Reload--;
	}
	for(int i : m_aOrder)
	{
		if(Only >= 0 && i != Only)
			continue;
		STee &T = m_aTee[i];
		T.m_Core.Move();
		T.m_Core.Quantize();
		T.m_EnteredFreeze = PathTouchesFreeze(T.m_PrevPos, T.m_Core.m_Pos);
		T.m_Dead |= TouchesDeath(T.m_Core.m_Pos);
	}
	m_Tick++;
}

int HookBotHookInput(bool Want, int PrevHook, int HookState)
{
	if(!Want)
		return 0;
	if(PrevHook && HookState != HOOK_GRABBED && HookState != HOOK_FLYING)
		return 0; // let go for a tick so the next press fires again
	return 1;
}

std::string HookBotSimDump(const CHookBotSim &S)
{
	std::string Out = std::to_string(S.m_Tick);
	for(const auto &T : S.m_aTee)
	{
		const CCharacterCore &C = T.m_Core;
		int Hooked = -1;
		for(int i = 0; i < 2; i++)
			if(C.HookedPlayer() >= 0 && C.HookedPlayer() == S.m_aTee[i].m_Core.m_Id)
				Hooked = i;
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), ";%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", C.m_Pos.x, C.m_Pos.y, C.m_Vel.x, C.m_Vel.y,
			C.m_HookPos.x, C.m_HookPos.y, C.m_HookDir.x, C.m_HookDir.y, C.m_HookTick, C.m_HookState, Hooked, C.m_Jumped, C.m_JumpedTotal, C.m_Jumps, T.m_FreezeTime, T.m_Reload,
			T.m_PrevInput.m_Hook, T.m_PrevInput.m_Fire, T.m_PrevInput.m_Direction, T.m_PrevInput.m_Jump, T.m_PrevInput.m_TargetX, T.m_PrevInput.m_TargetY);
		Out += aBuf;
	}
	return Out;
}

bool HookBotSimLoad(CHookBotSim &S, const char *pText)
{
	S.m_Tick = atoi(pText);
	const char *p = pText;
	for(auto &T : S.m_aTee)
	{
		p = strchr(p, ';');
		if(!p)
			return false;
		p++;
		CCharacterCore &C = T.m_Core;
		int Hooked;
		if(sscanf(p, "%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &C.m_Pos.x, &C.m_Pos.y, &C.m_Vel.x, &C.m_Vel.y, &C.m_HookPos.x, &C.m_HookPos.y, &C.m_HookDir.x,
			   &C.m_HookDir.y, &C.m_HookTick, &C.m_HookState, &Hooked, &C.m_Jumped, &C.m_JumpedTotal, &C.m_Jumps, &T.m_FreezeTime, &T.m_Reload, &T.m_PrevInput.m_Hook,
			   &T.m_PrevInput.m_Fire, &T.m_PrevInput.m_Direction, &T.m_PrevInput.m_Jump, &T.m_PrevInput.m_TargetX, &T.m_PrevInput.m_TargetY) != 22)
			return false;
		T.m_PrevPos = C.m_Pos;
		C.SetHookedPlayer(Hooked >= 0 ? S.m_aTee[Hooked].m_Core.m_Id : -1);
	}
	return true;
}

// ---- goal field ----

void CHookBotGoalField::Build(const CCollision *pCollision, vec2 GoalTile, float FreezeCost)
{
	BuildAlong(pCollision, GoalTile, FreezeCost, {}, 0, 1);
}

void CHookBotGoalField::BuildAlong(const CCollision *pCollision, vec2 GoalTile, float FreezeCost, const std::vector<vec2> &vLine, float Radius, float FarCost)
{
	m_GoalTile = GoalTile;
	m_W = pCollision->GetWidth();
	m_H = pCollision->GetHeight();
	m_vDist.assign((size_t)m_W * m_H, 1e9f);
	// per tile: blocked (wall, kill tile), freeze (expensive), air
	std::vector<uint8_t> vKind((size_t)m_W * m_H, 0);
	CHookBotSim Probe;
	Probe.m_pCollision = const_cast<CCollision *>(pCollision);
	for(int y = 0; y < m_H; y++)
		for(int x = 0; x < m_W; x++)
		{
			const vec2 C(x * 32 + 16, y * 32 + 16);
			const int Game = pCollision->GetCollisionAt(C.x, C.y), Front = pCollision->GetFrontCollisionAt(C.x, C.y);
			if(pCollision->CheckPoint(C) || Game == TILE_DEATH || Front == TILE_DEATH)
				vKind[(size_t)y * m_W + x] = 2;
			else if(Probe.InFreeze(C))
				vKind[(size_t)y * m_W + x] = 1;
		}
	// off the line: dearer
	std::vector<uint8_t> vNear;
	if(vLine.size() >= 2 && FarCost != 1)
	{
		vNear.assign((size_t)m_W * m_H, 0);
		const int R = (int)std::ceil(Radius);
		for(size_t k = 0; k + 1 < vLine.size(); k++)
		{
			const vec2 A = vLine[k], B = vLine[k + 1];
			const int x0 = std::max(0, (int)std::floor(std::min(A.x, B.x)) - R), x1 = std::min(m_W - 1, (int)std::ceil(std::max(A.x, B.x)) + R);
			const int y0 = std::max(0, (int)std::floor(std::min(A.y, B.y)) - R), y1 = std::min(m_H - 1, (int)std::ceil(std::max(A.y, B.y)) + R);
			const vec2 D = B - A;
			const float L2 = dot(D, D);
			for(int y = y0; y <= y1; y++)
				for(int x = x0; x <= x1; x++)
				{
					const vec2 C(x + 0.5f, y + 0.5f);
					const float u = L2 > 0 ? std::clamp(dot(C - A, D) / L2, 0.0f, 1.0f) : 0.0f;
					if(distance(C, A + D * u) <= Radius)
						vNear[(size_t)y * m_W + x] = 1;
				}
		}
	}
	// Dijkstra over 8 neighbours (no corner cutting)
	using TItem = std::pair<float, int>;
	std::priority_queue<TItem, std::vector<TItem>, std::greater<TItem>> Queue;
	const int gx = std::clamp((int)GoalTile.x, 0, m_W - 1), gy = std::clamp((int)GoalTile.y, 0, m_H - 1);
	m_vDist[(size_t)gy * m_W + gx] = 0;
	Queue.push({0.0f, gy * m_W + gx});
	while(!Queue.empty())
	{
		auto [d, i] = Queue.top();
		Queue.pop();
		if(d > m_vDist[i])
			continue;
		const int x = i % m_W, y = i / m_W;
		for(int dy = -1; dy <= 1; dy++)
			for(int dx = -1; dx <= 1; dx++)
			{
				const int nx = x + dx, ny = y + dy;
				if((!dx && !dy) || nx < 0 || ny < 0 || nx >= m_W || ny >= m_H)
					continue;
				const int j = ny * m_W + nx;
				if(vKind[j] == 2 || (dx && dy && (vKind[y * m_W + nx] == 2 || vKind[ny * m_W + x] == 2)) || (vKind[j] == 1 && FreezeCost <= 0))
					continue;
				const float Step = (dx && dy ? 1.4142f : 1.0f) * (vKind[j] == 1 ? FreezeCost : 1.0f) * (!vNear.empty() && !vNear[j] ? FarCost : 1.0f) * 32.0f;
				if(d + Step < m_vDist[j])
				{
					m_vDist[j] = d + Step;
					Queue.push({d + Step, j});
				}
			}
	}
}

vec2 CHookBotGoalField::BestReachable(const CHookBotGoalField &By) const
{
	vec2 Best = m_GoalTile;
	float BestD = 1e9f;
	for(int y = 0; y < m_H; y++)
		for(int x = 0; x < m_W; x++)
			if(m_vDist[(size_t)y * m_W + x] < 1e8f)
			{
				const float D = By.Dist(vec2(x * 32 + 16, y * 32 + 16));
				if(D < BestD)
				{
					BestD = D;
					Best = vec2(x, y);
				}
			}
	return Best;
}

float CHookBotGoalField::Dist(vec2 Pos) const
{
	if(m_vDist.empty())
		return 0;
	// bilinear between tile centers, over the reachable ones
	const float fx = Pos.x / 32 - 0.5f, fy = Pos.y / 32 - 0.5f;
	const int x0 = (int)std::floor(fx), y0 = (int)std::floor(fy);
	float Sum = 0, WSum = 0;
	for(int j = 0; j < 2; j++)
		for(int i = 0; i < 2; i++)
		{
			const int x = std::clamp(x0 + i, 0, m_W - 1), y = std::clamp(y0 + j, 0, m_H - 1);
			const float D = m_vDist[(size_t)y * m_W + x];
			if(D >= 1e8f)
				continue;
			const float W = (i ? fx - x0 : 1 - (fx - x0)) * (j ? fy - y0 : 1 - (fy - y0));
			Sum += W * D;
			WSum += W;
		}
	return WSum > 0.001f ? Sum / WSum : 1e6f;
}

// ---- bot ----

static float CeilingAbove(const CHookBotSim &S, vec2 Pos, float Range);
static CNetObj_PlayerInput PartnerModel(const CHookBotSim &S);
static CNetObj_PlayerInput RiderModel(const CHookBotSim &S);
static bool FreezeOnLine(const CHookBotSim &S, vec2 A, vec2 B);
// the move From -> To passes within 4 px of freeze: a plan I follow can be a few px off what I simulated (I follow it
// while within 3 px), and that's enough to clip the edge of a freeze tile (Stronghold after the ceiling swings: a jump
// from the very edge of a floor next to a 1-tile freeze strip)
static bool NearFreeze(const CHookBotSim &S, vec2 From, vec2 To)
{
	for(vec2 d : {vec2(0, 0), vec2(4, 0), vec2(-4, 0), vec2(0, 4), vec2(0, -4)})
		if(S.PathTouchesFreeze(From + d, To + d))
			return true;
	return false;
}
static int SteerTo(float Target, float X, float Vx);
static bool CanRescue(const CHookBotSim &S, vec2 Free, vec2 Stuck);
static bool HookableNear(const CHookBotSim &S, vec2 P, float Reach, float Above);
// tee 0 of the sim at rest on the ground
static bool AtRestSim(const CHookBotSim &S)
{
	const vec2 P = S.m_aTee[0].m_Core.m_Pos;
	const float h = CCharacterCore::PhysicalSize() / 2;
	return length(S.m_aTee[0].m_Core.m_Vel) < 0.6f && (S.m_pCollision->CheckPoint(P.x + h, P.y + h + 5) || S.m_pCollision->CheckPoint(P.x - h, P.y + h + 5));
}

void CHookBotBrain::Start(int Mode)
{
	m_Mode = Mode;
	m_Phase = 0;
	m_Plan = {};
	m_HoldBlocked = false;
	m_PfFree = false;
	m_PfFreeTicks = 0;
	FinishJob();
	m_pJob.reset();
	m_vBridge.clear();
	m_RescueEpoch++;
}

CNetObj_PlayerInput CHookBotBrain::Tick(int GameTick, const SHookBotTee &Bot, const SHookBotTee &Partner, bool BotFirst)
{
	m_PlanDeadline = PlanClock() + (int64_t)m_PlanBudgetUs * time_freq() / 1000000;
	m_pB = &Bot;
	m_pU = &Partner;
	m_BotFirst = BotFirst;
	m_Now = GameTick;
	m_pWhy = "";
	CNetObj_PlayerInput In = {};
	In.m_WantedWeapon = WEAPON_HAMMER + 1;
	In.m_TargetX = 1;
	if(m_Mode != MODE_IDLE)
	{
		Aim(In, Partner.m_Core.m_Pos - Bot.m_Core.m_Pos);
		if(m_Mode == MODE_ALED)
			Aled(In);
		else if(m_Mode == MODE_PSEUDOFLY)
			PseudoHammer(In);
		else if(m_Mode == MODE_PSEUDODRIVE)
			PseudoDrive(In);
		else if(m_Mode == MODE_HAMMERHIT)
			Hammerhit(In);
		else if(m_Mode == MODE_PLAY)
		{
			// stuck for good: both of us frozen and lying still for longer than a freeze lasts (3 s; 4 to be sure). Lying
			// outside the freeze, one of us would have thawed by then: we lie in freeze that renews it, and nobody is left to
			// get anybody out. (Rank 1 has both frozen and lying still for up to 2.8 s, someone always thaws.) Or one of
			// us lying in freeze for 20 s with no rescue coming. Then restart from the spawn, as a team does (/kill)
			CHookBotSim Probe;
			Probe.m_pCollision = m_pCollision;
			auto Lying = [&](const SHookBotTee &T) { return T.m_FreezeTime > 0 && T.m_Grounded && length(T.m_Core.m_Vel) < 1; };
			const bool StuckB = Lying(Bot) && Probe.InFreeze(Bot.m_Core.m_Pos), StuckU = Lying(Partner) && Probe.InFreeze(Partner.m_Core.m_Pos);
			m_StuckSince = StuckB || StuckU ? (m_StuckSince < 0 ? m_Now : m_StuckSince) : -1;
			m_BothStuckSince = Lying(Bot) && Lying(Partner) ? (m_BothStuckSince < 0 ? m_Now : m_BothStuckSince) : -1;
			// both free and standing still (for the walking rule's drop: someone has to go first)
			auto Idle = [&](const SHookBotTee &T) { return T.m_FreezeTime == 0 && T.m_Grounded && length(T.m_Core.m_Vel) < 0.5f; };
			m_BothIdleSince = Idle(Bot) && Idle(Partner) ? (m_BothIdleSince < 0 ? m_Now : m_BothIdleSince) : -1;
			// or no way on for 40 s (the route's waypoint the same all that time: both of us standing where no move was
			// found, nobody frozen, it waited for good)
			if(m_RouteIndex != m_ProgressIndex)
			{
				m_ProgressIndex = m_RouteIndex;
				m_ProgressSince = m_Now;
			}
			const bool NoWayOn = !m_vRoute.empty() && m_Now - m_ProgressSince >= 40 * SERVER_TICK_SPEED;
			// (only between two bots: with a person, the person decides when to start over)
			if(m_PartnerIsBot && ((m_BothStuckSince >= 0 && m_Now - m_BothStuckSince >= 4 * SERVER_TICK_SPEED) || (m_StuckSince >= 0 && m_Now - m_StuckSince >= 20 * SERVER_TICK_SPEED) || NoWayOn))
			{
				m_Say(m_BothStuckSince >= 0 ? "we're both stuck in the freeze, restarting" : NoWayOn ? "no way on from here, restarting" : "no way to get you out, restarting");
				m_ProgressSince = m_Now;
				m_StuckSince = m_BothStuckSince = -1;
				m_Restarts++;
				Start(MODE_PLAY);
				m_Solo = {};
				m_pSoloJob.reset();
				m_Team = {};
				m_TeamEndKind = TEAM_NONE;
				m_CoastDir = 0;
				m_Kill();
				m_pB = nullptr;
				m_pU = nullptr;
				return In;
			}
			if(!m_vRoute.empty())
				UpdateRoute();
			ReadGestures();
			Play(In);
		}
		else
			Hookfly(In);
		// a nod (yes, understood): my aim up and down, unless I'm aiming a hook or a hammer right now
		if(m_Now < m_NodUntil && !In.m_Hook && !In.m_Fire)
		{
			In.m_TargetX = 0;
			In.m_TargetY = (m_NodUntil - m_Now) / 4 % 2 ? 64 : -64;
		}
	}
	else
		m_Phase = 0;
	// fire is a press counter (odd = held)
	if(In.m_Fire && !(m_Fire & 1))
		m_Fire++;
	else if(!In.m_Fire && (m_Fire & 1))
		m_Fire++;
	In.m_Fire = m_Fire;
	m_Input = In;
	// the tee states only live for this call (the callers pass temporaries)
	m_pB = nullptr;
	m_pU = nullptr;
	return In;
}

int CHookBotBrain::ParseCommand(const char *pMessage, const char *pName)
{
	// ignore the name itself so a name like "aled" still works
	char aMsg[256];
	str_copy(aMsg, pMessage);
	if(char *pFound = (char *)str_find_nocase(aMsg, pName))
		for(int i = 0; pName[i] && pFound[i]; i++)
			pFound[i] = ' ';
	if(str_find_nocase(aMsg, "stop") || str_find_nocase(aMsg, "idle"))
		return MODE_IDLE;
	if(str_find_nocase(aMsg, "aled"))
		return MODE_ALED;
	if(str_find_nocase(aMsg, "play"))
		return MODE_PLAY;
	if(str_find_nocase(aMsg, "hammerhit") || str_find_nocase(aMsg, "hh"))
		return MODE_HAMMERHIT;
	if(str_find_nocase(aMsg, "pseudo") || str_find_nocase(aMsg, "pf"))
		return str_find_nocase(aMsg, "driv") ? MODE_PSEUDODRIVE : MODE_PSEUDOFLY;
	if(str_find_nocase(aMsg, "hookfly") || str_find_nocase(aMsg, "hf"))
		return MODE_HOOKFLY;
	return -1;
}

bool CHookBotBrain::ParseGoal(const char *pMessage, bool *pHere, vec2 *pTile)
{
	const char *pGoal = str_find_nocase(pMessage, "goal");
	if(!pGoal)
		return false;
	float x, y;
	if(sscanf(pGoal + 4, "%f %f", &x, &y) == 2)
	{
		*pHere = false;
		*pTile = vec2(x, y);
		return true;
	}
	*pHere = true;
	return true;
}

void CHookBotBrain::SetGoal(vec2 Tile)
{
	auto pField = std::make_shared<CHookBotGoalField>();
	pField->Build(m_pCollision, Tile);
	m_pGoal = pField;
	auto pAir = std::make_shared<CHookBotGoalField>();
	pAir->Build(m_pCollision, Tile, 0);
	m_pGoalAir = pAir;
	m_pGoalSolo = pField;
	m_pGoalFar = pField;
}

void CHookBotBrain::RoutePath(const char *pMapBaseName, char *pOut, int OutSize)
{
	char aName[IO_MAX_PATH_LENGTH];
	str_copy(aName, pMapBaseName);
	const char *pSep = str_rchr(aName, '_');
	if(pSep)
	{
		const int Len = str_length(pSep + 1);
		bool Hex = Len == 64 || Len == 8;
		for(const char *p = pSep + 1; Hex && *p; p++)
			Hex = (*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f');
		if(Hex)
			aName[pSep - aName] = '\0';
	}
	str_format(pOut, OutSize, "hookbot/routes/%s.txt", aName);
}

bool CHookBotBrain::ParseRoutes(const char *pText, std::vector<std::vector<vec2>> *pOut)
{
	pOut->clear();
	pOut->emplace_back();
	const char *p = pText;
	while(*p)
	{
		float x, y;
		if(str_startswith(p, "# path") && !pOut->back().empty())
			pOut->emplace_back();
		else if(*p != '#' && sscanf(p, "%f %f", &x, &y) == 2)
			pOut->back().emplace_back(x, y);
		while(*p && *p != '\n')
			p++;
		if(*p)
			p++;
	}
	if(pOut->back().empty())
		pOut->pop_back();
	return !pOut->empty();
}

bool CHookBotBrain::ParseRoute(const char *pText, std::vector<vec2> *pOut)
{
	std::vector<std::vector<vec2>> vvPaths;
	pOut->clear();
	if(!ParseRoutes(pText, &vvPaths))
		return false;
	*pOut = vvPaths[0];
	return true;
}

void CHookBotBrain::StartRoute(const std::vector<vec2> &vRoute, vec2 Me, vec2 Partner, int Index)
{
	m_vvRoutes.clear();
	m_RoutePath = 0;
	m_vRoute = vRoute;
	PickRouteIndex(Me, Partner, Index);
}

bool CHookBotBrain::StartRouteText(const char *pText, vec2 Me, vec2 Partner, char *pMsg, int MsgSize)
{
	std::vector<std::vector<vec2>> vvPaths;
	if(!ParseRoutes(pText, &vvPaths))
		return false;
	// the line I'm on: the one I can see closest to me, clearly (else the first: at the start the lines run together)
	int Path = 0;
	if(m_pCollision && vvPaths.size() > 1)
	{
		int Seg;
		float First = PathDist(vvPaths[0], 0, (int)vvPaths[0].size(), Me, &Seg), Best = First;
		for(int p = 1; p < (int)vvPaths.size(); p++)
		{
			const float D = PathDist(vvPaths[p], 0, (int)vvPaths[p].size(), Me, &Seg);
			if(D < Best && D < First - 64)
			{
				Best = D;
				Path = p;
			}
		}
	}
	StartRoute(vvPaths[Path], Me, Partner);
	if(vvPaths.size() > 1)
	{
		m_vvRoutes = vvPaths;
		m_RoutePath = Path;
	}
	m_NextPathCheck = 0;
	str_format(pMsg, MsgSize, "ok, following the route (%d waypoints%s), first: %d (tile %.0f %.0f)", (int)m_vRoute.size(),
		vvPaths.size() > 1 ? ", a line for each of us" : "", m_RouteIndex + 1, m_vRoute[m_RouteIndex].x, m_vRoute[m_RouteIndex].y);
	return true;
}

void CHookBotBrain::PickRouteIndex(vec2 Me, vec2 Partner, int Index)
{
	// the earliest waypoint near us (a map's finish can be right next to its spawn, so the nearest one may be the
	// last one); if we're already there, the one after it
	const vec2 Mid = (Me + Partner) / 2 / 32.0f;
	float Nearest = 1e9f;
	for(const vec2 &W : m_vRoute)
		Nearest = std::min(Nearest, distance(W, Mid));
	int Best = 0;
	while(Best + 1 < (int)m_vRoute.size() && distance(m_vRoute[Best], Mid) > std::max(16.0f, Nearest + 4))
		Best++;
	if(distance(m_vRoute[Best], Mid) * 32 < m_RouteNear && Best + 1 < (int)m_vRoute.size())
		Best++;
	if(Index >= 0)
		Best = std::min(Index, (int)m_vRoute.size() - 1);
	m_RouteIndex = Best;
	// on a thread (the fields for a goal are a few full-map searches, up to half a second: synchronously, a re-pick
	// after a kill or a big jump froze the game that long; the old goal stays until they're ready)
	SetRouteGoal(Best, !m_pGoal, (Me + Partner) / 2);
}

void CHookBotBrain::SetRouteGoal(int Index, bool Sync, vec2 PairMid)
{
	if(m_pGoalJob && m_pGoalJob->m_Thread.joinable())
		m_pGoalJob->m_Thread.join();
	m_pGoalJob = std::make_unique<SGoalJob>();
	SGoalJob &J = *m_pGoalJob;
	J.m_Tile = RouteGoal(Index);
	J.m_SoloTile = m_vRoute[std::min(Index + m_SoloLead, (int)m_vRoute.size() - 1)];
	J.m_FarTile = m_vRoute[std::min(Index + 2 * m_SoloLead, (int)m_vRoute.size() - 1)];
	// flying needs a goal reachable through open air: the farthest of the next waypoints that is (the run the route
	// came from may pass freeze further on, e.g. up through a freeze ceiling)
	for(int k = m_RouteLead; k >= 0; k--)
		J.m_vAirTiles.push_back(m_vRoute[std::min(Index + k, (int)m_vRoute.size() - 1)]);
	J.m_From = PairMid;
	auto Work = [](SGoalJob *pJ, const CCollision *pCollision) {
		pJ->m_pField = std::make_shared<CHookBotGoalField>();
		pJ->m_pField->Build(pCollision, pJ->m_Tile);
		pJ->m_pSolo = std::make_shared<CHookBotGoalField>();
		pJ->m_pSolo->Build(pCollision, pJ->m_SoloTile);
		pJ->m_pFar = std::make_shared<CHookBotGoalField>();
		pJ->m_pFar->Build(pCollision, pJ->m_FarTile);
		CHookBotSim Probe;
		Probe.m_pCollision = const_cast<CCollision *>(pCollision);
		// what open air connects to where we are: one search for all the candidates below (it used to be one per
		// candidate, up to 24 full-map searches per waypoint; open-air connection goes both ways). Where we are isn't
		// open air (we're in freeze): per candidate, as before
		CHookBotGoalField Here;
		Here.Build(pCollision, pJ->m_From / 32.0f, 0);
		const vec2 FromTile = vec2(std::floor(pJ->m_From.x / 32), std::floor(pJ->m_From.y / 32)) * 32.0f + vec2(16, 16);
		const bool HereOpen = !Probe.InFreeze(FromTile) && !pCollision->CheckPoint(FromTile);
		pJ->m_SoloByAir = !HereOpen || Here.Dist(vec2(std::floor(pJ->m_SoloTile.x), std::floor(pJ->m_SoloTile.y)) * 32.0f + vec2(16, 16)) < 1e5f;
		for(vec2 Tile : pJ->m_vAirTiles)
		{
			const bool First = Tile == pJ->m_vAirTiles[0];
			// a waypoint in freeze (the run passed it frozen): the nearest open tile next to it, the nearer to us first
			std::vector<vec2> vTry = {Tile};
			for(int r = 1; r <= 4; r++)
				for(int dy = -r; dy <= r; dy++)
					for(int dx = -r; dx <= r; dx++)
						if(std::max(std::abs(dx), std::abs(dy)) == r)
						{
							const vec2 T = Tile + vec2(dx, dy), C = T * 32.0f + vec2(16, 16);
							if(!Probe.InFreeze(C) && !pCollision->CheckPoint(C))
								vTry.push_back(T);
						}
			const vec2 C0 = Tile * 32.0f;
			if(Probe.InFreeze(C0) || pCollision->CheckPoint(C0))
			{
				vTry.erase(vTry.begin());
				std::stable_sort(vTry.begin(), vTry.end(), [&](vec2 a, vec2 b) { return distance(a * 32.0f, C0) + 0.01f * distance(a * 32.0f, pJ->m_From) <
													      distance(b * 32.0f, C0) + 0.01f * distance(b * 32.0f, pJ->m_From); });
			}
			bool Found = false;
			for(int i = 0; i < (int)vTry.size() && i < 6 && !Found; i++)
			{
				if(HereOpen && Here.Dist(vec2(std::floor(vTry[i].x), std::floor(vTry[i].y)) * 32.0f + vec2(16, 16)) >= 1e5f)
					continue;
				pJ->m_pAir = std::make_shared<CHookBotGoalField>();
				pJ->m_pAir->Build(pCollision, vTry[i], 0);
				Found = pJ->m_pAir->Dist(pJ->m_From) < 1e5f;
			}
			if(Found)
			{
				pJ->m_GoalByAir = First;
				break;
			}
		}
		if(!pJ->m_pAir)
		{
			// every waypoint ahead is deep in freeze (a line the run took frozen, like down a freeze column): nothing to
			// fly to (an air field from inside the freeze reaches nowhere)
			pJ->m_pAir = std::make_shared<CHookBotGoalField>();
			pJ->m_pAir->Build(pCollision, pJ->m_Tile, 0);
		}
		if(pJ->m_pAir->Dist(pJ->m_From) >= 1e5f)
		{
			// nothing ahead on the route is reachable through air from where we are (e.g. we fell back below a freeze
			// ceiling the route goes through): fly to the spot we can reach through air that's closest to the goal
			// That's for a freeze ceiling the route goes up through (you throw me from there); for a freeze wall to the
			// side it's an aled, which needs one of us frozen: flying up to the wall only makes us hover there
			const vec2 Stage = Here.BestReachable(*pJ->m_pField);
			const vec2 C = Stage * 32.0f + vec2(16, 16);
			const float Up = pJ->m_pField->Dist(C - vec2(0, 64));
			if(Up < pJ->m_pField->Dist(C - vec2(64, 0)) && Up < pJ->m_pField->Dist(C + vec2(64, 0)))
			{
				pJ->m_pAir = std::make_shared<CHookBotGoalField>();
				pJ->m_pAir->Build(pCollision, Stage, 0);
			}
		}
		pJ->m_Done = true;
	};
	if(m_AsyncPlanning && !Sync)
		J.m_Thread = std::thread(Work, &J, m_pCollision);
	else
	{
		Work(&J, m_pCollision);
		m_pGoal = J.m_pField;
		m_pGoalAir = J.m_pAir;
		m_GoalByAir = J.m_GoalByAir;
		m_SoloByAir = J.m_SoloByAir;
		m_pGoalSolo = J.m_pSolo;
		m_pGoalFar = J.m_pFar;
		m_pGoalJob.reset();
	}
}

void CHookBotBrain::UpdateRoute()
{
	if(m_pGoalJob)
	{
		if(!m_pGoalJob->m_Done)
			return;
		if(m_pGoalJob->m_Thread.joinable())
			m_pGoalJob->m_Thread.join();
		m_pGoal = m_pGoalJob->m_pField;
		m_pGoalAir = m_pGoalJob->m_pAir;
		m_GoalByAir = m_pGoalJob->m_GoalByAir;
		m_SoloByAir = m_pGoalJob->m_SoloByAir;
		m_pGoalSolo = m_pGoalJob->m_pSolo;
		m_pGoalFar = m_pGoalJob->m_pFar;
		m_pGoalJob.reset();
		// a plan in progress stays (e.g. pulling the partner through a wall right after an aled): the next goal is
		// only a few tiles further along, and the next plans use it
	}
	if(m_RouteIndex < 0)
		return;
	{
		// a kill and respawn (or a teleport), or we're nowhere near the stretch of the route we were on: pick where
		// on the route we are again, like at the start (after a kill at Stronghold it kept the waypoint of the
		// previous attempt, and from the bottom of the first shaft that goal pointed back towards the start)
		const vec2 Pos[2] = {m_pB->m_Core.m_Pos, m_pU->m_Core.m_Pos};
		bool Jumped = false;
		for(int i = 0; i < 2; i++)
		{
			Jumped |= m_aRouteLastPos[i] != vec2(0, 0) && distance(Pos[i], m_aRouteLastPos[i]) > 4 * 32;
			m_aRouteLastPos[i] = Pos[i];
		}
		float Near = 1e9f;
		for(int i = std::max(0, m_RouteIndex - 2); i <= std::min(m_RouteIndex + 4, (int)m_vRoute.size() - 1); i++)
			for(const vec2 &P : Pos)
				Near = std::min(Near, distance(P, m_vRoute[i] * 32.0f));
		if(Jumped || Near > 30 * 32)
		{
			const int Old = m_RouteIndex;
			PickRouteIndex(Pos[0], Pos[1]);
			if(m_RouteIndex != Old)
			{
				char aBuf[64];
				str_format(aBuf, sizeof(aBuf), "back to waypoint %d/%d", m_RouteIndex + 1, (int)m_vRoute.size());
				m_Say(aBuf);
			}
			return;
		}
	}
	UpdateRoutePath();
	if(m_RouteIndex + 1 >= (int)m_vRoute.size())
		return;
	// where one of us is frozen for good isn't progress (it may lie right next to later waypoints, e.g. at the bottom
	// of a shaft my line comes back to later): then only the other one counts. The same for both of us, whichever is
	// stuck (with only the partner's counted out, the stuck one's brain moved the route on and the other's didn't, and
	// their team plans differed: each played half of a different plan)
	CHookBotSim Probe;
	Probe.m_pCollision = m_pCollision;
	auto Stuck = [&](const SHookBotTee *pT) { return pT->m_FreezeTime > 0 && pT->m_Grounded && length(pT->m_Core.m_Vel) < 1 && Probe.InFreeze(pT->m_Core.m_Pos); };
	const bool BStuck = Stuck(m_pB), UStuck = Stuck(m_pU);
	const vec2 BPos = BStuck && !UStuck ? m_pU->m_Core.m_Pos : m_pB->m_Core.m_Pos, UPos = UStuck && !BStuck ? m_pB->m_Core.m_Pos : m_pU->m_Core.m_Pos;
	const vec2 W = m_vRoute[m_RouteIndex] * 32.0f;
	const float dB = distance(BPos, W), dU = distance(UPos, W);
	int Next = std::min(dB, dU) < m_RouteNear && std::max(dB, dU) < m_RouteFar ? m_RouteIndex + 1 : m_RouteIndex;
	// we can take a different line than the run the route came from (e.g. along the floor where it flew): once we
	// are nearer to a later stretch of the route than to the one leading to the current waypoint, move on. Only what
	// we can get to through open air counts (above a freeze band, a stretch of the route just below it isn't near)
	const vec2 Mid = (BPos + UPos) / 2;
	// (these two every 5 ticks: they're the costly part, and a waypoint 0.1 s later makes no difference)
	const bool Check = m_Now % 5 == 0;
	if(Check)
	{
		int Nearest = -1, Seg;
		float NearestDist = 1e9f;
		for(int i = std::max(0, m_RouteIndex - 1); i < std::min(m_RouteIndex + 4, (int)m_vRoute.size() - 1); i++)
		{
			const float D = PathDist(m_vRoute, i, i + 1, Mid, &Seg, true);
			if(D < NearestDist)
			{
				NearestDist = D;
				Nearest = i;
			}
		}
		if(Nearest >= m_RouteIndex && NearestDist < m_RouteFar)
			Next = std::max(Next, Nearest + 1);
	}
	// or by progress, whatever line we take: once one of us is closer to the goal (as the goal field measures it)
	// than a waypoint before it is, we're past that waypoint (we went straight down Stronghold's first shaft where the
	// run flew diagonally, never came near its waypoints, and the goal stayed behind us). (By the one of us behind
	// instead, the goal stayed a waypoint back on the corners of a post in Stronghold's big room, and the joint move
	// from there found nothing; the one behind has its own solo goal, m_aOwnIndex)
	if(m_pGoal && Check)
	{
		const float Ours = std::min(m_pGoal->Dist(BPos), m_pGoal->Dist(UPos));
		const int GoalIndex = std::min(m_RouteIndex + m_RouteLead, (int)m_vRoute.size() - 1);
		int k = m_RouteIndex;
		while(k < GoalIndex && m_pGoal->Dist(m_vRoute[k] * 32.0f + vec2(16, 16)) > Ours + 32)
			k++;
		Next = std::max(Next, k);
	}
	if(Next > m_RouteIndex)
	{
		m_RouteIndex = Next;
		SetRouteGoal(m_RouteIndex, false, Mid);
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "next: %d/%d (tile %.0f %.0f)", m_RouteIndex + 1, (int)m_vRoute.size(), m_vRoute[m_RouteIndex].x, m_vRoute[m_RouteIndex].y);
		m_Say(aBuf);
	}
	UpdateOwnRoute(Check);
}

void CHookBotBrain::UpdateOwnRoute(bool Check)
{
	const int Last = (int)m_vRoute.size() - 1;
	if(m_RouteIndex < 0 || Last < 0)
		return;
	// a new line, or back to an earlier waypoint (a restart): from the shared index again
	if(m_OwnPath != m_RoutePath || m_aOwnIndex[0] < 0)
	{
		m_aOwnIndex[0] = m_aOwnIndex[1] = m_RouteIndex;
		m_OwnPath = m_RoutePath;
	}
	const SHookBotTee *apT[2] = {m_pB, m_pU};
	for(int i = 0; i < 2; i++)
	{
		int &Own = m_aOwnIndex[i];
		if(Own > m_RouteIndex + 6 || Own < m_RouteIndex - 12)
			Own = m_RouteIndex;
		const vec2 P = apT[i]->m_Core.m_Pos;
		while(Own < std::min(Last, m_RouteIndex + 6) && distance(P, m_vRoute[Own] * 32.0f) < m_RouteNear)
			Own++;
		if(Check && apT[i]->m_FreezeTime == 0)
		{
			int Seg;
			float Nearest = 1e9f;
			int NearestSeg = -1;
			for(int k = std::max(0, Own - 1); k < std::min(Own + 4, Last); k++)
			{
				const float D = PathDist(m_vRoute, k, k + 1, P, &Seg, true);
				if(D < Nearest)
				{
					Nearest = D;
					NearestSeg = k;
				}
			}
			if(NearestSeg >= Own && Nearest < m_RouteFar)
				Own = NearestSeg + 1;
		}
		Own = std::clamp(Own, std::max(0, m_RouteIndex - 12), std::min(Last, m_RouteIndex + 6));
	}
	// my solo goal: on from my own index, with freeze at 40x (also where that's the shared solo goal: with its 8x the way
	// from the gap at the start of Stronghold's swing course to the waypoint 7 on went up through 5 rows of a freeze
	// band, and every first swing went into the pocket under it; the shared one stays at 8x for the team moves)
	const vec2 Tile = m_vRoute[std::min(m_aOwnIndex[0] + m_SoloLead, Last)];
	if(m_pOwnJob && m_pOwnJob->m_Done)
	{
		if(m_pOwnJob->m_Thread.joinable())
			m_pOwnJob->m_Thread.join();
		m_pOwnSolo = m_pOwnJob->m_pField;
		m_OwnSoloTile = m_pOwnJob->m_Tile;
		m_pOwnJob.reset();
	}
	if(Tile == m_OwnSoloTile || (m_pOwnJob && m_pOwnJob->m_Tile == Tile))
		return;
	if(m_pOwnJob && m_pOwnJob->m_Thread.joinable())
		m_pOwnJob->m_Thread.join();
	m_pOwnJob = std::make_unique<SOwnJob>();
	m_pOwnJob->m_Tile = Tile;
	for(int k = std::max(0, m_aOwnIndex[0] - 2); k <= std::min(m_aOwnIndex[0] + m_SoloLead, Last); k++)
		m_pOwnJob->m_vLine.push_back(m_vRoute[k]);
	auto Work = [](SOwnJob *pJ, const CCollision *pCollision) {
		// (freeze at 40x, not 8x: a solo swing can't pass it, and across 5 rows of freeze looked shorter than the way round
		// through the gap at its end, under Stronghold's freeze band at y 201-205; for the shared solo goal too it kept
		// us at the spawn for 16 s, where the way on is a drop into the freeze)
		// (HH_ALONG: along the route, off it 4x; where the route turns back, as in Stronghold's swing course, the way to the
		// waypoint 7 on goes round the far side, and a swing along a freeze band's top, below anything to climb by, was
		// as short as the route's way up first and along between the bands. With it, our solo swings kept to rank 1's
		// line through the course, fast and apart, and fell into the top band; the climbs, which carried us across it,
		// didn't start: through in 1 of 6 runs, without it 4 of 6)
		pJ->m_pField = std::make_shared<CHookBotGoalField>();
		if(getenv("HH_ALONG"))
			pJ->m_pField->BuildAlong(pCollision, pJ->m_Tile, 40, pJ->m_vLine, 5, 4);
		else
			pJ->m_pField->Build(pCollision, pJ->m_Tile, 40);
		pJ->m_Done = true;
	};
	if(m_AsyncPlanning)
		m_pOwnJob->m_Thread = std::thread(Work, m_pOwnJob.get(), m_pCollision);
	else
	{
		Work(m_pOwnJob.get(), m_pCollision);
		m_pOwnSolo = m_pOwnJob->m_pField;
		m_OwnSoloTile = Tile;
		m_pOwnJob.reset();
	}
}

float CHookBotBrain::PathDist(const std::vector<vec2> &vPath, int From, int To, vec2 Me, int *pSeg, bool AirOnly) const
{
	// the points along it every 16 px, nearest first: the first one I can see is the answer (a line-of-sight check
	// walks the line pixel by pixel: checking them all, every tick, was most of the bot's time on the game's thread)
	std::vector<std::pair<float, int>> vPoints; // distance, segment * 256 + step
	From = std::max(From, 0);
	To = std::min(To, (int)vPath.size() - 1);
	for(int i = From; i < To; i++)
	{
		const vec2 A = vPath[i] * 32.0f, B = vPath[i + 1] * 32.0f;
		const int Steps = std::clamp((int)(distance(A, B) / 16), 1, 255);
		for(int k = 0; k <= Steps; k++)
			vPoints.emplace_back(distance(Me, mix(A, B, k / (float)Steps)), i * 256 + k);
	}
	std::sort(vPoints.begin(), vPoints.end());
	CHookBotSim Probe;
	Probe.m_pCollision = m_pCollision;
	for(const auto &[D, Id] : vPoints)
	{
		const int i = Id / 256, k = Id % 256;
		const vec2 A = vPath[i] * 32.0f, B = vPath[i + 1] * 32.0f;
		const vec2 P = mix(A, B, k / (float)std::clamp((int)(distance(A, B) / 16), 1, 255));
		if(!m_pCollision->IntersectLine(Me, P, nullptr, nullptr) && (!AirOnly || !FreezeOnLine(Probe, Me, P)))
		{
			*pSeg = i;
			return D;
		}
	}
	return 1e9f;
}

void CHookBotBrain::UpdateRoutePath()
{
	if(m_vvRoutes.size() < 2 || m_RouteIndex < 0 || m_Now < m_NextPathCheck)
		return;
	m_NextPathCheck = m_Now + 10;
	const vec2 Me = m_pB->m_Core.m_Pos;
	// only the parts of the lines I can see count: a line on the other side of a wall isn't mine even if it's close
	// (on top of Stronghold's 3-tile post right of the ledge, the shaft line runs 4 tiles to my left, behind the post
	// and the wall above it; the other tee's line down the freeze column is right next to me)
	int Seg = m_RouteIndex;
	const float Mine = PathDist(m_vRoute, m_RouteIndex - 3, m_RouteIndex + 4, Me, &Seg);
	const vec2 Here = m_vRoute[m_RouteIndex];
	int BestPath = -1, BestSeg = 0;
	float Best = 1e9f;
	for(int p = 0; p < (int)m_vvRoutes.size(); p++)
	{
		if(p == m_RoutePath)
			continue;
		// the stretch of that path around where we are on mine
		const std::vector<vec2> &vPath = m_vvRoutes[p];
		int Near = 0;
		for(int i = 1; i < (int)vPath.size(); i++)
			if(distance(vPath[i], Here) < distance(vPath[Near], Here))
				Near = i;
		int S = Near;
		const float D = PathDist(vPath, Near - 6, Near + 6, Me, &S);
		if(D < Best)
		{
			Best = D;
			BestPath = p;
			BestSeg = S;
		}
	}
	// clearly on the other line, and not near mine (where the lines run together or side by side, it makes no
	// difference which one I follow, and switching back and forth between them does harm)
	if(BestPath < 0 || Best > 5 * 32 || Best > Mine - 4 * 32)
		return;
	m_RoutePath = BestPath;
	m_vRoute = m_vvRoutes[BestPath];
	m_RouteIndex = std::min(BestSeg + 1, (int)m_vRoute.size() - 1);
	SetRouteGoal(m_RouteIndex, false, (Me + m_pU->m_Core.m_Pos) / 2);
	m_NextPathCheck = m_Now + SERVER_TICK_SPEED; // don't flip back and forth
	char aBuf[96];
	str_format(aBuf, sizeof(aBuf), "I'm on the other line: next %d/%d (tile %.0f %.0f)", m_RouteIndex + 1, (int)m_vRoute.size(), m_vRoute[m_RouteIndex].x,
		m_vRoute[m_RouteIndex].y);
	m_Say(aBuf);
}

int CHookBotBrain::ParseDirection(const char *pMessage)
{
	if(str_find_nocase(pMessage, "left"))
		return -1;
	if(str_find_nocase(pMessage, "right"))
		return 1;
	if(str_find_nocase(pMessage, " up") || str_find_nocase(pMessage, "straight"))
		return 0;
	return 2;
}

void CHookBotBrain::ReadGestures()
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	const CNetObj_PlayerInput &PI = pU->m_Input;
	const bool Press = PI.m_Fire != m_PrevPartnerFire && (PI.m_Fire & 1);
	m_PrevPartnerFire = PI.m_Fire;
	// (a partner that is a brain too doesn't gesture: its rescue hammers aimed up read as "let's fly" a tick later, once it
	// was 70 px away, and I started a fly it knew nothing about, over Stronghold's freeze floor)
	if(!Press || pU->m_FreezeTime > 0 || m_PartnerIsBot)
		return;
	// a swing aimed up that can't hit me (the universal "let's fly"; a swing at me from below is a pseudofly hammer)
	const bool Up = PI.m_TargetY < 0 && absolute(PI.m_TargetX) < -PI.m_TargetY * 0.6f;
	if(Up && distance(pB->m_Core.m_Pos, pU->m_Core.m_Pos) > 70)
	{
		m_FlyWishUntil = m_Now + 5 * SERVER_TICK_SPEED;
		m_NodUntil = m_Now + 24;
	}
}

// towards m_pGoal with whatever works: when one of us is frozen, the hammerhit rescue (aleds included); when we're
// both free in the air and climbing gets us closer, a pseudofly (the upper one drives and steers along the goal
// field, the lower one hammers it up as long as that helps; so it hovers at the goal); else walk or fall
void CHookBotBrain::Play(CNetObj_PlayerInput &In)
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	const vec2 B = pB->m_Core.m_Pos, U = pU->m_Core.m_Pos;
	const bool Free = pB->m_FreezeTime == 0 && pU->m_FreezeTime == 0;
	// a team move (a throw, a catch) goes first: the partner is playing its half of it
	if(TeamMove(In))
		return;
	// fly only where the goal is reachable through open air (a freeze wall in the way takes an aled, not a fly)
	auto ClimbHelps = [&](vec2 P) { return m_pGoalAir && m_pGoalAir->Dist(P - vec2(0, 96)) < m_pGoalAir->Dist(P) - 40; };
	// the partner asked for a fly (swing aimed up): fly even where I wouldn't have; from standing, I jump to start it
	// (I drive, it hammers)
	const bool Wish = m_Now < m_FlyWishUntil;
	if(Wish && Free && pB->m_Grounded && pU->m_Grounded && distance(B, U) < 160)
	{
		m_pWhy = "fly wish: jump";
		In.m_Direction = 0;
		In.m_Hook = 0;
		In.m_Jump = !m_Input.m_Jump;
		return;
	}
	const vec2 Mid = (B + U) / 2;
	// both of us swinging on our own, close together, and climbing gets us closer: the swings give way to a fly (one of
	// us hovered on its swings beside the foot of Stronghold's unhookable shaft after the hookable blocks, where only a
	// fly gets up, until the other one came; with the swings going on, they never flew)
	const bool FlyNow = m_PartnerIsBot && m_Now >= m_NoFlyUntil && m_pGoalAir && Free && !pB->m_Grounded && !pU->m_Grounded && distance(B, U) < 250 && m_pGoalAir->Dist(Mid) < 1e5f && ClimbHelps(Mid);
	if(FlyNow && (m_Solo.m_Valid || m_pSoloJob || m_CoastDir))
	{
		if(m_pSoloJob && m_pSoloJob->m_Thread.joinable())
			m_pSoloJob->m_Thread.join();
		m_pSoloJob.reset();
		m_Solo = {};
		m_SoloRest = {};
		m_CoastDir = 0;
	}
	// in the middle of a swing plan on my own (or about to go on with the next one): see it through, whatever the
	// partner does meanwhile
	if(!FlyNow && pB->m_FreezeTime == 0 && m_pGoal && ((m_Solo.m_Valid && m_Solo.m_Swing) || (m_pSoloJob && m_pSoloJob->m_Air) || (m_CoastDir && !pB->m_Grounded)) &&
		SoloMove(In, PartnerStuckForGood()))
	{
		m_pWhy = "solo: swing";
		return;
	}
	if(!m_vRoute.empty() && m_pGoalAir && !m_pGoalJob && Free && !pB->m_Grounded && !pU->m_Grounded && m_pGoalAir->Dist(Mid) >= 1e5f && m_Now >= m_NextAirRebuild)
	{
		// we're somewhere the flying goal can't be reached from (fell back down): pick it again from here
		SetRouteGoal(m_RouteIndex, false, Mid);
		m_NextAirRebuild = m_Now + SERVER_TICK_SPEED;
	}
	const bool Fly = (m_pGoalAir && Free && !pB->m_Grounded && !pU->m_Grounded && distance(B, U) < 250 && m_pGoalAir->Dist(Mid) < 1e5f &&
				 (ClimbHelps(Mid) || m_pGoalAir->Dist(Mid) < 4 * 32)) ||
			 (Wish && Free && (!pB->m_Grounded || !pU->m_Grounded) && distance(B, U) < 250);
	// thrown up at freeze that the route goes through (Stronghold: through the freeze ceiling onto the blocks): steer
	// the way the route goes on from there, don't fight the throw (following the partner's x, as the pseudofly
	// hammerer does, killed the sideways speed that lands it on the blocks)
	auto ThrowSteer = [&]() {
		if(!m_pGoal || pB->m_FreezeTime > 0 || pB->m_Core.m_Vel.y > -8)
			return false;
		CHookBotSim Probe;
		Probe.m_pCollision = m_pCollision;
		bool FreezeAbove = false;
		for(float d = 16; d <= 128 && !FreezeAbove; d += 8)
			FreezeAbove = Probe.InFreeze(B - vec2(0, d));
		if(!FreezeAbove || m_pGoal->Dist(B - vec2(0, 128)) > m_pGoal->Dist(B) - 64)
			return false;
		const vec2 Above = B - vec2(0, 160);
		const float L = m_pGoal->Dist(Above + vec2(-64, 0)), R = m_pGoal->Dist(Above + vec2(64, 0));
		In.m_Direction = L < R - 8 ? -1 : R < L - 8 ? 1 : 0;
		return true;
	};
	// the partner is free but cut off: freeze between us and it's much further from the goal (e.g. I'm on the blocks
	// above Stronghold's freeze ceiling, it's below): stay above it and pull it up through when it jumps into reach
	if(m_Now >= m_NextCutOffCheck)
	{
		m_CutOff = m_pGoal && Free && m_pGoal->Dist(U) > m_pGoal->Dist(B) + 200 && !AirConnected(B, U);
		m_NextCutOffCheck = m_Now + 5;
	}
	const bool CutOff = m_CutOff && Free;
	if(m_DebugFly)
		printf("flydbg %d id %d: fly %d free %d grounded %d/%d dist %.0f air %.0f climb %d wish %d cutoff %d phase %d\n", m_Now, pB->m_Core.m_Id, Fly, Free, pB->m_Grounded, pU->m_Grounded, distance(B, U),
			m_pGoalAir ? m_pGoalAir->Dist(Mid) : -1.0f, ClimbHelps(Mid), Wish, CutOff, m_Phase);
	if(!Fly || CutOff)
	{
		if(m_Phase == 3)
			m_Phase = 0;
		// both of us standing safely, nobody to rescue: move on by myself if I can land somewhere further along
		CHookBotSim Probe;
		Probe.m_pCollision = m_pCollision;
		// (the partner on the ground, or far off: one of us swinging on ahead along Stronghold's hookable blocks after the
		// zig-zag's room never landed, and the other stood in its gap for good)
		const bool Calm = !CutOff && Free && (pU->m_Grounded || distance(B, U) > 12 * 32) && !Probe.InFreeze(U) && (pB->m_Grounded || m_Solo.m_Valid);
		// the partner is frozen for good (lying in freeze) and no rescue works from where I stand: go on by myself,
		// through freeze too if I land where I thaw, to where one does. Every new spot gets a second of rescue
		// planning first (Stronghold: from the platform below the ledge through the freeze onto the pillar in the
		// freeze floor, then back to the shaft's side opening, the partner lying at the bottom of the shaft)
		const bool PartnerStuck = PartnerStuckForGood();
		if(!PartnerStuck || m_Plan.m_Valid || (pB->m_Grounded && pB->m_FreezeTime == 0 && !m_Solo.m_Valid && distance(B, m_RescueTryPos) > 64))
		{
			m_RescueTryPos = B;
			m_RescueTrySince = m_Now;
		}
		// an alone move that got me nowhere (standing again within 2 tiles of where it started): no more of them from
		// here for a while, rescue planning gets the time
		if(m_AloneMoveFrom.x > -1e8f && pB->m_Grounded && !m_Solo.m_Valid && !m_pSoloJob)
		{
			if(distance(B, m_AloneMoveFrom) < 64)
			{
				m_AloneStuckPos = B;
				m_AloneStuckUntil = m_Now + 3 * SERVER_TICK_SPEED;
			}
			m_AloneMoveFrom = vec2(-1e9f, -1e9f);
		}
		const bool AloneStuck = m_Now < m_AloneStuckUntil && distance(B, m_AloneStuckPos) < 64;
		const bool Alone = !CutOff && PartnerStuck && !m_Plan.m_Valid && pB->m_FreezeTime == 0 && m_Now - m_RescueTrySince > SERVER_TICK_SPEED &&
				   (pB->m_Grounded || m_Solo.m_Valid) && !AloneStuck;
		// the partner far behind, its way on a fall through freeze: I go where I can hook it out as it comes by, and wait
		// there (FindCatchSpot)
		// (cut off or not: the partner above, its way down through the freeze, isn't for me to pull up)
		const bool Catch = Free && pB->m_FreezeTime == 0 && (pB->m_Grounded || m_Solo.m_Valid) && FindCatchSpot();
		m_CatchGo = Catch && distance(B, m_CatchSpot) > 24;
		// (a solo move from the ground only when the partner isn't far behind: then I wait for it)
		if(m_pGoal && (Calm || m_CatchGo) && (!pB->m_Grounded || !PartnerFarBehind() || m_CatchGo) && SoloMove(In))
		{
			m_pWhy = m_CatchGo ? "solo: to catch" : "solo";
			return;
		}
		if(Catch && !m_CatchGo && pB->m_Grounded)
		{
			m_Solo = {};
			In.m_Direction = 0;
			In.m_Jump = 0;
			In.m_Hook = 0;
			m_pWhy = "wait to catch";
			return;
		}
		// (an alone search already started, e.g. as a swing chain landed, is collected whatever the rescue planning does
		// meanwhile: it walked me off the spot it was planned from, and the plan went to waste)
		if(m_pGoal && (Alone || (PartnerStuck && m_pSoloJob && !m_pSoloJob->m_Air && pB->m_FreezeTime == 0)) && SoloMove(In, true))
		{
			m_pWhy = "alone";
			return;
		}
		m_Solo = {};
		Hammerhit(In, CutOff);
		if(pU->m_FreezeTime == 0)
			ThrowSteer();
		return;
	}
	if(m_Phase != 3)
	{
		// leave the rescue behind
		m_Plan = {};
		m_vBridge.clear();
		m_RescueEpoch++;
		m_PostEnd = -1;
		m_PfFree = false;
		m_Phase = 3;
	}
	if(m_FlyStyle == 1)
	{
		// hookfly: the upper one hooks the lower one up once its own rise slows, and they alternate; steer along the
		// goal field whenever I'm not busy hooking or dodging
		m_pWhy = "hookfly";
		Hookfly(In);
		if(In.m_Direction == 0)
		{
			const float L = m_pGoalAir->Dist(B + vec2(-48, -32)), R = m_pGoalAir->Dist(B + vec2(48, -32));
			In.m_Direction = L < R - 8 ? -1 : R < L - 8 ? 1 : 0;
		}
		return;
	}
	if(B.y < U.y)
	{
		// I'm the driver: steer along the goal field (looking a bit up, where we're heading)
		const float L = m_pGoalAir ? m_pGoalAir->Dist(B + vec2(-48, -32)) : 0, R = m_pGoalAir ? m_pGoalAir->Dist(B + vec2(48, -32)) : 0;
		m_DriveDir = L >= 1e5f && R >= 1e5f ? 0 : L < R - 8 ? -1 : R < L - 8 ? 1 : 0;
		m_pWhy = "fly: drive";
		PseudoDrive(In);
	}
	else
	{
		m_pWhy = "fly: hammer";
		PseudoHammer(In);
		// only kick it up while climbing still helps (hover at the goal)
		if(In.m_Fire && !ClimbHelps(U) && !Wish)
			In.m_Fire = 0;
	}
	ThrowSteer();
}

// the spots and jumps for a solo move from Base (tee 0 is me), best found by Deadline; thread-safe
// a solo plan steps only me, the partner stays where it is: one that runs into it (closer than Keep, the players' push
// starts at 35 px) or hooks it moves it in the game and not in the plan (Stronghold, the 3-wide gap at x 236-238 after
// the zig-zag's room: my swing bumped the partner off its edge into the freeze floor below)
static bool BumpsPartner(const CHookBotSim &S, float Keep)
{
	const CCharacterCore &Me = S.m_aTee[0].m_Core;
	return distance(Me.m_Pos, S.m_aTee[1].m_Core.m_Pos) < Keep || Me.HookedPlayer() >= 0;
}

CHookBotBrain::SSoloPlan CHookBotBrain::SoloSearch(const CHookBotSim &Base, const CHookBotGoalField &Field, int64_t Deadline, bool AllowFrozen)
{
	const CCollision *pCol = Base.m_pCollision;
	const vec2 B = Base.m_aTee[0].m_Core.m_Pos;
	const float h = CCharacterCore::PhysicalSize() / 2;
	auto OnGround = [&](vec2 P) { return pCol->CheckPoint(P.x + h, P.y + h + 5) || pCol->CheckPoint(P.x - h, P.y + h + 5); };
	// the spots to try: standing places (outside freeze and kill tiles) ahead of me, the edges of platforms too, best first
	const float Mine = Field.Dist(B);
	const float PartnerKeep = std::min(36.0f, distance(B, Base.m_aTee[1].m_Core.m_Pos) - 2);
	// what open air connects to me (a clean jump can't get anywhere else: the spots beyond a tall freeze column used up
	// the whole search, and it never tried the edge before it)
	CHookBotGoalField AirHere;
	if(!AllowFrozen)
		AirHere.Build(pCol, vec2(std::floor(B.x / 32), std::floor(B.y / 32)), 0);
	std::vector<std::pair<float, vec2>> vSpots;
	const int bx = (int)(B.x / 32), by = (int)(B.y / 32);
	const int Dy = AllowFrozen ? 20 : 12; // through freeze I can fall further (then wait to thaw)
	for(int y = by - Dy; y <= by + Dy; y++)
		for(int x = bx - 30; x <= bx + 30; x++)
		{
			const float Stand = (y + 1) * 32 - 15;
			for(float X : {x * 32 + 16.0f, x * 32 - 10.0f, x * 32 + 42.0f})
			{
				const vec2 C(X, Stand);
				if(pCol->CheckPoint(C) || Base.InFreeze(C) || pCol->GetCollisionAt(C.x, C.y) == TILE_DEATH || !OnGround(C))
					continue;
				// a foot inside a block: that's against its side, not on it
				if(pCol->CheckPoint(C.x + h, C.y + h - 2) || pCol->CheckPoint(C.x - h, C.y + h - 2))
					continue;
				// through freeze: ground under my middle (a foot against the side of a block isn't a place to land)
				if(AllowFrozen && !pCol->CheckPoint(C.x, C.y + h + 5))
					continue;
				// well below me, it's a fall: it needs open air above it to fall in from (Stronghold's post in the 3-tile gap
				// under the freeze column's wall is right next to the partner lying at the column's bottom, and the search
				// spent all its time on it, from the ledge 15 tiles above)
				if(y > by + 4 && (pCol->CheckPoint(C.x, C.y - 32) || pCol->CheckPoint(C.x, C.y - 64) || pCol->CheckPoint(C.x, C.y - 96)))
					continue;
				if(!AllowFrozen && AirHere.Dist(C) >= 1e5f)
					continue;
				const float D = Field.Dist(C);
				if(D < Mine - 96)
					vSpots.push_back({D, C});
			}
		}
	std::sort(vSpots.begin(), vSpots.end(), [](auto &a, auto &b) { return a.first < b.first; });
	if(AllowFrozen)
	{
		// any landing within 48 px of a spot counts here: one spot per place (a dozen spots a few px apart on the same
		// 3-wide block used up the whole search on the first two)
		std::vector<std::pair<float, vec2>> vDistinct;
		for(const auto &Spot : vSpots)
		{
			bool Near = false;
			for(const auto &Kept : vDistinct)
				Near |= distance(Spot.second, Kept.second) < 40;
			if(!Near)
				vDistinct.push_back(Spot);
		}
		vSpots = vDistinct;
	}
	if(vSpots.size() > 16)
		vSpots.resize(16);
	// a variant: back up for a run-up (0, 4 or 8 tiles), jump right away or near the end of the platform, air-jump
	// 6-33 ticks after or not; through freeze also: steer for a point short of the spot and let the speed carry me
	// (where I enter the freeze I can't steer any more: Stronghold's pillar in the freeze floor is right below a
	// block in the freeze band, and only a drop just beside the block with a little speed left lands on it)
	// and either brake to stop there, or hold on until there and then coast, so the speed carries me on through the
	// freeze (straight down beside that block lands in the freeze floor, on it lands on the block)
	static const float s_aAims[] = {0, 8, 16, 24, 32, 40, 48, 64, 96, 128, 192}; // px short of the spot
	const int NumAims = AllowFrozen ? (int)std::size(s_aAims) : 1, NumCtl = AllowFrozen ? 2 : 1;
	static const float s_aBackups[] = {0, 4, 8};
	static const int s_aAirDelays[] = {6, 9, 12, 15, 18, 21, 24, 27, 30, 33, -1};
	const int NumBackups = std::size(s_aBackups), NumAirDelays = std::size(s_aAirDelays);
	const int NumVariants = NumBackups * 2 * NumAirDelays * NumAims * NumCtl;
	const CNetObj_PlayerInput None = {};
	SSoloPlan Best;
	for(int Spot = 0; Spot < (int)vSpots.size() && PlanClock() < Deadline; Spot++)
	{
		// best spots first: once a jump works, later spots can't do better
		if(Best.m_Valid && vSpots[Spot].first > -Best.m_Score - 32)
			break;
		const vec2 T = vSpots[Spot].second;
		const int Side = T.x > B.x ? 1 : -1;
		for(int Variant = 0; Variant < NumVariants && (Variant % 32 || PlanClock() < Deadline); Variant++)
		{
			// the aim changes fastest (a search cut short by the deadline has tried them all for its first run-ups)
			const float AimX = T.x - Side * s_aAims[Variant % NumAims];
			const bool Coast = (Variant / NumAims) % NumCtl == 1;
			const int V = Variant / (NumAims * NumCtl);
			const int AirDelay = s_aAirDelays[V % NumAirDelays];
			const bool JumpAtEdge = (V / NumAirDelays) % 2 == 0;
			const float Backup = B.x - Side * 32.0f * s_aBackups[V / NumAirDelays / 2];
			SSoloPlan P;
			P.m_TargetX = T.x;
			CHookBotSim S = Base;
			bool BackedUp = absolute(Backup - B.x) < 1, Jumped = false, Bad = false, Landed = false;
			int JumpTick = -1;
			for(int t = 0; t < 180 && !Bad && !Landed; t++)
			{
				const auto &Me = S.m_aTee[0];
				const vec2 Pos = Me.m_Core.m_Pos;
				CNetObj_PlayerInput I = {};
				I.m_TargetX = 1;
				if(!BackedUp)
				{
					// back up, but not into the partner: stop a tee short of it (bumping into it pushed us both off the plan,
					// and I dropped it at a run by the edge); or as far as I get
					const vec2 U = S.m_aTee[1].m_Core.m_Pos;
					const bool NearPartner = absolute(U.y - Pos.y) < 40 && (U.x - Pos.x) * -Side > 0 && absolute(U.x - Pos.x) < 44;
					I.m_Direction = NearPartner ? 0 : SteerTo(Backup, Pos.x, Me.m_Core.m_Vel.x);
					BackedUp = (absolute(Pos.x - Backup) < 8 || NearPartner || (t > 5 && absolute(Me.m_Core.m_Vel.x) < 0.5f)) && absolute(Me.m_Core.m_Vel.x) < 2;
				}
				else if(Coast)
					I.m_Direction = (AimX - Pos.x) * Side > 0 ? Side : 0;
				else
					I.m_Direction = SteerTo(AimX, Pos.x, Me.m_Core.m_Vel.x);
				if(BackedUp && !Jumped && OnGround(Pos))
				{
					// jump now, or a few ticks before running off the platform (not on the last one: a tick late
					// and it's the air jump, which then can't reach)
					bool Go = !JumpAtEdge;
					if(JumpAtEdge)
						Go = !OnGround(Pos + vec2(Me.m_Core.m_Vel.x * 3 + Side * 4, 0));
					if(Go)
					{
						I.m_Jump = 1;
						Jumped = true;
						JumpTick = t;
					}
				}
				if(Jumped && AirDelay >= 0 && t == JumpTick + AirDelay)
					I.m_Jump = 1;
				P.m_vPath.push_back(Pos);
				P.m_vInputs.push_back(I);
				S.Step(I, None, 0);
				Bad = ((Me.m_EnteredFreeze || NearFreeze(S, Pos, Me.m_Core.m_Pos)) && !AllowFrozen) || Me.m_Dead || (t == JumpTick && (Me.m_Core.m_Jumped & 2)); // the first jump must be from the ground
				Bad |= BumpsPartner(S, PartnerKeep);
				Landed = Jumped && t > JumpTick + 3 && OnGround(Me.m_Core.m_Pos) && Me.m_Core.m_Vel.y >= 0 && absolute(Me.m_Core.m_Pos.x - T.x) < (AllowFrozen ? 48 : 12) &&
					 absolute(Me.m_Core.m_Pos.y - T.y) < 16;
			}
			if(Bad || !Landed)
				continue;
			// stand there a moment: still safe (and, if I went through freeze, until I've thawed, outside it)
			for(int t = 0; (t < 10 || S.m_aTee[0].m_FreezeTime > 0) && t < 200 && !Bad; t++)
			{
				CNetObj_PlayerInput Stay = {};
				Stay.m_TargetX = 1;
				P.m_vPath.push_back(S.m_aTee[0].m_Core.m_Pos);
				P.m_vInputs.push_back(Stay);
				S.Step(Stay, None, 0);
				Bad = (S.m_aTee[0].m_EnteredFreeze && !AllowFrozen) || S.m_aTee[0].m_Dead;
			}
			Bad |= S.m_aTee[0].m_FreezeTime > 0 || Base.InFreeze(S.m_aTee[0].m_Core.m_Pos);
			// it has to get me somewhere (standing where I stand is always an option: without this a plan that landed
			// right next to where I started won, and I jumped in place over and over)
			if(Bad || Field.Dist(S.m_aTee[0].m_Core.m_Pos) > Mine - 64)
				continue;
			P.m_Score = -Field.Dist(S.m_aTee[0].m_Core.m_Pos) - 0.5f * (int)P.m_vPath.size();
			P.m_Valid = true;
			if(P.m_Score > Best.m_Score)
				Best = P;
		}
	}
	return Best;
}

CHookBotBrain::SSoloPlan CHookBotBrain::SwingSearch(const CHookBotSim &Base, const CHookBotGoalField &Field, int64_t Deadline, bool Settle, bool Relaxed)
{
	const CCollision *pCol = Base.m_pCollision;
	const float PartnerKeep = std::min(36.0f, distance(Base.m_aTee[0].m_Core.m_Pos, Base.m_aTee[1].m_Core.m_Pos) - 2);
	const float h = CCharacterCore::PhysicalSize() / 2;
	auto OnGround = [&](vec2 P) { return pCol->CheckPoint(P.x + h, P.y + h + 5) || pCol->CheckPoint(P.x - h, P.y + h + 5); };
	struct SNode
	{
		CHookBotSim m_Sim;
		std::vector<CNetObj_PlayerInput> m_vInputs;
		std::vector<vec2> m_vPath;
		float m_Value = 1e9f;
		bool m_Hooking = false; // the last input held the hook (a new hook needs a tick without first)
		bool m_Safe = false; // a plan may end here
		bool m_Thawed = false; // went through freeze and thawed where it landed: the plan ends here
		int m_Parent = -1; // in the level before
		bool m_Expanded = false; // the search tried the moves from here
		int m_Cont = 0; // how many ticks on the search went from here (60 at most; 60 if it didn't try)
	};
	// where a hook aimed from P catches a hookable tile (the first solid tile on the line is a hookable one): one aim per
	// tile it would catch, those nearest the goal first
	auto Targets = [&](vec2 P) {
		struct STarget
		{
			float m_D;
			ivec2 m_Tile;
			vec2 m_Aim;
		};
		std::vector<STarget> vT;
		const int px = (int)(P.x / 32), py = (int)(P.y / 32);
		for(int y = py - 12; y <= py + 12; y++)
			for(int x = px - 12; x <= px + 12; x++)
			{
				const vec2 C(x * 32 + 16, y * 32 + 16);
				if(distance(C, P) > 360 || pCol->GetTile(x * 32 + 16, y * 32 + 16) != TILE_SOLID)
					continue;
				vec2 Hit, Before;
				if(pCol->IntersectLine(P, C, &Hit, &Before) != TILE_SOLID)
					continue;
				const ivec2 Tile((int)(Hit.x / 32), (int)(Hit.y / 32));
				bool Dup = false;
				for(const STarget &T : vT)
					Dup |= T.m_Tile == Tile;
				if(!Dup)
					vT.push_back({Field.Dist(Before), Tile, C});
			}
		std::sort(vT.begin(), vT.end(), [](const STarget &a, const STarget &b) { return a.m_D < b.m_D; });
		std::vector<vec2> vAims;
		for(int i = 0; i < (int)vT.size() && i < 6; i++)
			vAims.push_back(vT[i].m_Aim);
		return vAims;
	};
	const CNetObj_PlayerInput None = {};
	// Ticks of steering Dir (jumping on the first one); Hook: 0 none, 1 a new one at AimAt, 2 keep the one I'm on
	// (steering another way on it: into a swing holding one way, out of it the other); false if it touches freeze or a
	// kill tile
	auto Run = [&](const SNode &From, int Dir, bool Jump, int Hook, vec2 AimAt, int Ticks, SNode &To) {
		To = From;
		CHookBotSim::STee &Me = To.m_Sim.m_aTee[0];
		for(int t = 0; t < Ticks; t++)
		{
			const vec2 Pos = Me.m_Core.m_Pos;
			CNetObj_PlayerInput I = {};
			if(Hook == 2 && !From.m_vInputs.empty())
				I = From.m_vInputs.back();
			I.m_Direction = Dir;
			I.m_Jump = Jump && t == 0;
			if(Hook != 2)
				Aim(I, Hook ? AimAt - Pos : vec2(Dir ? Dir : 1, 0));
			I.m_Hook = Hook == 2 || (Hook == 1 && !(t == 0 && From.m_Hooking));
			To.m_vPath.push_back(Pos);
			To.m_vInputs.push_back(I);
			To.m_Sim.Step(I, None, 0);
			if(Me.m_Dead || BumpsPartner(To.m_Sim, PartnerKeep))
				return false;
			if(Me.m_EnteredFreeze)
			{
				// into freeze: fine only if I then land outside it and thaw there; that ends the plan (Stronghold: a
				// drop down a shaft, drifting over to pass a 1-tile freeze column on its side into an opening behind it)
				CNetObj_PlayerInput Frozen = {};
				Frozen.m_TargetX = 1;
				bool Landed = false;
				for(int f = 0; f < 350 && !Me.m_Dead; f++)
				{
					const vec2 P = Me.m_Core.m_Pos;
					Landed |= OnGround(P) && Me.m_Core.m_Vel.y >= 0 && !To.m_Sim.InFreeze(P);
					if(Landed && Me.m_FreezeTime == 0)
						break;
					To.m_vPath.push_back(P);
					To.m_vInputs.push_back(Frozen);
					To.m_Sim.Step(Frozen, None, 0);
				}
				if(Me.m_Dead || Me.m_FreezeTime > 0 || To.m_Sim.InFreeze(Me.m_Core.m_Pos))
					return false;
				To.m_Hooking = false;
				To.m_Thawed = true;
				To.m_Value = Field.Dist(Me.m_Core.m_Pos) + 0.5f * To.m_vInputs.size();
				return true;
			}
			if(Relaxed ? Me.m_EnteredFreeze : NearFreeze(To.m_Sim, Pos, Me.m_Core.m_Pos))
				return false;
		}
		To.m_Hooking = Hook != 0;
		To.m_Value = Field.Dist(Me.m_Core.m_Pos) + 0.5f * To.m_vInputs.size();
		return true;
	};
	// letting go of the hook here: where the speed carries me (the nearest to the goal I get before I land or touch freeze), and
	// whether it's safe to end a plan here (I land without coming near freeze, or stay in the air 3 seconds). Hanging
	// on a hook doesn't count as safe: it's only safe for now, and plans that ended there threw away the speed a long
	// freeze floor further on needed; scoring by where I end instead of where my speed carries me did too
	// (pvTail, pvTailPath: the inputs and path of a safe way to coast on until I land, if any)
	// (a safe one is judged by its safe ways on only: judged by how close one that ended in the freeze got, swings at the
	// start of Stronghold's swing course went to and fro under a block in a freeze band, each landing back on the ledge
	// where they started "safe", until one touched the band)
	auto Coast = [&](const SNode &N, float *pReach, std::vector<CNetObj_PlayerInput> *pvTail = nullptr, std::vector<vec2> *pvTailPath = nullptr) {
		const float Here = Field.Dist(N.m_Sim.m_aTee[0].m_Core.m_Pos);
		float AnyReach = Here, SafeReach = 1e9f;
		bool Safe = false;
		// with my air jump too, once I start falling, if I still have it (saving it for a long freeze floor is worth
		// something: the best run jumps over the end of one)
		const bool CanAirJump = !(N.m_Sim.m_aTee[0].m_Core.m_Jumped & 2);
		// (2: as late as I can, just before I'd come down: that cancels the whole fall, over a long freeze floor 9 tiles
		// more than at the top of the flight; Stronghold, the room at x 362-412 after the freeze shaft, it fell 3 short)
		for(int UseJump = 0; UseJump <= (CanAirJump ? 2 : 0); UseJump++)
		{
			CHookBotSim Tail = N.m_Sim;
			// still steering the way I was (in the air that keeps my speed; letting go of it, the air friction eats it)
			CNetObj_PlayerInput I = N.m_vInputs.empty() ? CNetObj_PlayerInput{} : N.m_vInputs.back();
			I.m_Jump = 0;
			I.m_Hook = 0;
			bool Jumped = false, Bad = false, Landed = false;
			float Reach = Here;
			std::vector<CNetObj_PlayerInput> vTail;
			std::vector<vec2> vTailPath;
			for(int t = 0; t < 150 && !Bad && !Landed; t++)
			{
				const vec2 Pos = Tail.m_aTee[0].m_Core.m_Pos;
				CNetObj_PlayerInput J = I;
				const vec2 Vel = Tail.m_aTee[0].m_Core.m_Vel;
				const vec2 Soon = Pos + Vel * 2.0f + vec2(0, 18);
				if(UseJump == 1 && !Jumped && Vel.y > 0)
					J.m_Jump = Jumped = true;
				else if(UseJump == 2 && !Jumped && Vel.y > 0 && (Tail.InFreeze(Soon) || Tail.m_pCollision->CheckPoint(Soon)))
					J.m_Jump = Jumped = true;
				vTail.push_back(J);
				vTailPath.push_back(Pos);
				Tail.Step(J, None, 0);
				Bad = Tail.m_aTee[0].m_EnteredFreeze || Tail.m_aTee[0].m_Dead || NearFreeze(Tail, Pos, Tail.m_aTee[0].m_Core.m_Pos);
				if(!Bad)
					Reach = std::min(Reach, Field.Dist(Tail.m_aTee[0].m_Core.m_Pos));
				Landed = !Bad && t > 0 && OnGround(Tail.m_aTee[0].m_Core.m_Pos) && Tail.m_aTee[0].m_Core.m_Vel.y >= 0;
			}
			// landed: brake to a stop there, without running off the edge into freeze (a landing at speed at the end
			// of a floor slid on into the freeze strip after it)
			for(int t = 0; t < 60 && Landed && !Bad && absolute(Tail.m_aTee[0].m_Core.m_Vel.x) > 0.5f; t++)
			{
				const vec2 Pos = Tail.m_aTee[0].m_Core.m_Pos;
				CNetObj_PlayerInput Brake = I;
				Brake.m_Direction = Tail.m_aTee[0].m_Core.m_Vel.x > 0 ? -1 : 1;
				vTail.push_back(Brake);
				vTailPath.push_back(Pos);
				Tail.Step(Brake, None, 0);
				Bad = Tail.m_aTee[0].m_EnteredFreeze || Tail.m_aTee[0].m_Dead || NearFreeze(Tail, Pos, Tail.m_aTee[0].m_Core.m_Pos);
			}
			if(!Bad && Landed && pvTail && pvTail->empty())
			{
				*pvTail = vTail;
				*pvTailPath = vTailPath;
			}
			Safe |= !Bad;
			AnyReach = std::min(AnyReach, Reach);
			// (one that lands: by where it comes to a stop, that's where I'll be; by how close it got on the way, a swing
			// at the start of Stronghold's swing course into a pocket between freeze bands, landing back on the ledge it
			// started from, won over running off that ledge into the course as rank 1 does, and the pocket was a dead end)
			if(!Bad)
				SafeReach = std::min(SafeReach, Landed ? Field.Dist(Tail.m_aTee[0].m_Core.m_Pos) : Reach);
		}
		*pReach = Safe ? SafeReach : AnyReach;
		return Safe;
	};
	std::vector<SNode> vBeam(1);
	vBeam[0].m_Sim = Base;
	vBeam[0].m_Hooking = Base.m_aTee[0].m_PrevInput.m_Hook;
	// it has to get me somewhere (2 tiles closer at least). The best plan that ends safe, even if another gets further
	// (it flew on past a floor it could have landed on, over a 15-tile freeze strip, and fell short); if none does
	// (a long swing chain over freeze, where no safe place is in reach yet), the best of the deepest ones: I plan on
	// from where it ends
	const SNode *pBest = nullptr;
	float BestValue = Field.Dist(Base.m_aTee[0].m_Core.m_Pos) - 64;
	// as deep as the time allows: a safe place to end can be a few swings and long flights away (Stronghold: from the
	// column after the ceiling swings, back left, down through the gap and right to the first solid floor)
	const int MaxDepth = 8;
	std::vector<std::vector<SNode>> vLevels;
	vLevels.reserve(MaxDepth); // pBest points into them
	for(int Depth = 0; Depth < MaxDepth && PlanClock() < Deadline; Depth++)
	{
		std::vector<SNode> vNext;
		for(int bi = 0; bi < (int)vBeam.size(); bi++)
		{
			const SNode &N = vBeam[bi];
			if(PlanClock() >= Deadline)
				break;
			if(N.m_Thawed)
				continue;
			const size_t First = vNext.size();
			const CHookBotSim::STee &Me = N.m_Sim.m_aTee[0];
			const vec2 P = Me.m_Core.m_Pos;
			const bool Grounded = OnGround(P);
			const bool CanJump = Grounded || !(Me.m_Core.m_Jumped & 2);
			SNode To;
			for(int Dir = -1; Dir <= 1; Dir++)
				for(int Jump = 0; Jump <= (CanJump ? 1 : 0); Jump++)
					for(int Ticks : {10, 20})
						if(Run(N, Dir, Jump, 0, vec2(0, 0), Ticks, To))
							vNext.push_back(To);
			for(vec2 AimAt : Targets(P))
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int Jump = 0; Jump <= (Grounded ? 1 : 0); Jump++)
						for(int Hold : {6, 12, 20, 32})
							if(Run(N, Dir, Jump, 1, AimAt, Hold, To))
								vNext.push_back(To);
			if(N.m_Hooking && Me.m_Core.m_HookState == HOOK_GRABBED)
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int Hold : {4, 8, 14})
						if(Run(N, Dir, false, 2, vec2(0, 0), Hold, To))
							vNext.push_back(To);
			for(size_t k = First; k < vNext.size(); k++)
				vNext[k].m_Parent = bi;
			if(!vLevels.empty())
				vLevels.back()[bi].m_Expanded = true;
		}
		if(vNext.empty())
			break;
		std::sort(vNext.begin(), vNext.end(), [](const SNode &a, const SNode &b) { return a.m_Value < b.m_Value; });
		// the best ones, by where their speed carries them; unsafe ones rank lower (they may still lead somewhere safe,
		// the next level sees)
		const int NumChecked = std::min((int)vNext.size(), 40);
		for(int i = 0; i < NumChecked; i++)
		{
			if(vNext[i].m_Thawed)
			{
				vNext[i].m_Safe = true;
				continue;
			}
			float Reach;
			vNext[i].m_Safe = Coast(vNext[i], &Reach);
			vNext[i].m_Value = Reach + 0.5f * vNext[i].m_vInputs.size() + (vNext[i].m_Safe ? 0 : 256);
		}
		vNext.resize(NumChecked);
		// which to keep: having my air jump still is worth something here (the long freeze floor that needs it may only
		// come in sight a few plans later: Stronghold's room at x 362-412 after the freeze shaft, it had used it in the gap
		// before); not in what a plan is worth (that let plans that got nowhere pass)
		auto Keep = [](const SNode &N) { return N.m_Value + ((N.m_Sim.m_aTee[0].m_Core.m_Jumped & 2) ? 96.0f : 0.0f); };
		std::sort(vNext.begin(), vNext.end(), [&](const SNode &a, const SNode &b) { return Keep(a) < Keep(b); });
		// the best few, not all the same: one per end spot (8 px) and speed (2 px/tick)
		std::vector<SNode> vKeep;
		for(SNode &N : vNext)
		{
			if(vKeep.size() >= 8)
				break;
			const CCharacterCore &C = N.m_Sim.m_aTee[0].m_Core;
			bool Same = false;
			for(const SNode &K : vKeep)
			{
				const CCharacterCore &KC = K.m_Sim.m_aTee[0].m_Core;
				Same |= distance(C.m_Pos, KC.m_Pos) < 8 && distance(C.m_Vel, KC.m_Vel) < 2;
			}
			if(!Same)
				vKeep.push_back(std::move(N));
		}
		vLevels.push_back(std::move(vKeep));
		if(getenv("HH_SWINGDBG"))
			for(const SNode &N : vLevels.back())
				printf("swingdbg level %d: value %.0f safe %d len %d ends %.1f %.1f v %.1f %.1f parent %d\n", (int)vLevels.size(), N.m_Value, N.m_Safe, (int)N.m_vInputs.size(),
					N.m_Sim.m_aTee[0].m_Core.m_Pos.x / 32, N.m_Sim.m_aTee[0].m_Core.m_Pos.y / 32, N.m_Sim.m_aTee[0].m_Core.m_Vel.x, N.m_Sim.m_aTee[0].m_Core.m_Vel.y, N.m_Parent);
		for(const SNode &N : vLevels.back())
			if(N.m_Safe && N.m_Value < BestValue)
			{
				BestValue = N.m_Value;
				pBest = &N;
			}
		vBeam = vLevels.back();
	}
	// how far the search went on from each node: a plan that doesn't end safe has to end where I can go on (Stronghold's
	// swing course after the gap: two short swings took me along just above a freeze band, out of reach of anything
	// hookable but the blocks in that band, and the next search found nothing)
	// (one the search never tried the moves from, at the last level or cut off by the time, counts as going on; in ticks,
	// not levels: two more levels could be a few ticks, and in Stronghold's swing course both of us went on at 20 px/tick
	// straight into a freeze column)
	for(std::vector<SNode> &vLevel : vLevels)
		for(SNode &N : vLevel)
			N.m_Cont = N.m_Expanded ? 0 : 60;
	for(int k = (int)vLevels.size() - 1; k > 0; k--)
		for(const SNode &N : vLevels[k])
			if(N.m_Parent >= 0 && N.m_Parent < (int)vLevels[k - 1].size())
			{
				SNode &P = vLevels[k - 1][N.m_Parent];
				P.m_Cont = std::max(P.m_Cont, std::min(60, N.m_Cont + (int)N.m_vInputs.size() - (int)P.m_vInputs.size()));
			}
	if(getenv("HH_SWINGDBG"))
		for(int k = 0; k < (int)vLevels.size(); k++)
			for(const SNode &N : vLevels[k])
				printf("swingdbg cont level %d: value %.0f safe %d expanded %d cont %d parent %d\n", k + 1, N.m_Value, N.m_Safe, N.m_Expanded, N.m_Cont, N.m_Parent);
	if(!pBest)
	{
		// none ends safe: the best of any length (an unsafe one carries 256 extra in m_Value)
		// (and not one that's doomed where it ends: its last input for 3 more ticks has to stay clear of the freeze, else
		// the next search from there finds nothing at all; it ended right under a freeze band, still rising)
		auto Viable = [&](const SNode &N) {
			CHookBotSim S = N.m_Sim;
			const CNetObj_PlayerInput I = N.m_vInputs.empty() ? CNetObj_PlayerInput{} : N.m_vInputs.back();
			for(int t = 0; t < 3; t++)
			{
				const vec2 P = S.m_aTee[0].m_Core.m_Pos;
				S.Step(I, None, 0);
				if(S.m_aTee[0].m_Dead || S.m_aTee[0].m_EnteredFreeze || NearFreeze(S, P, S.m_aTee[0].m_Core.m_Pos))
					return false;
			}
			return true;
		};
		// (where the search went on 40 more ticks from the cut at 30, if anywhere; else 20; else any. Preferring ends with
		// something hookable above me as well got through Stronghold's swing course in 0 of 8 runs, without it 2 of 8)
		// (a safe one doesn't carry the 256: taking it off those too, a safe swing that lost ground won, and at the start of
		// Stronghold's swing course I went to and fro under a block in a freeze band, landing back on the ledge each time)
		const float Start = BestValue;
		auto Unsafe = [](const SNode &N) { return N.m_Value - (N.m_Safe ? 0 : 256); };
		auto ContAfterCut = [](const SNode &N) { return N.m_Cont + std::max(0, (int)N.m_vInputs.size() - 30); };
		for(int MinCont : {40, 20, 0})
		{
			if(pBest)
				break;
			for(const std::vector<SNode> &vLevel : vLevels)
				for(const SNode &N : vLevel)
					if(ContAfterCut(N) >= MinCont && Unsafe(N) < BestValue && (MinCont > 0 || Viable(N)))
					{
						BestValue = Unsafe(N);
						pBest = &N;
					}
		}
		if(!pBest)
			BestValue = Start;
	}
	if(getenv("HH_SWINGDBG"))
		printf("swingdbg start %.0f: chose %s\n", Field.Dist(Base.m_aTee[0].m_Core.m_Pos),
			pBest ? (std::string("value ") + std::to_string((int)pBest->m_Value) + " safe " + std::to_string(pBest->m_Safe) + " cont " + std::to_string(pBest->m_Cont) + " len " + std::to_string(pBest->m_vInputs.size())).c_str() : "none");
	SSoloPlan Plan;
	if(!pBest)
		return Plan;
	Plan.m_Valid = true;
	Plan.m_Swing = true;
	Plan.m_Safe = pBest->m_Safe;
	Plan.m_vInputs = pBest->m_vInputs;
	Plan.m_vPath = pBest->m_vPath;
	// I only follow the start of a plan that doesn't end safe, and plan again from there: what comes later in it was
	// chosen from much further away (a long plan took me past the gap in the freeze without the swing through it)
	if(!pBest->m_Safe && Plan.m_vInputs.size() > 30)
	{
		Plan.m_vRestInputs.assign(Plan.m_vInputs.begin() + 30, Plan.m_vInputs.end());
		Plan.m_vRestPath.assign(Plan.m_vPath.begin() + 30, Plan.m_vPath.end());
		Plan.m_vInputs.resize(30);
		Plan.m_vPath.resize(30);
		Plan.m_Score = -BestValue;
		Plan.m_TargetX = Plan.m_vPath.back().x;
		Plan.m_EndsGrounded = false;
		return Plan;
	}
	Plan.m_Score = -pBest->m_Value;
	Plan.m_TargetX = pBest->m_Sim.m_aTee[0].m_Core.m_Pos.x;
	Plan.m_EndsGrounded = OnGround(pBest->m_Sim.m_aTee[0].m_Core.m_Pos) && pBest->m_Sim.m_aTee[0].m_Core.m_Vel.y >= 0;
	if(Settle && !Plan.m_EndsGrounded)
	{
		// on to where I land, and no further
		float Reach;
		std::vector<CNetObj_PlayerInput> vTail;
		std::vector<vec2> vTailPath;
		Coast(*pBest, &Reach, &vTail, &vTailPath);
		if(!vTail.empty())
		{
			Plan.m_vInputs.insert(Plan.m_vInputs.end(), vTail.begin(), vTail.end());
			Plan.m_vPath.insert(Plan.m_vPath.end(), vTailPath.begin(), vTailPath.end());
			Plan.m_EndsGrounded = true;
		}
	}
	return Plan;
}

int64_t CHookBotBrain::TestDeadline(int Ms) { return PlanClock() + time_freq() * Ms / 1000; }

bool CHookBotBrain::PartnerFarBehind() const
{
	return m_pGoal && m_pU->m_FreezeTime == 0 && m_pGoal->Dist(m_pU->m_Core.m_Pos) > m_pGoal->Dist(m_pB->m_Core.m_Pos) + 320;
}

// down the air field tile by tile from P (the way open air goes to the air goal), or down pField: does it get Tiles above
// P?
bool CHookBotBrain::AirWayClimbs(vec2 P, int Tiles, const CHookBotGoalField *pField) const
{
	const CHookBotGoalField *pF = pField ? pField : m_pGoalAir.get();
	if(!pF || pF->Dist(P) >= 1e5f)
		return false;
	vec2 C = P;
	float D = pF->Dist(C), MinY = C.y;
	for(int k = 0; k < 300 && MinY >= P.y - Tiles * 32; k++)
	{
		vec2 Next = C;
		for(int dy = -1; dy <= 1; dy++)
			for(int dx = -1; dx <= 1; dx++)
			{
				const vec2 N = C + vec2(dx * 32.0f, dy * 32.0f);
				if((dx || dy) && pF->Dist(N) < D)
				{
					D = pF->Dist(N);
					Next = N;
				}
			}
		if(Next == C)
			break;
		C = Next;
		MinY = std::min(MinY, C.y);
	}
	return MinY < P.y - Tiles * 32;
}

bool CHookBotBrain::PartnerStuckForGood() const
{
	CHookBotSim Probe;
	Probe.m_pCollision = m_pCollision;
	return m_pU->m_FreezeTime > 0 && m_pU->m_Grounded && length(m_pU->m_Core.m_Vel) < 1 && Probe.InFreeze(m_pU->m_Core.m_Pos);
}

bool CHookBotBrain::SoloMove(CNetObj_PlayerInput &In, bool AllowFrozen)
{
	const SHookBotTee *pB = m_pB;
	const vec2 B = pB->m_Core.m_Pos;
	const int Now = m_Now;
	// the further goal for a solo part; on the way to rescue the partner (AllowFrozen), the usual one: that's about
	// getting to where the rescue can start, not about speed (with the further one, it went the other way over the
	// freeze band from the platform below Stronghold's ledge, instead of down to the pillar and back to the shaft)
	std::shared_ptr<const CHookBotGoalField> SoloField = m_pGoalSolo && !AllowFrozen ? (m_pOwnSolo ? m_pOwnSolo : m_pGoalSolo) : m_pGoal;
	// on my way to wait for the partner's fall: there
	if(m_CatchGo && m_pCatchField && !AllowFrozen)
		SoloField = m_pCatchField;
	// on my way to the partner stuck for good: towards where it lies
	if(AllowFrozen && PartnerStuckForGood() && !getenv("HH_NOPARTNERFIELD"))
	{
		const vec2 U = m_pU->m_Core.m_Pos;
		if(!m_pPartnerField || distance(U, m_PartnerFieldAt) > 32)
		{
			auto pField = std::make_shared<CHookBotGoalField>();
			pField->Build(m_pCollision, vec2(std::floor(U.x / 32), std::floor(U.y / 32)));
			m_pPartnerField = pField;
			m_PartnerFieldAt = U;
		}
		SoloField = m_pPartnerField;
	}
	// the partner is free but far behind: I don't run off, I land at the first safe place and wait there (Play doesn't
	// start another solo move from the ground then: Stronghold, after the swings along the ceiling and back down through
	// the gap, on the floor before the next freeze strip)
	const bool Settle = PartnerFarBehind() && !m_CatchGo;
	auto Work = [](SSoloJob *pJ) {
		const int64_t Start = PlanClock();
		if(!pJ->m_Air)
			pJ->m_Result = SoloSearch(pJ->m_Base, *pJ->m_pField, Start + time_freq() / (pJ->m_AllowFrozen ? 5 : 10), pJ->m_AllowFrozen);
		// no jump gets me anywhere: with the hook then
		if(!pJ->m_Result.m_Valid)
		{
			pJ->m_Result = SwingSearch(pJ->m_Base, *pJ->m_pField, PlanClock() + time_freq() * (pJ->m_LongSwing ? 2 : 1) / 5, pJ->m_Settle);
			// in the air with nothing found: anything that keeps me out of the freeze, close to it or not (right under a
			// freeze band, still rising, every move came within the usual 4 px at once, and I coasted into it)
			if(!pJ->m_Result.m_Valid && pJ->m_Air)
				pJ->m_Result = SwingSearch(pJ->m_Base, *pJ->m_pField, PlanClock() + time_freq() / 10, pJ->m_Settle, true);
			// from the ground, a swing plan that doesn't end safe comes last: first through freeze, onto a spot
			// outside it where I thaw (Stronghold: a drop down a shaft, drifting over to pass a 1-tile freeze column on
			// its side into an opening behind it; a swing plan hovering on its air jump won, and I fell to the freeze at
			// the bottom)
			if(!pJ->m_Air && !pJ->m_AllowFrozen && !(pJ->m_Result.m_Valid && pJ->m_Result.m_Safe))
			{
				SSoloPlan Frozen = SoloSearch(pJ->m_Base, *pJ->m_pField, PlanClock() + time_freq() / 5, true);
				if(Frozen.m_Valid)
					pJ->m_Result = Frozen;
			}
			// on my way to rescue the partner, only somewhere safe: hopping about with nowhere to land gets me no nearer
			// to a rescue (it jumped up and down on the spot, the route going up from there, and never tried again)
			if(pJ->m_AllowFrozen && pJ->m_Result.m_Valid && !pJ->m_Result.m_Safe)
				pJ->m_Result = {};
			// a plan that doesn't end safe is for a chain of swings, planned on as I go; one without a hook is a jump into
			// nothing (Stronghold, the end of the ledge after the freeze ceiling, nothing hookable in reach: it ran off it
			// into the freeze pool 15 tiles below)
			if(pJ->m_Result.m_Valid && !pJ->m_Result.m_Safe &&
				std::none_of(pJ->m_Result.m_vInputs.begin(), pJ->m_Result.m_vInputs.end(), [](const CNetObj_PlayerInput &I) { return I.m_Hook != 0; }))
				pJ->m_Result = {};
		}
		if(getenv("HH_SOLODBG"))
			printf("solodbg %.1f %.1f air %d frozen %d: valid %d swing %d safe %d len %d ends %.1f %.1f score %.0f from %.0f goal %.0f,%.0f\n", pJ->m_From.x / 32, pJ->m_From.y / 32, pJ->m_Air, pJ->m_AllowFrozen, pJ->m_Result.m_Valid,
				pJ->m_Result.m_Swing, pJ->m_Result.m_Safe, (int)pJ->m_Result.m_vPath.size(), pJ->m_Result.m_vPath.empty() ? 0.0f : pJ->m_Result.m_vPath.back().x / 32,
				pJ->m_Result.m_vPath.empty() ? 0.0f : pJ->m_Result.m_vPath.back().y / 32, pJ->m_Result.m_Score, pJ->m_pField->Dist(pJ->m_Base.m_aTee[0].m_Core.m_Pos), pJ->m_pField->m_GoalTile.x, pJ->m_pField->m_GoalTile.y);
		pJ->m_Done = true;
	};
	// follow a plan: its inputs, as simulated
	if(m_Solo.m_Valid)
	{
		const int k = Now - m_SoloStart;
		const int Size = m_Solo.m_vInputs.size();
		if(k >= 0 && k < Size && distance(B, m_Solo.m_vPath[k]) < 3.0f)
		{
			const CNetObj_PlayerInput &P = m_Solo.m_vInputs[k];
			In.m_Direction = P.m_Direction;
			In.m_Jump = P.m_Jump;
			if(m_Solo.m_Swing)
			{
				In.m_Hook = P.m_Hook;
				In.m_TargetX = P.m_TargetX;
				In.m_TargetY = P.m_TargetY;
				// plan the next swings from where this plan ends, while I fly (on a thread; ready when it ends)
				// (if it ends on the ground, the usual rules decide from there: e.g. wait for the partner)
				if(!m_pSoloJob && Size - k <= 15 && m_pGoal && !m_Solo.m_EndsGrounded)
				{
					m_pSoloJob = std::make_unique<SSoloJob>();
					m_pSoloJob->m_LongSwing = !m_AsyncPlanning;
					SSoloJob &J = *m_pSoloJob;
					InitSim(J.m_Base);
					for(int i = k; i < Size; i++)
						J.m_Base.Step(m_Solo.m_vInputs[i], CNetObj_PlayerInput{}, 0);
					J.m_From = J.m_Base.m_aTee[0].m_Core.m_Pos;
					J.m_pField = SoloField;
					J.m_Settle = Settle;
					J.m_Air = true;
					J.m_StartTick = m_SoloStart + Size;
					if(m_AsyncPlanning)
						J.m_Thread = std::thread(Work, &J);
					else
						Work(&J);
				}
			}
			return true;
		}
		const bool Ended = k >= Size;
		if(getenv("HH_SOLODBG") && !Ended && k >= 0)
			printf("solodbg %d id %d: plan dropped at %d of %d, at %.1f %.1f, planned %.1f %.1f\n", Now, pB->m_Core.m_Id, k, Size, B.x / 32, B.y / 32, m_Solo.m_vPath[k].x / 32, m_Solo.m_vPath[k].y / 32);
		// between swing plans in the air: keep steering the way I was (keeps my speed) until the next one
		if(m_Solo.m_Swing && !m_Solo.m_vInputs.empty())
			m_CoastDir = m_Solo.m_vInputs.back().m_Direction;
		m_Solo = {};
		if(!Ended)
			m_NextSoloSearch = Now + 5;
		// a plan that has me back on the ground: the usual rules decide from here (a new search right away went round in
		// circles: alone, right next to the partner, plans that went nowhere one after another, and the rescue planning
		// and the rule against moves that get nowhere never got their turn)
		else if(pB->m_Grounded)
			return false;
	}
	if(pB->m_Grounded || pB->m_FreezeTime > 0)
		m_CoastDir = 0;
	// collect a search; start one (on a thread) when I stand, and stand still while it runs
	if(m_pSoloJob)
	{
		// (planned from further on in the air: wait for that moment too)
		if(!m_pSoloJob->m_Done || (m_pSoloJob->m_Air && Now < m_pSoloJob->m_StartTick))
		{
			In.m_Direction = m_pSoloJob->m_Air ? m_CoastDir : 0;
			return true;
		}
		if(m_pSoloJob->m_Thread.joinable())
			m_pSoloJob->m_Thread.join();
		SSoloPlan Result = m_pSoloJob->m_Result;
		const vec2 From = m_pSoloJob->m_From;
		const bool Air = m_pSoloJob->m_Air;
		const int StartTick = m_pSoloJob->m_StartTick;
		m_pSoloJob.reset();
		// planned from where I stood, or from where the last swing plan ended (it starts when that one ends)
		if(Result.m_Valid && (Air ? Now >= StartTick : distance(From, B) < 2.0f))
		{
			m_Solo = Result;
			m_SoloStart = Air ? StartTick : Now;
			m_SoloRest = {};
			if(!Result.m_vRestInputs.empty())
			{
				m_SoloRest.m_Valid = m_SoloRest.m_Swing = true;
				m_SoloRest.m_Safe = false;
				m_SoloRest.m_vInputs = Result.m_vRestInputs;
				m_SoloRest.m_vPath = Result.m_vRestPath;
			}
			if(AllowFrozen && !Air)
				m_AloneMoveFrom = B;
			return SoloMove(In, AllowFrozen);
		}
		if(getenv("HH_SOLODBG") && !Result.m_Valid)
			printf("solodbg %d id %d: none found; air %d start %d rest %d (%d ticks) dist %.1f\n", Now, pB->m_Core.m_Id, Air, StartTick, m_SoloRest.m_Valid, (int)m_SoloRest.m_vInputs.size(),
				m_SoloRest.m_Valid ? distance(m_SoloRest.m_vPath[0], B) : -1.0f);
		// none from the end of the last one: on with the rest of the last one, 30 ticks of it at a time
		if(Air && Now == StartTick && m_SoloRest.m_Valid && distance(m_SoloRest.m_vPath[0], B) < 1.0f)
		{
			m_Solo = m_SoloRest;
			m_SoloStart = StartTick;
			m_SoloRest = {};
			if(m_Solo.m_vInputs.size() > 30)
			{
				m_SoloRest = m_Solo;
				m_SoloRest.m_vInputs.erase(m_SoloRest.m_vInputs.begin(), m_SoloRest.m_vInputs.begin() + 30);
				m_SoloRest.m_vPath.erase(m_SoloRest.m_vPath.begin(), m_SoloRest.m_vPath.begin() + 30);
				m_Solo.m_vInputs.resize(30);
				m_Solo.m_vPath.resize(30);
			}
			if(getenv("HH_SOLODBG"))
				printf("solodbg %d id %d: on with the rest of the last plan (%d ticks, %d more)\n", Now, pB->m_Core.m_Id, (int)m_Solo.m_vInputs.size(), (int)m_SoloRest.m_vInputs.size());
			return SoloMove(In, AllowFrozen);
		}
		m_SoloRest = {};
		// nowhere to jump to from here: walk as usual for a while (a while longer through freeze: that search is the
		// costly one)
		m_NextSoloSearch = Now + (Air ? 5 : AllowFrozen ? 50 : 25);
		if(!Air)
			return false;
	}
	// in the air between swing plans, the last search found nothing: keep steering and try again from a bit further on
	if(m_CoastDir && !pB->m_Grounded && pB->m_FreezeTime == 0 && m_pGoal)
	{
		In.m_Direction = m_CoastDir;
		if(Now >= m_NextSoloSearch)
		{
			m_pSoloJob = std::make_unique<SSoloJob>();
			m_pSoloJob->m_LongSwing = !m_AsyncPlanning; // 400 ms inline (found Stronghold's swings after the freeze shaft 3 of 3 times; 200 ms now and then)
			SSoloJob &J = *m_pSoloJob;
			InitSim(J.m_Base);
			CNetObj_PlayerInput Coast = {};
			Coast.m_Direction = m_CoastDir;
			Coast.m_TargetX = 1;
			const int Lead = 10; // ticks: the search takes up to 200 ms on a thread
			for(int i = 0; i < Lead; i++)
				J.m_Base.Step(Coast, CNetObj_PlayerInput{}, 0);
			J.m_From = J.m_Base.m_aTee[0].m_Core.m_Pos;
			J.m_pField = SoloField;
			J.m_Settle = Settle;
			J.m_Air = true;
			J.m_StartTick = Now + Lead;
			if(m_AsyncPlanning)
				J.m_Thread = std::thread(Work, &J);
			else
				Work(&J);
		}
		return true;
	}
	if(!pB->m_Grounded || pB->m_FreezeTime > 0)
		return false;
	if(Now < m_NextSoloSearch)
		return false;
	// the partner a bot too: one of us at a time (both jumped on the same tick, each then saw the other in the air and
	// stopped following its plan, again and again; Stronghold, the room before the gap at x 398): only while it stands
	// still, the one of us nearer the goal on even ticks, the other on every tenth (on odd ones, the one behind went first
	// whenever the first search came on an odd tick: at the start of Stronghold's swing course it had the one in front in
	// its way to the ledge's edge, jumped and swung into a pocket between freeze bands instead)
	// (only near each other: far apart we can't get in each other's way, and Stronghold after the zig-zag's room, one of
	// us swinging on along the hookable blocks never stood still again, and the other stood in its gap for good)
	if(m_PartnerIsBot && !AllowFrozen && distance(B, m_pU->m_Core.m_Pos) < 12 * 32)
	{
		const SHookBotTee *pU = m_pU;
		const float Mine = m_pGoal ? m_pGoal->Dist(B) : 0, Its = m_pGoal ? m_pGoal->Dist(pU->m_Core.m_Pos) : 0;
		const bool First = Mine < Its - 1 || (absolute(Mine - Its) <= 1 && pB->m_Core.m_Id < pU->m_Core.m_Id);
		if(!pU->m_Grounded || length(pU->m_Core.m_Vel) > 0.01f || (First ? Now % 2 != 0 : Now % 10 != 5))
			return false;
	}
	m_pSoloJob = std::make_unique<SSoloJob>();
	m_pSoloJob->m_LongSwing = !m_AsyncPlanning;
	InitSim(m_pSoloJob->m_Base);
	m_pSoloJob->m_From = B;
	m_pSoloJob->m_pField = SoloField;
	m_pSoloJob->m_Settle = Settle;
	m_pSoloJob->m_AllowFrozen = AllowFrozen;
	if(m_AsyncPlanning)
		m_pSoloJob->m_Thread = std::thread(Work, m_pSoloJob.get());
	else
		Work(m_pSoloJob.get());
	In.m_Direction = 0;
	return true;
}

// pseudofly, my half as the hammerer (below): stay under the driver and hammer it up whenever my reload allows,
// at the closest point before we bump into each other (player collision starts at 35 px)
void CHookBotBrain::PseudoHammer(CNetObj_PlayerInput &In)
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	const vec2 B = pB->m_Core.m_Pos, U = pU->m_Core.m_Pos, VB = pB->m_Core.m_Vel, VU = pU->m_Core.m_Vel;
	const float Dist = distance(B, U), NextDist = distance(B + VB, U + VU);
	const bool Ready = pB->m_Reload == 0 && !(m_Fire & 1) && pB->m_FreezeTime == 0;
	const bool Now = NextDist >= Dist || NextDist < 38 || Dist < m_Pseudo.m_FireDist || NextDist >= 60;
	In.m_Fire = Ready && Dist < 60 && B.y - U.y > m_Pseudo.m_MinDy && Now;
	const float Err = U.x - B.x + (VU.x - VB.x) * m_Pseudo.m_Lead;
	In.m_Direction = Err > m_Pseudo.m_Dead ? 1 : Err < -m_Pseudo.m_Dead ? -1 : 0;
	In.m_Hook = 0;
	In.m_Jump = 0;
}

// pseudofly, my half as the driver (above): hook the hammerer while the rope is long, let go when it comes close,
// hook again after the kick, and steer
void CHookBotBrain::PseudoDrive(CNetObj_PlayerInput &In)
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	const vec2 B = pB->m_Core.m_Pos, U = pU->m_Core.m_Pos, VB = pB->m_Core.m_Vel, VU = pU->m_Core.m_Vel;
	const float Dist = distance(B, U), NextDist = distance(B + VB, U + VU);
	const int HookState = pB->m_Core.m_HookState;
	const bool Grabbed = pB->m_Core.HookedPlayer() == pU->m_Core.m_Id && HookState == HOOK_GRABBED;
	if(!m_PfFree && Grabbed && Dist < m_Pseudo.m_ReleaseDist && NextDist < Dist)
	{
		m_PfFree = true;
		m_PfFreeTicks = 0;
	}
	else if(m_PfFree && (++m_PfFreeTicks > m_Pseudo.m_MaxFree || (Dist > m_Pseudo.m_RehookDist && NextDist > Dist)))
		m_PfFree = false;
	const bool Want = !m_PfFree && U.y > B.y + 10 && !pB->m_Grounded && Dist < 360;
	In.m_Hook = HookBotHookInput(Want, m_Input.m_Hook, HookState);
	In.m_Direction = m_DriveDir;
	if(VB.x * m_DriveDir > m_Pseudo.m_MaxVx || (B.x - U.x) * m_DriveDir > m_Pseudo.m_MaxBehind)
		In.m_Direction = 0;
	// start: both standing next to each other -> I jump, the partner hammers me up
	In.m_Jump = 0;
	if(pB->m_Grounded && pU->m_Grounded)
	{
		const float Dx = U.x - B.x;
		if(absolute(Dx) > 80)
			In.m_Direction = Dx > 0 ? 1 : -1;
		else if(!m_Input.m_Jump && absolute(U.y - B.y) < 20)
			In.m_Jump = 1;
		else
			In.m_Direction = 0;
	}
}

void CHookBotBrain::Hookfly(CNetObj_PlayerInput &In)
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	const vec2 B = pB->m_Core.m_Pos, U = pU->m_Core.m_Pos, VB = pB->m_Core.m_Vel;
	const float Dx = U.x - B.x, Dy = U.y - B.y; // Dy > 0: the partner is below
	const int HookState = pB->m_Core.m_HookState;
	const bool UserHooksMe = pU->m_Core.HookedPlayer() == pB->m_Core.m_Id && pU->m_Core.m_HookState == HOOK_GRABBED;
	const bool IHookUser = pB->m_Core.HookedPlayer() == pU->m_Core.m_Id && HookState == HOOK_GRABBED;
	const bool BGround = pB->m_Grounded, UGround = pU->m_Grounded;

	bool Want = false;
	if(IHookUser)
		Want = Dy > -m_Fly.m_Release; // pull until it has passed me (hooking it after that pulls it back down)
	else if(HookState == HOOK_FLYING && m_Input.m_Hook)
		Want = Dy > 0;
	else if(!UserHooksMe && !BGround && Dy > 10 && VB.y > -(UGround ? m_Fly.m_StartWait : m_Fly.m_Wait) && distance(B, U) < 360)
		Want = true; // I am the upper one and my rise is (almost) over: hook it up
	// aled: keep the fly below the freeze until the launch (release early so the partner can't reach it)
	// aled: fly as high as is safe below the freeze until a launch works: let go at the last safe tick, and don't
	// hook again before the partner has passed me
	if(Dy < 0 || BGround || m_Mode != MODE_ALED)
		m_HoldBlocked = false;
	if(m_Mode == MODE_ALED && Want && (m_HoldBlocked || HoldIsRisky()))
	{
		Want = false;
		m_HoldBlocked = true;
	}
	In.m_Hook = HookBotHookInput(Want, m_Input.m_Hook, HookState);
	// aim a bit ahead for the hook (80 px/tick)
	Aim(In, U + pU->m_Core.m_Vel * (distance(B, U) / 80.0f) - B);

	// the hooker moves away from its partner (a longer rope pulls harder), the rider passes beside it
	int Away = B.x >= U.x ? 1 : -1;
	In.m_Direction = Want ? Away * m_Fly.m_HookerDir : 0;
	if(UserHooksMe && absolute(Dx) < m_Fly.m_Dodge)
		In.m_Direction = Away;
	if(absolute(Dx) > m_Fly.m_MaxDx)
		In.m_Direction = Dx > 0 ? 1 : -1;

	// start: both standing next to each other -> I jump
	In.m_Jump = 0;
	if(BGround && UGround)
	{
		if(absolute(Dx) > 80)
			In.m_Direction = Dx > 0 ? 1 : -1;
		else if(!m_Input.m_Jump && absolute(Dy) < 20)
			In.m_Jump = 1;
	}
}

// would hooking the partner one more tick (then letting go) send either of us into the freeze before the next
// half-cycle is over? (the partner is modelled hooking me back as usual)
bool CHookBotBrain::HoldIsRisky()
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	CHookBotSim S;
	InitSim(S);
	if(CeilingAbove(S, pU->m_Core.m_Pos, 16 * 32) < 0)
		return false;
	CNetObj_PlayerInput Hold = m_Input, None = {};
	Hold.m_Hook = 1;
	Hold.m_Fire = 0;
	Hold.m_Jump = 0;
	Aim(Hold, pU->m_Core.m_Pos - pB->m_Core.m_Pos);
	bool UserIn = false;
	for(int t = 0; t < 90; t++)
	{
		S.Step(t == 0 ? Hold : RiderModel(S), PartnerModel(S));
		const auto &Bt = S.m_aTee[0], &Ut = S.m_aTee[1];
		if(Bt.m_EnteredFreeze)
			return true;
		// the partner going cleanly through is what a launch looks like: fine
		if(UserIn && !Ut.m_EnteredFreeze && !S.InFreeze(Ut.m_Core.m_Pos))
			return !FreezeOnLine(S, Bt.m_Core.m_Pos, Ut.m_Core.m_Pos);
		UserIn |= Ut.m_EnteredFreeze;
	}
	return UserIn;
}

bool CHookBotBrain::AirConnected(vec2 A, vec2 B, int Radius) const
{
	const int ax = (int)(A.x / 32), ay = (int)(A.y / 32), bx = (int)(B.x / 32), by = (int)(B.y / 32);
	if(std::abs(bx - ax) > Radius || std::abs(by - ay) > Radius)
		return true; // too far to tell: not known to be cut off (it made me turn back in the middle of a long jump)
	const int N = 2 * Radius + 1;
	std::vector<uint8_t> vSeen((size_t)N * N, 0);
	std::vector<int> vQueue = {Radius * N + Radius};
	vSeen[Radius * N + Radius] = 1;
	CHookBotSim Probe;
	Probe.m_pCollision = m_pCollision;
	for(size_t q = 0; q < vQueue.size(); q++)
	{
		const int lx = vQueue[q] % N, ly = vQueue[q] / N;
		if(lx - Radius + ax == bx && ly - Radius + ay == by)
			return true;
		for(int d = 0; d < 4; d++)
		{
			const int nx = lx + (d == 0) - (d == 1), ny = ly + (d == 2) - (d == 3);
			if(nx < 0 || ny < 0 || nx >= N || ny >= N || vSeen[ny * N + nx])
				continue;
			vSeen[ny * N + nx] = 1;
			const vec2 C((nx - Radius + ax) * 32 + 16, (ny - Radius + ay) * 32 + 16);
			const int Game = m_pCollision->GetCollisionAt(C.x, C.y), Front = m_pCollision->GetFrontCollisionAt(C.x, C.y);
			if(m_pCollision->CheckPoint(C) || Probe.InFreeze(C) || Game == TILE_DEATH || Front == TILE_DEATH)
				continue;
			vQueue.push_back(ny * N + nx);
		}
	}
	return false;
}

bool CHookBotBrain::FreezeBetween(vec2 A, vec2 B) const
{
	CHookBotSim Tmp;
	Tmp.m_pCollision = m_pCollision;
	float Len = distance(A, B);
	for(float d = 0; d <= Len; d += 4)
		if(Tmp.InFreeze(A + (B - A) * (d / std::max(Len, 1.0f))))
			return true;
	return false;
}

// the hammer as in CCharacter::FireWeapon (hit if the target is within 42 px of pos + 21 * aim)
static bool HammerReaches(vec2 From, vec2 To) { return distance(From, To) < 61.5f; }
static void HammerPush(CCharacterCore &Target, vec2 From)
{
	vec2 Dir = distance(From, Target.m_Pos) > 0 ? normalize(Target.m_Pos - From) : vec2(0, -1);
	Target.m_Vel += vec2(0, -1) + normalize(Dir + vec2(0, -1.1f)) * 10.0f;
}

// how far below Pos the freeze starts (px, up to 256; 256 if there is none or something solid comes first)
static float FreezeClearance(const CHookBotSim &S, vec2 Pos)
{
	for(float d = 0; d < 256; d += 4)
	{
		vec2 P = Pos + vec2(0, d);
		if(S.InFreeze(P))
			return d;
		if(S.m_pCollision->CheckPoint(P.x, P.y))
			return 256;
	}
	return 256;
}

static bool FreezeOnLine(const CHookBotSim &S, vec2 A, vec2 B)
{
	float Len = distance(A, B);
	for(float d = 0; d <= Len; d += 4)
		if(S.InFreeze(mix(A, B, d / std::max(Len, 1.0f))))
			return true;
	return false;
}

// what I expect a hookflying partner (tee 1) to do: hook me up once I'm below it and its rise slows down, let go
// once I've passed it (frozen partners do nothing anyway)
static CNetObj_PlayerInput PartnerModel(const CHookBotSim &S)
{
	CNetObj_PlayerInput In = {};
	const auto &U = S.m_aTee[1], &B = S.m_aTee[0];
	if(U.m_FreezeTime > 0)
		return In;
	vec2 D = B.m_Core.m_Pos - U.m_Core.m_Pos;
	In.m_TargetX = round_to_int(D.x);
	In.m_TargetY = round_to_int(D.y);
	if(!In.m_TargetX && !In.m_TargetY)
		In.m_TargetY = -1;
	bool Grabbed = U.m_Core.HookedPlayer() == B.m_Core.m_Id && U.m_Core.m_HookState == HOOK_GRABBED;
	bool Want;
	if(Grabbed)
		Want = D.y > 0;
	else if(U.m_Core.m_HookState == HOOK_FLYING && U.m_PrevInput.m_Hook)
		Want = true;
	else
		Want = B.m_Core.HookedPlayer() != U.m_Core.m_Id && D.y > 10 && U.m_Core.m_Vel.y > -12;
	In.m_Hook = HookBotHookInput(Want, U.m_PrevInput.m_Hook, U.m_Core.m_HookState);
	// hookflyers step away from the partner they are hooking (a longer rope pulls harder)
	if(Want)
		In.m_Direction = U.m_Core.m_Pos.x >= B.m_Core.m_Pos.x ? 1 : -1;
	return In;
}

// me in a lookahead after letting go: no hook, pass beside the partner while it pulls me
static CNetObj_PlayerInput RiderModel(const CHookBotSim &S)
{
	CNetObj_PlayerInput In = {};
	const auto &U = S.m_aTee[1], &B = S.m_aTee[0];
	In.m_TargetX = round_to_int(U.m_Core.m_Pos.x - B.m_Core.m_Pos.x);
	In.m_TargetY = round_to_int(U.m_Core.m_Pos.y - B.m_Core.m_Pos.y);
	if(!In.m_TargetX && !In.m_TargetY)
		In.m_TargetY = -1;
	bool Pulled = U.m_Core.HookedPlayer() == B.m_Core.m_Id && U.m_Core.m_HookState == HOOK_GRABBED;
	if(Pulled && absolute(U.m_Core.m_Pos.x - B.m_Core.m_Pos.x) < 48)
		In.m_Direction = B.m_Core.m_Pos.x >= U.m_Core.m_Pos.x ? 1 : -1;
	return In;
}

// the first freeze row above Pos (within Range px), -1 if none
static float CeilingAbove(const CHookBotSim &S, vec2 Pos, float Range)
{
	for(float d = 0; d < Range; d += 4)
		if(S.InFreeze(Pos - vec2(0, d)))
			return Pos.y - d;
	return -1;
}

bool CHookBotBrain::TryLaunch(CNetObj_PlayerInput &In)
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	if(pB->m_FreezeTime || pU->m_FreezeTime)
		return false;
	CHookBotSim Base;
	InitSim(Base);
	const vec2 B = pB->m_Core.m_Pos, U = pU->m_Core.m_Pos;
	if(CeilingAbove(Base, U, 12 * 32) < 0 || Base.InFreeze(B) || Base.InFreeze(U))
		return false;
	const bool IHookUser = pB->m_Core.HookedPlayer() == pU->m_Core.m_Id && pB->m_Core.m_HookState == HOOK_GRABBED;
	const bool CanHammer = HammerReaches(B, U) && pB->m_Reload == 0 && pU->m_Core.m_Vel.y < 2;
	if(!IHookUser && !CanHammer)
		return false;
	m_Stats.m_LaunchChecks++;
	CNetObj_PlayerInput None = {};
	// launch = this tick's input: let go or keep hooking, with or without a hammer
	for(int Hammer = 1; Hammer >= 0; Hammer--)
		for(int Hold = 0; Hold <= 1; Hold++)
		{
			if((Hammer && !CanHammer) || (!Hammer && !IHookUser) || (Hold && !IHookUser))
				continue;
			CHookBotSim S = Base;
			CNetObj_PlayerInput Launch = {};
			Aim(Launch, U - B);
			Launch.m_Hook = Hold;
			if(Hammer)
			{
				HammerPush(S.m_aTee[1].m_Core, B);
				S.m_aTee[0].m_Reload = 16; // a hammer hit reloads for 320 ms
			}
			S.Step(Launch, PartnerModel(S));
			// quick filter: the partner comes out on the far side of the freeze (me letting go or holding on) and I
			// don't touch it first
			bool Through = false;
			for(int Keep = 0; Keep <= 1 && !Through; Keep++)
			{
				CHookBotSim Q = S;
				CNetObj_PlayerInput Cont = Launch;
				Cont.m_Hook = Keep;
				for(int t = 0; t < 50 && !Through; t++)
				{
					Q.Step(Cont, PartnerModel(Q));
					if(Q.m_aTee[0].m_EnteredFreeze)
						break;
					Through = Q.m_aTee[1].m_FreezeTime > 0 && !Q.m_aTee[1].m_EnteredFreeze && !Q.InFreeze(Q.m_aTee[1].m_Core.m_Pos) && FreezeOnLine(Q, Q.m_aTee[0].m_Core.m_Pos, Q.m_aTee[1].m_Core.m_Pos);
				}
			}
			if(!Through || PlanClock() > m_PlanDeadline)
				continue;
			m_Stats.m_Through++;
			SPlan Plan = PlanDive(S, Hold);
			if(!Plan.m_Valid)
				continue;
			// go: this tick is the launch, the dive plan starts on the next one
			In = Launch;
			In.m_WantedWeapon = WEAPON_HAMMER + 1;
			In.m_Fire = Hammer;
			m_Plan = Plan;
			m_PlanStart = m_Now + 1;
			m_NextPlan = m_PlanStart;
			m_Stats.m_Launches++;
			m_Stats.m_HammerLaunches += Hammer;
			return true;
		}
	(void)None;
	return false;
}

void CHookBotBrain::Aled(CNetObj_PlayerInput &In)
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	CHookBotSim Probe;
	Probe.m_pCollision = m_pCollision;
	const vec2 B = pB->m_Core.m_Pos, U = pU->m_Core.m_Pos;
	const int Now = m_Now;

	if(m_Phase == 0)
	{
		Hookfly(In);
		if(TryLaunch(In))
		{
			m_Phase = 1;
			return;
		}
		// the partner is frozen above the freeze anyway (e.g. it got there by itself): try to dive
		if(pU->m_FreezeTime > 0 && pB->m_FreezeTime == 0 && !Probe.InFreeze(U) && FreezeBetween(B, U))
		{
			m_Phase = 1;
			m_Plan = {};
			m_NextPlan = Now;
		}
		else
			return;
	}
	if(m_Phase == 1)
	{
		const int t = Now - m_PlanStart; // plan tick whose input this is
		In.m_Direction = 0;
		In.m_Jump = 0;
		In.m_Hook = 0;
		Aim(In, U - B);
		if(pB->m_FreezeTime > 0 || (pU->m_FreezeTime == 0 && t > 3 && !m_Plan.m_Valid))
		{
			m_Phase = 0;
			m_Plan = {};
			m_Stats.m_Aborts++;
			m_Say("aled failed, let's fly again");
			return;
		}
		// the hammer runs before my own freeze check: the first tick my last move touched freeze is the moment
		if(Probe.PathTouchesFreeze(pB->m_PrevPos, B))
		{
			In.m_Fire = 1;
			m_Stats.m_Fired++;
			m_Phase = 2;
			m_FiredTick = Now;
			return;
		}
		bool OnTrack = m_Plan.m_Valid && t < (int)m_Plan.m_vPath.size() && distance(B, m_Plan.m_vPath[t]) < 2.0f;
		if(!OnTrack && Now >= m_NextPlan)
		{
			CHookBotSim Base;
			InitSim(Base);
			m_Plan = PlanDive(Base, m_Input.m_Hook);
			m_Stats.m_Replans++;
			m_Stats.m_ReplanFails += !m_Plan.m_Valid;
			m_PlanStart = Now;
			m_NextPlan = Now + 4;
		}
		const int k = Now - m_PlanStart;
		if(m_Plan.m_Valid)
		{
			In.m_Hook = HookBotHookInput(m_Plan.m_HookAt >= 0 && k >= m_Plan.m_HookAt, m_Input.m_Hook, pB->m_Core.m_HookState);
			In.m_Jump = k == m_Plan.m_JumpAt;
			In.m_Direction = m_Plan.m_Dir;
		}
		return;
	}
	// after the hammer
	Aim(In, U - B);
	if(Now == m_FiredTick + 2)
	{
		if(pU->m_FreezeTime == 0)
			m_Say("aled! hook me up through and hammer me if you can");
		else
			m_Say("missed the aled, sorry");
	}
	if(Now > m_FiredTick + 2 && pB->m_FreezeTime == 0)
	{
		m_Mode = MODE_HOOKFLY;
		m_Phase = 0;
	}
}

// hammerhit: when the partner is frozen (or about to be) and I'm free, follow a rescue plan: hook it out of the
// freeze and hammer it once it is clear. Otherwise walk in m_DriveDir on the ground and wait in the air.
void CHookBotBrain::Hammerhit(CNetObj_PlayerInput &In, bool ForceRescue)
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	const vec2 B = pB->m_Core.m_Pos, U = pU->m_Core.m_Pos;
	const int Now = m_Now;
	In.m_Direction = 0;
	In.m_Jump = 0;
	In.m_Hook = 0;
	Aim(In, U - B);
	CollectJob();
	// one on the other's head over a 1-tile freeze: the frozen one holds hammer, the one on top hammers it. The
	// hammer unfreezes it for a moment, and a tee that was just unfrozen swings a held hammer at once
	// (CCharacter::m_FrozenLastTick), which kicks the one on top up; then a new hammerhit starts. My half when
	// frozen is here; the one on top is a rescue plan option (SPlan::m_PreHammer)
	auto OnHeadOf = [](const SHookBotTee *pTop, const SHookBotTee *pBottom) {
		const vec2 d = pTop->m_Core.m_Pos - pBottom->m_Core.m_Pos;
		return d.y < -20 && d.y > -44 && absolute(d.x) < 28;
	};
	// frozen: hold the way a hammer should take me (ignored until one unfreezes me for a tick)
	if(pB->m_FreezeTime > 0)
		In.m_Direction = FrozenHoldDir();
	else
		m_FrozenDirSaid = 0;
	if(pB->m_FreezeTime > 0 && OnHeadOf(pU, pB))
	{
		m_pWhy = "frozen: hold hammer";
		In.m_Fire = 1; // held, aimed up at the partner
		return;
	}
	// the partner needs me if it is frozen, or will be soon if it does nothing (so I start planning while I'm still
	// in a good spot)
	bool PartnerStuck = pU->m_FreezeTime > 0 || ForceRescue;
	{
		// frozen, but lying still outside the freeze: it thaws by itself, so wait (don't drag it anywhere)
		CHookBotSim Probe;
		Probe.m_pCollision = m_pCollision;
		if(!ForceRescue && pU->m_FreezeTime > 0 && pU->m_Grounded && length(pU->m_Core.m_Vel) < 1 && !Probe.InFreeze(U) &&
			!Probe.PathTouchesFreeze(pU->m_PrevPos, U))
			PartnerStuck = false;
		// frozen in the air, on its way to land outside the freeze without touching any (just hammered there): it thaws
		// by itself too (a new rescue hooked it back from there, and my second hammer dropped me into the freeze)
		// (not in the middle of a rescue plan: it knows where it takes the partner and me; dropping it there let go of the
		// hook and left me drifting into the freeze column next to me)
		// (a replanned one waiting for its start too: dropped in that gap, I let go in the middle of a drag over Stronghold's
		// freeze pool and fell in)
		const bool PlanRunning = m_Plan.m_Valid;
		// (from inside the freeze too, flying out of it: it's frozen anyway, where it comes to rest is what counts)
		if(!ForceRescue && PartnerStuck && !PlanRunning && pU->m_FreezeTime > 0 && !pU->m_Grounded)
		{
			CHookBotSim S;
			InitSim(S);
			const CNetObj_PlayerInput None = {};
			const float h = CCharacterCore::PhysicalSize() / 2;
			bool Dead = false, Landed = false;
			for(int t = 0; t < 120 && !Dead && !Landed; t++)
			{
				S.Step(None, None, 1);
				const auto &P = S.m_aTee[1];
				Dead = P.m_Dead;
				Landed = P.m_Core.m_Vel.y >= 0 && (S.m_pCollision->CheckPoint(P.m_Core.m_Pos.x + h, P.m_Core.m_Pos.y + h + 5) || S.m_pCollision->CheckPoint(P.m_Core.m_Pos.x - h, P.m_Core.m_Pos.y + h + 5));
			}
			if(Landed && !Dead && !S.InFreeze(S.m_aTee[1].m_Core.m_Pos))
				PartnerStuck = false;
		}
	}
	// a rescue by the hook alone under way is seen through to its end (letting go, and the fall after it that the plan
	// checked): the partner dragged out of the freeze counts as thawing by itself above, and dropping the plan there let
	// go of it early and let me fall into the freeze at the bottom of the pit below
	if(m_Plan.m_Valid && m_Plan.m_HookOnly && pB->m_FreezeTime == 0 && Now - m_PlanStart >= 0)
		PartnerStuck = true;
	if(!PartnerStuck && pB->m_FreezeTime == 0)
	{
		CHookBotSim S;
		InitSim(S);
		const CNetObj_PlayerInput None = {};
		PartnerStuck = S.m_aTee[1].m_EnteredFreeze;
		for(int t = 0; t < 20 && !PartnerStuck; t++)
		{
			S.Step(None, None);
			PartnerStuck = S.m_aTee[1].m_EnteredFreeze;
		}
	}
	if(pB->m_FreezeTime > 0 || !PartnerStuck)
	{
		m_NoRescueSince = -1;
		// a plan dropped with me in the air (it was hooking the partner, say): a way down that keeps me out of the freeze
		// (let go without, the hook's pull had me drift into the freeze column next to me)
		if(m_Plan.m_Valid && pB->m_FreezeTime == 0 && !pB->m_Grounded && Now >= m_PostEnd)
		{
			m_PostDir = SafeFallDir(&m_PostJump);
			m_PostEnd = Now + 60;
		}
		if(m_Plan.m_Valid || !m_vBridge.empty())
			m_RescueEpoch++;
		m_Plan = {};
		m_vBridge.clear();
		m_pWhy = pB->m_FreezeTime > 0 ? "frozen" : "hh: nothing";
		if(pB->m_FreezeTime == 0 && Now < m_PostEnd && !pB->m_Grounded)
		{
			m_pWhy = "hh: post fall";
			In.m_Direction = m_PostDir; // after my hammer: the fall the plan checked (until I land)
			// (and the air jump it checked)
			if(m_PostJump && !(pB->m_Core.m_Jumped & 2) && (m_PostJump == 1 || pB->m_Core.m_Vel.y > 0) && !m_Input.m_Jump)
			{
				In.m_Jump = 1;
				m_PostJump = 0;
			}
		}
		else if(pB->m_FreezeTime == 0 && pB->m_Grounded)
		{
			m_pWhy = "hh: walk";
			In.m_Direction = m_DriveDir;
			if(m_pGoal)
			{
				// walk where half a second of walking gets me closer to the goal. In this order: a move that gets closer
				// without freezing; else a drop into freeze where the way goes on from there (Simple Down's first drop,
				// the start of a hammerhit: standing at the ledge forever isn't progress either, both of us did); else
				// stay. Never onto a kill tile, and never into freeze that throws away what we had (Stronghold's
				// ceiling gap: 50 tiles down, ~1600 px further from the goal): a drop has to land closer to the goal (where
				// it hits the freeze, measured just above it) than where I stand, i.e. the way goes on down there
				const float Here = m_pGoal->Dist(B);
				float BestMove = Here - 32, BestDrop = 1e9f;
				int MoveDir = 2, DropDir = 2;
				for(int Dir : {-1, 1})
				{
					CHookBotSim S;
					InitSim(S);
					CNetObj_PlayerInput Walk = {}, None = {};
					Walk.m_Direction = Dir;
					Walk.m_TargetX = 1;
					bool Dead = false, Frozen = false;
					vec2 After = B, Clear = B;
					// half a second of walking, then what happens after it (17 tiles down takes most of a second,
					// 50 tiles almost two)
					const float h = CCharacterCore::PhysicalSize() / 2;
					bool Airborne = false;
					for(int t = 0; t < 125 && !Dead && !Frozen; t++)
					{
						// walk for half a second; once off the ground, let go (as I do live: I only steer on the ground)
						const vec2 P = S.m_aTee[0].m_Core.m_Pos;
						Airborne |= t > 0 && !(S.m_pCollision->CheckPoint(P.x + h, P.y + h + 5) || S.m_pCollision->CheckPoint(P.x - h, P.y + h + 5));
						S.Step(t < 25 && !Airborne ? Walk : None, None);
						Dead = S.m_aTee[0].m_Dead;
						Frozen = S.m_aTee[0].m_EnteredFreeze;
						if(!Frozen)
							Clear = S.m_aTee[0].m_Core.m_Pos;
						if(t == 24)
							After = S.m_aTee[0].m_Core.m_Pos;
					}
					if(Dead)
						continue;
					if(!Frozen && m_pGoal->Dist(After) < BestMove)
					{
						BestMove = m_pGoal->Dist(After);
						MoveDir = Dir;
					}
					// where it hits the freeze, measured just before it; a real fall (at least 2 tiles down), not walking
					// into a freeze wall next to me
					// allowed when it lands closer to the goal; or not much further (400 px) when the partner isn't behind
					// me (we're both stuck at the same ledge, someone has to go first; if it's behind, I wait for it)
					// never while it's far behind: frozen I'm no use to it when it gets here (I wait free)
					// (but not when we've both stood here for 2 s with nothing else to do: on the corners of a post in
					// Stronghold's big room, after a fall-catch, both waited for good; the one ahead drops into the freeze
					// floor, and the hammerhit goes on from there)
					const bool BothIdle = m_BothIdleSince >= 0 && m_Now - m_BothIdleSince > 2 * SERVER_TICK_SPEED;
					const bool PartnerBehind = m_pGoal->Dist(U) > Here + 96 && !BothIdle;
					bool DropOk = !PartnerFarBehind() && (m_pGoal->Dist(Clear) < Here - 16 || (!PartnerBehind && m_pGoal->Dist(Clear) < Here + 400));
					// following a route: only into freeze the route goes near (Stronghold: off a ledge onto a freeze floor 16
					// tiles below, where the route jumps over a 24-tile gap high above it)
					if(DropOk && !m_vRoute.empty() && m_RouteIndex >= 0)
					{
						int Seg;
						DropOk = PathDist(m_vRoute, m_RouteIndex - 2, m_RouteIndex + 6, Clear, &Seg) < 8 * 32;
					}
					// with a bot partner, only where it can get me out if I come to rest in the freeze (off the platform beside
					// Stronghold's freeze shaft into the pool: nowhere to get me out from, while the way on was the shaft)
					if(Frozen && DropOk && m_PartnerIsBot)
					{
						for(int t = 0; t < 150 && !AtRestSim(S); t++)
							S.Step(None, None);
						const vec2 E = S.m_aTee[0].m_Core.m_Pos;
						DropOk = !S.m_aTee[0].m_Dead && (!S.InFreeze(E) || CanRescue(S, U, E));
					}
					if(Frozen && Clear.y > B.y + 64 && DropOk && m_pGoal->Dist(Clear) < BestDrop)
					{
						BestDrop = m_pGoal->Dist(Clear);
						DropDir = Dir;
					}
				}
				In.m_Direction = MoveDir != 2 ? MoveDir : DropDir != 2 ? DropDir : 0;
				m_pWhy = MoveDir != 2 ? "hh: walk" : DropDir != 2 ? "hh: drop" : "hh: stand";
			}
		}
		return;
	}
	// the simple way first, before any plan: standing on the ground outside the freeze, with the partner frozen,
	// hammer it where it rests outside the freeze (checked: where it comes to rest, 8 px clear of freeze on both sides)
	// if it's in reach now; if it's still falling and will come to rest in reach, wait for it (Stronghold, a 3-wide edge
	// by a freeze column, the partner landing frozen at the column's foot: one hammer from standing, and wait. The plans
	// kept hopping off the edge instead, and hooking it back into the column)
	if(PartnerStuck && !ForceRescue && pB->m_FreezeTime == 0 && pB->m_Grounded && pU->m_FreezeTime > 0)
	{
		CHookBotSim Probe;
		Probe.m_pCollision = m_pCollision;
		const float h = CCharacterCore::PhysicalSize() / 2;
		auto RestsSafe = [&](CHookBotSim &S, vec2 *pRest) {
			const CNetObj_PlayerInput None = {};
			for(int t = 0; t < 90; t++)
			{
				S.Step(None, None, 1);
				const auto &P = S.m_aTee[1];
				if(P.m_Dead)
					return false;
				const vec2 Q = P.m_Core.m_Pos;
				*pRest = Q;
				if(length(P.m_Core.m_Vel) < 0.6f && (S.m_pCollision->CheckPoint(Q.x + h, Q.y + h + 5) || S.m_pCollision->CheckPoint(Q.x - h, Q.y + h + 5)))
					return !S.InFreeze(Q) && !S.InFreeze(Q + vec2(8, 0)) && !S.InFreeze(Q - vec2(8, 0));
			}
			return false;
		};
		if(!Probe.InFreeze(B) && pB->m_Reload == 0 && HammerReaches(B, U))
		{
			CHookBotSim S;
			InitSim(S);
			HammerPush(S.m_aTee[1].m_Core, B);
			S.m_aTee[1].m_FreezeTime = 0;
			vec2 Rest;
			if(RestsSafe(S, &Rest))
			{
				if(m_Plan.m_Valid || !m_vBridge.empty())
					m_RescueEpoch++;
				m_Plan = {};
				m_vBridge.clear();
				In.m_Fire = 1;
				m_pWhy = "hh: direct hammer";
				return;
			}
		}
		if(!Probe.InFreeze(B) && !pU->m_Grounded)
		{
			CHookBotSim S;
			InitSim(S);
			vec2 Rest;
			const CNetObj_PlayerInput None = {};
			bool Settles = false;
			for(int t = 0; t < 90 && !Settles; t++)
			{
				S.Step(None, None, 1);
				Rest = S.m_aTee[1].m_Core.m_Pos;
				Settles = length(S.m_aTee[1].m_Core.m_Vel) < 0.6f && (S.m_pCollision->CheckPoint(Rest.x + h, Rest.y + h + 5) || S.m_pCollision->CheckPoint(Rest.x - h, Rest.y + h + 5));
			}
			if(Settles && HammerReaches(B, Rest))
			{
				// (standing still, no hook, whatever plan I had: it lands in reach, then the hammer above)
				if(m_Plan.m_Valid || !m_vBridge.empty())
					m_RescueEpoch++;
				m_Plan = {};
				m_vBridge.clear();
				m_pWhy = "hh: wait for it to land";
				return;
			}
		}
	}
	const int k = Now - m_PlanStart; // plan tick whose input this is
	const bool OnTrack = m_Plan.m_Valid && k >= 0 && k < (int)m_Plan.m_vPath.size() && distance(B, m_Plan.m_vPath[k]) < 2.0f &&
			     distance(U, m_Plan.m_vUserPath[k]) < 2.0f;
	if(m_DebugFly && m_Plan.m_Valid && k >= 0 && k < (int)m_Plan.m_vPath.size() && !OnTrack)
		printf("offdbg %d id %d: k %d of %d me %.2f %.2f planned %.2f %.2f partner %.2f %.2f planned %.2f %.2f\n", Now, pB->m_Core.m_Id, k, (int)m_Plan.m_vPath.size(), B.x, B.y,
			m_Plan.m_vPath[k].x, m_Plan.m_vPath[k].y, U.x, U.y, m_Plan.m_vUserPath[k].x, m_Plan.m_vUserPath[k].y);
	const bool Waiting = m_Plan.m_Valid && k < 0; // the plan starts after the bridge
	// (a plan whose hook timing is precise, from a jump: no improving it once its hook is about to fire; the improvements
	// changed when the hook fired, the hook in flight missed, and the plans after it no longer matched what happened)
	const bool Precise = m_Plan.m_Valid && (m_Plan.m_HookOnly || m_Plan.m_AirJumpAt >= 0 || m_Plan.m_DirAfter != 2) && k >= m_Plan.m_HookAt - 1;
	if(!m_pJob && !Waiting && (!OnTrack || (m_Plan.m_FireAt - k > m_RescueLead + 2 && !Precise)) && (m_Plan.m_Valid || Now >= m_NextRescueJob))
		StartJob(*pB, *pU); // plans ahead from the state after the bridge (which starts now)
	// the partner isn't frozen yet (only about to be): no leaving the ground for it yet, it may still land where I can
	// hammer it from where I stand (I jumped off a 3-wide edge by a freeze column for a partner flying in, and fell)
	// (not when it's cut off and I'm to pull it through: it's free and meant to stay so, and I never helped it up)
	const bool Premature = !ForceRescue && m_Plan.m_Valid && !m_Plan.m_Grounded && pU->m_FreezeTime == 0 && pB->m_Grounded;
	m_pWhy = "hh: planning";
	if(m_Plan.m_Valid && k >= 0 && OnTrack && !Premature)
	{
		m_pWhy = m_Plan.m_HookOnly ? "hh: plan (hook only)" : m_Plan.m_Aled ? "hh: plan (aled)" : "hh: plan";
		const CNetObj_PlayerInput P = RescueInput(m_Plan, k, m_Input.m_Hook, pB->m_Core.m_HookState, U - B, B, pB->m_Core.m_Vel);
		In.m_Direction = P.m_Direction;
		In.m_Jump = P.m_Jump;
		In.m_Hook = P.m_Hook;
		// and its aim (it may aim the hook above the partner: aimed at its middle, the hook hit a corner the plan missed)
		In.m_TargetX = P.m_TargetX;
		In.m_TargetY = P.m_TargetY;
		if(k == 0 && m_Plan.m_PreHammer)
			In.m_Fire = 1;
		if(k == m_Plan.m_FireAt)
		{
			if(m_DebugFly)
				printf("firedbg %d id %d: k %d dist %.1f reload %d planned me %.1f %.1f partner %.1f %.1f, now me %.1f %.1f partner %.1f %.1f hookonly %d post %d\n", m_Now, pB->m_Core.m_Id, k, distance(B, U),
					pB->m_Reload, m_Plan.m_vPath[k].x, m_Plan.m_vPath[k].y, m_Plan.m_vUserPath[k].x, m_Plan.m_vUserPath[k].y, B.x, B.y, U.x, U.y, m_Plan.m_HookOnly, m_Plan.m_PostDir);
			// the hammer, or (a rescue by the hook alone) just letting go
			In.m_Fire = !m_Plan.m_HookOnly;
			In.m_Hook = 0;
			In.m_Direction = m_Plan.m_PostDir;
			m_PostDir = m_Plan.m_PostDir;
			m_PostJump = m_Plan.m_PostJump;
			m_PostEnd = Now + 60;
			m_RescueStats.m_Fired++;
			m_Plan = {};
			m_vBridge.clear();
			m_RescueEpoch++;
		}
		return;
	}
	if(Premature)
	{
		m_pWhy = "hh: plan waits (partner not frozen yet)";
		return; // (not its bridge either: that's the plan's jump)
	}
	if(Now >= m_BridgeStart && Now < m_BridgeStart + (int)m_vBridge.size())
	{
		const CNetObj_PlayerInput &P = m_vBridge[Now - m_BridgeStart];
		m_pWhy = "hh: bridge";
		In.m_Direction = P.m_Direction;
		In.m_Jump = P.m_Jump;
		In.m_Hook = HookBotHookInput(P.m_Hook, m_Input.m_Hook, pB->m_Core.m_HookState);
		In.m_TargetX = P.m_TargetX;
		In.m_TargetY = P.m_TargetY;
		return;
	}
}

int CHookBotBrain::FrozenHoldDir()
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	const vec2 B = pB->m_Core.m_Pos, U = pU->m_Core.m_Pos;
	// only while lying still in the freeze (frozen in the air, the partner hooks me rather than hammers me; lying
	// outside it I thaw by myself), and the partner free to hammer me
	CHookBotSim Probe;
	Probe.m_pCollision = m_pCollision;
	if(!m_pGoal || !pB->m_Grounded || length(pB->m_Core.m_Vel) > 1 || !Probe.InFreeze(B) || pU->m_FreezeTime > 0)
	{
		m_FrozenDir = 0;
		return 0;
	}
	if(m_Now < m_NextFrozenDirCheck)
		return m_FrozenDir;
	m_NextFrozenDirCheck = m_Now + 10;
	// where the partner can hammer me from: where it is if it's in reach, else any free spot in reach around me
	std::vector<vec2> vFrom;
	if(pU->m_FreezeTime == 0 && HammerReaches(U, B))
		vFrom.push_back(U);
	else
		for(float r : {40.0f, 56.0f})
			for(int a = 0; a < 12 && vFrom.size() < 12; a++)
			{
				const vec2 P = B + direction(a * 2 * pi / 12) * r;
				if(!m_pCollision->TestBox(P, vec2(28, 28)) && !Probe.InFreeze(P))
					vFrom.push_back(P);
			}
	int Best = 0;
	if(!vFrom.empty())
	{
		// the hammer, then a tick with my input, then the freeze has me again: where do I end up, on average over
		// where the hammer can come from
		float aScore[3];
		for(int d = -1; d <= 1; d++)
		{
			float Sum = 0;
			for(vec2 From : vFrom)
			{
				CHookBotSim S;
				InitSim(S);
				CHookBotSim::STee &T = S.m_aTee[0];
				T.m_FreezeTime = 0;
				HammerPush(T.m_Core, From);
				CNetObj_PlayerInput In = {}, None = {};
				In.m_Direction = d;
				In.m_TargetX = 1;
				for(int t = 0; t < 100 && !T.m_Dead; t++)
					S.Step(In, None, 0);
				Sum += T.m_Dead ? 1e5f : m_pGoal->Dist(T.m_Core.m_Pos);
			}
			aScore[d + 1] = Sum / vFrom.size();
		}
		// a clear difference only (holding nothing is the default)
		for(int d : {-1, 1})
			if(aScore[d + 1] < aScore[1] - 32 && aScore[d + 1] < aScore[Best + 1])
				Best = d;
	}
	m_FrozenDir = Best;
	if(Best != 0 && Best != m_FrozenDirSaid)
	{
		m_Say(Best > 0 ? "hammer me, I'm holding right" : "hammer me, I'm holding left");
		m_FrozenDirSaid = Best;
	}
	return Best;
}

int CHookBotBrain::SafeFallDir(int *pJump) const
{
	const float h = CCharacterCore::PhysicalSize() / 2;
	if(pJump)
		*pJump = 0;
	// (my air jump too, if I have it: dropped in the middle of a drag over Stronghold's freeze pool, only the air jump
	// reached the next block)
	const bool CanJump = !(m_pB->m_Core.m_Jumped & 2);
	for(int Jump = 0; Jump <= (CanJump ? 2 : 0); Jump++)
		for(int Dir : {0, -1, 1})
		{
			CHookBotSim S;
			InitSim(S);
			CNetObj_PlayerInput Steer = {}, None = {};
			Steer.m_Direction = Dir;
			Steer.m_TargetX = 1;
			bool Bad = false, Landed = false, Jumped = false;
			for(int t = 0; t < 200 && !Bad && !Landed; t++)
			{
				CNetObj_PlayerInput I = Steer;
				if(!Jumped && ((Jump == 1 && t == 0) || (Jump == 2 && S.m_aTee[0].m_Core.m_Vel.y > 0)))
					I.m_Jump = Jumped = true;
				S.Step(I, None, 0);
				const auto &Me = S.m_aTee[0];
				Bad = Me.m_EnteredFreeze || Me.m_Dead;
				Landed = Me.m_Core.m_Vel.y >= 0 && (S.m_pCollision->CheckPoint(Me.m_Core.m_Pos.x + h, Me.m_Core.m_Pos.y + h + 5) || S.m_pCollision->CheckPoint(Me.m_Core.m_Pos.x - h, Me.m_Core.m_Pos.y + h + 5));
				if(Landed)
					Steer.m_Direction = 0;
			}
			if(!Bad)
			{
				if(pJump)
					*pJump = Jump;
				return Dir;
			}
		}
	return 0;
}

// steer to x = Target and hold there (a little lead for the speed)
static int SteerTo(float Target, float X, float Vx)
{
	const float Err = Target - (X + Vx * 3);
	return Err > 4 ? 1 : Err < -4 ? -1 : 0;
}

CNetObj_PlayerInput CHookBotBrain::RescueInput(const SPlan &P, int t, int PrevHook, int HookState, vec2 AimAt, vec2 Pos, vec2 Vel)
{
	CNetObj_PlayerInput In = {};
	Aim(In, AimAt + vec2(0, P.m_AimDy));
	const bool Want = ((P.m_HookAt >= 0 && t >= P.m_HookAt && t < P.m_HookEnd) || (P.m_Hook2At >= 0 && t >= P.m_Hook2At)) && t < P.m_FireAt;
	In.m_Hook = HookBotHookInput(Want, PrevHook, HookState);
	In.m_Jump = t == P.m_JumpAt || t == P.m_AirJumpAt;
	In.m_Direction = P.m_DirAfter != 2 && t >= P.m_HookAt ? P.m_DirAfter : P.m_TargetX >= 0 ? SteerTo(P.m_TargetX, Pos.x, Vel.x) : P.m_Dir;
	return In;
}

void CHookBotBrain::CollectJob()
{
	if(!m_pJob || !m_pJob->m_Done)
		return;
	// (only this job's thread: joining the others too made the game wait for a solo search or a goal field build)
	if(m_pJob->m_Thread.joinable())
		m_pJob->m_Thread.join();
	SRescueJob &J = *m_pJob;
	m_LastPlanUs = J.m_Us;
	m_HookCursor = J.m_HookCursor;
	if(J.m_Epoch == m_RescueEpoch)
	{
		m_RescueCursor = J.m_Cursor;
		if(J.m_Fresh)
		{
			m_RescueStats.m_Plans++;
			m_RescueStats.m_Found += J.m_Result.m_Valid;
			// found nothing: not right away again (back to back, the planner kept a core busy while the partner lay
			// somewhere I couldn't get it out of)
			if(!J.m_Result.m_Valid)
				m_NextRescueJob = m_Now + 10;
			if(J.m_Result.m_Valid)
				m_NoRescueSince = -1;
			else if(m_NoRescueSince < 0)
				m_NoRescueSince = m_Now;
		}
		// the result is at least as good as the seed (the plan I followed); a fresh job replaces whatever I had
		if(J.m_Result.m_Valid || J.m_Fresh)
		{
			if(m_DebugFly && J.m_Result.m_Valid && (J.m_Fresh || J.m_Result.m_Score != m_Plan.m_Score))
				printf("plandbg %d id %d: %s score %.0f hook %d-%d fire %d jump %d air %d dir %d post %d me frozen %d partner ends %.1f %.1f\n", m_Now, m_pB->m_Core.m_Id,
					J.m_Result.m_HookOnly ? "hook-only" : J.m_Result.m_Aled ? "aled" : J.m_Result.m_Delivered ? "delivery" : "save", J.m_Result.m_Score, J.m_Result.m_HookAt,
					J.m_Result.m_HookEnd > 100000 ? -1 : J.m_Result.m_HookEnd, J.m_Result.m_FireAt, J.m_Result.m_JumpAt, J.m_Result.m_AirJumpAt, J.m_Result.m_Dir, J.m_Result.m_PostDir,
					J.m_Result.m_MeFrozen, J.m_Result.m_PartnerEnd.x / 32, J.m_Result.m_PartnerEnd.y / 32);
			m_Plan = J.m_Result;
			m_PlanStart = J.m_Anchor;
		}
	}
	m_pJob.reset();
}

void CHookBotBrain::StartJob(const SHookBotTee &Bot, const SHookBotTee &Partner)
{
	auto pJob = std::make_unique<SRescueJob>();
	SRescueJob &J = *pJob;
	// looking for a rescue by the hook alone too (no hammer rescue came up for a while) takes longer (100 ms, 5 ticks):
	// the plan starts further ahead then (anchored 2 ticks ahead, it arrived after its jump should have happened)
	const bool HookOnly = m_NoRescueSince >= 0 && m_Now - m_NoRescueSince >= SERVER_TICK_SPEED / 2;
	const int Now = m_Now, Lead = HookOnly ? std::max(m_RescueLead, 7) : m_RescueLead;
	CHookBotSim S;
	InitSim(S);
	const int k = Now - m_PlanStart;
	const bool Follow = m_Plan.m_Valid && k >= 0 && k < (int)m_Plan.m_vPath.size() && distance(Bot.m_Core.m_Pos, m_Plan.m_vPath[k]) < 2.0f &&
			    distance(Partner.m_Core.m_Pos, m_Plan.m_vUserPath[k]) < 2.0f;
	// the bridge: what I play until the job's plan starts (the current plan, or just falling), predicted exactly
	m_vBridge.clear();
	m_BridgeStart = Now;
	int PrevHook = m_Input.m_Hook;
	const CNetObj_PlayerInput None = {};
	// no plan to follow: head for the partner meanwhile (a rescue from far above only fits the lookahead once I'm on
	// my way down); the way half a second of walking or steering brings me closest, never into freeze or a kill tile
	int ApproachDir = 0;
	// (not off the ground while the partner isn't frozen yet: it may still land where I can help from here, and on
	// Stronghold's ledge after the ceiling the half second of the check didn't see the freeze pool 15 tiles below)
	if(!Follow && !(Partner.m_FreezeTime == 0 && Bot.m_Grounded))
	{
		float BestD = 1e9f;
		for(int Dir : {0, -1, 1})
		{
			CHookBotSim A = S;
			CNetObj_PlayerInput Move = {};
			Move.m_Direction = Dir;
			Move.m_TargetX = 1;
			bool Bad = false;
			for(int t = 0; t < 25 && !Bad; t++)
			{
				A.Step(Move, None);
				Bad = A.m_aTee[0].m_EnteredFreeze || A.m_aTee[0].m_Dead;
			}
			const float D = Bad ? 1e8f : distance(A.m_aTee[0].m_Core.m_Pos, A.m_aTee[1].m_Core.m_Pos) + (Dir ? 4.0f : 0.0f);
			if(D < BestD)
			{
				BestD = D;
				ApproachDir = Dir;
			}
		}
	}
	for(int t = 0; t < Lead; t++)
	{
		CNetObj_PlayerInput In = {};
		Aim(In, S.m_aTee[1].m_Core.m_Pos - S.m_aTee[0].m_Core.m_Pos);
		In.m_Direction = ApproachDir;
		if(Follow)
			In = RescueInput(m_Plan, k + t, PrevHook, S.m_aTee[0].m_Core.m_HookState, S.m_aTee[1].m_Core.m_Pos - S.m_aTee[0].m_Core.m_Pos,
				S.m_aTee[0].m_Core.m_Pos, S.m_aTee[0].m_Core.m_Vel);
		CNetObj_PlayerInput Bridge = In;
		Bridge.m_Hook = In.m_Hook; // stored as "want", the hook rule is applied again live
		m_vBridge.push_back(Bridge);
		PrevHook = In.m_Hook;
		S.Step(In, None);
	}
	J.m_Base = S;
	J.m_PrevHook = PrevHook;
	J.m_Anchor = Now + Lead;
	J.m_Goal = m_DriveDir;
	J.m_pField = m_pGoal;
	J.m_HookOnly = HookOnly;
	J.m_Epoch = m_RescueEpoch;
	J.m_Fresh = !Follow;
	if(Follow)
	{
		// the plan I follow, re-anchored to the job's start, is the one to beat
		const int d = k + Lead;
		J.m_Seed = m_Plan;
		J.m_Seed.m_Score += d; // the score counts ticks to the hammer from the plan's start
		J.m_Seed.m_HookAt = std::max(0, J.m_Seed.m_HookAt - d);
		J.m_Seed.m_HookEnd -= d;
		if(J.m_Seed.m_Hook2At >= 0)
			J.m_Seed.m_Hook2At = std::max(0, J.m_Seed.m_Hook2At - d);
		J.m_Seed.m_JumpAt = J.m_Seed.m_JumpAt >= d ? J.m_Seed.m_JumpAt - d : -1;
		J.m_Seed.m_AirJumpAt = J.m_Seed.m_AirJumpAt >= d ? J.m_Seed.m_AirJumpAt - d : -1;
		J.m_Seed.m_FireAt -= d;
		J.m_Seed.m_vPath.erase(J.m_Seed.m_vPath.begin(), J.m_Seed.m_vPath.begin() + std::min<int>(d, J.m_Seed.m_vPath.size()));
		J.m_Seed.m_vUserPath.erase(J.m_Seed.m_vUserPath.begin(), J.m_Seed.m_vUserPath.begin() + std::min<int>(d, J.m_Seed.m_vUserPath.size()));
		J.m_Cursor = m_RescueCursor;
	}
	else
	{
		m_Plan = {};
		J.m_Cursor = 0;
	}
	// (looking for a rescue by the hook alone: no hammer rescue came up for a while, and I've nothing better to do)
	J.m_Deadline = PlanClock() + (int64_t)(J.m_HookOnly ? std::max(m_RescueBudgetUs, 100000) : m_RescueBudgetUs) * time_freq() / 1000000;
	J.m_HookCursor = m_HookCursor;
	m_pJob = std::move(pJob);
	auto Work = [](SRescueJob *pJ) {
		const int64_t Start = PlanClock();
		pJ->m_Result = PlanRescue(pJ->m_Base, pJ->m_PrevHook, &pJ->m_Cursor, pJ->m_Seed, pJ->m_Goal, pJ->m_pField.get(), pJ->m_Deadline, pJ->m_HookOnly, &pJ->m_HookCursor);
		pJ->m_Us = (int)((PlanClock() - Start) * 1000000 / time_freq());
		pJ->m_Done = true;
	};
	if(m_AsyncPlanning)
		m_pJob->m_Thread = std::thread(Work, m_pJob.get());
	else
		Work(m_pJob.get());
}

CHookBotBrain::SPlan CHookBotBrain::PlanRescue(const CHookBotSim &Base, int PrevHook0, int *pCursor, const SPlan &Seed, int Goal, const CHookBotGoalField *pField, int64_t Deadline, bool HookOnly, int *pHookCursor)
{
	SPlan Best = Seed;
	const int After = 30, MinFree = 12;
	int Horizon = 80;
	const vec2 U0 = Base.m_aTee[1].m_Core.m_Pos;
	const float U0Dist = pField ? pField->Dist(U0) : 0;
	const bool CanJump = !(Base.m_aTee[0].m_Core.m_Jumped & 2);
	const CNetObj_PlayerInput None = {};
	// progress: towards the goal field's goal if there is one, else along the goal direction
	auto Progress = [&](vec2 P) { return pField ? U0Dist - pField->Dist(P) : Goal ? Goal * (P.x - U0.x) : -absolute(P.x - U0.x); };

	struct SRoll
	{
		CHookBotSim m_S;
		int m_PrevHook = 0;
		bool m_Airborne = false; // I've left the ground
		std::vector<vec2> m_vPath, m_vUserPath; // [k]: positions when deciding plan tick k
	};
	struct SFamily
	{
		int m_HookAt, m_HookEnd, m_Hook2At, m_Dir;
		float m_TargetX;
		bool m_Pre;
		float m_AimDy; // aim above the partner's center: past a corner in the way (Stronghold's tunnel below the freeze strip)
		int m_AirJumpAt = -1, m_DirAfter = 2; // an air jump; from the hook on, another direction (2: no)
	};
	std::vector<SRoll> vSnaps;
	// the hammer in the early input of tick t (positions from the end of tick t-1). Cheapest checks first: the partner
	// in reach and out of the freeze with a clean last move; an upper bound on the score; the partner staying free
	// (and alive) on its own; the score; only then my own fall after the hammer (it must not end on a kill tile)
	// HookOnly: no hammer, I let go of the hook at t: the hook alone has dragged the partner out of the freeze, and it
	// comes to rest outside it (Stronghold: from the tunnel below a freeze strip, a jump and a hook at the top of it,
	// and running on holding it drags the partner lying on the strip down through a freeze column into the tunnel)
	auto TryFire = [&](const SRoll &R, int t, const SFamily &F, int JumpAt, bool HookOnly) {
		const auto &Bot = R.m_S.m_aTee[0], &Usr = R.m_S.m_aTee[1];
		const vec2 Bp = Bot.m_Core.m_Pos, Up = Usr.m_Core.m_Pos;
		if(Bot.m_FreezeTime > 0 || Bot.m_Dead || Usr.m_Dead)
			return;
		if(HookOnly ? (Usr.m_EnteredFreeze || R.m_S.InFreeze(Up) || Progress(Up) - t + 400 <= Best.m_Score) : (Bot.m_Reload > 0 || !HammerReaches(Bp, Up)))
			return;
		// lying in freeze, the hammer can't free it (it refreezes at once), only push it: a delivery at best (below)
		const bool InFreezeNow = Usr.m_EnteredFreeze || R.m_S.InFreeze(Up);
		// an aled: the partner comes out free beyond a freeze wall that I stay behind (it then pulls me through). With a
		// goal field, the wall shows as a jump in the distance to go (freeze costs 8x per tile), which the freeze
		// floor I might be lying in doesn't make
		// (the freeze strictly between us: not the floor I'm lying in, nor the spot the partner is at)
		const vec2 Along = distance(Bp, Up) > 0 ? normalize(Up - Bp) * 16.0f : vec2(0, 0);
		const bool Aled = distance(Bp, Up) > 40 && FreezeOnLine(R.m_S, Bp + Along, Up - Along) &&
				  (pField ? pField->Dist(Up) < pField->Dist(Bp) - 150 : Goal && Goal * (Up.x - Bp.x) > 0);
		CCharacterCore Kicked = Usr.m_Core;
		if(!HookOnly)
			HammerPush(Kicked, Bp);
		const float Clear = std::min(FreezeClearance(R.m_S, Up), 128.0f);
		const float Bound = Progress(Up) + length(Kicked.m_Vel) * After + 5.0f * After - t + 2.0f * After + Clear + (Aled ? 400 : 0) + 250;
		if(Bound <= Best.m_Score)
			return;
		CHookBotSim S = R.m_S;
		S.m_aTee[1].m_Core.m_Vel = Kicked.m_Vel;
		if(!HookOnly)
		{
			S.m_aTee[1].m_FreezeTime = 0;
			S.m_aTee[0].m_Reload = 16;
		}
		const CHookBotSim AfterHammer = S;
		// how long the partner stays free doing nothing: in a narrow tunnel it lands in the freeze soon, so it has to
		// act quickly; it must never die
		int FreeTicks = After;
		vec2 Uend = Up;
		bool Landed = false; // it comes to stand on something outside the freeze: safe, whatever comes next
		for(int j = 0; j < After; j++)
		{
			S.Step(None, None, 1);
			if(S.m_aTee[1].m_Dead)
				return;
			if(S.m_aTee[1].m_EnteredFreeze || S.m_aTee[1].m_FreezeTime > 0)
			{
				FreeTicks = j;
				break;
			}
			Uend = S.m_aTee[1].m_Core.m_Pos;
			const float h = CCharacterCore::PhysicalSize() / 2;
			if(S.m_aTee[1].m_Core.m_Vel.y >= 0 && (S.m_pCollision->CheckPoint(Uend.x + h, Uend.y + h + 5) || S.m_pCollision->CheckPoint(Uend.x - h, Uend.y + h + 5)))
			{
				Landed = true;
				FreeTicks = After;
				break;
			}
		}
		// without its air jump (it was frozen in the air, not lying on the ground, which gives it back) the partner can
		// hardly save me later: it needs more air time, and it's worse anyway ("didn't give me dj")
		const bool NoJump = Usr.m_Core.m_Jumped & 2;
		// not free (or not for long): a delivery still counts, if it comes to rest outside the freeze, where it thaws by
		// itself (hammering a partner that landed in a freeze column through it onto the safe spot behind it)
		bool Delivered = false;
		if(HookOnly || InFreezeNow || FreeTicks < (NoJump ? 20 : MinFree))
		{
			const float h = CCharacterCore::PhysicalSize() / 2;
			for(int j = 0; j < 60 && !Delivered; j++)
			{
				S.Step(None, None, 1);
				const auto &P = S.m_aTee[1];
				if(P.m_Dead)
					return;
				const vec2 Q = P.m_Core.m_Pos;
				Delivered = length(P.m_Core.m_Vel) < 0.6f && !P.m_EnteredFreeze && !S.InFreeze(Q) &&
					    (S.m_pCollision->CheckPoint(Q.x + h, Q.y + h + 5) || S.m_pCollision->CheckPoint(Q.x - h, Q.y + h + 5));
				Uend = Q;
			}
			if(!Delivered)
				return;
		}
		// progress (sooner is better), time to act, and like people do it (sus'terism 2 rank 1, 573 hammerhits): lift
		// the partner well clear of the freeze (median 124 px) and hammer from the air, not while dropping into the
		// freeze myself
		// a delivered partner is safe for sure (it only has to wait a moment), a freed one in the air over freeze may not
		// be: rank landed-and-free, then delivered, then freed in the air
		float Score = Delivered ? Progress(Uend) - 1.0f * t + 150 : Progress(Uend) - 1.0f * t + 2.0f * FreeTicks + Clear;
		// delivered to rest right next to freeze (within 8 px; freezing goes by the middle): a bump and it's back in (a 3-wide free part between two freeze columns:
		// thrown to its far end for the progress, it came to rest a pixel from the next column, and I hooked it back)
		if(Delivered && (R.m_S.InFreeze(Uend + vec2(8, 0)) || R.m_S.InFreeze(Uend - vec2(8, 0))))
			Score -= 150;
		// by the hook alone it's still frozen when it gets there: the wait counts like any other (the hammer frees it at
		// once; without this, hook-only deliveries won over the hammer rescues of Stronghold's opening and slowed it down)
		if(HookOnly)
			Score -= S.m_aTee[1].m_FreezeTime;
		if(Bot.m_EnteredFreeze)
			Score -= 80;
		if(Aled && !Delivered)
			Score += 400;
		if(NoJump && !Delivered)
			Score -= 200;
		if(Landed && !Delivered)
			Score += 250;
		// I never left the ground: I stay where I stood, safe (on the edge by a freeze column, every plan that jumped
		// ended with me in the freeze one way or another)
		if(!R.m_Airborne && !HookOnly)
			Score += 100;
		if(Score <= Best.m_Score)
			return;
		// my own fall after the hammer: never onto a kill tile; and if I can stay out of the freeze (land on something),
		// better (on a freeze floor I land in it whatever I do; on Stronghold's blocks above the freeze ceiling I
		// hammered the partner out over the drop and fell off the end)
		// (where I come to rest matters: frozen in the freeze, the partner has to be able to get me out, within 12 tiles
		// of where it is with nothing in between (Stronghold's corridor after the ledge: it dragged the partner across and
		// fell into the freeze pool 18 tiles below the blocks, out of anybody's reach); passing through freeze and landing
		// outside it, I only wait to thaw)
		int PostDir = 2, PostJump = 0;
		bool MeFrozen = true, MeStuck = true;
		const bool CanAirJump = !(AfterHammer.m_aTee[0].m_Core.m_Jumped & 2);
		for(int Variant = 0; Variant < (CanAirJump ? 6 : 3); Variant++)
		{
			// steering none / left / right, then the same with my air jump once I'm falling
			const int Dir = Variant % 3 == 0 ? 0 : Variant % 3 == 1 ? -1 : 1, Jump = Variant >= 3 ? 2 : 0;
			bool Jumped = false;
			CHookBotSim Me = AfterHammer;
			CNetObj_PlayerInput Steer = {};
			Steer.m_Direction = Dir;
			Steer.m_TargetX = 1;
			bool Safe = true, Frozen = false, Landed = false;
			const float h = CCharacterCore::PhysicalSize() / 2;
			// (by the hook alone: until I've landed and stood a moment, 60 ticks didn't see the end of a 55-tile fall into
			// freeze; after a hammer 60, as tuned: a hammerhit ends in the freeze floor sooner or later anyway)
			for(int j = 0, Stood = 0; j < (HookOnly ? 200 : 60) && Safe && (Stood < 10 || !HookOnly); j++)
			{
				// steer until I land (as the live bot does), then stand
				const vec2 P = Me.m_aTee[0].m_Core.m_Pos;
				if(Me.m_pCollision->CheckPoint(P.x + h, P.y + h + 5) || Me.m_pCollision->CheckPoint(P.x - h, P.y + h + 5))
				{
					Steer.m_Direction = 0;
					Landed = true;
				}
				Stood += Landed;
				CNetObj_PlayerInput I = Steer;
				if(Jump && !Jumped && !Landed && Me.m_aTee[0].m_Core.m_Vel.y > 0)
					I.m_Jump = Jumped = true;
				Me.Step(I, None);
				Safe = !Me.m_aTee[0].m_Dead;
				Frozen |= Me.m_aTee[0].m_EnteredFreeze;
			}
			bool Stuck = false;
			if(Safe && Frozen)
			{
				// frozen: on until I come to rest, then in the freeze or not
				for(int j = 0; j < 250 && Safe; j++)
				{
					const vec2 P = Me.m_aTee[0].m_Core.m_Pos;
					if(length(Me.m_aTee[0].m_Core.m_Vel) < 0.6f && (Me.m_pCollision->CheckPoint(P.x + h, P.y + h + 5) || Me.m_pCollision->CheckPoint(P.x - h, P.y + h + 5)))
						break;
					Me.Step(None, None, 0);
					Safe = !Me.m_aTee[0].m_Dead;
				}
				const vec2 Rest = Me.m_aTee[0].m_Core.m_Pos;
				Stuck = Me.InFreeze(Rest);
				if(Stuck && !CanRescue(Me, Uend, Rest))
					Safe = false;
			}
			if(Safe && (PostDir == 2 || (MeStuck && !Stuck) || (MeFrozen && !Frozen)))
			{
				PostDir = Dir;
				PostJump = Jump;
				MeFrozen = Frozen;
				MeStuck = Stuck;
			}
			if(Safe && !Frozen)
				break;
		}
		if(PostDir == 2)
			return;
		// by the hook alone, not if I end up stuck in the freeze myself: that only swaps who has to be rescued (it pulled
		// the partner out of a notch and fell into the freeze at the bottom of the pit below)
		if(MeStuck && HookOnly)
			return;
		if(MeFrozen)
		{
			Score -= MeStuck ? 150 : 50;
			if(Score <= Best.m_Score)
				return;
		}
		Best.m_Score = Score;
		Best.m_Valid = true;
		Best.m_Aled = Aled && !Delivered;
		Best.m_Delivered = Delivered;
		Best.m_PartnerEnd = Uend;
		Best.m_HookAt = F.m_HookAt;
		Best.m_HookEnd = F.m_HookEnd;
		Best.m_Hook2At = F.m_Hook2At;
		Best.m_JumpAt = JumpAt;
		Best.m_Dir = F.m_Dir;
		Best.m_TargetX = F.m_TargetX;
		Best.m_FireAt = t;
		Best.m_PostDir = PostDir;
		Best.m_PostJump = PostJump;
		Best.m_PreHammer = F.m_Pre;
		Best.m_HookOnly = HookOnly;
		Best.m_AimDy = F.m_AimDy;
		Best.m_AirJumpAt = F.m_AirJumpAt;
		Best.m_DirAfter = F.m_DirAfter;
		Best.m_MeFrozen = MeFrozen;
		Best.m_Grounded = !R.m_Airborne;
		Best.m_vPath = R.m_vPath;
		Best.m_vUserPath = R.m_vUserPath;
	};
	// (End: stop there instead of at the horizon; R is then left at that tick for the caller to go on from)
	auto Run = [&](SRoll &R0, int t0, const SFamily &F, int JumpAt, std::vector<SRoll> *pSnaps, int End = -1, bool InPlace = false) {
		SRoll Copy;
		if(!InPlace)
			Copy = R0;
		SRoll &R = InPlace ? R0 : Copy;
		auto &Bot = R.m_S.m_aTee[0];
		for(int t = t0; t < (End >= 0 ? End : Horizon); t++)
		{
			if(t > 0 && t >= F.m_HookAt && ((t & 1) == 0 || F.m_HookEnd == 0))
			{
				TryFire(R, t, F, JumpAt, false);
				if(HookOnly && t >= F.m_HookAt + 6 && (t & 3) == 0)
					TryFire(R, t, F, JumpAt, true);
			}
			if(Bot.m_FreezeTime > 0 || Bot.m_EnteredFreeze || Bot.m_Dead || R.m_S.m_aTee[1].m_Dead)
				return;
			if(pSnaps)
				pSnaps->push_back(R);
			CNetObj_PlayerInput In = {};
			Aim(In, R.m_S.m_aTee[1].m_Core.m_Pos + vec2(0, F.m_AimDy) - Bot.m_Core.m_Pos);
			const bool Want = (t >= F.m_HookAt && t < F.m_HookEnd) || (F.m_Hook2At >= 0 && t >= F.m_Hook2At);
			In.m_Hook = HookBotHookInput(Want, R.m_PrevHook, Bot.m_Core.m_HookState);
			In.m_Jump = t == JumpAt || t == F.m_AirJumpAt;
			In.m_Direction = F.m_DirAfter != 2 && t >= F.m_HookAt ? F.m_DirAfter : F.m_TargetX >= 0 ? SteerTo(F.m_TargetX, Bot.m_Core.m_Pos.x, Bot.m_Core.m_Vel.x) : F.m_Dir;
			R.m_PrevHook = In.m_Hook;
			R.m_S.Step(In, None);
			{
				const float h = CCharacterCore::PhysicalSize() / 2;
				const vec2 P = Bot.m_Core.m_Pos;
				R.m_Airborne |= !(R.m_S.m_pCollision->CheckPoint(P.x + h, P.y + h + 5) || R.m_S.m_pCollision->CheckPoint(P.x - h, P.y + h + 5));
			}
			R.m_vPath.push_back(Bot.m_Core.m_Pos);
			R.m_vUserPath.push_back(R.m_S.m_aTee[1].m_Core.m_Pos);
		}
	};

	SRoll Root;
	Root.m_S = Base;
	Root.m_PrevHook = PrevHook0;
	Root.m_vPath.push_back(Base.m_aTee[0].m_Core.m_Pos);
	Root.m_vUserPath.push_back(U0);
	// standing on the frozen partner: the plan may start by hammering it; it answers with its held hammer the moment
	// it is unfrozen (then it refreezes in the freeze it lies in), which kicks me up
	const auto &B0 = Base.m_aTee[0], &P0 = Base.m_aTee[1];
	const vec2 OnHead = B0.m_Core.m_Pos - P0.m_Core.m_Pos;
	const bool CanPre = P0.m_FreezeTime > 0 && B0.m_FreezeTime == 0 && B0.m_Reload == 0 && OnHead.y < -20 && OnHead.y > -44 && absolute(OnHead.x) < 28;
	SRoll PreRoot = Root;
	if(CanPre)
	{
		HammerPush(PreRoot.m_S.m_aTee[1].m_Core, B0.m_Core.m_Pos);
		PreRoot.m_S.m_aTee[1].m_FreezeTime = 0;
		HammerPush(PreRoot.m_S.m_aTee[0].m_Core, P0.m_Core.m_Pos);
		PreRoot.m_S.m_aTee[0].m_Reload = 16;
	}
	// plan families: when to hook and for how long (hold until the hammer; a fling that lets go early; a short lift,
	// then hook again until the hammer, which is how people swing a partner through a wall), a direction held
	// throughout, and whether to start with the head hammer; the jump tick is searched inside a family
	struct SHooks
	{
		int m_For, m_Gap;
	};
	static const SHooks s_aHooks[] = {{1 << 20, -1}, {8, -1}, {4, 16}, {14, -1}, {6, 10}, {20, -1}, {4, 8}, {10, 12}};
	static const int s_aHookAt[] = {0, 2, 4, 7, 10, 14, 19, 25};
	// how to move: hold a direction, or steer to an x and hold there: just short of a freeze wall near me (that's
	// where an aled is hammered from: swing the partner past me through the wall), or beside the partner
	struct SMove
	{
		int m_Dir;
		float m_TargetX;
	};
	std::vector<SMove> vMoves = {{(Goal && !pField) ? Goal : 1, -1}, {0, -1}, {(Goal && !pField) ? -Goal : -1, -1}};
	for(int Side : {-1, 1})
	{
		for(float Y : {B0.m_Core.m_Pos.y, P0.m_Core.m_Pos.y})
			for(float d = 8; d < 14 * 32; d += 8)
			{
				const float X = B0.m_Core.m_Pos.x + Side * d;
				if(Base.InFreeze(vec2(X, Y)))
				{
					const float Edge = std::floor(X / 32) * 32 + (Side < 0 ? 32 : 0); // the wall's side facing me
					for(float Gap : {8.0f, 18.0f})
						if(Edge - Side * Gap > 0)
							vMoves.push_back({0, Edge - Side * Gap});
					break;
				}
			}
		vMoves.push_back({0, P0.m_Core.m_Pos.x + Side * 64});
	}
	// rescues by the hook alone from a jump (only when HookOnly, first): walk to a spot, jump (and air-jump), hook some
	// ticks after the jump, aiming at the partner or above it, then hold a direction to drag it out (Stronghold: the
	// partner lying in a freeze notch in the side of a pillar, 18 tiles up: walk 4 tiles back, jump, air-jump, hook
	// it at the top aiming a tee above it, run back, which drags it out onto the floor). The hook timings of the
	// families below count from the plan's start and end at tick 25, one direction all the way, one jump: none of that
	// could do it
	if(HookOnly && pHookCursor)
	{
		static const int s_aAir[] = {-1, 8, 12, 16, 20, 24}, s_aHook[] = {4, 8, 12, 16, 20, 24, 28, 32, 36};
		static const float s_aAim[] = {0, -24, -40};
		const int NumX = 21, NumFam = NumX * (int)std::size(s_aAir) * (int)std::size(s_aHook);
		Horizon = 150;
		for(int n = 0; n < NumFam && PlanClock() <= Deadline; n++)
		{
			const int Fam = (*pHookCursor)++ % NumFam;
			// the nearest spots first: 0, -1, 1, -2, 2, ... tiles
			const int k = Fam / ((int)std::size(s_aAir) * (int)std::size(s_aHook));
			const float X = B0.m_Core.m_Pos.x + 32.0f * ((k + 1) / 2) * (k % 2 ? -1 : 1);
			const int AirRel = s_aAir[Fam / (int)std::size(s_aHook) % (int)std::size(s_aAir)], HookRel = s_aHook[Fam % (int)std::size(s_aHook)];
			if(AirRel >= 0 && HookRel <= AirRel - 8)
				continue; // hooking well before the air jump is the plain jump's family
			if(Base.m_pCollision->CheckPoint(X, B0.m_Core.m_Pos.y))
				continue;
			// walk there (and stand), then jump
			SFamily F = {};
			F.m_TargetX = X;
			F.m_Dir = 0;
			F.m_Pre = false;
			F.m_HookAt = 1 << 20; // (none until the jump)
			F.m_HookEnd = 1 << 20;
			F.m_Hook2At = -1;
			SRoll R = Root;
			int JumpAt = -1;
			for(int t = 0; t < 40 && JumpAt < 0; t++)
			{
				const auto &Me = R.m_S.m_aTee[0];
				if(absolute(Me.m_Core.m_Pos.x - X) < 8 && absolute(Me.m_Core.m_Vel.x) < 2 && Me.m_Core.m_Vel.y == 0)
					JumpAt = t;
				else
					Run(R, t, F, -1, nullptr, t + 1, true);
			}
			if(JumpAt < 0 || R.m_S.m_aTee[0].m_FreezeTime > 0 || R.m_S.m_aTee[0].m_Dead)
				continue;
			F.m_HookAt = JumpAt + HookRel;
			F.m_AirJumpAt = AirRel >= 0 ? JumpAt + AirRel : -1;
			for(float Aim : s_aAim)
				for(int DirAfter : {-1, 1, 0})
				{
					F.m_AimDy = Aim;
					F.m_DirAfter = DirAfter;
					Run(R, JumpAt, F, JumpAt, nullptr, Horizon);
				}
		}
		Horizon = 80;
	}

	// first, every time: no hook at all, standing where I am (jumping at some tick, maybe): wait and hammer it when it's in
	// reach (from the edge I stand on as it lands frozen next to it; every plan used to hook, and the ones that jumped
	// and hammered from the air dropped me in the freeze)
	{
		SFamily F = {};
		F.m_HookAt = 0;
		F.m_HookEnd = 0;
		F.m_Hook2At = -1;
		F.m_Dir = 0;
		F.m_TargetX = -1;
		F.m_Pre = false;
		F.m_AimDy = 0;
		vSnaps.clear();
		Run(Root, 0, F, -1, CanJump ? &vSnaps : nullptr);
		for(int JumpAt = 0; JumpAt < (int)vSnaps.size() && JumpAt <= 50 && PlanClock() <= Deadline; JumpAt += 2)
			Run(vSnaps[JumpAt], JumpAt, F, JumpAt, nullptr);
	}

	const int NumMoves = vMoves.size();
	const int NumHooks = std::size(s_aHooks), NumPlain = NumHooks * 8 * NumMoves;
	// aiming at its center, or half a tee above it (the head start of the hook clears a corner in the way that way)
	const int NumAims = 2, NumAimed = NumPlain * NumAims;
	const int NumFamilies = NumAimed * (CanPre ? 2 : 1);
	for(int n = 0; n < NumFamilies && PlanClock() <= Deadline; n++)
	{
		int Family = (*pCursor)++ % NumFamilies;
		SFamily F;
		F.m_Pre = Family >= NumAimed;
		Family %= NumAimed;
		F.m_AimDy = Family >= NumPlain ? -16.0f : 0.0f;
		Family %= NumPlain;
		const SHooks &H = s_aHooks[Family / (8 * NumMoves)];
		F.m_HookAt = s_aHookAt[Family / NumMoves % 8];
		F.m_Dir = vMoves[Family % NumMoves].m_Dir;
		F.m_TargetX = vMoves[Family % NumMoves].m_TargetX;
		F.m_HookEnd = H.m_For == 1 << 20 ? H.m_For : F.m_HookAt + H.m_For;
		F.m_Hook2At = H.m_Gap >= 0 ? F.m_HookEnd + H.m_Gap : -1;
		vSnaps.clear();
		Run(F.m_Pre ? PreRoot : Root, 0, F, -1, CanJump ? &vSnaps : nullptr);
		for(int JumpAt = 0; JumpAt < (int)vSnaps.size() && JumpAt <= 50; JumpAt += 2)
			Run(vSnaps[JumpAt], JumpAt, F, JumpAt, nullptr);
	}
	return Best;
}

CHookBotBrain::SPlan CHookBotBrain::PlanDive(const CHookBotSim &Base, int PrevHook0) const
{
	const int64_t Start = PlanClock();
	SPlan Best;
	const int Horizon = 80;
	const vec2 B0 = Base.m_aTee[0].m_Core.m_Pos, U0 = Base.m_aTee[1].m_Core.m_Pos;
	const int Toward = U0.x > B0.x ? 1 : -1;
	const bool CanJump = !(Base.m_aTee[0].m_Core.m_Jumped & 2);
	const CNetObj_PlayerInput None = {};

	// one rollout state; rollouts that only differ in the jump tick share everything before it
	struct SRoll
	{
		CHookBotSim m_S;
		int m_PrevHook = 0;
		bool m_UserOut = false; // the partner has come out on the far side of the freeze
		std::vector<vec2> m_vPath, m_vUserPath; // m_vPath[k]: where I am when deciding plan tick k
	};
	std::vector<SRoll> vSnaps;
	// runs from R (at tick t0) to the hammer or a dead end; with pSnaps, stores the state before every tick
	auto Run = [&](SRoll R, int t0, int HookAt, int JumpAt, int Dir, std::vector<SRoll> *pSnaps) {
		auto &Bot = R.m_S.m_aTee[0];
		auto &Usr = R.m_S.m_aTee[1];
		for(int t = t0; t < Horizon; t++)
		{
			if(t > 0 && Bot.m_EnteredFreeze)
			{
				// fire in the early input of tick t, with the positions from the end of tick t-1
				vec2 Bp = Bot.m_Core.m_Pos, Up = Usr.m_Core.m_Pos;
				float d = distance(Bp, Up);
				// the partner has to be on the far side of the freeze I just entered
				auto &St = const_cast<CHookBotBrain *>(this)->m_Stats;
				St.m_Entered++;
				if(!HammerReaches(Bp, Up))
				{
					St.m_FarReach++;
					return;
				}
				if(Bot.m_Reload > 0)
				{
					St.m_Reloading++;
					return;
				}
				if(Bot.m_FreezeTime > 0 || Usr.m_EnteredFreeze || R.m_S.InFreeze(Up) || !FreezeOnLine(R.m_S, R.m_vPath[R.m_vPath.size() - 2], Up))
				{
					St.m_NotBeyond++;
					return;
				}
				CHookBotSim After = R.m_S;
				HammerPush(After.m_aTee[1].m_Core, Bp);
				After.m_aTee[1].m_FreezeTime = 0;
				for(int k = 0; k < 12; k++)
				{
					After.Step(None, None);
					if(After.m_aTee[1].m_EnteredFreeze || After.m_aTee[1].m_FreezeTime)
					{
						const_cast<CHookBotBrain *>(this)->m_Stats.m_Refrozen++;
						return;
					}
				}
				float Score = (61.5f - d) - 0.1f * t;
				if(Score > Best.m_Score)
				{
					Best.m_Score = Score;
					Best.m_Valid = true;
					Best.m_HookAt = HookAt;
					Best.m_JumpAt = JumpAt;
					Best.m_Dir = Dir;
					Best.m_FireAt = t;
					Best.m_Dist = d;
					Best.m_vPath = R.m_vPath;
					Best.m_vUserPath = R.m_vUserPath;
				}
				return;
			}
			if(Bot.m_FreezeTime > 0 || Bot.m_Core.m_Pos.y > B0.y + 400)
				return;
			// hopeless: the partner fell back into the freeze, or I'm falling away with no jump left
			if(R.m_UserOut && Usr.m_EnteredFreeze)
				return;
			R.m_UserOut |= Usr.m_FreezeTime > 0 && !Usr.m_EnteredFreeze;
			bool JumpLeft = pSnaps ? CanJump : JumpAt >= t;
			if(!JumpLeft && Bot.m_Core.m_Vel.y > 0 && Bot.m_Core.m_Pos.y > B0.y + 32 && (HookAt < 0 || HookAt < t - 10))
				return;
			if(pSnaps)
				pSnaps->push_back(R);
			CNetObj_PlayerInput In = {};
			Aim(In, Usr.m_Core.m_Pos - Bot.m_Core.m_Pos);
			In.m_Hook = HookBotHookInput(HookAt >= 0 && t >= HookAt, R.m_PrevHook, Bot.m_Core.m_HookState);
			In.m_Jump = t == JumpAt;
			In.m_Direction = Dir;
			R.m_PrevHook = In.m_Hook;
			R.m_S.Step(In, PartnerModel(R.m_S));
			R.m_vPath.push_back(Bot.m_Core.m_Pos);
			R.m_vUserPath.push_back(Usr.m_Core.m_Pos);
		}
	};

	SRoll Root;
	Root.m_S = Base;
	Root.m_PrevHook = PrevHook0;
	Root.m_vPath.push_back(B0);
	Root.m_vUserPath.push_back(U0);
	for(int HookAt : {0, -1, 3, 7, 1, 2, 5, 10, 14, 18, 24, 30})
		for(int Dir : {0, Toward})
		{
			if(PlanClock() > m_PlanDeadline)
				break;
			vSnaps.clear();
			Run(Root, 0, HookAt, -1, Dir, CanJump ? &vSnaps : nullptr);
			for(int JumpAt = 0; JumpAt < (int)vSnaps.size() && JumpAt <= 70 && PlanClock() <= m_PlanDeadline; JumpAt++)
				Run(vSnaps[JumpAt], JumpAt, HookAt, JumpAt, Dir, nullptr);
		}
	const_cast<CHookBotBrain *>(this)->m_LastPlanUs = (int)((PlanClock() - Start) * 1000000 / time_freq());
	return Best;
}

// ---- team moves ----

static bool OnGroundAt(const CCollision *pCol, vec2 P)
{
	const float h = CCharacterCore::PhysicalSize() / 2;
	return pCol->CheckPoint(P.x + h, P.y + h + 5) || pCol->CheckPoint(P.x - h, P.y + h + 5);
}

static bool AtRest(const CHookBotSim &S, int i)
{
	return length(S.m_aTee[i].m_Core.m_Vel) < 0.6f && OnGroundAt(S.m_pCollision, S.m_aTee[i].m_Core.m_Pos);
}

// resting this close to freeze (8 px, freezing goes by the middle), a bump puts it back in
static bool NextToFreeze(const CHookBotSim &S, vec2 P)
{
	return S.InFreeze(P + vec2(8, 0)) || S.InFreeze(P - vec2(8, 0));
}

void CHookBotBrain::InitTeamSim(CHookBotSim &S) const
{
	if(m_pB->m_Core.m_Id < m_pU->m_Core.m_Id)
		S.Init(m_pCollision, m_pTeams, *m_pB, *m_pU, m_BotFirst);
	else
		S.Init(m_pCollision, m_pTeams, *m_pU, *m_pB, !m_BotFirst);
	S.m_Tick = m_Now;
}

// the columns (x of tile centers) within 12 tiles of From where freeze is the first thing above From's row (within 22
// tiles) and the goal is at least 300 closer right above that freeze: where one of us can go up through it. Nearest first
static std::vector<float> FreezeColumns(const CHookBotSim &S, const CHookBotGoalField &Field, vec2 From)
{
	std::vector<float> vCols;
	const int fx = (int)(From.x / 32), fy = (int)(From.y / 32);
	const float Here = Field.Dist(From);
	for(int d = 0; d <= 12; d++)
		for(int Side : {-1, 1})
		{
			if(d == 0 && Side == 1)
				continue;
			const int x = fx + d * Side;
			for(int y = fy - 1; y >= std::max(1, fy - 22); y--)
			{
				const vec2 C(x * 32 + 16, y * 32 + 16);
				if(S.m_pCollision->CheckPoint(C))
					break;
				if(!S.InFreeze(C))
					continue;
				int Top = y;
				while(Top > 0 && S.InFreeze(vec2(C.x, Top * 32 + 16)))
					Top--;
				const vec2 Above(C.x, Top * 32 + 16);
				if(!S.m_pCollision->CheckPoint(Above) && Field.Dist(Above) < Here - 300)
					vCols.push_back(C.x);
				break;
			}
		}
	return vCols;
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanThrow(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_THROW;
	int Steps = 0, FoundAt = 0;
	const vec2 Mid = (Base.m_aTee[0].m_Core.m_Pos + Base.m_aTee[1].m_Core.m_Pos) / 2;
	const std::vector<float> vAll = FreezeColumns(Base, Field, Mid);
	if(vAll.empty())
		return Best;
	// every other column, the nearest 4 (they only set which way and how far each of us drifts before the throw)
	std::vector<float> vCols;
	for(int i = 0; i < (int)vAll.size() && vCols.size() < 4; i += 2)
		vCols.push_back(vAll[i]);
	// (from SimMapBots.ThrowSearch: of 746k throws from Stronghold's floor below its ceiling gap, the 529 that worked
	// had the thrown one air-jump at 20-28 ticks, near the top of its jump, and the thrower jump 0-16 ticks after it,
	// mostly 2, air-jumping 6-18 ticks later or not at all; the hammer right after the air jump: the air jump sets the
	// speed up to 12 px/tick, the hammer adds 11 on top)
	static const int s_aAirP[] = {28, 24, 20, 16};
	static const int s_aJumpT[] = {2, 0, 4, 8, 12};
	static const int s_aAirT[] = {6, 10, -1, 14, 18};
	const CNetObj_PlayerInput None = {};
	auto Input = [](int Dir, bool Jump, vec2 AimDir) {
		CNetObj_PlayerInput I = {};
		I.m_Direction = Dir;
		I.m_Jump = Jump;
		Aim(I, AimDir);
		return I;
	};
	for(int Up = 0; Up <= 1; Up++)
	{
		const int P = Up, T = 1 - Up; // thrown, thrower
		const vec2 P0 = Base.m_aTee[P].m_Core.m_Pos, T0 = Base.m_aTee[T].m_Core.m_Pos;
		const float P0Dist = Field.Dist(P0), T0Dist = Field.Dist(T0);
		std::vector<float> vXT = vCols;
		vXT.push_back(T0.x);
		for(float XP : vCols)
			for(int AirP : s_aAirP)
				for(float XT : vXT)
					for(int JumpT : s_aJumpT)
						for(int AirT : s_aAirT)
						{
							// (once one works, a while longer for a better one)
							if(Steps > MaxSteps || (Best.m_Valid && Steps > FoundAt + MaxSteps / 6))
								goto Done;
							const int AirTAbs = AirT >= 0 ? JumpT + AirT : -1;
							CHookBotSim S = Base;
							std::vector<CNetObj_PlayerInput> avIn[2];
							std::vector<vec2> avPath[2];
							for(int t = 0; t <= AirP + 3; t++)
							{
								const auto &Pt = S.m_aTee[P], &Tt = S.m_aTee[T];
								const vec2 Pp = Pt.m_Core.m_Pos, Tp = Tt.m_Core.m_Pos;
								// the hammer in the early input of tick t (positions from the end of the last one), right after
								// the thrown one's air jump
								if(t > AirP && Tt.m_Reload == 0 && Tt.m_FreezeTime == 0 && Pt.m_FreezeTime == 0 && HammerReaches(Tp, Pp) && Pp.y < Tp.y - 8 &&
									!S.m_pCollision->IntersectLine(Tp, Pp, nullptr, nullptr))
								{
									for(int DirAfter : {0, -1, 1})
									{
										CHookBotSim H = S;
										HammerPush(H.m_aTee[P].m_Core, Tp);
										H.m_aTee[T].m_Reload = 16;
										std::vector<CNetObj_PlayerInput> avTail[2];
										std::vector<vec2> avTailPath[2];
										bool Bad = false, Apex = false;
										int Rest = 0;
										for(int k = 0; k < 300 && !Bad && Rest < 2; k++)
										{
											const auto &Hp = H.m_aTee[P], &Ht = H.m_aTee[T];
											// it has to be through by the top of its flight
											if(!Apex && Hp.m_Core.m_Vel.y >= 0)
											{
												Apex = true;
												if(H.InFreeze(Hp.m_Core.m_Pos) || Field.Dist(Hp.m_Core.m_Pos) > P0Dist - 300)
												{
													Bad = true;
													break;
												}
											}
											CNetObj_PlayerInput Ip = Input(DirAfter, false, Ht.m_Core.m_Pos - Hp.m_Core.m_Pos);
											// the thrower: back onto the floor it threw from (its air jump may come after the hammer)
											CNetObj_PlayerInput It = Input(SteerTo(T0.x, Ht.m_Core.m_Pos.x, Ht.m_Core.m_Vel.x), t + k == AirTAbs, Hp.m_Core.m_Pos - Ht.m_Core.m_Pos);
											It.m_Fire = k == 0;
											avTailPath[P].push_back(Hp.m_Core.m_Pos);
											avTailPath[T].push_back(Ht.m_Core.m_Pos);
											avTail[P].push_back(Ip);
											avTail[T].push_back(It);
											H.Step(P == 0 ? Ip : It, P == 0 ? It : Ip);
											Steps++;
											Bad = H.m_aTee[0].m_Dead || H.m_aTee[1].m_Dead || H.m_aTee[T].m_EnteredFreeze;
											Rest = AtRest(H, 0) && AtRest(H, 1) && Apex ? Rest + 1 : 0;
										}
										if(Bad || Rest < 2)
											continue;
										const vec2 Pe = H.m_aTee[P].m_Core.m_Pos, Te = H.m_aTee[T].m_Core.m_Pos;
										const float Gain = P0Dist - Field.Dist(Pe), Loss = Field.Dist(Te) - T0Dist;
										if(H.InFreeze(Pe) || H.InFreeze(Te) || Gain < 300 || Loss > 64)
											continue;
										const int Len = t + (int)avTail[P].size();
										float Score = Gain - std::max(Loss, 0.0f) - 0.5f * Len;
										if(NextToFreeze(H, Pe))
											Score -= 150;
										if(NextToFreeze(H, Te))
											Score -= 150;
										if(Score <= Best.m_Score)
											continue;
										if(!Best.m_Valid)
											FoundAt = Steps;
										Best.m_Valid = true;
										Best.m_Score = Score;
										Best.m_Up = P;
										Best.m_aEnd[P] = Pe;
										Best.m_aEnd[T] = Te;
										for(int i = 0; i < 2; i++)
										{
											Best.m_avIn[i] = avIn[i];
											Best.m_avIn[i].insert(Best.m_avIn[i].end(), avTail[i].begin(), avTail[i].end());
											Best.m_avPath[i] = avPath[i];
											Best.m_avPath[i].insert(Best.m_avPath[i].end(), avTailPath[i].begin(), avTailPath[i].end());
										}
									}
								}
								if(t == AirP + 3)
									break;
								CNetObj_PlayerInput Ip = Input(SteerTo(XP, Pp.x, Pt.m_Core.m_Vel.x), t == 0 || t == AirP, Tp - Pp);
								CNetObj_PlayerInput It = Input(SteerTo(XT, Tp.x, Tt.m_Core.m_Vel.x), t == JumpT || t == AirTAbs, Pp - Tp);
								avPath[P].push_back(Pp);
								avPath[T].push_back(Tp);
								avIn[P].push_back(Ip);
								avIn[T].push_back(It);
								S.Step(P == 0 ? Ip : It, P == 0 ? It : Ip);
								Steps++;
								if(S.m_aTee[0].m_EnteredFreeze || S.m_aTee[1].m_EnteredFreeze || S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead)
									break;
							}
						}
	}
Done:
	Best.m_Steps = Steps;
	(void)None;
	return Best;
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanCatch(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_CATCH;
	int Steps = 0;
	const int Top = Field.Dist(Base.m_aTee[0].m_Core.m_Pos) < Field.Dist(Base.m_aTee[1].m_Core.m_Pos) ? 0 : 1, Low = 1 - Top;
	const vec2 T0 = Base.m_aTee[Top].m_Core.m_Pos, L0 = Base.m_aTee[Low].m_Core.m_Pos;
	const float T0Dist = Field.Dist(T0), L0Dist = Field.Dist(L0);
	// where the low one jumps: under the freeze above it, nearest to the top one first, every other tile
	std::vector<float> vAll = FreezeColumns(Base, Field, L0);
	if(vAll.empty())
		return Best;
	std::stable_sort(vAll.begin(), vAll.end(), [&](float a, float b) { return absolute(a - T0.x) < absolute(b - T0.x); });
	std::vector<float> vXL;
	for(int i = 0; i < (int)vAll.size() && vXL.size() < 6; i += 2)
		vXL.push_back(vAll[i]);
	// where the top one hooks from: the edge of its floor towards that freeze (as far out as it can stand outside it), a
	// bit back from there, or where it stands
	const int Side = vXL[0] > T0.x ? 1 : -1;
	float Edge = T0.x;
	for(int i = 0; i < 400; i++)
	{
		const vec2 Next(Edge + Side * 2, T0.y);
		if(!OnGroundAt(Base.m_pCollision, Next) || Base.InFreeze(Next) || Base.m_pCollision->TestBox(Next, vec2(28, 28)))
			break;
		Edge = Next.x;
	}
	std::vector<float> vXT = {Edge, Edge - Side * 12};
	if(absolute(T0.x - Edge) > 24)
		vXT.push_back(T0.x);
	// (from SimMapBots.CatchSearch: 1117 of 57600 catches worked from Stronghold's floor below the gap, with the jump
	// from a standstill or a run, the air jump 10-22 ticks after or none, the hook 0-36 ticks after the jump)
	static const int s_aJumpL[] = {0, 8, 4, 12, 16, 24};
	static const int s_aAirL[] = {16, 22, 10, -1};
	static const int s_aHookRel[] = {8, 12, 16, 4, 20, 24, 28, 32, 0, 36};
	const int MaxHookRel = 36;
	auto Input = [](int Dir, bool Jump, vec2 AimDir) {
		CNetObj_PlayerInput I = {};
		I.m_Direction = Dir;
		I.m_Jump = Jump;
		Aim(I, AimDir);
		return I;
	};
	struct SSnap
	{
		CHookBotSim m_S;
		int m_Tick;
	};
	for(float XL : vXL)
		for(int JumpL : s_aJumpL)
			for(int AirL : s_aAirL)
				for(float XT : vXT)
				{
					if(Steps > MaxSteps)
						goto Done;
					const int AirLAbs = AirL >= 0 ? JumpL + AirL : -1;
					auto LowInput = [&](const CHookBotSim &S, int t) {
						const auto &L = S.m_aTee[Low];
						return Input(SteerTo(XL, L.m_Core.m_Pos.x, L.m_Core.m_Vel.x), t == JumpL || t == AirLAbs, S.m_aTee[Top].m_Core.m_Pos - L.m_Core.m_Pos);
					};
					// the part before the hook, the same for every hook timing: both walk to their spots, the low one jumps
					CHookBotSim S = Base;
					std::vector<CNetObj_PlayerInput> avIn[2];
					std::vector<vec2> avPath[2];
					std::vector<SSnap> vSnaps;
					bool Bad = false;
					for(int t = 0; t <= JumpL + MaxHookRel && !Bad; t++)
					{
						vSnaps.push_back({S, t});
						const auto &Tt = S.m_aTee[Top], &Lt = S.m_aTee[Low];
						CNetObj_PlayerInput Il = LowInput(S, t);
						CNetObj_PlayerInput It = Input(SteerTo(XT, Tt.m_Core.m_Pos.x, Tt.m_Core.m_Vel.x), false, Lt.m_Core.m_Pos - Tt.m_Core.m_Pos);
						avPath[Top].push_back(Tt.m_Core.m_Pos);
						avPath[Low].push_back(Lt.m_Core.m_Pos);
						avIn[Top].push_back(It);
						avIn[Low].push_back(Il);
						S.Step(Top == 0 ? It : Il, Top == 0 ? Il : It);
						Steps++;
						Bad = S.m_aTee[0].m_EnteredFreeze || S.m_aTee[1].m_EnteredFreeze || S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead;
					}
					for(int HookRel : s_aHookRel)
					{
						const int HookAt = JumpL + HookRel;
						if(HookAt >= (int)vSnaps.size())
							continue;
						const CHookBotSim &At = vSnaps[HookAt].m_S;
						const vec2 Tp = At.m_aTee[Top].m_Core.m_Pos, Lp = At.m_aTee[Low].m_Core.m_Pos;
						// a hook can't get it from here (380 px, and the hook starts 42 px out) or through a wall
						if(distance(Tp, Lp) > 400 || At.m_pCollision->IntersectLine(Tp, Lp, nullptr, nullptr))
							continue;
						for(float AimDy : {0.0f, -16.0f})
							for(int Dir : {Side < 0 ? 1 : -1, 0, Side})
							{
								if(Steps > MaxSteps)
									goto Done;
								CHookBotSim H = At;
								std::vector<CNetObj_PlayerInput> avTail[2];
								std::vector<vec2> avTailPath[2];
								int PrevHook = H.m_aTee[Top].m_PrevInput.m_Hook;
								bool Fail = false, Grabbed = false, Released = false;
								float StopX = 0;
								int Rest = 0;
								for(int k = 0; k < 300 && !Fail && Rest < 2; k++)
								{
									const int t = HookAt + k;
									const auto &Ht = H.m_aTee[Top], &Hl = H.m_aTee[Low];
									Grabbed |= Ht.m_Core.HookedPlayer() == Hl.m_Core.m_Id && Ht.m_Core.m_HookState == HOOK_GRABBED;
									if(k == 8 && !Grabbed)
									{
										Fail = true;
										break;
									}
									// hold it while it's below me, then let go and stop
									const bool Want = !Released && Hl.m_Core.m_Pos.y > Ht.m_Core.m_Pos.y - 8 && (k < 8 || Ht.m_Core.HookedPlayer() == Hl.m_Core.m_Id);
									if(!Want && !Released)
									{
										Released = true;
										StopX = Ht.m_Core.m_Pos.x;
									}
									CNetObj_PlayerInput It = Input(Released ? SteerTo(StopX, Ht.m_Core.m_Pos.x, Ht.m_Core.m_Vel.x) : Dir, false, Hl.m_Core.m_Pos + vec2(0, AimDy) - Ht.m_Core.m_Pos);
									It.m_Hook = HookBotHookInput(Want, PrevHook, Ht.m_Core.m_HookState);
									PrevHook = It.m_Hook;
									CNetObj_PlayerInput Il = LowInput(H, t);
									avTailPath[Top].push_back(Ht.m_Core.m_Pos);
									avTailPath[Low].push_back(Hl.m_Core.m_Pos);
									avTail[Top].push_back(It);
									avTail[Low].push_back(Il);
									H.Step(Top == 0 ? It : Il, Top == 0 ? Il : It);
									Steps++;
									Fail = H.m_aTee[0].m_Dead || H.m_aTee[1].m_Dead || H.m_aTee[Top].m_EnteredFreeze;
									// falling back with the hook gone, still below the freeze: no catch
									if(k > 8 && !Fail && H.m_aTee[Low].m_Core.m_Vel.y > 0 && H.m_aTee[Top].m_Core.HookedPlayer() != H.m_aTee[Low].m_Core.m_Id &&
										Field.Dist(H.m_aTee[Low].m_Core.m_Pos) > L0Dist - 300 && !H.InFreeze(H.m_aTee[Low].m_Core.m_Pos) && H.m_aTee[Low].m_FreezeTime == 0)
										Fail = true;
									Rest = Released && AtRest(H, 0) && AtRest(H, 1) ? Rest + 1 : 0;
								}
								if(Fail || Rest < 2)
									continue;
								const vec2 Te = H.m_aTee[Top].m_Core.m_Pos, Le = H.m_aTee[Low].m_Core.m_Pos;
								const float Gain = L0Dist - Field.Dist(Le), Loss = Field.Dist(Te) - T0Dist;
								if(H.InFreeze(Te) || H.InFreeze(Le) || Gain < 300 || Loss > 96)
									continue;
								const int Len = HookAt + (int)avTail[Top].size();
								float Score = Gain - std::max(Loss, 0.0f) - 0.5f * Len - 0.25f * H.m_aTee[Low].m_FreezeTime;
								if(NextToFreeze(H, Te))
									Score -= 150;
								if(NextToFreeze(H, Le))
									Score -= 150;
								if(Score <= Best.m_Score)
									continue;
								Best.m_Valid = true;
								Best.m_Score = Score;
								Best.m_Up = Low;
								Best.m_aEnd[Top] = Te;
								Best.m_aEnd[Low] = Le;
								for(int i = 0; i < 2; i++)
								{
									Best.m_avIn[i].assign(avIn[i].begin(), avIn[i].begin() + HookAt);
									Best.m_avIn[i].insert(Best.m_avIn[i].end(), avTail[i].begin(), avTail[i].end());
									Best.m_avPath[i].assign(avPath[i].begin(), avPath[i].begin() + HookAt);
									Best.m_avPath[i].insert(Best.m_avPath[i].end(), avTailPath[i].begin(), avTailPath[i].end());
								}
							}
					}
				}
Done:
	Best.m_Steps = Steps;
	return Best;
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanGather(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_GATHER;
	int Steps = 0;
	const CCollision *pCol = Base.m_pCollision;
	const vec2 Mid = (Base.m_aTee[0].m_Core.m_Pos + Base.m_aTee[1].m_Core.m_Pos) / 2;
	// the spots: standing places outside the freeze within 10 tiles to the side, 4 up to 16 down, with a freeze ceiling
	// above that the way goes up through; nearest to the goal first, the best 4 floors' worth
	struct SSpot
	{
		float m_D;
		vec2 m_Pos;
	};
	std::vector<SSpot> vSpots;
	const int mx = (int)(Mid.x / 32), my = (int)(Mid.y / 32);
	for(int y = my - 4; y <= my + 16; y++)
		for(int x = mx - 10; x <= mx + 10; x++)
		{
			const vec2 C(x * 32 + 16, (y + 1) * 32 - 15);
			if(pCol->CheckPoint(C) || Base.InFreeze(C) || pCol->GetCollisionAt(C.x, C.y) == TILE_DEATH || !OnGroundAt(pCol, C) || Base.TouchesDeath(C) ||
				!pCol->CheckPoint(C.x, C.y + 20) || pCol->TestBox(C, vec2(28, 28)) || NextToFreeze(Base, C))
				continue;
			if(FreezeColumns(Base, Field, C).empty())
				continue;
			vSpots.push_back({Field.Dist(C), C});
		}
	std::stable_sort(vSpots.begin(), vSpots.end(), [](const SSpot &a, const SSpot &b) { return a.m_D < b.m_D; });
	if(vSpots.size() > 6)
		vSpots.resize(6);
	if(vSpots.empty())
		return Best;
	static const int s_aAir[] = {-1, 0, 8, 16, 24, 32};
	auto Input = [](int Dir, bool Jump) {
		CNetObj_PlayerInput I = {};
		I.m_Direction = Dir;
		I.m_Jump = Jump;
		I.m_TargetX = 1;
		return I;
	};
	for(int i = 0; i < (int)vSpots.size(); i++)
		for(int j = 0; j < (int)vSpots.size(); j++)
		{
			const vec2 A = vSpots[i].m_Pos, B = vSpots[j].m_Pos;
			// next to each other on the same floor (not on top of each other: a tee's width apart at least)
			if(i == j || absolute(A.y - B.y) > 1 || absolute(A.x - B.x) < 40 || absolute(A.x - B.x) > 5 * 32)
				continue;
			for(int Air0 : s_aAir)
				for(int Air1 : s_aAir)
				{
					if(Steps > MaxSteps)
						goto Done;
					CHookBotSim S = Base;
					std::vector<CNetObj_PlayerInput> avIn[2];
					std::vector<vec2> avPath[2];
					const float aX[2] = {A.x, B.x};
					const int aAir[2] = {Air0, Air1};
					bool Bad = false;
					int Rest = 0;
					for(int t = 0; t < 200 && !Bad && Rest < 2; t++)
					{
						CNetObj_PlayerInput aI[2];
						for(int k = 0; k < 2; k++)
						{
							const auto &T = S.m_aTee[k];
							// (an air jump only if I still have it; none from the ground)
							const bool Jump = t == aAir[k] && !(T.m_Core.m_Jumped & 2) && !OnGroundAt(pCol, T.m_Core.m_Pos);
							aI[k] = Input(SteerTo(aX[k], T.m_Core.m_Pos.x, T.m_Core.m_Vel.x), Jump);
							avPath[k].push_back(T.m_Core.m_Pos);
							avIn[k].push_back(aI[k]);
						}
						S.Step(aI[0], aI[1]);
						Steps++;
						Bad = S.m_aTee[0].m_EnteredFreeze || S.m_aTee[1].m_EnteredFreeze || S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead;
						Rest = AtRest(S, 0) && AtRest(S, 1) ? Rest + 1 : 0;
					}
					if(Bad || Rest < 2)
						continue;
					const vec2 E0 = S.m_aTee[0].m_Core.m_Pos, E1 = S.m_aTee[1].m_Core.m_Pos;
					// both on that floor, next to each other
					if(absolute(E0.y - A.y) > 4 || absolute(E1.y - A.y) > 4 || absolute(E0.x - E1.x) > 6 * 32 || NextToFreeze(S, E0) || NextToFreeze(S, E1))
						continue;
					const float Score = -Field.Dist((E0 + E1) / 2) - 0.5f * avIn[0].size();
					if(Score <= Best.m_Score)
						continue;
					Best.m_Valid = true;
					Best.m_Score = Score;
					Best.m_aEnd[0] = E0;
					Best.m_aEnd[1] = E1;
					for(int k = 0; k < 2; k++)
					{
						Best.m_avIn[k] = avIn[k];
						Best.m_avPath[k] = avPath[k];
					}
				}
		}
Done:
	Best.m_Steps = Steps;
	return Best;
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanLeap(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_LEAP;
	int Steps = 0, FoundAt = 0;
	const CCollision *pCol = Base.m_pCollision;
	const vec2 aP0[2] = {Base.m_aTee[0].m_Core.m_Pos, Base.m_aTee[1].m_Core.m_Pos};
	const float aD0[2] = {Field.Dist(aP0[0]), Field.Dist(aP0[1])};
	const vec2 Mid = (aP0[0] + aP0[1]) / 2;
	// which way: where the goal is nearer, 6 tiles to the side
	const int Dir = Field.Dist(Mid + vec2(192, 0)) <= Field.Dist(Mid - vec2(192, 0)) ? 1 : -1;
	const int Front = (aP0[0].x - aP0[1].x) * Dir > 0 ? 0 : 1, Back = 1 - Front;
	// (from SimMapBots.LeapSearch with LS_RELAX, from four spots on Stronghold's ledge after the ceiling: 268-437 of
	// 9984 worked, spread over all of these, the hook or not; the most often: the one behind jumping 6 ticks before the
	// end and air-jumping 14-18 ticks later, the one in front air-jumping late)
	static const int s_aHook[][3] = {{0, 0, 0}, {1, 0, 0}, {1, 4, 4}, {1, 0, -4}, {1, 0, -8}, {1, 4, 0}, {1, 0, 4}, {1, 0, 8}, {1, 0, 16}}; // hook?, at, let go rel. to the jump
	static const int s_aEdgeB[] = {6, 0, 3};
	static const int s_aAirB[] = {14, 18, 10, 22, 26, 30, -1, 6};
	static const int s_aEdgeF[] = {0, 3, 6, 12};
	static const int s_aAirF[] = {30, 26, 22, 18, 14, 10, 6, -1};
	for(const auto &H : s_aHook)
		for(int EdgeB : s_aEdgeB)
			for(int AirB : s_aAirB)
				for(int EdgeF : s_aEdgeF)
					for(int AirF : s_aAirF)
					{
						if(Steps > MaxSteps || (Best.m_Valid && Steps > FoundAt + MaxSteps / 6))
							goto Done;
						CHookBotSim S = Base;
						std::vector<CNetObj_PlayerInput> avIn[2];
						std::vector<vec2> avPath[2];
						int PrevHook = S.m_aTee[Front].m_PrevInput.m_Hook;
						int aJump[2] = {-1, -1}, Rest = 0;
						bool Bad = false;
						for(int t = 0; t < 400 && !Bad && Rest < 2; t++)
						{
							CNetObj_PlayerInput aI[2] = {};
							for(int i = 0; i < 2; i++)
							{
								const auto &T = S.m_aTee[i];
								const int Edge = i == Back ? EdgeB : EdgeF, Air = i == Back ? AirB : AirF;
								aI[i].m_Direction = Dir;
								aI[i].m_TargetX = Dir;
								// jump when the floor ends within Edge+1 ticks at this speed
								if(aJump[i] < 0 && OnGroundAt(pCol, T.m_Core.m_Pos) && !OnGroundAt(pCol, T.m_Core.m_Pos + vec2(T.m_Core.m_Vel.x * (Edge + 1) + Dir * 4, 0)))
								{
									aI[i].m_Jump = 1;
									aJump[i] = t;
								}
								if(aJump[i] >= 0 && Air >= 0 && t == aJump[i] + Air)
									aI[i].m_Jump = 1;
							}
							if(H[0])
							{
								// the one in front hooks the one behind (it gets dragged faster than it runs)
								const auto &F = S.m_aTee[Front], &B = S.m_aTee[Back];
								Aim(aI[Front], B.m_Core.m_Pos - F.m_Core.m_Pos);
								const bool Want = t >= H[1] && (aJump[Back] < 0 || t < aJump[Back] + H[2]);
								aI[Front].m_Hook = HookBotHookInput(Want, PrevHook, F.m_Core.m_HookState);
								PrevHook = aI[Front].m_Hook;
							}
							for(int i = 0; i < 2; i++)
							{
								avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
								avIn[i].push_back(aI[i]);
							}
							S.Step(aI[0], aI[1]);
							Steps++;
							Bad = S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead;
							Rest = t > 10 && AtRest(S, 0) && AtRest(S, 1) ? Rest + 1 : 0;
						}
						if(Bad || Rest < 2 || aJump[0] < 0 || aJump[1] < 0)
							continue;
						const vec2 aE[2] = {S.m_aTee[0].m_Core.m_Pos, S.m_aTee[1].m_Core.m_Pos};
						const bool aIn[2] = {S.InFreeze(aE[0]), S.InFreeze(aE[1])};
						// at most one of us in the freeze, and then right where the other one can pull it out
						if((aIn[0] && aIn[1]) || ((aIn[0] || aIn[1]) && (distance(aE[0], aE[1]) > 8 * 32 || pCol->IntersectLine(aE[0], aE[1], nullptr, nullptr))))
							continue;
						const float aGain[2] = {aD0[0] - Field.Dist(aE[0]), aD0[1] - Field.Dist(aE[1])};
						if(aGain[0] + aGain[1] < 400 || std::min(aGain[0], aGain[1]) < 0)
							continue;
						float Score = aGain[0] + aGain[1] - 0.5f * avIn[0].size();
						for(int i = 0; i < 2; i++)
						{
							if(aIn[i])
								Score -= 300;
							else if(NextToFreeze(S, aE[i]))
								Score -= 100;
						}
						if(Score <= Best.m_Score)
							continue;
						if(!Best.m_Valid)
							FoundAt = Steps;
						Best.m_Valid = true;
						Best.m_Score = Score;
						Best.m_Up = aIn[0] ? 1 : 0; // (the one outside the freeze)
						for(int i = 0; i < 2; i++)
						{
							Best.m_aEnd[i] = aE[i];
							Best.m_avIn[i] = avIn[i];
							Best.m_avPath[i] = avPath[i];
						}
					}
Done:
	Best.m_Steps = Steps;
	return Best;
}

// can the free one get the one stuck in the freeze out from somewhere? Within 12 tiles with nothing solid between: yes
// (over a freeze floor the freed one comes down to it from the air: the hammerhit cycle of Stronghold's big room).
// Else a spot to stand on outside the freeze within 10 tiles of the stuck one with nothing solid between (where its
// hook reaches it from) that the free one can get to, through air or freeze, within 40 tiles (Stronghold's freeze
// column at x 302-316: rank 1 leaves one lying at its bottom for 7 s while the other comes round to the pillar beside its
// exit and drags it out; in the freeze pool between the corridor's blocks nowhere to stand is near enough)
// from Pos, getting on by myself: through the air any way, into freeze sideways or down and in it only down (straight
// or drifting): somewhere at least Gain closer to the goal within 30 tiles? (no: a closed pocket, e.g. the 3-wide gaps
// between Stronghold's freeze columns at x 440-454 y 244-252, which look near the goal through the freeze beside them)
static bool ProgressFrom(const CHookBotSim &S, const CHookBotGoalField &Field, vec2 Pos, float Gain)
{
	const CCollision *pCol = S.m_pCollision;
	const int R = 30, N = 2 * R + 1;
	const int fx = (int)(Pos.x / 32), fy = (int)(Pos.y / 32);
	const float Here = Field.Dist(Pos);
	std::vector<uint8_t> vSeen((size_t)N * N, 0);
	std::vector<int> vQueue = {R * N + R};
	vSeen[R * N + R] = 1;
	for(size_t q = 0; q < vQueue.size(); q++)
	{
		const int lx = vQueue[q] % N, ly = vQueue[q] / N;
		const vec2 C((lx - R + fx) * 32 + 16, (ly - R + fy) * 32 + 16);
		const bool InFrz = S.InFreeze(C);
		if(!InFrz && Field.Dist(C) < Here - Gain)
			return true;
		for(int d = 0; d < 6; d++)
		{
			static const int s_aDx[] = {1, -1, 0, 0, 1, -1}, s_aDy[] = {0, 0, 1, -1, 1, 1};
			const int nx = lx + s_aDx[d], ny = ly + s_aDy[d];
			if(nx < 0 || ny < 0 || nx >= N || ny >= N || vSeen[ny * N + nx])
				continue;
			const vec2 D((nx - R + fx) * 32 + 16, (ny - R + fy) * 32 + 16);
			if(pCol->CheckPoint(D) || S.TouchesDeath(D))
				continue;
			const bool ToFrz = S.InFreeze(D);
			if(InFrz ? s_aDy[d] != 1 : (d >= 4 || (ToFrz && s_aDy[d] < 0)))
				continue;
			vSeen[ny * N + nx] = 1;
			vQueue.push_back(ny * N + nx);
		}
	}
	return false;
}

// at least Min tiles of open air (no freeze, nothing solid) connected to tile T
static bool OpenAround(const CHookBotSim &S, ivec2 T, int Min)
{
	std::vector<ivec2> vQueue = {T};
	for(size_t q = 0; q < vQueue.size() && (int)vQueue.size() < Min; q++)
		for(int d = 0; d < 4; d++)
		{
			const ivec2 N(vQueue[q].x + (d == 0) - (d == 1), vQueue[q].y + (d == 2) - (d == 3));
			const vec2 C(N.x * 32 + 16, N.y * 32 + 16);
			if(S.m_pCollision->CheckPoint(C) || S.InFreeze(C) || S.TouchesDeath(C) || std::find(vQueue.begin(), vQueue.end(), N) != vQueue.end())
				continue;
			vQueue.push_back(N);
		}
	return (int)vQueue.size() >= Min;
}

static bool CanRescue(const CHookBotSim &S, vec2 Free, vec2 Stuck)
{
	const CCollision *pCol = S.m_pCollision;
	if(distance(Free, Stuck) <= 12 * 32 && !pCol->IntersectLine(Free, Stuck, nullptr, nullptr))
		return true;
	const int R = 40, N = 2 * R + 1;
	const int fx = (int)(Free.x / 32), fy = (int)(Free.y / 32), sx = (int)(Stuck.x / 32), sy = (int)(Stuck.y / 32);
	if(std::abs(sx - fx) > R - 10 || std::abs(sy - fy) > R - 10)
		return false;
	std::vector<uint8_t> vSeen((size_t)N * N, 0);
	std::vector<int> vQueue = {R * N + R};
	vSeen[R * N + R] = 1;
	for(size_t q = 0; q < vQueue.size(); q++)
	{
		const int lx = vQueue[q] % N, ly = vQueue[q] / N, x = lx - R + fx, y = ly - R + fy;
		const vec2 C(x * 32 + 16, y * 32 + 16);
		// a spot to stand on near it, its hook's line clear, and not in a closed pocket (open air around it, 40 tiles at
		// least: a 3x6 pocket under a block in Stronghold's freeze mass at x 446-448 is in hook reach of the column beside
		// it, and a rescue into it ends with both of us there for good)
		// (or from up to 3.5 tiles above it, jumping: rank 1 pulls the one lying on Stronghold's freeze floor above the
		// corridor after the shaft through the freeze strip so; from the corridor's floor the line clips the block's corner)
		if(std::abs(x - sx) <= 10 && std::abs(y - sy) <= 10 && !S.InFreeze(C) && pCol->CheckPoint(C.x, C.y + 32) && distance(C, Stuck) <= 10 * 32 && OpenAround(S, ivec2(x, y), 40))
		{
			if(!pCol->IntersectLine(C, Stuck, nullptr, nullptr))
				return true;
			for(float Up : {48.0f, 80.0f, 112.0f})
			{
				const vec2 J = C - vec2(0, Up);
				if(pCol->IntersectLine(C, J, nullptr, nullptr) || S.InFreeze(J))
					break;
				if(distance(J, Stuck) <= 10 * 32 && !pCol->IntersectLine(J, Stuck, nullptr, nullptr))
					return true;
			}
		}
		// through the air any way; into freeze sideways or down (jumping up into it only drops me back), and in it only
		// down, straight or drifting (frozen, I fall). Any way through freeze reached a 3-wide pocket under a block in
		// the freeze mass at Stronghold x 446-448 y 235-240, from where the partner lying on the freeze column beside
		// it looked rescuable, and the joint move that left it there was taken
		const bool InFrz = S.InFreeze(C);
		for(int d = 0; d < 6; d++)
		{
			static const int s_aDx[] = {1, -1, 0, 0, 1, -1}, s_aDy[] = {0, 0, 1, -1, 1, 1};
			const int nx = lx + s_aDx[d], ny = ly + s_aDy[d];
			if(nx < 0 || ny < 0 || nx >= N || ny >= N || vSeen[ny * N + nx])
				continue;
			const vec2 D((nx - R + fx) * 32 + 16, (ny - R + fy) * 32 + 16);
			if(pCol->CheckPoint(D) || S.TouchesDeath(D))
				continue;
			const bool ToFrz = S.InFreeze(D);
			if(InFrz ? s_aDy[d] != 1 : (d >= 4 || (ToFrz && s_aDy[d] < 0)))
				continue;
			vSeen[ny * N + nx] = 1;
			vQueue.push_back(ny * N + nx);
		}
	}
	return false;
}

// can a tee at From get to To by itself (within 3 tiles of it, outside the freeze), or at least come by within 10 tiles
// of it with nothing solid between (where a free partner at To hooks it out of its fall)? Coarser than a search, but by
// what a tee can do: through the air sideways and down any way, up only 9 tiles above where it last stood (a jump and
// the air jump) unless something hookable is within 9 tiles; into freeze sideways or down, then frozen: falling, and
// what's left of its speed carries it sideways only in the first 4 rows, 3 tiles at most when it ran into the freeze
// from the ground, 1 when it fell in (vx decays by 0.95 a tick, and a fall leaves little time); out of the freeze it
// thaws where it lands, never in a freeze floor. Stronghold's zig-zag below the bottom room is all unhookable: a
// 3-wide freeze column beside a shaft takes more speed than a tee falling down the shaft has, and a fall-catch that
// left one of us above it and the other below was taken, the one above stuck for good
static bool CanReach(const CHookBotSim &S, vec2 From, vec2 To)
{
	const CCollision *pCol = S.m_pCollision;
	const int R = 40, N = 2 * R + 1;
	const int fx = (int)(From.x / 32), fy = (int)(From.y / 32), tx = (int)(To.x / 32), ty = (int)(To.y / 32);
	if(std::abs(tx - fx) > R - 4 || std::abs(ty - fy) > R - 4)
		return false;
	enum
	{
		UP = 9,
		RUN_DRIFT = 3,
		FALL_DRIFT = 1,
		DRIFT_ROWS = 4,
		CATCH = 10,
	};
	// something hookable within 9 tiles (prefix sums over the window and its margin)
	const int M = R + 9, NM = 2 * M + 1;
	std::vector<int> vHookSum((size_t)(NM + 1) * (NM + 1), 0);
	for(int y = 0; y < NM; y++)
		for(int x = 0; x < NM; x++)
		{
			const bool Hookable = pCol->GetTile((x - M + fx) * 32 + 16, (y - M + fy) * 32 + 16) == TILE_SOLID;
			vHookSum[(y + 1) * (NM + 1) + x + 1] = Hookable + vHookSum[y * (NM + 1) + x + 1] + vHookSum[(y + 1) * (NM + 1) + x] - vHookSum[y * (NM + 1) + x];
		}
	auto HookNear = [&](int lx, int ly) {
		const int x0 = lx, y0 = ly, x1 = lx + 19, y1 = ly + 19;
		return vHookSum[y1 * (NM + 1) + x1] - vHookSum[y0 * (NM + 1) + x1] - vHookSum[y1 * (NM + 1) + x0] + vHookSum[y0 * (NM + 1) + x0] > 0;
	};
	// per tile: 0 air, 1 solid, 2 freeze, 3 death
	std::vector<int8_t> vKind((size_t)N * N);
	for(int y = 0; y < N; y++)
		for(int x = 0; x < N; x++)
		{
			const vec2 C((x - R + fx) * 32 + 16, (y - R + fy) * 32 + 16);
			vKind[y * N + x] = pCol->CheckPoint(C) ? 1 : S.TouchesDeath(C) ? 3 : S.InFreeze(C) ? 2 : 0;
		}
	auto Kind = [&](int x, int y) { return x < 0 || y < 0 || x >= N || y >= N ? 1 : vKind[y * N + x]; };
	const bool CanCatch = !S.InFreeze(To);
	// free: the most climb left seen per tile (-1: not yet); frozen: a bit per (rows fallen since, drift left), one for
	// past the first rows
	std::vector<int8_t> vFree((size_t)N * N, -1);
	std::vector<uint32_t> vFrozen((size_t)N * N, 0);
	struct SState
	{
		int m_X, m_Y;
		int m_Left; // free: climb left; frozen: drift left
		int m_Rows; // frozen: rows fallen since it froze
		bool m_Frozen;
	};
	std::vector<SState> vQueue;
	// where I come to lie in the freeze (from there, the other one may come and get me out)
	std::vector<ivec2> vStuck;
	auto Push = [&](int x, int y, bool Frozen, int Left, int Rows) {
		const int k = Kind(x, y);
		if(k == 1 || k == 3)
			return;
		if(k == 2 && !Frozen)
		{
			Frozen = true;
			Rows = 0;
		}
		// frozen and landed outside the freeze: I thaw there
		if(Frozen && k == 0 && Kind(x, y + 1) == 1)
			Frozen = false;
		if(!Frozen)
		{
			if(Kind(x, y + 1) == 1 || HookNear(x, y))
				Left = UP;
			if(vFree[y * N + x] >= Left)
				return;
			vFree[y * N + x] = Left;
		}
		else
		{
			if(Rows >= DRIFT_ROWS)
				Left = 0, Rows = DRIFT_ROWS;
			const uint32_t Bit = 1u << (Rows * 4 + Left);
			if(vFrozen[y * N + x] & Bit)
				return;
			vFrozen[y * N + x] |= Bit;
		}
		vQueue.push_back({x, y, Left, Rows, Frozen});
	};
	Push(R, R, false, UP, 0);
	if(Kind(R, R) == 2)
	{
		vQueue.clear();
		vFrozen[R * N + R] = 0;
		Push(R, R, true, 0, DRIFT_ROWS);
	}
	for(size_t q = 0; q < vQueue.size(); q++)
	{
		const SState St = vQueue[q];
		const int gx = St.m_X - R + fx, gy = St.m_Y - R + fy;
		if(!St.m_Frozen && std::abs(gx - tx) <= 3 && std::abs(gy - ty) <= 3)
			return true;
		if(CanCatch && std::abs(gx - tx) <= CATCH && std::abs(gy - ty) <= CATCH)
		{
			const vec2 C(gx * 32 + 16, gy * 32 + 16);
			if(distance(C, To) <= CATCH * 32 && !pCol->IntersectLine(C, To, nullptr, nullptr))
				return true;
		}
		if(St.m_Frozen)
		{
			if(Kind(St.m_X, St.m_Y) == 2 && Kind(St.m_X, St.m_Y + 1) == 1 && vStuck.size() < 12 &&
				std::none_of(vStuck.begin(), vStuck.end(), [&](ivec2 P) { return std::abs(P.x - gx) <= 2 && std::abs(P.y - gy) <= 2; }))
				vStuck.emplace_back(gx, gy);
			Push(St.m_X, St.m_Y + 1, true, St.m_Left, St.m_Rows + 1);
			if(St.m_Left > 0)
				for(int Dx : {-1, 1})
					if(Kind(St.m_X + Dx, St.m_Y) != 1) // not round a corner
						Push(St.m_X + Dx, St.m_Y + 1, true, St.m_Left - 1, St.m_Rows + 1);
			continue;
		}
		// into freeze sideways from where I stand: with a run-up; any other way in: falling
		for(int Dx : {-1, 1})
		{
			const bool Run = Kind(St.m_X, St.m_Y + 1) == 1;
			Push(St.m_X + Dx, St.m_Y, false, Kind(St.m_X + Dx, St.m_Y) == 2 ? (Run ? RUN_DRIFT : FALL_DRIFT) : St.m_Left, 0);
		}
		Push(St.m_X, St.m_Y + 1, false, Kind(St.m_X, St.m_Y + 1) == 2 ? FALL_DRIFT : St.m_Left, 0);
		if(St.m_Left > 0 && Kind(St.m_X, St.m_Y - 1) == 0)
			Push(St.m_X, St.m_Y - 1, false, St.m_Left - 1, 0);
	}
	// or I come to lie in the freeze somewhere the other one can get me out from (a sacrifice: Stronghold after the pit,
	// one of us caught onto the ledge in the room below, the other dives down the freeze shaft beside it and is dragged
	// out at its foot)
	if(CanCatch)
		for(ivec2 P : vStuck)
			if(CanRescue(S, To, vec2(P.x * 32 + 16, P.y * 32 + 16)))
				return true;
	return false;
}

bool CHookBotBrain::TestCanReach(const CHookBotSim &S, vec2 From, vec2 To) { return CanReach(S, From, To); }
bool CHookBotBrain::TestCanRescue(const CHookBotSim &S, vec2 Free, vec2 Stuck) { return CanRescue(S, Free, Stuck); }

// where a team move's end leaves us: both at rest, at most one of us in the freeze and then where the other can pull it
// out (within 8 tiles, nothing in between); the value: what we gained towards the goal, minus 300 for one in the freeze,
// 100 for one right next to it, 400 for ending more than 6 tiles apart with freeze between us (one running on ahead won
// before, and the long way back to help the other ended with both in Stronghold's freeze pool). False if it's no good
// (also if one of us loses more than 2 tiles)
static bool TeamEndValue(const CHookBotSim &S, const CHookBotGoalField &Field, const float aD0[2], float *pValue, float MinGain = -64)
{
	const vec2 aE[2] = {S.m_aTee[0].m_Core.m_Pos, S.m_aTee[1].m_Core.m_Pos};
	const bool aIn[2] = {S.InFreeze(aE[0]), S.InFreeze(aE[1])};
	if(S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead || (aIn[0] && aIn[1]))
		return false;
	if((aIn[0] || aIn[1]) && !CanRescue(S, aIn[0] ? aE[1] : aE[0], aIn[0] ? aE[0] : aE[1]))
		return false;
	float aGain[2] = {aD0[0] - Field.Dist(aE[0]), aD0[1] - Field.Dist(aE[1])};
	// (one lying in the freeze counts only as far on as the other, who gets it out towards itself: in the chamber after
	// Stronghold's unhookable room, a climb that left one frozen on the pillar in the freeze block, the other back on the
	// floor before it, won over both of us standing there, and every rescue brought it back to the floor)
	if(!getenv("HH_OLDRESCUEGAIN"))
		for(int i = 0; i < 2; i++)
			if(aIn[i])
				aGain[i] = std::min(aGain[i], aD0[i] - Field.Dist(aE[1 - i]));
	if(std::min(aGain[0], aGain[1]) < MinGain)
		return false;
	// both out of the freeze but apart: one of us has to be able to get to the other (not one above a freeze column it
	// can't get through and the other below it, each on its own for good)
	if(!aIn[0] && !aIn[1] && distance(aE[0], aE[1]) > 3 * 32 && !CanReach(S, aE[0], aE[1]) && !CanReach(S, aE[1], aE[0]))
		return false;
	float Value = aGain[0] + aGain[1];
	if(distance(aE[0], aE[1]) > 6 * 32 && FreezeOnLine(S, aE[0], aE[1]))
		Value -= 400;
	for(int i = 0; i < 2; i++)
	{
		if(aIn[i])
			Value -= 300;
		else if(NextToFreeze(S, aE[i]))
			Value -= 100;
	}
	*pValue = Value;
	return true;
}

// the columns within 12 tiles of From where, under From's floor, freeze comes first (within 3 tiles; at most 6 tiles of
// it) with open air at least 8 tiles deep under it, and the goal is at least 300 closer below it: a freeze floor to
// drop through. Nearest first
static std::vector<float> DropColumns(const CHookBotSim &S, const CHookBotGoalField &Field, vec2 From)
{
	std::vector<float> vCols;
	const int fx = (int)(From.x / 32), fy = (int)(From.y / 32) + 1;
	const float Here = Field.Dist(From);
	auto C = [](int x, int y) { return vec2(x * 32 + 16, y * 32 + 16); };
	for(int d = 0; d <= 12; d++)
		for(int Side : {-1, 1})
		{
			if(d == 0 && Side == 1)
				continue;
			const int x = fx + d * Side;
			int y = fy;
			while(y < fy + 3 && !S.m_pCollision->CheckPoint(C(x, y)) && !S.InFreeze(C(x, y)))
				y++;
			if(!S.InFreeze(C(x, y)))
				continue;
			int Band = 0;
			while(S.InFreeze(C(x, y)) && Band <= 6)
			{
				y++;
				Band++;
			}
			if(Band > 6)
				continue;
			const int Below = y;
			int Open = 0;
			while(Open < 8 && !S.m_pCollision->CheckPoint(C(x, y)) && !S.InFreeze(C(x, y)))
			{
				y++;
				Open++;
			}
			if(Open >= 8 && Field.Dist(C(x, Below)) < Here - 300)
				vCols.push_back(x * 32 + 16);
		}
	return vCols;
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanDrop(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_DROP;
	const int64_t Steps0 = ts_HookBotSimSteps;
	auto Used = [&]() { return (int)(ts_HookBotSimSteps - Steps0); };
	const CCollision *pCol = Base.m_pCollision;
	const vec2 Mid = (Base.m_aTee[0].m_Core.m_Pos + Base.m_aTee[1].m_Core.m_Pos) / 2;
	const std::vector<float> vAll = DropColumns(Base, Field, Mid);
	if(vAll.empty())
		return Best;
	const float aD0[2] = {Field.Dist(Base.m_aTee[0].m_Core.m_Pos), Field.Dist(Base.m_aTee[1].m_Core.m_Pos)};
	// the one nearer the freeze drops (the other one walking past it pushed it into the pit)
	const int P = absolute(Base.m_aTee[0].m_Core.m_Pos.x - vAll[0]) <= absolute(Base.m_aTee[1].m_Core.m_Pos.x - vAll[0]) ? 0 : 1, T = 1 - P;
	const vec2 T0 = Base.m_aTee[T].m_Core.m_Pos;
	const int Side = vAll[0] > T0.x ? 1 : -1;
	// where it falls in: 2 to 8 tiles out from the edge (from the edge, straight down, my hook's line runs through the
	// floor's corner), and the freeze band's bottom there
	float EdgeX = T0.x;
	for(int i = 0; i < 400; i++)
	{
		const vec2 Next(EdgeX + Side * 2, T0.y);
		if(!OnGroundAt(pCol, Next) || Base.InFreeze(Next) || pCol->TestBox(Next, vec2(28, 28)))
			break;
		EdgeX = Next.x;
	}
	std::vector<float> vCols;
	for(float X : vAll)
		if((X - EdgeX) * Side >= 2 * 32 && (X - EdgeX) * Side <= 8 * 32 && vCols.size() < 3 && (vCols.empty() || absolute(X - vCols.back()) >= 64))
			vCols.push_back(X);
	if(vCols.empty())
		return Best;
	int BandBottom = (int)(T0.y / 32) + 1;
	while(BandBottom < (int)(T0.y / 32) + 12 && !Base.InFreeze(vec2(vCols[0], BandBottom * 32 + 16)))
		BandBottom++;
	while(BandBottom < (int)(T0.y / 32) + 12 && Base.InFreeze(vec2(vCols[0], BandBottom * 32 + 16)))
		BandBottom++;
	const float Below = BandBottom * 32; // px: the top of the first open row under the freeze
	// my hold, a controller: hook it while it's going to sink below the band I keep it in, let go while it's above, so it
	// hangs right under the freeze while its freeze runs out (pulled back into the freeze more than a second after it
	// froze, the freeze would start over). Rank 1 holds it at rows 64-67, 1-3 tiles under the band, for about 1.5 s
	static const int s_aLo[] = {24, 56, 88}; // px under the freeze, the band's top; its bottom 48 px further
	static const int s_aEdgeBack[] = {10, 4, 20}; // px back from the very edge (hanging over it, the hook's pull takes me in)
	static const int s_aRelease[] = {110, 125, 95, 140}; // ticks after it froze: let go (it thaws 150 ticks after)
	static const int s_aDropDelay[] = {0, 10, 20};
	struct SCand
	{
		CHookBotSim m_S;
		std::vector<CNetObj_PlayerInput> m_avIn[2];
		std::vector<vec2> m_avPath[2];
		int m_aPrevHook[2];
		int m_P;
		int m_Margin;
	};
	std::vector<SCand> vCand;
	auto Input = [](int Dir, vec2 AimDir) {
		CNetObj_PlayerInput I = {};
		I.m_Direction = Dir;
		Aim(I, AimDir);
		return I;
	};
	for(float XP : vCols)
		for(int JumpP = 0; JumpP <= 1; JumpP++)
			for(int Lo : s_aLo)
				for(int EdgeBack : s_aEdgeBack)
					for(int Brace = 0; Brace <= 1; Brace++)
						for(int Release : s_aRelease)
							for(int DropDelay : s_aDropDelay)
							{
								if(Used() > MaxSteps / 2)
									goto Rescues;
								const float StandX = EdgeX - Side * EdgeBack;
								SCand C;
								C.m_S = Base;
								C.m_P = P;
								int PrevHook = Base.m_aTee[T].m_PrevInput.m_Hook, FrozenAt = -1;
								bool Bad = false, Done = false;
								for(int t = 0; t < 400 && !Bad && !Done; t++)
								{
									CHookBotSim &S = C.m_S;
									const auto &Pt = S.m_aTee[P], &Tt = S.m_aTee[T];
									if(FrozenAt < 0 && Pt.m_FreezeTime > 0)
										FrozenAt = t;
									// done here: it has thawed in the air, and I'm through the freeze too, frozen, below
									if(FrozenAt >= 0 && Pt.m_FreezeTime == 0 && Tt.m_FreezeTime > 0 && !OnGroundAt(pCol, Pt.m_Core.m_Pos) && Tt.m_Core.m_Pos.y > Below)
									{
										Done = true;
										break;
									}
									CNetObj_PlayerInput aI[2];
									aI[P] = Input(SteerTo(XP, Pt.m_Core.m_Pos.x, Pt.m_Core.m_Vel.x), Tt.m_Core.m_Pos - Pt.m_Core.m_Pos);
									// (maybe jumping in, at the end of the floor)
									if(JumpP && FrozenAt < 0 && OnGroundAt(pCol, Pt.m_Core.m_Pos) && !OnGroundAt(pCol, Pt.m_Core.m_Pos + vec2(Pt.m_Core.m_Vel.x * 2, 0)))
										aI[P].m_Jump = 1;
									const bool Holding = FrozenAt >= 0 && t < FrozenAt + Release;
									const float Predicted = Pt.m_Core.m_Pos.y + Pt.m_Core.m_Vel.y * 3;
									const bool Want = Holding && Predicted > Below + Lo + 48;
									const bool Passed = Pt.m_Core.m_Pos.y > Tt.m_Core.m_Pos.y + 16 || (Pt.m_Core.m_Pos.x - StandX) * Side > 40;
									int Dir = 0;
									if(FrozenAt >= 0 && t >= FrozenAt + Release + DropDelay)
										Dir = SteerTo(XP, Tt.m_Core.m_Pos.x, Tt.m_Core.m_Vel.x); // after it: into the pit
									else if(Passed)
										Dir = Brace && Tt.m_Core.m_HookState == HOOK_GRABBED ? -Side : SteerTo(StandX, Tt.m_Core.m_Pos.x, Tt.m_Core.m_Vel.x);
									const vec2 AimAt = Pt.m_Core.m_Pos + Pt.m_Core.m_Vel * (distance(Pt.m_Core.m_Pos, Tt.m_Core.m_Pos) / 80.0f);
									aI[T] = Input(Dir, AimAt - Tt.m_Core.m_Pos);
									aI[T].m_Hook = HookBotHookInput(Want, PrevHook, Tt.m_Core.m_HookState);
									PrevHook = aI[T].m_Hook;
									for(int i = 0; i < 2; i++)
									{
										C.m_avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
										C.m_avIn[i].push_back(aI[i]);
									}
									S.Step(aI[0], aI[1]);
									// (it landed frozen: too late; I fell in before it froze: no)
									Bad = S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead || (Pt.m_FreezeTime > 0 && FrozenAt >= 0 && t > FrozenAt + 5 && OnGroundAt(pCol, Pt.m_Core.m_Pos) && Pt.m_Core.m_Vel.y >= 0) ||
									      (Tt.m_FreezeTime > 0 && (FrozenAt < 0 || t < FrozenAt + Release));
								}
								if(ms_DebugTeam && DropDelay == 0 && EdgeBack == 10 && Release == 110 && !Brace)
									printf("drop try: XP %.1f jump %d lo %d: frozen at %d, %s; P %.1f %.1f frz %d, T %.1f %.1f frz %d hook %d\n", XP / 32, JumpP, Lo, FrozenAt, Done ? "done" : Bad ? "bad" : "timeout",
										C.m_S.m_aTee[P].m_Core.m_Pos.x / 32, C.m_S.m_aTee[P].m_Core.m_Pos.y / 32, C.m_S.m_aTee[P].m_FreezeTime, C.m_S.m_aTee[T].m_Core.m_Pos.x / 32, C.m_S.m_aTee[T].m_Core.m_Pos.y / 32,
										C.m_S.m_aTee[T].m_FreezeTime, C.m_S.m_aTee[T].m_Core.m_HookState);
								if(!Done)
									continue;
								// how long it stays in the air doing nothing: the time it has to get me out
								CHookBotSim F = C.m_S;
								int Margin = 0;
								while(Margin < 100 && !OnGroundAt(pCol, F.m_aTee[P].m_Core.m_Pos))
								{
									F.Step(CNetObj_PlayerInput{}, CNetObj_PlayerInput{}, P);
									Margin++;
								}
								if(Margin < 15 || distance(C.m_S.m_aTee[P].m_Core.m_Pos, C.m_S.m_aTee[T].m_Core.m_Pos) > 380)
									continue;
								C.m_aPrevHook[P] = 0;
								C.m_aPrevHook[T] = PrevHook;
								C.m_Margin = Margin;
								vCand.push_back(std::move(C));
							}
Rescues:
	std::stable_sort(vCand.begin(), vCand.end(), [](const SCand &a, const SCand &b) { return a.m_Margin > b.m_Margin; });
	if(ms_DebugTeam)
		printf("drop: %d columns, %d candidates (best margin %d), %d ticks\n", (int)vCols.size(), (int)vCand.size(), vCand.empty() ? -1 : vCand[0].m_Margin, Used());
	for(int c = 0; c < (int)vCand.size() && c < 8 && Used() < MaxSteps; c++)
	{
		SCand &C = vCand[c];
		// its rescue (as tee 0 of a sim of its own), on a budget of simulated ticks: the other brain has to find the same
		CHookBotSim R = C.m_S;
		if(P == 1)
			R.Swap();
		int Cursor = 0, HookCursor = 0;
		SPlan Res;
		{
			CStepBudget Budget(std::min(100000, MaxSteps - Used()));
			Res = PlanRescue(R, C.m_aPrevHook[P], &Cursor, SPlan(), 0, &Field, 0, true, &HookCursor);
		}
		if(ms_DebugTeam)
			printf("drop: candidate %d (margin %d): rescue %s score %.0f\n", c, C.m_Margin, Res.m_Valid ? "found" : "none", Res.m_Score);
		if(!Res.m_Valid)
			continue;
		// play it: the rescue's inputs for it, nothing for me (frozen); then its fall after, 30 ticks
		CHookBotSim &S = C.m_S;
		int PrevHook = C.m_aPrevHook[P];
		bool Jumped = false;
		for(int t = 0; t <= Res.m_FireAt + 30; t++)
		{
			const auto &Pt = S.m_aTee[P], &Tt = S.m_aTee[T];
			CNetObj_PlayerInput aI[2] = {};
			aI[T].m_TargetX = 1;
			if(t < Res.m_FireAt)
			{
				aI[P] = RescueInput(Res, t, PrevHook, Pt.m_Core.m_HookState, Tt.m_Core.m_Pos - Pt.m_Core.m_Pos, Pt.m_Core.m_Pos, Pt.m_Core.m_Vel);
				if(t == 0 && Res.m_PreHammer)
					aI[P].m_Fire = 1;
			}
			else
			{
				Aim(aI[P], Tt.m_Core.m_Pos - Pt.m_Core.m_Pos);
				aI[P].m_Direction = Res.m_PostDir;
				if(t == Res.m_FireAt && !Res.m_HookOnly)
					aI[P].m_Fire = 1;
				if(Res.m_PostJump && !Jumped && !(Pt.m_Core.m_Jumped & 2) && Pt.m_Core.m_Vel.y > 0 && t > Res.m_FireAt)
					aI[P].m_Jump = Jumped = true;
			}
			if(aI[P].m_Fire && Pt.m_Reload == 0 && HammerReaches(Pt.m_Core.m_Pos, Tt.m_Core.m_Pos))
			{
				HammerPush(S.m_aTee[T].m_Core, Pt.m_Core.m_Pos);
				S.m_aTee[T].m_FreezeTime = 0;
				S.m_aTee[P].m_Reload = 16;
			}
			PrevHook = aI[P].m_Hook;
			for(int i = 0; i < 2; i++)
			{
				C.m_avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
				C.m_avIn[i].push_back(aI[i]);
			}
			S.Step(aI[0], aI[1]);
		}
		if(S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead || S.m_aTee[P].m_FreezeTime > 0)
			continue;
		Best.m_Valid = true;
		Best.m_Score = Res.m_Score + aD0[0] - Field.Dist(S.m_aTee[0].m_Core.m_Pos) + aD0[1] - Field.Dist(S.m_aTee[1].m_Core.m_Pos);
		Best.m_Up = P;
		for(int i = 0; i < 2; i++)
		{
			Best.m_aEnd[i] = S.m_aTee[i].m_Core.m_Pos;
			Best.m_avIn[i] = C.m_avIn[i];
			Best.m_avPath[i] = C.m_avPath[i];
		}
		break;
	}
	Best.m_Steps = Used();
	return Best;
}

CHookBotBrain::SJointParams CHookBotBrain::ms_Joint;
bool CHookBotBrain::ms_DebugTeam = false;

// ---- a fall caught on the hook ----

// a pseudofly for plans: what PseudoDrive and PseudoHammer do, with their state kept by the plan (each of us keeps its
// own in the brain, and the other's isn't known: from the state alone both of us plan the same fly)
struct SFlyState
{
	int m_Driver = 0; // the upper one drives; standing level, this one (it jumps first)
	bool m_Free = false;
	int m_FreeTicks = 0;
	int m_aPrevHook[2] = {0, 0};
	int m_aPrevFire[2] = {0, 0};
	int m_aPrevJump[2] = {0, 0};
};

// one tick of it, the driver steering towards DriveDir: the inputs (m_Fire 1: a hammer), the hammer applied, stepped
static void FlyStep(CHookBotSim &S, SFlyState &F, const CHookBotBrain::SPseudoParams &P, int DriveDir, CNetObj_PlayerInput aIn[2])
{
	const CCollision *pCol = S.m_pCollision;
	const vec2 P0 = S.m_aTee[0].m_Core.m_Pos, P1 = S.m_aTee[1].m_Core.m_Pos;
	if(P0.y < P1.y - 1)
		F.m_Driver = 0;
	else if(P1.y < P0.y - 1)
		F.m_Driver = 1;
	const int d = F.m_Driver, h = 1 - d;
	auto &D = S.m_aTee[d], &H = S.m_aTee[h];
	const vec2 B = D.m_Core.m_Pos, U = H.m_Core.m_Pos, VB = D.m_Core.m_Vel, VU = H.m_Core.m_Vel;
	const float Dist = distance(B, U), NextDist = distance(B + VB, U + VU);
	const bool DG = OnGroundAt(pCol, B), HG = OnGroundAt(pCol, U);
	CNetObj_PlayerInput &ID = aIn[d], &IH = aIn[h];
	ID = {};
	IH = {};
	// the driver: hook the hammerer while the rope is long, let go when it comes close, hook again after the kick
	const bool Grabbed = D.m_Core.HookedPlayer() == H.m_Core.m_Id && D.m_Core.m_HookState == HOOK_GRABBED;
	if(!F.m_Free && Grabbed && Dist < P.m_ReleaseDist && NextDist < Dist)
	{
		F.m_Free = true;
		F.m_FreeTicks = 0;
	}
	else if(F.m_Free && (++F.m_FreeTicks > P.m_MaxFree || (Dist > P.m_RehookDist && NextDist > Dist)))
		F.m_Free = false;
	const bool Want = !F.m_Free && U.y > B.y + 10 && !DG && Dist < 360 && D.m_FreezeTime == 0;
	ID.m_Hook = HookBotHookInput(Want, F.m_aPrevHook[d], D.m_Core.m_HookState);
	Aim(ID, U - B);
	ID.m_Direction = DriveDir;
	if(VB.x * DriveDir > P.m_MaxVx || (B.x - U.x) * DriveDir > P.m_MaxBehind)
		ID.m_Direction = 0;
	if(!DG && !(D.m_Core.m_Jumped & 2) && VB.y > P.m_AirJumpVy)
		ID.m_Jump = 1;
	if(DG && HG)
	{
		const float Dx = U.x - B.x;
		if(absolute(Dx) > 80)
			ID.m_Direction = Dx > 0 ? 1 : -1;
		else if(!F.m_aPrevJump[d] && absolute(U.y - B.y) < 20)
			ID.m_Jump = 1;
		else
			ID.m_Direction = 0;
	}
	// the hammerer: under the driver, hammering it up when it's in reach and stops coming closer (or is about to bump into
	// me or leave my reach)
	const bool Ready = H.m_Reload == 0 && !F.m_aPrevFire[h] && H.m_FreezeTime == 0;
	const bool Now = P.m_CloseOnly ? (NextDist < 38 || Dist < P.m_FireDist) : (NextDist >= Dist || NextDist < 38 || Dist < P.m_FireDist || NextDist >= 60);
	IH.m_Fire = Ready && Dist < 60 && U.y - B.y > P.m_MinDy && Now && !pCol->IntersectLine(U, B, nullptr, nullptr);
	const float Err = B.x - U.x + (VB.x - VU.x) * P.m_Lead;
	IH.m_Direction = Err > P.m_Dead ? 1 : Err < -P.m_Dead ? -1 : 0;
	Aim(IH, B - U);
	for(int i = 0; i < 2; i++)
	{
		F.m_aPrevHook[i] = aIn[i].m_Hook;
		F.m_aPrevFire[i] = aIn[i].m_Fire;
		F.m_aPrevJump[i] = aIn[i].m_Jump;
	}
	if(IH.m_Fire && HammerReaches(U, B))
	{
		HammerPush(D.m_Core, U);
		D.m_FreezeTime = 0;
		H.m_Reload = 16;
	}
	S.Step(aIn[0], aIn[1]);
}

// a small open-air distance grid (tiles, 4 ways) to a target tile, within Radius tiles of a centre: steering a fly in a
// plan (a full goal field takes too long to build for every candidate)
struct SAirGrid
{
	int m_X0 = 0, m_Y0 = 0, m_N = 0;
	std::vector<int> m_vDist;
	void Build(const CHookBotSim &S, ivec2 Center, ivec2 Target, int Radius)
	{
		m_N = 2 * Radius + 1;
		m_X0 = Center.x - Radius;
		m_Y0 = Center.y - Radius;
		m_vDist.assign((size_t)m_N * m_N, 1 << 30);
		auto Open = [&](int x, int y) {
			const vec2 C(x * 32 + 16, y * 32 + 16);
			return !S.m_pCollision->CheckPoint(C) && !S.InFreeze(C) && !S.TouchesDeath(C);
		};
		const int tx = Target.x - m_X0, ty = Target.y - m_Y0;
		if(tx < 0 || ty < 0 || tx >= m_N || ty >= m_N || !Open(Target.x, Target.y))
			return;
		std::vector<int> vQueue = {ty * m_N + tx};
		m_vDist[ty * m_N + tx] = 0;
		for(size_t q = 0; q < vQueue.size(); q++)
		{
			const int lx = vQueue[q] % m_N, ly = vQueue[q] / m_N;
			for(int d = 0; d < 4; d++)
			{
				const int nx = lx + (d == 0) - (d == 1), ny = ly + (d == 2) - (d == 3);
				if(nx < 0 || ny < 0 || nx >= m_N || ny >= m_N || m_vDist[ny * m_N + nx] < (1 << 30) || !Open(nx + m_X0, ny + m_Y0))
					continue;
				m_vDist[ny * m_N + nx] = m_vDist[ly * m_N + lx] + 1;
				vQueue.push_back(ny * m_N + nx);
			}
		}
	}
	int Dist(vec2 Pos) const
	{
		const int x = (int)std::floor(Pos.x / 32) - m_X0, y = (int)std::floor(Pos.y / 32) - m_Y0;
		return x < 0 || y < 0 || x >= m_N || y >= m_N ? 1 << 30 : m_vDist[y * m_N + x];
	}
	// which way the driver steers: the side (looking a bit up) that's closer
	int Dir(vec2 B) const
	{
		const int L = Dist(B + vec2(-48, -32)), R = Dist(B + vec2(48, -32));
		return L < R ? -1 : R < L ? 1 : 0;
	}
};

// where one of us can let itself fall into freeze from above: runs of open tiles right above freeze within Radius tiles
// of From that open air connects to From, the nearest (through the air) first; each as the tile 2 above its middle
static std::vector<ivec2> FreezeMouths(const CHookBotSim &S, vec2 From, int Radius, int Max)
{
	const ivec2 F((int)std::floor(From.x / 32), (int)std::floor(From.y / 32));
	SAirGrid Here;
	Here.Build(S, F, F, Radius);
	auto Open = [&](int x, int y) {
		const vec2 C(x * 32 + 16, y * 32 + 16);
		return !S.m_pCollision->CheckPoint(C) && !S.InFreeze(C) && !S.TouchesDeath(C);
	};
	std::vector<std::pair<int, ivec2>> vMouths;
	for(int y = F.y - Radius + 2; y <= F.y + Radius; y++)
		for(int x = F.x - Radius; x <= F.x + Radius; x++)
		{
			if(!Open(x, y) || !S.InFreeze(vec2(x * 32 + 16, y * 32 + 48)) || (x > F.x - Radius && Open(x - 1, y) && S.InFreeze(vec2(x * 32 - 16, y * 32 + 48))))
				continue;
			// a run starts here: to its end
			int End = x;
			while(End + 1 <= F.x + Radius && Open(End + 1, y) && S.InFreeze(vec2(End * 32 + 48, y * 32 + 48)))
				End++;
			ivec2 T((x + End) / 2, y - 2);
			if(!Open(T.x, T.y))
				T.y = y;
			int Best = 1 << 30;
			for(int k = x; k <= End; k++)
				Best = std::min(Best, Here.Dist(vec2(k * 32 + 16, y * 32 + 16)));
			if(Best < (1 << 30))
				vMouths.push_back({Best, T});
		}
	std::sort(vMouths.begin(), vMouths.end(), [](auto &a, auto &b) { return a.first < b.first || (a.first == b.first && (a.second.y < b.second.y || (a.second.y == b.second.y && a.second.x < b.second.x))); });
	std::vector<ivec2> vOut;
	for(int i = 0; i < (int)vMouths.size() && i < Max; i++)
		vOut.push_back(vMouths[i].second);
	return vOut;
}

// the catch after the split: the diver steers DDir until it's frozen; the catcher steers PreDir (jumping at Jump), throws
// its hook at Hook aimed where the diver will be Lead ticks later, and once it holds the diver steers HoldDir for Release
// ticks, then lets go and stands
// an aim from From at a tee at To that the hook catches it with: the nearest angle to straight at it whose hook path
// passes within reach of it (the core: the hook starts 42 px out, flies until the hook length or a wall, and takes a
// tee within 30 px of the way it flew; from the end of a floor at a tee below beside it, straight at it hits the floor
// first: rank 1 aims further out, past the edge); vec2(0, 0) if none
static vec2 HookAimAt(const CCollision *pCol, vec2 From, vec2 To)
{
	const vec2 D = To - From;
	if(length(D) < 1)
		return D;
	const float A0 = std::atan2(D.y, D.x);
	for(int k = 0; k <= 16; k++)
	{
		const float A = A0 + (k % 2 ? 1 : -1) * ((k + 1) / 2) * 0.05f;
		const vec2 Dir(std::cos(A), std::sin(A));
		const vec2 Start = From + Dir * CCharacterCore::PhysicalSize() * 1.5f;
		vec2 End = From + Dir * 380.0f, Hit;
		if(pCol->IntersectLine(Start, End, &Hit, nullptr))
			End = Hit;
		vec2 Closest;
		if(!closest_point_on_line(Start, End, To, Closest))
			Closest = distance(Start, To) < distance(End, To) ? Start : End;
		if(distance(Closest, To) < CCharacterCore::PhysicalSize() - 2)
			return Dir * 100;
	}
	return vec2(0, 0);
}

struct SCatchParams
{
	int m_Diver = 1, m_DDir = 0, m_PreDir = 0, m_Jump = -1, m_Hook = 1 << 30, m_Lead = 2, m_HoldDir = 0, m_Release = 0;
	int m_Kick = -1; // the catcher hammers the diver on its way at this tick (if in reach: rank 1 kicks it over the wall)
	bool m_UsePreX = false; // until it holds the diver, the catcher steers to m_PreX and stops there (not m_PreDir)
	float m_PreX = 0;
	bool m_Rehook = false; // the catcher hooks the diver again when its hook runs out before m_Release (a hook holds a tee
			       // 60 ticks)
	bool m_HoldJump = false; // holding, the catcher jumps up a wall it walks into (onto the block over the pocket)
	bool m_DiverJump = false; // the diver, thawed, air-jumps falling toward the freeze
	bool m_FreeHammer = false; // the catcher hammers the diver free once it's out of the freeze and in reach (rank 1 through
				   // the corner between the floor and the block, the diver up under the block)
	int m_DDirAir = -2; // the diver's direction once off the ground, until it freezes (-2: m_DDir still; rank 1 runs off the
			    // edge into the gap above Stronghold's freeze column and steers back at once, to fall at its far
			    // side onto the corner of the block where the other hooks it out)
};
struct SCatchState
{
	int m_aPrevHook[2] = {0, 0};
	vec2 m_HookAim = vec2(0, 0);
	int m_GrabAt = -1; // the tick from which the catcher holds the diver
	bool m_Lost = false; // the hook missed
	bool m_DiverFroze = false;
	bool m_DiverLanded = false;
	bool m_DiverLeft = false; // off the ground since the start
};
static void CatchStep(CHookBotSim &S, const SCatchParams &C, SCatchState &St, int t, CNetObj_PlayerInput aI[2])
{
	const int dv = C.m_Diver, ct = 1 - dv;
	const auto &Dv = S.m_aTee[dv], &Ct = S.m_aTee[ct];
	aI[0] = {};
	aI[1] = {};
	aI[dv].m_TargetX = 1;
	St.m_DiverLeft |= !OnGroundAt(S.m_pCollision, Dv.m_Core.m_Pos);
	aI[dv].m_Direction = Dv.m_FreezeTime > 0 ? 0 : C.m_DDirAir != -2 && St.m_DiverLeft && !St.m_DiverFroze ? C.m_DDirAir : C.m_DDir;
	St.m_DiverFroze |= Dv.m_FreezeTime > 0;
	// (and stands once it lands: steering on, it walked off the floor at the shaft's bottom into the freeze beside it)
	if(C.m_DiverJump && St.m_DiverFroze && Dv.m_FreezeTime == 0 && Ct.m_Core.HookedPlayer() != Dv.m_Core.m_Id && OnGroundAt(S.m_pCollision, Dv.m_Core.m_Pos))
		St.m_DiverLanded = true;
	if(St.m_DiverLanded)
		aI[dv].m_Direction = 0;
	// (once let go: held up under the block, it spent it at once)
	if(C.m_DiverJump && St.m_DiverFroze && Dv.m_FreezeTime == 0 && Ct.m_Core.HookedPlayer() != Dv.m_Core.m_Id && !(Dv.m_Core.m_Jumped & 2) && Dv.m_Core.m_Vel.y > 0 && !OnGroundAt(S.m_pCollision, Dv.m_Core.m_Pos) &&
		FreezeClearance(S, Dv.m_Core.m_Pos) < 96)
		aI[dv].m_Jump = 1;
	CNetObj_PlayerInput &I = aI[ct];
	Aim(I, Dv.m_Core.m_Pos - Ct.m_Core.m_Pos);
	bool Hooking = false, HoldJump = false;
	const int PreDir = C.m_UsePreX ? SteerTo(C.m_PreX, Ct.m_Core.m_Pos.x, Ct.m_Core.m_Vel.x) : C.m_PreDir;
	if(t < C.m_Hook)
		I.m_Direction = PreDir;
	else if(St.m_GrabAt < 0)
	{
		if(t == C.m_Hook)
			St.m_HookAim = Dv.m_Core.m_Pos + Dv.m_Core.m_Vel * (float)C.m_Lead - Ct.m_Core.m_Pos;
		Hooking = !St.m_Lost && t - C.m_Hook < 12;
		St.m_Lost |= !Hooking;
		I.m_TargetX = round_to_int(St.m_HookAim.x);
		I.m_TargetY = round_to_int(St.m_HookAim.y);
		I.m_Direction = PreDir;
	}
	else if(t < St.m_GrabAt + C.m_Release && (Ct.m_Core.HookedPlayer() == Dv.m_Core.m_Id || C.m_Rehook))
	{
		Hooking = true;
		I.m_Direction = C.m_HoldDir;
		// (once the diver is past the wall, up under the block and free: jumping before, it would pull it up around the
		// corner, or out of reach of the hammer that frees it; rank 1 hammers it, then jumps)
		HoldJump = C.m_HoldJump && C.m_HoldDir && OnGroundAt(S.m_pCollision, Ct.m_Core.m_Pos) && S.m_pCollision->CheckPoint(Ct.m_Core.m_Pos + vec2(C.m_HoldDir * 18.0f, 0)) &&
			   Ct.m_Core.HookedPlayer() == Dv.m_Core.m_Id && (Dv.m_Core.m_Pos.x - Ct.m_Core.m_Pos.x) * C.m_HoldDir > 24 && Dv.m_Core.m_Pos.y - Ct.m_Core.m_Pos.y < 40 && Dv.m_FreezeTime == 0;
		// (and lets go as it jumps, to hook it again on the way up: 60 more ticks of pull from up there, where no hook gets
		// to it past the block any more; rank 1 so)
		if(HoldJump && C.m_Rehook)
			Hooking = false;
		else if(Ct.m_Core.HookedPlayer() != Dv.m_Core.m_Id)
		{
			const vec2 A = HookAimAt(S.m_pCollision, Ct.m_Core.m_Pos, Dv.m_Core.m_Pos + Dv.m_Core.m_Vel * 2.0f);
			if(A.x != 0 || A.y != 0)
			{
				I.m_TargetX = round_to_int(A.x);
				I.m_TargetY = round_to_int(A.y);
			}
		}
	}
	I.m_Jump = t == C.m_Jump || HoldJump;
	I.m_Hook = HookBotHookInput(Hooking, St.m_aPrevHook[ct], Ct.m_Core.m_HookState);
	St.m_aPrevHook[ct] = I.m_Hook;
	St.m_aPrevHook[dv] = 0;
	// (the hammer needs no clear line: the server finds whoever is near a point 21 px out the way it aims)
	const bool FreeIt = C.m_FreeHammer && St.m_GrabAt >= 0 && Dv.m_FreezeTime > 0 && !S.InFreeze(Dv.m_Core.m_Pos);
	if((t == C.m_Kick || FreeIt) && Ct.m_Reload == 0 && Ct.m_FreezeTime == 0 && HammerReaches(Ct.m_Core.m_Pos, Dv.m_Core.m_Pos) &&
		(FreeIt || !S.m_pCollision->IntersectLine(Ct.m_Core.m_Pos, Dv.m_Core.m_Pos, nullptr, nullptr)))
	{
		I.m_Fire = 1;
		Aim(I, Dv.m_Core.m_Pos - Ct.m_Core.m_Pos);
		HammerPush(S.m_aTee[dv].m_Core, Ct.m_Core.m_Pos);
		S.m_aTee[dv].m_FreezeTime = 0;
		S.m_aTee[ct].m_Reload = 16;
	}
	S.Step(aI[0], aI[1]);
	if(St.m_GrabAt < 0 && t >= C.m_Hook && !St.m_Lost)
	{
		if(Ct.m_Core.HookedPlayer() == Dv.m_Core.m_Id)
			St.m_GrabAt = t + 1;
		else if(t > C.m_Hook && Ct.m_Core.m_HookState != HOOK_FLYING)
			St.m_Lost = true;
	}
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanFall(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps, const SPseudoParams &Pseudo)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_FALL;
	int Steps = 0;
	const CCollision *pCol = Base.m_pCollision;
	const float aD0[2] = {Field.Dist(Base.m_aTee[0].m_Core.m_Pos), Field.Dist(Base.m_aTee[1].m_Core.m_Pos)};
	float BestValue = 300; // it has to get us somewhere
	const int FlyMax = 360, Split = 3;
	// the fly goes to above a freeze mouth near us (a mouth that isn't above us needs no fly: the diver walks or falls
	// into it from where we are, the split right away)
	const vec2 Mid = (Base.m_aTee[0].m_Core.m_Pos + Base.m_aTee[1].m_Core.m_Pos) / 2;
	const ivec2 MidTile((int)std::floor(Mid.x / 32), (int)std::floor(Mid.y / 32));
	std::vector<ivec2> vMouths;
	for(const ivec2 &M : FreezeMouths(Base, Mid, 30, 6))
		if(M.y < MidTile.y - 2 && vMouths.size() < 3)
			vMouths.push_back(M);
	const int FlyR = 40;
	// where open air takes each of us from here: the diver has to end up somewhere else (from where it is, not from
	// between us: one on the floor above Stronghold's freeze holes at x 413-421, the other on the floor below, their
	// middle in the room between, where the caught one was to land)
	SAirGrid aHereAir[2];
	for(int i = 0; i < 2; i++)
	{
		const ivec2 T((int)std::floor(Base.m_aTee[i].m_Core.m_Pos.x / 32), (int)std::floor(Base.m_aTee[i].m_Core.m_Pos.y / 32));
		aHereAir[i].Build(Base, T, T, FlyR);
	}
	if(ms_DebugTeam)
		for(const ivec2 &M : vMouths)
			printf("fall: mouth %d %d\n", M.x, M.y);
	// the plan: the fly (to which mouth, who drives first, how long), then the catch
	struct SSplit
	{
		CHookBotSim m_S;
		int m_Drive, m_Driver, m_T;
	};
	int BestDrive = -1, BestDriver = 0, BestFlyTicks = 0;
	SCatchParams BestC;
	// both at rest from here on, each doing what the catch has it do; false if it's no good: someone dead, or one of us
	// at rest in the freeze, or the catcher frozen at all
	// (HH_FALLTRACE="hook,holddir,release[,prex]": that catch, tick by tick from its release; prex in tiles, else no pre-x)
	int aTrace[4] = {-1, -9, -1, -1};
	const int NumTrace = getenv("HH_FALLTRACE") ? sscanf(getenv("HH_FALLTRACE"), "%d,%d,%d,%d", &aTrace[0], &aTrace[1], &aTrace[2], &aTrace[3]) : 0;
	auto ToRest = [&](CHookBotSim &S, const SCatchParams &C, SCatchState St, int t) {
		int Rest = 0;
		const bool Trace = NumTrace >= 3 && C.m_Hook == aTrace[0] && C.m_HoldDir == aTrace[1] && C.m_Release == aTrace[2] && (NumTrace < 4 ? !C.m_UsePreX : C.m_UsePreX && (int)(C.m_PreX / 32) == aTrace[3]);
		for(int k = 0; k < 300 && Rest < 3; k++, t++)
		{
			CNetObj_PlayerInput aI[2];
			CatchStep(S, C, St, t, aI);
			if(Trace)
				printf("falltrace t %d: %.1f %.1f v %.1f %.1f frz %d hook %d in %+d%s%s%s | %.1f %.1f v %.1f %.1f frz %d hook %d jmp %d clr %.0f in %+d%s%s%s\n", t, S.m_aTee[0].m_Core.m_Pos.x / 32, S.m_aTee[0].m_Core.m_Pos.y / 32,
					S.m_aTee[0].m_Core.m_Vel.x, S.m_aTee[0].m_Core.m_Vel.y, S.m_aTee[0].m_FreezeTime, S.m_aTee[0].m_Core.m_HookState, aI[0].m_Direction, aI[0].m_Jump ? " J" : "", aI[0].m_Hook ? " H" : "",
					aI[0].m_Fire ? " F" : "", S.m_aTee[1].m_Core.m_Pos.x / 32, S.m_aTee[1].m_Core.m_Pos.y / 32, S.m_aTee[1].m_Core.m_Vel.x, S.m_aTee[1].m_Core.m_Vel.y, S.m_aTee[1].m_FreezeTime,
					S.m_aTee[1].m_Core.m_HookState, S.m_aTee[1].m_Core.m_Jumped, FreezeClearance(S, S.m_aTee[1].m_Core.m_Pos), aI[1].m_Direction, aI[1].m_Jump ? " J" : "", aI[1].m_Hook ? " H" : "", aI[1].m_Fire ? " F" : "");
			Steps++;
			const auto &Ct = S.m_aTee[1 - C.m_Diver];
			if(S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead || Ct.m_FreezeTime > 0)
				return false;
			Rest = AtRest(S, 0) && AtRest(S, 1) ? Rest + 1 : 0;
			if(Rest && (S.InFreeze(S.m_aTee[0].m_Core.m_Pos) || S.InFreeze(S.m_aTee[1].m_Core.m_Pos)))
				return false;
		}
		return Rest >= 3;
	};
	const bool Why = ms_DebugTeam && getenv("HH_FALLSTATS") && atoi(getenv("HH_FALLSTATS")) >= 2;
	auto Judge = [&](CHookBotSim &S, const SCatchParams &C, const SCatchState &St, int t, const SSplit &Sp) {
		if(Why)
			printf("fall judge: pre %s%.0f jump %d at release %.1f %.1f / %.1f %.1f, ", C.m_UsePreX ? "x " : "dir ", C.m_UsePreX ? C.m_PreX / 32 : (float)C.m_PreDir, C.m_Jump, S.m_aTee[0].m_Core.m_Pos.x / 32,
				S.m_aTee[0].m_Core.m_Pos.y / 32, S.m_aTee[1].m_Core.m_Pos.x / 32, S.m_aTee[1].m_Core.m_Pos.y / 32);
		const bool Rested = ToRest(S, C, St, t);
		if(Why)
			printf("hook %d hold %d for %d: ends %.1f %.1f %s / %.1f %.1f %s, rest %d", C.m_Hook, C.m_HoldDir, C.m_Release, S.m_aTee[0].m_Core.m_Pos.x / 32, S.m_aTee[0].m_Core.m_Pos.y / 32,
				S.InFreeze(S.m_aTee[0].m_Core.m_Pos) ? "frz" : "", S.m_aTee[1].m_Core.m_Pos.x / 32, S.m_aTee[1].m_Core.m_Pos.y / 32, S.InFreeze(S.m_aTee[1].m_Core.m_Pos) ? "frz" : "", Rested);
		if(!Rested)
		{
			if(Why)
				printf("\n");
			return;
		}
		// (the catcher may end up further from the goal than it was, e.g. back up on the platform beside the shaft from
		// above the pool next to it: the diver gets far)
		float Value;
		bool EndOk = TeamEndValue(S, Field, aD0, &Value, -800);
		// a walking catch: the diver far on, both free out of the freeze, though the catcher may be left where neither
		// of us gets to the other (Stronghold 132-135 s: the diver dragged under the floor into the pocket and on to
		// the shaft beyond, the catcher left on the block above it; its way on is another fall, the diver catching it
		// below)
		if(!EndOk && C.m_Rehook)
		{
			const int dv = C.m_Diver, ct = 1 - dv;
			const float GainD = aD0[dv] - Field.Dist(S.m_aTee[dv].m_Core.m_Pos), GainC = aD0[ct] - Field.Dist(S.m_aTee[ct].m_Core.m_Pos);
			if(!S.m_aTee[0].m_Dead && !S.m_aTee[1].m_Dead && S.m_aTee[dv].m_FreezeTime == 0 && !S.InFreeze(S.m_aTee[0].m_Core.m_Pos) && !S.InFreeze(S.m_aTee[1].m_Core.m_Pos) && GainD >= 1000 &&
				GainC >= -1200)
			{
				EndOk = true;
				Value = GainD + GainC;
			}
		}
		if(Why)
			printf(" end %d gain %.0f/%.0f air %d\n", EndOk, aD0[0] - Field.Dist(S.m_aTee[0].m_Core.m_Pos), aD0[1] - Field.Dist(S.m_aTee[1].m_Core.m_Pos),
				aHereAir[C.m_Diver].Dist(S.m_aTee[C.m_Diver].m_Core.m_Pos) < (1 << 30));
		if(!EndOk)
			return;
		// the diver has to get somewhere open air doesn't take us (not just shuffle along the floor it started from), and
		// not into a closed pocket (it has to be able to go on from there)
		if(aD0[C.m_Diver] - Field.Dist(S.m_aTee[C.m_Diver].m_Core.m_Pos) < 400 || aHereAir[C.m_Diver].Dist(S.m_aTee[C.m_Diver].m_Core.m_Pos) < (1 << 30) ||
			!ProgressFrom(S, Field, S.m_aTee[C.m_Diver].m_Core.m_Pos, 96))
			return;
		Value -= 0.5f * (Sp.m_T + t);
		if(Value <= BestValue)
			return;
		BestValue = Value;
		BestDrive = Sp.m_Drive;
		BestDriver = Sp.m_Driver;
		BestFlyTicks = Sp.m_T;
		BestC = C;
		if(ms_DebugTeam)
			printf("fall: value %.0f, fly %d ticks to mouth %d, diver %d dir %d kick %d, catcher pre %d jump %d hook %d lead %d hold %d for %d, ends %.1f %.1f / %.1f %.1f (%d searched)\n", Value, Sp.m_T,
				Sp.m_Drive, C.m_Diver, C.m_DDir, C.m_Kick, C.m_PreDir, C.m_Jump, C.m_Hook, C.m_Lead, C.m_HoldDir, C.m_Release, S.m_aTee[0].m_Core.m_Pos.x / 32, S.m_aTee[0].m_Core.m_Pos.y / 32,
				S.m_aTee[1].m_Core.m_Pos.x / 32, S.m_aTee[1].m_Core.m_Pos.y / 32, Steps);
	};
	// the flies, a split every few ticks
	std::vector<SSplit> vSplits;
	const bool Standing = OnGroundAt(pCol, Base.m_aTee[0].m_Core.m_Pos) && OnGroundAt(pCol, Base.m_aTee[1].m_Core.m_Pos);
	vSplits.push_back({Base, -1, 0, 0});
	for(int Drive = 0; Drive < (int)vMouths.size(); Drive++)
		for(int Driver = 0; Driver < (Standing ? 2 : 1); Driver++)
		{
			SAirGrid Grid;
			Grid.Build(Base, MidTile, vMouths[Drive], FlyR);
			CHookBotSim S = Base;
			SFlyState F;
			F.m_Driver = Driver;
			for(int i = 0; i < 2; i++)
				F.m_aPrevHook[i] = Base.m_aTee[i].m_PrevInput.m_Hook;
			int Over = -1;
			for(int t = 1; t <= FlyMax; t++)
			{
				CNetObj_PlayerInput aI[2];
				FlyStep(S, F, Pseudo, Grid.Dir(S.m_aTee[F.m_Driver].m_Core.m_Pos), aI);
				Steps++;
				if(ms_DebugTeam && getenv("HH_FLYTRACE") && t % atoi(getenv("HH_FLYTRACE")) == 0)
					printf("fly %d/%d t %d: %.1f %.1f v %.1f %.1f %s | %.1f %.1f v %.1f %.1f %s\n", Drive, Driver, t, S.m_aTee[0].m_Core.m_Pos.x / 32, S.m_aTee[0].m_Core.m_Pos.y / 32,
						S.m_aTee[0].m_Core.m_Vel.x, S.m_aTee[0].m_Core.m_Vel.y, S.m_aTee[0].m_FreezeTime ? "F" : "", S.m_aTee[1].m_Core.m_Pos.x / 32, S.m_aTee[1].m_Core.m_Pos.y / 32,
						S.m_aTee[1].m_Core.m_Vel.x, S.m_aTee[1].m_Core.m_Vel.y, S.m_aTee[1].m_FreezeTime ? "F" : "");
				const auto &T0 = S.m_aTee[0], &T1 = S.m_aTee[1];
				if(T0.m_Dead || T1.m_Dead || T0.m_FreezeTime > 0 || T1.m_FreezeTime > 0 || (t > 20 && OnGroundAt(pCol, T0.m_Core.m_Pos) && OnGroundAt(pCol, T1.m_Core.m_Pos)))
					break;
				if(t % Split == 0)
					vSplits.push_back({S, Drive, Driver, t});
				// over the mouth: a little longer, then that's it
				const vec2 Dp = S.m_aTee[F.m_Driver].m_Core.m_Pos;
				if(Over < 0 && absolute(Dp.x / 32 - (vMouths[Drive].x + 0.5f)) < 2 && Dp.y / 32 < vMouths[Drive].y + 2)
					Over = t;
				if(Over >= 0 && t > Over + 30)
					break;
			}
		}
	// each split, each of us as the diver, steering left / nothing / right until it's frozen: its fall with nobody
	// catching it (if it then lands outside the freeze, that's a plan already); worth a catch if it freezes and passes
	// near the other
	struct SCand
	{
		int m_Split, m_Diver, m_DDir, m_DDirAir, m_Kick, m_Frozen;
		float m_Fell; // how far the diver is from the goal where its fall takes it uncaught (the catch only bends it)
		ivec2 m_Where; // and where that is (4-tile cells)
		float m_FrozenX; // where the diver freezes
	};
	std::vector<SCand> vCands;
	for(int Sn = 0; Sn < (int)vSplits.size() && Steps < MaxSteps; Sn++)
		for(int Diver = 0; Diver < 2; Diver++)
			for(int Kick = -1; Kick <= 2; Kick += 3)
				for(int DDir = -1; DDir <= 1; DDir++)
					for(int DAir = -2; DAir <= 1; DAir++)
			{
				const SSplit &Sp = vSplits[Sn];
				// a kick only where the hammer reaches
				if(Kick >= 0 && !HammerReaches(Sp.m_S.m_aTee[0].m_Core.m_Pos, Sp.m_S.m_aTee[1].m_Core.m_Pos))
					continue;
				// (steering otherwise in the air only after running off somewhere)
				if(DAir != -2 && (DAir == DDir || DDir == 0))
					continue;
				SCatchParams C;
				C.m_Diver = Diver;
				C.m_DDir = DDir;
				C.m_DDirAir = DAir;
				C.m_Kick = Kick;
				int Frozen = -1, MinDist = 1 << 30;
				float FrozenX = 0;
				CHookBotSim S = Sp.m_S;
				SCatchState St;
				for(int t = 0; t < 100 && Frozen < 0; t++)
				{
					CNetObj_PlayerInput aI[2];
					CatchStep(S, C, St, t, aI);
					Steps++;
					if(S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead)
						break;
					if(S.m_aTee[Diver].m_FreezeTime > 0)
					{
						Frozen = t;
						FrozenX = S.m_aTee[Diver].m_Core.m_Pos.x;
					}
				}
				if(Frozen < 0)
					continue;
				CHookBotSim E = S;
				Judge(E, C, St, Frozen + 1, Sp);
				for(int t = Frozen + 1; t < Frozen + 100; t++)
				{
					CNetObj_PlayerInput aI[2];
					CatchStep(S, C, St, t, aI);
					Steps++;
					if(t < Frozen + 60)
						MinDist = std::min(MinDist, (int)distance(S.m_aTee[0].m_Core.m_Pos, S.m_aTee[1].m_Core.m_Pos));
				}
				const vec2 Fell = S.m_aTee[Diver].m_Core.m_Pos;
				// (the catcher can still walk some tiles closer before it hooks: Stronghold's freeze pool at x 443-451 below a
				// hole in the floor, the catcher on the floor below 15 tiles from it)
				if(MinDist <= 800)
					vCands.push_back({Sn, Diver, DDir, DAir, Kick, Frozen, Field.Dist(Fell), ivec2((int)std::floor(Fell.x / 128), (int)std::floor(Fell.y / 128)), FrozenX});
			}
	// by where the fall takes the diver uncaught: a turn for each place in turn (the places nearest the goal first),
	// within a place in the order of the splits (all the falls down a freeze column, whose bottom is nearest the goal and
	// nowhere to land outside the freeze, used up the time before a fall down the shaft beside it got its turn)
	{
		std::vector<std::pair<ivec2, std::vector<SCand>>> vPlaces;
		for(const SCand &Cd : vCands)
		{
			auto It = std::find_if(vPlaces.begin(), vPlaces.end(), [&](const auto &P) { return P.first == Cd.m_Where; });
			if(It == vPlaces.end())
			{
				vPlaces.push_back({Cd.m_Where, {}});
				It = vPlaces.end() - 1;
			}
			It->second.push_back(Cd);
		}
		auto BestFell = [](const std::vector<SCand> &v) {
			float b = 1e9f;
			for(const SCand &Cd : v)
				b = std::min(b, Cd.m_Fell);
			return b;
		};
		std::stable_sort(vPlaces.begin(), vPlaces.end(), [&](const auto &a, const auto &b) { return BestFell(a.second) < BestFell(b.second); });
		vCands.clear();
		for(size_t k = 0;; k++)
		{
			bool Any = false;
			for(const auto &P : vPlaces)
				if(k < P.second.size())
				{
					vCands.push_back(P.second[k]);
					Any = true;
				}
			if(!Any)
				break;
		}
	}
	// the catches: a coarse search of every candidate first (the first plan found will do: the search is costly), then
	// a finer one, each with its share of what's left
	for(int Level = 0; Level < 2 && BestValue <= 300; Level++)
	{
		const int LevelLimit = Steps + (MaxSteps - Steps) / (Level == 0 ? 3 : 1);
		for(int Cn = 0; Cn < (int)vCands.size() && Steps < LevelLimit && BestValue <= 300; Cn++)
		{
			const SCand &Cd = vCands[Cn];
			const SSplit &Sp = vSplits[Cd.m_Split];
			const int Diver = Cd.m_Diver, Frozen = Cd.m_Frozen;
			const int Limit = Steps + (LevelLimit - Steps) / ((int)vCands.size() - Cn);
			SCatchParams C;
			C.m_Diver = Diver;
			C.m_DDir = Cd.m_DDir;
			C.m_DDirAir = Cd.m_DDirAir;
			C.m_Kick = Cd.m_Kick;
			const int HMin = std::max(0, Frozen - 8), HMax = Frozen + 50, HookStep = Level == 0 ? 3 : 2, RelStep = Level == 0 ? 6 : 3;
			std::vector<int> vJumps = {-1};
			if(Level == 1)
				for(int J : {0, Frozen - 24, Frozen - 16, Frozen - 8, Frozen, Frozen + 8})
					if(J >= 0 && std::find(vJumps.begin(), vJumps.end(), J) == vJumps.end())
						vJumps.push_back(J);
			// grabs that leave us where another one did (8 px, 2 px/tick) go the same way after: once
			std::vector<std::array<int, 8>> vGrabbed;
			// the catcher before its hook: a direction all the way, or (finer) to a spot a few tiles off and stop there (it
			// walked on into Stronghold's freeze pool at x 443-451 before the diver came)
			// (or to 4, 7 or 10 tiles from where the diver freezes, on its own side: on the very edge of the floor it was
			// pulled off into the freeze by its own hook, the spots further back out of reach)
			const int NumPre = Level == 0 ? 3 : 12;
			const float CatcherX = Sp.m_S.m_aTee[1 - Diver].m_Core.m_Pos.x, Side = CatcherX >= Cd.m_FrozenX ? 1.0f : -1.0f;
			for(int Pre = 0; Pre < NumPre && Steps < Limit; Pre++)
				for(int Jump : vJumps)
				{
					if(Steps >= Limit)
						break;
					static const int s_aPreTiles[] = {-6, -3, 3, 6, 9, 12}, s_aNearTiles[] = {4, 7, 10};
					C.m_UsePreX = Pre >= 3;
					C.m_PreDir = Pre < 3 ? Pre - 1 : 0;
					C.m_PreX = Pre < 3 ? 0 : Pre < 9 ? CatcherX + s_aPreTiles[Pre - 3] * 32 : Cd.m_FrozenX + Side * s_aNearTiles[Pre - 9] * 32;
					C.m_Jump = Jump;
					C.m_Rehook = C.m_HoldJump = C.m_DiverJump = C.m_FreeHammer = false;
					// the catcher's way up to each hook tick
					std::vector<std::pair<CHookBotSim, SCatchState>> vPre;
					{
						SCatchParams P = C;
						P.m_Hook = 1 << 30;
						CHookBotSim S = Sp.m_S;
						SCatchState St;
						for(int t = 0; t <= HMax; t++)
						{
							vPre.push_back({S, St});
							CNetObj_PlayerInput aI[2];
							CatchStep(S, P, St, t, aI);
							Steps++;
							if(S.m_aTee[1 - Diver].m_FreezeTime > 0 || S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead)
								break;
						}
					}
					for(int Hook = std::max(HMin, Jump); Hook < (int)vPre.size() && Steps < Limit; Hook += HookStep)
					{
						if(distance(vPre[Hook].first.m_aTee[0].m_Core.m_Pos, vPre[Hook].first.m_aTee[1].m_Core.m_Pos) > 450)
							continue;
						for(int Lead = Level == 0 ? 2 : 1; Lead <= (Level == 0 ? 2 : 3); Lead++)
						{
							C.m_Hook = Hook;
							C.m_Lead = Lead;
							C.m_Release = 1 << 30;
							C.m_Rehook = C.m_HoldJump = C.m_DiverJump = C.m_FreeHammer = false;
							CHookBotSim S = vPre[Hook].first;
							SCatchState St = vPre[Hook].second;
							int t = Hook;
							for(; t < Hook + 12 && St.m_GrabAt < 0 && !St.m_Lost; t++)
							{
								CNetObj_PlayerInput aI[2];
								CatchStep(S, C, St, t, aI);
								Steps++;
							}
							if(St.m_GrabAt < 0)
								continue;
							std::array<int, 8> Key;
							for(int i = 0; i < 2; i++)
							{
								const auto &Core = S.m_aTee[i].m_Core;
								Key[i * 4 + 0] = (int)std::floor(Core.m_Pos.x / 8);
								Key[i * 4 + 1] = (int)std::floor(Core.m_Pos.y / 8);
								Key[i * 4 + 2] = (int)std::floor(Core.m_Vel.x / 2);
								Key[i * 4 + 3] = (int)std::floor(Core.m_Vel.y / 2);
							}
							if(std::find(vGrabbed.begin(), vGrabbed.end(), Key) != vGrabbed.end())
								continue;
							vGrabbed.push_back(Key);
							// (walking with it: on and on, hooking it again when the hook runs out; rank 1 drags the frozen
							// one through the freeze under the floor it walks on into the pocket beyond, Stronghold 132-134
							// s: the rope holds through walls)
							for(int HoldDir = -1; HoldDir <= 1; HoldDir++)
							{
								C.m_HoldDir = HoldDir;
								C.m_Rehook = C.m_HoldJump = C.m_DiverJump = C.m_FreeHammer = HoldDir != 0;
								CHookBotSim R = S;
								SCatchState RSt = St;
								const int MaxHold = HoldDir ? 150 : 36;
								int Unhooked = 0;
								for(int Hold = 0, u = t; Hold <= MaxHold && Steps < Limit; Hold++, u++)
								{
									if(Hold % (Hold > 36 ? 6 : RelStep) == 0)
									{
										SCatchParams Rel = C;
										Rel.m_Release = u - RSt.m_GrabAt;
										CHookBotSim E = R;
										Judge(E, Rel, RSt, u, Sp);
									}
									if(NumTrace >= 3 && C.m_Hook == aTrace[0] && C.m_HoldDir == aTrace[1] && Hold <= aTrace[2] && (NumTrace < 4 ? !C.m_UsePreX : C.m_UsePreX && (int)(C.m_PreX / 32) == aTrace[3]))
										printf("falltrace hold %d (t %d): %.1f %.1f v %.1f %.1f frz %d hook %d hooked %d | %.1f %.1f v %.1f %.1f frz %d hook %d hooked %d\n", Hold, u, R.m_aTee[0].m_Core.m_Pos.x / 32,
											R.m_aTee[0].m_Core.m_Pos.y / 32, R.m_aTee[0].m_Core.m_Vel.x, R.m_aTee[0].m_Core.m_Vel.y, R.m_aTee[0].m_FreezeTime, R.m_aTee[0].m_Core.m_HookState, R.m_aTee[0].m_Core.HookedPlayer(),
											R.m_aTee[1].m_Core.m_Pos.x / 32, R.m_aTee[1].m_Core.m_Pos.y / 32, R.m_aTee[1].m_Core.m_Vel.x, R.m_aTee[1].m_Core.m_Vel.y, R.m_aTee[1].m_FreezeTime, R.m_aTee[1].m_Core.m_HookState,
											R.m_aTee[1].m_Core.HookedPlayer());
									Unhooked = R.m_aTee[1 - Diver].m_Core.HookedPlayer() != R.m_aTee[Diver].m_Core.m_Id ? Unhooked + 1 : 0;
									if(Unhooked > (C.m_Rehook ? 8 : 0))
										break;
									CNetObj_PlayerInput aI[2];
									CatchStep(R, C, RSt, u, aI);
									Steps++;
								}
							}
						}
					}
				}
			if(ms_DebugTeam && getenv("HH_FALLSTATS"))
				printf("fall split: level %d mouth %d fly %d diver %d ddir %d air %d frozen %d at %.1f %.1f: %d grabs\n", Level, Sp.m_Drive, Sp.m_T, Diver, Cd.m_DDir, Cd.m_DDirAir, Frozen,
					Sp.m_S.m_aTee[Diver].m_Core.m_Pos.x / 32, Sp.m_S.m_aTee[Diver].m_Core.m_Pos.y / 32, (int)vGrabbed.size());
		}
	}
	Best.m_Steps = Steps;
	if(BestValue <= 300)
		return Best;
	// the plan: the fly, then the catch to the end
	CHookBotSim S = Base;
	SFlyState F;
	F.m_Driver = BestDriver;
	for(int i = 0; i < 2; i++)
		F.m_aPrevHook[i] = Base.m_aTee[i].m_PrevInput.m_Hook;
	SAirGrid Grid;
	if(BestDrive >= 0)
		Grid.Build(Base, MidTile, vMouths[BestDrive], FlyR);
	for(int t = 0; t < BestFlyTicks; t++)
	{
		CNetObj_PlayerInput aI[2];
		for(int i = 0; i < 2; i++)
			Best.m_avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
		FlyStep(S, F, Pseudo, Grid.Dir(S.m_aTee[F.m_Driver].m_Core.m_Pos), aI);
		for(int i = 0; i < 2; i++)
			Best.m_avIn[i].push_back(aI[i]);
	}
	SCatchState St;
	int Rest = 0;
	for(int t = 0; t < 600 && Rest < 3; t++)
	{
		CNetObj_PlayerInput aI[2];
		for(int i = 0; i < 2; i++)
			Best.m_avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
		CatchStep(S, BestC, St, t, aI);
		for(int i = 0; i < 2; i++)
			Best.m_avIn[i].push_back(aI[i]);
		Rest = AtRest(S, 0) && AtRest(S, 1) ? Rest + 1 : 0;
	}
	Best.m_Valid = true;
	Best.m_Score = BestValue;
	Best.m_Up = BestC.m_Diver;
	Best.m_aEnd[0] = S.m_aTee[0].m_Core.m_Pos;
	Best.m_aEnd[1] = S.m_aTee[1].m_Core.m_Pos;
	return Best;
}

// a freeze floor ahead of From (standing): within 4 tiles the way on (Dir), the floor under our feet turns to freeze, for
// at least 10 tiles (shorter ones a jump clears)
static bool FreezeFloorAhead(const CHookBotSim &S, vec2 From, int Dir)
{
	const float FeetY = From.y + CCharacterCore::PhysicalSize() / 2 + 8;
	int Start = -1, Len = 0;
	for(int i = 1; i <= 60; i++)
	{
		const vec2 P(From.x + Dir * i * 32.0f, FeetY);
		const bool Frz = S.InFreeze(P) || S.InFreeze(P - vec2(0, 16));
		if(Start < 0)
		{
			if(Frz)
				Start = i;
			else if(i > 4 || !S.m_pCollision->CheckPoint(P))
				return false;
			continue;
		}
		if(!Frz)
			break;
		Len++;
	}
	return Start >= 0 && Len >= 9;
}

// a dash: both of us fly across a freeze floor with nothing to hook but each other (Stronghold, the corridor after the
// zig-zag: 45 tiles of freeze floor under an unhookable ceiling 8 tiles up; rank 1 crosses it in 2 s at 14-20 px/tick:
// one drags the other past it on the hook and hammers it as it comes by, then each holds the way on and they hook each
// other up whenever one sinks)
struct SDashParams
{
	int m_Puller = 0; // who hooks first
	int m_aJump[2] = {-1, -1}; // the tick each jumps off the ground (-1: at the edge)
	int m_HookAt = 0; // the tick the first hook starts
	int m_PullerDir = 1; // the puller's direction while it pulls (1: the way on, 0, -1: back)
	int m_Release = 0; // the hooker lets go once the other is this far past it (px, the way on)
	bool m_Hammer = true; // the first puller hammers the other as it comes by
	int m_Rehook = 32; // after that, the one ahead hooks the one behind once it's this far below (px; -1: never)
	int m_AirJumpH = 64; // air jump when the freeze is this close below, falling (px)
};
struct SDashState
{
	int m_Hooker = -1;
	int m_HookTick = 0;
	bool m_FirstDone = false;
	int m_aPrevHook[2] = {0, 0};
};
static void DashStep(const CHookBotSim &S, const SDashParams &P, SDashState &St, int Dir, int t, CNetObj_PlayerInput aI[2])
{
	const CCollision *pCol = S.m_pCollision;
	const vec2 aPos[2] = {S.m_aTee[0].m_Core.m_Pos, S.m_aTee[1].m_Core.m_Pos};
	if(St.m_Hooker >= 0)
	{
		const int h = St.m_Hooker, o = 1 - h;
		const auto &H = S.m_aTee[h];
		bool Done = (aPos[o].x - aPos[h].x) * Dir > P.m_Release || H.m_FreezeTime > 0;
		// a hook that missed or ran out; one that pulled the other up past me
		Done |= t - St.m_HookTick > 3 && H.m_Core.m_HookState != HOOK_GRABBED && H.m_Core.m_HookState != HOOK_FLYING;
		Done |= St.m_FirstDone && aPos[o].y < aPos[h].y - 16;
		if(Done)
		{
			St.m_Hooker = -1;
			St.m_FirstDone = true;
		}
	}
	else if(!St.m_FirstDone && t >= P.m_HookAt)
	{
		St.m_Hooker = P.m_Puller;
		St.m_HookTick = t;
	}
	else if(St.m_FirstDone && P.m_Rehook >= 0)
	{
		const int a = (aPos[0].x - aPos[1].x) * Dir > 0 ? 0 : 1, b = 1 - a;
		if(aPos[b].y - aPos[a].y > P.m_Rehook && distance(aPos[a], aPos[b]) < 320 && S.m_aTee[a].m_FreezeTime == 0 && !OnGroundAt(pCol, aPos[a]) &&
			!pCol->IntersectLine(aPos[a], aPos[b], nullptr, nullptr))
		{
			St.m_Hooker = a;
			St.m_HookTick = t;
		}
	}
	for(int i = 0; i < 2; i++)
	{
		aI[i] = {};
		const auto &T = S.m_aTee[i];
		Aim(aI[i], aPos[1 - i] - aPos[i]);
		if(T.m_FreezeTime > 0)
		{
			St.m_aPrevHook[i] = 0;
			continue;
		}
		aI[i].m_Direction = St.m_Hooker == i && !St.m_FirstDone ? P.m_PullerDir * Dir : Dir;
		if(OnGroundAt(pCol, aPos[i]))
		{
			const bool Edge = !OnGroundAt(pCol, aPos[i] + vec2(T.m_Core.m_Vel.x * 3 + Dir * 4, 0));
			aI[i].m_Jump = P.m_aJump[i] < 0 ? Edge : t >= P.m_aJump[i];
		}
		else if(!(T.m_Core.m_Jumped & 2) && T.m_Core.m_Vel.y > 0 && FreezeClearance(S, aPos[i]) < P.m_AirJumpH)
			aI[i].m_Jump = 1;
		aI[i].m_Hook = HookBotHookInput(St.m_Hooker == i, St.m_aPrevHook[i], T.m_Core.m_HookState);
		St.m_aPrevHook[i] = aI[i].m_Hook;
	}
	if(P.m_Hammer && St.m_Hooker >= 0 && !St.m_FirstDone)
	{
		const int h = St.m_Hooker, o = 1 - h;
		const auto &H = S.m_aTee[h];
		if(H.m_FreezeTime == 0 && H.m_Reload == 0 && HammerReaches(aPos[h], aPos[o]) && (aPos[o].x - aPos[h].x) * Dir > -8)
			aI[h].m_Fire = 1;
	}
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanDash(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_DASH;
	int Steps = 0;
	const vec2 aP0[2] = {Base.m_aTee[0].m_Core.m_Pos, Base.m_aTee[1].m_Core.m_Pos};
	const vec2 Mid = (aP0[0] + aP0[1]) / 2;
	// the way on: the side where the field is nearest a bit above us (level with us, the freeze floor's 8x cost made the
	// way back look shorter)
	float aSide[2] = {1e9f, 1e9f};
	for(int Side = 0; Side < 2; Side++)
		for(float Dx : {96.0f, 192.0f, 320.0f})
			for(float Dy : {-32.0f, -96.0f})
				aSide[Side] = std::min(aSide[Side], Field.Dist(Mid + vec2(Side ? Dx : -Dx, Dy)));
	const int Dir = aSide[0] < aSide[1] ? -1 : 1;
	const float aD0[2] = {Field.Dist(aP0[0]), Field.Dist(aP0[1])};
	const int Front = (aP0[0].x - aP0[1].x) * Dir > 0 ? 0 : 1;
	struct SJumps
	{
		int m_Front, m_Back;
	};
	static const SJumps s_aJumps[] = {{-1, -1}, {0, -1}, {6, -1}, {-1, 0}, {0, 8}};
	static const int s_aHookAt[] = {0, 8, 16};
	static const int s_aPullerDir[] = {1, 0, -1};
	static const int s_aRelease[] = {-16, 24, 64};
	static const int s_aRehook[] = {16, 64, -1};
	static const int s_aAirJumpH[] = {48, 128};
	SDashParams BestP;
	float BestValue = 300; // it has to get us well on
	int Tried = 0, Rested = 0;
	for(int Puller = 0; Puller < 2; Puller++)
		for(const SJumps &J : s_aJumps)
			for(int HookAt : s_aHookAt)
				for(int PullerDir : s_aPullerDir)
					for(int Release : s_aRelease)
						for(int Hammer = 1; Hammer >= 0; Hammer--)
							for(int Rehook : s_aRehook)
								for(int AirJumpH : s_aAirJumpH)
								{
									if(Steps >= MaxSteps)
										break;
									SDashParams P;
									P.m_Puller = Puller == 0 ? Front : 1 - Front;
									P.m_aJump[Front] = J.m_Front;
									P.m_aJump[1 - Front] = J.m_Back;
									P.m_HookAt = HookAt;
									P.m_PullerDir = PullerDir;
									P.m_Release = Release;
									P.m_Hammer = Hammer;
									P.m_Rehook = Rehook;
									P.m_AirJumpH = AirJumpH;
									Tried++;
									CHookBotSim S = Base;
									SDashState St;
									int Rest = 0;
									for(int t = 0; t < 320 && Rest < 2; t++)
									{
										CNetObj_PlayerInput aI[2];
										DashStep(S, P, St, Dir, t, aI);
										for(int i = 0; i < 2; i++)
											if(aI[i].m_Fire)
											{
												HammerPush(S.m_aTee[1 - i].m_Core, S.m_aTee[i].m_Core.m_Pos);
												S.m_aTee[1 - i].m_FreezeTime = 0;
												S.m_aTee[i].m_Reload = 16;
											}
										S.Step(aI[0], aI[1]);
										Steps++;
										if(S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead)
											break;
										Rest = t > 10 && AtRest(S, 0) && AtRest(S, 1) ? Rest + 1 : 0;
									}
									float Value;
									if(Rest < 2 || !TeamEndValue(S, Field, aD0, &Value))
										continue;
									Rested++;
									if(Value > BestValue)
									{
										BestValue = Value;
										BestP = P;
									}
								}
	if(ms_DebugTeam)
		printf("dash: dir %d, %d tried, %d at rest and good, best %.0f, %d ticks\n", Dir, Tried, Rested, BestValue, Steps);
	if(BestValue <= 300)
	{
		Best.m_Steps = Steps;
		return Best;
	}
	// the best again, recorded
	CHookBotSim S = Base;
	SDashState St;
	int Rest = 0;
	for(int t = 0; t < 320 && Rest < 2; t++)
	{
		CNetObj_PlayerInput aI[2];
		DashStep(S, BestP, St, Dir, t, aI);
		for(int i = 0; i < 2; i++)
		{
			Best.m_avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
			Best.m_avIn[i].push_back(aI[i]);
		}
		for(int i = 0; i < 2; i++)
			if(aI[i].m_Fire)
			{
				HammerPush(S.m_aTee[1 - i].m_Core, S.m_aTee[i].m_Core.m_Pos);
				S.m_aTee[1 - i].m_FreezeTime = 0;
				S.m_aTee[i].m_Reload = 16;
			}
		S.Step(aI[0], aI[1]);
		Rest = t > 10 && AtRest(S, 0) && AtRest(S, 1) ? Rest + 1 : 0;
	}
	if(ms_DebugTeam)
		printf("dash best: puller %d jumps %d/%d hook at %d dir %d release %d hammer %d rehook %d airjump %d: ends %.1f %.1f / %.1f %.1f\n", BestP.m_Puller, BestP.m_aJump[0], BestP.m_aJump[1], BestP.m_HookAt,
			BestP.m_PullerDir, BestP.m_Release, BestP.m_Hammer, BestP.m_Rehook, BestP.m_AirJumpH, S.m_aTee[0].m_Core.m_Pos.x / 32, S.m_aTee[0].m_Core.m_Pos.y / 32, S.m_aTee[1].m_Core.m_Pos.x / 32,
			S.m_aTee[1].m_Core.m_Pos.y / 32);
	Best.m_Valid = true;
	Best.m_Score = BestValue;
	Best.m_aEnd[0] = S.m_aTee[0].m_Core.m_Pos;
	Best.m_aEnd[1] = S.m_aTee[1].m_Core.m_Pos;
	Best.m_Up = Base.InFreeze(Best.m_aEnd[0]) ? 0 : 1;
	Best.m_Steps = Steps;
	return Best;
}

// how far above Pos the freeze starts (px, up to 256; 256 if there is none or something solid comes first)
static float FreezeClearanceUp(const CHookBotSim &S, vec2 Pos)
{
	for(float d = 0; d < 256; d += 4)
	{
		const vec2 P = Pos - vec2(0, d);
		if(S.InFreeze(P))
			return d;
		if(S.m_pCollision->CheckPoint(P.x, P.y))
			return 256;
	}
	return 256;
}

// a hop up through a freeze ceiling right above us (Stronghold, the 1-tile tunnel under 2 rows of freeze at x 329-359:
// no room to jump below it). Rank 1: one jumps into the ceiling and rises through it frozen; the other jumps as it comes
// back down and hammers it free on its jump tick (its own middle still below the freeze); the freed one (back in the
// freeze, but a tee that froze less than a second ago doesn't freeze again before the second is over) air-jumps out
// above, hooks the other up through, hammers it free above the freeze, and they fly on (a pseudofly). Then where we come
// to rest, stopping the fly every 8 ticks
struct SHopParams
{
	int m_Jumper = 0;
	int m_JDir = 0; // the jumper's direction as it jumps
	int m_HJump = 40; // the tick the other one jumps
	int m_HDir = 0;
	int m_HFire = 0; // it hammers this many ticks after its jump
	int m_AirJump = 1; // the freed one air-jumps this many ticks after the hammer
};

static int FieldDir(const CHookBotGoalField &Field, vec2 P)
{
	const float L = Field.Dist(P + vec2(-48, -32)), R = Field.Dist(P + vec2(48, -32));
	return L < R - 8 ? -1 : R < L - 8 ? 1 : 0;
}

// a spot to wait at for the partner's fall through freeze, to hook it out as it comes by (Stronghold after the long shaft,
// rank 1 135-138 s: the one dragged through to the pocket under the block goes down the shaft beyond, swings left on the
// hookable blocks and stands on the block beside the freeze column the other comes down frozen, hooks it out as it
// passes and drags it onto the block; from the shaft's bottom, where my way on went, nobody gets anyone out of the
// column): the partner, standing free far behind me, walks the way its goal goes into the freeze and falls; where that
// fall goes 4 tiles down or more, the standing spot outside the freeze within 300 px of it with a clear line, one I can
// get to, nearest the fall. Checked every half second, or when the partner moves
bool CHookBotBrain::FindCatchSpot()
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	// (the partner behind me, standing or not: after the drag under the floor after Stronghold's long shaft, the one on the
	// ledge walked on down the shaft while the other was still on its way back to the gap in the air, its way through the
	// freeze pool not 320 longer than mine by the goal field)
	if(!m_PartnerIsBot || !m_pGoal || pU->m_FreezeTime > 0 || m_pGoal->Dist(pU->m_Core.m_Pos) < m_pGoal->Dist(pB->m_Core.m_Pos) + 160 || getenv("HH_NOCATCHSPOT"))
		return false;
	if(m_Now < m_CatchCheckAt && distance(pU->m_Core.m_Pos, m_CatchFor) < 32)
		return m_CatchSpot.x > -1e8f;
	m_CatchCheckAt = m_Now + SERVER_TICK_SPEED / 2;
	m_CatchFor = pU->m_Core.m_Pos;
	m_CatchSpot = vec2(-1e9f, -1e9f);
	CHookBotSim S;
	InitSim(S);
	const CNetObj_PlayerInput None = {};
	std::vector<vec2> vFall;
	float EnterY = 0;
	for(int t = 0; t < 400; t++)
	{
		const auto &T = S.m_aTee[1];
		CNetObj_PlayerInput In = {};
		In.m_TargetX = 1;
		if(T.m_FreezeTime == 0)
		{
			In.m_Direction = FieldDir(*m_pGoal, T.m_Core.m_Pos);
			// (it stands: no fall)
			if(!In.m_Direction && OnGroundAt(m_pCollision, T.m_Core.m_Pos))
				break;
		}
		S.Step(None, In, 1);
		if(T.m_Dead)
			return false;
		if(T.m_FreezeTime > 0)
		{
			if(vFall.empty())
				EnterY = T.m_Core.m_Pos.y;
			vFall.push_back(T.m_Core.m_Pos);
			if(vFall.size() > 150 || AtRest(S, 1))
				break;
		}
		else if(!vFall.empty())
			break;
	}
	float BestD = 1e9f;
	for(size_t k = 0; k < vFall.size(); k += 2)
	{
		const vec2 P = vFall[k];
		if(P.y < EnterY + 4 * 32)
			continue;
		const int px = (int)std::floor(P.x / 32), py = (int)std::floor(P.y / 32);
		for(int y = py - 9; y <= py + 9; y++)
			for(int x = px - 9; x <= px + 9; x++)
			{
				const vec2 C(x * 32 + 16, y * 32 + 16);
				if(m_pCollision->CheckPoint(C) || !m_pCollision->CheckPoint(C + vec2(0, 32)) || S.InFreeze(C))
					continue;
				const vec2 Stand(C.x, (y + 1) * 32 - CCharacterCore::PhysicalSize() / 2);
				const float D = distance(Stand, P);
				if(D > 300 || D >= BestD || S.InFreeze(Stand) || m_pCollision->IntersectLine(Stand, P, nullptr, nullptr) || !CanReach(S, pB->m_Core.m_Pos, Stand))
					continue;
				BestD = D;
				m_CatchSpot = Stand;
			}
	}
	if(m_CatchSpot.x > -1e8f && (!m_pCatchField || distance(m_CatchSpot, m_CatchFieldAt) > 1))
	{
		auto pField = std::make_shared<CHookBotGoalField>();
		pField->Build(m_pCollision, vec2(std::floor(m_CatchSpot.x / 32), std::floor(m_CatchSpot.y / 32)), 40);
		m_pCatchField = pField;
		m_CatchFieldAt = m_CatchSpot;
	}
	if(ms_DebugTeam)
		printf("catch spot t%d id %d: partner %.1f %.1f, fall %d ticks from %.1f, spot %.1f %.1f (%.0f px from the fall)\n", m_Now, pB->m_Core.m_Id, pU->m_Core.m_Pos.x / 32, pU->m_Core.m_Pos.y / 32, (int)vFall.size(),
			EnterY / 32, m_CatchSpot.x / 32, m_CatchSpot.y / 32, BestD);
	return m_CatchSpot.x > -1e8f;
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanHop(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps, const SPseudoParams &Pseudo)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_HOP;
	int Steps = 0;
	const float aD0[2] = {Field.Dist(Base.m_aTee[0].m_Core.m_Pos), Field.Dist(Base.m_aTee[1].m_Core.m_Pos)};
	struct SRun
	{
		CHookBotSim m_S;
		std::vector<CNetObj_PlayerInput> m_avIn[2];
		std::vector<vec2> m_avPath[2];
		void Step(const CNetObj_PlayerInput aI[2])
		{
			for(int i = 0; i < 2; i++)
			{
				m_avPath[i].push_back(m_S.m_aTee[i].m_Core.m_Pos);
				m_avIn[i].push_back(aI[i]);
			}
			for(int i = 0; i < 2; i++)
				if(aI[i].m_Fire)
				{
					HammerPush(m_S.m_aTee[1 - i].m_Core, m_S.m_aTee[i].m_Core.m_Pos);
					m_S.m_aTee[1 - i].m_FreezeTime = 0;
					m_S.m_aTee[i].m_Reload = 16;
				}
			m_S.Step(aI[0], aI[1]);
		}
	};
	// to rest from R: each steers the way the field goes down, with its air jump once falling or not; the plan if it's
	// the best so far
	float BestValue = 300;
	int Tried = 0, Freed = 0, Coasts = 0;
	auto TryRest = [&](const SRun &R) {
		for(int Jumps = 0; Jumps < 2; Jumps++)
		{
			SRun C = R;
			int Rest = 0;
			for(int t = 0; t < 200 && Rest < 2; t++)
			{
				CNetObj_PlayerInput aI[2] = {};
				for(int i = 0; i < 2; i++)
				{
					const auto &T = C.m_S.m_aTee[i];
					aI[i].m_TargetX = 1;
					if(T.m_FreezeTime > 0 || OnGroundAt(C.m_S.m_pCollision, T.m_Core.m_Pos))
						continue;
					aI[i].m_Direction = FieldDir(Field, T.m_Core.m_Pos);
					aI[i].m_Jump = Jumps && !(T.m_Core.m_Jumped & 2) && T.m_Core.m_Vel.y > 2;
				}
				C.Step(aI);
				Steps++;
				Rest = AtRest(C.m_S, 0) && AtRest(C.m_S, 1) ? Rest + 1 : 0;
				if(C.m_S.m_aTee[0].m_Dead || C.m_S.m_aTee[1].m_Dead)
					break;
			}
			Coasts++;
			float Value;
			if(Rest < 2 || !TeamEndValue(C.m_S, Field, aD0, &Value))
				continue;
			Value -= 0.5f * C.m_avIn[0].size();
			if(Value > BestValue)
			{
				BestValue = Value;
				Best.m_Valid = true;
				Best.m_Score = Value;
				for(int i = 0; i < 2; i++)
				{
					Best.m_avIn[i] = C.m_avIn[i];
					Best.m_avPath[i] = C.m_avPath[i];
					Best.m_aEnd[i] = C.m_S.m_aTee[i].m_Core.m_Pos;
				}
			}
		}
	};
	for(int Jumper = 0; Jumper < 2; Jumper++)
		for(int JDir = -1; JDir <= 1; JDir++)
			for(int Under = 0; Under < 2; Under++)
		{
			const int j = Jumper, h = 1 - j;
			// the jump, the other one standing or walking right under it (the hammer only reaches straight up: rank 1
			// stood 1 px apart): the states before each tick
			std::vector<SRun> vPre;
			{
				SRun R;
				R.m_S = Base;
				for(int t = 0; t <= 70; t++)
				{
					vPre.push_back(R);
					CNetObj_PlayerInput aI[2] = {};
					aI[0].m_TargetX = aI[1].m_TargetX = 1;
					aI[j].m_Jump = t == 0;
					aI[j].m_Direction = R.m_S.m_aTee[j].m_FreezeTime > 0 ? 0 : JDir;
					const auto &J = R.m_S.m_aTee[j], &H = R.m_S.m_aTee[h];
					if(Under && J.m_Core.m_Pos.y < H.m_Core.m_Pos.y - 40)
						aI[h].m_Direction = SteerTo(J.m_Core.m_Pos.x, H.m_Core.m_Pos.x, H.m_Core.m_Vel.x);
					R.Step(aI);
					Steps++;
				}
			}
			// (by the tick: the freed one has to be just above the freeze, within 7 px of it, for the one tick the other
			// one can still hammer, one tick into the freeze after its jump; the freeze takes a tee a tick late)
			for(int HJump = 16; HJump <= 70; HJump++)
				for(int HDir = -1; HDir <= 1; HDir++)
					for(int HFire = 0; HFire <= 2; HFire++)
						for(int AirJump : {0, 2, 5})
						{
							if(Steps >= MaxSteps)
								break;
							Tried++;
							SRun R = vPre[HJump];
							// the other one jumps and hammers (within reach of the hammer, before the freeze has it)
							bool Hit = false;
							for(int k = 0; k <= HFire; k++)
							{
								CNetObj_PlayerInput aI[2] = {};
								aI[0].m_TargetX = aI[1].m_TargetX = 1;
								auto &J = R.m_S.m_aTee[j], &H = R.m_S.m_aTee[h];
								aI[h].m_Jump = k == 0;
								aI[h].m_Direction = HDir;
								Aim(aI[h], J.m_Core.m_Pos - H.m_Core.m_Pos);
								if(k == HFire && H.m_FreezeTime == 0 && H.m_Reload == 0 && J.m_FreezeTime > 0 && HammerReaches(H.m_Core.m_Pos, J.m_Core.m_Pos) &&
									!R.m_S.m_pCollision->IntersectLine(H.m_Core.m_Pos, J.m_Core.m_Pos, nullptr, nullptr))
								{
									aI[h].m_Fire = 1;
									Hit = true;
								}
								R.Step(aI);
								Steps++;
							}
							if(getenv("HH_HOPDBG") && HDir == 0 && AirJump == 0)
								printf("hopdbg j %d jdir %d hjump %d hfire %d: J %.1f %.1f frz %d, H %.1f %.1f frz %d, dist %.1f: %s\n", j, JDir, HJump, HFire, R.m_S.m_aTee[j].m_Core.m_Pos.x, R.m_S.m_aTee[j].m_Core.m_Pos.y,
									R.m_S.m_aTee[j].m_FreezeTime, R.m_S.m_aTee[h].m_Core.m_Pos.x, R.m_S.m_aTee[h].m_Core.m_Pos.y, R.m_S.m_aTee[h].m_FreezeTime,
									distance(R.m_S.m_aTee[j].m_Core.m_Pos, R.m_S.m_aTee[h].m_Core.m_Pos), Hit ? "HIT" : "-");
							if(!Hit)
								continue;
							// the freed one: out above, hooking the other up; it hammers it free once it's out of the
							// freeze; then the pseudofly
							SFlyState F;
							F.m_Driver = j;
							int aPrevHook[2] = {0, 0};
							bool BothFree = false, Lost = false;
							static int s_Traced = 0;
							const bool Trace = getenv("HH_HOPDBG") && getenv("HH_HOPDBG")[0] == '3' && R.m_S.m_aTee[j].m_FreezeTime == 0 && R.m_S.m_aTee[j].m_Core.m_Pos.y < 7232 && s_Traced++ < 3;
							for(int t = 0; t < 200 && !Lost; t++)
							{
								auto &J = R.m_S.m_aTee[j], &H = R.m_S.m_aTee[h];
								if(!BothFree)
								{
									if(J.m_FreezeTime > 0 && H.m_FreezeTime > 0)
									{
										Lost = true;
										break;
									}
									CNetObj_PlayerInput aI[2] = {};
									aI[0].m_TargetX = aI[1].m_TargetX = 1;
									if(J.m_FreezeTime == 0)
									{
										Aim(aI[j], H.m_Core.m_Pos - J.m_Core.m_Pos);
										aI[j].m_Hook = HookBotHookInput(H.m_Core.m_Pos.y > J.m_Core.m_Pos.y + 10, aPrevHook[j], J.m_Core.m_HookState);
										aI[j].m_Jump = t == AirJump;
										aI[j].m_Direction = FieldDir(Field, J.m_Core.m_Pos);
										if(J.m_Reload == 0 && H.m_FreezeTime > 0 && !R.m_S.InFreeze(H.m_Core.m_Pos) && HammerReaches(J.m_Core.m_Pos, H.m_Core.m_Pos) &&
											!R.m_S.m_pCollision->IntersectLine(H.m_Core.m_Pos, J.m_Core.m_Pos, nullptr, nullptr))
											aI[j].m_Fire = 1;
									}
									aPrevHook[j] = aI[j].m_Hook;
									if(Trace)
										printf("hoptrace %d: J %.1f %.1f v %.1f %.1f frz %d hook %d jumped %d | H %.1f %.1f v %.1f %.1f frz %d | in J dir %d jump %d hook %d fire %d\n", t, J.m_Core.m_Pos.x, J.m_Core.m_Pos.y, J.m_Core.m_Vel.x,
											J.m_Core.m_Vel.y, J.m_FreezeTime, J.m_Core.m_HookState, J.m_Core.m_Jumped, H.m_Core.m_Pos.x, H.m_Core.m_Pos.y, H.m_Core.m_Vel.x, H.m_Core.m_Vel.y, H.m_FreezeTime, aI[j].m_Direction,
											aI[j].m_Jump, aI[j].m_Hook, aI[j].m_Fire);
									R.Step(aI);
									Steps++;
									if(R.m_S.m_aTee[0].m_FreezeTime == 0 && R.m_S.m_aTee[1].m_FreezeTime == 0 && !R.m_S.InFreeze(R.m_S.m_aTee[0].m_Core.m_Pos) &&
										!R.m_S.InFreeze(R.m_S.m_aTee[1].m_Core.m_Pos))
									{
										BothFree = true;
										Freed++;
										F.m_aPrevHook[j] = aPrevHook[j];
									}
									continue;
								}
								if(J.m_FreezeTime > 0 || H.m_FreezeTime > 0)
								{
									if(getenv("HH_HOPDBG") && getenv("HH_HOPDBG")[0] == '2')
										printf("hopfly j %d jdir %d under %d hjump %d hdir %d hfire %d air %d: lost at %d, J %.1f %.1f v %.1f %.1f frz %d, H %.1f %.1f v %.1f %.1f frz %d\n", j, JDir, Under, HJump, HDir, HFire, AirJump, t,
											J.m_Core.m_Pos.x / 32, J.m_Core.m_Pos.y / 32, J.m_Core.m_Vel.x, J.m_Core.m_Vel.y, J.m_FreezeTime, H.m_Core.m_Pos.x / 32, H.m_Core.m_Pos.y / 32, H.m_Core.m_Vel.x, H.m_Core.m_Vel.y, H.m_FreezeTime);
									break;
								}
								if(t % 8 == 0)
									TryRest(R);
								CNetObj_PlayerInput aI[2];
								const int d = R.m_S.m_aTee[0].m_Core.m_Pos.y < R.m_S.m_aTee[1].m_Core.m_Pos.y ? 0 : 1;
								const vec2 aBefore[2] = {R.m_S.m_aTee[0].m_Core.m_Pos, R.m_S.m_aTee[1].m_Core.m_Pos};
								FlyStep(R.m_S, F, Pseudo, FieldDir(Field, R.m_S.m_aTee[d].m_Core.m_Pos), aI);
								for(int i = 0; i < 2; i++)
								{
									R.m_avPath[i].push_back(aBefore[i]);
									R.m_avIn[i].push_back(aI[i]);
								}
								Steps++;
							}
						}
		}
	if(ms_DebugTeam)
		printf("hop: %d tried, %d got both free, %d coasts, best %.0f, %d ticks\n", Tried, Freed, Coasts, BestValue, Steps);
	if(Best.m_Valid)
		Best.m_Up = Base.InFreeze(Best.m_aEnd[0]) ? 0 : 1;
	Best.m_Steps = Steps;
	return Best;
}

// a hookfly between the two of us, for the planners: the upper one hooks the lower one up once its own rise slows
// (Wait), and lets go once the lower one has passed it (Release px above); each air-jumps falling faster than AirJumpVy,
// and steers the way Dir says
struct SHookflyPol
{
	float m_Wait = 8;
	int m_Release = -10;
	float m_AirJumpVy = 4;
};
template<typename TDir>
static void HookflyPlanStep(CHookBotSim &S, const SHookflyPol &P, int aPrevHook[2], TDir Dir, CNetObj_PlayerInput aI[2])
{
	for(int i = 0; i < 2; i++)
	{
		const auto &Me = S.m_aTee[i], &O = S.m_aTee[1 - i];
		aI[i] = {};
		aI[i].m_TargetX = 1;
		if(Me.m_FreezeTime > 0)
		{
			aPrevHook[i] = 0;
			continue;
		}
		const vec2 B = Me.m_Core.m_Pos, U = O.m_Core.m_Pos;
		const float Dy = U.y - B.y, Dist = distance(B, U);
		const bool IHook = Me.m_Core.HookedPlayer() == O.m_Core.m_Id && Me.m_Core.m_HookState == HOOK_GRABBED;
		const bool HooksMe = O.m_Core.HookedPlayer() == Me.m_Core.m_Id && O.m_Core.m_HookState == HOOK_GRABBED;
		const bool Ground = OnGroundAt(S.m_pCollision, B);
		bool Want = false;
		if(IHook)
			Want = Dy > -P.m_Release;
		else if(Me.m_Core.m_HookState == HOOK_FLYING && aPrevHook[i])
			Want = Dy > 0;
		else if(!HooksMe && !Ground && Dy > 10 && Me.m_Core.m_Vel.y > -P.m_Wait && Dist < 360)
			Want = true;
		aI[i].m_Hook = HookBotHookInput(Want, aPrevHook[i], Me.m_Core.m_HookState);
		Aim(aI[i], U + O.m_Core.m_Vel * (Dist / 80.0f) - B);
		// as the live hookfly: the hooker moves away from the other one (a longer rope pulls harder), the one pulled
		// steps aside when it's right below me, and towards it when far off; else the way the air goes
		const float Dx = U.x - B.x;
		const int Away = B.x >= U.x ? 1 : -1;
		aI[i].m_Direction = Want ? Away : 0;
		if(HooksMe && absolute(Dx) < 48)
			aI[i].m_Direction = Away;
		if(absolute(Dx) > 120)
			aI[i].m_Direction = Dx > 0 ? 1 : -1;
		if(!aI[i].m_Direction)
			aI[i].m_Direction = Dir(B);
		if(!Ground && !(Me.m_Core.m_Jumped & 2) && Me.m_Core.m_Vel.y > P.m_AirJumpVy)
			aI[i].m_Jump = 1;
		aPrevHook[i] = aI[i].m_Hook;
	}
	S.Step(aI[0], aI[1]);
}

// a climb as rank 1 goes up Stronghold's unhookable shaft (112-114 s): the upper one keeps the lower one on its hook
// (the rope drags it up at 15 px/tick as long as it's longer than about 42 px, where the pull beats gravity), and the
// lower one, catching up, hammers the upper one up again (-11 px/tick each time, every 20-25 ticks). Whoever is higher
// is the upper one; the lower one steers under it
struct SClimbPol
{
	int m_UDir = 0; // the upper one steers: 0 straight up whenever that gets as close as a side, 1 the way the air goes, 2
			// the way the goal goes (through freeze too: above Stronghold's shaft the air ends over the freeze strip
			// into the corridor, and rank 1 drops through it)
	float m_Dv = -100; // the lower one hammers once the upper one rises slower than it by this much (px/tick)
	int m_Dead = 6; // the lower one doesn't steer within this many px of under the upper one
	float m_UJumpVy = 1000; // the upper one air-jumps once falling faster than this
	float m_LJumpVy = 1000; // the lower one too
	float m_Release = 0; // the upper one lets go once the lower one is closer than this
	float m_Lag = 0; // the lower one keeps this far behind the upper one the way it steers (rank 1 climbs diagonally: rope
			 // and hammer push sideways too, 12-14 px/tick; steering in the air alone goes 5)
	bool m_UpOnly = false; // the lower one hammers only where climbing gets the upper one closer through the air (rank 1
			       // crosses the 10 rows under the freeze column after Stronghold's striped room sideways and low, and
			       // climbs from the shaft beside it; hammered up all the way, the upper one hit the column's foot)
};
template<typename TDir>
static void ClimbPolStep(CHookBotSim &S, const SClimbPol &P, int aPrevHook[2], int aPrevFire[2], TDir UDir, CNetObj_PlayerInput aI[2], const CHookBotGoalField *pAir = nullptr)
{
	const int u = S.m_aTee[0].m_Core.m_Pos.y <= S.m_aTee[1].m_Core.m_Pos.y ? 0 : 1, l = 1 - u;
	auto &U = S.m_aTee[u], &L = S.m_aTee[l];
	const vec2 Up = U.m_Core.m_Pos, Lp = L.m_Core.m_Pos;
	const float Dist = distance(Up, Lp);
	aI[0] = aI[1] = {};
	aI[0].m_TargetX = aI[1].m_TargetX = 1;
	int Ux = 0;
	if(U.m_FreezeTime == 0)
	{
		const bool Ground = OnGroundAt(S.m_pCollision, Up);
		aI[u].m_Hook = HookBotHookInput(!Ground && Lp.y > Up.y + 10 && Dist < 360 && Dist >= P.m_Release, aPrevHook[u], U.m_Core.m_HookState);
		Aim(aI[u], Lp - Up);
		aI[u].m_Direction = UDir(Up);
		Ux = aI[u].m_Direction;
		if(!Ground && !(U.m_Core.m_Jumped & 2) && U.m_Core.m_Vel.y > P.m_UJumpVy)
			aI[u].m_Jump = 1;
	}
	if(L.m_FreezeTime == 0)
	{
		const bool Ground = OnGroundAt(S.m_pCollision, Lp);
		Aim(aI[l], Up - Lp);
		const float Err = Up.x - Ux * P.m_Lag - Lp.x + (U.m_Core.m_Vel.x - L.m_Core.m_Vel.x) * 4;
		aI[l].m_Direction = Err > P.m_Dead ? 1 : Err < -P.m_Dead ? -1 : 0;
		if(!Ground && !(L.m_Core.m_Jumped & 2) && L.m_Core.m_Vel.y > P.m_LJumpVy)
			aI[l].m_Jump = 1;
		const float NextDist = distance(Up + U.m_Core.m_Vel, Lp + L.m_Core.m_Vel);
		const bool UpHelps = !P.m_UpOnly || !pAir || pAir->Dist(Up - vec2(0, 96)) < pAir->Dist(Up) - 40;
		if(L.m_Reload == 0 && !aPrevFire[l] && U.m_FreezeTime == 0 && HammerReaches(Lp, Up) && Up.y < Lp.y - 10 && UpHelps &&
			(U.m_Core.m_Vel.y - L.m_Core.m_Vel.y > P.m_Dv || NextDist < 38) && !S.m_pCollision->IntersectLine(Lp, Up, nullptr, nullptr))
		{
			aI[l].m_Fire = 1;
			HammerPush(U.m_Core, Lp);
			L.m_Reload = 16;
		}
	}
	for(int i = 0; i < 2; i++)
	{
		aPrevHook[i] = aI[i].m_Hook;
		aPrevFire[i] = aI[i].m_Fire;
	}
	S.Step(aI[0], aI[1]);
}

// something hookable within reach of P (a clear line to it), at least Above px higher than P
static bool HookableNear(const CHookBotSim &S, vec2 P, float Reach, float Above)
{
	const CCollision *pCol = S.m_pCollision;
	const int px = (int)(P.x / 32), py = (int)(P.y / 32), r = (int)(Reach / 32) + 1;
	for(int y = py - r; y <= py + r; y++)
		for(int x = px - r; x <= px + r; x++)
		{
			const vec2 C(x * 32 + 16, y * 32 + 16);
			if(distance(C, P) > Reach || C.y > P.y - Above || pCol->GetTile(C.x, C.y) != TILE_SOLID)
				continue;
			vec2 Hit;
			if(pCol->IntersectLine(P, C, &Hit, nullptr) == TILE_SOLID)
				return true;
		}
	return false;
}

// a climb (Stronghold, the unhookable shaft at x 179-187 after the hookable blocks: both of us swung there on our own,
// and a live fly started as we were already falling apart): a pseudofly from where we are (FlyStep, the upper one
// driving along Air), stopped every 6 ticks: to rest from there (TeamEndValue), or the plan ends in the air, where each of
// us can swing on by itself (free, 8 px clear of the freeze, something hookable within 340 px) at least 300 closer.
// The best of both drivers
CHookBotBrain::STeamPlan CHookBotBrain::PlanClimb(const CHookBotSim &Base, const CHookBotGoalField &Field, const CHookBotGoalField &Air, int MaxSteps, const SPseudoParams &Pseudo, int MaxTicks)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_CLIMB;
	int Steps = 0;
	const float aD0[2] = {Field.Dist(Base.m_aTee[0].m_Core.m_Pos), Field.Dist(Base.m_aTee[1].m_Core.m_Pos)};
	const bool StartFrozen = Base.m_aTee[0].m_FreezeTime > 0 || Base.m_aTee[1].m_FreezeTime > 0;
	float BestValue = 300;
	auto AirDir = [&](vec2 P) {
		const float L = Air.Dist(P + vec2(-48, -32)), R = Air.Dist(P + vec2(48, -32));
		return L >= 1e5f && R >= 1e5f ? 0 : L < R - 8 ? -1 : R < L - 8 ? 1 : 0;
	};
	// (or straight up whenever that gets as close as a side: in Stronghold's shaft the air goes up-left over its left wall,
	// and the driver pressed against that wall all the way up, 3 px/tick; rank 1 climbs it at 10)
	auto UpDir = [&](vec2 P) {
		const float L = Air.Dist(P + vec2(-48, -32)), R = Air.Dist(P + vec2(48, -32)), Up = Air.Dist(P + vec2(0, -48));
		if(Up < 1e5f && Up <= std::min(L, R) + 8)
			return 0;
		return L >= 1e5f && R >= 1e5f ? 0 : L < R - 8 ? -1 : R < L - 8 ? 1 : 0;
	};
	// the standing spot nearest the goal, within 5 tiles of it, outside the freeze (the to-rest coasts may steer to it: at the
	// top of the unhookable shaft after Stronghold's striped room the goal hangs in the air, and the one place to stand is
	// the edge of the column top beside the freeze window, a few px wide; steering by the fields, every coast fell back
	// down the shaft)
	// (two of them a tee apart, one each: the spot there holds one of us, and rank 1's other one goes through the window
	// and stands on the column's far edge)
	float aStandX[2] = {-1, -1};
	{
		const vec2 G = Field.m_GoalTile * 32 + vec2(16, 16);
		const float h = CCharacterCore::PhysicalSize() / 2;
		std::vector<std::pair<float, float>> vSpots; // distance to the goal, x
		for(int ty = -5; ty <= 5; ty++)
			for(int qx = -24; qx <= 24; qx++)
			{
				const vec2 P(G.x + qx * 8.0f, (std::floor(G.y / 32) + ty + 1) * 32 - h);
				if(Base.m_pCollision->CheckPoint(P + vec2(h, 0)) || Base.m_pCollision->CheckPoint(P - vec2(h, 0)) || Base.m_pCollision->CheckPoint(P - vec2(0, h)) || Base.InFreeze(P))
					continue;
				if(!Base.m_pCollision->CheckPoint(P + vec2(h - 1, h + 2)) && !Base.m_pCollision->CheckPoint(P + vec2(-h + 1, h + 2)))
					continue;
				vSpots.push_back({distance(P, G), P.x});
			}
		std::sort(vSpots.begin(), vSpots.end());
		for(const auto &Sp : vSpots)
		{
			if(aStandX[0] < 0)
				aStandX[0] = Sp.second;
			else if(absolute(Sp.second - aStandX[0]) >= 32)
			{
				aStandX[1] = Sp.second;
				break;
			}
		}
		if(aStandX[1] < 0)
			aStandX[1] = aStandX[0];
	}
	const float StandX = aStandX[0];
	auto Record = [&](const std::vector<CNetObj_PlayerInput> *pavIn, const std::vector<vec2> *pavPath, const CHookBotSim &S, float Value) {
		BestValue = Value;
		Best.m_Valid = true;
		Best.m_Score = Value;
		for(int i = 0; i < 2; i++)
		{
			Best.m_avIn[i] = pavIn[i];
			Best.m_avPath[i] = pavPath[i];
			Best.m_aEnd[i] = S.m_aTee[i].m_Core.m_Pos;
		}
	};
	// the styles: a pseudofly with either of us driving; a hookfly (rank 1 climbs Stronghold's unhookable shaft so: the
	// upper one hooks the lower one up, and they take turns)
	static const SHookflyPol s_aHookfly[] = {{12, 0, 1000}, {12, 0, 6}, {8, 0, 1000}, {12, -20, 1000}, {16, 0, 1000}, {12, 20, 6}, {40, 0, 1000}, {40, -30, 4}, {4, 0, 2}};
	const int NumPseudo = 4, NumStyles = NumPseudo + (int)std::size(s_aHookfly);
	SPseudoParams ClimbPseudo = Pseudo;
	if(const char *pEnv = getenv("HH_CLIMBPSEUDO"))
	{
		int CloseOnly = 0;
		sscanf(pEnv, "%f,%f,%f,%d,%f,%f,%d,%f,%f,%d,%f", &ClimbPseudo.m_FireDist, &ClimbPseudo.m_MinDy, &ClimbPseudo.m_Lead, &ClimbPseudo.m_Dead, &ClimbPseudo.m_ReleaseDist,
			&ClimbPseudo.m_RehookDist, &ClimbPseudo.m_MaxFree, &ClimbPseudo.m_MaxVx, &ClimbPseudo.m_MaxBehind, &CloseOnly, &ClimbPseudo.m_AirJumpVy);
		ClimbPseudo.m_CloseOnly = CloseOnly;
	}
	// then the climb policies: all of them screened first (how close the fly gets us, both free and clear of the freeze,
	// no ends tried), the best few tried as the styles above
	std::vector<SClimbPol> vPols;
	// (the lower one's air jump made no difference in any of them)
	for(int UDir = 0; UDir < 3; UDir++)
		for(float Dv : {-100.0f, 0.0f, 3.0f})
			for(int Dead : {6, 16})
				for(float UJump : {1000.0f, 0.0f, 4.0f})
					for(float LJump : {1000.0f})
						for(float Release : {0.0f, 40.0f})
							for(float Lag : {0.0f, 40.0f, 80.0f})
						{
							SClimbPol Pol;
							Pol.m_UDir = UDir;
							Pol.m_Dv = Dv;
							Pol.m_Dead = Dead;
							Pol.m_UJumpVy = UJump;
							Pol.m_LJumpVy = LJump;
							Pol.m_Release = Release;
							Pol.m_Lag = Lag;
							vPols.push_back(Pol);
							// (and hammering only where climbing helps, steering by the air or the goal, hammering any time)
							if(UDir >= 1 && Dv < -50)
							{
								Pol.m_UpOnly = true;
								vPols.push_back(Pol);
							}
						}
	std::vector<float> vPolScore(vPols.size(), -1e9f);
	std::vector<int> vSel;
	const int NumSel = getenv("HH_CLIMBSEL") ? atoi(getenv("HH_CLIMBSEL")) : 6;
	for(int Pass = 0; Pass < 2; Pass++)
	{
	if(Pass == 1)
	{
		std::vector<int> vOrder(vPols.size());
		for(int i = 0; i < (int)vOrder.size(); i++)
			vOrder[i] = i;
		std::stable_sort(vOrder.begin(), vOrder.end(), [&](int a, int b) { return vPolScore[a] > vPolScore[b]; });
		for(int i = 0; i < NumSel && i < (int)vOrder.size(); i++)
			if(vPolScore[vOrder[i]] > 0)
				vSel.push_back(vOrder[i]);
		if(getenv("HH_CLIMBDBG"))
			for(int i : vSel)
				printf("climbdbg policy %d (udir %d dv %.0f dead %d ujump %.0f ljump %.0f release %.0f lag %.0f): %.0f\n", i, vPols[i].m_UDir, vPols[i].m_Dv, vPols[i].m_Dead, vPols[i].m_UJumpVy,
					vPols[i].m_LJumpVy, vPols[i].m_Release, vPols[i].m_Lag, vPolScore[i]);
	}
	// (in the second pass the chosen policies first: the budget may run out)
	// (the hookfly styles only on request, HH_CLIMBHOOKFLY: they never got far up the shaft)
	const int NumAll = Pass == 0 ? (int)vPols.size() : (int)vSel.size() + (getenv("HH_CLIMBHOOKFLY") ? NumStyles : NumPseudo);
	for(int Run = 0; Run < NumAll; Run++)
		for(int Wait = 0; Wait <= 8; Wait += 4)
			for(float RescueAbove : {32.0f, 0.0f, 64.0f})
		{
			// (starting with both of us free, the rescue variants are all the same; starting with one of us frozen, the
			// waits are: the rescue comes first)
			if(!StartFrozen && RescueAbove != 32.0f)
				continue;
			if(StartFrozen && Wait > 0)
				continue;
			if(Pass == 0 && (Wait > 0 || RescueAbove != 32.0f))
				continue;
			const int PolIdx = Pass == 0 ? Run : Run < (int)vSel.size() ? vSel[Run] : -1;
			const int Style = PolIdx >= 0 ? NumStyles + Run : Run - (int)vSel.size();
			CHookBotSim S = Base;
			std::vector<CNetObj_PlayerInput> avIn[2];
			std::vector<vec2> avPath[2];
			SFlyState F;
			F.m_Driver = Style % 2;
			int aPrevHook[2], aPrevFire[2];
			for(int i = 0; i < 2; i++)
			{
				aPrevHook[i] = F.m_aPrevHook[i] = Base.m_aTee[i].m_PrevInput.m_Hook;
				aPrevFire[i] = Base.m_aTee[i].m_PrevInput.m_Fire & 1;
			}
			float MinY = 1e9f;
			int Stop = -1;
			bool Rescued = false;
			// points where the plan may end mid-climb (the next climb goes on from there): close together, both free, clear
			// of the freeze, if the climb went on 60 ticks clean after it
			struct SMid
			{
				int m_T;
				float m_Value;
				vec2 m_aPos[2];
			};
			std::vector<SMid> vMid;
			for(int t = 0; t < MaxTicks && Steps < MaxSteps; t++)
			{
				MinY = std::min(MinY, std::max(S.m_aTee[0].m_Core.m_Pos.y, S.m_aTee[1].m_Core.m_Pos.y));
				Stop = t;
				const bool aFrz[2] = {S.m_aTee[0].m_FreezeTime > 0, S.m_aTee[1].m_FreezeTime > 0};
				if((aFrz[0] && aFrz[1]) || S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead)
					break;
				// (a rescue only at the start, from one of us frozen; a freeze after that ends it)
				if((aFrz[0] || aFrz[1]) && (!StartFrozen || Rescued))
					break;
				if(Pass == 0 && t >= 12 && !aFrz[0] && !aFrz[1])
				{
					const vec2 P0 = S.m_aTee[0].m_Core.m_Pos, P1 = S.m_aTee[1].m_Core.m_Pos;
					if(!NearFreeze(S, P0, P0) && !NearFreeze(S, P1, P1))
						vPolScore[PolIdx] = std::max(vPolScore[PolIdx], aD0[0] + aD0[1] - Field.Dist(P0) - Field.Dist(P1) - 0.5f * t);
				}
				// (every 3 ticks while one of us flies fast just above freeze: the way from the top of Stronghold's shaft into
				// the corridor is through the freeze strip at x 149-154 in a window 1.5 rows high, at speed; every 12 ticks
				// missed it)
				bool DropWindow = false;
				for(int i = 0; i < 2; i++)
					DropWindow |= !aFrz[i] && absolute(S.m_aTee[i].m_Core.m_Vel.x) > 6 && FreezeClearance(S, S.m_aTee[i].m_Core.m_Pos) < 96;
				if(Pass == 1 && t >= 12 && t % 6 == 0 && !aFrz[0] && !aFrz[1] && (!StartFrozen || Rescued))
				{
					const vec2 P0 = S.m_aTee[0].m_Core.m_Pos, P1 = S.m_aTee[1].m_Core.m_Pos;
					const float G0 = aD0[0] - Field.Dist(P0), G1 = aD0[1] - Field.Dist(P1);
					// (with something to swing on above each of us too, if no climb goes on from there: one ended at the corner
					// of the freeze strip at the top of Stronghold's shaft, 340 px from the hookable ceiling, and both fell onto
					// the freeze floor; where one of us stays up swinging on the ceiling, it gets the other through the strip)
					if(distance(P0, P1) < 4 * 32 && std::min(G0, G1) > 150 && !NearFreeze(S, P0, P0) && !NearFreeze(S, P1, P1) && HookableNear(S, P0, 300, 64) &&
						HookableNear(S, P1, 300, 64))
						vMid.push_back({t, G0 + G1 - 0.5f * t, {P0, P1}});
				}
				if(Pass == 1 && t >= 12 && (t % 6 == 0 || (t % 3 == 0 && DropWindow)))
				{
					// an end in the air (every 6 ticks; to rest from there every 12)
					bool Ok = !aFrz[0] && !aFrz[1];
					for(int i = 0; i < 2 && Ok; i++)
					{
						const vec2 P = S.m_aTee[i].m_Core.m_Pos;
						// (something to swing on above me: one below it was no use after a climb; within 300 px: at the top of
						// Stronghold's shaft it let go of us 350 px from the hookable ceiling, and the solo searches, which
						// aim within 360, found nothing; both fell onto the freeze floor)
						Ok = !NearFreeze(S, P, P) && FreezeClearance(S, P) > 8 && HookableNear(S, P, 300, 64);
						// (and 48 px clear of the freeze above me: one let go 19 px under the freeze band at the top of
						// Stronghold's swing course swung up into it at once)
						for(float dy = 14; dy <= 62 && Ok; dy += 8)
							for(float dx : {-14.0f, 0.0f, 14.0f})
								Ok = Ok && !S.InFreeze(P + vec2(dx, -dy));
						// (and time to: holding some direction, 20 ticks clear of the freeze; a hand-off in Stronghold's swing
						// course let go of one of us rising at 9 px/tick right under a freeze band, and it froze in it; with 12,
						// one at the top of the shaft let go of us 3 tiles above the freeze strip into the corridor, and we
						// both drifted onto it)
						bool Clear = false;
						for(int Dir = -1; Dir <= 1 && Ok && !Clear; Dir++)
						{
							CHookBotSim C = S;
							CNetObj_PlayerInput aC[2] = {};
							aC[0].m_TargetX = aC[1].m_TargetX = 1;
							aC[i].m_Direction = Dir;
							Clear = true;
							for(int k = 0; k < 20 && Clear; k++)
							{
								const vec2 Q = C.m_aTee[i].m_Core.m_Pos;
								C.Step(aC[0], aC[1]);
								Steps++;
								Clear = !C.m_aTee[i].m_EnteredFreeze && !C.m_aTee[i].m_Dead && !NearFreeze(C, Q, C.m_aTee[i].m_Core.m_Pos);
							}
						}
						Ok = Ok && Clear;
					}
					float Value = aD0[0] + aD0[1] - Field.Dist(S.m_aTee[0].m_Core.m_Pos) - Field.Dist(S.m_aTee[1].m_Core.m_Pos) - 0.5f * t;
					if(Ok && std::min(aD0[0] - Field.Dist(S.m_aTee[0].m_Core.m_Pos), aD0[1] - Field.Dist(S.m_aTee[1].m_Core.m_Pos)) > 150 && Value > BestValue)
						Record(avIn, avPath, S, Value);
					// to rest from here: each steering the way the open air goes, or the way the goal goes (through
					// freeze too: after Stronghold's unhookable shaft, rank 1 drops into the corridor through the freeze
					// at x 148-153 and both lie there frozen until they thaw)
					// (2, 3: as 0, 1, with the air jump once falling: at the top of Stronghold's shaft we fly left at 10
					// px/tick 5 tiles above the freeze floor, and coasting on we came down on it 8 tiles short of the
					// freeze strip rank 1 drops into the corridor through)
					for(int Steer = 0; Steer < (StandX >= 0 ? 8 : 4) && !aFrz[0] && !aFrz[1] && (t % 12 == 0 || DropWindow); Steer++)
					{
						CHookBotSim C = S;
						std::vector<CNetObj_PlayerInput> avCIn[2] = {avIn[0], avIn[1]};
						std::vector<vec2> avCPath[2] = {avPath[0], avPath[1]};
						int Rest = 0;
						for(int k = 0; k < 200 && Rest < 2; k++)
						{
							CNetObj_PlayerInput aI[2] = {};
							for(int i = 0; i < 2; i++)
							{
								aI[i].m_TargetX = 1;
								if(C.m_aTee[i].m_FreezeTime == 0 && !OnGroundAt(C.m_pCollision, C.m_aTee[i].m_Core.m_Pos))
								{
									// (4-7: each to a standing spot near the goal, either way round, without and with the air jump)
									aI[i].m_Direction = Steer >= 4 ? SteerTo(aStandX[(i + Steer / 6) % 2], C.m_aTee[i].m_Core.m_Pos.x, C.m_aTee[i].m_Core.m_Vel.x) :
												Steer % 2 ? FieldDir(Field, C.m_aTee[i].m_Core.m_Pos) : AirDir(C.m_aTee[i].m_Core.m_Pos);
									aI[i].m_Jump = (Steer == 2 || Steer == 3 || Steer == 5 || Steer == 7) && !(C.m_aTee[i].m_Core.m_Jumped & 2) && C.m_aTee[i].m_Core.m_Vel.y > 2;
								}
								avCPath[i].push_back(C.m_aTee[i].m_Core.m_Pos);
								avCIn[i].push_back(aI[i]);
							}
							C.Step(aI[0], aI[1]);
							Steps++;
							Rest = AtRest(C, 0) && AtRest(C, 1) ? Rest + 1 : 0;
						}
						float RestValue;
						const bool RestOk = Rest >= 2 && TeamEndValue(C, Field, aD0, &RestValue);
						if(getenv("HH_CLIMBDBG") && getenv("HH_CLIMBDBG")[0] == '3' && Pass == 1)
							printf("climbrest style %d t %d steer %d: rest %d ends %.1f %.1f%s / %.1f %.1f%s: %s %.0f\n", Style, t, Steer, Rest, C.m_aTee[0].m_Core.m_Pos.x / 32, C.m_aTee[0].m_Core.m_Pos.y / 32,
								C.InFreeze(C.m_aTee[0].m_Core.m_Pos) ? " (in freeze)" : "", C.m_aTee[1].m_Core.m_Pos.x / 32, C.m_aTee[1].m_Core.m_Pos.y / 32, C.InFreeze(C.m_aTee[1].m_Core.m_Pos) ? " (in freeze)" : "",
								RestOk ? "ok" : "no", RestOk ? RestValue : 0.0f);
						if(RestOk)
						{
							RestValue -= 0.5f * avCIn[0].size();
							if(RestValue > BestValue)
								Record(avCIn, avCPath, C, RestValue);
						}
					}
				}
				CNetObj_PlayerInput aI[2];
				const vec2 aBefore[2] = {S.m_aTee[0].m_Core.m_Pos, S.m_aTee[1].m_Core.m_Pos};
				if(getenv("HH_CLIMBDBG") && getenv("HH_CLIMBDBG")[0] == '2' && Pass == 1 && Style == (getenv("HH_CLIMBSTYLE") ? atoi(getenv("HH_CLIMBSTYLE")) : 2) && Wait == 0 && RescueAbove == 32.0f)
					printf("climbtrace %3d: %.1f %.1f v %.1f %.1f frz %d hook %d | %.1f %.1f v %.1f %.1f frz %d hook %d | rescued %d\n", t, S.m_aTee[0].m_Core.m_Pos.x / 32, S.m_aTee[0].m_Core.m_Pos.y / 32,
						S.m_aTee[0].m_Core.m_Vel.x, S.m_aTee[0].m_Core.m_Vel.y, S.m_aTee[0].m_FreezeTime, S.m_aTee[0].m_Core.m_HookState, S.m_aTee[1].m_Core.m_Pos.x / 32, S.m_aTee[1].m_Core.m_Pos.y / 32,
						S.m_aTee[1].m_Core.m_Vel.x, S.m_aTee[1].m_Core.m_Vel.y, S.m_aTee[1].m_FreezeTime, S.m_aTee[1].m_Core.m_HookState, Rescued);
				if(aFrz[0] || aFrz[1])
				{
					// the free one hooks the frozen one up while it's below, and hammers it free once it's out of the
					// freeze tiles (rank 1 starts up Stronghold's unhookable shaft so: one lies frozen on a block in the
					// freeze band, the other comes by above, drags it up and hammers it)
					const int f = aFrz[0] ? 1 : 0, z = 1 - f;
					auto &Fr = S.m_aTee[f], &Zr = S.m_aTee[z];
					aI[0] = aI[1] = {};
					aI[0].m_TargetX = aI[1].m_TargetX = 1;
					Aim(aI[f], Zr.m_Core.m_Pos - Fr.m_Core.m_Pos);
					// (pulling until it's a tile above me, and staying under it: rank 1 hammers it as it comes past; leaving
					// it level, steering on the way on, it rose out of reach and I fell)
					aI[f].m_Hook = HookBotHookInput(Zr.m_Core.m_Pos.y > Fr.m_Core.m_Pos.y - RescueAbove && distance(Zr.m_Core.m_Pos, Fr.m_Core.m_Pos) < 360, aPrevHook[f], Fr.m_Core.m_HookState);
					aPrevHook[f] = F.m_aPrevHook[f] = aI[f].m_Hook;
					aPrevHook[z] = F.m_aPrevHook[z] = 0;
					aI[f].m_Direction = SteerTo(Zr.m_Core.m_Pos.x + Zr.m_Core.m_Vel.x * 4, Fr.m_Core.m_Pos.x, Fr.m_Core.m_Vel.x);
					aI[f].m_Jump = !OnGroundAt(S.m_pCollision, Fr.m_Core.m_Pos) && !(Fr.m_Core.m_Jumped & 2) && Fr.m_Core.m_Vel.y > 2;
					if(Fr.m_Reload == 0 && !S.InFreeze(Zr.m_Core.m_Pos) && HammerReaches(Fr.m_Core.m_Pos, Zr.m_Core.m_Pos) &&
						!S.m_pCollision->IntersectLine(Fr.m_Core.m_Pos, Zr.m_Core.m_Pos, nullptr, nullptr))
					{
						aI[f].m_Fire = 1;
						HammerPush(Zr.m_Core, Fr.m_Core.m_Pos);
						Zr.m_FreezeTime = 0;
						Fr.m_Reload = 16;
						Rescued = true;
					}
					S.Step(aI[0], aI[1]);
				}
				else if(t < Wait)
				{
					// first a few ticks of each steering the way the open air goes (the fly starts from closer)
					for(int i = 0; i < 2; i++)
					{
						aI[i] = {};
						aI[i].m_TargetX = 1;
						aI[i].m_Direction = AirDir(S.m_aTee[i].m_Core.m_Pos);
					}
					S.Step(aI[0], aI[1]);
				}
				else if(PolIdx >= 0)
				{
					if(vPols[PolIdx].m_UDir == 2)
						ClimbPolStep(S, vPols[PolIdx], aPrevHook, aPrevFire, [&](vec2 P) { return FieldDir(Field, P); }, aI, &Air);
					else if(vPols[PolIdx].m_UDir == 1)
						ClimbPolStep(S, vPols[PolIdx], aPrevHook, aPrevFire, AirDir, aI, &Air);
					else
						ClimbPolStep(S, vPols[PolIdx], aPrevHook, aPrevFire, UpDir, aI);
				}
				else if(Style >= NumPseudo)
					HookflyPlanStep(S, s_aHookfly[Style - NumPseudo], aPrevHook, AirDir, aI);
				else
				{
					const int d = S.m_aTee[0].m_Core.m_Pos.y < S.m_aTee[1].m_Core.m_Pos.y ? 0 : 1;
					const vec2 Dp = S.m_aTee[d].m_Core.m_Pos;
					FlyStep(S, F, Style < 2 ? Pseudo : ClimbPseudo, Style < 2 ? AirDir(Dp) : UpDir(Dp), aI);
				}
				for(int i = 0; i < 2; i++)
				{
					avPath[i].push_back(aBefore[i]);
					avIn[i].push_back(aI[i]);
					aPrevFire[i] = aI[i].m_Fire & 1;
				}
				Steps++;
			}
			// an end mid-climb: where the climb went on clean 60 ticks after it (a bit less than one where each of us can
			// swing on: in Stronghold's swing course, those ended above freeze bands and the solo swings from there fell in,
			// and no climb went on from there either)
			{
				const bool Clean = S.m_aTee[0].m_FreezeTime == 0 && S.m_aTee[1].m_FreezeTime == 0 && !S.m_aTee[0].m_Dead && !S.m_aTee[1].m_Dead;
				const int CleanUntil = Clean ? Stop : Stop - 1;
				for(const SMid &M : vMid)
					if(M.m_T + 60 <= CleanUntil && M.m_Value - 50 > BestValue && M.m_T <= (int)avIn[0].size())
					{
						BestValue = M.m_Value - 50;
						Best.m_Valid = true;
						Best.m_Score = BestValue;
						for(int i = 0; i < 2; i++)
						{
							Best.m_avIn[i].assign(avIn[i].begin(), avIn[i].begin() + M.m_T);
							Best.m_avPath[i].assign(avPath[i].begin(), avPath[i].begin() + M.m_T);
							Best.m_aEnd[i] = M.m_aPos[i];
						}
					}
			}
			if(getenv("HH_CLIMBDBG") && Pass == 1)
				printf("climbdbg style %d wait %d: lower one at best %.1f, stopped at %d (%s)\n", Style, Wait, MinY / 32, Stop,
					S.m_aTee[0].m_FreezeTime > 0 || S.m_aTee[1].m_FreezeTime > 0 ? "frozen" : "time");
		}
	}
	if(ms_DebugTeam && getenv("HH_SIMDUMP"))
		printf("simdump climb %s\n", HookBotSimDump(Base).c_str());
	if(ms_DebugTeam)
		printf("climb from %.0f,%.0f,%d,%.0f,%.0f,%d,%.1f,%.1f,%.1f,%.1f: best %.0f, %d ticks\n", Base.m_aTee[0].m_Core.m_Pos.x, Base.m_aTee[0].m_Core.m_Pos.y, Base.m_aTee[0].m_FreezeTime > 0,
			Base.m_aTee[1].m_Core.m_Pos.x, Base.m_aTee[1].m_Core.m_Pos.y, Base.m_aTee[1].m_FreezeTime > 0, Base.m_aTee[0].m_Core.m_Vel.x, Base.m_aTee[0].m_Core.m_Vel.y, Base.m_aTee[1].m_Core.m_Vel.x,
			Base.m_aTee[1].m_Core.m_Vel.y, BestValue, Steps);
	if(Best.m_Valid)
		Best.m_Up = 0;
	Best.m_Steps = Steps;
	return Best;
}

// a drag from standing (Stronghold's big room: the two of us on the corners of a post, the 1-wide freeze column on it
// between us; the one beyond it hooks the other through and both fly on to the next post's corners, as a joint move
// once found from 10 px further on, and not from here): the hooker holds a direction while it pulls, either of us may
// jump (the ground jump on a given tick, the air jump once falling), it lets go after Release ticks, and both steer the
// way the goal field goes down from there. A grid, judged where we come to rest
CHookBotBrain::STeamPlan CHookBotBrain::PlanDrag(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_DRAG;
	int Steps = 0;
	const float aD0[2] = {Field.Dist(Base.m_aTee[0].m_Core.m_Pos), Field.Dist(Base.m_aTee[1].m_Core.m_Pos)};
	float BestValue = 300;
	static const int s_aHJump[] = {-1, 0, 4, 8, 14};
	static const int s_aPJump[] = {-1, 0, 3, 6, 10};
	static const int s_aRelease[] = {8, 14, 20, 28, 40, 60};
	auto GoDir = [&](vec2 P) {
		const float L = Field.Dist(P + vec2(-48, 0)), R = Field.Dist(P + vec2(48, 0));
		return L < R - 8 ? -1 : R < L - 8 ? 1 : 0;
	};
	for(int h = 0; h < 2; h++)
		for(int HDir = -1; HDir <= 1; HDir++)
			for(int PDir = -1; PDir <= 1; PDir++)
				for(int HJump : s_aHJump)
					for(int PJump : s_aPJump)
						for(int Release : s_aRelease)
							for(int AirJ = 1; AirJ >= 0; AirJ--)
							{
								if(Steps >= MaxSteps)
									break;
								const int p = 1 - h;
								CHookBotSim S = Base;
								std::vector<CNetObj_PlayerInput> avIn[2];
								std::vector<vec2> avPath[2];
								int PrevHook = 0, Rest = 0;
								bool Bad = false;
								for(int t = 0; t < 300 && Rest < 2 && !Bad; t++)
								{
									CNetObj_PlayerInput aI[2] = {};
									for(int i = 0; i < 2; i++)
									{
										const auto &T = S.m_aTee[i];
										aI[i].m_TargetX = 1;
										if(T.m_FreezeTime > 0)
											continue;
										const bool Ground = OnGroundAt(S.m_pCollision, T.m_Core.m_Pos);
										if(t < Release)
										{
											aI[i].m_Direction = i == h ? HDir : PDir;
											aI[i].m_Jump = t == (i == h ? HJump : PJump) && Ground;
										}
										else
											aI[i].m_Direction = GoDir(T.m_Core.m_Pos);
										if(AirJ && !Ground && !(T.m_Core.m_Jumped & 2) && T.m_Core.m_Vel.y > 2)
											aI[i].m_Jump = 1;
									}
									const auto &H = S.m_aTee[h];
									Aim(aI[h], S.m_aTee[p].m_Core.m_Pos - H.m_Core.m_Pos);
									aI[h].m_Hook = H.m_FreezeTime > 0 ? 0 : HookBotHookInput(t < Release, PrevHook, H.m_Core.m_HookState);
									PrevHook = aI[h].m_Hook;
									for(int i = 0; i < 2; i++)
									{
										avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
										avIn[i].push_back(aI[i]);
									}
									S.Step(aI[0], aI[1]);
									Steps++;
									Bad = S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead;
									// (a hook that missed: nothing to drag)
									Bad |= t == 6 && H.m_FreezeTime == 0 && H.m_Core.HookedPlayer() != S.m_aTee[p].m_Core.m_Id;
									Rest = t > Release && AtRest(S, 0) && AtRest(S, 1) ? Rest + 1 : 0;
								}
								float Value;
								if(Bad || Rest < 2 || !TeamEndValue(S, Field, aD0, &Value))
									continue;
								Value -= 0.5f * avIn[0].size();
								if(Value > BestValue)
								{
									BestValue = Value;
									Best.m_Valid = true;
									Best.m_Score = Value;
									for(int i = 0; i < 2; i++)
									{
										Best.m_avIn[i] = avIn[i];
										Best.m_avPath[i] = avPath[i];
										Best.m_aEnd[i] = S.m_aTee[i].m_Core.m_Pos;
									}
								}
							}
	if(ms_DebugTeam)
		printf("drag: best %.0f, %d ticks\n", BestValue, Steps);
	if(Best.m_Valid)
		Best.m_Up = Base.InFreeze(Best.m_aEnd[0]) ? 0 : 1;
	Best.m_Steps = Steps;
	return Best;
}

// down through a freeze column into a room whose floor is freeze too (Stronghold after the corridor: the column at x
// 98-107 at its end, the room below with its freeze floor 44 rows down; rank 1, 121-126 s): standing, one of us (the
// diver) runs at the column and jumps into it; the other (the holder) hooks it there and holds it a while, steering
// (rank 1 holds it hanging under the corridor's floor for a second and more: falling straight on, it would come down
// frozen on the freeze floor), lets go, and dives in after it; the diver, thawed in the air, hooks the holder as it falls
// past and both steer the way the goal goes, air-jumping when the freeze comes close below. A grid, judged where we
// come to rest
// a fling: one of us (the thrower) runs, jumps and hooks the other, drags it along the floor a while, lets go and hammers
// it as it comes by, and it flies on through the freeze beyond, frozen, to land on the other side (Stronghold, the chamber
// after the unhookable room: a block of freeze 9 tiles wide with an unhookable pillar inside; rank 1 so, 150-152 s, the
// thrown one at 17 px/tick, and it thaws on the far side). A grid: who throws, when it jumps, how soon it hooks after
// that, its direction while it holds, how long it holds, whether the other jumps as it's let go; to rest from there
CHookBotBrain::STeamPlan CHookBotBrain::PlanFling(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_FLING;
	int Steps = 0;
	const float aD0[2] = {Field.Dist(Base.m_aTee[0].m_Core.m_Pos), Field.Dist(Base.m_aTee[1].m_Core.m_Pos)};
	float BestValue = 300;
	const bool Stats = ms_DebugTeam && getenv("HH_FLINGDBG");
	// (rank 1's thrower hooks 2 ticks after it starts running, jumps 3 later, runs on 8 more and then brakes, so the other
	// passes under it, and lets go and hammers it 25 ticks after it took hold)
	// (both may back off first: rank 1's thrower starts 5 tiles from the freeze, the other 8; from 1 and 3, the thrower ran
	// into the freeze itself)
	static const int s_aJump[] = {0, 3, 6}, s_aHook[] = {1, 3, 6}, s_aSwitch[] = {0, 8}, s_aBackT[] = {0, 10, 20}, s_aBackD[] = {0, 15};
	const int RelStep = 3;
	// one step of the fling: the thrower runs the way, jumps, hooks the other, holds it (the way, then HDir), lets go at
	// Release and hammers it as it comes by; the other jumps as it's let go (DJump) and steers the way a while
	struct SFling
	{
		int m_Dir, m_T, m_JumpT, m_HookAt, m_HDir, m_Switch, m_Release, m_DJump, m_BackT, m_BackD;
		float m_HamMin = 0; // the hammer waits (up to 10 ticks after letting go) until it pushes this much the way (px/tick):
				    // it pushes up and away from me, most the way with the other diagonally below me ahead
	};
	auto Step = [&](CHookBotSim &S, const SFling &F, int t, int aPrevHook[2], int &GrabAt, bool &Hammered, CNetObj_PlayerInput aI[2]) {
		const int T = F.m_T, D = 1 - T;
		aI[0] = aI[1] = {};
		aI[0].m_TargetX = aI[1].m_TargetX = 1;
		auto &Tt = S.m_aTee[T], &Dd = S.m_aTee[D];
		const int ReleaseAt = GrabAt >= 0 ? GrabAt + F.m_Release : 1 << 20, HookAt = F.m_BackT + F.m_HookAt;
		if(Tt.m_FreezeTime == 0)
		{
			Aim(aI[T], Dd.m_Core.m_Pos - Tt.m_Core.m_Pos);
			const bool Hooking = t >= HookAt && t < ReleaseAt && (GrabAt < 0 ? t < HookAt + 12 : Tt.m_Core.HookedPlayer() == Dd.m_Core.m_Id);
			aI[T].m_Hook = HookBotHookInput(Hooking, aPrevHook[T], Tt.m_Core.m_HookState);
			aI[T].m_Direction = t < F.m_BackT ? -F.m_Dir : t < HookAt + F.m_Switch ? F.m_Dir : t < ReleaseAt ? F.m_HDir : 0;
			aI[T].m_Jump = t == F.m_BackT + F.m_JumpT;
			// (the hammer needs no clear line)
			const vec2 Away = distance(Tt.m_Core.m_Pos, Dd.m_Core.m_Pos) > 0 ? normalize(Dd.m_Core.m_Pos - Tt.m_Core.m_Pos) : vec2(0, -1);
			const float PushX = normalize(Away + vec2(0, -1.1f)).x * 10.0f * F.m_Dir;
			if(!Hammered && t >= ReleaseAt && t < ReleaseAt + 10 && (PushX >= F.m_HamMin || t == ReleaseAt + 9) && Tt.m_Reload == 0 && HammerReaches(Tt.m_Core.m_Pos, Dd.m_Core.m_Pos))
			{
				aI[T].m_Fire = 1;
				HammerPush(Dd.m_Core, Tt.m_Core.m_Pos);
				Dd.m_FreezeTime = 0;
				Tt.m_Reload = 16;
				Hammered = true;
			}
		}
		if(Dd.m_FreezeTime == 0)
		{
			aI[D].m_Direction = t < F.m_BackD ? -F.m_Dir : t >= ReleaseAt && t < ReleaseAt + 8 ? F.m_Dir : 0;
			aI[D].m_Jump = F.m_DJump && t == ReleaseAt;
		}
		for(int i = 0; i < 2; i++)
			aPrevHook[i] = aI[i].m_Hook;
		S.Step(aI[0], aI[1]);
		if(GrabAt < 0 && S.m_aTee[T].m_Core.HookedPlayer() == S.m_aTee[D].m_Core.m_Id)
			GrabAt = t + 1;
	};
	// (towards freeze within 8 tiles along our floor, either way: the goal field's slope beside the freeze points away from
	// it, round a long way with less freeze)
	const vec2 Mid0 = (Base.m_aTee[0].m_Core.m_Pos + Base.m_aTee[1].m_Core.m_Pos) / 2;
	for(int Dir = -1; Dir <= 1; Dir += 2)
	{
		bool FreezeSide = false;
		for(int k = 1; k <= 8 && !FreezeSide; k++)
		{
			const vec2 C = Mid0 + vec2(Dir * 32.0f * k, 0);
			if(Base.m_pCollision->CheckPoint(C))
				break;
			FreezeSide = Base.InFreeze(C) || Base.InFreeze(C - vec2(0, 32));
		}
		if(!FreezeSide)
			continue;
		for(int T = 0; T < 2; T++)
			for(int BackT : s_aBackT)
				for(int BackD : s_aBackD)
					for(int JumpT : s_aJump)
						for(int HookAt : s_aHook)
							for(int HDir = -1; HDir <= 0; HDir++)
								for(int Switch : s_aSwitch)
						{
							if(Steps >= MaxSteps || (HDir == Dir && Switch))
								continue;
							const int D = 1 - T;
							SFling F = {Dir, T, JumpT, HookAt, Dir == 1 ? HDir : -HDir, Switch, 1 << 20, 0, BackT, BackD};
							// the hold, to 32 ticks after the hook takes hold; a release every 2 ticks from 6 on branches off
							CHookBotSim S = Base;
							std::vector<CNetObj_PlayerInput> avIn[2];
							std::vector<vec2> avPath[2];
							int aPrevHook[2] = {0, 0}, GrabAt = -1;
							bool Hammered = false;
							for(int t = 0; t < 110; t++)
							{
								if(GrabAt >= 0 && t >= GrabAt + 6 && (t - GrabAt) % RelStep == 0)
									for(int DJump = 0; DJump < 2; DJump++)
										for(float HamMin : {0.0f})
									{
										SFling R = F;
										R.m_Release = t - GrabAt;
										R.m_DJump = DJump;
										R.m_HamMin = HamMin;
										CHookBotSim E = S;
										std::vector<CNetObj_PlayerInput> avEIn[2] = {avIn[0], avIn[1]};
										std::vector<vec2> avEPath[2] = {avPath[0], avPath[1]};
										int aEPrev[2] = {aPrevHook[0], aPrevHook[1]}, EGrab = GrabAt, Rest = 0;
										bool EHam = Hammered;
										vec2 HamPos(0, 0), HamVel(0, 0);
										for(int u = t; u < t + 300 && Rest < 3; u++)
										{
											CNetObj_PlayerInput aI[2];
											for(int i = 0; i < 2; i++)
												avEPath[i].push_back(E.m_aTee[i].m_Core.m_Pos);
											const bool WasHam = EHam;
											Step(E, R, u, aEPrev, EGrab, EHam, aI);
											if(EHam && !WasHam)
											{
												HamPos = E.m_aTee[D].m_Core.m_Pos;
												HamVel = E.m_aTee[D].m_Core.m_Vel;
											}
											for(int i = 0; i < 2; i++)
												avEIn[i].push_back(aI[i]);
											Steps++;
											if(E.m_aTee[0].m_Dead || E.m_aTee[1].m_Dead || E.m_aTee[T].m_FreezeTime > 0)
												break;
											Rest = AtRest(E, 0) && AtRest(E, 1) ? Rest + 1 : 0;
										}
										if(Rest < 3)
											continue;
										float Value;
										const float GainD = aD0[D] - Field.Dist(E.m_aTee[D].m_Core.m_Pos);
										const bool Ok = GainD > 300 && TeamEndValue(E, Field, aD0, &Value, -200);
										if(Stats)
											printf("fling dir %d t %d back %d/%d jump %d hook %d hdir %d switch %d release %d djump %d hammin %.0f: hammer at %.1f %.1f v %.1f %.1f, ends %.1f %.1f%s / %.1f %.1f%s, gain %.0f, %s %.0f\n", Dir, T, BackT, BackD, JumpT, HookAt, HDir, Switch,
												R.m_Release, DJump, HamMin, HamPos.x / 32, HamPos.y / 32, HamVel.x, HamVel.y, E.m_aTee[0].m_Core.m_Pos.x / 32, E.m_aTee[0].m_Core.m_Pos.y / 32, E.InFreeze(E.m_aTee[0].m_Core.m_Pos) ? " frz" : "",
												E.m_aTee[1].m_Core.m_Pos.x / 32, E.m_aTee[1].m_Core.m_Pos.y / 32, E.InFreeze(E.m_aTee[1].m_Core.m_Pos) ? " frz" : "", GainD, Ok ? "ok" : "no", Ok ? Value : 0.0f);
										if(!Ok)
											continue;
										Value -= 0.5f * avEIn[0].size();
										if(Value > BestValue)
										{
											BestValue = Value;
											Best.m_Valid = true;
											Best.m_Score = Value;
											Best.m_Up = D;
											for(int i = 0; i < 2; i++)
											{
												Best.m_avIn[i] = avEIn[i];
												Best.m_avPath[i] = avEPath[i];
												Best.m_aEnd[i] = E.m_aTee[i].m_Core.m_Pos;
											}
										}
									}
								if(GrabAt >= 0 && t > GrabAt + 32)
									break;
								CNetObj_PlayerInput aI[2];
								for(int i = 0; i < 2; i++)
									avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
								Step(S, F, t, aPrevHook, GrabAt, Hammered, aI);
								for(int i = 0; i < 2; i++)
									avIn[i].push_back(aI[i]);
								Steps++;
								if(S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead || S.m_aTee[T].m_FreezeTime > 0 || S.m_aTee[D].m_FreezeTime > 0 || (GrabAt < 0 && t > BackT + HookAt + 12))
									break;
							}
						}
	}
	if(ms_DebugTeam)
		printf("fling: best %.0f, %d ticks\n", BestValue, Steps);
	Best.m_Steps = Steps;
	return Best;
}

// a launch into a climb from standing (Stronghold's room after the freeze pools: the ledge at its foot is under a freeze
// pool, the open air beside it beyond the freeze pool's floor; rank 1 runs off the ledge's end, jumps and flies on up the
// room's right wall, 146-150 s; jumping where we stood, the climbs went up into the pool above and got nowhere): both run
// the way the open air goes, the first jumps at its floor's end or after a while, the second some ticks after it (it
// waits at the end), both steering that way, air-jumping once falling; the best few states with both in the air, clear
// of the freeze, go to the climb planner (`PlanClimb`), and launch and climb make one plan
CHookBotBrain::STeamPlan CHookBotBrain::PlanLaunch(const CHookBotSim &Base, const CHookBotGoalField &Field, const CHookBotGoalField &Air, int MaxSteps, const SPseudoParams &Pseudo)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_CLIMB;
	int Steps = 0;
	const float aD0[2] = {Field.Dist(Base.m_aTee[0].m_Core.m_Pos), Field.Dist(Base.m_aTee[1].m_Core.m_Pos)};
	float BestValue = 300;
	const vec2 Mid0 = (Base.m_aTee[0].m_Core.m_Pos + Base.m_aTee[1].m_Core.m_Pos) / 2;
	const int Dir = Air.Dist(Mid0 + vec2(-64, -32)) < Air.Dist(Mid0 + vec2(64, -32)) ? -1 : 1;
	const bool Stats = ms_DebugTeam && getenv("HH_LAUNCHDBG");
	if(getenv("HH_SIMDUMP"))
		printf("simdump launch %s\n", HookBotSimDump(Base).c_str());
	struct SLaunch
	{
		CHookBotSim m_S;
		std::vector<CNetObj_PlayerInput> m_avIn[2];
		std::vector<vec2> m_avPath[2];
		float m_Value;
	};
	std::vector<SLaunch> vL;
	static const int s_aRun[] = {0, 8, 16, 24, 40};
	static const int s_aStagger[] = {0, 4, 8, 12};
	// (and off the ground: on that way, straight down or back, jumping off or walking off: running and jumping at the
	// end of the block above Stronghold's shaft after the striped room carried us across it into the freeze column beside)
	for(int AirSteer = -1; AirSteer <= 1; AirSteer++)
	for(int WalkOff = 0; WalkOff < 2; WalkOff++)
	for(int First = 0; First < 2; First++)
		for(int Run : s_aRun)
			for(int Stagger : s_aStagger)
			{
				if(Stagger == 0 && First == 1)
					continue;
				CHookBotSim S = Base;
				std::vector<CNetObj_PlayerInput> avIn[2];
				std::vector<vec2> avPath[2];
				int aJumpAt[2] = {-1, -1}, BothAirSince = -1;
				for(int t = 0; t < 120; t++)
				{
					CNetObj_PlayerInput aI[2] = {};
					for(int i = 0; i < 2; i++)
					{
						const auto &T = S.m_aTee[i];
						aI[i].m_TargetX = 1;
						const bool Ground = OnGroundAt(S.m_pCollision, T.m_Core.m_Pos);
						aI[i].m_Direction = Ground || aJumpAt[i] < 0 ? Dir : AirSteer * Dir;
						const bool Edge = Ground && !S.m_pCollision->CheckPoint(T.m_Core.m_Pos + vec2(Dir * 24.0f, 24));
						if(aJumpAt[i] < 0 && Ground)
						{
							if(i == First ? (Edge || t >= Run) : (aJumpAt[First] >= 0 && t >= aJumpAt[First] + Stagger))
							{
								// (walking off: on over the edge, "jumped" from here on)
								aI[i].m_Jump = !WalkOff;
								aJumpAt[i] = t;
							}
							else if(Edge)
								aI[i].m_Direction = 0;
						}
						else if(aJumpAt[i] >= 0 && !Ground && !(T.m_Core.m_Jumped & 2) && T.m_Core.m_Vel.y > 0 && FreezeClearance(S, T.m_Core.m_Pos) < 96)
							aI[i].m_Jump = 1;
					}
					for(int i = 0; i < 2; i++)
					{
						avIn[i].push_back(aI[i]);
						avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
					}
					S.Step(aI[0], aI[1]);
					Steps++;
					// (one of us may freeze: rank 1 drops down the shaft after the striped room, one lands frozen on the
					// freeze floor at its foot, the other hooks it up and keeps under it till it thaws, and the climb
					// starts from there; the climb planner starts so from one of us frozen)
					const bool aFrz[2] = {S.m_aTee[0].m_FreezeTime > 0, S.m_aTee[1].m_FreezeTime > 0};
					if(S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead || (aFrz[0] && aFrz[1]))
						break;
					bool aAir[2];
					for(int i = 0; i < 2; i++)
						aAir[i] = !OnGroundAt(S.m_pCollision, S.m_aTee[i].m_Core.m_Pos);
					if(!(aAir[0] || aFrz[0]) || !(aAir[1] || aFrz[1]))
					{
						BothAirSince = -1;
						continue;
					}
					if(BothAirSince < 0)
						BothAirSince = t;
					// (later ones too: dropping down a shaft to climb the one beside it, the climb starts near the bottom)
					// (every 2 ticks: dropping down that shaft, the calm moment right after the air jump above its freeze floor
					// is what the climb needs)
					const int k = t - BothAirSince;
					if(k < 2 || k % 2)
						continue;
					const vec2 P0 = S.m_aTee[0].m_Core.m_Pos, P1 = S.m_aTee[1].m_Core.m_Pos;
					bool Clear = distance(P0, P1) < 360;
					for(int i = 0; i < 2 && Clear; i++)
						if(!aFrz[i])
							Clear = !NearFreeze(S, S.m_aTee[i].m_Core.m_Pos, S.m_aTee[i].m_Core.m_Pos) && FreezeClearance(S, S.m_aTee[i].m_Core.m_Pos) >= 48;
					if(!Clear)
						continue;
					SLaunch L;
					L.m_S = S;
					for(int i = 0; i < 2; i++)
					{
						L.m_avIn[i] = avIn[i];
						L.m_avPath[i] = avPath[i];
					}
					// (close together: the climb's rope reaches 360 px, and it starts with the upper one hooking the other)
					// (and slow: falling at 25 px/tick 10 rows above the freeze floor at the foot of the shaft after the striped
					// room, the climb had no time before both were in it)
					const float Fall = std::max(S.m_aTee[0].m_Core.m_Vel.y, S.m_aTee[1].m_Core.m_Vel.y);
					L.m_Value = Air.Dist((P0 + P1) / 2) + 0.5f * t + 2 * std::max(0.0f, distance(P0, P1) - 96) + 16 * std::max(0.0f, Fall - 8);
					vL.push_back(std::move(L));
				}
			}
	std::stable_sort(vL.begin(), vL.end(), [](const SLaunch &a, const SLaunch &b) { return a.m_Value < b.m_Value; });
	if(Stats)
		for(int k = 0; k < (int)vL.size() && k < 25; k++)
			printf("launch state %d: %.1f %.1f v %.1f %.1f%s / %.1f %.1f v %.1f %.1f%s after %d, value %.0f\n", k, vL[k].m_S.m_aTee[0].m_Core.m_Pos.x / 32, vL[k].m_S.m_aTee[0].m_Core.m_Pos.y / 32,
				vL[k].m_S.m_aTee[0].m_Core.m_Vel.x, vL[k].m_S.m_aTee[0].m_Core.m_Vel.y, vL[k].m_S.m_aTee[0].m_FreezeTime ? " frz" : "", vL[k].m_S.m_aTee[1].m_Core.m_Pos.x / 32,
				vL[k].m_S.m_aTee[1].m_Core.m_Pos.y / 32, vL[k].m_S.m_aTee[1].m_Core.m_Vel.x, vL[k].m_S.m_aTee[1].m_Core.m_Vel.y, vL[k].m_S.m_aTee[1].m_FreezeTime ? " frz" : "",
				(int)vL[k].m_avIn[0].size(), vL[k].m_Value);
	std::vector<const SLaunch *> vPick;
	for(const SLaunch &L : vL)
	{
		bool Same = false;
		for(const SLaunch *pO : vPick)
			Same |= distance(pO->m_S.m_aTee[0].m_Core.m_Pos, L.m_S.m_aTee[0].m_Core.m_Pos) < 16 && distance(pO->m_S.m_aTee[1].m_Core.m_Pos, L.m_S.m_aTee[1].m_Core.m_Pos) < 16;
		if(!Same && vPick.size() < 3)
			vPick.push_back(&L);
	}
	for(int k = 0; k < (int)vPick.size() && Steps < MaxSteps; k++)
	{
		const SLaunch &L = *vPick[k];
		STeamPlan Climb = PlanClimb(L.m_S, Field, Air, std::max(100000, (MaxSteps - Steps) / ((int)vPick.size() - k)), Pseudo);
		Steps += Climb.m_Steps;
		if(Stats)
			printf("launch %d (%.1f %.1f / %.1f %.1f after %d ticks, air %.0f): climb %s %.0f\n", k, L.m_S.m_aTee[0].m_Core.m_Pos.x / 32, L.m_S.m_aTee[0].m_Core.m_Pos.y / 32, L.m_S.m_aTee[1].m_Core.m_Pos.x / 32,
				L.m_S.m_aTee[1].m_Core.m_Pos.y / 32, (int)L.m_avIn[0].size(), L.m_Value, Climb.m_Valid ? "found" : "none", Climb.m_Score);
		if(!Climb.m_Valid)
			continue;
		const int Len = (int)L.m_avIn[0].size() + (int)Climb.m_avIn[0].size();
		const float Value = aD0[0] + aD0[1] - Field.Dist(Climb.m_aEnd[0]) - Field.Dist(Climb.m_aEnd[1]) - 0.5f * Len;
		if(Value > BestValue)
		{
			BestValue = Value;
			Best.m_Valid = true;
			Best.m_Score = Value;
			for(int i = 0; i < 2; i++)
			{
				Best.m_avIn[i] = L.m_avIn[i];
				Best.m_avIn[i].insert(Best.m_avIn[i].end(), Climb.m_avIn[i].begin(), Climb.m_avIn[i].end());
				Best.m_avPath[i] = L.m_avPath[i];
				Best.m_avPath[i].insert(Best.m_avPath[i].end(), Climb.m_avPath[i].begin(), Climb.m_avPath[i].end());
				Best.m_aEnd[i] = Climb.m_aEnd[i];
			}
		}
	}
	if(ms_DebugTeam)
		printf("launch: %d states, best %.0f, %d ticks\n", (int)vL.size(), BestValue, Steps);
	Best.m_Steps = Steps;
	return Best;
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanColumnDrop(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_COLUMN;
	int Steps = 0;
	const float aD0[2] = {Field.Dist(Base.m_aTee[0].m_Core.m_Pos), Field.Dist(Base.m_aTee[1].m_Core.m_Pos)};
	float BestValue = 300;
	// the diver backs off a while, runs at the column and jumps this far from its edge (rank 1's comes running at 14 px/tick
	// and jumps over the holder: from standing next to it, a jump only got to the edge of the freeze, and the hook pulled
	// it back out); the holder hooks it once it has been frozen a while
	static const int s_aBack[] = {0, 10, 20, 30};
	static const float s_aJumpDist[] = {48, 96, 160, 224};
	// (rank 1's holder hooks it 26 ticks after it froze, holds the whole 60 a hook holds a tee, and jumps in after it 12
	// before letting go: hooked 12 after, it came down on the freeze floor 13 ticks before it thawed)
	static const int s_aHookDelay[] = {18, 24, 30, 36};
	static const int s_aHold[] = {45, 60};
	static const int s_aDiveAfter[] = {-40, -30, -20, -10, 0};
	const bool Stats = getenv("HH_COLUMNDBG");
	// where the diver thaws in the air, the holder after it within reach: from there a climb (PlanClimb: the free one
	// hooks the frozen one up, hammers it free, and we fly on together; rank 1 so, 124-126 s), for the best few
	struct SThaw
	{
		CHookBotSim m_S;
		std::vector<CNetObj_PlayerInput> m_avIn[2];
		std::vector<vec2> m_avPath[2];
		float m_Dist;
		int m_Diver;
	};
	std::vector<SThaw> vThaw;
	for(int d = 0; d < 2; d++)
		for(int Side = -1; Side <= 1; Side += 2)
		{
			// the column's near edge, along the diver's row
			const vec2 P0 = Base.m_aTee[d].m_Core.m_Pos;
			float EdgeX = -1;
			for(int k = 1; k <= 20 && EdgeX < 0; k++)
			{
				const vec2 C(P0.x + Side * 32.0f * k, P0.y);
				if(Base.m_pCollision->CheckPoint(C))
					break;
				if(Base.InFreeze(C))
					EdgeX = (std::floor(C.x / 32) + (Side < 0 ? 1 : 0)) * 32;
			}
			if(EdgeX < 0)
				continue;
			for(int Back : s_aBack)
				for(float JumpDist : s_aJumpDist)
					for(int HookDelay : s_aHookDelay)
						for(int Hold : s_aHold)
							for(int HDir = -1; HDir <= 1; HDir++)
								for(int DiveAfter : s_aDiveAfter)
								{
									if(Steps >= MaxSteps)
										break;
									const int h = 1 - d, Catch = 20;
									CHookBotSim S = Base;
									std::vector<CNetObj_PlayerInput> avIn[2];
									std::vector<vec2> avPath[2];
									int aPrevHook[2] = {0, 0};
									bool aWasFrozen[2] = {false, false}, Caught = false, Jumped = false;
									int CatchStart = -1, Rest = 0, FrozeAt = -1;
									bool Bad = false;
									for(int t = 0; t < 700 && Rest < 2 && !Bad; t++)
									{
										CNetObj_PlayerInput aI[2] = {};
										aI[0].m_TargetX = aI[1].m_TargetX = 1;
										auto &D = S.m_aTee[d], &H = S.m_aTee[h];
										if(aWasFrozen[d] && D.m_FreezeTime == 0 && !OnGroundAt(S.m_pCollision, D.m_Core.m_Pos) && aWasFrozen[h])
										{
											// the diver thawed in the air, the holder dived after it: a climb from here, if it's in reach
											const float Dist = distance(D.m_Core.m_Pos, H.m_Core.m_Pos);
											if(Dist < 20 * 32 && FreezeClearance(S, D.m_Core.m_Pos) > 32)
											{
												SThaw T;
												T.m_S = S;
												for(int i = 0; i < 2; i++)
												{
													T.m_avIn[i] = avIn[i];
													T.m_avPath[i] = avPath[i];
												}
												T.m_Dist = Dist;
												T.m_Diver = d;
												vThaw.push_back(std::move(T));
											}
											Bad = true;
											break;
										}
										for(int i = 0; i < 2; i++)
											aWasFrozen[i] |= S.m_aTee[i].m_FreezeTime > 0;
										if(FrozeAt < 0 && aWasFrozen[d])
											FrozeAt = t;
										const int HookAt = FrozeAt < 0 ? 1 << 20 : FrozeAt + HookDelay, Release = HookAt + Hold, Dive = Release + DiveAfter;
										// the diver: back off, at the column and in
										if(D.m_FreezeTime == 0)
										{
											const bool Ground = OnGroundAt(S.m_pCollision, D.m_Core.m_Pos);
											if(!aWasFrozen[d])
											{
												aI[d].m_Direction = t < Back ? -Side : Side;
												if(t >= Back && !Jumped && Ground && absolute(D.m_Core.m_Pos.x - EdgeX) < JumpDist)
													aI[d].m_Jump = Jumped = true;
											}
											else
											{
												// thawed: the holder in reach as it comes by, hooked a while
												const vec2 DH = H.m_Core.m_Pos - D.m_Core.m_Pos;
												if(!Caught && length(DH) < 360 && DH.y < 64 && aWasFrozen[h] && !S.m_pCollision->IntersectLine(D.m_Core.m_Pos, H.m_Core.m_Pos, nullptr, nullptr))
												{
													Caught = true;
													CatchStart = t;
												}
												const bool Hooking = Caught && t - CatchStart < Catch;
												Aim(aI[d], DH);
												aI[d].m_Hook = HookBotHookInput(Hooking, aPrevHook[d], D.m_Core.m_HookState);
												aI[d].m_Direction = FieldDir(Field, D.m_Core.m_Pos);
												if(!Ground && !(D.m_Core.m_Jumped & 2) && D.m_Core.m_Vel.y > 0 && FreezeClearance(S, D.m_Core.m_Pos) < 96)
													aI[d].m_Jump = 1;
											}
										}
										// the holder: out of the diver's way (backing off it the way it backs off), hooks it, holds, lets
										// go, dives after it
										if(H.m_FreezeTime == 0)
										{
											const bool Ground = OnGroundAt(S.m_pCollision, H.m_Core.m_Pos);
											if(!aWasFrozen[h])
											{
												Aim(aI[h], D.m_Core.m_Pos - H.m_Core.m_Pos);
												aI[h].m_Hook = HookBotHookInput(t >= HookAt && t < Release, aPrevHook[h], H.m_Core.m_HookState);
												aI[h].m_Direction = t >= HookAt && t < Release ? HDir : t >= Dive ? Side : 0;
												aI[h].m_Jump = t == Dive && Ground;
											}
											else
											{
												aI[h].m_Direction = FieldDir(Field, H.m_Core.m_Pos);
												if(!Ground && !(H.m_Core.m_Jumped & 2) && H.m_Core.m_Vel.y > 0 && FreezeClearance(S, H.m_Core.m_Pos) < 96)
													aI[h].m_Jump = 1;
											}
										}
										for(int i = 0; i < 2; i++)
										{
											aPrevHook[i] = aI[i].m_Hook;
											avPath[i].push_back(S.m_aTee[i].m_Core.m_Pos);
											avIn[i].push_back(aI[i]);
										}
										if(const char *pTrace = getenv("HH_COLUMNTRACE"))
										{
											int aP[8];
											if(sscanf(pTrace, "%d,%d,%d,%d,%d,%d,%d,%d", &aP[0], &aP[1], &aP[2], &aP[3], &aP[4], &aP[5], &aP[6], &aP[7]) == 8 && aP[0] == d && aP[1] == Side && aP[2] == Back &&
												aP[3] == (int)JumpDist && aP[4] == HookDelay && aP[5] == Hold && aP[6] == HDir && aP[7] == DiveAfter && t % 3 == 0)
												printf("columntrace %3d: D %.1f %.1f v %.1f %.1f frz %d hook %d | H %.1f %.1f v %.1f %.1f frz %d hook %d\n", t, D.m_Core.m_Pos.x / 32, D.m_Core.m_Pos.y / 32, D.m_Core.m_Vel.x,
													D.m_Core.m_Vel.y, D.m_FreezeTime, D.m_Core.m_HookState, H.m_Core.m_Pos.x / 32, H.m_Core.m_Pos.y / 32, H.m_Core.m_Vel.x, H.m_Core.m_Vel.y, H.m_FreezeTime,
													H.m_Core.m_HookState);
										}
										S.Step(aI[0], aI[1]);
										Steps++;
										Bad = S.m_aTee[0].m_Dead || S.m_aTee[1].m_Dead;
										// (the diver has to get into the freeze)
										Bad |= t == Back + 80 && !aWasFrozen[d] && S.m_aTee[d].m_FreezeTime == 0;
										Rest = Dive < (1 << 19) && t > Dive && AtRest(S, 0) && AtRest(S, 1) ? Rest + 1 : 0;
									}
									float Value;
									const bool Ok = !Bad && Rest >= 2 && TeamEndValue(S, Field, aD0, &Value);
									if(Stats && Rest >= 2 && !Bad)
										printf("column d %d side %d back %d jumpdist %.0f hookdelay %d hold %d hdir %d dive %d: ends %.1f %.1f%s / %.1f %.1f%s, %s %.0f\n", d, Side, Back, JumpDist, HookDelay, Hold, HDir,
											DiveAfter, S.m_aTee[0].m_Core.m_Pos.x / 32, S.m_aTee[0].m_Core.m_Pos.y / 32, S.InFreeze(S.m_aTee[0].m_Core.m_Pos) ? " frz" : "", S.m_aTee[1].m_Core.m_Pos.x / 32,
											S.m_aTee[1].m_Core.m_Pos.y / 32, S.InFreeze(S.m_aTee[1].m_Core.m_Pos) ? " frz" : "", Ok ? "ok" : "no", Ok ? Value - 0.5f * avIn[0].size() : 0.0f);
									if(!Ok)
										continue;
									Value -= 0.5f * avIn[0].size();
									if(Value > BestValue)
									{
										BestValue = Value;
										Best.m_Valid = true;
										Best.m_Score = Value;
										Best.m_Up = d;
										for(int i = 0; i < 2; i++)
										{
											Best.m_avIn[i] = avIn[i];
											Best.m_avPath[i] = avPath[i];
											Best.m_aEnd[i] = S.m_aTee[i].m_Core.m_Pos;
										}
									}
								}
		}
	std::stable_sort(vThaw.begin(), vThaw.end(), [](const SThaw &a, const SThaw &b) { return a.m_Dist < b.m_Dist; });
	if(Stats)
		printf("column: %d thaw states\n", (int)vThaw.size());
	// (many candidates end in the same state: the diver's run and jump don't depend on the holder's timing; each once)
	std::vector<const SThaw *> vPick;
	for(const SThaw &T : vThaw)
	{
		bool Same = false;
		for(const SThaw *pO : vPick)
			Same |= distance(pO->m_S.m_aTee[0].m_Core.m_Pos, T.m_S.m_aTee[0].m_Core.m_Pos) < 4 && distance(pO->m_S.m_aTee[1].m_Core.m_Pos, T.m_S.m_aTee[1].m_Core.m_Pos) < 4;
		if(!Same && vPick.size() < 3)
			vPick.push_back(&T);
	}
	const SPseudoParams Pseudo;
	for(int k = 0; k < (int)vPick.size() && Steps < MaxSteps; k++)
	{
		const SThaw &T = *vPick[k];
		STeamPlan Climb = PlanClimb(T.m_S, Field, Field, std::max(100000, (MaxSteps - Steps) / ((int)vPick.size() - k)), Pseudo);
		Steps += Climb.m_Steps;
		if(Stats)
			printf("column thaw %d (%.1f tiles apart, at %.1f %.1f / %.1f %.1f): climb %s %.0f\n", k, T.m_Dist / 32, T.m_S.m_aTee[0].m_Core.m_Pos.x / 32, T.m_S.m_aTee[0].m_Core.m_Pos.y / 32,
				T.m_S.m_aTee[1].m_Core.m_Pos.x / 32, T.m_S.m_aTee[1].m_Core.m_Pos.y / 32, Climb.m_Valid ? "found" : "none", Climb.m_Score);
		if(!Climb.m_Valid)
			continue;
		const int Len = (int)T.m_avIn[0].size() + (int)Climb.m_avIn[0].size();
		const float Value = aD0[0] + aD0[1] - Field.Dist(Climb.m_aEnd[0]) - Field.Dist(Climb.m_aEnd[1]) - 0.5f * Len;
		if(Value > BestValue)
		{
			BestValue = Value;
			Best.m_Valid = true;
			Best.m_Score = Value;
			Best.m_Up = T.m_Diver;
			for(int i = 0; i < 2; i++)
			{
				Best.m_avIn[i] = T.m_avIn[i];
				Best.m_avIn[i].insert(Best.m_avIn[i].end(), Climb.m_avIn[i].begin(), Climb.m_avIn[i].end());
				Best.m_avPath[i] = T.m_avPath[i];
				Best.m_avPath[i].insert(Best.m_avPath[i].end(), Climb.m_avPath[i].begin(), Climb.m_avPath[i].end());
				Best.m_aEnd[i] = Climb.m_aEnd[i];
			}
		}
	}
	if(ms_DebugTeam)
		printf("column drop: best %.0f, %d ticks\n", BestValue, Steps);
	Best.m_Steps = Steps;
	return Best;
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanJoint(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps, int Effort)
{
	// two passes: steps of K ticks (fine timing), then of 2K ticks twice as deep (moves that take long, like a drag down
	// a shaft after a jump); the better plan of the two. (On Stronghold's corridor of freeze walls each found plans from
	// states the other didn't)
	// (more passes with other step lengths when ms_Joint.m_Passes asks: each sees timings the others don't)
	static const int s_aK[] = {1, 2, 0, 0};
	STeamPlan Best;
	int Steps = 0;
	// (Effort 2 on: other step lengths, for the retries while we both stand with nothing found; the search is brittle:
	// from a post's corners in Stronghold's big room it found a move 10 px off where it found none)
	const int Passes = Effort ? 4 : ms_Joint.m_Passes, Width = Effort ? ms_Joint.m_Width * 8 / 5 : ms_Joint.m_Width;
	const int K0 = Effort >= 2 ? std::max(2, ms_Joint.m_K + Effort % 3 - 1 + (Effort % 3 == 1 ? 2 : 0)) : ms_Joint.m_K;
	for(int Pass = 0; Pass < Passes; Pass++)
	{
		const int K = Pass < 2 ? K0 * s_aK[Pass] : Pass == 2 ? K0 * 3 / 2 : std::max(2, K0 - 1);
		STeamPlan P = PlanJointPass(Base, Field, MaxSteps / Passes, K, Pass == 1 ? ms_Joint.m_MaxDepth : ms_Joint.m_MaxDepth * 4 / K, Width);
		Steps += P.m_Steps;
		if(P.m_Valid && (!Best.m_Valid || P.m_Score > Best.m_Score))
			Best = P;
	}
	Best.m_Steps = Steps;
	return Best;
}

CHookBotBrain::STeamPlan CHookBotBrain::PlanJointPass(const CHookBotSim &Base, const CHookBotGoalField &Field, int MaxSteps, int K, int MaxDepth, int Width)
{
	STeamPlan Best;
	Best.m_Kind = TEAM_JOINT;
	int Steps = 0;
	const CCollision *pCol = Base.m_pCollision;
	const float aD0[2] = {Field.Dist(Base.m_aTee[0].m_Core.m_Pos), Field.Dist(Base.m_aTee[1].m_Core.m_Pos)};
	const int NumCoast = ms_Joint.m_NumCoast;
	struct SNode
	{
		CHookBotSim m_Sim;
		std::vector<CNetObj_PlayerInput> m_avIn[2];
		std::vector<vec2> m_avPath[2];
		int m_aPrevHook[2] = {0, 0};
		float m_Cheap = 0, m_Value = -1e9f;
		bool m_Ok = false;
	};
	// where we come to rest coasting on from N: in the air each keeps its direction, with its air jump once falling or
	// not; on the ground it stops; one holding the other on its hook lets go now, or 4 or 8 ticks later (when to let go
	// of a drag decides where the other lands). The best of these; the tail of inputs and paths that gets there
	vec2 aCoastEnd[2];
	auto Coast = [&](const SNode &N, float *pValue, std::vector<CNetObj_PlayerInput> *pavTail, std::vector<vec2> *pavTailPath) {
		bool Any = false;
		bool Hooking = false;
		for(int i = 0; i < 2; i++)
			Hooking |= N.m_aPrevHook[i] && N.m_Sim.m_aTee[i].m_Core.HookedPlayer() == N.m_Sim.m_aTee[1 - i].m_Core.m_Id;
		for(int Variant = 0; Variant < (Hooking ? 12 : 4); Variant++)
		{
			const int Jumps = Variant % 4, HoldFor = Variant / 4 * 4;
			int aPrevHook[2] = {N.m_aPrevHook[0], N.m_aPrevHook[1]};
			CHookBotSim S = N.m_Sim;
			std::vector<CNetObj_PlayerInput> avTail[2];
			std::vector<vec2> avTailPath[2];
			bool aJumped[2] = {false, false};
			int Rest = 0;
			for(int t = 0; t < 300 && Rest < 2 && !S.m_aTee[0].m_Dead && !S.m_aTee[1].m_Dead; t++)
			{
				CNetObj_PlayerInput aI[2] = {};
				for(int i = 0; i < 2; i++)
				{
					const auto &T = S.m_aTee[i];
					const bool Ground = OnGroundAt(pCol, T.m_Core.m_Pos);
					aI[i].m_TargetX = 1;
					aI[i].m_Direction = Ground || N.m_avIn[i].empty() ? 0 : N.m_avIn[i].back().m_Direction;
					if((Jumps >> i & 1) && !aJumped[i] && !Ground && T.m_Core.m_Vel.y > 0 && !(T.m_Core.m_Jumped & 2))
						aI[i].m_Jump = aJumped[i] = true;
					if(t < HoldFor && N.m_aPrevHook[i])
					{
						Aim(aI[i], S.m_aTee[1 - i].m_Core.m_Pos - T.m_Core.m_Pos);
						aI[i].m_Hook = HookBotHookInput(true, aPrevHook[i], T.m_Core.m_HookState);
					}
					aPrevHook[i] = aI[i].m_Hook;
					avTailPath[i].push_back(T.m_Core.m_Pos);
					avTail[i].push_back(aI[i]);
				}
				S.Step(aI[0], aI[1]);
				Steps++;
				Rest = AtRest(S, 0) && AtRest(S, 1) ? Rest + 1 : 0;
			}
			float Value;
			if(Rest < 2 || !TeamEndValue(S, Field, aD0, &Value))
				continue;
			Value -= 0.5f * (N.m_avIn[0].size() + avTail[0].size());
			if(!Any || Value > *pValue)
			{
				Any = true;
				*pValue = Value;
				if(pavTail)
					for(int i = 0; i < 2; i++)
					{
						pavTail[i] = avTail[i];
						pavTailPath[i] = avTailPath[i];
						aCoastEnd[i] = S.m_aTee[i].m_Core.m_Pos;
					}
			}
		}
		return Any;
	};
	std::vector<SNode> vBeam(1);
	vBeam[0].m_Sim = Base;
	for(int i = 0; i < 2; i++)
		vBeam[0].m_aPrevHook[i] = Base.m_aTee[i].m_PrevInput.m_Hook;
	const SNode *pBest = nullptr;
	float BestValue = 200; // it has to get us somewhere
	std::vector<std::vector<SNode>> vLevels;
	vLevels.reserve(MaxDepth);
	for(int Depth = 0; Depth < MaxDepth && Steps < MaxSteps; Depth++)
	{
		std::vector<SNode> vNext;
		for(const SNode &N : vBeam)
		{
			// each of us: direction, jump or not, hook on the other or not (a frozen one does nothing)
			std::vector<int> avOpt[2];
			for(int i = 0; i < 2; i++)
			{
				const auto &T = N.m_Sim.m_aTee[i];
				if(T.m_FreezeTime > 0)
				{
					avOpt[i].push_back(0);
					continue;
				}
				const bool CanJump = OnGroundAt(pCol, T.m_Core.m_Pos) || !(T.m_Core.m_Jumped & 2);
				// (a hammer at the other one only when it's in reach)
				const auto &O = N.m_Sim.m_aTee[1 - i];
				const bool CanFire = T.m_Reload == 0 && HammerReaches(T.m_Core.m_Pos, O.m_Core.m_Pos) && !pCol->IntersectLine(T.m_Core.m_Pos, O.m_Core.m_Pos, nullptr, nullptr);
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int Jump = 0; Jump <= (CanJump ? 1 : 0); Jump++)
						for(int Hook = 0; Hook <= 1; Hook++)
							for(int Fire = 0; Fire <= (CanFire ? 1 : 0); Fire++)
								avOpt[i].push_back((Dir + 1) | Jump << 2 | Hook << 3 | Fire << 4);
			}
			for(int O0 : avOpt[0])
				for(int O1 : avOpt[1])
				{
					SNode C = N;
					const int aO[2] = {O0, O1};
					bool Bad = false;
					for(int t = 0; t < K && !Bad; t++)
					{
						CNetObj_PlayerInput aI[2] = {};
						// the hammers in the early input of the first tick (positions from the end of the last one): both
						// pushes from where we are now
						vec2 aPush[2];
						for(int i = 0; i < 2; i++)
							aPush[i] = C.m_Sim.m_aTee[i].m_Core.m_Pos;
						for(int i = 0; i < 2; i++)
						{
							const auto &T = C.m_Sim.m_aTee[i], &O = C.m_Sim.m_aTee[1 - i];
							aI[i].m_Direction = (aO[i] & 3) - 1;
							aI[i].m_Jump = (aO[i] >> 2 & 1) && t == 0;
							Aim(aI[i], O.m_Core.m_Pos - T.m_Core.m_Pos);
							aI[i].m_Hook = HookBotHookInput(aO[i] >> 3 & 1, C.m_aPrevHook[i], T.m_Core.m_HookState);
							aI[i].m_Fire = (aO[i] >> 4 & 1) && t == 0;
							C.m_aPrevHook[i] = aI[i].m_Hook;
							C.m_avPath[i].push_back(T.m_Core.m_Pos);
							C.m_avIn[i].push_back(aI[i]);
						}
						for(int i = 0; i < 2; i++)
							if(aI[i].m_Fire)
							{
								HammerPush(C.m_Sim.m_aTee[1 - i].m_Core, aPush[i]);
								C.m_Sim.m_aTee[1 - i].m_FreezeTime = 0;
								C.m_Sim.m_aTee[i].m_Reload = 16;
							}
						C.m_Sim.Step(aI[0], aI[1]);
						Steps++;
						Bad = C.m_Sim.m_aTee[0].m_Dead || C.m_Sim.m_aTee[1].m_Dead || (C.m_Sim.m_aTee[0].m_FreezeTime > 0 && C.m_Sim.m_aTee[1].m_FreezeTime > 0 &&
																   C.m_Sim.InFreeze(C.m_Sim.m_aTee[0].m_Core.m_Pos) && C.m_Sim.InFreeze(C.m_Sim.m_aTee[1].m_Core.m_Pos));
					}
					if(Bad)
						continue;
					// cheap: how near the goal we are, and how fast we're heading there (both of us)
					float Cheap = 0;
					for(int i = 0; i < 2; i++)
					{
						const auto &T = C.m_Sim.m_aTee[i];
						Cheap += Field.Dist(T.m_Core.m_Pos) - 4.0f * (Field.Dist(T.m_Core.m_Pos) - Field.Dist(T.m_Core.m_Pos + T.m_Core.m_Vel));
					}
					C.m_Cheap = Cheap;
					vNext.push_back(std::move(C));
				}
		}
		if(vNext.empty())
			break;
		std::sort(vNext.begin(), vNext.end(), [](const SNode &a, const SNode &b) { return a.m_Cheap < b.m_Cheap; });
		// the best by the cheap measure, not all the same (8 px, 2 px/tick), get the real one: where we come to rest
		std::vector<SNode> vChecked;
		for(SNode &N : vNext)
		{
			if((int)vChecked.size() >= NumCoast || Steps >= MaxSteps)
				break;
			bool Same = false;
			for(const SNode &K2 : vChecked)
			{
				bool S2 = true;
				for(int i = 0; i < 2; i++)
					S2 &= distance(N.m_Sim.m_aTee[i].m_Core.m_Pos, K2.m_Sim.m_aTee[i].m_Core.m_Pos) < 8 && distance(N.m_Sim.m_aTee[i].m_Core.m_Vel, K2.m_Sim.m_aTee[i].m_Core.m_Vel) < 2;
				Same |= S2;
			}
			if(Same)
				continue;
			N.m_Ok = Coast(N, &N.m_Value, nullptr, nullptr);
			vChecked.push_back(std::move(N));
		}
		// the next beam: the best by where they end, then (to keep a way on that isn't safe yet, like speeding up for a
		// jump) the best by the cheap measure
		std::sort(vChecked.begin(), vChecked.end(), [](const SNode &a, const SNode &b) { return (a.m_Ok ? a.m_Value : -1e9f) > (b.m_Ok ? b.m_Value : -1e9f); });
		vLevels.push_back({});
		std::vector<SNode> &vLevel = vLevels.back();
		for(SNode &N : vChecked)
			if(N.m_Ok && (int)vLevel.size() < Width / 2)
				vLevel.push_back(N);
		std::sort(vChecked.begin(), vChecked.end(), [](const SNode &a, const SNode &b) { return a.m_Cheap < b.m_Cheap; });
		for(SNode &N : vChecked)
		{
			if((int)vLevel.size() >= Width)
				break;
			bool Have = false;
			for(const SNode &L : vLevel)
				Have |= L.m_avIn[0].size() == N.m_avIn[0].size() && L.m_Sim.m_aTee[0].m_Core.m_Pos == N.m_Sim.m_aTee[0].m_Core.m_Pos && L.m_Sim.m_aTee[1].m_Core.m_Pos == N.m_Sim.m_aTee[1].m_Core.m_Pos;
			if(!Have)
				vLevel.push_back(N);
		}
		for(const SNode &N : vLevel)
			if(N.m_Ok && N.m_Value > BestValue)
			{
				BestValue = N.m_Value;
				pBest = &N;
			}
		vBeam = vLevel;
	}
	if(!pBest)
	{
		Best.m_Steps = Steps;
		return Best;
	}
	std::vector<CNetObj_PlayerInput> avTail[2];
	std::vector<vec2> avTailPath[2];
	float Value;
	Coast(*pBest, &Value, avTail, avTailPath);
	Best.m_Valid = true;
	Best.m_Score = Value;
	for(int i = 0; i < 2; i++)
	{
		Best.m_avIn[i] = pBest->m_avIn[i];
		Best.m_avIn[i].insert(Best.m_avIn[i].end(), avTail[i].begin(), avTail[i].end());
		Best.m_avPath[i] = pBest->m_avPath[i];
		Best.m_avPath[i].insert(Best.m_avPath[i].end(), avTailPath[i].begin(), avTailPath[i].end());
	}
	Best.m_aEnd[0] = aCoastEnd[0];
	Best.m_aEnd[1] = aCoastEnd[1];
	Best.m_Up = Base.InFreeze(Best.m_aEnd[0]) ? 1 : 0;
	Best.m_Steps = Steps;
	return Best;
}

// a fall caught on the hook from where we are (flying): the plan, if there is one, and my input from it
bool CHookBotBrain::TryFall(CNetObj_PlayerInput &In, int Budget)
{
	CHookBotSim S;
	InitTeamSim(S);
	STeamPlan Plan = PlanFall(S, m_pGoalFar ? *m_pGoalFar : *m_pGoal, getenv("HH_FALLBUDGET") ? atoi(getenv("HH_FALLBUDGET")) : Budget > 0 ? Budget : m_TeamBudget, m_Pseudo);
	if(ms_DebugTeam)
		printf("fall try t%d id %d: %.1f %.1f / %.1f %.1f: %s (%d searched)\n", m_Now, m_pB->m_Core.m_Id, S.m_aTee[0].m_Core.m_Pos.x / 32, S.m_aTee[0].m_Core.m_Pos.y / 32, S.m_aTee[1].m_Core.m_Pos.x / 32,
			S.m_aTee[1].m_Core.m_Pos.y / 32, Plan.m_Valid ? "found" : "none", Plan.m_Steps);
	if(!Plan.m_Valid)
		return false;
	Plan.m_aId[0] = S.m_aTee[0].m_Core.m_Id;
	Plan.m_aId[1] = S.m_aTee[1].m_Core.m_Id;
	Plan.m_Start = m_Now;
	m_Team = Plan;
	m_TeamEndKind = TEAM_NONE;
	const int Me = Plan.m_aId[0] == m_pB->m_Core.m_Id ? 0 : 1;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), Plan.m_Up == Me ? "I fall in, catch me: I land at %.1f %.1f (%d ticks searched)" : "you fall in, I catch you: you land at %.1f %.1f (%d ticks searched)",
		Plan.m_aEnd[Plan.m_Up].x / 32, Plan.m_aEnd[Plan.m_Up].y / 32, Plan.m_Steps);
	m_Say(aBuf);
	return TeamMove(In);
}

bool CHookBotBrain::TeamMove(CNetObj_PlayerInput &In)
{
	const SHookBotTee *pB = m_pB, *pU = m_pU;
	const vec2 B = pB->m_Core.m_Pos, U = pU->m_Core.m_Pos;
	if(!m_PartnerIsBot || !m_pGoal)
		return false;
	CHookBotSim Probe;
	Probe.m_pCollision = m_pCollision;
	// my half, while we're both where the plan has us (both of us see it the moment either is off, and drop it)
	if(m_Team.m_Valid)
	{
		const int Me = m_Team.m_aId[0] == pB->m_Core.m_Id ? 0 : 1;
		const int k = m_Now - m_Team.m_Start;
		const int Len = m_Team.m_avIn[Me].size();
		if(ms_DebugTeam && k >= 0 && k < Len && (distance(B, m_Team.m_avPath[Me][k]) > 0.01f || distance(U, m_Team.m_avPath[1 - Me][k]) > 0.01f))
			printf("team drift t%d id %d k %d/%d kind %d: me %+.2f %+.2f frz %d hook %d | partner %+.2f %+.2f frz %d hook %d\n", m_Now, pB->m_Core.m_Id, k, Len, m_Team.m_Kind, B.x - m_Team.m_avPath[Me][k].x,
				B.y - m_Team.m_avPath[Me][k].y, pB->m_FreezeTime, pB->m_Core.m_HookState, U.x - m_Team.m_avPath[1 - Me][k].x, U.y - m_Team.m_avPath[1 - Me][k].y, pU->m_FreezeTime, pU->m_Core.m_HookState);
		if(k >= 0 && k < Len && distance(B, m_Team.m_avPath[Me][k]) < 1.0f && distance(U, m_Team.m_avPath[1 - Me][k]) < 1.0f)
		{
			const CNetObj_PlayerInput &P = m_Team.m_avIn[Me][k];
			m_pWhy = m_Team.m_Kind == TEAM_THROW ? "team: throw" : m_Team.m_Kind == TEAM_CATCH ? "team: catch" : m_Team.m_Kind == TEAM_LEAP ? "team: leap" : m_Team.m_Kind == TEAM_JOINT ? "team: joint" : m_Team.m_Kind == TEAM_DROP ? "team: drop" : m_Team.m_Kind == TEAM_FALL ? "team: fall" : m_Team.m_Kind == TEAM_DASH ? "team: dash" : m_Team.m_Kind == TEAM_HOP ? "team: hop" : m_Team.m_Kind == TEAM_CLIMB ? "team: climb" : m_Team.m_Kind == TEAM_DRAG ? "team: drag" : m_Team.m_Kind == TEAM_COLUMN ? "team: column" : m_Team.m_Kind == TEAM_FLING ? "team: fling" : "team: gather";
			In.m_Direction = P.m_Direction;
			In.m_Jump = P.m_Jump;
			In.m_Hook = P.m_Hook;
			In.m_TargetX = P.m_TargetX;
			In.m_TargetY = P.m_TargetY;
			In.m_Fire = P.m_Fire;
			return true;
		}
		if(k < Len && ms_DebugTeam && k >= 0)
			printf("team wrong t%d id %d k %d/%d: me %.2f %.2f (plan %.2f %.2f) v %.2f %.2f frz %d | partner %.2f %.2f (plan %.2f %.2f) v %.2f %.2f frz %d\n", m_Now, pB->m_Core.m_Id, k, Len, B.x, B.y,
				m_Team.m_avPath[Me][k].x, m_Team.m_avPath[Me][k].y, pB->m_Core.m_Vel.x, pB->m_Core.m_Vel.y, pB->m_FreezeTime, U.x, U.y, m_Team.m_avPath[1 - Me][k].x, m_Team.m_avPath[1 - Me][k].y,
				pU->m_Core.m_Vel.x, pU->m_Core.m_Vel.y, pU->m_FreezeTime);
		if(k < Len)
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "%s went wrong at tick %d of %d", m_Team.m_Kind == TEAM_THROW ? "the throw" : m_Team.m_Kind == TEAM_CATCH ? "the catch" : m_Team.m_Kind == TEAM_LEAP ? "the leap" : m_Team.m_Kind == TEAM_JOINT ? "the joint move" : m_Team.m_Kind == TEAM_DROP ? "the drop" : m_Team.m_Kind == TEAM_FALL ? "the fall" : m_Team.m_Kind == TEAM_DASH ? "the dash" : m_Team.m_Kind == TEAM_HOP ? "the hop" : m_Team.m_Kind == TEAM_CLIMB ? "the climb" : m_Team.m_Kind == TEAM_DRAG ? "the drag" : m_Team.m_Kind == TEAM_COLUMN ? "the column drop" : m_Team.m_Kind == TEAM_FLING ? "the fling" : "landing", k, Len);
			m_Say(aBuf);
		}
		m_TeamEnd = m_Now;
		m_TeamEndKind = k >= Len ? m_Team.m_Kind : TEAM_NONE;
		// a climb that ends in the air: each of us swings on by itself from there (no fly again for 60 ticks: another climb
		// can be planned a second after one ends, and in Stronghold's swing course the fly came first, half a second after
		// one, and dropped us both into the freeze)
		if((m_Team.m_Kind == TEAM_CLIMB || m_Team.m_Kind == TEAM_COLUMN) && k >= Len && !pB->m_Grounded && pB->m_FreezeTime == 0)
		{
			m_CoastDir = m_Team.m_avIn[Me].empty() || !m_Team.m_avIn[Me].back().m_Direction ? (B.x < U.x ? -1 : 1) : m_Team.m_avIn[Me].back().m_Direction;
			m_NextSoloSearch = m_Now;
			m_NoFlyUntil = m_Now + 60;
			m_ClimbAgainAt = m_Now;
		}
		m_Team = {};
	}
	// after a team move: the one it left frozen lies outside the freeze and thaws; the other one waits for it, still
	if(m_TeamEndKind != TEAM_NONE && m_Now - m_TeamEnd < 4 * SERVER_TICK_SPEED && pB->m_FreezeTime == 0 && pB->m_Grounded && pU->m_FreezeTime > 0 && pU->m_Grounded &&
		!Probe.InFreeze(U))
	{
		m_pWhy = "team: wait for it to thaw";
		In.m_Direction = 0;
		In.m_Jump = 0;
		In.m_Hook = 0;
		return true;
	}
	// one of us lying frozen in freeze, the other standing still outside it: a joint move first, judged by where we both
	// end up (the plain rescues went round in circles in Stronghold's 3-wide gaps between freeze walls: freeing the
	// partner for a moment counted, and it fell back into the wall each time)
	{
		auto Lies = [&](const SHookBotTee *pT) { return pT->m_FreezeTime > 0 && pT->m_Grounded && length(pT->m_Core.m_Vel) < 1 && Probe.InFreeze(pT->m_Core.m_Pos); };
		auto Stands = [&](const SHookBotTee *pT) { return pT->m_FreezeTime == 0 && pT->m_Grounded && length(pT->m_Core.m_Vel) <= 0.01f && !Probe.InFreeze(pT->m_Core.m_Pos); };
		// (or flying, every half second: where there's nowhere to stand, the free one never stood still, and swung about
		// until it froze too; Stronghold's pocket between freeze columns at x 443-451 below the freeze shaft)
		auto Flies = [&](const SHookBotTee *pT) { return pT->m_FreezeTime == 0 && !pT->m_Grounded && m_Now % 25 == 0; };
		if(((Lies(pB) && (Stands(pU) || Flies(pU))) || (Lies(pU) && (Stands(pB) || Flies(pB)))) && !(distance(B, m_aTeamFailPos[0]) < 2 && distance(U, m_aTeamFailPos[1]) < 2))
		{
			CHookBotSim S;
			InitTeamSim(S);
			STeamPlan Plan;
			// the free one flying close by, and climbing gets us closer: a climb that starts with getting the other one up
			// and free (rank 1 at Stronghold's unhookable shaft after the hookable blocks: one lies frozen on a block in
			// the freeze band, the other comes by above, drags it up, hammers it free, and they hookfly up)
			const vec2 Mid = (B + U) / 2;
			if((Flies(pB) || Flies(pU)) && m_pGoalAir && distance(B, U) < 10 * 32 && m_pGoalAir->Dist(Mid) < 1e5f && m_pGoalAir->Dist(Mid - vec2(0, 96)) < m_pGoalAir->Dist(Mid) - 40)
				Plan = PlanClimb(S, m_pGoalSolo ? *m_pGoalSolo : *m_pGoal, *m_pGoalAir, m_TeamBudget / 2, m_Pseudo);
			if(!Plan.m_Valid)
				Plan = PlanJoint(S, *m_pGoal, m_TeamBudget);
			if(!Plan.m_Valid)
				Plan = PlanJoint(S, *m_pGoal, 2 * m_TeamBudget, 1);
			if(!Plan.m_Valid)
			{
				m_aTeamFailPos[0] = B;
				m_aTeamFailPos[1] = U;
				char aBuf[96];
				str_format(aBuf, sizeof(aBuf), "no joint move out of the freeze (%d ticks searched)", Plan.m_Steps);
				m_Say(aBuf);
				return false;
			}
			Plan.m_aId[0] = S.m_aTee[0].m_Core.m_Id;
			Plan.m_aId[1] = S.m_aTee[1].m_Core.m_Id;
			Plan.m_Start = m_Now;
			m_Team = Plan;
			m_TeamEndKind = TEAM_NONE;
			const int Me = Plan.m_aId[0] == pB->m_Core.m_Id ? 0 : 1;
			char aBuf[160];
			str_format(aBuf, sizeof(aBuf), "%s: I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)", Plan.m_Kind == TEAM_CLIMB ? "up together" : "out of the freeze together", Plan.m_aEnd[Me].x / 32,
				Plan.m_aEnd[Me].y / 32, Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
			m_Say(aBuf);
			return TeamMove(In);
		}
	}
	// one of us frozen and falling, the other free, the way on beyond freeze: catch it on the hook and let it land beyond
	// (every 10 ticks; Stronghold, above the pool after the pit: the hammerhit rescues there dropped one down the freeze
	// column and the other into the pool)
	if(m_Now % 10 == 0 && !m_SoloByAir && distance(B, U) < 16 * 32 &&
		((pB->m_FreezeTime > 0 && !pB->m_Grounded && pU->m_FreezeTime == 0) || (pU->m_FreezeTime > 0 && !pU->m_Grounded && pB->m_FreezeTime == 0)) && TryFall(In, m_TeamBudget / 3))
		return true;
	if(pB->m_FreezeTime || pU->m_FreezeTime || Probe.InFreeze(B) || Probe.InFreeze(U))
		return false;
	// standing still: on the ground, or on the other one's head (that isn't the ground to the game, and the one on top
	// jitters up and down a little there: on Stronghold's block corner after the corridor's first walls they stood like
	// that for good, and no team move ever started)
	auto OnHead = [](const SHookBotTee *pTop, const SHookBotTee *pBottom) {
		const vec2 d = pTop->m_Core.m_Pos - pBottom->m_Core.m_Pos;
		return d.y < -20 && d.y > -44 && absolute(d.x) < 28 && absolute(pTop->m_Core.m_Vel.x) < 1 && absolute(pTop->m_Core.m_Vel.y) < 3;
	};
	auto StandsStill = [&](const SHookBotTee *pT, const SHookBotTee *pO) { return (pT->m_Grounded && length(pT->m_Core.m_Vel) <= 0.01f) || (OnHead(pT, pO) && pO->m_Grounded && length(pO->m_Core.m_Vel) <= 0.01f); };
	const bool Still = StandsStill(pB, pU) && StandsStill(pU, pB);
	const bool Together = (absolute(B.y - U.y) < 8 || OnHead(pB, pU) || OnHead(pU, pB)) && distance(B, U) < 6 * 32;
	if(!Still)
	{
		// a team move just ended with both of us on the ground: settle, so the next one is planned from standing (the
		// solo jump and the walking rule's drop went first: off the platform beside Stronghold's freeze shaft into the
		// pool, right after a joint move had put us both there)
		if(m_TeamEndKind != TEAM_NONE && m_Now - m_TeamEnd < 15 && pB->m_Grounded && pU->m_Grounded)
		{
			m_pWhy = "team: settle";
			In.m_Direction = 0;
			In.m_Jump = 0;
			In.m_Hook = 0;
			return true;
		}
		// both flying together, the way on beyond freeze: one of us falls into it, the other catches it on the hook and
		// lets it land beyond (every 25 ticks; both of us look on the same ticks)
		if(m_Now % 25 == 0 && !m_SoloByAir && !pB->m_Grounded && !pU->m_Grounded && distance(B, U) < 250 && TryFall(In))
			return true;
		// both free, at least one of us in the air, close together, and climbing gets us closer: a climb (every 10 ticks;
		// Stronghold's unhookable shaft after the hookable blocks)
		// (both in the air: with one on the ground, it ran all through the hammerhit cycles under Stronghold's first
		// ceiling gap, and the search took most of the run's time)
		if(getenv("HH_CLIMBTRIG") && (m_Now % 10 == 0 || (m_ClimbAgainAt >= 0 && m_Now >= m_ClimbAgainAt)) && m_pGoalAir)
			printf("climbtrig %d again %d ground %d %d: dist %.1f teamend %d hookable %d air %.0f helps %d/%d nearfrz %d %d\n", m_Now, m_ClimbAgainAt >= 0 && m_Now >= m_ClimbAgainAt, pB->m_Grounded, pU->m_Grounded, distance(B, U) / 32, m_Now - m_TeamEnd, HookableNear(Probe, (B + U) / 2, 380, 64),
				m_pGoalAir->Dist((B + U) / 2), m_pGoalAir->Dist((B + U) / 2 - vec2(0, 96)) < m_pGoalAir->Dist((B + U) / 2) - 40,
				m_pGoalSolo && m_pGoalSolo->Dist((B + U) / 2 - vec2(0, 96)) < m_pGoalSolo->Dist((B + U) / 2) - 40, NearFreeze(Probe, B, B), NearFreeze(Probe, U, U));
		// (within 18 tiles where nothing hookable is above us: at the foot of Stronghold's shaft each of us comes by on its own
		// swings, apart, and by the time both were within 10 both had used their air jumps and one fell into the freeze)
		// (and once right after a climb that ended in the air: climbs chain on, each sees 300 ticks; in Stronghold's swing
		// course one from the right side ended above a freeze band, out of the shaft's sight, and both solo swings from
		// there fell into the band)
		const bool ClimbAgain = m_ClimbAgainAt >= 0 && m_Now >= m_ClimbAgainAt;
		m_ClimbAgainAt = -1;
		// (chained on, whether or not climbing gets us closer from here, or the open air goes on anywhere: the planner judges;
		// above Stronghold's swing course the way on goes mostly sideways to the shaft, and 3 tiles up gained less than 40;
		// above the freeze strip into the corridor after the shaft, no waypoint on is in the open air)
		if((ClimbAgain || (m_Now % 10 == 0 && m_Now - m_TeamEnd > SERVER_TICK_SPEED)) && m_pGoalAir && !pB->m_Grounded && !pU->m_Grounded &&
			(distance(B, U) < 10 * 32 || (distance(B, U) < 18 * 32 && !HookableNear(Probe, (B + U) / 2, 380, 64))) && (ClimbAgain || m_pGoalAir->Dist((B + U) / 2) < 1e5f) &&
			(ClimbAgain || m_pGoalAir->Dist((B + U) / 2 - vec2(0, 96)) < m_pGoalAir->Dist((B + U) / 2) - 40 ||
				// (or the open-air way climbs 10 tiles: at the foot of the unhookable room after Stronghold's freeze pools it
				// goes sideways first)
				AirWayClimbs((B + U) / 2, 10) ||
				// (or the goal 7 waypoints on, from between us or where either of us is: at the foot of Stronghold's shaft the
				// air goes to the next waypoint, level with us, and between us is the far side of the freeze ring of a block)
				(m_pGoalSolo && (m_pGoalSolo->Dist((B + U) / 2 - vec2(0, 96)) < m_pGoalSolo->Dist((B + U) / 2) - 40 ||
							m_pGoalSolo->Dist(B - vec2(0, 96)) < m_pGoalSolo->Dist(B) - 40 || m_pGoalSolo->Dist(U - vec2(0, 96)) < m_pGoalSolo->Dist(U) - 40))) &&
			!NearFreeze(Probe, B, B) && !NearFreeze(Probe, U, U))
		{
			CHookBotSim S;
			InitTeamSim(S);
			// (judged by the goal 7 waypoints on: by the one 3 on, the climb up Stronghold's course ended in the air at the
			// shaft's foot above a freeze band, with nothing to swing on but blocks ringed with freeze, and both fell in)
			STeamPlan Plan = PlanClimb(S, m_pGoalSolo ? *m_pGoalSolo : *m_pGoal, *m_pGoalAir, m_TeamBudget / 2, m_Pseudo);
			// (no climb on from where one ended: a joint move from the air, once; at the top of Stronghold's shaft we fly
			// left at 10-14 px/tick above the freeze floor, and the way into the corridor is one of us into the freeze strip
			// and the other knocking it on through)
			if(!Plan.m_Valid && ClimbAgain && !getenv("HH_NOCLIMBJOINT"))
				Plan = PlanJoint(S, *m_pGoal, m_TeamBudget);
			if(Plan.m_Valid)
			{
				Plan.m_aId[0] = S.m_aTee[0].m_Core.m_Id;
				Plan.m_aId[1] = S.m_aTee[1].m_Core.m_Id;
				Plan.m_Start = m_Now;
				m_Team = Plan;
				m_TeamEndKind = TEAM_NONE;
				const int Me = Plan.m_aId[0] == pB->m_Core.m_Id ? 0 : 1;
				char aBuf[160];
				str_format(aBuf, sizeof(aBuf), "%s: I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)", Plan.m_Kind == TEAM_CLIMB ? "up together" : "together then", Plan.m_aEnd[Me].x / 32,
					Plan.m_aEnd[Me].y / 32, Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
				m_Say(aBuf);
				return TeamMove(In);
			}
		}
		// both free, below a freeze ceiling the way goes up through, one of us in the air: land on a floor there next to
		// each other, for the throw (every 10 ticks: it's a search; both of us look on the same ticks)
		if(m_Now % 10 || distance(B, U) > 10 * 32 || (pB->m_Grounded && pU->m_Grounded))
			return false;
		CHookBotSim S;
		InitTeamSim(S);
		if(FreezeColumns(S, *m_pGoal, (B + U) / 2).empty())
			return false;
		STeamPlan Plan = PlanGather(S, *m_pGoal, m_TeamBudget / 8);
		if(!Plan.m_Valid)
			return false;
		Plan.m_aId[0] = S.m_aTee[0].m_Core.m_Id;
		Plan.m_aId[1] = S.m_aTee[1].m_Core.m_Id;
		Plan.m_Start = m_Now;
		m_Team = Plan;
		m_TeamEndKind = TEAM_NONE;
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "let's land at %.1f %.1f for the throw (%d ticks searched)", (Plan.m_aEnd[0].x + Plan.m_aEnd[1].x) / 64, Plan.m_aEnd[0].y / 32, Plan.m_Steps);
		m_Say(aBuf);
		return TeamMove(In);
	}
	// both of us free and standing still outside the freeze: a throw or a catch, if none was found from here before;
	// standing here with nothing found, every 2 s a joint move once more with other step lengths (both of us count the
	// same idle time, so we try the same)
	if(distance(B, m_aTeamFailPos[0]) < 2 && distance(U, m_aTeamFailPos[1]) < 2)
	{
		if(m_BothIdleSince < 0 || m_Now - m_BothIdleSince < 2 * SERVER_TICK_SPEED || (m_Now - m_BothIdleSince) % (2 * SERVER_TICK_SPEED))
			return false;
		const int Retry = (m_Now - m_BothIdleSince) / (2 * SERVER_TICK_SPEED);
		if(Retry > 6)
			return false;
		CHookBotSim S;
		InitTeamSim(S);
		STeamPlan Plan = PlanJoint(S, *m_pGoal, 2 * m_TeamBudget, 1 + Retry);
		if(ms_DebugTeam)
			printf("team retry %d t%d id %d: %s (%d searched)\n", Retry, m_Now, pB->m_Core.m_Id, Plan.m_Valid ? "found" : "none", Plan.m_Steps);
		if(!Plan.m_Valid)
			return false;
		Plan.m_aId[0] = S.m_aTee[0].m_Core.m_Id;
		Plan.m_aId[1] = S.m_aTee[1].m_Core.m_Id;
		Plan.m_Start = m_Now;
		m_Team = Plan;
		m_TeamEndKind = TEAM_NONE;
		const int Me = Plan.m_aId[0] == pB->m_Core.m_Id ? 0 : 1;
		char aBuf[160];
		str_format(aBuf, sizeof(aBuf), "together then (try %d): I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)", Retry, Plan.m_aEnd[Me].x / 32, Plan.m_aEnd[Me].y / 32,
			Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
		m_Say(aBuf);
		return TeamMove(In);
	}
	CHookBotSim S;
	InitTeamSim(S);
	STeamPlan Plan;
	const float DistB = m_pGoal->Dist(B), DistU = m_pGoal->Dist(U);
	if(Together)
	{
		// next to each other on a floor, with a freeze ceiling above that the way goes up through: a throw; the way on
		// beyond freeze (not through open air): a leap across together
		if(!DropColumns(S, *m_pGoal, (B + U) / 2).empty())
			Plan = PlanDrop(S, *m_pGoal, m_TeamBudget);
		if(!Plan.m_Valid && !FreezeColumns(S, *m_pGoal, (B + U) / 2).empty())
			Plan = PlanThrow(S, *m_pGoal, m_TeamBudget);
		// a freeze ceiling right above our heads, no room to throw under it: hop up through it (Stronghold, the 1-tile
		// tunnel after the corridor)
		if(!Plan.m_Valid && !FreezeColumns(S, *m_pGoal, (B + U) / 2).empty() && FreezeClearanceUp(S, (B + U) / 2) < 3 * 32)
			Plan = PlanHop(S, *m_pGoal, 3 * m_TeamBudget, m_Pseudo);
		if(!Plan.m_Valid && !m_GoalByAir)
			Plan = PlanLeap(S, *m_pGoal, m_TeamBudget);
		if(!Plan.m_Valid && !m_GoalByAir)
			Plan = PlanFall(S, m_pGoalFar ? *m_pGoalFar : *m_pGoal, m_TeamBudget, m_Pseudo);
		if(!Plan.m_Valid && !m_GoalByAir)
			Plan = PlanJoint(S, *m_pGoal, m_TeamBudget);
		if(!Plan.m_Valid && !m_GoalByAir)
			Plan = PlanJoint(S, *m_pGoal, 2 * m_TeamBudget, 1);
		// the way on beyond freeze beside us, too wide to jump: one flings the other through it (it lands frozen on the far
		// side and thaws there)
		if(!Plan.m_Valid && !getenv("HH_NOFLING"))
			Plan = PlanFling(S, *m_pGoal, m_TeamBudget);
		// the way on below a freeze column next to us, too deep to fall through it and land free: one dives in, the other
		// holds it under the floor and dives after it, the first one thawed catches the other (Stronghold, the corridor's
		// end; rank 1 so, 122-126 s)
		if(!Plan.m_Valid && !m_GoalByAir && !getenv("HH_NOCOLUMN"))
			Plan = PlanColumnDrop(S, m_pGoalFar ? *m_pGoalFar : *m_pGoal, 3 * m_TeamBudget);
		if(!Plan.m_Valid && !m_GoalByAir)
			Plan = PlanDrag(S, *m_pGoal, m_TeamBudget);
		// a freeze floor ahead, too long to jump: across it together (Stronghold, the corridor after the zig-zag)
		if(!Plan.m_Valid)
		{
			const vec2 Mid = (B + U) / 2;
			const int Way = m_pGoal->Dist(Mid + vec2(-192, -64)) < m_pGoal->Dist(Mid + vec2(192, -64)) ? -1 : 1;
			if(FreezeFloorAhead(S, B.x * Way > U.x * Way ? B : U, Way))
				Plan = PlanDash(S, *m_pGoal, m_TeamBudget);
		}
		// the way on up through open air, too high to jump, nothing else found: a run, a jump and a climb from there (or the
		// way 7 waypoints on climbs: Stronghold after the striped room, down a shaft whose foot is a freeze floor, along
		// under a freeze column and up the unhookable shaft beside it; the air goal 3 waypoints on is level with the foot)
		if(!Plan.m_Valid && m_GoalByAir && m_pGoalAir && (AirWayClimbs((B + U) / 2, 10) || (m_pGoalSolo && AirWayClimbs((B + U) / 2, 10, m_pGoalSolo.get()))) && !getenv("HH_NOLAUNCH"))
			Plan = PlanLaunch(S, m_pGoalSolo ? *m_pGoalSolo : *m_pGoal, *m_pGoalAir, 2 * m_TeamBudget, m_Pseudo);
		// (not from here again either: the dash's search ran every tick, both of us standing on the ledge before the slope in
		// Stronghold's room after the freeze pools)
		if(!Plan.m_Valid && FreezeColumns(S, *m_pGoal, (B + U) / 2).empty() && DropColumns(S, *m_pGoal, (B + U) / 2).empty() && m_GoalByAir)
		{
			m_aTeamFailPos[0] = B;
			m_aTeamFailPos[1] = U;
			return false;
		}
	}
	// (within 40 tiles: Stronghold, after the zig-zag's room, one of us thrown through the freeze column into the gap at
	// x 236-238 y 225, the other on the platform 30 tiles above, 32.3 tiles apart: a joint move brings it down, and
	// with 32 nobody moved for 37 s)
	else if(distance(B, U) < 40 * 32 && (!AirConnected(B, U) || !m_GoalByAir))
	{
		// one above a freeze band, the other below it (open air doesn't connect us), the way going up through it: a catch
		if(absolute(DistB - DistU) > 300 && absolute(B.x - U.x) < 20 * 32 && absolute(B.y - U.y) < 25 * 32 && !AirConnected(B, U))
			Plan = PlanCatch(S, *m_pGoal, m_TeamBudget);
		// or one of us falls into the freeze and the other catches it (Stronghold below the freeze shaft: one on the floor
		// above the freeze holes at x 413-421, the other on the floor below; the one above walks into the hole at x 446-448
		// and the other drags it out of the freeze pool under it)
		if(!Plan.m_Valid && !m_GoalByAir)
			Plan = PlanFall(S, m_pGoalFar ? *m_pGoalFar : *m_pGoal, m_TeamBudget, m_Pseudo);
		// else (or none found) the general one: apart, with freeze or a gap between us, or the way on beyond freeze
		// (Stronghold's corridor of freeze walls: on either side of the gap between two blocks, nobody moved)
		if(!Plan.m_Valid)
			Plan = PlanJoint(S, *m_pGoal, m_TeamBudget);
		if(!Plan.m_Valid)
			Plan = PlanJoint(S, *m_pGoal, 2 * m_TeamBudget, 1);
	}
	else
		return false;
	if(!Plan.m_Valid)
	{
		m_aTeamFailPos[0] = B;
		m_aTeamFailPos[1] = U;
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "no %s from here (%d ticks searched)", Plan.m_Kind == TEAM_DRAG ? "drag" : Plan.m_Kind == TEAM_COLUMN ? "column drop" : Plan.m_Kind == TEAM_FLING ? "fling" : Plan.m_Kind == TEAM_HOP ? "hop" : Plan.m_Kind == TEAM_DASH ? "dash" : Plan.m_Kind == TEAM_THROW ? "throw" : Plan.m_Kind == TEAM_LEAP ? "throw or leap" : Plan.m_Kind == TEAM_JOINT ? "team move" : "catch", Plan.m_Steps);
		m_Say(aBuf);
		return false;
	}
	Plan.m_aId[0] = S.m_aTee[0].m_Core.m_Id;
	Plan.m_aId[1] = S.m_aTee[1].m_Core.m_Id;
	Plan.m_Start = m_Now;
	m_Team = Plan;
	m_TeamEndKind = TEAM_NONE;
	{
		const int Me = Plan.m_aId[0] == pB->m_Core.m_Id ? 0 : 1;
		char aBuf[160];
		if(Plan.m_Kind == TEAM_DROP)
			str_format(aBuf, sizeof(aBuf), Plan.m_Up == Me ? "I drop first, hold me, I'll get you out below (%d ticks searched)" : "you drop first, I hold you, then get me out below (%d ticks searched)", Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_FALL)
			str_format(aBuf, sizeof(aBuf), Plan.m_Up == Me ? "I fall in, catch me: I land at %.1f %.1f (%d ticks searched)" : "you fall in, I catch you: you land at %.1f %.1f (%d ticks searched)",
				Plan.m_aEnd[Plan.m_Up].x / 32, Plan.m_aEnd[Plan.m_Up].y / 32, Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_FLING)
			str_format(aBuf, sizeof(aBuf), Plan.m_Up == Me ? "fling me across: I land at %.1f %.1f (%d ticks searched)" : "I fling you across: you land at %.1f %.1f (%d ticks searched)",
				Plan.m_aEnd[Plan.m_Up].x / 32, Plan.m_aEnd[Plan.m_Up].y / 32, Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_CLIMB)
			str_format(aBuf, sizeof(aBuf), "off we go, up together: I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)", Plan.m_aEnd[Me].x / 32, Plan.m_aEnd[Me].y / 32,
				Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_COLUMN)
			str_format(aBuf, sizeof(aBuf), Plan.m_Up == Me ? "I dive in, hold me under the floor and come after me: I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)" :
									 "you dive in, I hold you under the floor and come after you: I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)",
				Plan.m_aEnd[Me].x / 32, Plan.m_aEnd[Me].y / 32, Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_DRAG)
			str_format(aBuf, sizeof(aBuf), "I drag you along: I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)", Plan.m_aEnd[Me].x / 32, Plan.m_aEnd[Me].y / 32,
				Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_HOP)
			str_format(aBuf, sizeof(aBuf), "up through together: I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)", Plan.m_aEnd[Me].x / 32, Plan.m_aEnd[Me].y / 32,
				Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_DASH)
			str_format(aBuf, sizeof(aBuf), "across together: I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)", Plan.m_aEnd[Me].x / 32, Plan.m_aEnd[Me].y / 32,
				Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_JOINT)
			str_format(aBuf, sizeof(aBuf), "together then: I end at %.1f %.1f, you at %.1f %.1f (%d ticks searched)", Plan.m_aEnd[Me].x / 32, Plan.m_aEnd[Me].y / 32,
				Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_LEAP)
			str_format(aBuf, sizeof(aBuf), "let's jump across together: I land at %.1f %.1f, you at %.1f %.1f (%d ticks searched)", Plan.m_aEnd[Me].x / 32, Plan.m_aEnd[Me].y / 32,
				Plan.m_aEnd[1 - Me].x / 32, Plan.m_aEnd[1 - Me].y / 32, Plan.m_Steps);
		else if(Plan.m_Kind == TEAM_THROW)
			str_format(aBuf, sizeof(aBuf), Plan.m_Up == Me ? "throw me up, I land at %.1f %.1f (%d ticks searched)" : "I throw you up, you land at %.1f %.1f (%d ticks searched)",
				Plan.m_aEnd[Plan.m_Up].x / 32, Plan.m_aEnd[Plan.m_Up].y / 32, Plan.m_Steps);
		else
			str_format(aBuf, sizeof(aBuf), Plan.m_Up == Me ? "jump, catch me: I land at %.1f %.1f (%d ticks searched)" : "jump, I'll catch you: you land at %.1f %.1f (%d ticks searched)",
				Plan.m_aEnd[Plan.m_Up].x / 32, Plan.m_aEnd[Plan.m_Up].y / 32, Plan.m_Steps);
		m_Say(aBuf);
	}
	return TeamMove(In);
}
