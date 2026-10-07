// nadesearch: stage search for grenade stacks on a single block (see FINDINGS.md).
//
// A "stage" starts from an exact state (the replay of an input prefix: the tee at rest, or just launched by a stack)
// and ends with a stack at step E: the tee jumps at step E-1 (ground jump, or its air jump) and k grenades explode at
// step E right below it. Between, the tee flies (air control + an optional air jump + landing + optional hop) and fires
// grenades (>= 25 steps apart) whose flights all end exactly at step E next to it.
//
// The search is "inverse": the y-motion only depends on the jump schedule, so for every jump schedule (y-program) and
// every E the tool computes, for every possible fire step s, the band of x positions from which a grenade fired at s
// lands under the tee at E ("gates"). A DFS picks gates (spacing >= 25) whose x-bands an air-controlled x-path can
// visit (interval propagation with a speed bound, landing on the block). The best candidates are then realised
// exactly: an x-controller search finds dir inputs through the gate bands, SolveAim finds the integer aims, and the
// whole stage is replayed on the exact stepper (server projectile semantics).
//
// usage: nadesearch MAP PREFIX OUTPREFIX [key=value...]
//   tmax=320     longest stage (steps after the prefix)
//   wait=60      longest stand on the block before the final jump
//   jmin/jmax    range of the first jump (air jump in flight, or ground hop when standing), steps after the prefix
//   hop=1        also try a ground hop after landing (+ an air jump in it)
//   top=12       candidates to realise exactly
//   out=4        best realised stages written as OUTPREFIX<i>.txt (prefix + stage inputs)
//   minvy=0      only report stages with a launch speed above this
//   airfinal=1   also try ending with the air jump at E-1 instead of landing
//   kmax=5       largest stack (grenades incl. the point-blank one)
//   xc=          x of the tee at E (default: block centre)
#include "nade.h"

#include <base/logger.h>
#include <base/str.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <string>

static std::map<std::string, std::string> gs_Args;
static double ArgF(const char *k, double d)
{
	auto it = gs_Args.find(k);
	return it == gs_Args.end() ? d : std::atof(it->second.c_str());
}
static int ArgI(const char *k, int d) { return (int)ArgF(k, d); }

// block geometry (the single solid tile)
static float gs_BX0, gs_BX1, gs_BTop; // block x range [BX0, BX1), top y
static float gs_XC; // tee x at E (target)

static STasInput MkIn(int Dir, int Jump, int Fire = 0, int TX = 0, int TY = 1000)
{
	STasInput In;
	In.m_Dir = Dir;
	In.m_Jump = Jump;
	In.m_Hook = 0;
	In.m_Fire = Fire;
	In.m_TX = TX;
	In.m_TY = TY;
	In.m_Weapon = 3;
	return In;
}

struct SInterval
{
	float a, b;
};

