#include "hookbot_plugin.h"

namespace {
class CHookBotPlugin : public IHookBot
{
	CHookBotBrain m_Brain;

public:
	int Abi() const override { return HOOKBOT_PLUGIN_ABI; }
	void Init(CCollision *pCollision, CTeamsCore *pTeams) override { m_Brain.Init(pCollision, pTeams); }
	void Start(int Mode) override { m_Brain.Start(Mode); }
	int Mode() const override { return m_Brain.m_Mode; }
	int DriveDir() const override { return m_Brain.m_DriveDir; }
	void SetDriveDir(int Dir) override { m_Brain.m_DriveDir = Dir; }
	void SetSay(std::function<void(const char *)> Say) override { m_Brain.m_Say = std::move(Say); }
	bool StartRoute(const char *pText, vec2 Me, vec2 Partner, char *pMsg, int MsgSize) override
	{
		return m_Brain.StartRouteText(pText, Me, Partner, pMsg, MsgSize);
	}
	void ClearRoute() override
	{
		m_Brain.m_vRoute.clear();
		m_Brain.m_vvRoutes.clear();
		m_Brain.m_RouteIndex = -1;
	}
	int RouteIndex() const override { return m_Brain.m_RouteIndex; }
	void SetGoal(vec2 Tile) override { m_Brain.SetGoal(Tile); }
	bool GoalTile(vec2 *pTile) const override
	{
		if(!m_Brain.m_pGoal)
			return false;
		*pTile = m_Brain.m_pGoal->m_GoalTile;
		return true;
	}
	CNetObj_PlayerInput Tick(int GameTick, const SHookBotTee &Bot, const SHookBotTee &Partner, bool BotFirst) override
	{
		return m_Brain.Tick(GameTick, Bot, Partner, BotFirst);
	}
	void LogState(int *pPlanValid, float *pPlanScore, int *pPlans, int *pFound, int *pFired) const override
	{
		*pPlanValid = m_Brain.m_Plan.m_Valid;
		*pPlanScore = m_Brain.m_Plan.m_Score;
		*pPlans = m_Brain.m_RescueStats.m_Plans;
		*pFound = m_Brain.m_RescueStats.m_Found;
		*pFired = m_Brain.m_RescueStats.m_Fired;
	}
};
}

extern "C" IHookBot *HookBotCreate()
{
	return new CHookBotPlugin;
}
