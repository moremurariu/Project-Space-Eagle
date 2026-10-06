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
#include <string>
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
// EX = self: DX DY R: rank aims by the kick they give the tee itself (on the stripped path) along (DX, DY); R unused.
int main(int argc, const char **argv)
{
	if(argc < 11 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: lobscan MAP RUN STRIP_RT T0 T1 EX EY R TE0 TE1 [N=7200] | lobscan MAP RUN STRIP_RT T0 T1 self DX DY TE0 TE1 [N]\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const int Strip = std::atoi(argv[3]), T0 = std::atoi(argv[4]), T1 = std::atoi(argv[5]);
	const vec2 Tgt(std::atof(argv[6]), std::atof(argv[7]));
	const float R = std::atof(argv[8]);
	const int Te0 = std::atoi(argv[9]), Te1 = std::atoi(argv[10]), N = argc > 11 ? std::atoi(argv[11]) : 7200;
	const bool Self = std::string(argv[6]) == "self";
	const vec2 Want = Self ? normalize(vec2(std::atof(argv[7]), std::atof(argv[8]))) : vec2(0, 0);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	const int Cut = Strip + 67; // 0-based line index of race tick Strip
	if(Self)
	{
		// self mode: the kick each lob gives the tee itself on the (stripped) run's own path; aims ranked by the kick's
		// component along (DX, DY); prints the best few per fire tick
		for(int i = 0; i < Cut && i < (int)vIn.size(); i++)
			G.Step(vIn[i]);
		static CTasFast B, F2, T2;
		B.FromGameFull(G);
		std::vector<vec2> vBase;
		std::vector<STasInput> vS;
		for(int i = Cut; i < (int)vIn.size() && i - 67 <= Te1 + 1; i++)
		{
			STasInput In = vIn[i];
			In.m_Fire = 0;
			vS.push_back(In);
		}
		F2.CopyFrom(B);
		for(auto &In : vS)
		{
			F2.Step(In);
			vBase.push_back(F2.Vel());
		}
		F2.CopyFrom(B);
		for(int k = 0; k < (int)vS.size(); k++)
		{
			const int Rt = Strip + k;
			if(Rt > T1)
				break;
			if(Rt >= T0 && F2.ReloadTimer() == 0 && F2.HasGrenade())
			{
				struct SR { float m_S; int m_TX, m_TY, m_Te; vec2 m_K; };
				std::vector<SR> vR;
				for(int a = 0; a < N; a++)
				{
					const float Ang = 2 * pi * a / N;
					STasInput S = vS[k];
					S.m_Fire = 1;
					S.m_Weapon = 3;
					S.m_TX = (int16_t)std::lround(std::cos(Ang) * 10000);
					S.m_TY = (int16_t)std::lround(std::sin(Ang) * 10000);
					T2.CopyFrom(F2);
					T2.Step(S);
					for(int j = k + 1; j < (int)vS.size() && Strip + j <= Te1; j++)
					{
						if(std::fabs(T2.Vel().x - vBase[j - 1].x) + std::fabs(T2.Vel().y - vBase[j - 1].y) > 0.3f)
							break;
						T2.Step(vS[j]);
						const vec2 Dv = T2.Vel() - vBase[j];
						if(length(Dv) > 0.5f)
						{
							if(Strip + j >= Te0 && !T2.m_Dead)
								vR.push_back({dot(Dv, Want), S.m_TX, S.m_TY, Strip + j, Dv});
							break;
						}
					}
				}
				std::sort(vR.begin(), vR.end(), [](const SR &x, const SR &y) { return x.m_S > y.m_S; });
				for(int q = 0; q < (int)vR.size() && q < 6; q++)
					std::printf("lob t0 %d aim %d %d te %d kick %.2f %.2f along %.2f\n", Rt, vR[q].m_TX, vR[q].m_TY, vR[q].m_Te, vR[q].m_K.x, vR[q].m_K.y, vR[q].m_S);
			}
			F2.Step(vS[k]);
		}
		return 0;
	}
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