// x-bands of fire positions at height y from which a grenade explodes right under the tee (at Q, x = gs_XC) Tau steps later.
// Tile hits on the block top with the hit point within [XC-Align, XC+Align] (clipped to the block), plus for Tau == 101
// lifetime explosions just under Q.
static int GateBands(float y, int Tau, float Qy, float Align, SInterval *pOut)
{
	int n = 0;
	auto Add = [&](float a, float b) {
		if(a > b)
			std::swap(a, b);
		// merge with an overlapping band
		for(int i = 0; i < n; i++)
			if(!(b < pOut[i].a - 1 || a > pOut[i].b + 1))
			{
				pOut[i].a = std::min(pOut[i].a, a);
				pOut[i].b = std::max(pOut[i].b, b);
				return;
			}
		pOut[n++] = {a, b};
	};
	float xe0 = std::max(gs_BX0 + 0.5f, gs_XC - Align), xe1 = std::min(gs_BX1 - 0.5f, gs_XC + Align);
	if(xe0 > xe1)
		return 0;
	// tile hit on the top surface during (Tau-1, Tau]
	float wmin[2] = {1e9f, 1e9f}, wmax[2] = {-1e9f, -1e9f};
	bool any = false;
	for(int i = 0; i <= 8; i++)
	{
		float tc = Tau - 1 + i / 8.0f;
		if(tc <= 0.05f)
			continue;
		float r = 21.0f + 20.0f * tc;
		float dy = (gs_BTop - y - 0.28f * tc * tc) / r;
		if(dy < -1 || dy > 1)
			continue;
		if(20.0f * dy + 0.56f * tc <= 0) // must be descending
			continue;
		float w = r * std::sqrt(1 - dy * dy);
		any = true;
		wmin[0] = std::min(wmin[0], w);
		wmax[0] = std::max(wmax[0], w);
	}
	if(any)
	{
		Add(xe0 - wmax[0], xe1 - wmin[0]); // aim right (fire from the left)
		Add(xe0 + wmin[0], xe1 + wmax[0]); // aim left
	}
	if(Tau == 101)
	{
		// lifetime explosion at T = (x, Qy + dd), dd in [8, 40]: |T - (P + 2856.28 y)| = 2041
		for(float dd = 8; dd <= 40; dd += 8)
		{
			float ty = Qy + dd;
			if(ty > gs_BTop - 0.5f && (gs_XC > gs_BX0 - 1 && gs_XC < gs_BX1 + 1))
				continue; // inside the block
			float dyy = ty - y - 2856.28f;
			float r = 2041.0f;
			if(std::fabs(dyy) > r)
				continue;
			float w = std::sqrt(r * r - dyy * dyy);
			Add(gs_XC - Align - w, gs_XC + Align - w);
			Add(gs_XC - Align + w, gs_XC + Align + w);
		}
	}
	return n;
}

struct SGate
{
	int s; // fire step
	int n;
	SInterval aI[8];
};

struct SYProg
{
	int J1 = -1, H = -1, J2 = -1; // jump steps (absolute), -1 = none
	bool AirFinal = false;
};

struct SCand
{
	SYProg Y;
	int E;
	int K; // gates (without the point-blank one)
	int aS[5];
	SInterval aX[5]; // chosen band per gate
	float Score;
};

// the y-profile of a y-program (dir 0 all the way, the tee stays over the block)
struct SProfile
{
	std::vector<vec2> vP; // pos after step k (index k - K0), [0] = start
	std::vector<uint8_t> vGround; // grounded after step k (IsOnGround at the pos after step k)
	std::vector<uint8_t> vCanAir; // air jump still available after step k
	int Land = -1; // first grounded step (absolute) after leaving the ground
};

static int gs_K0; // prefix length
static CNadeG gs_Start;
static std::vector<STasInput> gs_vPrefix;

static void Profile(const SYProg &Y, int Steps, SProfile &Pr)
{
	CNadeG F = gs_Start;
	Pr.vP.assign(1, F.m_Pos);
	Pr.vGround.assign(1, F.Grounded());
	Pr.vCanAir.assign(1, !(F.m_Core.m_Jumped & 2));
	Pr.Land = -1;
	bool Left = !F.Grounded();
	for(int k = gs_K0 + 1; k <= gs_K0 + Steps; k++)
	{
		int J = (k == Y.J1 || k == Y.H || k == Y.J2) ? 1 : 0;
		// the y-motion does not depend on x: keep the tee over the block (so it lands like an x-controlled path would)
		F.m_Core.m_Pos.x = gs_XC;
		F.m_Pos.x = gs_XC;
		F.m_Core.m_Vel.x = 0;
		F.Step(MkIn(0, J));
		bool G = F.Grounded();
		if(!G)
			Left = true;
		if(G && Left && Pr.Land < 0 && k > Y.J1)
			Pr.Land = k;
		Pr.vP.push_back(F.m_Pos);
		Pr.vGround.push_back(G);
		Pr.vCanAir.push_back(!(F.m_Core.m_Jumped & 2));
	}
}

