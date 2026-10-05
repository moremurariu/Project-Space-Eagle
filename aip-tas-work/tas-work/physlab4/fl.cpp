// fl: physlab4 final-maze lab (trace / distance-field / timing).
// usage: fl <map> <mode> key=value...
//   entry: tp=x,y,vx,vy tpk=K reload=N air=0|1 prefix=FILE (default e1025 + Teero-like k2390 state)
//   trace in=FILE      FILE lines: "d j h f deg [n]" (aim in degrees, y down) or "d j h f tx ty w" (raw input line)
//                      prints the state after every input
//   df                 geodesic distance to the finish along Teero's track
//   bench              time Step / CopyFrom
#include "flcommon.h"

#include <sstream>

static std::vector<STasInput> ReadScript(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	if(!f)
	{
		std::printf("cannot read %s\n", pPath);
		std::exit(1);
	}
	char aLine[512];
	while(std::fgets(aLine, sizeof(aLine), f))
	{
		if(aLine[0] == '#')
			continue;
		std::istringstream L(aLine);
		std::vector<double> a;
		double x;
		while(L >> x)
			a.push_back(x);
		if(a.size() == 7)
		{
			STasInput In;
			In.m_Dir = (int)a[0];
			In.m_Jump = (int)a[1];
			In.m_Hook = (int)a[2];
			In.m_Fire = (int)a[3];
			In.m_TX = (int)a[4];
			In.m_TY = (int)a[5];
			In.m_Weapon = (int)a[6];
			v.push_back(In);
		}
		else if(a.size() == 5 || a.size() == 6)
		{
			int n = a.size() == 6 ? (int)a[5] : 1;
			for(int i = 0; i < n; i++)
				v.push_back(MkIn((int)a[0], (int)a[1], (int)a[2], (int)a[3], (float)a[4]));
		}
	}
	std::fclose(f);
	return v;
}

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: fl <map> trace|df|bench key=value...\n");
		return 1;
	}
	std::string Mode = argv[2];
	SEntry E;
	std::string In;
	for(int i = 3; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		std::string K = A.substr(0, Eq), V = Eq == std::string::npos ? "" : A.substr(Eq + 1);
		if(EntryArg(E, K, V))
			continue;
		if(K == "in")
			In = V;
	}
	if(Mode == "trace")
	{
		CTasGame G;
		MakeEntry(G, E);
		PrintState(G);
		std::vector<STasInput> v = ReadScript(In.c_str());
		std::vector<std::pair<int, vec2>> vE;
		for(const auto &I : v)
		{
			int NP = G.NumProjectiles();
			G.Step(I);
			PrintState(G, &I);
			if(G.NumProjectiles() > NP || (I.m_Fire && G.NumProjectiles() > 0))
			{
				Explosions(G, vE);
				for(auto &e : vE)
					std::printf("   pending explosion k %d at %.1f %.1f\n", e.first, e.second.x, e.second.y);
			}
		}
	}
	else if(Mode == "df")
	{
		BuildDF();
		FILE *f = std::fopen("../teero_track.txt", "r");
		char aLine[256];
		float PrevD = -1;
		while(std::fgets(aLine, sizeof(aLine), f))
		{
			int k;
			float x, y;
			if(std::sscanf(aLine, "%d %f %f", &k, &x, &y) == 3 && k >= (getenv("K0") ? atoi(getenv("K0")) : 2370))
			{
				float D = gs_DF.Sample(vec2(x, y));
				std::printf("k %d pos %.0f %.0f D %.0f dD %.1f\n", k, x, y, D, PrevD >= 0 ? PrevD - D : 0.0f);
				PrevD = D;
			}
		}
		std::fclose(f);
	}
	else if(Mode == "scanfire")
	{
		// scanfire in=FILE at=I len=L [step=0.5]: replay FILE's first I inputs, then fire at every aim on input I (keeping
		// its dir/jump/hook), continue with FILE's inputs I+1..I+L without shots; report explosion and final state
		int At = std::atoi(getenv("AT") ? getenv("AT") : "0"), Len = std::atoi(getenv("LEN") ? getenv("LEN") : "8");
		float StepDeg = std::atof(getenv("STEP") ? getenv("STEP") : "1");
		std::vector<STasInput> v = ReadScript(In.c_str());
		CTasGame G0, G;
		MakeEntry(G0, E);
		for(int i = 0; i < At; i++)
			G0.Step(v[i]);
		std::printf("base: ");
		PrintState(G0);
		{
			G.CopyFrom(G0);
			for(int i = At; i <= At + Len && i < (int)v.size(); i++)
			{
				STasInput I = v[i];
				I.m_Fire = 0;
				G.Step(I);
			}
			std::printf("nofire: ");
			PrintState(G);
		}
		for(float a = -180; a < 180; a += StepDeg)
		{
			G.CopyFrom(G0);
			STasInput F = v[At];
			STasInput A = MkIn(F.m_Dir, F.m_Jump, F.m_Hook, 1, a);
			if(F.m_Hook && G0.m_LastHook)
			{
				A.m_TX = F.m_TX;
				A.m_TY = F.m_TY;
				A.m_TX = (int16_t)std::lround(std::cos(a * pi / 180) * 1000);
				A.m_TY = (int16_t)std::lround(std::sin(a * pi / 180) * 1000);
			}
			G.Step(A);
			std::vector<std::pair<int, vec2>> vE;
			Explosions(G, vE);
			int Ek = vE.empty() ? -1 : vE[0].first;
			vec2 Ep = vE.empty() ? vec2(0, 0) : vE[0].second;
			bool Dd = Dead(G);
			for(int i = At + 1; i <= At + Len && i < (int)v.size() && !Dd; i++)
			{
				STasInput I = v[i];
				I.m_Fire = 0;
				G.Step(I);
				Dd = Dead(G);
			}
			if(Dd)
				continue;
			std::printf("aim %7.2f expl k %d at %.0f %.0f | ", a, Ek, Ep.x, Ep.y);
			PrintState(G);
		}
	}
	else if(Mode == "bench")
	{
		CTasGame G, T;
		MakeEntry(G, E);
		STasInput I = MkIn(1, 0, 0, 0, 0);
		auto t0 = std::chrono::steady_clock::now();
		const int N = 200000;
		for(int i = 0; i < N; i++)
			T.CopyFrom(G);
		auto t1 = std::chrono::steady_clock::now();
		for(int i = 0; i < N; i++)
		{
			T.CopyFrom(G);
			T.Step(I);
		}
		auto t2 = std::chrono::steady_clock::now();
		std::printf("copy %.3f us, copy+step %.3f us\n", std::chrono::duration<double>(t1 - t0).count() / N * 1e6, std::chrono::duration<double>(t2 - t1).count() / N * 1e6);
	}
	return 0;
}
