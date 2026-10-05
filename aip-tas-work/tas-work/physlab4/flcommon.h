// physlab4 common helpers: entry states (teleport after a prefix), input files, distance field to the finish.
#ifndef FLCOMMON_H
#define FLCOMMON_H
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/entities/projectile.h>
#include <game/client/prediction/gameworld.h>
#include "../../ddnet/src/tas/sim.h"
#undef private
#undef protected
#include "../../ddnet/src/tas/distfield.h"

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	if(!f)
		return v;
	char aLine[512];
	while(std::fgets(aLine, sizeof(aLine), f))
	{
		int d, j, h, fi, tx, ty, w;
		if(std::sscanf(aLine, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
		{
			STasInput In;
			In.m_Dir = d;
			In.m_Jump = j;
			In.m_Hook = h;
			In.m_Fire = fi;
			In.m_TX = tx;
			In.m_TY = ty;
			In.m_Weapon = w;
			v.push_back(In);
		}
	}
	std::fclose(f);
	return v;
}

static void WriteInputs(const char *pPath, const std::vector<STasInput> &v, const char *pHeader = nullptr)
{
	FILE *f = std::fopen(pPath, "w");
	if(!f)
		return;
	for(const auto &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

struct SEntry
{
	vec2 m_P = vec2(7817, 4000), m_V = vec2(60.7f, 21.6f);
	int m_K = 2390; // race tick (Teero track frame) of the entry state
	int m_Reload = 3;
	int m_Air = 1; // 1: air jump available, 0: used
	std::string m_Prefix = "../runs/ex/e1025.txt";
};

static std::vector<STasInput> gs_vPrefix;

// fresh world: spawn, replay the prefix (starts the race, gives the grenade), teleport to the entry
static void MakeEntry(CTasGame &G, const SEntry &E)
{
	if(gs_vPrefix.empty())
		gs_vPrefix = ReadInputs(E.m_Prefix.c_str());
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	for(const auto &In : gs_vPrefix)
		G.Step(In);
	if(!getenv("FL_NOSHIFT"))
	{
		// the race clock must satisfy m_StartTick >= 0 for CheckRace to record the finish: move the world clock
		// forward (no projectiles exist at this point; all other timers are countdowns or relative)
		G.m_Tick += 4000;
		G.m_pWorld->m_GameTick = G.m_Tick;
	}
	G.SetState(E.m_P, E.m_V);
	CCharacterCore &C = G.Chr()->m_Core;
	C.m_HookState = HOOK_IDLE;
	C.m_HookTick = 0;
	C.m_HookPos = E.m_P;
	C.SetHookedPlayer(-1);
	C.m_Jumped = E.m_Air ? 0 : 2;
	C.m_JumpedTotal = E.m_Air ? 0 : 1;
	G.m_LastHook = 0;
	G.m_LastJump = 0;
	G.m_StartTick = G.m_Tick - E.m_K;
	G.Chr()->m_ReloadTimer = E.m_Reload;
	// no projectiles in flight
	for(CEntity *pEnt = G.m_pWorld->FindFirst(CGameWorld::ENTTYPE_PROJECTILE); pEnt;)
	{
		CEntity *pNext = pEnt->TypeNext();
		G.m_pWorld->RemoveEntity(pEnt);
		pEnt = pNext;
	}
}

static int Rt(const CTasGame &G) { return G.m_Tick - G.m_StartTick; }
static bool Dead(const CTasGame &G) { return G.Frozen() || G.EnteredFreeze() || G.m_StartTick == -2; }
static int JumpsLeft(const CTasGame &G) { return G.Grounded() ? 2 : ((G.Jumped() & 2) ? 0 : 1); }

static STasInput MkIn(int d, int j, int h, int f, float Deg)
{
	STasInput In;
	In.m_Dir = d;
	In.m_Jump = j;
	In.m_Hook = h;
	In.m_Fire = f;
	In.m_TX = (int16_t)std::lround(std::cos(Deg * pi / 180) * 1000);
	In.m_TY = (int16_t)std::lround(std::sin(Deg * pi / 180) * 1000);
	if(!In.m_TX && !In.m_TY)
		In.m_TY = -1;
	In.m_Weapon = 3;
	return In;
}

static void PrintState(const CTasGame &G, const STasInput *pIn = nullptr, FILE *f = stdout)
{
	vec2 P = G.Pos(), V = G.Vel();
	if(pIn)
		std::fprintf(f, "in %2d %d %d %d %6.1f | ", pIn->m_Dir, pIn->m_Jump, pIn->m_Hook, pIn->m_Fire, std::atan2((float)pIn->m_TY, (float)pIn->m_TX) * 180 / pi);
	std::fprintf(f, "k %d pos %.0f %.0f v %.2f %.2f |v| %.2f hook %d (%.0f %.0f) j %d gr %d rl %d pr %d frz %d fin %d\n", Rt(G), P.x, P.y, V.x, V.y, length(V), G.HookState(),
		G.HookPos().x, G.HookPos().y, G.Jumped(), (int)G.Grounded(), G.ReloadTimer(), G.NumProjectiles(), (int)Dead(G), G.m_FinishTick >= 0 ? G.m_FinishTick - G.m_StartTick : -1);
}

// all pending explosions: (tick, pos)
static void Explosions(const CTasGame &G, std::vector<std::pair<int, vec2>> &vOut)
{
	vOut.clear();
	for(CEntity *pEnt = G.m_pWorld->FindFirst(CGameWorld::ENTTYPE_PROJECTILE); pEnt; pEnt = pEnt->TypeNext())
	{
		CProjectile *pProj = (CProjectile *)pEnt;
		int Life = pProj->m_LifeSpan;
		for(int T = G.m_Tick + 1; T < G.m_Tick + 200; T++)
		{
			float Pt = (T - pProj->m_StartTick - 1) / (float)SERVER_TICK_SPEED;
			float Ct = (T - pProj->m_StartTick) / (float)SERVER_TICK_SPEED;
			vec2 PrevPos = pProj->GetPos(Pt), CurPos = pProj->GetPos(Ct), ColPos, NewPos;
			int Collide = CTasGame::Collision()->IntersectLine(PrevPos, CurPos, &ColPos, &NewPos);
			if(Life > -1)
				Life--;
			if(Collide || Life == -1)
			{
				vOut.push_back({T - G.m_StartTick, ColPos});
				break;
			}
		}
	}
}

// parse key=value args into the entry
static bool EntryArg(SEntry &E, const std::string &K, const std::string &V)
{
	if(K == "tp")
		std::sscanf(V.c_str(), "%f,%f,%f,%f", &E.m_P.x, &E.m_P.y, &E.m_V.x, &E.m_V.y);
	else if(K == "tpk")
		E.m_K = std::stoi(V);
	else if(K == "reload")
		E.m_Reload = std::stoi(V);
	else if(K == "air")
		E.m_Air = std::stoi(V);
	else if(K == "prefix")
		E.m_Prefix = V;
	else
		return false;
	return true;
}

static SDistField gs_DF;
static void BuildDF()
{
	gs_DFStencil = 3;
	gs_DF.Build(CTasGame::Map(), {TILE_FINISH});
}

#endif
