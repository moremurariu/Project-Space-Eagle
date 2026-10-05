// hcx: (1+lambda) hill climbing over per-tick inputs after a cut, scored exactly by the gate crossing
// (physlab3 experiment tool, single thread).
// Genes per tick: dir, jump, hook, fire, aim (deg, y down). Aims are written with radius 10000 (fine shot aims).
// Score (lower better): fractional gate tick (x <= gatex, vx <= -gatevx, y <= gatey) - spw * |v| at the gate;
// not reached: 1e4 + distance left; freeze: 2e4 - ticks survived.
// usage: hcx <map> prefix=FILE cut=N seed=FILE [iters=200000 seconds=120 out=FILE T=110 rng=1 lam=1 gatex=7700
//        gatey=2098 gatevx=15 spw=0.01 lock=K (no mutations in the first K ticks) firemin=RT (no shots before)
//        survive=20 (gate state must survive this many ticks of simple rollouts)]
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/entities/projectile.h>
#include <game/client/prediction/gameworld.h>
#include "../../ddnet/src/tas/sim.h"
#undef private
#undef protected

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <string>
#include <vector>

struct SGene
{
	int d = 0, j = 0, h = 0, f = 0;
	float a = 0; // deg
	int tx = 0, ty = -1; // exact aim (kept from the seed until the aim is mutated)
	void SetA(float A)
	{
		a = A;
		tx = (int)std::lround(std::cos(a * pi / 180) * 10000);
		ty = (int)std::lround(std::sin(a * pi / 180) * 10000);
		if(!tx && !ty)
			ty = -1;
	}
};

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

static float g_GateX = 7700, g_GateY = 2098, g_GateY0 = 2040, g_GateVx = 15, g_Spw = 0.01f;
static int g_T = 110, g_Survive = 20, g_FireMin = -1000000;

static STasInput ToIn(const SGene &g, const STasInput &Prev)
{
	STasInput In;
	In.m_Dir = g.d;
	In.m_Jump = g.j;
	In.m_Hook = g.h;
	In.m_Fire = g.f;
	In.m_Weapon = 3;
	// aim matters for hook launches and shots; keep the previous aim otherwise (identical inputs)
	bool Use = g.f || (g.h && !Prev.m_Hook);
	if(Use)
	{
		In.m_TX = (int16_t)g.tx;
		In.m_TY = (int16_t)g.ty;
	}
	else
	{
		In.m_TX = Prev.m_TX;
		In.m_TY = Prev.m_TY;
	}
	return In;
}

static bool Survives(const CTasGame &G, int N)
{
	if(G.Frozen() || G.EnteredFreeze())
		return false;
	static vec2 aPos[256];
	N = std::min(N, 256);
	for(int d = 1; d >= -1; d--)
	{
		if(G.Rollout(N, d, false, aPos) == N)
			return true;
		if(G.m_LastHook && G.Rollout(N, d, true, aPos) == N)
			return true;
	}
	for(int a = 0; a < 16; a++)
	{
		float Ang = 2 * pi * a / 16;
		for(int d = 1; d >= -1; d--)
			if(G.RolloutHook(N, d, (int)(std::cos(Ang) * 1000), (int)(std::sin(Ang) * 1000), aPos) == N)
				return true;
	}
	return false;
}

// full-simulation survival: some simple policy (dir -1/0, no hook or a held hook down/down-left/down-right/left)
// keeps the tee out of freeze for N ticks
static bool FullSurvives(const CTasGame &G0, int N)
{
	static CTasGame S;
	const float aAng[] = {-1, 90, 135, 45, 180, 160};
	for(int d = -1; d <= 0; d++)
		for(float A : aAng)
		{
			S.CopyFrom(G0);
			STasInput In;
			In.m_Dir = d;
			In.m_Weapon = 3;
			In.m_Hook = A >= 0;
			In.m_TX = A >= 0 ? (int16_t)std::lround(std::cos(A * pi / 180) * 1000) : 0;
			In.m_TY = A >= 0 ? (int16_t)std::lround(std::sin(A * pi / 180) * 1000) : -1;
			bool Ok = true;
			for(int t = 0; t < N && Ok; t++)
			{
				S.Step(In);
				Ok = !(S.Frozen() || S.EnteredFreeze());
			}
			if(Ok)
				return true;
		}
	return false;
}

struct SRes
{
	double m_S;
	int m_Rt = -1;
	float m_Sp = 0;
	vec2 m_P, m_V;
	int m_Len = 0; // ticks used
};

