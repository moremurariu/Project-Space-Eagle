// x_pfscan: pre-fire scan. For every fire tick F0..F1 on a run (its own state at that tick) and every aim (0.1 deg
// steps), the grenade's flight (exact collision) and its explosion tick / point; the kick the run's tee would get there
// (12 x (1 - clamp((d - 48) / 87)), from the explosion to the tee) and its component along the run's velocity at that
// tick. Lists the best pre-fires that explode in E0..E1 (e.g. a turn exit), i.e. candidates to turn an approach kick
// that the next turn brakes away into a second kick at the exit (a lob + point-blank stack).
// usage: x_pfscan MAP RUN F0 F1 E0 E1 [N=20] [dir=dx,dy (score along this direction instead of the run's velocity)]
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include "tasio.h"

#include <game/collision.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

static const float R0 = 28.0f * 0.75f;

int main(int argc, const char **argv)
{
	if(argc < 7 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_pfscan MAP RUN F0 F1 E0 E1 [N=20] [dir=dx,dy]\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const int F0 = std::atoi(argv[3]), F1 = std::atoi(argv[4]), E0 = std::atoi(argv[5]), E1 = std::atoi(argv[6]);
	int NBest = 20;
	vec2 Dir(0, 0);
	for(int i = 7; i < argc; i++)
	{
		std::string A = argv[i];
		if(A.rfind("dir=", 0) == 0)
			std::sscanf(A.c_str() + 4, "%f,%f", &Dir.x, &Dir.y);
		else
			NBest = std::atoi(A.c_str());
	}
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i0 = 0;
	for(; i0 < vIn.size() && !G.HasGrenade(); i0++)
		G.Step(vIn[i0]);
	CFastG S;
	S.FromGame(G);
	// the run's tee (character position, velocity) by race tick, and its state at the fire ticks
	std::vector<vec2> vP(4000), vV(4000);
	std::vector<CFastG> vF;
	std::vector<int> vFRt;
	for(size_t i = i0; i < vIn.size(); i++)
	{
		int Rt = S.RaceTick();
		if(Rt >= F0 - 1 && Rt <= F1 - 1)
		{
			vF.push_back(S);
			vFRt.push_back(Rt + 1); // the shot is fired in the step to race tick Rt + 1
		}
		S.Step(vIn[i]);
		if(S.RaceTick() >= 0 && S.RaceTick() < 4000)
		{
			vP[S.RaceTick()] = S.m_Pos;
			vV[S.RaceTick()] = S.m_Core.m_Vel;
		}
		if(S.m_FinishTick >= 0)
			break;
	}
	struct SC
	{
		float m_Score, m_F, m_Cos, m_D;
		int m_Fire, m_Expl;
		float m_Ang;
		vec2 m_X;
	};
	std::vector<SC> vC;
	const CTuningParams &Tu = S.m_Core.m_Tuning;
	const float Curv = Tu.m_GrenadeCurvature, Speed = Tu.m_GrenadeSpeed;
	for(size_t q = 0; q < vF.size(); q++)
	{
		const CFastG &St = vF[q];
		// CCharacter::m_Pos at FireWeapon is the position after the previous step
		vec2 P = St.m_Pos;
		for(int a = 0; a < 3600; a++)
		{
			float Ang = a * 0.1f * pi / 180.0f;
			vec2 D(std::cos(Ang), std::sin(Ang));
			vec2 P0 = P + D * R0;
			vec2 Prev = P0, Col, NewPos;
			for(int k = 1; k <= 100; k++)
			{
				vec2 Cur = CalcPos(P0, D, Curv, Speed, k / (float)SERVER_TICK_SPEED);
				if(CTasGame::Collision()->IntersectLine(Prev, Cur, &Col, &NewPos))
				{
					int E = vFRt[q] + k;
					if(E >= E0 && E <= E1 && E < 4000)
					{
						vec2 Tee = vP[E - 1]; // position when the projectiles tick (before the tee's next move)
						float d = distance(Tee, Col);
						float l = 1 - std::clamp((d - 48.0f) / 87.0f, 0.0f, 1.0f);
						float F = (int)(6.0f * l) ? 12.0f * l : 0.0f;
						if(F > 0)
						{
							vec2 Kd = normalize(Tee - Col);
							vec2 Ref = length(Dir) > 0 ? normalize(Dir) : normalize(vV[E - 1]);
							float c = dot(Kd, Ref);
							vC.push_back({F * c, F, c, d, vFRt[q], E, a * 0.1f, Col});
						}
					}
					break;
				}
				Prev = Cur;
			}
		}
	}
	std::sort(vC.begin(), vC.end(), [](const SC &a, const SC &b) { return a.m_Score > b.m_Score; });
	std::printf("%zu candidates exploding in %d..%d\n", vC.size(), E0, E1);
	// best per (fire, explosion tick)
	std::vector<std::pair<int, int>> vSeen;
	int n = 0;
	for(const SC &c : vC)
	{
		auto Key = std::make_pair(c.m_Fire, c.m_Expl);
		if(std::find(vSeen.begin(), vSeen.end(), Key) != vSeen.end())
			continue;
		vSeen.push_back(Key);
		std::printf("fire %d aim %6.1f -> expl %d at %.0f,%.0f dist %.0f |f| %.1f cos %.2f along %.1f (run v %.1f,%.1f at %d)\n", c.m_Fire, c.m_Ang, c.m_Expl,
			c.m_X.x, c.m_X.y, c.m_D, c.m_F, c.m_Cos, c.m_Score, vV[c.m_Expl - 1].x, vV[c.m_Expl - 1].y, c.m_Expl - 1);
		if(++n >= NBest)
			break;
	}
	return 0;
}
