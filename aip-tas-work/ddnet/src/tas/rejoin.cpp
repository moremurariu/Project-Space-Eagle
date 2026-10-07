// rejoin: exact rejoin search. Track a run Delta ticks ahead of its own timeline (state at rt t targets the run's state at
// t+Delta) with inputs near the run's own; every candidate close to the target is tested by replaying the run's remaining
// inputs from it (exact, fast stepper): if that finishes Delta ticks earlier, the spliced run is written.
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

// usage: rejoin MAP RUN A B DELTA [beam=2000] [out=FILE] [dpos=0 dvel=0 seed=1] [wv=4] [test=40]
//   A: start rt (the run's own state), B: last rt of the window (target state rt B+Delta), DELTA: ticks to gain
//   prefix=FILE: start from the end of another run's inputs instead (e.g. a faster window); the target is still RUN
#include <unordered_set>
struct SRef
{
	vec2 m_Pos, m_Vel, m_HookPos;
	int m_HookState, m_Reload, m_NumProj;
};
struct SNode
{
	CTasFast F;
	int m_Parent;
	STasInput m_In;
	float m_S;
};
int main(int argc, const char **argv)
{
	if(argc < 6 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: rejoin MAP RUN A B DELTA [key=val...]\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const int A = std::atoi(argv[3]), B = std::atoi(argv[4]), D = std::atoi(argv[5]);
	int Beam = 2000, Test = 40;
	float Dp = 0, Dv = 0, Wv = 4;
	unsigned Seed = 1;
	std::string Out = "rejoin_out.txt", Prefix;
	for(int i = 6; i < argc; i++)
	{
		std::string a = argv[i];
		size_t e = a.find('=');
		if(e == std::string::npos)
			continue;
		std::string K = a.substr(0, e), V = a.substr(e + 1);
		if(K == "beam") Beam = std::stoi(V);
		else if(K == "out") Out = V;
		else if(K == "dpos") Dp = std::stof(V);
		else if(K == "dvel") Dv = std::stof(V);
		else if(K == "seed") Seed = std::stoi(V);
		else if(K == "wv") Wv = std::stof(V);
		else if(K == "test") Test = std::stoi(V);
		else if(K == "prefix") Prefix = V;
	}
	// the run's own states from rt A on (fast stepper), and its finish
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	for(int i = 0; i < A + 68 && i < (int)vIn.size(); i++)
		G.Step(vIn[i]);
	static CTasFast F0;
	F0.FromGameFull(G);
	std::vector<SRef> vR(vIn.size() + 2);
	int Fin = -1;
	{
		static CTasFast F;
		F = F0;
		auto Rec = [&](int Rt) {
			vR[Rt] = {F.Pos(), F.Vel(), F.HookPos(), F.HookState(), F.ReloadTimer(), F.NumProjectiles()};
		};
		Rec(A);
		for(int i = A + 68; i < (int)vIn.size(); i++)
		{
			F.Step(vIn[i]);
			Rec(i - 67);
			if(F.m_FinishTick >= 0)
			{
				Fin = F.m_FinishTick - F.m_StartTick;
				break;
			}
			if(F.m_Dead)
				break;
		}
	}
	if(Fin < 0)
	{
		std::printf("the run does not finish on the fast stepper from rt %d\n", A);
		return 1;
	}
	std::printf("run finishes at %d; window rt %d..%d, target = run state Delta=%d ticks later\n", Fin, A, B, D);
	// continuation test: from a state at rt t standing for the run's state at t+D, replay the run's inputs
	auto Cont = [&](const CTasFast &S, int t) {
		static CTasFast F;
		F = S;
		for(int i = t + D + 68; i < (int)vIn.size(); i++)
		{
			F.Step(vIn[i]);
			if(F.m_Dead)
				return -1;
			if(F.m_FinishTick >= 0)
				return F.m_FinishTick - F.m_StartTick;
		}
		return -1;
	};
	auto In = [&](int Rt) { return vIn[std::clamp(Rt + 67, 0, (int)vIn.size() - 1)]; }; // the input that makes state rt
	auto Score = [&](const CTasFast &S, int Rt) {
		const SRef &R = vR[std::min(Rt + D, (int)vR.size() - 1)];
		float d = distance(S.Pos(), R.m_Pos) + Wv * distance(S.Vel(), R.m_Vel);
		if(S.HookState() != R.m_HookState)
			d += 20;
		else if(R.m_HookState == HOOK_GRABBED)
			d += 0.5f * distance(S.HookPos(), R.m_HookPos);
		if(S.ReloadTimer() != R.m_Reload)
			d += 5;
		return d;
	};
	std::vector<std::vector<SNode>> vSteps;
	std::vector<SNode> vCur(1);
	vCur[0].F = F0;
	int Start = A;
	std::vector<STasInput> vPre(vIn.begin(), vIn.begin() + A + 68);
	if(!Prefix.empty())
	{
		vPre = ReadInputs(Prefix.c_str());
		CTasGame P;
		P.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(const auto &I : vPre)
			P.Step(I);
		vCur[0].F.FromGameFull(P);
		Start = (int)vPre.size() - 68;
		std::printf("start from prefix %s at rt %d (pos %.0f %.0f vel %.2f %.2f)\n", Prefix.c_str(), Start, vCur[0].F.Pos().x, vCur[0].F.Pos().y, vCur[0].F.Vel().x, vCur[0].F.Vel().y);
	}
	if(Dp > 0 || Dv > 0)
	{
		auto Rnd = [&]() { Seed = Seed * 1103515245u + 12345u; return ((Seed >> 8) & 0xffff) / 65535.0f * 2.0f - 1.0f; };
		vCur[0].F.SetState(F0.Pos() + vec2(std::round(Rnd() * Dp), std::round(Rnd() * Dp)), F0.Vel() + vec2(Rnd() * Dv, Rnd() * Dv));
		std::printf("perturbed start: pos %.0f %.0f vel %.3f %.3f\n", vCur[0].F.Pos().x, vCur[0].F.Pos().y, vCur[0].F.Vel().x, vCur[0].F.Vel().y);
	}
	vCur[0].m_Parent = -1;
	int BestT = -1, BestIdx = -1, BestFin = Fin;
	for(int t = Start + 1; t <= B && BestT < 0; t++)
	{
		std::vector<SNode> vNext;
		std::unordered_set<uint64_t> Seen;
		for(int p = 0; p < (int)vCur.size(); p++)
		{
			std::vector<STasInput> vA;
			for(int sh = -1; sh <= 1; sh++)
			{
				STasInput Base = In(t + D + sh);
				for(int dir = -1; dir <= 1; dir++)
					for(int h = 0; h < 2; h++)
						for(int j = 0; j < 2; j++)
						{
							STasInput I = Base;
							I.m_Dir = dir;
							I.m_Hook = h ? Base.m_Hook : !Base.m_Hook;
							if(!j)
								I.m_Jump = 0;
							else if(!Base.m_Jump)
								continue;
							bool Dup = false;
							for(auto &X : vA)
								Dup |= X.m_Dir == I.m_Dir && X.m_Jump == I.m_Jump && X.m_Hook == I.m_Hook && X.m_Fire == I.m_Fire && X.m_TX == I.m_TX && X.m_TY == I.m_TY;
							if(!Dup)
								vA.push_back(I);
						}
			}
			for(const auto &I : vA)
			{
				SNode N;
				N.F = vCur[p].F;
				N.F.Step(I);
				if(N.F.m_Dead)
					continue;
				if(!Seen.insert(N.F.Hash()).second)
					continue;
				N.m_Parent = p;
				N.m_In = I;
				N.m_S = Score(N.F, t);
				vNext.push_back(N);
			}
		}
		std::sort(vNext.begin(), vNext.end(), [](const SNode &a, const SNode &b) { return a.m_S < b.m_S; });
		if((int)vNext.size() > Beam)
			vNext.resize(Beam);
		// exact rejoin tests on the closest states
		for(int k = 0; k < (int)vNext.size() && k < Test; k++)
		{
			if(vNext[k].m_S > 60)
				break;
			int f = Cont(vNext[k].F, t);
			if(f >= 0 && f <= Fin - D)
			{
				BestT = t;
				BestIdx = k;
				BestFin = f;
				break;
			}
		}
		if(vNext.empty())
		{
			std::printf("beam died at rt %d\n", t);
			return 0;
		}
		if((t - Start) % 10 == 0 || BestT >= 0)
			std::printf("rt %d: beam %zu best dist %.1f\n", t, vNext.size(), vNext[0].m_S);
		vSteps.push_back(vCur);
		vCur = std::move(vNext);
	}
	if(BestT < 0)
	{
		std::printf("NO REJOIN with Delta %d by rt %d\n", D, B);
		return 0;
	}
	// rebuild: run inputs to rt A, the window inputs, then the run's inputs from rt BestT+D+1
	std::vector<STasInput> vW;
	int Idx = BestIdx;
	vSteps.push_back(vCur);
	for(int s = (int)vSteps.size() - 1; s >= 1; s--)
	{
		vW.push_back(vSteps[s][Idx].m_In);
		Idx = vSteps[s][Idx].m_Parent;
	}
	std::reverse(vW.begin(), vW.end());
	std::vector<STasInput> vOut = vPre;
	vOut.insert(vOut.end(), vW.begin(), vW.end());
	vOut.insert(vOut.end(), vIn.begin() + BestT + D + 68, vIn.end());
	FILE *f = std::fopen(Out.c_str(), "w");
	for(const auto &I : vOut)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", I.m_Dir, I.m_Jump, I.m_Hook, I.m_Fire, I.m_TX, I.m_TY, I.m_Weapon);
	std::fclose(f);
	std::printf("REJOIN at rt %d (= run rt %d), finish %d (run %d) -> %s (%zu inputs)\n", BestT, BestT + D, BestFin, Fin, Out.c_str(), vOut.size());
	return 0;
}
