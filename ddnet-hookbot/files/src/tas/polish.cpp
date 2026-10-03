// polish: local search on a complete input sequence against the true objective
// (earliest arrival at a gate, then highest speed there).
// usage: polish <map> base=FILE key=value...
//   prefix=FILE / tp=x,y,vx,vy / gren=1 / jumped=N / started=1   start state (as in tas2); base inputs follow it
//   stopx=PX | stopxlt=PX | goal=x0,y0,x1,y1 (tiles)   gate
//   seconds=S seed=N out=FILE lambda=L (ticks credited per px/tick of arrival speed, default 0.05)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "sim.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
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

static void WriteInputs(const std::string &Path, const std::vector<STasInput> &v)
{
	FILE *f = std::fopen(Path.c_str(), "w");
	for(const auto &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

static float gs_StopX = 0, gs_StopXLt = 0;
static float gs_aGoal[4];
static bool gs_HasGoal = false;
static float gs_Lambda = 0.05f;

static bool AtGate(const CTasGame &G)
{
	vec2 P = G.Pos();
	if(gs_StopX > 0)
		return P.x > gs_StopX;
	if(gs_StopXLt > 0)
		return P.x < gs_StopXLt;
	if(gs_HasGoal)
		return P.x >= gs_aGoal[0] * 32 && P.y >= gs_aGoal[1] * 32 && P.x < (gs_aGoal[2] + 1) * 32 && P.y < (gs_aGoal[3] + 1) * 32;
	return G.m_FinishTick >= 0;
}

struct SRes
{
	bool m_Ok = false;
	int m_Ticks = 1 << 30; // inputs used until the gate
	float m_Speed = 0;
	float Value() const { return m_Ok ? m_Ticks - gs_Lambda * m_Speed : 1e9f; }
};

static const int CP = 20;

struct SEval
{
	std::unique_ptr<CTasGame> m_pStart;
	std::vector<std::unique_ptr<CTasGame>> m_vCp; // state before input i*CP
	CTasGame m_G;

	void Checkpoints(const std::vector<STasInput> &v)
	{
		m_vCp.clear();
		auto pG = std::make_unique<CTasGame>();
		pG->CopyFrom(*m_pStart);
		for(size_t i = 0; i <= v.size(); i++)
		{
			if(i % CP == 0)
			{
				auto pC = std::make_unique<CTasGame>();
				pC->CopyFrom(*pG);
				m_vCp.push_back(std::move(pC));
			}
			if(i < v.size())
			{
				pG->Step(v[i]);
				if(AtGate(*pG))
					break;
			}
		}
	}
	SRes Eval(const std::vector<STasInput> &v, int From)
	{
		SRes R;
		int c = std::min(From / CP, (int)m_vCp.size() - 1);
		m_G.CopyFrom(*m_vCp[c]);
		for(size_t i = (size_t)c * CP; i < v.size(); i++)
		{
			m_G.Step(v[i]);
			if(m_G.Frozen() || m_G.EnteredFreeze() || m_G.m_StartTick == -2)
				return R;
			if(AtGate(m_G))
			{
				R.m_Ok = true;
				R.m_Ticks = (int)i + 1;
				R.m_Speed = length(m_G.Vel());
				return R;
			}
		}
		return R;
	}
};

static uint64_t gs_Seed = 88172645463325252ull;
static int Rnd(int n)
{
	gs_Seed ^= gs_Seed << 13;
	gs_Seed ^= gs_Seed >> 7;
	gs_Seed ^= gs_Seed << 17;
	return (int)(gs_Seed % (uint64_t)std::max(1, n));
}

static void SetAim(STasInput &In, float Ang)
{
	In.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
	In.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
	if(!In.m_TX && !In.m_TY)
		In.m_TY = -1;
}
static float AimOf(const STasInput &In) { return std::atan2((float)In.m_TY, (float)In.m_TX); }

// segments [a, b) where a flag is held
template<typename F>
static void Segments(const std::vector<STasInput> &v, int L, F Flag, std::vector<std::pair<int, int>> &vOut)
{
	vOut.clear();
	for(int t = 0; t < L;)
	{
		if(!Flag(v[t]))
		{
			t++;
			continue;
		}
		int a = t;
		while(t < L && Flag(v[t]))
			t++;
		vOut.push_back({a, t});
	}
}

static const char *gs_apKind[] = {"shot-aim", "shot-move", "shot-add", "shot-del", "hook-edge", "hook-aim", "jump", "dir", "hook-add"};

// returns the first changed tick, or -1
static int Mutate(std::vector<STasInput> &v, int L, int &Kind)
{
	std::vector<std::pair<int, int>> vSeg;
	Kind = Rnd(9);
	auto FireF = [](const STasInput &In) { return In.m_Fire != 0; };
	auto HookF = [](const STasInput &In) { return In.m_Hook != 0; };
	switch(Kind)
	{
	case 0: // nudge a shot's aim (the aim at every tick of the press)
	{
		Segments(v, L, FireF, vSeg);
		if(vSeg.empty())
			return -1;
		auto [a, b] = vSeg[Rnd(vSeg.size())];
		float d = (Rnd(2) ? 1 : -1) * (0.2f + Rnd(40) * 0.25f) * pi / 180.0f;
		for(int t = a; t < b; t++)
			if(!v[t].m_Hook || (t > 0 && v[t - 1].m_Hook)) // don't re-aim a hook launch
				SetAim(v[t], AimOf(v[t]) + d);
		return a;
	}
	case 1: // move a shot by 1..4 ticks
	{
		Segments(v, L, FireF, vSeg);
		if(vSeg.empty())
			return -1;
		auto [a, b] = vSeg[Rnd(vSeg.size())];
		int k = (1 + Rnd(4)) * (Rnd(2) ? 1 : -1);
		STasInput Aim = v[a];
		for(int t = a; t < b; t++)
			v[t].m_Fire = 0;
		int na = std::clamp(a + k, 0, L - 1), nb = std::clamp(b + k, 1, L);
		for(int t = na; t < nb; t++)
		{
			v[t].m_Fire = 1;
			if(!v[t].m_Hook || (t > 0 && v[t - 1].m_Hook))
			{
				v[t].m_TX = Aim.m_TX;
				v[t].m_TY = Aim.m_TY;
			}
		}
		return std::min(a, na);
	}
	case 2: // add a shot with a random aim
	{
		int t = Rnd(L);
		float Ang = Rnd(360) * pi / 180.0f;
		for(int j = t; j < std::min(L, t + 2); j++)
		{
			v[j].m_Fire = 1;
			if(!v[j].m_Hook || (j > 0 && v[j - 1].m_Hook))
				SetAim(v[j], Ang);
		}
		return t;
	}
	case 3: // remove a shot
	{
		Segments(v, L, FireF, vSeg);
		if(vSeg.empty())
			return -1;
		auto [a, b] = vSeg[Rnd(vSeg.size())];
		for(int t = a; t < b; t++)
			v[t].m_Fire = 0;
		return a;
	}
	case 4: // move a hook press or release by 1..3 ticks
	{
		std::vector<int> vEdges;
		for(int t = 1; t < L; t++)
			if(v[t].m_Hook != v[t - 1].m_Hook)
				vEdges.push_back(t);
		if(vEdges.empty())
			return -1;
		int t = vEdges[Rnd(vEdges.size())];
		int k = 1 + Rnd(3);
		bool Press = v[t].m_Hook;
		if(Rnd(2))
		{
			for(int j = std::max(1, t - k); j < t; j++)
			{
				v[j].m_Hook = v[t].m_Hook;
				if(Press)
				{
					v[j].m_TX = v[t].m_TX;
					v[j].m_TY = v[t].m_TY;
				}
			}
			return std::max(0, t - k);
		}
		for(int j = t; j < std::min(L, t + k); j++)
			v[j].m_Hook = v[t - 1].m_Hook;
		if(Press && t + k < L)
		{
			v[t + k].m_TX = v[t].m_TX;
			v[t + k].m_TY = v[t].m_TY;
		}
		return t;
	}
	case 5: // nudge a hook's aim
	{
		Segments(v, L, HookF, vSeg);
		if(vSeg.empty())
			return -1;
		auto [a, b] = vSeg[Rnd(vSeg.size())];
		float Ang = AimOf(v[a]) + (Rnd(2) ? 1 : -1) * (0.5f + Rnd(12)) * pi / 180.0f;
		SetAim(v[a], Ang);
		return a;
	}
	case 6: // toggle jump over 1..3 ticks
	{
		int t = Rnd(L);
		int k = 1 + Rnd(3);
		uint8_t J = !v[t].m_Jump;
		for(int j = t; j < std::min(L, t + k); j++)
			v[j].m_Jump = J;
		return t;
	}
	case 7: // direction over a short range
	{
		int t = Rnd(L);
		int k = 1 + Rnd(6);
		int8_t D = (int8_t)(Rnd(3) - 1);
		for(int j = t; j < std::min(L, t + k); j++)
			v[j].m_Dir = D;
		return t;
	}
	default: // add a hook of 2..12 ticks with a random aim (where none is held)
	{
		int t = Rnd(L);
		if(v[t].m_Hook || (t > 0 && v[t - 1].m_Hook))
			return -1;
		int k = 2 + Rnd(11);
		float Ang = Rnd(360) * pi / 180.0f;
		STasInput A;
		SetAim(A, Ang);
		for(int j = t; j < std::min(L, t + k); j++)
		{
			if(v[j].m_Hook)
				break;
			v[j].m_Hook = 1;
			if(j == t)
			{
				v[j].m_TX = A.m_TX;
				v[j].m_TY = A.m_TY;
			}
		}
		return t;
	}
	}
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: polish <map> base=FILE ...\n");
		return 1;
	}
	std::string Base, Prefix, Out = "polished.txt";
	bool Tp = false;
	float TpX = 0, TpY = 0, TpVx = 0, TpVy = 0;
	int Gren = 0, Jumped = -1, Started = 0;
	double Seconds = 60;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		const char *v = V.c_str();
		if(K == "base")
			Base = V;
		else if(K == "prefix")
			Prefix = V;
		else if(K == "tp")
			Tp = std::sscanf(v, "%f,%f,%f,%f", &TpX, &TpY, &TpVx, &TpVy) == 4;
		else if(K == "gren")
			Gren = atoi(v);
		else if(K == "jumped")
			Jumped = atoi(v);
		else if(K == "started")
			Started = atoi(v);
		else if(K == "stopx")
			gs_StopX = atof(v);
		else if(K == "stopxlt")
			gs_StopXLt = atof(v);
		else if(K == "goal")
			gs_HasGoal = std::sscanf(v, "%f,%f,%f,%f", &gs_aGoal[0], &gs_aGoal[1], &gs_aGoal[2], &gs_aGoal[3]) == 4;
		else if(K == "seconds")
			Seconds = atof(v);
		else if(K == "seed")
			gs_Seed = 88172645463325252ull ^ (uint64_t)atoll(v) * 2654435761ull;
		else if(K == "lambda")
			gs_Lambda = atof(v);
		else if(K == "out")
			Out = V;
		else
		{
			std::printf("unknown key %s\n", K.c_str());
			return 1;
		}
	}
	SEval E;
	E.m_pStart = std::make_unique<CTasGame>();
	CTasGame &S = *E.m_pStart;
	S.Spawn(CTasGame::Map().m_vSpawns[0]);
	if(!Prefix.empty())
		for(const auto &In : ReadInputs(Prefix.c_str()))
			S.Step(In);
	if(Gren)
	{
		S.Chr()->GiveWeapon(WEAPON_GRENADE);
		S.Chr()->m_Core.m_ActiveWeapon = WEAPON_GRENADE;
	}
	if(Tp)
		S.SetState(vec2(TpX, TpY), vec2(TpVx, TpVy));
	if(Jumped >= 0)
		S.Chr()->m_Core.m_Jumped = Jumped;
	if(Started && !S.m_Started)
	{
		S.m_Started = true;
		S.m_StartTick = S.m_Tick;
	}
	std::vector<STasInput> vBest = ReadInputs(Base.c_str());
	if(vBest.empty())
	{
		std::printf("no base inputs\n");
		return 1;
	}
	// pad with the last input so mutations that slow down can still reach the gate
	for(int i = 0; i < 60; i++)
		vBest.push_back(vBest.back());
	E.Checkpoints(vBest);
	SRes Best = E.Eval(vBest, 0);
	std::printf("base: ok %d ticks %d speed %.2f\n", Best.m_Ok, Best.m_Ticks, Best.m_Speed);
	if(!Best.m_Ok)
		return 1;
	auto T0 = std::chrono::steady_clock::now();
	long Evals = 0, Acc = 0;
	int aAcc[9] = {0};
	std::vector<STasInput> vTry;
	while(std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count() < Seconds)
	{
		for(int Rep = 0; Rep < 50; Rep++)
		{
			vTry = vBest;
			int L = std::min((int)vTry.size(), Best.m_Ticks);
			int Kind;
			int From = Mutate(vTry, L, Kind);
			if(From < 0)
				continue;
			// sometimes stack a second mutation
			if(Rnd(3) == 0)
			{
				int K2;
				int F2 = Mutate(vTry, L, K2);
				if(F2 >= 0)
					From = std::min(From, F2);
			}
			SRes R = E.Eval(vTry, From);
			Evals++;
			if(R.m_Ok && R.Value() < Best.Value() - 1e-4f)
			{
				Best = R;
				vBest = vTry;
				E.Checkpoints(vBest);
				Acc++;
				aAcc[Kind]++;
				double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
				std::printf("[%6.1fs %8ld evals] ticks %d speed %.2f (%s)\n", Sec, Evals, Best.m_Ticks, Best.m_Speed, gs_apKind[Kind]);
				std::fflush(stdout);
				std::vector<STasInput> vW(vBest.begin(), vBest.begin() + Best.m_Ticks);
				WriteInputs(Out, vW);
			}
		}
	}
	std::printf("done: ticks %d speed %.2f, %ld evals, %ld accepted:", Best.m_Ticks, Best.m_Speed, Evals, Acc);
	for(int k = 0; k < 9; k++)
		std::printf(" %s=%d", gs_apKind[k], aAcc[k]);
	std::printf("\n");
	std::vector<STasInput> vW(vBest.begin(), vBest.begin() + Best.m_Ticks);
	WriteInputs(Out, vW);
	return 0;
}