// interval propagation feasibility of an x-path through the chosen gates
struct SXCons
{
	int k; // step (absolute): constraint on pos after step k
	SInterval I;
};

static bool XFeasible(const std::vector<SXCons> &vC, float x0, float vmax, SInterval *pChosen = nullptr)
{
	// vC sorted by k; returns whether a path exists (speed bound vmax per step)
	float lo = x0, hi = x0;
	int kprev = gs_K0;
	for(size_t i = 0; i < vC.size(); i++)
	{
		float d = vmax * (vC[i].k - kprev);
		lo -= d;
		hi += d;
		lo = std::max(lo, vC[i].I.a);
		hi = std::min(hi, vC[i].I.b);
		if(lo > hi)
			return false;
		if(pChosen)
			pChosen[i] = {lo, hi};
		kprev = vC[i].k;
	}
	return true;
}


// ---------------- exact realisation ----------------
struct SPlan
{
	std::vector<STasInput> vIn; // stage inputs (steps K0+1 .. E)
	int E = 0, K = 0;
	float KickY = 0; // sum of upward kicks at E
	vec2 Launch = vec2(0, 0); // velocity after step E
	float Apex = 0; // height (standing y - min y) reached afterwards with idle inputs
	int ApexStep = 0;
	std::string Desc;
};

struct SBeamState
{
	CNadeG F;
	std::vector<STasInput> vIn;
	float Cost;
};

static int JumpAt(const SYProg &Y, int k) { return (k == Y.J1 || k == Y.H || k == Y.J2) ? 1 : 0; }

// find dir inputs from the start to step E-2 satisfying the x constraints (pos after step k in [a,b])
static bool RealizeX(const SYProg &Y, int E, std::vector<SXCons> vC, std::vector<STasInput> &vOut, int Beam)
{
	std::sort(vC.begin(), vC.end(), [](const SXCons &a, const SXCons &b) { return a.k < b.k; });
	std::vector<SBeamState> vB(1);
	vB[0].F = gs_Start;
	vB[0].Cost = 0;
	int kprev = gs_K0;
	for(size_t ci = 0; ci < vC.size(); ci++)
	{
		const SXCons &C = vC[ci];
		int Dk = C.k - kprev;
		if(Dk <= 0)
			continue;
		float Mid = 0.5f * (C.I.a + C.I.b), Half = std::max(0.5f * (C.I.b - C.I.a), 1.0f);
		std::vector<SBeamState> vNext;
		int Step = Dk > 60 ? 2 : 1;
		for(const SBeamState &B : vB)
			for(int d1 = -1; d1 <= 1; d1++)
				for(int d2 = -1; d2 <= 1; d2++)
				{
					if(d1 == d2 && d1 != 0)
					{
					}
					for(int n1 = 0; n1 <= Dk; n1 += Step)
					{
						if(d1 == d2 && n1 > 0)
							break;
						SBeamState S;
						S.F = B.F;
						S.vIn = B.vIn;
						for(int k = kprev + 1; k <= C.k; k++)
						{
							STasInput In = MkIn(k - kprev <= n1 ? d1 : d2, JumpAt(Y, k));
							S.F.Step(In);
							S.vIn.push_back(In);
						}
						float x = S.F.m_Pos.x;
						if(x < C.I.a || x > C.I.b)
							continue;
						S.Cost = B.Cost + std::fabs(x - Mid) / Half + 0.05f * std::fabs(S.F.m_Core.m_Vel.x);
						vNext.push_back(std::move(S));
					}
				}
		if(vNext.empty())
			return false;
		std::sort(vNext.begin(), vNext.end(), [](const SBeamState &a, const SBeamState &b) { return a.Cost < b.Cost; });
		// keep a diverse beam (distinct x / vx)
		std::vector<SBeamState> vKeep;
		for(SBeamState &S : vNext)
		{
			bool Dup = false;
			for(const SBeamState &K : vKeep)
				if(std::fabs(K.F.m_Pos.x - S.F.m_Pos.x) < 1.5f && std::fabs(K.F.m_Core.m_Vel.x - S.F.m_Core.m_Vel.x) < 0.6f)
					Dup = true;
			if(!Dup)
				vKeep.push_back(std::move(S));
			if((int)vKeep.size() >= Beam)
				break;
		}
		vB = std::move(vKeep);
		kprev = C.k;
	}
	vOut = vB[0].vIn;
	return true;
}

