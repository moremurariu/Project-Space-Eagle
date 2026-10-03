#include "hookbot.h"

#include "entities/character.h"
#include "gamecontext.h"
#include "gamecontroller.h"
#include "player.h"
#include "teams.h"

#include <base/str.h>

#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/collision.h>

void CHookBot::Init(CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
	m_Brain.m_Say = [this](const char *pText) { Say(pText); };
}

int CHookBot::BotId() const
{
	return m_pGameServer->Server()->MaxClients() - 1;
}

void CHookBot::Say(const char *pText)
{
	m_pGameServer->SendChat(BotId(), TEAM_ALL, pText);
}

SHookBotTee CHookBot::TeeState(CCharacter *pChr)
{
	SHookBotTee T;
	T.m_Core = pChr->m_Core;
	T.m_Core.m_Id = pChr->GetPlayer()->GetCid();
	T.m_PrevPos = pChr->m_PrevPos;
	T.m_FreezeTime = pChr->m_FreezeTime;
	T.m_Reload = pChr->m_ReloadTimer;
	T.m_Grounded = pChr->IsGrounded();
	T.m_Input = pChr->m_Input;
	return T;
}

bool CHookBot::TicksFirst(CGameContext *pGameServer, CCharacter *pA, CCharacter *pB)
{
	for(CEntity *pEnt = pGameServer->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER); pEnt; pEnt = pEnt->TypeNext())
	{
		if(pEnt == (CEntity *)pA)
			return true;
		if(pEnt == (CEntity *)pB)
			return false;
	}
	return true;
}

void CHookBot::InitSim(CHookBotSim &Sim, CGameContext *pGameServer, CCharacter *pA, CCharacter *pB)
{
	Sim.Init(pGameServer->Collision(), &pGameServer->m_pController->Teams().m_Core, TeeState(pA), TeeState(pB), TicksFirst(pGameServer, pA, pB));
	Sim.m_Tick = pGameServer->Server()->Tick() + 1;
}

void CHookBot::OnTick()
{
	if(!g_Config.m_SvHookbot)
	{
		m_Active = false;
		return;
	}
	m_Brain.Init(m_pGameServer->Collision(), &m_pGameServer->m_pController->Teams().m_Core);
	if(g_Config.m_DbgDummies < 1)
		g_Config.m_DbgDummies = 1; // the bot is the first debug dummy
	CPlayer *pPlayer = m_pGameServer->m_apPlayers[BotId()];
	m_Active = pPlayer != nullptr;
	if(m_Active && str_comp(m_pGameServer->Server()->ClientName(BotId()), g_Config.m_SvHookbotName) != 0)
		m_pGameServer->Server()->SetClientName(BotId(), g_Config.m_SvHookbotName);
	if(m_Active && str_comp(pPlayer->TeeInfos().m_aSkinName, g_Config.m_SvHookbotSkin) != 0)
		pPlayer->SetTeeInfos(g_Config.m_SvHookbotSkin, false, 0, 0);
}

