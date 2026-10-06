// x_lob: exact lob scan from a run's own positions: for fire race ticks [rt0, rt1] (shot fired in the step from rt to
// rt+1, starting at the run's m_Pos at rt) and aims [a0, a1] in steps, print the explosion race tick and point when it
// falls in the box [bx0,bx1]x[by0,by1] and in ticks [e0, e1].
// usage: x_lob <map> <inputs> rt0 rt1 a0 a1 step bx0 bx1 by0 by1 e0 e1
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include <game/collision.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>
static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	int d, j, h, fi, tx, ty, w;
	char aLine[256];
	while(f && std::fgets(aLine, sizeof(aLine), f))
	{
		int n = std::sscanf(aLine, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w);
		if(n < 6)
			continue;
		STasInput In;
		In.m_Dir = d; In.m_Jump = j; In.m_Hook = h; In.m_Fire = fi; In.m_TX = tx; In.m_TY = ty; In.m_Weapon = n >= 7 ? w : -1;
		v.push_back(In);
	}
	if(f)
		std::fclose(f);
	return v;
}
int main(int argc, const char **argv)
{
	if(argc < 14 || !CTasGame::LoadMap(argv[1]))
		return 1;
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	int Rt0 = atoi(argv[3]), Rt1 = atoi(argv[4]);
	float A0 = atof(argv[5]), A1 = atof(argv[6]), St = atof(argv[7]);
	float Bx0 = atof(argv[8]), Bx1 = atof(argv[9]), By0 = atof(argv[10]), By1 = atof(argv[11]);
	int E0 = atoi(argv[12]), E1 = atoi(argv[13]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i = 0;
	for(; i < vIn.size() && !G.HasGrenade(); i++)
		G.Step(vIn[i]);
	CFastG F;
	F.FromGame(G);
	std::map<int, vec2> Pos;
	for(; i < vIn.size(); i++)
	{
		Pos[F.RaceTick()] = F.m_Pos;
		F.Step(vIn[i]);
		if(F.m_FinishTick >= 0)
			break;
	}
	const float Curv = F.m_Core.m_Tuning.m_GrenadeCurvature, Speed = F.m_Core.m_Tuning.m_GrenadeSpeed;
	for(int Rt = Rt0; Rt <= Rt1; Rt++)
	{
		vec2 P = Pos[Rt];
		for(float A = A0; A <= A1; A += St)
		{
			// the aim as an input would be quantised: use 10000-scaled components like the searches
			int TX = (int)std::lround(std::cos(A * pi / 180) * 10000), TY = (int)std::lround(std::sin(A * pi / 180) * 10000);
			vec2 D = normalize(vec2(TX, TY));
			vec2 P0 = P + D * 21.0f;
			for(int Tau = 1; Tau <= 100; Tau++)
			{
				vec2 Pr = CalcPos(P0, D, Curv, Speed, (Tau - 1) / (float)SERVER_TICK_SPEED);
				vec2 Cu = CalcPos(P0, D, Curv, Speed, Tau / (float)SERVER_TICK_SPEED);
				vec2 Col, NP;
				if(CTasGame::Collision()->IntersectLine(Pr, Cu, &Col, &NP))
				{
					int Re = Rt + Tau;
					if(Re >= E0 && Re <= E1 && Col.x >= Bx0 && Col.x <= Bx1 && Col.y >= By0 && Col.y <= By1)
						std::printf("fire rt %d (pos %.0f %.0f) aim %.2f (tx %d ty %d) -> explodes in the step to rt %d at %.1f %.1f\n", Rt + 1, P.x, P.y, A, TX, TY, Re, Col.x, Col.y);
					break;
				}
			}
		}
	}
	return 0;
}
