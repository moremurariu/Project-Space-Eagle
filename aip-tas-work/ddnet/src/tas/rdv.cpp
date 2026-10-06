// rdv: rendezvous (pre-fire) opportunities along a run. From the state at every tick, a grenade fired with each of
// N aims is flown exactly (CFastG, projectiles ignore the tee in solo); if it explodes 'minflight'..'maxflight' ticks
// later next to where the run itself is at that tick, the kick it would have given (strength, d(v^2) with the run's
// velocity) is reported. Shows where long pre-fires / double kicks are geometrically available on our own line.
// usage: rdv MAP INPUTS [from_rt=1000] [to_rt=99999] [aims=180] [minflight=10] [maxflight=30] [mingain=300] [loaded=0]
//   loaded=1: only ticks where the run itself holds a loaded grenade (a lob there displaces none of its shots)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "tasfast.h"

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

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: rdv MAP INPUTS [from_rt] [to_rt] [aims] [minflight] [maxflight] [mingain]\n");
		return 1;
	}
	CFastG::Init();
	const int R0 = argc > 3 ? std::atoi(argv[3]) : 1000, R1 = argc > 4 ? std::atoi(argv[4]) : 99999;
	const int NA = argc > 5 ? std::atoi(argv[5]) : 180, F0 = argc > 6 ? std::atoi(argv[6]) : 10, F1 = argc > 7 ? std::atoi(argv[7]) : 30;
	const float MinGain = argc > 8 ? std::atof(argv[8]) : 300;
	const bool Loaded = argc > 9 && std::atoi(argv[9]);
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	std::vector<CTasFast> vS; // state before input i
	std::vector<vec2> vP, vV; // after input i
	std::vector<int> vRt;
	for(const auto &In : vIn)
	{
		CTasFast F;
		F.FromGameFull(G);
		vS.push_back(F);
		G.Step(In);
		vP.push_back(G.Pos());
		vV.push_back(G.Vel());
		vRt.push_back(G.m_Started && G.m_StartTick >= 0 ? G.m_Tick - G.m_StartTick : -1000);
	}
	int Count = 0;
	for(size_t i = 0; i < vIn.size(); i++)
	{
		const int Rt = vRt[i];
		if(Rt < R0 || Rt > R1 || !vS[i].HasGrenade())
			continue;
		if(Loaded && (vS[i].m_ReloadTimer != 0 || vIn[i].m_Fire))
			continue;
		float BestG = -1e9f;
		int BestA = -1, BestT = 0;
		vec2 BestE;
		for(int a = 0; a < NA; a++)
		{
			float Ang = 2 * pi * a / NA;
			CTasFast F = vS[i];
			F.m_ReloadTimer = 0;
			STasInput In = vIn[i];
			In.m_Fire = 1;
			In.m_Weapon = 3;
			In.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
			In.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
			if(F.m_Fire & 1)
				F.m_Fire++; // make it a fresh press
			F.Step(In);
			vec2 E;
			int Te;
			if(!F.NextExplosion(E, Te))
				continue;
			int Fl = Te - F.m_Tick;
			if(Fl < F0 || Fl > F1)
				continue;
			size_t j = i + Fl + 1; // the run's state after the step that ends at server tick Te ... index of that tick
			if(j >= vP.size())
				continue;
			// the explosion applies before the tee moves in tick Te: compare with the position after tick Te-1
			vec2 P = vP[j - 1], V = vV[j - 1];
			float d = distance(P, E);
			if(d >= 135 + 28)
				continue;
			float l = 1 - std::clamp((d - 48.0f) / 87.0f, 0.0f, 1.0f);
			float Dmg = 6 * l;
			if(!(int)Dmg)
				continue;
			vec2 K = (d > 0 ? normalize(P - E) : vec2(0, 1)) * Dmg * 2;
			float Gain = dot(V + K, V + K) - dot(V, V);
			if(Gain > BestG)
			{
				BestG = Gain;
				BestA = a;
				BestT = Fl;
				BestE = E;
			}
		}
		if(BestA >= 0 && BestG >= MinGain)
		{
			Count++;
			// the run's own next shot after i (a point-blank there stacks with the lob if it comes >= 25 ticks later)
			int Next = -1;
			for(size_t k = i + 1; k < vIn.size() && k < i + 60; k++)
				if(vIn[k].m_Fire && !vIn[k - 1].m_Fire)
				{
					Next = vRt[k];
					break;
				}
			std::printf("rt %d pos %.0f %.0f |v| %.1f: aim %.1f flight %d -> expl %.0f %.0f at rt %d, tee |v| %.1f, gain %+.0f (run's next shot rt %d)\n", Rt, vP[i].x, vP[i].y,
				length(vV[i]), 360.0f * BestA / NA, BestT, BestE.x, BestE.y, Rt + BestT, length(vV[i + BestT]), BestG, Next);
		}
	}
	std::printf("%d ticks with a rendezvous kick >= %.0f\n", Count, MinGain);
	return 0;
}
