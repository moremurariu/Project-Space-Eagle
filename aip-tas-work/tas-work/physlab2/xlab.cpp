// xlab: interactive physics lab with save/restore (physlab2, new tool; links the existing build-sim objects).
// Reads commands from stdin, one per line:
//   load PATH [N]        fresh spawn + replay the first N input lines of PATH (all if N omitted)
//   save K / restore K   copy the whole world (tee, hook, projectiles, reload) into / from slot K
//   in d j h f tx ty [n] hold this input n ticks (weapon = grenade once picked up)
//   r d j h f tx ty w    one tick of a replay-file line (explicit weapon)
//   a d j h f deg [n]    same with the aim given in degrees (tx,ty = 1000*cos,sin; y down)
//   p                    print the state
//   tp x y vx vy / gren / reload0   experiments only (teleport, give grenade, clear reload)
//   loud / quiet         print / don't print after every tick
//   exp                  print all pending explosions (tick, pos)
//   sync                 print "OK" and flush (use after a batch)
// State line: S rt x y vx vy hook hx hy jumped grounded reload proj frz gren(0 none,1 got,2 active)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/entities/projectile.h>
#include <game/client/prediction/gameworld.h>
#include "../../ddnet/src/tas/sim.h"
#undef private
#undef protected

#include <game/collision.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

static void Print(const CTasGame &G)
{
	vec2 P = G.Pos(), V = G.Vel(), H = G.HookPos();
	std::printf("S %d %.2f %.2f %.4f %.4f %d %.1f %.1f %d %d %d %d %d %d\n", G.m_Started ? G.m_Tick - G.m_StartTick : -1, P.x, P.y, V.x, V.y,
		G.HookState(), H.x, H.y, G.Jumped(), G.Grounded(), G.ReloadTimer(), G.NumProjectiles(), (G.Frozen() || G.EnteredFreeze()) ? 1 : 0,
		G.HasGrenade() ? (G.ActiveWeapon() == WEAPON_GRENADE ? 2 : 1) : 0);
}

static void Explosions(const CTasGame &G)
{
	// like NextExplosion, but for every projectile
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
				std::printf("E %d %.2f %.2f\n", T - G.m_StartTick, ColPos.x, ColPos.y);
				break;
			}
		}
	}
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: xlab <map>  (commands on stdin)\n");
		return 1;
	}
	std::vector<std::unique_ptr<CTasGame>> vSlots;
	auto G = std::make_unique<CTasGame>();
	G->Spawn(CTasGame::Map().m_vSpawns[0]);
	bool Loud = false;
	std::string Line;
	auto Step = [&](int d, int j, int h, int f, int tx, int ty, int n) {
		STasInput In;
		In.m_Dir = d;
		In.m_Jump = j;
		In.m_Hook = h;
		In.m_Fire = f;
		In.m_TX = tx;
		In.m_TY = ty;
		In.m_Weapon = G->HasGrenade() ? WEAPON_GRENADE : -1;
		for(int i = 0; i < n; i++)
		{
			G->Step(In);
			if(Loud)
				Print(*G);
		}
	};
	while(std::getline(std::cin, Line))
	{
		std::istringstream L(Line);
		std::string Cmd;
		if(!(L >> Cmd))
			continue;
		if(Cmd == "load")
		{
			std::string Path;
			int N = 1 << 30;
			L >> Path;
			if(!(L >> N))
				N = 1 << 30;
			G = std::make_unique<CTasGame>();
			G->Spawn(CTasGame::Map().m_vSpawns[0]);
			FILE *f = std::fopen(Path.c_str(), "r");
			int d, j, h, fi, tx, ty, w, k = 0;
			while(f && k < N && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
			{
				STasInput In;
				In.m_Dir = d;
				In.m_Jump = j;
				In.m_Hook = h;
				In.m_Fire = fi;
				In.m_TX = tx;
				In.m_TY = ty;
				In.m_Weapon = w;
				G->Step(In);
				k++;
				if(Loud)
					Print(*G);
			}
			if(f)
				std::fclose(f);
		}
		else if(Cmd == "save" || Cmd == "restore")
		{
			int K;
			L >> K;
			while((int)vSlots.size() <= K)
				vSlots.push_back(std::make_unique<CTasGame>());
			if(Cmd == "save")
				vSlots[K]->CopyFrom(*G);
			else
				G->CopyFrom(*vSlots[K]);
		}
		else if(Cmd == "in")
		{
			int d, j, h, f, tx, ty, n = 1;
			L >> d >> j >> h >> f >> tx >> ty;
			if(!(L >> n))
				n = 1;
			Step(d, j, h, f, tx, ty, n);
		}
		else if(Cmd == "r")
		{
			// one replay-file line: d j h f tx ty w (explicit weapon)
			int d, j, h, f, tx, ty, w;
			L >> d >> j >> h >> f >> tx >> ty >> w;
			STasInput In;
			In.m_Dir = d;
			In.m_Jump = j;
			In.m_Hook = h;
			In.m_Fire = f;
			In.m_TX = tx;
			In.m_TY = ty;
			In.m_Weapon = w;
			G->Step(In);
			if(Loud)
				Print(*G);
		}
		else if(Cmd == "a")
		{
			int d, j, h, f, n = 1;
			double Deg;
			L >> d >> j >> h >> f >> Deg;
			if(!(L >> n))
				n = 1;
			int tx = (int)std::lround(std::cos(Deg * M_PI / 180) * 1000), ty = (int)std::lround(std::sin(Deg * M_PI / 180) * 1000);
			Step(d, j, h, f, tx, ty, n);
		}
		else if(Cmd == "p")
			Print(*G);
		else if(Cmd == "tp")
		{
			float px, py, vx, vy;
			L >> px >> py >> vx >> vy;
			G->SetState(vec2(px, py), vec2(vx, vy));
		}
		else if(Cmd == "gren")
		{
			CCharacter *pChr = G->Chr();
			pChr->GiveWeapon(WEAPON_GRENADE);
			pChr->m_Core.m_ActiveWeapon = WEAPON_GRENADE;
		}
		else if(Cmd == "reload0")
			G->Chr()->m_ReloadTimer = 0;
		else if(Cmd == "loud")
			Loud = true;
		else if(Cmd == "quiet")
			Loud = false;
		else if(Cmd == "exp")
			Explosions(*G);
		else if(Cmd == "sync")
		{
			std::printf("OK\n");
			std::fflush(stdout);
		}
	}
	std::fflush(stdout);
	return 0;
}
