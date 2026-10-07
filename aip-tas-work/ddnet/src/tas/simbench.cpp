// simbench MAP RUN [mode]: uniform simulator benchmark for every branch's tree.
//   replay : CTasGame steps the whole RUN from spawn (REPS times) -> ns/tick, plus a position checksum
//   beam   : beam-style copy+step: states sampled along RUN, each copied and stepped with random inputs -> ns/expansion
//   fuzz   : exactness: from sampled states, N random-input ticks on the fast stepper(s) vs plain CTasGame with
//            every fast path off; compares position, velocity, hook state, freeze every tick
// Build flags: SB_TASSOLO (CCharacterCore::ms_TasSolo), SB_TECACHE (CCollision::ms_TileExistsCache), SB_CFAST (faraday CFast),
// SB_CFASTG (CFastG / CTasFast). Env: TAS_NOFAST=1 disables the collision fast paths.
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/gameworld.h>
#undef private
#undef protected
#include "sim.h"
#ifdef SB_CFAST
#include "fast.h"
#endif
#ifdef SB_CFASTG
#include "tasfast.h"
#endif

#include <game/collision.h>
#include <game/gamecore.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	int d, j, h, fi, tx, ty, w;
	while(f && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
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
	if(f)
		std::fclose(f);
	return v;
}

static uint64_t gs_R = 0x9e3779b97f4a7c15ull;
static int Rnd(int n)
{
	gs_R ^= gs_R << 13;
	gs_R ^= gs_R >> 7;
	gs_R ^= gs_R << 17;
	return (int)(gs_R % (uint64_t)n);
}

static void SetFast(bool On)
{
	CCollision::ms_FastPaths = On;
#ifdef SB_TASSOLO
	CCharacterCore::ms_TasSolo = On;
#endif
#ifdef SB_TECACHE
	CCollision::ms_TileExistsCache = On;
#endif
}

static STasInput RandomInput(const STasInput &Prev, bool AllowFire)
{
	STasInput In = Prev;
	if(Rnd(4) == 0)
	{
		In.m_Dir = Rnd(3) - 1;
		In.m_Jump = Rnd(6) == 0;
		In.m_Hook = Rnd(2);
		float Ang = Rnd(3600) * 3.14159265f / 1800;
		In.m_TX = (int)(std::cos(Ang) * 1000);
		In.m_TY = (int)(std::sin(Ang) * 1000);
	}
	In.m_Fire = AllowFire && Rnd(10) == 0 ? !Prev.m_Fire : 0;
	return In;
}

