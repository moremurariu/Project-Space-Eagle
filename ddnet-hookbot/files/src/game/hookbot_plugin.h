// The client's view of the hookfly bot's brain, so the brain can be hot-reloaded (cl_hookbot_hotreload, see
// docs/HOOKBOT.md): the client only talks to it through this interface, never through CHookBotBrain's members, so
// src/game/hookbot.cpp and hookbot.h can change freely between reloads. Changing this file, gamecore.h or
// collision.h needs a client rebuild (HOOKBOT_PLUGIN_ABI catches the common cases).
#ifndef GAME_HOOKBOT_PLUGIN_H
#define GAME_HOOKBOT_PLUGIN_H

#include "hookbot.h"

#include <functional>
#include <vector>

class IHookBot
{
public:
	virtual ~IHookBot() = default;
	virtual int Abi() const = 0;
	virtual void Init(CCollision *pCollision, CTeamsCore *pTeams) = 0;
	virtual void Start(int Mode) = 0;
	virtual int Mode() const = 0;
	virtual int DriveDir() const = 0;
	virtual void SetDriveDir(int Dir) = 0;
	virtual void SetSay(std::function<void(const char *)> Say) = 0;
	// a route file's text (false: no waypoints in it); *pMsg: what to say
	virtual bool StartRoute(const char *pText, vec2 Me, vec2 Partner, char *pMsg, int MsgSize) = 0;
	virtual void ClearRoute() = 0;
	virtual int RouteIndex() const = 0;
	virtual void SetGoal(vec2 Tile) = 0;
	virtual bool GoalTile(vec2 *pTile) const = 0; // false: no goal
	virtual CNetObj_PlayerInput Tick(int GameTick, const SHookBotTee &Bot, const SHookBotTee &Partner, bool BotFirst) = 0;
	// for cl_hookbot_log
	virtual void LogState(int *pPlanValid, float *pPlanScore, int *pPlans, int *pFound, int *pFired) const = 0;
};

// what the client and a plugin must agree on: the interface (bump HOOKBOT_PLUGIN_VERSION whenever IHookBot changes)
// and the layouts that cross it
#define HOOKBOT_PLUGIN_VERSION 2
#define HOOKBOT_PLUGIN_ABI ((int)(HOOKBOT_PLUGIN_VERSION * 100000 + sizeof(IHookBot) + 3 * sizeof(SHookBotTee) + 5 * sizeof(CCharacterCore) + 7 * sizeof(CNetObj_PlayerInput)))

extern "C" IHookBot *HookBotCreate();
typedef IHookBot *(*FHookBotCreate)();

#endif
