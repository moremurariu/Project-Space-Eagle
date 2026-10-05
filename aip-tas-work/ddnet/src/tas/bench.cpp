// bench <map> <inputs>: cost of CTasGame CopyFrom+Step vs. a bare CCharacterCore step
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/gameworld.h>
#undef private
#undef protected
#include "sim.h"

#include <game/collision.h>
#include <game/gamecore.h>
#include <cmath>

#include <chrono>
#include <cstdio>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	int d, j, h, fi, tx, ty, w;
	while(f && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
	{
		STasInput In;
		In.m_Dir = d; In.m_Jump = j; In.m_Hook = h; In.m_Fire = fi; In.m_TX = tx; In.m_TY = ty; In.m_Weapon = w;
		v.push_back(In);
	}
	if(f)
		std::fclose(f);
	return v;
}

int main(int argc, const char **argv)
{
	CTasGame::LoadMap(argv[1]);
	auto vIn = ReadInputs(argv[2]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	for(int i = 0; i < 500; i++)
		G.Step(vIn[i]);
	if(getenv("DIFFTEST"))
	{
		// random inputs from states along the run: fast paths on vs off must give identical states every tick
		uint64_t R = 12345;
		auto Rnd = [&](int n) { R ^= R << 13; R ^= R >> 7; R ^= R << 17; return (int)(R % (uint64_t)n); };
		CTasGame Base, A, B;
		long Ticks = 0, Bad = 0;
		for(int Trial = 0; Trial < atoi(getenv("DIFFTEST")); Trial++)
		{
			int Start = 60 + Rnd((int)vIn.size() - 60);
			Base.Spawn(CTasGame::Map().m_vSpawns[0]);
			CTasGame G0;
			G0.Spawn(CTasGame::Map().m_vSpawns[0]);
			for(int i = 0; i < Start; i++)
				G0.Step(vIn[i]);
			A.CopyFrom(G0);
			CCollision::ms_FastPaths = false;
			CCharacterCore::ms_TasSolo = false;
			B.CopyFrom(G0);
			STasInput In;
			for(int k = 0; k < 120; k++)
			{
				if(Rnd(4) == 0)
				{
					In.m_Dir = Rnd(3) - 1;
					In.m_Jump = Rnd(6) == 0;
					In.m_Hook = Rnd(2);
					float Ang = Rnd(3600) * 3.14159265f / 1800;
					In.m_TX = (int16_t)(std::cos(Ang) * 1000);
					In.m_TY = (int16_t)(std::sin(Ang) * 1000);
				}
				CCollision::ms_FastPaths = true;
				CCharacterCore::ms_TasSolo = true;
				A.Step(In);
				CCollision::ms_FastPaths = false;
				CCharacterCore::ms_TasSolo = false;
				B.Step(In);
				Ticks++;
				if(A.Hash() != B.Hash() || A.Frozen() != B.Frozen() || A.EnteredFreeze() != B.EnteredFreeze() || A.m_StartTick != B.m_StartTick)
				{
					Bad++;
					std::printf("MISMATCH trial %d start %d tick %d pos %.2f %.2f / %.2f %.2f\n", Trial, Start, k, A.Pos().x, A.Pos().y, B.Pos().x, B.Pos().y);
					break;
				}
				if(A.Frozen())
					break;
			}
		}
		CCollision::ms_FastPaths = true;
		CCharacterCore::ms_TasSolo = true;
		std::printf("difftest: %ld ticks, %ld mismatches\n", Ticks, Bad);
		return 0;
	}
	CTasGame T;
	const int N = getenv("BENCH_N") ? atoi(getenv("BENCH_N")) : 200000;
	auto t0 = std::chrono::steady_clock::now();
	for(int i = 0; i < N; i++)
	{
		T.CopyFrom(G);
		T.Step(vIn[500 + (i % 8)]);
	}
	auto t1 = std::chrono::steady_clock::now();
	for(int i = 0; i < N; i++)
		T.CopyFrom(G);
	auto t2 = std::chrono::steady_clock::now();
	CCharacterCore C = G.Chr()->m_Core;
	double Sum = 0;
	for(int i = 0; i < N; i++)
	{
		CCharacterCore D = C;
		D.SetCoreWorld(nullptr, CTasGame::Collision(), &G.Chr()->GameWorld()->m_Teams);
		CNetObj_PlayerInput In = {};
		In.m_Direction = vIn[500 + (i % 8)].m_Dir;
		In.m_Hook = vIn[500 + (i % 8)].m_Hook;
		In.m_TargetX = vIn[500 + (i % 8)].m_TX;
		In.m_TargetY = vIn[500 + (i % 8)].m_TY;
		D.m_Input = In;
		D.Tick(true);
		D.Move();
		D.Quantize();
		Sum += D.m_Pos.x;
	}
	auto t3 = std::chrono::steady_clock::now();
	auto us = [&](auto a, auto b) { return std::chrono::duration<double, std::micro>(b - a).count() / N; };
	std::printf("copy+step %.3f us, copy %.3f us, core copy+step %.3f us (sizeof core %zu) %.0f\n", us(t0, t1), us(t1, t2), us(t2, t3), sizeof(CCharacterCore), Sum);
}