void CHookBot::OnChat(int ClientId, const char *pMessage)
{
	if(!m_Active || ClientId == BotId() || !str_find_nocase(pMessage, g_Config.m_SvHookbotName))
		return;
	if(str_find_nocase(pMessage, "route"))
	{
		char aPath[IO_MAX_PATH_LENGTH];
		CHookBotBrain::RoutePath(m_pGameServer->Map()->BaseName(), aPath, sizeof(aPath));
		char *pText = m_pGameServer->Storage()->ReadFileStr(aPath, IStorage::TYPE_ALL);
		std::vector<vec2> vRoute;
		const bool Ok = pText && CHookBotBrain::ParseRoute(pText, &vRoute);
		CCharacter *pWho = m_pGameServer->GetPlayerChar(ClientId), *pMe = m_pGameServer->GetPlayerChar(BotId());
		if(!Ok || !pWho || !pMe)
		{
			free(pText);
			Say(!Ok ? "no route for this map" : "can't see us, spawn first");
			return;
		}
		m_Brain.Init(m_pGameServer->Collision(), &m_pGameServer->m_pController->Teams().m_Core);
		Start(CHookBotBrain::MODE_PLAY, ClientId);
		char aBuf[160];
		m_Brain.StartRouteText(pText, pMe->m_Pos, pWho->m_Pos, aBuf, sizeof(aBuf));
		free(pText);
		Say(aBuf);
		return;
	}
	bool GoalHere;
	vec2 GoalTile;
	if(CHookBotBrain::ParseGoal(pMessage, &GoalHere, &GoalTile))
	{
		CCharacter *pWho = m_pGameServer->GetPlayerChar(ClientId);
		if(GoalHere && !pWho)
			return;
		if(GoalHere)
			GoalTile = pWho->m_Pos / 32.0f;
		m_Brain.Init(m_pGameServer->Collision(), &m_pGameServer->m_pController->Teams().m_Core);
		m_Brain.SetGoal(GoalTile);
		m_Brain.m_vRoute.clear();
		m_Brain.m_RouteIndex = -1;
		// a goal is also the order to go there
		if(m_Brain.m_Mode != CHookBotBrain::MODE_HAMMERHIT)
			Start(CHookBotBrain::MODE_PLAY, ClientId);
		char aGoal[128];
		str_format(aGoal, sizeof(aGoal), "ok, let's get to tile %.0f %.0f", GoalTile.x, GoalTile.y);
		Say(aGoal);
		return;
	}
	int Mode = CHookBotBrain::ParseCommand(pMessage, g_Config.m_SvHookbotName);
	bool Come = str_find_nocase(pMessage, "come") || str_find_nocase(pMessage, "tp") || str_find_nocase(pMessage, "here");

	CCharacter *pU = m_pGameServer->GetPlayerChar(ClientId);
	CCharacter *pB = m_pGameServer->GetPlayerChar(BotId());
	const int Dir = CHookBotBrain::ParseDirection(pMessage);
	if(Mode < 0 && !Come && Dir != 2 && (m_Brain.m_Mode == CHookBotBrain::MODE_PSEUDODRIVE || m_Brain.m_Mode == CHookBotBrain::MODE_HAMMERHIT))
	{
		m_Brain.m_DriveDir = Dir;
		Say(Dir < 0 ? "ok, left" : Dir > 0 ? "ok, right" : "ok, straight up");
		return;
	}
	if(Mode < 0 && !Come)
	{
		char aHelp[256];
		str_format(aHelp, sizeof(aHelp), "say: %s hookfly | %s hookfly 2 tile aled | %s pseudofly | %s pseudofly drive right | %s hammerhit right | %s come | %s stop",
			g_Config.m_SvHookbotName, g_Config.m_SvHookbotName, g_Config.m_SvHookbotName, g_Config.m_SvHookbotName, g_Config.m_SvHookbotName, g_Config.m_SvHookbotName,
			g_Config.m_SvHookbotName);
		Say(aHelp);
		return;
	}
	if(pU && pB && (Come || (Mode > CHookBotBrain::MODE_IDLE && distance(pU->m_Pos, pB->m_Pos) > 600)))
	{
		vec2 Pos = pU->m_Pos + vec2(48, 0);
		if(m_pGameServer->Collision()->TestBox(Pos, CCharacterCore::PhysicalSizeVec2()))
			Pos = pU->m_Pos;
		pB->SetPosition(Pos);
		pB->m_Pos = pB->m_PrevPos = Pos;
		pB->ResetVelocity();
		pB->ResetHook();
		pB->Unfreeze();
	}
	if(Mode < 0)
		return;
	Start(Mode, ClientId);
	m_Brain.m_DriveDir = Dir == 2 ? (Mode == CHookBotBrain::MODE_HAMMERHIT ? 1 : 0) : Dir;
	char aBuf[128];
	if(Mode == CHookBotBrain::MODE_IDLE)
		str_copy(aBuf, "ok, stopping");
	else if(Mode == CHookBotBrain::MODE_HOOKFLY)
		str_format(aBuf, sizeof(aBuf), "%s: hookfly! stand next to me, I jump first", m_pGameServer->Server()->ClientName(ClientId));
	else if(Mode == CHookBotBrain::MODE_PSEUDOFLY)
		str_format(aBuf, sizeof(aBuf), "%s: pseudofly! you drive: jump, hook me and steer, I hammer you up", m_pGameServer->Server()->ClientName(ClientId));
	else if(Mode == CHookBotBrain::MODE_PSEUDODRIVE)
		str_format(aBuf, sizeof(aBuf), "%s: pseudofly! I jump and drive, you hammer me. steer me with: %s left / right / up", m_pGameServer->Server()->ClientName(ClientId), g_Config.m_SvHookbotName);
	else if(Mode == CHookBotBrain::MODE_HAMMERHIT)
		str_format(aBuf, sizeof(aBuf), "%s: hammerhit! when you're frozen I hook you out and hammer you, then get me out", m_pGameServer->Server()->ClientName(ClientId));
	else if(Mode == CHookBotBrain::MODE_PLAY)
		str_copy(aBuf, m_Brain.m_pGoal ? "ok, let's get to the goal" : "set a goal first: goal <x> <y> (tiles) or goal here");
	else
		str_format(aBuf, sizeof(aBuf), "%s: hookfly, then I 2-tile aled you through the freeze above", m_pGameServer->Server()->ClientName(ClientId));
	Say(aBuf);
}

void CHookBot::Start(int Mode, int Partner)
{
	m_Partner = Partner;
	m_Brain.Start(Mode);
}

bool CHookBot::GetInput(int ClientId, CNetObj_PlayerInput *pInput)
{
	if(!IsBot(ClientId))
		return false;
	if(m_Partner >= 0 && !m_pGameServer->m_apPlayers[m_Partner])
	{
		m_Brain.Start(CHookBotBrain::MODE_IDLE);
		m_Partner = -1;
	}
	CCharacter *pB = m_pGameServer->GetPlayerChar(BotId());
	CCharacter *pU = m_Partner >= 0 ? m_pGameServer->GetPlayerChar(m_Partner) : nullptr;
	if(pB && pU)
	{
		*pInput = m_Brain.Tick(m_pGameServer->Server()->Tick(), TeeState(pB), TeeState(pU), TicksFirst(m_pGameServer, pB, pU));
	}
	else
	{
		*pInput = {};
		pInput->m_WantedWeapon = WEAPON_HAMMER + 1;
		pInput->m_TargetX = 1;
	}
	return true;
}
