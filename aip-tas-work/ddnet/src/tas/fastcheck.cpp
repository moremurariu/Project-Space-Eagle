// Checks CFast against CTasGame (tick-by-tick equality) and measures both step rates.
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fast.h"
#include "tasio.h"
#include <game/collision.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>

static bool Same(const CTasGame &G, const CFast &F, int t, bool Verbose)
{
	const CCharacterCore &A = G.Chr()->m_Core, &B = F.m_Core;
	bool Ok = A.m_Pos == B.m_Pos && A.m_Vel == B.m_Vel && A.m_HookState == B.m_HookState && A.m_HookPos == B.m_HookPos &&
		  A.m_HookDir == B.m_HookDir && A.m_HookTick == B.m_HookTick && A.m_Jumped == B.m_Jumped && A.m_JumpedTotal == B.m_JumpedTotal &&
		  G.m_StartTick == F.m_StartTick && G.HasGrenade() == F.m_Got;
	bool DeadG = G.Frozen() || G.EnteredFreeze() || G.m_StartTick == -2;
	Ok = Ok && DeadG == F.m_Dead;
	if(!Ok && Verbose)
		std::printf("detail: G frozen %d entered %d freezetime %d prevpos %.2f %.2f pos %.2f %.2f | F prevpos %.2f %.2f pos %.2f %.2f\n", G.Frozen(), G.EnteredFreeze(), G.Chr()->m_FreezeTime,
			G.Chr()->m_PrevPos.x, G.Chr()->m_PrevPos.y, G.Chr()->m_Pos.x, G.Chr()->m_Pos.y, F.m_PrevPos.x, F.m_PrevPos.y, F.m_Pos.x, F.m_Pos.y);
	if(!Ok && Verbose)
		std::printf("MISMATCH t %d: G pos %.2f %.2f vel %.5f %.5f hook %d jumped %d start %d got %d dead %d | F pos %.2f %.2f vel %.5f %.5f hook %d jumped %d start %d got %d dead %d\n",
			t, A.m_Pos.x, A.m_Pos.y, A.m_Vel.x, A.m_Vel.y, A.m_HookState, A.m_Jumped, G.m_StartTick, G.HasGrenade(), DeadG,
			B.m_Pos.x, B.m_Pos.y, B.m_Vel.x, B.m_Vel.y, B.m_HookState, B.m_Jumped, F.m_StartTick, F.m_Got, F.m_Dead);
	return Ok;
}

int main(int argc, const char **argv)
{
	if(argc < 3)
	{
		std::printf("usage: fastcheck MAP INPUTS [trials]\n");
		return 1;
	}
	int Trials = argc > 3 ? std::atoi(argv[3]) : 2000;
	if(!CTasGame::LoadMap(argv[1]))
		return 1;
	CFast::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const SMapInfo &M = CTasGame::Map();

	// 1. the run itself
	{
		CTasGame G;
		G.Spawn(M.m_vSpawns[0]);
		CFast F;
		F.FromGame(G);
		for(size_t i = 0; i < vIn.size(); i++)
		{
			CCollision::ms_FastPaths = false;
			G.Step(vIn[i]);
			CCollision::ms_FastPaths = true;
			F.Step(vIn[i]);
			if(!Same(G, F, i + 1, true))
				return 1;
		}
		std::printf("run: identical over %d ticks, pickup %d at race tick %d\n", (int)vIn.size(), F.m_Got, F.RaceTick());
	}

	// 2. random mutations from random cut points (both continue from the same prefix state)
	std::mt19937 Rng(12345);
	std::vector<CTasGame *> vPrefix; // states every 50 ticks
	{
		CTasGame G;
		G.Spawn(M.m_vSpawns[0]);
		for(size_t i = 0; i < vIn.size(); i++)
		{
			if(i % 50 == 0)
			{
				CTasGame *p = new CTasGame();
				p->CopyFrom(G);
				vPrefix.push_back(p);
			}
			G.Step(vIn[i]);
		}
	}
	long Ticks = 0;
	int Bad = 0;
	for(int Tr = 0; Tr < Trials; Tr++)
	{
		int Pi = Rng() % vPrefix.size();
		CTasGame G;
		G.CopyFrom(*vPrefix[Pi]);
		CFast F;
		F.FromGame(G);
		int Len = 20 + Rng() % 200;
		int Mode = Rng() % 3;
		for(int k = 0; k < Len; k++)
		{
			size_t i = Pi * 50 + k;
			STasInput In = i < vIn.size() ? vIn[i] : vIn.back();
			if(Mode == 0 || (Rng() % 4 == 0))
			{
				// random input
				In.m_Dir = (int)(Rng() % 3) - 1;
				In.m_Jump = Rng() % 4 == 0;
				In.m_Hook = Rng() % 2;
				float A = (Rng() % 3600) / 3600.0f * 2 * 3.14159265f;
				In.m_TX = (int)(std::cos(A) * 300);
				In.m_TY = (int)(std::sin(A) * 300);
			}
			CCollision::ms_FastPaths = false;
			G.Step(In);
			CCollision::ms_FastPaths = true;
			F.Step(In);
			Ticks++;
			if(!Same(G, F, k, Bad < 5))
			{
				Bad++;
				break;
			}
			if(F.m_Dead || F.m_Got)
				break;
		}
	}
	std::printf("fuzz: %d trials, %ld ticks, %d mismatches\n", Trials, Ticks, Bad);

	// 3. speed
	{
		CCollision::ms_FastPaths = false;
		auto T0 = std::chrono::steady_clock::now();
		int Reps = getenv("REPS") ? atoi(getenv("REPS")) : 200;
		for(int r = 0; r < Reps; r++)
		{
			CTasGame G;
			G.Spawn(M.m_vSpawns[0]);
			for(const auto &In : vIn)
				G.Step(In);
		}
		double SG = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
		CTasGame G0;
		G0.Spawn(M.m_vSpawns[0]);
		CFast F0;
		F0.FromGame(G0);
		CCollision::ms_FastPaths = true;
		std::printf("fast paths available: %d\n", CTasGame::Collision()->m_FastOk);
		T0 = std::chrono::steady_clock::now();
		int RepsF = getenv("REPSF") ? atoi(getenv("REPSF")) : 2000;
		for(int r = 0; r < RepsF; r++)
		{
			CFast F = F0;
			for(const auto &In : vIn)
				F.Step(In);
		}
		double SF = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
		CCollision::ms_FastPaths = true;
		T0 = std::chrono::steady_clock::now();
		for(int r = 0; r < Reps; r++)
		{
			CTasGame G;
			G.Spawn(M.m_vSpawns[0]);
			for(const auto &In : vIn)
				G.Step(In);
		}
		double SG2 = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
		std::printf("speed: CTasGame with fast collision %.0f ticks/s\n", Reps * vIn.size() / SG2);
		double RG = Reps * vIn.size() / SG, RF = RepsF * vIn.size() / SF;
		std::printf("speed: CTasGame %.0f ticks/s, CFast %.0f ticks/s (x%.1f); state copy %d bytes\n", RG, RF, RF / RG, (int)sizeof(CFast));
	}
	return Bad ? 1 : 0;
}
