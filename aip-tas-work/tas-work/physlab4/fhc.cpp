// fhc: physlab4 local search (hill climbing) on a post-entry input sequence with the exact simulator.
// usage: fhc <map> in=FILE out=FILE [entry keys] gate=finish|x>X|y<Y... iters=200000 seed=1 lo=0 hi=N
//   Objective: first tick at the gate (finish: the official finish tick) + a sub-tick term (how far past the gate
//   line the tee is at that tick: earlier crossing = smaller), dead / never = huge. Mutations (positions in
//   [lo, hi)): dir runs, jump toggles, hook press/release shifts, hook / fire aim nudges, fire shifts / add / remove.
//   Accepts equal or better ((1+1) hill climbing with plateau moves).
#include "flcommon.h"

#include <random>

static std::string gs_Gate = "finish";
static float gs_GateV = 0;
static float gs_VW = 0; // objective credit per px/t of velocity along the gate normal

static int gs_GateIdx = 1 << 30; // input index at which the last evaluated sequence reached the gate
static double Objective(const CTasGame &Start, const std::vector<STasInput> &v, int From, const CTasGame *pFromState, std::vector<std::unique_ptr<CTasGame>> *pCk, int CkEvery)
{
	gs_GateIdx = 1 << 30;
	static CTasGame G;
	G.CopyFrom(pFromState ? *pFromState : Start);
	int i0 = pFromState ? From : 0;
	for(int i = i0; i < (int)v.size(); i++)
	{
		if(pCk && i % CkEvery == 0)
			(*pCk)[i / CkEvery]->CopyFrom(G);
		vec2 Prev = G.Pos();
		G.Step(v[i]);
		if(Dead(G))
			return 1e9 - i;
		vec2 P = G.Pos();
		bool At = false;
		double Sub = 0;
		if(gs_Gate == "finish")
		{
			if(G.m_FinishTick >= 0)
			{
				At = true;
				// finish is detected one tick after the crossing move; sub-tick: how far past x=8822.67 the tee was
				Sub = -std::min(1.0, std::max(0.0, (double)(P.x - 8822.67f) / 40.0));
				gs_GateIdx = i;
				return (G.m_FinishTick - G.m_StartTick) + 0.5 * Sub;
			}
		}
		else if(gs_Gate == "col" || gs_Gate == "under" || gs_Gate == "gap" || gs_Gate == "room")
		{
			vec2 Nrm;
			float Line;
			if(gs_Gate == "col")
				At = P.y < 3900 && P.x > 8470 && P.x < 8580, Nrm = vec2(0, -1), Line = -3900;
			else if(gs_Gate == "under")
				At = P.x < 9050 && P.x > 8700 && P.y > 3870 && P.y < 4070, Nrm = vec2(-1, 0), Line = -9050;
			else if(gs_Gate == "gap")
				At = P.y > 4352 && P.x > 9200, Nrm = vec2(0, 1), Line = 4352;
			else
				At = P.x < 9000 && P.y > 4410, Nrm = vec2(-1, 0), Line = -9000;
			if(At)
			{
				float d = std::fabs(dot(P - Prev, Nrm));
				Sub = d > 1e-3f ? (dot(P, Nrm) - Line) / d : 0;
				gs_GateIdx = i;
				return Rt(G) - std::clamp(Sub, 0.0, 1.0) - gs_VW * dot(G.Vel(), Nrm);
			}
		}
		else
		{
			char c = gs_Gate[0], op = gs_Gate[1];
			float q = c == 'x' ? P.x : P.y, qp = c == 'x' ? Prev.x : Prev.y;
			At = op == '>' ? q > gs_GateV : q < gs_GateV;
			if(At)
			{
				float d = std::fabs(q - qp);
				Sub = d > 1e-3f ? std::fabs(q - gs_GateV) / d : 0;
				gs_GateIdx = i;
				vec2 Nrm = c == 'x' ? vec2(op == '>' ? 1 : -1, 0) : vec2(0, op == '>' ? 1 : -1);
				return Rt(G) - std::min(1.0, Sub) - gs_VW * dot(G.Vel(), Nrm);
			}
		}
	}
	// not reached: penalize by remaining distance
	return 1e6 + gs_DF.Sample(G.Pos());
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: fhc <map> in=FILE out=FILE ...\n");
		return 1;
	}
	SEntry E;
	std::string In, Out = "results/hc.txt";
	long Iters = 200000;
	int Seed = 1, Lo = 0, Hi = 1 << 30, Extend = 0;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(EntryArg(E, K, V))
			continue;
		if(K == "in") In = V;
		else if(K == "out") Out = V;
		else if(K == "gate") gs_Gate = V;
		else if(K == "iters") Iters = std::stol(V);
		else if(K == "seed") Seed = std::stoi(V);
		else if(K == "lo") Lo = std::stoi(V);
		else if(K == "hi") Hi = std::stoi(V);
		else if(K == "extend") Extend = std::stoi(V);
		else if(K == "vw") gs_VW = std::stof(V);
	}
	if(gs_Gate[0] == 'x' || gs_Gate[0] == 'y')
		gs_GateV = std::atof(gs_Gate.c_str() + 2);
	BuildDF();
	std::vector<STasInput> v = ReadInputs(In.c_str());
	for(int k = 0; k < Extend; k++)
		v.push_back(v.empty() ? MkIn(1, 0, 0, 0, 0) : v.back());
	for(auto &I : v)
		I.m_Weapon = 3;
	CTasGame Start;
	MakeEntry(Start, E);
	const int N = v.size();
	Hi = std::min(Hi, N);
	const int CkEvery = 4;
	std::vector<std::unique_ptr<CTasGame>> vCk(N / CkEvery + 2);
	for(auto &p : vCk)
		p = std::make_unique<CTasGame>();
	double Best = Objective(Start, v, 0, nullptr, &vCk, CkEvery);
	std::printf("start objective %.3f (%d inputs, gate at input %d)\n", Best, N, gs_GateIdx);
	const int HiUser = Hi;
	Hi = std::min(HiUser, gs_GateIdx + 1);
	std::mt19937 Rng(Seed);
	auto RI = [&](int a, int b) { return std::uniform_int_distribution<int>(a, b)(Rng); };
	auto RF = [&](float a, float b) { return std::uniform_real_distribution<float>(a, b)(Rng); };
	auto Aim = [](const STasInput &I) { return std::atan2((float)I.m_TY, (float)I.m_TX) * 180 / pi; };
	auto SetAim = [](STasInput &I, float Deg) {
		I.m_TX = (int16_t)std::lround(std::cos(Deg * pi / 180) * 1000);
		I.m_TY = (int16_t)std::lround(std::sin(Deg * pi / 180) * 1000);
		if(!I.m_TX && !I.m_TY)
			I.m_TY = -1;
	};
	long Acc = 0, Imp = 0;
	auto t0 = std::chrono::steady_clock::now();
	std::vector<STasInput> w;
	for(long It = 0; It < Iters; It++)
	{
		w = v;
		int First = N;
		int Kind = RI(0, 9);
		int i = RI(Lo, Hi - 1);
		switch(Kind)
		{
		case 0:
		case 1:
		{
			// dir run
			int L = RI(1, 4), d = RI(-1, 1);
			for(int k = i; k < std::min(Hi, i + L); k++)
				w[k].m_Dir = d;
			First = i;
			break;
		}
		case 2:
			w[i].m_Jump ^= 1;
			First = i;
			break;
		case 3:
		{
			// shift a hook press or release boundary by +-1..2
			int j = i;
			while(j < Hi - 1 && w[j].m_Hook == w[j + 1].m_Hook)
				j++;
			if(j >= Hi - 1)
				continue;
			int s = RI(0, 1) ? 1 : -1;
			if(s > 0)
				w[j + 1].m_Hook = w[j].m_Hook; // boundary later
			else
				w[j].m_Hook = w[j + 1].m_Hook; // boundary earlier
			// a press needs the aim of the press tick
			for(int k = std::max(0, j - 1); k <= std::min(N - 1, j + 2); k++)
				if(w[k].m_Hook && (k == 0 || !w[k - 1].m_Hook))
				{
					int src = k == j ? j + 1 : j;
					if(src < N && (w[src].m_TX != w[k].m_TX || w[src].m_TY != w[k].m_TY) && !w[k].m_Fire)
						w[k].m_TX = v[std::min(N - 1, j + 1)].m_TX, w[k].m_TY = v[std::min(N - 1, j + 1)].m_TY;
				}
			First = std::max(0, j - 1);
			break;
		}
		case 4:
		case 5:
		{
			// nudge the aim of a hook press or a shot near i
			int j = i;
			while(j < Hi && !((w[j].m_Hook && (j == 0 || !w[j - 1].m_Hook)) || w[j].m_Fire))
				j++;
			if(j >= Hi)
				continue;
			float da = RI(0, 3) == 0 ? RF(-15, 15) : RF(-2, 2);
			float a = Aim(w[j]) + da;
			// keep the aim on the following held ticks the same (only matters for presses / shots)
			int k = j;
			int16_t TX0 = w[j].m_TX, TY0 = w[j].m_TY;
			do
			{
				SetAim(w[k], a);
				k++;
			} while(k < N && w[k].m_TX == TX0 && w[k].m_TY == TY0 && !w[k].m_Fire && !(w[k].m_Hook && !w[k - 1].m_Hook));
			First = j;
			break;
		}
		case 6:
		{
			// move a shot by +-1..3 ticks
			int j = i;
			while(j < Hi && !w[j].m_Fire)
				j++;
			if(j >= Hi)
				continue;
			int s = RI(-3, 3);
			int t = std::clamp(j + s, Lo, Hi - 1);
			if(t == j)
				continue;
			STasInput F = w[j];
			w[j].m_Fire = 0;
			w[t].m_Fire = 1;
			if(!w[t].m_Hook || (t > 0 && w[t - 1].m_Hook))
			{
				w[t].m_TX = F.m_TX;
				w[t].m_TY = F.m_TY;
			}
			First = std::min(j, t);
			break;
		}
		case 7:
		{
			// add or remove a shot
			if(w[i].m_Fire)
				w[i].m_Fire = 0;
			else
			{
				w[i].m_Fire = 1;
				if(!w[i].m_Hook || (i > 0 && w[i - 1].m_Hook))
					SetAim(w[i], RF(-180, 180));
			}
			First = i;
			break;
		}
		case 8:
		{
			// hook press with a new aim at i (or release)
			if(w[i].m_Hook)
			{
				int L = RI(1, 3);
				for(int k = i; k < std::min(Hi, i + L); k++)
					w[k].m_Hook = 0;
			}
			else
			{
				float a = RF(-180, 180);
				int L = RI(2, 12);
				for(int k = i; k < std::min(Hi, i + L); k++)
				{
					w[k].m_Hook = 1;
					if(!w[k].m_Fire)
						SetAim(w[k], a);
				}
			}
			First = i;
			break;
		}
		default:
		{
			// dir change single tick
			w[i].m_Dir = RI(-1, 1);
			First = i;
			break;
		}
		}
		int Ck = First / CkEvery;
		double O = Objective(Start, w, Ck * CkEvery, vCk[Ck].get(), nullptr, CkEvery);
		if(O <= Best)
		{
			if(O < Best - 1e-9)
			{
				Imp++;
				double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
				std::printf("it %ld obj %.3f (kind %d at %d) %.0fs\n", It, O, Kind, First, Sec);
				std::fflush(stdout);
			}
			Best = O;
			v = w;
			Acc++;
			// refresh checkpoints from the changed point
			Objective(Start, v, Ck * CkEvery, vCk[Ck].get(), &vCk, CkEvery);
			Hi = std::max(Lo + 1, std::min(HiUser, gs_GateIdx + 1));
			// the checkpoint at Ck itself was overwritten with the same state: fine
		}
	}
	WriteInputs(Out.c_str(), v);
	double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
	std::printf("final objective %.3f accepted %ld improved %ld (%.0fs) -> %s\n", Best, Acc, Imp, Sec, Out.c_str());
	return 0;
}