static SRes Eval(const CTasGame &Start, const STasInput &Last, const std::vector<SGene> &vG, std::vector<STasInput> *pOut = nullptr)
{
	static CTasGame G;
	G.CopyFrom(Start);
	STasInput Prev = Last;
	SRes R;
	if(pOut)
		pOut->clear();
	for(int t = 0; t < (int)vG.size(); t++)
	{
		STasInput In = ToIn(vG[t], Prev);
		vec2 P0 = G.Pos();
		G.Step(In);
		if(pOut)
			pOut->push_back(In);
		Prev = In;
		if(G.Frozen() || G.EnteredFreeze())
		{
			R.m_S = 2e4 - t;
			R.m_Len = t + 1;
			return R;
		}
		vec2 P = G.Pos(), V = G.Vel();
		if(P.x <= g_GateX && V.x <= -g_GateVx && P.y <= g_GateY && P.y >= g_GateY0)
		{
			int Rt = G.m_Tick - G.m_StartTick;
			float Fr = P0.x > P.x ? (P0.x - g_GateX) / (P0.x - P.x) : 1.0f;
			Fr = std::clamp(Fr, 0.0f, 1.0f);
			R.m_Rt = Rt;
			R.m_Sp = length(V);
			R.m_P = P;
			R.m_V = V;
			R.m_Len = t + 1;
			R.m_S = (Rt - 1) + Fr - g_Spw * R.m_Sp;
			if(g_Survive > 0 && !FullSurvives(G, g_Survive))
				R.m_S += 50;
			return R;
		}
	}
	R.m_S = 1e4 + std::max(0.0f, G.Pos().x - g_GateX) / 30.0f + std::max(0.0f, G.Pos().y - g_GateY) / 30.0f;
	R.m_Len = (int)vG.size();
	return R;
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
		return 1;
	std::string Prefix, Seed, Out = "hcx_out.txt";
	int Cut = 0, Lock = 0, Lam = 1;
	long long Iters = 200000;
	double Seconds = 120;
	unsigned Rng = 1;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "prefix") Prefix = V;
		else if(K == "seed") Seed = V;
		else if(K == "out") Out = V;
		else if(K == "cut") Cut = std::stoi(V);
		else if(K == "lock") Lock = std::stoi(V);
		else if(K == "lam") Lam = std::stoi(V);
		else if(K == "iters") Iters = std::stoll(V);
		else if(K == "seconds") Seconds = std::stod(V);
		else if(K == "rng") Rng = (unsigned)std::stoul(V);
		else if(K == "T") g_T = std::stoi(V);
		else if(K == "gatex") g_GateX = std::stof(V);
		else if(K == "gatey") g_GateY = std::stof(V);
		else if(K == "gatey0") g_GateY0 = std::stof(V);
		else if(K == "gatevx") g_GateVx = std::stof(V);
		else if(K == "spw") g_Spw = std::stof(V);
		else if(K == "survive") g_Survive = std::stoi(V);
		else if(K == "firemin") g_FireMin = std::stoi(V);
		else
			std::printf("unknown key %s\n", K.c_str());
	}
	auto vPre = ReadInputs(Prefix.c_str());
	auto vSeed = ReadInputs(Seed.c_str());
	CTasGame Start;
	Start.Spawn(CTasGame::Map().m_vSpawns[0]);
	for(int i = 0; i < Cut; i++)
		Start.Step(vPre[i]);
	const int Rt0 = Start.m_Tick - Start.m_StartTick;
	STasInput Last = vPre[Cut - 1];
	std::vector<SGene> vG(g_T);
	for(int t = 0; t < g_T; t++)
	{
		SGene g;
		if(Cut + t < (int)vSeed.size())
		{
			const STasInput &In = vSeed[Cut + t];
			g.d = In.m_Dir;
			g.j = In.m_Jump;
			g.h = In.m_Hook;
			g.f = In.m_Fire;
			g.a = std::atan2((float)In.m_TY, (float)In.m_TX) * 180 / pi;
			g.tx = In.m_TX;
			g.ty = In.m_TY;
		}
		else
		{
			g.d = -1;
			g.SetA(90);
		}
		vG[t] = g;
	}
	// shots before firemin are not allowed (removed from the seed too)
	for(int t = 0; t < g_T; t++)
		if(Rt0 + 1 + t < g_FireMin)
			vG[t].f = 0;
	SRes Best = Eval(Start, Last, vG);
	std::printf("start rt %d; seed score %.3f (gate rt %d |v| %.2f)\n", Rt0, Best.m_S, Best.m_Rt, Best.m_Sp);
	std::mt19937 R(Rng);
	auto U = [&](int a, int b) { return std::uniform_int_distribution<int>(a, b)(R); };
	auto F = [&]() { return std::uniform_real_distribution<float>(0, 1)(R); };
	auto N01 = [&]() { return std::normal_distribution<float>(0, 1)(R); };
	auto T0 = std::chrono::steady_clock::now();
	long long It = 0, Acc = 0;
	double LastPrint = 0;
	while(It < Iters)
	{
		double El = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
		if(El > Seconds)
			break;
		if(El - LastPrint > 10)
		{
			LastPrint = El;
			std::printf("  %.0fs it %lld acc %lld best %.3f (rt %d |v| %.2f)\n", El, It, Acc, Best.m_S, Best.m_Rt, Best.m_Sp);
			std::fflush(stdout);
		}
		std::vector<SGene> vBestChild;
		SRes BestChild;
		BestChild.m_S = 1e30;
		for(int l = 0; l < Lam; l++)
		{
			It++;
			std::vector<SGene> v = vG;
			const int Len = std::min(g_T, std::max(Best.m_Len + 2, Lock + 4));
			int nm = 1 + (F() < 0.3f ? U(1, 3) : 0);
			for(int m = 0; m < nm; m++)
			{
				int i = U(Lock, Len - 1);
				float k = F();
				if(k < 0.16f)
				{ // dir segment
					int L = U(1, 8), d = U(-1, 1);
					for(int t = i; t < std::min(Len, i + L); t++)
						v[t].d = d;
				}
				else if(k < 0.34f)
				{ // hook segment with a new aim (random or perturbed)
					int L = U(1, 14);
					float a = F() < 0.5f ? F() * 360 : v[i].a + N01() * 20;
					if(i > 0)
						v[i - 1].h = 0;
					for(int t = i; t < std::min(Len, i + L); t++)
					{
						v[t].h = 1;
						v[t].SetA(a);
					}
					if(i + L < g_T)
						v[i + L].h = 0;
				}
				else if(k < 0.44f)
				{ // hook off / extend
					int L = U(1, 6);
					int val = U(0, 1);
					for(int t = i; t < std::min(Len, i + L); t++)
						v[t].h = val;
				}
				else if(k < 0.60f)
				{ // aim perturbation of a launch or shot tick near i
					int best = -1;
					for(int dd = 0; dd < 12 && best < 0; dd++)
						for(int s : {i + dd, i - dd})
							if(s >= Lock && s < Len && (v[s].f || (v[s].h && (s == 0 || !v[s - 1].h))))
							{
								best = s;
								break;
							}
					if(best >= 0)
					{
						float sc = std::pow(10.0f, -1.5f + 2.5f * F()); // 0.03 .. 10 deg
						float da = N01() * sc;
						v[best].SetA(v[best].a + da);
						// a hook launch: the held ticks share the aim (irrelevant), keep consistent
						for(int t = best + 1; t < Len && v[t].h && !v[best].f; t++)
							v[t].SetA(v[best].a);
					}
				}
				else if(k < 0.72f)
				{ // shots: add / move / remove
					std::vector<int> vF;
					for(int t = Lock; t < Len; t++)
						if(v[t].f)
							vF.push_back(t);
					float q = F();
					if(q < 0.4f || vF.empty())
					{
						if(Rt0 + 1 + i >= g_FireMin)
						{
							v[i].f = 1;
							v[i].SetA(F() * 360);
						}
					}
					else if(q < 0.8f)
					{
						int s = vF[U(0, (int)vF.size() - 1)];
						int ns = std::clamp(s + U(-3, 3), Lock, Len - 1);
						if(Rt0 + 1 + ns >= g_FireMin)
						{
							SGene gs = v[s];
							v[s].f = 0;
							v[ns].f = 1;
							v[ns].a = gs.a;
							v[ns].tx = gs.tx;
							v[ns].ty = gs.ty;
						}
					}
					else
						v[vF[U(0, (int)vF.size() - 1)]].f = 0;
				}
				else if(k < 0.80f)
				{ // jump toggle
					v[i].j = 1 - v[i].j;
				}
				else if(k < 0.88f)
				{ // shift a block by one tick
					int L = U(2, 20);
					int e = std::min(Len - 1, i + L);
					if(F() < 0.5f && i > Lock)
					{
						for(int t = i - 1; t < e; t++)
							v[t] = v[t + 1];
					}
					else
					{
						for(int t = e; t > i; t--)
							v[t] = v[t - 1];
					}
				}
				else
				{ // copy-forward: re-randomise dirs of a segment
					int L = U(1, 4);
					for(int t = i; t < std::min(Len, i + L); t++)
						v[t].d = U(-1, 1);
				}
			}
			SRes Rr = Eval(Start, Last, v);
			if(Rr.m_S < BestChild.m_S)
			{
				BestChild = Rr;
				vBestChild = v;
			}
		}
		if(BestChild.m_S <= Best.m_S)
		{
			if(BestChild.m_S < Best.m_S - 1e-6)
				Acc++;
			Best = BestChild;
			vG = vBestChild;
		}
	}
	std::vector<STasInput> vIn;
	Best = Eval(Start, Last, vG, &vIn);
	FILE *f = std::fopen(Out.c_str(), "w");
	for(int i = 0; i < Cut; i++)
	{
		const STasInput &In = vPre[i];
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	}
	for(int t = 0; t < Best.m_Len; t++)
	{
		const STasInput &In = vIn[t];
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	}
	std::fclose(f);
	std::printf("FINAL score %.3f gate rt %d |v| %.2f pos %.0f %.0f vel %.2f %.2f (%lld it, %lld acc) -> %s\n", Best.m_S, Best.m_Rt, Best.m_Sp, Best.m_P.x,
		Best.m_P.y, Best.m_V.x, Best.m_V.y, It, Acc, Out.c_str());
	return 0;
}
