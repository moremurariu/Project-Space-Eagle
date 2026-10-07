// x_pert: how sensitive is a run to a small state error? Replay the run (exact CFastG stepper) to race tick K, perturb
// the state (velocity in 1/256 px/t quanta, or position in px), keep replaying the run's own inputs and report where
// the perturbed run ends (finish race tick, or death) and how far it strayed. Tells whether an approximate rejoin onto
// a run (same position and velocity up to a few quanta) can reuse that run's remaining inputs.
// usage: x_pert <map> <inputs> K1,K2,.. [q=1,4,16,64] [px=1,2]
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
#include <cstring>
#include <sstream>
#include <string>
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

static std::vector<int> Ints(const char *s)
{
	std::vector<int> v;
	std::stringstream ss(s);
	std::string t;
	while(std::getline(ss, t, ','))
		if(!t.empty())
			v.push_back(std::atoi(t.c_str()));
	return v;
}

int main(int argc, const char **argv)
{
	if(argc < 4 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_pert <map> <inputs> K1,K2,.. [q=1,4,16,64] [px=1,2]\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	std::vector<int> vK = Ints(argv[3]), vQ = {1, 4, 16, 64}, vPx = {1, 2};
	for(int a = 4; a < argc; a++)
	{
		if(!std::strncmp(argv[a], "q=", 2))
			vQ = Ints(argv[a] + 2);
		else if(!std::strncmp(argv[a], "px=", 3))
			vPx = Ints(argv[a] + 3);
	}
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i0 = 0;
	for(; i0 < vIn.size() && !G.HasGrenade(); i0++)
		G.Step(vIn[i0]);
	CFastG F0;
	F0.FromGame(G);
	// unperturbed reference positions
	std::vector<vec2> vRef;
	int RefFin = -1;
	{
		CFastG F = F0;
		for(size_t i = i0; i < vIn.size(); i++)
		{
			F.Step(vIn[i]);
			vRef.push_back(F.m_Core.m_Pos);
			if(F.m_FinishTick >= 0)
			{
				RefFin = F.RaceTick();
				break;
			}
		}
	}
	std::printf("reference finish rt %d\n", RefFin);
	for(int K : vK)
	{
		CFastG F = F0;
		size_t i = i0;
		for(; i < vIn.size() && F.RaceTick() < K; i++)
			F.Step(vIn[i]);
		const size_t iK = i;
		struct SP
		{
			const char *m_pName;
			float m_Dx, m_Dy, m_Dvx, m_Dvy;
			int m_Amount;
		};
		std::vector<SP> vP;
		for(int q : vQ)
		{
			float d = q / 256.0f;
			vP.push_back({"vx+", 0, 0, d, 0, q});
			vP.push_back({"vx-", 0, 0, -d, 0, q});
			vP.push_back({"vy+", 0, 0, 0, d, q});
			vP.push_back({"vy-", 0, 0, 0, -d, q});
		}
		for(int p : vPx)
		{
			vP.push_back({"x+", (float)p, 0, 0, 0, p});
			vP.push_back({"x-", (float)-p, 0, 0, 0, p});
			vP.push_back({"y+", 0, (float)p, 0, 0, p});
			vP.push_back({"y-", 0, (float)-p, 0, 0, p});
		}
		std::printf("K %d (pos %.0f %.0f vel %.3f %.3f):", K, F.m_Core.m_Pos.x, F.m_Core.m_Pos.y, F.m_Core.m_Vel.x, F.m_Core.m_Vel.y);
		for(const SP &P : vP)
		{
			CFastG H = F;
			H.m_Core.m_Vel += vec2(P.m_Dvx, P.m_Dvy);
			H.m_Core.m_Pos += vec2(P.m_Dx, P.m_Dy);
			H.m_Pos = H.m_Core.m_Pos;
			float MaxDev = 0;
			int Merge = -1, Fin = -1;
			bool Dead = false;
			for(size_t j = iK; j < vIn.size(); j++)
			{
				H.Step(vIn[j]);
				size_t r = j - i0;
				if(r < vRef.size())
				{
					float dv = distance(H.m_Core.m_Pos, vRef[r]);
					MaxDev = std::max(MaxDev, dv);
					if(dv == 0 && Merge < 0)
						Merge = H.RaceTick();
					else if(dv > 0)
						Merge = -1;
				}
				if(H.m_Dead)
				{
					Dead = true;
					break;
				}
				if(H.m_FinishTick >= 0)
				{
					Fin = H.RaceTick();
					break;
				}
			}
			std::printf(" %s%d:%s", P.m_pName, P.m_Amount, Dead ? "dead" : Fin >= 0 ? std::to_string(Fin).c_str() : "nofin");
			if(Merge >= 0)
				std::printf("(merged@%d)", Merge);
			else
				std::printf("(dev %.0f)", MaxDev);
		}
		std::printf("\n");
		std::fflush(stdout);
	}
	return 0;
}