static bool RealizeCand(const SCand &C, SPlan &Plan, int Beam)
{
	const int E = C.E;
	SProfile Pr;
	Profile(C.Y, E - gs_K0, Pr);
	std::vector<SXCons> vC;
	for(int i = 0; i < C.K; i++)
		vC.push_back({C.aS[i] - 1, C.aX[i]});
	// over the block when landing and while standing (merged per step below), centred before the final jump
	for(int k = 1; k <= E - 2 - gs_K0; k++)
		if(Pr.vGround[k] || (k + 1 < (int)Pr.vGround.size() && Pr.vGround[k + 1]))
			vC.push_back({gs_K0 + k, {gs_BX0 - 24, gs_BX1 + 24}});
	vC.push_back({E - 2, {gs_XC - 6, gs_XC + 6}});
	std::sort(vC.begin(), vC.end(), [](const SXCons &a, const SXCons &b) { return a.k < b.k; });
	std::vector<SXCons> vM;
	for(const SXCons &c : vC)
	{
		if(!vM.empty() && vM.back().k == c.k)
		{
			vM.back().I.a = std::max(vM.back().I.a, c.I.a);
			vM.back().I.b = std::min(vM.back().I.b, c.I.b);
			if(vM.back().I.a > vM.back().I.b)
				return false;
		}
		else
			vM.push_back(c);
	}
	// thin out the standing constraints (keep every 4th + the first/last of each run) to keep segments long
	std::vector<SXCons> vT;
	for(size_t i = 0; i < vM.size(); i++)
	{
		bool Gate = false;
		for(int g = 0; g < C.K; g++)
			if(vM[i].k == C.aS[g] - 1)
				Gate = true;
		if(Gate || i == 0 || i + 1 == vM.size() || vM[i].k != vM[i - 1].k + 1 || vM[i + 1].k != vM[i].k + 1 || vM[i].k % 4 == 0)
			vT.push_back(vM[i]);
	}
	std::vector<STasInput> vIn;
	if(!RealizeX(C.Y, E, vT, vIn, Beam))
		return false;
	// final jump at E-1, point-blank at E
	vIn.push_back(MkIn(0, 1));
	vIn.push_back(MkIn(0, 0, 1, 0, 30000));
	// replay without the gate fires to get exact positions
	CNadeG F = gs_Start;
	std::vector<vec2> vP(1, F.m_Pos);
	for(size_t i = 0; i + 1 < vIn.size(); i++)
	{
		F.Step(vIn[i]);
		vP.push_back(F.m_Pos);
	}
	vec2 Q = vP[E - 1 - gs_K0];
	// aims
	int K = 0;
	char aBuf[512];
	std::string Desc;
	for(int i = 0; i < C.K; i++)
	{
		int s = C.aS[i];
		SAim A = SolveAim(vP[s - 1 - gs_K0], E - s + 1, Q);
		if(!A.m_Ok || A.m_Kick.y > -6.0f)
			continue;
		STasInput &In = vIn[s - 1 - gs_K0];
		In.m_Fire = 1;
		In.m_TX = A.m_TX;
		In.m_TY = A.m_TY;
		K++;
		str_format(aBuf, sizeof(aBuf), " s%d(tau %d from %.0f,%.0f kick %.1f,%.1f %s)", s, E - s + 1, vP[s - 1 - gs_K0].x, vP[s - 1 - gs_K0].y, A.m_Kick.x, A.m_Kick.y,
			A.m_F.m_Collide ? "hit" : "air");
		Desc += aBuf;
	}
	// exact replay with fires
	F = gs_Start;
	std::vector<SNadeExpl> vLog;
	F.m_pLog = &vLog;
	for(const STasInput &In : vIn)
		F.Step(In);
	float KickY = 0;
	int NE = 0;
	for(const SNadeExpl &X : vLog)
	{
		KickY += -X.m_Force.y;
		if(X.m_Tick == F.m_Tick)
			NE++;
	}
	Plan.vIn = vIn;
	Plan.E = E;
	Plan.K = NE;
	Plan.KickY = KickY;
	Plan.Launch = F.m_Core.m_Vel;
	// apex
	float MinY = F.m_Pos.y;
	int MinK = E;
	CNadeG F2 = F;
	F2.m_pLog = nullptr;
	for(int k = 0; k < 600 && (F2.m_Core.m_Vel.y < 0 || k < 2); k++)
	{
		F2.Step(MkIn(0, 0));
		if(F2.m_Pos.y < MinY)
		{
			MinY = F2.m_Pos.y;
			MinK = E + k + 1;
		}
	}
	Plan.Apex = gs_BTop - 15.0f - MinY; // above the standing height on the platform
	Plan.ApexStep = MinK;
	str_format(aBuf, sizeof(aBuf), "E %d J1 %d H %d J2 %d %s: %d expl at E, kickY %.1f, launch %.2f %.2f, apex %.0f px at step %d (double risk %d) |", E, C.Y.J1, C.Y.H, C.Y.J2,
		C.Y.AirFinal ? "airfinal" : "ground", NE, KickY, Plan.Launch.x, Plan.Launch.y, Plan.Apex, MinK, F.m_DoubleRisk);
	Plan.Desc = std::string(aBuf) + Desc;
	return F.m_DoubleRisk == 0 && !F.m_Bad;
}