static double Now()
{
	return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

int main(int argc, const char **argv)
{
	if(argc < 3)
	{
		std::printf("usage: simbench MAP RUN [replay|beam|fuzz|all]\n");
		return 1;
	}
	CTasGame::LoadMap(argv[1]);
	bool Fast = getenv("TAS_NOFAST") == nullptr;
	SetFast(Fast);
#ifdef SB_CFAST
	CFast::Init();
#endif
#ifdef SB_CFASTG
	CFastG::Init();
#endif
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	std::string Mode = argc > 3 ? argv[3] : "all";
	const int Reps = getenv("REPS") ? atoi(getenv("REPS")) : 20;
	const int Samples = getenv("SAMPLES") ? atoi(getenv("SAMPLES")) : 40;
	const int PerSample = getenv("PER") ? atoi(getenv("PER")) : 5000;
	const int Limit = getenv("LIMIT") ? atoi(getenv("LIMIT")) : (int)vIn.size(); // only use the first LIMIT inputs
	const int N = std::min(Limit, (int)vIn.size());
	std::printf("inputs %d (using %d), fast paths %s\n", (int)vIn.size(), N, Fast ? "on" : "off");

	// reference states along the run (plain stepping), used by beam and fuzz
	std::vector<int> vSampleTick;
	for(int s = 0; s < Samples; s++)
		vSampleTick.push_back(60 + (long)(N - 61) * s / Samples);

	if(Mode == "replay" || Mode == "all")
	{
		double Best = 1e9;
		double Sum = 0;
		for(int r = 0; r < Reps; r++)
		{
			CTasGame G;
			G.Spawn(CTasGame::Map().m_vSpawns[0]);
			double t0 = Now();
			for(int i = 0; i < N; i++)
				G.Step(vIn[i]);
			double t1 = Now();
			Best = std::min(Best, t1 - t0);
			if(r == 0)
			{
				Sum = G.Pos().x * 1000 + G.Pos().y + G.m_StartTick * 7 + (G.m_FinishTick >= 0 ? G.m_FinishTick - G.m_StartTick : -1) * 1e6;
				std::printf("replay end: pos %.2f %.2f start %d finish rt %d frozen %d grenade %d\n", G.Pos().x, G.Pos().y, G.m_StartTick,
					G.m_FinishTick >= 0 ? G.m_FinishTick - G.m_StartTick : -1, G.Frozen(), G.HasGrenade());
			}
		}
		std::printf("RESULT replay CTasGame %.1f ns/tick (best of %d) chk %.2f\n", Best / N * 1e9, Reps, Sum);
#ifdef SB_CFAST
		{
			// CFast only models the segment before the grenade: stop at the pickup
			double BestF = 1e9;
			int Steps = 0;
			for(int r = 0; r < Reps; r++)
			{
				CTasGame G;
				G.Spawn(CTasGame::Map().m_vSpawns[0]);
				CFast F;
				F.FromGame(G);
				double t0 = Now();
				int i = 0;
				for(; i < N && !F.m_Got; i++)
					F.Step(vIn[i]);
				double t1 = Now();
				Steps = i;
				BestF = std::min(BestF, (t1 - t0) / i);
			}
			std::printf("RESULT replay CFast %.1f ns/tick (pre-pickup, %d ticks)\n", BestF * 1e9, Steps);
		}
#endif
#ifdef SB_CFASTG
		{
			double BestF = 1e9;
			for(int r = 0; r < Reps; r++)
			{
				CTasGame G;
				G.Spawn(CTasGame::Map().m_vSpawns[0]);
				CTasFast F;
				F.FromGameFull(G);
				double t0 = Now();
				for(int i = 0; i < N; i++)
					F.Step(vIn[i]);
				double t1 = Now();
				BestF = std::min(BestF, t1 - t0);
				if(r == 0)
					std::printf("replay end CFastG: pos %.2f %.2f finish rt %d dead %d bad %d\n", F.m_Pos.x, F.m_Pos.y,
						F.m_FinishTick >= 0 ? F.m_FinishTick - F.m_StartTick : -1, F.m_Dead, F.m_Bad);
			}
			std::printf("RESULT replay CFastG %.1f ns/tick\n", BestF / N * 1e9);
		}
#endif
	}

	if(Mode == "beam" || Mode == "all" || Mode == "fuzz")
	{
		// sample states
		std::vector<CTasGame> vS(vSampleTick.size());
		{
			CTasGame G;
			G.Spawn(CTasGame::Map().m_vSpawns[0]);
			size_t s = 0;
			for(int i = 0; i < N && s < vSampleTick.size(); i++)
			{
				G.Step(vIn[i]);
				while(s < vSampleTick.size() && vSampleTick[s] == i + 1)
					vS[s++].CopyFrom(G);
			}
		}
		if(Mode == "beam" || Mode == "all")
		{
			// one expansion = copy the parent state + one step with a fresh random input
			std::vector<STasInput> vR(1024);
			STasInput P = vIn[0];
			for(auto &R : vR)
				P = R = RandomInput(P, false);
			{
				CTasGame T;
				double t0 = Now();
				long Cnt = 0;
				for(size_t s = 0; s < vS.size(); s++)
					for(int k = 0; k < PerSample; k++, Cnt++)
					{
						T.CopyFrom(vS[s]);
						T.Step(vR[k & 1023]);
					}
				double t1 = Now();
				std::printf("RESULT beam CTasGame copy+step %.1f ns\n", (t1 - t0) / Cnt * 1e9);
			}
#ifdef SB_CFAST
			{
				std::vector<CFast> vF;
				for(auto &S : vS)
					if(!S.HasGrenade())
					{
						vF.emplace_back();
						vF.back().FromGame(S);
					}
				CFast T;
				double t0 = Now();
				long Cnt = 0;
				for(size_t s = 0; s < vF.size(); s++)
					for(int k = 0; k < PerSample; k++, Cnt++)
					{
						T = vF[s];
						T.Step(vR[k & 1023]);
					}
				double t1 = Now();
				std::printf("RESULT beam CFast copy+step %.1f ns (%zu pre-pickup samples, sizeof %zu)\n", (t1 - t0) / Cnt * 1e9, vF.size(), sizeof(CFast));
			}
#endif
#ifdef SB_CFASTG
			{
				std::vector<CTasFast> vF(vS.size());
				for(size_t s = 0; s < vS.size(); s++)
					vF[s].FromGameFull(vS[s]);
				CTasFast T;
				double t0 = Now();
				long Cnt = 0;
				for(size_t s = 0; s < vF.size(); s++)
					for(int k = 0; k < PerSample; k++, Cnt++)
					{
						T.CopyFrom(vF[s]);
						T.Step(vR[k & 1023]);
					}
				double t1 = Now();
				std::printf("RESULT beam CTasFast copy+step %.1f ns (sizeof %zu)\n", (t1 - t0) / Cnt * 1e9, sizeof(CTasFast));
			}
#endif
		}
		if(Mode == "fuzz" || Mode == "all")
		{
			const int Trials = getenv("TRIALS") ? atoi(getenv("TRIALS")) : 400;
			long Ticks = 0, BadGame = 0, BadF = 0, BadG = 0;
			for(int t = 0; t < Trials; t++)
			{
				const CTasGame &S = vS[Rnd((int)vS.size())];
				CTasGame Ref, A;
				SetFast(false);
				Ref.CopyFrom(S);
				SetFast(Fast);
				A.CopyFrom(S);
#ifdef SB_CFAST
				CFast F;
				F.FromGame(S);
				bool UseF = !S.HasGrenade();
#endif
#ifdef SB_CFASTG
				CTasFast FG;
				FG.FromGameFull(S);
				bool UseG = true;
#endif
				STasInput In = vIn[0];
				for(int k = 0; k < 150; k++)
				{
					In = RandomInput(In, S.HasGrenade());
					SetFast(false);
					Ref.Step(In);
					SetFast(Fast);
					A.Step(In);
					Ticks++;
					vec2 P = Ref.Pos(), V = Ref.Vel();
					bool RefDead = Ref.EnteredFreeze() || Ref.Frozen() || Ref.m_StartTick == -2;
					if(A.Pos() != P || A.Vel() != V || A.Frozen() != Ref.Frozen() || A.HookState() != Ref.HookState())
					{
						if(BadGame++ < 5)
							std::printf("MISMATCH CTasGame(fast) trial %d tick %d\n", t, k);
						break;
					}
#ifdef SB_CFAST
					if(UseF)
					{
						F.Step(In);
						if(F.m_Got || Ref.HasGrenade())
							UseF = false;
						else if(F.m_Core.m_Pos != P || F.m_Core.m_Vel != V || F.m_Dead != RefDead)
						{
							if(BadF++ < 5)
								std::printf("MISMATCH CFast trial %d tick %d\n", t, k);
							UseF = false;
						}
					}
#endif
#ifdef SB_CFASTG
					if(UseG)
					{
						FG.Step(In);
						if(FG.m_Bad)
							UseG = false;
						else if(FG.m_Core.m_Pos != P || FG.m_Core.m_Vel != V || FG.m_Dead != RefDead)
						{
							if(BadG++ < 5)
								std::printf("MISMATCH CFastG trial %d tick %d: pos %.3f %.3f / %.3f %.3f vel %.4f %.4f / %.4f %.4f dead %d frozen %d entered %d fire %d grenade %d\n", t, k, FG.m_Core.m_Pos.x, FG.m_Core.m_Pos.y, P.x, P.y, FG.m_Core.m_Vel.x, FG.m_Core.m_Vel.y, V.x, V.y, FG.m_Dead, Ref.Frozen(), Ref.EnteredFreeze(), In.m_Fire, Ref.HasGrenade());
							UseG = false;
						}
					}
#endif
					if(RefDead)
						break;
				}
			}
			std::printf("RESULT fuzz %ld ticks: CTasGame(fast paths %s) %ld mismatches", Ticks, Fast ? "on" : "off", BadGame);
#ifdef SB_CFAST
			std::printf(", CFast %ld", BadF);
#endif
#ifdef SB_CFASTG
			std::printf(", CFastG %ld", BadG);
#endif
			std::printf("\n");
		}
	}
	return 0;
}
