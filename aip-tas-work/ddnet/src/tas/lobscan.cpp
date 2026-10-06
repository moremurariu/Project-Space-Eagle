// eaudit: energy audit of a run (post-grenade, fast stepper): per tick, the change of kinetic energy v^2/2 split into
// explosions, hook, direction input (vs pressing along vx), jump, collisions and the rest (gravity, friction).
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "tasfast.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	if(!f)
		return v;
	int d, j, h, fi, tx, ty, w;
	while(std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
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
	std::fclose(f);
	return v;
}


// lobscan MAP RUN STRIP_RT T0 T1 EX EY R TE0 TE1 [N=7200]: RUN's inputs with every shot from race tick STRIP_RT on removed;
// at each race tick t in [T0, T1] (reload permitting) a shot with each of N aims replaces that tick's input fire; prints
// the aims whose grenade explodes within R px of (EX, EY) at race tick TE0..TE1 (exact flight on the fast stepper).
int main(int argc, const char **argv)
{
	if(argc < 11 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: lobscan MAP RUN STRIP_RT T0 T1 EX EY R TE0 TE1 [N=7200]\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const int Strip = std::atoi(argv[3]), T0 = std::atoi(argv[4]), T1 = std::atoi(argv[5]);
	const vec2 Tgt(std::atof(argv[6]), std::atof(argv[7]));
	const float R = std::atof(argv[8]);
	const int Te0 = std::atoi(argv[9]), Te1 = std::atoi(argv[10]), N = argc > 11 ? std::atoi(argv[11]) : 7200;
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	const int Cut = Strip + 67; // 0-based line index of race tick Strip
	for(int i = 0; i < Cut && i < (int)vIn.size(); i++)
		G.Step(vIn[i]);
	static CTasFast F, T;
	F.FromGameFull(G);
	int Found = 0;
	for(int i = Cut; i < (int)vIn.size(); i++)
	{
		const int Rt = i - 67;
		if(Rt > T1)
			break;
		STasInput In = vIn[i];
		In.m_Fire = 0;
		if(Rt >= T0 && F.ReloadTimer() == 0 && F.HasGrenade())
		{
			int Best = 0;
			for(int a = 0; a < N; a++)
			{
				const float Ang = 2 * pi * a / N;
				STasInput S = In;
				S.m_Fire = 1;
				S.m_Weapon = 3;
				S.m_TX = (int16_t)std::lround(std::cos(Ang) * 10000);
				S.m_TY = (int16_t)std::lround(std::sin(Ang) * 10000);
				T.CopyFrom(F);
				T.Step(S);
				if(T.NumProjectiles() == 0)
					continue;
				vec2 E;
				int Tk;
				if(!T.NextExplosion(E, Tk))
					continue;
				const int Te = Rt + (Tk - T.m_Tick);
				const float D = distance(E, Tgt);
				if(D <= R && Te >= Te0 && Te <= Te1)
				{
					std::printf("lob t0 %d aim %d %d expl %.1f %.1f te %d dist %.1f  tee %.0f %.0f\n", Rt, S.m_TX, S.m_TY, E.x, E.y, Te, D, F.Pos().x, F.Pos().y);
					Best++;
				}
			}
			Found += Best;
		}
		F.Step(In);
		if(F.m_Dead)
		{
			std::printf("dead at rt %d (shots stripped)\n", Rt);
			break;
		}
	}
	std::printf("%d aims\n", Found);
	return 0;
}
