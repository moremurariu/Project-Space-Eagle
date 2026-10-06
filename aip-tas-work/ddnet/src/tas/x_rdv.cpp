// x_rdv: rendezvous scan along a run. For every fire tick in [rt0, rt1], every aim (step in degrees): fly the grenade
// exactly (tee-independent) to its explosion; the run's own tee position at the explosion step gives the kick it would
// get (exact force formula). Reports the best kicks (speed gain along the run's velocity) per explosion tick.
// usage: x_rdv <map> <inputs> rt0 rt1 [step_deg=0.1] [minflight=2]
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include <game/collision.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	if(!f)
		return v;
	int d, j, h, fi, tx, ty, w;
	char aLine[256];
	while(std::fgets(aLine, sizeof(aLine), f))
	{
		int n = std::sscanf(aLine, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w);
		if(n < 6)
			continue;
		STasInput In;
		In.m_Dir = d;
		In.m_Jump = j;
		In.m_Hook = h;
		In.m_Fire = fi;
		In.m_TX = tx;
		In.m_TY = ty;
		In.m_Weapon = n >= 7 ? w : -1;
		v.push_back(In);
	}
	std::fclose(f);
	return v;
}

int main(int argc, const char **argv)
{
	if(argc < 5 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_rdv <map> <inputs> rt0 rt1 [step] [minflight]\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const int Rt0 = std::atoi(argv[3]), Rt1 = std::atoi(argv[4]);
	const float StepDeg = argc > 5 ? std::atof(argv[5]) : 0.1f;
	const int MinFlight = argc > 6 ? std::atoi(argv[6]) : 2;
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i = 0;
	for(; i < vIn.size() && !G.HasGrenade(); i++)
		G.Step(vIn[i]);
	CFastG F;
	F.FromGame(G);
	// the run's tee: position (m_Pos) and velocity per race tick, after each step
	std::map<int, vec2> Pos, Vel;
	std::map<int, int> Reload;
	int StartTick = F.m_StartTick;
	for(; i < vIn.size(); i++)
	{
		Pos[F.RaceTick()] = F.m_Pos;
		Vel[F.RaceTick()] = F.m_Core.m_Vel;
		Reload[F.RaceTick()] = F.m_ReloadTimer;
		F.Step(vIn[i]);
		if(F.m_FinishTick >= 0)
			break;
	}
	const float Curv = F.m_Core.m_Tuning.m_GrenadeCurvature, Speed = F.m_Core.m_Tuning.m_GrenadeSpeed;
	const float Strength = F.m_Core.m_Tuning.m_ExplosionStrength;
	struct SRes
	{
		float m_Gain = -1e9f;
		int m_FireRt = 0;
		float m_Aim = 0, m_Dist = 0, m_F = 0, m_Cos = 0;
		vec2 m_X;
	};
	std::map<int, SRes> Best; // per explosion race tick
	for(int Rt = Rt0; Rt <= Rt1; Rt++)
	{
		if(!Pos.count(Rt))
			continue;
		// fired in the step from race tick Rt (state Pos[Rt]) -> start tick = Rt + StartTick
		vec2 P = Pos[Rt];
		for(float A = 0; A < 360.0f; A += StepDeg)
		{
			vec2 D(std::cos(A * pi / 180), std::sin(A * pi / 180));
			vec2 P0 = P + D * 21.0f;
			for(int Tau = 1; Tau <= 100; Tau++)
			{
				vec2 Pr = CalcPos(P0, D, Curv, Speed, (Tau - 1) / (float)SERVER_TICK_SPEED);
				vec2 Cu = CalcPos(P0, D, Curv, Speed, Tau / (float)SERVER_TICK_SPEED);
				vec2 Col, NP;
				if(CTasGame::Collision()->IntersectLine(Pr, Cu, &Col, &NP))
				{
					// explodes in the step to race tick Rt + Tau, before the tee moves: tee at Pos[Rt + Tau - 1]
					int Re = Rt + Tau;
					if(Tau < MinFlight || !Pos.count(Re - 1))
						break;
					vec2 Q = Pos[Re - 1], V = Vel[Re - 1];
					float l = distance(Q, Col);
					if(l >= 135.0f + 28.0f)
						break;
					vec2 Fd = l > 0 ? normalize(Q - Col) : vec2(0, 1);
					float k = 1 - std::clamp((l - 48.0f) / (135.0f - 48.0f), 0.0f, 1.0f);
					float Dmg = Strength * k;
					if(!(int)Dmg)
						break;
					vec2 Fk = Fd * Dmg * 2;
					float Gain = length(V + Fk) - length(V);
					SRes &B = Best[Re];
					if(Gain > B.m_Gain)
					{
						B.m_Gain = Gain;
						B.m_FireRt = Rt;
						B.m_Aim = A;
						B.m_Dist = l;
						B.m_F = length(Fk);
						B.m_Cos = length(V) > 0.1f ? dot(normalize(V), normalize(Fk)) : 0;
						B.m_X = Col;
					}
					break;
				}
			}
		}
	}
	for(auto &[Re, B] : Best)
		if(B.m_Gain > 2.0f)
			std::printf("explode rt %d: fire rt %d (flight %d) aim %.1f -> dist %.0f |f| %.1f cos %.2f gain |v| +%.1f at (%.0f,%.0f) |v| %.1f reload@fire %d\n", Re, B.m_FireRt,
				Re - B.m_FireRt, B.m_Aim, B.m_Dist, B.m_F, B.m_Cos, B.m_Gain, B.m_X.x, B.m_X.y, length(Vel[Re - 1]), Reload[B.m_FireRt]);
	return 0;
}
