// Physics lab: run a small script of teleports and inputs and print the tee's state every tick.
// usage: lab <map> <script>   (script "-" = stdin)
// script lines:
//   tp x y vx vy          teleport (pixels, px/tick)
//   gren                  give the grenade and switch to it
//   jumped n              set the jump state (0 = both jumps, 2|1 bits as in the core)
//   in dir jump hook fire tx ty [n]   hold this input for n ticks (default 1)
//   quiet / loud          stop / start printing every tick
//   print                 print the state now
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "distfield.h"
#include "sim.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

static SDistField gs_DF;
static int gs_PadA0 = 0, gs_PadA1 = 360, gs_PadStep = 10;
static int gs_ReleaseAfter = 0; // prefire: release the hook and hold right after the explosion
static int gs_KeepFireFrom = -1; // prefire: keep the file's shots from this tick on

static void Print(const CTasGame &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	std::printf("rt=%d t=%d pos %.2f %.2f (tile %.2f %.2f) vel %.3f %.3f |v| %.3f hook %d jumped %d grounded %d reload %d proj %d frz %d\n", G.m_Started ? G.m_Tick - G.m_StartTick : -1, G.m_Tick, P.x, P.y, P.x / 32, P.y / 32,
		V.x, V.y, length(V), G.HookState(), G.Jumped(), G.Grounded(), G.ReloadTimer(), G.NumProjectiles(), G.Frozen() || G.EnteredFreeze());
}

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: lab <map> <script|->\n");
		return 1;
	}
	std::string Script;
	if(std::strcmp(argv[2], "-") == 0)
	{
		std::stringstream ss;
		ss << std::cin.rdbuf();
		Script = ss.str();
	}
	else
		Script = argv[2];
	for(char &c : Script)
		if(c == ';')
			c = '\n';
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	bool Loud = true;
	std::istringstream Lines(Script);
	std::string Line;
	while(std::getline(Lines, Line))
	{
		std::istringstream L(Line);
		std::string Cmd;
		if(!(L >> Cmd))
			continue;
		if(Cmd == "tp")
		{
			float x, y, vx, vy;
			L >> x >> y >> vx >> vy;
			G.SetState(vec2(x, y), vec2(vx, vy));
		}
		else if(Cmd == "gren")
		{
			CCharacter *pChr = G.Chr();
			pChr->GiveWeapon(WEAPON_GRENADE);
			pChr->m_Core.m_ActiveWeapon = WEAPON_GRENADE;
		}
		else if(Cmd == "jumped")
		{
			int j;
			L >> j;
			G.Chr()->m_Core.m_Jumped = j;
		}
		else if(Cmd == "replay")
		{
			std::string Path;
			L >> Path;
			FILE *f = std::fopen(Path.c_str(), "r");
			int d, j, h, fi, tx, ty, w;
			while(f && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
			{
				STasInput In;
				In.m_Dir = d;
				In.m_Jump = j;
				In.m_Hook = h;
				In.m_Fire = fi;
				In.m_TX = tx;
				In.m_TY = ty;
				In.m_Weapon = w;
				G.Step(In);
				if(Loud)
				{
					std::printf("in %d %d %d %d %d %d | ", d, j, h, fi, tx, ty);
					Print(G);
				}
			}
			if(f)
				std::fclose(f);
		}
		else if(Cmd == "scanfire")
		{
			// scanfire N K dir: fire at N evenly spaced aims, hold dir for K ticks, report the velocity change
			int N, K, Dir;
			L >> N >> K >> Dir;
			vec2 V0 = G.Vel();
			CTasGame T;
			for(int a = 0; a < N; a++)
			{
				float Ang = 2 * pi * a / N;
				STasInput In;
				In.m_Dir = Dir;
				In.m_Fire = 1;
				In.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
				In.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
				In.m_Weapon = WEAPON_GRENADE;
				T.CopyFrom(G);
				T.Step(In);
				In.m_Fire = 0;
				for(int k = 1; k < K; k++)
					T.Step(In);
				vec2 V = T.Vel();
				std::printf("aim %6.1f deg (%5d %5d): vel %7.2f %7.2f |v| %6.2f  dv %6.2f %6.2f  frz %d\n", Ang * 180 / pi, In.m_TX, In.m_TY, V.x, V.y, length(V), V.x - V0.x, V.y - V0.y - 0.5f * K, T.Frozen() || T.EnteredFreeze());
			}
		}
		else if(Cmd == "prefire")
		{
			// prefire FILE t1lo t1hi tcheck: shots stripped from FILE, then one shot at t1 with every aim (1 deg); report the best vx at tcheck
			std::string Path;
			int T1lo, T1hi, TC;
			L >> Path >> T1lo >> T1hi >> TC;
			std::vector<STasInput> v;
			FILE *f = std::fopen(Path.c_str(), "r");
			int d, j, h, fi, tx, ty, w;
			while(f && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
			{
				STasInput In;
				In.m_Dir = d;
				In.m_Jump = j;
				In.m_Hook = h;
				In.m_Fire = (gs_KeepFireFrom >= 0 && (int)v.size() >= gs_KeepFireFrom) ? fi : 0;
				In.m_TX = tx;
				In.m_TY = ty;
				In.m_Weapon = w;
				v.push_back(In);
			}
			if(f)
				std::fclose(f);
			struct SR
			{
				float m_Vx, m_V;
				int m_T1, m_A;
				vec2 m_P;
			};
			std::vector<SR> vR;
			CTasGame T;
			for(int t1 = T1lo; t1 <= T1hi; t1++)
				for(int a = 0; a < 360; a++)
				{
					T.CopyFrom(G);
					bool Dead = false;
					bool Exploded = false;
					for(int k = 0; k < TC && k < (int)v.size(); k++)
					{
						STasInput In = v[k];
						if(Exploded && gs_ReleaseAfter)
						{
							In.m_Hook = 0;
							In.m_Dir = 1;
						}
						if(k == t1 || k == t1 + 1)
						{
							In.m_Fire = 1;
							if(!In.m_Hook || (k > 0 && v[k - 1].m_Hook))
							{
								In.m_TX = (int16_t)std::lround(std::cos(a * pi / 180) * 1000);
								In.m_TY = (int16_t)std::lround(std::sin(a * pi / 180) * 1000);
							}
						}
						int Before = T.NumProjectiles();
						T.Step(In);
						if(k > t1 && Before > 0 && T.NumProjectiles() == 0)
							Exploded = true;
						if(T.Frozen() || T.EnteredFreeze())
						{
							Dead = true;
							break;
						}
					}
					if(!Dead)
						vR.push_back({T.Vel().x, length(T.Vel()), t1, a, T.Pos()});
				}
			std::sort(vR.begin(), vR.end(), [](const SR &a, const SR &b) { return a.m_Vx > b.m_Vx; });
			for(int k = 0; k < 12 && k < (int)vR.size(); k++)
				std::printf("t1 %d aim %d: vx %.2f |v| %.2f pos %.1f %.1f\n", vR[k].m_T1, vR[k].m_A, vR[k].m_Vx, vR[k].m_V, vR[k].m_P.x / 32, vR[k].m_P.y / 32);
		}
		else if(Cmd == "swing")
		{
			// swing K: from the current state, try jump tick j, hook start s, aim a (1 deg), hold len, dir during hook;
			// afterwards release, hold left (-1), no hook. Score after K ticks: E = v^2 + (y - y0) (rising costs);
			// requires alive and x moved left. Prints the best 15.
			int K;
			L >> K;
			const vec2 P0 = G.Pos();
			struct SR
			{
				float m_E, m_V;
				int m_J, m_S, m_A, m_Len, m_Dir;
				vec2 m_P;
			};
			std::vector<SR> vR;
			CTasGame T;
			for(int j = 0; j <= 8; j++)
				for(int st = 0; st <= 8; st++)
					for(int a = 180; a < 360; a++)
						for(int Len = 3; Len <= 20; Len += 1)
							for(int Dir = -1; Dir <= 0; Dir++)
							{
								T.CopyFrom(G);
								bool Dead = false;
								for(int k = 0; k < K; k++)
								{
									STasInput In;
									In.m_Dir = (k >= st && k < st + Len) ? Dir : -1;
									In.m_Jump = k == j;
									In.m_Hook = k >= st && k < st + Len;
									In.m_TX = (int16_t)std::lround(std::cos(a * pi / 180) * 1000);
									In.m_TY = (int16_t)std::lround(std::sin(a * pi / 180) * 1000);
									T.Step(In);
									if(T.Frozen() || T.EnteredFreeze())
									{
										Dead = true;
										break;
									}
								}
								if(Dead || T.Pos().x > P0.x)
									continue;
								vec2 V = T.Vel();
								vR.push_back({dot(V, V) + (T.Pos().y - P0.y), length(V), j, st, a, Len, Dir, T.Pos()});
							}
			std::sort(vR.begin(), vR.end(), [](const SR &x, const SR &y) { return x.m_E > y.m_E; });
			vec2 V0 = G.Vel();
			std::printf("start E %.0f (|v| %.2f), %zu alive\n", dot(V0, V0), length(V0), vR.size());
			for(int k = 0; k < 15 && k < (int)vR.size(); k++)
				std::printf("E %.0f |v| %.2f jump %d hook@%d aim %d len %d dir %d -> pos %.1f %.1f\n", vR[k].m_E, vR[k].m_V, vR[k].m_J, vR[k].m_S, vR[k].m_A, vR[k].m_Len, vR[k].m_Dir, vR[k].m_P.x / 32, vR[k].m_P.y / 32);
		}
		else if(Cmd == "padrange")
			L >> gs_PadA0 >> gs_PadA1 >> gs_PadStep;
		else if(Cmd == "releaseafter")
			L >> gs_ReleaseAfter;
		else if(Cmd == "padscan")
		{
			// padscan FILE t1lo t1hi x0 y0 x1 y1 te0 te1: shots from the FILE's path at t1 whose explosion lands in the box (px) at
			// input index te0..te1; prints t1 aim TE E
			std::string Path;
			int T1lo, T1hi, TE0, TE1;
			float Bx0, By0, Bx1, By1;
			L >> Path >> T1lo >> T1hi >> Bx0 >> By0 >> Bx1 >> By1 >> TE0 >> TE1;
			std::vector<STasInput> v;
			FILE *f = std::fopen(Path.c_str(), "r");
			int d, j, h, fi, tx, ty, w;
			while(f && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
			{
				STasInput In;
				In.m_Dir = d;
				In.m_Jump = j;
				In.m_Hook = h;
				In.m_Fire = 0;
				In.m_TX = tx;
				In.m_TY = ty;
				In.m_Weapon = w;
				v.push_back(In);
			}
			if(f)
				std::fclose(f);
			CTasGame T;
			for(int t1 = T1lo; t1 <= T1hi; t1++)
				for(int a10 = gs_PadA0 * 10; a10 < gs_PadA1 * 10; a10 += gs_PadStep)
				{
					float a = a10 / 10.0f;
					if(v[t1].m_Hook && !(t1 > 0 && v[t1 - 1].m_Hook))
						continue;
					T.CopyFrom(G);
					for(int k = 0; k < t1; k++)
						T.Step(v[k]);
					STasInput In = v[t1];
					In.m_Fire = 1;
					In.m_TX = (int16_t)std::lround(std::cos(a * pi / 180) * 1000);
					In.m_TY = (int16_t)std::lround(std::sin(a * pi / 180) * 1000);
					T.Step(In);
					vec2 E;
					int TE;
					if(!T.NextExplosion(E, TE))
						continue;
					int k1 = TE - G.m_Tick - 1;
					if(k1 >= TE0 && k1 <= TE1 && E.x >= Bx0 && E.x <= Bx1 && E.y >= By0 && E.y <= By1)
						std::printf("PAD t1 %d aim %.1f %d %d te %d E %.0f %.0f\n", t1, a, In.m_TX, In.m_TY, k1, E.x, E.y);
				}
		}
		else if(Cmd == "distinit")
		{
			gs_DFStencil = 3;
			gs_DF.Build(CTasGame::Map(), {TILE_FINISH});
			std::printf("dist field ready\n");
		}
		else if(Cmd == "scorefile")
		{
			// scorefile IN OUT: lines "t x y vx vy" -> "t D along"
			std::string In, Out;
			L >> In >> Out;
			FILE *fi = std::fopen(In.c_str(), "r");
			FILE *fo = std::fopen(Out.c_str(), "w");
			int t;
			float x, y, vx, vy;
			while(fi && std::fscanf(fi, "%d %f %f %f %f", &t, &x, &y, &vx, &vy) == 5)
			{
				vec2 P(x, y);
				float D = gs_DF.Sample(P);
				vec2 Gr = gs_DF.Grad(P);
				std::fprintf(fo, "%d %.1f %.2f\n", t, D, -dot(vec2(vx, vy), Gr));
			}
			if(fi)
				std::fclose(fi);
			if(fo)
				std::fclose(fo);
		}
		else if(Cmd == "stack2")
		{
			// stack2 FILE t1lo t1hi tcheck: shot 1 at t1 (1 deg aims) whose explosion comes 25..28 ticks later near the tee,
			// shot 2 at t1+25..t1+27 (72 aims); hook released / dir right after the first explosion. Best vx at tcheck.
			std::string Path;
			int T1lo, T1hi, TC;
			L >> Path >> T1lo >> T1hi >> TC;
			std::vector<STasInput> v;
			FILE *f = std::fopen(Path.c_str(), "r");
			int d, j, h, fi, tx, ty, w;
			while(f && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
			{
				STasInput In;
				In.m_Dir = d;
				In.m_Jump = j;
				In.m_Hook = h;
				In.m_Fire = 0;
				In.m_TX = tx;
				In.m_TY = ty;
				In.m_Weapon = w;
				v.push_back(In);
			}
			if(f)
				std::fclose(f);
			// base positions
			std::vector<vec2> vPos;
			{
				CTasGame B;
				B.CopyFrom(G);
				for(auto &In : v)
				{
					B.Step(In);
					vPos.push_back(B.Pos());
				}
			}
			auto Aim = [](STasInput &In, float Deg) {
				In.m_TX = (int16_t)std::lround(std::cos(Deg * pi / 180) * 1000);
				In.m_TY = (int16_t)std::lround(std::sin(Deg * pi / 180) * 1000);
			};
			struct SR
			{
				float m_Vx, m_V;
				int m_T1, m_A1, m_T2, m_A2;
			};
			std::vector<SR> vR;
			CTasGame T, U;
			int NCand = 0;
			for(int t1 = T1lo; t1 <= T1hi; t1++)
				for(int a1 = 0; a1 < 360; a1++)
				{
					// first shot: where and when does it explode?
					T.CopyFrom(G);
					for(int k = 0; k < t1; k++)
						T.Step(v[k]);
					STasInput In = v[t1];
					In.m_Fire = 1;
					bool HookLaunch = In.m_Hook && !(t1 > 0 && v[t1 - 1].m_Hook);
					if(HookLaunch)
						continue;
					Aim(In, a1);
					T.Step(In);
					vec2 E;
					int TE;
					if(!T.NextExplosion(E, TE))
						continue;
					int k1 = TE - G.m_Tick - 1; // input index of the explosion tick
					if(k1 < t1 + 24 || k1 > t1 + 29 || k1 >= (int)vPos.size() || distance(vPos[k1], E) > 90)
						continue;
					NCand++;
					for(int t2 = t1 + 25; t2 <= t1 + 27; t2++)
						for(int a2 = 0; a2 < 360; a2 += 5)
						{
							U.CopyFrom(T);
							bool Dead = false, Exploded = false;
							for(int k = t1 + 1; k < TC && k < (int)v.size(); k++)
							{
								STasInput I2 = v[k];
								if(k == t1 + 1)
									I2.m_Fire = 0;
								if(Exploded)
								{
									I2.m_Hook = 0;
									I2.m_Dir = 1;
								}
								if(k == t2 || k == t2 + 1)
								{
									I2.m_Fire = 1;
									if(!(I2.m_Hook && !v[k - 1].m_Hook))
										Aim(I2, a2);
								}
								int Before = U.NumProjectiles();
								U.Step(I2);
								if(Before > U.NumProjectiles())
									Exploded = true;
								if(U.Frozen() || U.EnteredFreeze())
								{
									Dead = true;
									break;
								}
							}
							if(!Dead)
								vR.push_back({U.Vel().x, length(U.Vel()), t1, a1, t2, a2});
						}
				}
			std::sort(vR.begin(), vR.end(), [](const SR &a, const SR &b) { return a.m_Vx > b.m_Vx; });
			std::printf("first-shot candidates %d\n", NCand);
			for(int k = 0; k < 10 && k < (int)vR.size(); k++)
				std::printf("t1 %d a1 %d t2 %d a2 %d: vx %.2f |v| %.2f\n", vR[k].m_T1, vR[k].m_A1, vR[k].m_T2, vR[k].m_A2, vR[k].m_Vx, vR[k].m_V);
		}
		else if(Cmd == "keepfirefrom")
			L >> gs_KeepFireFrom;
		else if(Cmd == "reload0")
			G.Chr()->m_ReloadTimer = 0;
		else if(Cmd == "reload")
		{
			int r;
			L >> r;
			G.Chr()->m_ReloadTimer = r;
		}
		else if(Cmd == "nextexp")
		{
			vec2 E;
			int T;
			if(G.NextExplosion(E, T))
				std::printf("next explosion at tick %d pos %.1f %.1f\n", T, E.x, E.y);
			else
				std::printf("no projectile\n");
		}
		else if(Cmd == "quiet")
			Loud = false;
		else if(Cmd == "loud")
			Loud = true;
		else if(Cmd == "print")
			Print(G);
		else if(Cmd == "in")
		{
			STasInput In;
			int d, j, h, f, tx, ty, n = 1;
			L >> d >> j >> h >> f >> tx >> ty;
			if(!(L >> n))
				n = 1;
			In.m_Dir = d;
			In.m_Jump = j;
			In.m_Hook = h;
			In.m_Fire = f;
			In.m_TX = tx;
			In.m_TY = ty;
			In.m_Weapon = G.HasGrenade() ? WEAPON_GRENADE : -1;
			for(int i = 0; i < n; i++)
			{
				G.Step(In);
				if(Loud)
					Print(G);
			}
		}
	}
	Print(G);
	return 0;
}
