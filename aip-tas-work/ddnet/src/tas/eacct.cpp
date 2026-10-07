// Per-tick energy accounting of a pre-grenade run with CFast: E = |v|^2 - y (gravity 0.5/tick^2 conserves it in free flight).
#include "fast.h"
#include "tasio.h"
#include <game/collision.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

int main(int argc, const char **argv)
{
	if(argc < 3)
		return 1;
	if(!CTasGame::LoadMap(argv[1]))
		return 1;
	CFast::Init();
	CCollision::ms_FastPaths = true;
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	CFast F;
	F.FromGame(G);
	double Sum[8] = {0};
	const char *apName[] = {"jump", "hook<15", "hookbrake", "collide", "dir0/against", "ground", "other", ""};
	for(size_t i = 0; i < vIn.size(); i++)
	{
		vec2 V0 = F.m_Core.m_Vel, P0 = F.m_Core.m_Pos;
		int J0 = F.m_Core.m_Jumped;
		bool Gr = CTasGame::Collision()->IsOnGround(P0, 28.0f);
		CFast Pre = F;
		F.Step(vIn[i]);
		// velocity right after the core tick (before Move) is not kept; recompute: core-tick-only copy
		CCharacterCore C = Pre.m_Core;
		CNetObj_PlayerInput In = {};
		In.m_Direction = vIn[i].m_Dir;
		In.m_TargetX = vIn[i].m_TX;
		In.m_TargetY = vIn[i].m_TY;
		if(!In.m_TargetX && !In.m_TargetY)
			In.m_TargetY = -1;
		In.m_Jump = vIn[i].m_Jump;
		In.m_Hook = vIn[i].m_Hook;
		C.m_Input = In;
		C.Tick(true, true);
		vec2 VT = C.m_Vel; // after input/gravity/hook
		vec2 V1 = F.m_Core.m_Vel, P1 = F.m_Core.m_Pos;
		float E0 = dot(V0, V0) - P0.y, E1 = dot(V1, V1) - P1.y;
		// split: core tick change (minus gravity's conserved part) and move change
		float dTick = dot(VT, VT) - dot(V0, V0) - (VT.y + V0.y) * 0.5f * 0; // gravity term handled via y below
		float dMove = dot(V1, V1) - dot(VT, VT);
		int Cause = 6;
		bool Jumped = (F.m_Core.m_Jumped & 1) && !(J0 & 1);
		bool Hooked = Pre.m_Core.m_HookState == HOOK_GRABBED || C.m_HookState == HOOK_GRABBED;
		if(Jumped)
			Cause = 0;
		else if(Hooked)
			Cause = length(VT) < 15.0f ? 1 : 2;
		else if(Gr)
			Cause = 5;
		else if(vIn[i].m_Dir == 0 || vIn[i].m_Dir * V0.x < 0)
			Cause = 4;
		float dE = E1 - E0;
		float dCol = dMove;
		Sum[3] += dCol;
		Sum[Cause] += dE - dCol;
		int rt = F.m_Tick - (F.m_StartTick >= 0 ? F.m_StartTick : F.m_Tick);
		if(argc > 3 || std::fabs(dE) > 30)
			std::printf("rt %4d pos %6.0f %6.0f |v| %5.2f E %7.1f dE %7.1f (move %7.1f) in %2d %d %d %4d %4d cause %s%s\n", rt, P1.x, P1.y, length(V1), E1, dE, dCol,
				vIn[i].m_Dir, vIn[i].m_Jump, vIn[i].m_Hook, vIn[i].m_TX, vIn[i].m_TY, apName[Cause], Gr ? " grounded" : "");
		(void)dTick;
	}
	for(int k = 0; k < 7; k++)
		std::printf("total %-12s %9.1f\n", apName[k], Sum[k]);
	return 0;
}
