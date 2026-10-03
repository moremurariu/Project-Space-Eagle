// nest: beam search whose candidates are valued by a small inner beam search a few ticks ahead.
// usage: nest <map> key=value...
//   prefix=FILE tp=x,y,vx,vy gren=1 jumped=N started=1     start state
//   stopx=PX | goal=x0,y0,x1,y1 (tiles) | goal=grenade | goal=finish
//   w1=N (outer beam) pre=K (inner-evaluate the best K*w1 children) w0=N (inner beam) h=N (inner horizon)
//   irep=N (inner decisions every N ticks) iang=N ifire=N (inner hook / shot angles)
//   angles=N fireangles=N firerange=PX alpha=A vref=V eshare=F threads=N maxticks=N out=FILE
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "distfield.h"
#include "sim.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
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

struct SParams
{
	int m_W1 = 50, m_Pre = 3, m_W0 = 20, m_H = 20, m_IRep = 2, m_IAng = 24, m_IFire = 24;
	int m_Angles = 64, m_FireAngles = 48;
	float m_FireRange = 300;
	float m_Alpha = 8, m_Vref = 25, m_EShare = 0.3f;
	int m_Threads = 2, m_MaxTicks = 400;
	std::string m_Out = "nest.txt";
	float m_StopX = 0;
	int m_GoalMode = 0; // 0 finish 1 grenade 2 rect 3 stopx
	float m_aGoal[4] = {0, 0, 0, 0};
};
static SParams gs_P;
static SDistField gs_Dist;
static int gs_T0 = 0; // tick of the start state

static bool GoalReached(const CTasGame &G)
{
	switch(gs_P.m_GoalMode)
	{
	case 0: return G.m_FinishTick >= 0;
	case 1: return G.HasGrenade();
	case 3: return G.Pos().x > gs_P.m_StopX;
	default:
	{
		vec2 P = G.Pos();
		const float *g = gs_P.m_aGoal;
		return P.x >= g[0] * 32 && P.y >= g[1] * 32 && P.x < (g[2] + 1) * 32 && P.y < (g[3] + 1) * 32;
	}
	}
}

static bool Dead(const CTasGame &G) { return G.Frozen() || G.EnteredFreeze() || G.m_StartTick == -2; }
static int JumpsLeft(const CTasGame &G) { return G.Grounded() ? 2 : ((G.Jumped() & 2) ? 0 : 1); }

static float PlainScore(const CTasGame &G)
{
	vec2 P = G.Pos();
	float D = gs_Dist.Sample(P);
	if(D > 1e8f)
		return 1e9f;
	vec2 Gr = gs_Dist.Grad(P);
	float Along = -dot(G.Vel(), Gr);
	return (G.m_Tick - gs_T0) + (D - gs_P.m_Alpha * Along) / gs_P.m_Vref;
}
static float Energy(const CTasGame &G) { return dot(G.Vel(), G.Vel()) - G.Pos().y + 200.0f * JumpsLeft(G); }
static float ArrivalValue(const CTasGame &G) { return (G.m_Tick - gs_T0) - 0.05f * length(G.Vel()); }