int main(int argc, const char **argv)
{
	if(getenv("NADE_LOG"))
		log_set_global_logger_default();
	if(argc < 4 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: nadesearch MAP PREFIX OUTPREFIX [key=value...]\n");
		return 1;
	}
	for(int i = 4; i < argc; i++)
	{
		const char *pEq = std::strchr(argv[i], '=');
		if(pEq)
			gs_Args[std::string(argv[i], pEq - argv[i])] = pEq + 1;
	}
	CNadeG::Init();
	// the launch platform: the first solid tile row at or below (px, py) (default: the spawn point), extended left and
	// right along that row while the tiles are solid with air above (a flat top surface)
	{
		const SMapInfo &M = CTasGame::Map();
		vec2 Sp = M.m_vSpawns.empty() ? vec2(0, 0) : M.m_vSpawns[0];
		float px = ArgF("px", Sp.x), py = ArgF("py", Sp.y);
		int tx = (int)(px / 32), ty = (int)(py / 32);
		auto Solid = [&](int x, int y) { return M.Tile(x, y) == 1 || M.Tile(x, y) == 3; };
		while(ty < M.m_H && !Solid(tx, ty))
			ty++;
		if(ty >= M.m_H)
		{
			std::printf("no platform below %.0f,%.0f\n", px, py);
			return 1;
		}
		int x0 = tx, x1 = tx;
		while(x0 > 0 && Solid(x0 - 1, ty) && !Solid(x0 - 1, ty - 1))
			x0--;
		while(x1 < M.m_W - 1 && Solid(x1 + 1, ty) && !Solid(x1 + 1, ty - 1))
			x1++;
		gs_BX0 = x0 * 32.0f;
		gs_BX1 = x1 * 32.0f + 32;
		gs_BTop = ty * 32.0f;
	}
	gs_XC = ArgF("xc", 0.5f * (gs_BX0 + gs_BX1));
	std::printf("platform: x %.0f..%.0f, top y %.0f, stack x %.0f\n", gs_BX0, gs_BX1, gs_BTop, gs_XC);
	const int TMax = ArgI("tmax", 320), WMax = ArgI("wait", 60), Top = ArgI("top", 12), NOut = ArgI("out", 4);
	const int KMax = ArgI("kmax", 5);
	const bool Hop = ArgI("hop", 1), AirFinal = ArgI("airfinal", 1);
	const float Align = ArgF("align", 6.0f);
	const int Verbose = ArgI("v", 1);

	gs_vPrefix = ReadInputFile(argv[2]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	CNadeG F;
	F.FromGame(G);
	for(const STasInput &In : gs_vPrefix)
		F.Step(In);
	gs_K0 = (int)gs_vPrefix.size();
	// tp=x,y,vx,vy[,airjump(1=available),reload]: EXPLORATION ONLY - teleport the tee after the prefix (the result is
	// not a valid run; used to test which entry states allow a stack)
	if(gs_Args.count("tp"))
	{
		float x, y, vx, vy;
		int Air = 1, Rl = 0;
		std::sscanf(gs_Args["tp"].c_str(), "%f,%f,%f,%f,%d,%d", &x, &y, &vx, &vy, &Air, &Rl);
		F.m_Core.m_Pos = F.m_Pos = F.m_PrevPos = vec2(x, y);
		F.m_Core.m_Vel = vec2(vx, vy);
		F.m_Core.m_Jumped = Air ? 0 : 3;
		F.m_Core.m_JumpedTotal = Air ? 0 : 1;
		F.m_ReloadTimer = Rl;
		F.m_NumProj = 0;
		std::printf("TELEPORTED (exploration only)\n");
	}
	gs_Start = F;
	const int SReady = gs_K0 + F.m_ReloadTimer + 1;
	std::printf("start: step %d pos %.0f %.0f vel %.2f %.2f grounded %d jumped %d reload %d (first fire step %d) proj %d\n", gs_K0, F.m_Pos.x, F.m_Pos.y, F.m_Core.m_Vel.x,
		F.m_Core.m_Vel.y, F.Grounded(), F.m_Core.m_Jumped, F.m_ReloadTimer, SReady, F.m_NumProj);
	const bool StartGround = F.Grounded();
	const float VMax = std::max(5.0f, std::fabs(F.m_Core.m_Vel.x)) * 1.0f;

	// ---------- y-programs ----------
	std::vector<SYProg> vY;
	{
		int J1min = gs_K0 + ArgI("jmin", 1), J1max = gs_K0 + ArgI("jmax", TMax - 20);
		SYProg Y0p;
		vY.push_back(Y0p);
		for(int j = J1min; j <= J1max; j++)
		{
			SYProg Y;
			Y.J1 = j;
			vY.push_back(Y);
		}
	}
	auto T0 = std::chrono::steady_clock::now();
	std::vector<SCand> vCand;
	SProfile Pr;
	std::vector<SYProg> vYAll;
	// expand hops lazily: for each base program, after the landing, ground hop H (and an air jump J2 in the hop)
	for(size_t yi = 0; yi < vY.size(); yi++)
	{
		const SYProg &Yb = vY[yi];
		Profile(Yb, TMax, Pr);
		std::vector<SYProg> vVar = {Yb};
		if(Hop && Pr.Land > 0)
		{
			for(int h = Pr.Land + 1; h <= std::min(Pr.Land + 40, gs_K0 + TMax - 30); h += 2)
			{
				SYProg Y = Yb;
				Y.H = h;
				vVar.push_back(Y);
				for(int j2 = h + 4; j2 <= h + 40; j2 += 3)
				{
					SYProg Y2 = Y;
					Y2.J2 = j2;
					vVar.push_back(Y2);
				}
			}
		}
		for(const SYProg &Y : vVar)
		{
			if(&Y != &vVar[0])
				Profile(Y, TMax, Pr);
			else if(Y.H >= 0)
				Profile(Y, TMax, Pr);
			// ---- candidate E ----
			// (a) ground final: grounded after step E-2 (so the jump at step E-1 is a ground jump), last grounding
			// (b) air final: in the air over the block at E-2, air jump available, tee within reach of the block top
			int LastJ = std::max(Y.J1, std::max(Y.H, Y.J2));
			for(int E = gs_K0 + 30; E <= gs_K0 + TMax; E++)
			{
				int iE2 = E - 2 - gs_K0;
				if(iE2 < 1)
					continue;
				bool Gr = Pr.vGround[iE2];
				bool AF = false;
				if(E - 1 <= LastJ + 1)
					continue; // the final jump must come after the program's jumps (and a release)
				if(!Gr)
				{
					if(!AirFinal || !Pr.vCanAir[iE2])
						continue;
					float h = gs_BTop - Pr.vP[iE2].y; // height of the tee centre above the top
					if(h < 15 || h > 45)
						continue;
					AF = true;
				}
				else
				{
					// stood long enough? limit the wait after the last landing
					int k = iE2;
					while(k > 0 && Pr.vGround[k])
						k--;
					if(iE2 - k > WMax)
						continue;
				}
				// tee position at E (after the jump at E-1): approximately pos(E-2) + jump
				float Qy = Pr.vP[iE2].y - (AF ? 12.0f : 13.2f);
				// gates
				std::vector<SGate> vG;
				int sLo = std::max(SReady, E - 100), sHi = E - 25;
				for(int s = sLo; s <= sHi; s++)
				{
					int ip = s - 1 - gs_K0;
					if(ip < 0)
						continue;
					SGate Gt;
					Gt.s = s;
					Gt.n = GateBands(Pr.vP[ip].y, E - s + 1, Qy, Align, Gt.aI);
					if(Gt.n)
						vG.push_back(Gt);
				}
				bool PB = SReady <= E; // point-blank shot available
				if(vG.empty())
					continue;
				// base constraints: grounded steps (stay over the block), the tee over the block at E-2
				std::vector<SXCons> vBase;
				for(int k = 1; k <= iE2; k++)
					if(Pr.vGround[k])
						vBase.push_back({gs_K0 + k, {gs_BX0 - 26, gs_BX1 + 26}});
				vBase.push_back({E - 2, {gs_XC - 8, gs_XC + 8}});
				// DFS over gates (in s order, spacing >= 25)
				SCand Best;
				Best.K = -1;
				std::vector<int> vSel;
				std::vector<SInterval> vSelI;
				std::vector<SXCons> vC;
				auto Feas = [&]() {
					vC = vBase;
					for(size_t i = 0; i < vSel.size(); i++)
						vC.push_back({vSel[i] - 1, vSelI[i]});
					std::sort(vC.begin(), vC.end(), [](const SXCons &a, const SXCons &b) { return a.k < b.k; });
					// merge same-k constraints
					std::vector<SXCons> vM;
					for(const SXCons &c : vC)
					{
						if(!vM.empty() && vM.back().k == c.k)
						{
							vM.back().I.a = std::max(vM.back().I.a, c.I.a);
							vM.back().I.b = std::min(vM.back().I.b, c.I.b);
							if(vM.back().I.a > vM.back().I.b)
								return false;
						}
						else
							vM.push_back(c);
					}
					return XFeasible(vM, gs_Start.m_Pos.x, VMax);
				};
				std::function<void(size_t)> Dfs = [&](size_t i0) {
					int K = (int)vSel.size();
					if(K > Best.K)
					{
						Best.K = K;
						for(int i = 0; i < K; i++)
						{
							Best.aS[i] = vSel[i];
							Best.aX[i] = vSelI[i];
						}
					}
					if(K + 1 + (PB ? 1 : 0) > KMax)
						return;
					for(size_t i = i0; i < vG.size(); i++)
					{
						if(!vSel.empty() && vG[i].s < vSel.back() + 25)
							continue;
						for(int b = 0; b < vG[i].n; b++)
						{
							vSel.push_back(vG[i].s);
							vSelI.push_back(vG[i].aI[b]);
							if(Feas())
								Dfs(i + 1);
							vSel.pop_back();
							vSelI.pop_back();
						}
					}
				};
				Dfs(0);
				if(Best.K >= 1)
				{
					Best.Y = Y;
					Best.Y.AirFinal = AF;
					Best.E = E;
					// ties: prefer a higher launch point (a ground jump from up to 4 px above the standing height)
					Best.Score = Best.K + (PB ? 1 : 0) + (AF ? -0.1f : 0.0f) + 0.01f * std::clamp(gs_Start.m_Pos.y * 0 + (gs_BTop - 15.0f) - Pr.vP[iE2].y, 0.0f, 5.0f);
					vCand.push_back(Best);
				}
			}
		}
	}
	double Dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
	std::sort(vCand.begin(), vCand.end(), [](const SCand &a, const SCand &b) {
		if(a.Score != b.Score)
			return a.Score > b.Score;
		return a.E < b.E;
	});
	std::printf("screening: %zu candidates in %.1f s\n", vCand.size(), Dt);
	int Shown = 0;
	for(const SCand &C : vCand)
	{
		if(Shown++ >= ArgI("show", 20))
			break;
		std::printf("  score %.1f E %d J1 %d H %d J2 %d %s gates:", C.Score, C.E, C.Y.J1, C.Y.H, C.Y.J2, C.Y.AirFinal ? "airfinal" : "ground");
		for(int i = 0; i < C.K; i++)
			std::printf(" s%d(tau %d, x %.0f..%.0f)", C.aS[i], C.E - C.aS[i] + 1, C.aX[i].a, C.aX[i].b);
		std::printf("\n");
	}
	// realise the best candidates exactly
	std::vector<SPlan> vPlans;
	int Tried = 0;
	const int Beam = ArgI("beam", 6);
	auto T1 = std::chrono::steady_clock::now();
	for(size_t ci = 0; ci < vCand.size() && Tried < Top; ci++)
	{
		const SCand &C = vCand[ci];
		if(C.Score < vCand[0].Score - ArgF("slack", 0.5f))
			break;
		Tried++;
		SPlan P;
		bool Ok = RealizeCand(C, P, Beam);
		if(!P.vIn.empty() && Verbose)
			std::printf("realised %s %s\n", Ok ? "ok " : "BAD", P.Desc.c_str());
		else if(Verbose > 1)
			std::printf("realise failed: E %d J1 %d\n", C.E, C.Y.J1);
		if(Ok && !P.vIn.empty())
			vPlans.push_back(P);
	}
	std::printf("realisation: %d tried, %zu plans in %.1f s\n", Tried, vPlans.size(), std::chrono::duration<double>(std::chrono::steady_clock::now() - T1).count());
	std::sort(vPlans.begin(), vPlans.end(), [](const SPlan &a, const SPlan &b) { return a.Apex > b.Apex; });
	for(int i = 0; i < NOut && i < (int)vPlans.size(); i++)
	{
		std::vector<STasInput> vAll = gs_vPrefix;
		vAll.insert(vAll.end(), vPlans[i].vIn.begin(), vPlans[i].vIn.end());
		char aPath[512];
		str_format(aPath, sizeof(aPath), "%s%d.txt", argv[3], i);
		WriteInputFile(aPath, vAll);
		std::printf("best %d: %s -> %s\n", i, vPlans[i].Desc.c_str(), aPath);
	}
	(void)Verbose;
	(void)StartGround;
	return 0;
}