static void HookTargets(const CTasGame &G, int NumAngles, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.Pos();
	const float Range = 380.0f + 6.0f * length(G.Vel()) + 20.0f;
	int aSeen[256];
	int NumSeen = 0;
	for(int a = 0; a < NumAngles; a++)
	{
		float Ang = 2 * pi * a / NumAngles;
		vec2 D(std::cos(Ang), std::sin(Ang));
		int Hit = -1;
		for(float r = 42.0f; r < Range; r += 4.0f)
		{
			vec2 Q = P + D * r;
			int tx = (int)std::floor(Q.x / 32), ty = (int)std::floor(Q.y / 32);
			if(M.Tile(tx, ty) == TILE_SOLID)
			{
				Hit = ty * M.m_W + tx;
				break;
			}
		}
		if(Hit < 0)
			continue;
		bool Dup = false;
		for(int i = 0; i < NumSeen; i++)
			Dup |= aSeen[i] == Hit;
		if(Dup || NumSeen >= 256)
			continue;
		aSeen[NumSeen++] = Hit;
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

static void FireTargets(const CTasGame &G, int NumAngles, float Range, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.Pos();
	for(int a = 0; a < NumAngles; a++)
	{
		float Ang = 2 * pi * a / NumAngles;
		vec2 D(std::cos(Ang), std::sin(Ang));
		bool Hit = false;
		for(float r = 8.0f; r < Range && !Hit; r += 6.0f)
		{
			vec2 Q = P + D * r;
			Hit = M.Tile((int)std::floor(Q.x / 32), (int)std::floor(Q.y / 32)) == TILE_SOLID;
		}
		if(!Hit)
			continue;
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

// every input that makes a difference this tick: dir x jump(new press) x hook(keep/release or new aim) and shots
static void GenActions(const CTasGame &G, const STasInput &Prev, int HookAngles, int FireAngles, std::vector<STasInput> &vOut)
{
	vOut.clear();
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vHooks, s_vFire;
	const bool Hooking = G.m_LastHook;
	if(!Hooking)
		HookTargets(G, HookAngles, s_vHooks);
	const bool CanJump = G.Grounded() || !(G.Jumped() & 2);
	const int Weapon = G.HasGrenade() ? WEAPON_GRENADE : -1;
	const bool CanFire = G.HasGrenade() && G.ActiveWeapon() == WEAPON_GRENADE && G.ReloadTimer() == 0;
	if(CanFire)
		FireTargets(G, FireAngles, gs_P.m_FireRange, s_vFire);
	for(int Dir = -1; Dir <= 1; Dir++)
		for(int Jump = 0; Jump <= 1; Jump++)
		{
			if(Jump && (G.m_LastJump || !CanJump))
				continue;
			STasInput In;
			In.m_Dir = Dir;
			In.m_Jump = Jump;
			In.m_Weapon = Weapon;
			In.m_TX = Prev.m_TX;
			In.m_TY = Prev.m_TY;
			In.m_Hook = Hooking;
			vOut.push_back(In);
			if(Hooking)
			{
				STasInput R = In;
				R.m_Hook = 0;
				vOut.push_back(R);
			}
			else
				for(auto [TX, TY] : s_vHooks)
				{
					STasInput H = In;
					H.m_Hook = 1;
					H.m_TX = TX;
					H.m_TY = TY;
					vOut.push_back(H);
				}
			if(CanFire && !Jump)
				for(auto [TX, TY] : s_vFire)
				{
					// a hook that hasn't launched yet would follow the new aim: only shoot with the hook held or idle-and-released
					if(Hooking && (G.HookState() == HOOK_IDLE || G.HookState() == HOOK_RETRACTED))
						continue;
					STasInput F = In;
					F.m_Fire = 1;
					F.m_TX = TX;
					F.m_TY = TY;
					vOut.push_back(F);
				}
		}
}

static uint64_t CellKey(const CTasGame &G, float Cell, float VCell)
{
	vec2 P = G.Pos(), V = G.Vel();
	int64_t k = (int64_t)std::floor(P.x / Cell);
	k = k * 4096 + (int64_t)std::floor(P.y / Cell);
	k = k * 256 + ((int64_t)std::floor(V.x / VCell) & 255);
	k = k * 256 + ((int64_t)std::floor(V.y / VCell) & 255);
	int hs = G.HookState();
	k = k * 3 + (hs <= 0 ? 0 : (hs == HOOK_GRABBED ? 2 : 1));
	k = k * 3 + JumpsLeft(G);
	k = k * 4 + (G.ReloadTimer() > 0 ? 1 : 0) + (G.NumProjectiles() > 0 ? 2 : 0);
	k = k * 2 + G.m_LastJump;
	k = k * 2 + G.m_LastHook;
	return (uint64_t)k;
}

// the state after a shot's explosion (holding the same input), or false if it dies
static bool AfterExplosion(const CTasGame &G, const STasInput &In, CTasGame &Out)
{
	Out.CopyFrom(G);
	STasInput L = In;
	L.m_Fire = 0;
	for(int k = 0; k < 12 && Out.NumProjectiles() > 0; k++)
	{
		Out.Step(L);
		if(Dead(Out))
			return false;
	}
	return true;
}

struct SNode
{
	std::unique_ptr<CTasGame> m_pG;
	STasInput m_Prev;
};

struct SCand
{
	int m_Parent;
	STasInput m_In;
	float m_S, m_E;
	uint64_t m_Hash, m_Cell;
	bool m_Arrived;
};

// value of a state: the best plain score after H ticks of a small beam search (lower is better)
static float InnerValue(const CTasGame &Start, const STasInput &Prev)
{
	const int W = gs_P.m_W0;
	std::vector<SNode> vBeam;
	vBeam.push_back({std::make_unique<CTasGame>(), Prev});
	vBeam[0].m_pG->CopyFrom(Start);
	float BestArr = 1e9f;
	std::vector<STasInput> vActs;
	CTasGame Tmp, Look;
	struct SC
	{
		int m_P;
		STasInput m_In;
		float m_S, m_E;
		uint64_t m_Hash, m_Cell;
	};
	std::vector<SC> vC;
	for(int h = 0; h < gs_P.m_H; h += gs_P.m_IRep)
	{
		vC.clear();
		for(int i = 0; i < (int)vBeam.size(); i++)
		{
			const CTasGame &G = *vBeam[i].m_pG;
			GenActions(G, vBeam[i].m_Prev, gs_P.m_IAng, gs_P.m_IFire, vActs);
			for(const STasInput &In0 : vActs)
			{
				Tmp.CopyFrom(G);
				bool Bad = false, Arr = false;
				for(int r = 0; r < gs_P.m_IRep; r++)
				{
					STasInput In = In0;
					if(r > 0)
						In.m_Fire = 0, In.m_Jump = In0.m_Jump; // a held jump does nothing more; release fire after the press
					Tmp.Step(In);
					if(Dead(Tmp))
					{
						Bad = true;
						break;
					}
					if(GoalReached(Tmp))
					{
						Arr = true;
						break;
					}
				}
				if(Bad)
					continue;
				if(Arr)
				{
					BestArr = std::min(BestArr, ArrivalValue(Tmp));
					continue;
				}
				const CTasGame *pE = &Tmp;
				if(Tmp.NumProjectiles() > 0)
				{
					if(!AfterExplosion(Tmp, In0, Look))
						continue;
					pE = &Look;
				}
				float S = PlainScore(*pE);
				if(S > 1e8f)
					continue;
				vC.push_back({i, In0, S, Energy(*pE), Tmp.Hash(), CellKey(Tmp, 16, 2)});
			}
		}
		if(vC.empty())
		{
			vBeam.clear();
			break;
		}
		// select: (1-eshare) by score, eshare by energy, one per cell
		std::vector<int> vI(vC.size());
		for(size_t k = 0; k < vC.size(); k++)
			vI[k] = (int)k;
		std::sort(vI.begin(), vI.end(), [&](int a, int b) { return vC[a].m_S < vC[b].m_S; });
		std::unordered_set<uint64_t> Cells, Hashes;
		std::vector<int> vSel;
		const int NS = (int)(W * (1.0f - gs_P.m_EShare));
		for(int k : vI)
		{
			if((int)vSel.size() >= NS)
				break;
			if(!Hashes.insert(vC[k].m_Hash).second || !Cells.insert(vC[k].m_Cell).second)
				continue;
			vSel.push_back(k);
		}
		if(gs_P.m_EShare > 0)
		{
			float Lim = vC[vI[0]].m_S + 40.0f;
			std::vector<int> vE;
			for(int k : vI)
				if(vC[k].m_S <= Lim)
					vE.push_back(k);
			std::sort(vE.begin(), vE.end(), [&](int a, int b) { return vC[a].m_E > vC[b].m_E; });
			for(int k : vE)
			{
				if((int)vSel.size() >= W)
					break;
				if(!Hashes.insert(vC[k].m_Hash).second || !Cells.insert(vC[k].m_Cell).second)
					continue;
				vSel.push_back(k);
			}
		}
		std::vector<SNode> vNew;
		for(int k : vSel)
		{
			SNode N;
			N.m_pG = std::make_unique<CTasGame>();
			N.m_pG->CopyFrom(*vBeam[vC[k].m_P].m_pG);
			for(int r = 0; r < gs_P.m_IRep; r++)
			{
				STasInput In = vC[k].m_In;
				if(r > 0)
					In.m_Fire = 0;
				N.m_pG->Step(In);
			}
			N.m_Prev = vC[k].m_In;
			vNew.push_back(std::move(N));
		}
		vBeam = std::move(vNew);
	}
	float Best = BestArr;
	for(auto &N : vBeam)
		Best = std::min(Best, PlainScore(*N.m_pG));
	return Best;
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: nest <map> key=value...\n");
		return 1;
	}
	std::string Prefix;
	bool Tp = false;
	float TpX = 0, TpY = 0, TpVx = 0, TpVy = 0;
	int Gren = 0, Jumped = -1, Started = 0;
	std::string Goal = "finish";
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		const char *v = V.c_str();
		if(K == "prefix")
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
		{
			gs_P.m_StopX = atof(v);
			gs_P.m_GoalMode = 3;
		}
		else if(K == "goal")
			Goal = V;
		else if(K == "w1")
			gs_P.m_W1 = atoi(v);
		else if(K == "pre")
			gs_P.m_Pre = atoi(v);
		else if(K == "w0")
			gs_P.m_W0 = atoi(v);
		else if(K == "h")
			gs_P.m_H = atoi(v);
		else if(K == "irep")
			gs_P.m_IRep = atoi(v);
		else if(K == "iang")
			gs_P.m_IAng = atoi(v);
		else if(K == "ifire")
			gs_P.m_IFire = atoi(v);
		else if(K == "angles")
			gs_P.m_Angles = atoi(v);
		else if(K == "fireangles")
			gs_P.m_FireAngles = atoi(v);
		else if(K == "firerange")
			gs_P.m_FireRange = atof(v);
		else if(K == "alpha")
			gs_P.m_Alpha = atof(v);
		else if(K == "vref")
			gs_P.m_Vref = atof(v);
		else if(K == "eshare")
			gs_P.m_EShare = atof(v);
		else if(K == "threads")
			gs_P.m_Threads = atoi(v);
		else if(K == "maxticks")
			gs_P.m_MaxTicks = atoi(v);
		else if(K == "out")
			gs_P.m_Out = V;
		else
		{
			std::printf("unknown key %s\n", K.c_str());
			return 1;
		}
	}
	const SMapInfo &M = CTasGame::Map();
	vec2 GrenPos(0, 0);
	for(int y = 0; y < M.m_H; y++)
		for(int x = 0; x < M.m_W; x++)
			if(M.Tile(x, y) == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
				GrenPos = vec2(x * 32 + 16, y * 32 + 16);
	if(gs_P.m_GoalMode == 3)
		gs_Dist.Build(M, {TILE_FINISH});
	else if(Goal == "finish")
		gs_Dist.Build(M, {TILE_FINISH});
	else if(Goal == "grenade")
	{
		gs_P.m_GoalMode = 1;
		gs_Dist.BuildFromPoint(M, GrenPos);
	}
	else
	{
		gs_P.m_GoalMode = 2;
		float *g = gs_P.m_aGoal;
		if(std::sscanf(Goal.c_str(), "%f,%f,%f,%f", &g[0], &g[1], &g[2], &g[3]) != 4)
			return 1;
		SMapInfo Tmp = M;
		for(int y = (int)g[1]; y <= (int)g[3]; y++)
			for(int x = (int)g[0]; x <= (int)g[2]; x++)
				if(Tmp.m_vGame[y * Tmp.m_W + x] == 0)
					Tmp.m_vGame[y * Tmp.m_W + x] = 250;
		gs_Dist.Build(Tmp, {250});
	}

	std::vector<STasInput> vPrefix;
	if(!Prefix.empty())
		vPrefix = ReadInputs(Prefix.c_str());
	auto pStart = std::make_unique<CTasGame>();
	pStart->Spawn(M.m_vSpawns[0]);
	for(const auto &In : vPrefix)
		pStart->Step(In);
	if(Gren)
	{
		pStart->Chr()->GiveWeapon(WEAPON_GRENADE);
		pStart->Chr()->m_Core.m_ActiveWeapon = WEAPON_GRENADE;
	}
	if(Tp)
		pStart->SetState(vec2(TpX, TpY), vec2(TpVx, TpVy));
	if(Jumped >= 0)
		pStart->Chr()->m_Core.m_Jumped = Jumped;
	if(Started && !pStart->m_Started)
	{
		pStart->m_Started = true;
		pStart->m_StartTick = pStart->m_Tick;
	}
	gs_T0 = pStart->m_Tick;
	std::printf("start tick %d pos %.0f %.0f vel %.2f %.2f\n", gs_T0, pStart->Pos().x, pStart->Pos().y, pStart->Vel().x, pStart->Vel().y);

	std::vector<SNode> vBeam;
	vBeam.push_back({std::move(pStart), vPrefix.empty() ? STasInput() : vPrefix.back()});
	std::vector<std::vector<std::pair<int, STasInput>>> vHist;
	const int NT = std::max(1, gs_P.m_Threads);
	auto T0 = std::chrono::steady_clock::now();
	for(int Step = 0; Step < gs_P.m_MaxTicks && !vBeam.empty(); Step++)
	{
		// expand
		std::vector<std::vector<SCand>> vTC(NT);
		std::atomic<int> Next{0};
		auto Exp = [&](int T) {
			CTasGame Tmp, Look;
			std::vector<STasInput> vActs;
			while(true)
			{
				int i = Next.fetch_add(1);
				if(i >= (int)vBeam.size())
					break;
				const CTasGame &G = *vBeam[i].m_pG;
				GenActions(G, vBeam[i].m_Prev, gs_P.m_Angles, gs_P.m_FireAngles, vActs);
				for(const STasInput &In : vActs)
				{
					Tmp.CopyFrom(G);
					Tmp.Step(In);
					if(Dead(Tmp))
						continue;
					if(GoalReached(Tmp))
					{
						vTC[T].push_back({i, In, ArrivalValue(Tmp), 0, Tmp.Hash(), 0, true});
						continue;
					}
					const CTasGame *pE = &Tmp;
					if(Tmp.NumProjectiles() > 0)
					{
						if(!AfterExplosion(Tmp, In, Look))
							continue;
						pE = &Look;
					}
					float S = PlainScore(*pE);
					if(S > 1e8f)
						continue;
					vTC[T].push_back({i, In, S, Energy(*pE), Tmp.Hash(), CellKey(Tmp, 8, 1), false});
				}
			}
		};
		{
			std::vector<std::thread> vTh;
			for(int T = 1; T < NT; T++)
				vTh.emplace_back(Exp, T);
			Exp(0);
			for(auto &Th : vTh)
				Th.join();
		}
		std::vector<SCand> vAll;
		for(auto &v : vTC)
			vAll.insert(vAll.end(), v.begin(), v.end());
		// arrivals end the search: reconstruct and write the best one
		float BestArr = 1e9f;
		int BestArrK = -1;
		for(int k = 0; k < (int)vAll.size(); k++)
			if(vAll[k].m_Arrived && vAll[k].m_S < BestArr)
			{
				BestArr = vAll[k].m_S;
				BestArrK = k;
			}
		if(BestArrK >= 0)
		{
			std::vector<STasInput> v;
			v.push_back(vAll[BestArrK].m_In);
			int Slot = vAll[BestArrK].m_Parent;
			for(int s = (int)vHist.size() - 1; s >= 0; s--)
			{
				v.push_back(vHist[s][Slot].second);
				Slot = vHist[s][Slot].first;
			}
			std::reverse(v.begin(), v.end());
			std::vector<STasInput> Full = vPrefix;
			Full.insert(Full.end(), v.begin(), v.end());
			WriteInputs(gs_P.m_Out, Full);
			std::printf("ARRIVED after %d ticks (value %.2f) [%.0fs] -> %s\n", Step + 1, BestArr, std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count(),
				gs_P.m_Out.c_str());
			return 0;
		}
		if(vAll.empty())
		{
			std::printf("beam died at step %d\n", Step);
			return 1;
		}
		// prefilter by plain score (+ energy share), one per cell
		std::sort(vAll.begin(), vAll.end(), [](const SCand &a, const SCand &b) { return a.m_S < b.m_S; });
		std::unordered_set<uint64_t> Cells, Hashes;
		std::vector<int> vPre;
		const int NPre = gs_P.m_Pre * gs_P.m_W1;
		for(int k = 0; k < (int)vAll.size() && (int)vPre.size() < (int)(NPre * (1.0f - gs_P.m_EShare)); k++)
			if(Hashes.insert(vAll[k].m_Hash).second && Cells.insert(vAll[k].m_Cell).second)
				vPre.push_back(k);
		{
			float Lim = vAll[0].m_S + 40.0f;
			std::vector<int> vE;
			for(int k = 0; k < (int)vAll.size() && vAll[k].m_S <= Lim; k++)
				vE.push_back(k);
			std::sort(vE.begin(), vE.end(), [&](int a, int b) { return vAll[a].m_E > vAll[b].m_E; });
			for(int k : vE)
			{
				if((int)vPre.size() >= NPre)
					break;
				if(Hashes.insert(vAll[k].m_Hash).second && Cells.insert(vAll[k].m_Cell).second)
					vPre.push_back(k);
			}
		}
		// inner evaluation
		std::vector<float> vVal(vPre.size(), 1e9f);
		std::atomic<int> NextV{0};
		auto Ev = [&]() {
			CTasGame Tmp;
			while(true)
			{
				int q = NextV.fetch_add(1);
				if(q >= (int)vPre.size())
					break;
				const SCand &C = vAll[vPre[q]];
				Tmp.CopyFrom(*vBeam[C.m_Parent].m_pG);
				Tmp.Step(C.m_In);
				vVal[q] = InnerValue(Tmp, C.m_In);
			}
		};
		{
			std::vector<std::thread> vTh;
			for(int T = 1; T < NT; T++)
				vTh.emplace_back(Ev);
			Ev();
			for(auto &Th : vTh)
				Th.join();
		}
		std::vector<int> vOrd(vPre.size());
		for(size_t q = 0; q < vPre.size(); q++)
			vOrd[q] = (int)q;
		std::sort(vOrd.begin(), vOrd.end(), [&](int a, int b) { return vVal[a] < vVal[b]; });
		std::vector<SNode> vNew;
		std::vector<std::pair<int, STasInput>> vH;
		for(int q : vOrd)
		{
			if((int)vNew.size() >= gs_P.m_W1 || vVal[q] > 1e8f)
				break;
			const SCand &C = vAll[vPre[q]];
			SNode N;
			N.m_pG = std::make_unique<CTasGame>();
			N.m_pG->CopyFrom(*vBeam[C.m_Parent].m_pG);
			N.m_pG->Step(C.m_In);
			N.m_Prev = C.m_In;
			vH.push_back({C.m_Parent, C.m_In});
			vNew.push_back(std::move(N));
		}
		vHist.push_back(std::move(vH));
		vBeam = std::move(vNew);
		if(!vBeam.empty())
		{
			const CTasGame &L = *vBeam[0].m_pG;
			std::printf("step %d: cands %zu pre %zu | best value %.2f pos %.0f %.0f vel %.1f %.1f [%.0fs]\n", Step, vAll.size(), vPre.size(), vVal[vOrd[0]], L.Pos().x, L.Pos().y,
				L.Vel().x, L.Vel().y, std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count());
			std::fflush(stdout);
		}
	}
	std::printf("no arrival\n");
	return 1;
}
