// x_tig: Teero-input guided tracking search. A reference run is known only from a video: per-frame inputs read from it
// (direction, jump, cursor aim, shots with their aim, each with a certainty) and the tee's tracked position (label per
// tick, +-10 px). This search reproduces it in the exact stepper: the beam follows the tracked positions in time
// (label = our race tick + off) and its branching is cut down to what the video allows: jumps and shots only near the
// ticks they were seen, new hooks aimed where the cursor was (a few degrees around it, plus a coarse ring), shots
// aimed where they were seen (fine perturbations; a full ring when the aim was not readable). Dir is free.
// usage: x_tig <map> <prefix> csv=FILE track=FILE off=11.5 [beam=3000] [cap=160] [tolj=1] [tolf=1] [ring=16]
//        [pert=3,6] [fpert=0.5,1,2,4] [fring=64] [qp=2] [qv=0.5] [maxt=2700] [threads=4] [out=PATH] [every=25]
//        [lagw=0] (adds lagw * ticks behind the reference line, by projection, to the tracking cost)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "fastg.h"
#include "sim.h"
#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

static std::vector<STasInput> ReadInputs(const char *pPath)
{
	std::vector<STasInput> v;
	FILE *f = std::fopen(pPath, "r");
	int d, j, h, fi, tx, ty, w;
	char aLine[256];
	while(f && std::fgets(aLine, sizeof(aLine), f))
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
	if(f)
		std::fclose(f);
	return v;
}

static void WriteInputs(const char *pPath, const std::vector<STasInput> &v)
{
	FILE *f = std::fopen(pPath, "w");
	if(!f)
		return;
	for(const STasInput &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

// per label (tick of the reference) what the video shows
struct SRefTick
{
	bool m_Seen = false;
	int m_Dir = 0;
	bool m_Jump = false;
	float m_Aim = 0; // degrees
	float m_AimCert = 0;
	bool m_Fire = false;
	float m_FireAim = 0;
	float m_FireCert = 0;
};

static std::vector<std::string> SplitCsv(const std::string &s)
{
	std::vector<std::string> v;
	std::string c;
	for(char ch : s)
	{
		if(ch == ',')
		{
			v.push_back(c);
			c.clear();
		}
		else if(ch != '\r' && ch != '\n')
			c += ch;
	}
	v.push_back(c);
	return v;
}

static float ToF(const std::string &s, float d = 0) { return s.empty() ? d : std::strtof(s.c_str(), nullptr); }

struct SNode
{
	CFastG m_G;
	STasInput m_In;
	int m_Parent;
	float m_Cost;
	float m_Rank;
	int m_PendTau = -1; // reserved shot: fired from m_PendPos in the step from tick m_PendTau, aim decided at its explosion
	vec2 m_PendPos = vec2(0, 0);
	int m_PatchTau = -1; // this step resolved the reserved shot: fire + aim go into the input of the step from that tick
	int16_t m_PatchTX = 0, m_PatchTY = 0;
};

// retro shot: a grenade fired from APos in the step from tick Tau that explodes in the step from G (tick t) to t+1 at a
// solid point near the tee (as in x_ds: aim solved per target point, flight verified exactly)
struct SRetro
{
	int16_t m_TX, m_TY;
	vec2 m_P0, m_Dir, m_Col, m_Kick;
	int m_Tau = -1;
};

static void RetroFind(const CFastG &G, int Tau, vec2 APos, int K, std::vector<SRetro> &vOut)
{
	vOut.clear();
	if(G.m_NumProj >= CFastG::MAX_PROJ)
		return;
	const SMapInfo &M = CTasGame::Map();
	const vec2 Q = G.m_Pos;
	const int TauT = G.m_Tick + 1 - Tau;
	if(TauT < 2 || TauT > 99)
		return;
	vec2 aE[48];
	int NE = 0;
	for(int a = 0; a < 48; a++)
	{
		const float Ang = 2 * pi * a / 48;
		const vec2 D(std::cos(Ang), std::sin(Ang));
		for(float r = 6.0f; r < 100.0f; r += 3.0f)
		{
			vec2 P = Q + D * r;
			if(M.Tile((int)std::floor(P.x / 32), (int)std::floor(P.y / 32)) == TILE_SOLID)
			{
				aE[NE++] = P;
				break;
			}
		}
	}
	const float Curv = G.m_Core.m_Tuning.m_GrenadeCurvature, Speed = G.m_Core.m_Tuning.m_GrenadeSpeed;
	const float Vg = Speed / SERVER_TICK_SPEED, Cg = Curv / 10000.0f * Vg * Vg, R0 = CCharacterCore::PhysicalSize() * 0.75f;
	const float Strength = G.m_Core.m_Tuning.m_ExplosionStrength;
	std::vector<SRetro> vC;
	for(int e = 0; e < NE; e++)
	{
		const vec2 W = aE[e] - APos;
		auto F = [&](float T2) { return length(vec2(W.x, W.y - Cg * T2 * T2)) - R0 - Vg * T2; };
		float Lo = TauT - 1.0f, Hi = (float)TauT;
		if(!(F(Lo) > 0 && F(Hi) <= 0))
			continue;
		for(int it = 0; it < 22; it++)
		{
			float Mid = 0.5f * (Lo + Hi);
			if(F(Mid) > 0)
				Lo = Mid;
			else
				Hi = Mid;
		}
		vec2 Dn = normalize(vec2(W.x, W.y - Cg * Hi * Hi));
		int16_t TX = (int16_t)std::lround(Dn.x * 10000), TY = (int16_t)std::lround(Dn.y * 10000);
		if(!TX && !TY)
			TY = -1;
		const vec2 Dir = normalize(vec2(TX, TY));
		const vec2 P0 = APos + Dir * R0;
		bool Ok = false;
		vec2 Col;
		for(int T2 = 1; T2 <= TauT; T2++)
		{
			vec2 Prev = CalcPos(P0, Dir, Curv, Speed, (T2 - 1) / (float)SERVER_TICK_SPEED);
			vec2 Cur = CalcPos(P0, Dir, Curv, Speed, T2 / (float)SERVER_TICK_SPEED);
			if(Cur.x < 0 || Cur.y < 0 || Cur.x >= M.m_W * 32 || Cur.y >= M.m_H * 32)
				break;
			vec2 NewPos;
			if(CTasGame::Collision()->IntersectLine(Prev, Cur, &Col, &NewPos))
			{
				Ok = T2 == TauT;
				break;
			}
		}
		if(!Ok || !(distance(Q, Col) < 135.0f + CCharacterCore::PhysicalSize()))
			continue;
		vec2 Diff = Q - Col;
		float l = length(Diff);
		vec2 Fd = l ? normalize(Diff) : vec2(0, 1);
		l = 1 - std::clamp((l - 48.0f) / (135.0f - 48.0f), 0.0f, 1.0f);
		float Dmg = Strength * l;
		if(!(int)Dmg)
			continue;
		bool Dup = false;
		for(auto &C : vC)
			if(distance(C.m_Col, Col) < 6.0f)
				Dup = true;
		if(Dup)
			continue;
		vC.push_back({TX, TY, P0, Dir, Col, Fd * Dmg * 2, Tau});
	}
	std::sort(vC.begin(), vC.end(), [](const SRetro &a, const SRetro &b) { return length(a.m_Kick) > length(b.m_Kick); });
	for(int k = 0; k < (int)vC.size() && k < K; k++)
		vOut.push_back(vC[k]);
}

static void ApplyRetro(CFastG &G, int Tau, const SRetro &R)
{
	int At = 0;
	while(At < G.m_NumProj && G.m_aProj[At].m_StartTick > Tau)
		At++;
	for(int k = G.m_NumProj; k > At; k--)
		G.m_aProj[k] = G.m_aProj[k - 1];
	G.m_NumProj++;
	SFastProj &P = G.m_aProj[At];
	P.m_Pos = R.m_P0;
	P.m_Dir = R.m_Dir;
	P.m_StartTick = Tau;
	P.m_LifeSpan = 100 - (G.m_Tick - Tau);
	G.m_ReloadTimer = std::max(0, 25 - (G.m_Tick - Tau));
	// the patched input pressed fire in the step from Tau and released it in the next one (if already done)
	int Add = (Tau + 2 <= G.m_Tick) ? 2 : 1;
	G.m_Fire += Add;
	G.m_Input.m_Fire += Add;
	G.m_LatestInput.m_Fire += Add;
	G.m_LatestPrevInput.m_Fire += Add;
	G.m_Core.m_Input.m_Fire += Add;
}

int main(int argc, const char **argv)
{
	if(argc < 3 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: x_tig <map> <prefix> csv=FILE track=FILE off=11.5 ...\n");
		return 1;
	}
	CFastG::Init();
	std::map<std::string, std::string> Kv;
	for(int i = 3; i < argc; i++)
	{
		const char *e = std::strchr(argv[i], '=');
		if(e)
			Kv[std::string(argv[i], e - argv[i])] = e + 1;
	}
	auto Get = [&](const char *k, const char *d) { return Kv.count(k) ? Kv[k] : std::string(d); };
	auto List = [&](const char *k, const char *d) {
		std::vector<float> v;
		std::stringstream ss(Get(k, d));
		std::string t;
		while(std::getline(ss, t, ','))
			if(!t.empty())
				v.push_back(std::stof(t));
		return v;
	};
	const float Off = std::stof(Get("off", "11.5"));
	const int Beam = std::stoi(Get("beam", "3000"));
	const float Cap = std::stof(Get("cap", "160"));
	const int TolJ = std::stoi(Get("tolj", "1")), TolF = std::stoi(Get("tolf", "1"));
	const int Ring = std::stoi(Get("ring", "16")), FRing = std::stoi(Get("fring", "64"));
	const std::vector<float> vPert = List("pert", "3,6"), vFPert = List("fpert", "0.5,1,2,4");
	const float Qp = std::stof(Get("qp", "2")), Qv = std::stof(Get("qv", "0.5"));
	const int MaxT = std::stoi(Get("maxt", "2700"));
	const int NumThreads = std::stoi(Get("threads", "4"));
	const int Every = std::stoi(Get("every", "25"));
	const std::string Out = Get("out", "tig.txt");
	const float LagW = std::stof(Get("lagw", "0"));
	const int RetroK = std::stoi(Get("retro", "6"));
	const int DebugRt = std::stoi(Get("debug", "-1"));
	const int PendWin = std::stoi(Get("pendwin", "4"));
	const int Quota = std::stoi(Get("quota", "4"));
	const float CellP = std::stof(Get("cellp", "6")), CellV = std::stof(Get("cellv", "1.5"));
	const float Warp = std::stof(Get("warp", "3")), WarpPen = std::stof(Get("warppen", "3")), Dz = std::stof(Get("dz", "8"));
	const float PendB = std::stof(Get("pendb", "20")), FreeB = std::stof(Get("freeb", "10"));

	// reference inputs per label
	std::vector<SRefTick> vRef(6000);
	{
		FILE *f = std::fopen(Get("csv", "").c_str(), "r");
		if(!f)
		{
			std::printf("cannot read csv\n");
			return 1;
		}
		char aLine[4096];
		std::fgets(aLine, sizeof(aLine), f);
		std::vector<std::string> vH = SplitCsv(aLine);
		auto Col = [&](const char *n) {
			for(size_t i = 0; i < vH.size(); i++)
				if(vH[i] == n)
					return (int)i;
			return -1;
		};
		int cS = Col("s_since_start"), cDir = Col("dir"), cJ = Col("jump_arrow"), cA = Col("aim_angle_deg"), cAC = Col("aim_certainty"), cF = Col("fire"),
		    cFA = Col("fire_aim_deg"), cFC = Col("fire_aim_certainty"), cW = Col("weapon");
		while(std::fgets(aLine, sizeof(aLine), f))
		{
			std::vector<std::string> v = SplitCsv(aLine);
			if((int)v.size() <= std::max({cS, cDir, cJ, cA, cAC, cF, cFA, cFC, cW}))
				continue;
			float c = ToF(v[cS]) * 50;
			int L = (int)std::lround(c);
			if(L < 0 || L >= (int)vRef.size())
				continue;
			SRefTick &R = vRef[L];
			R.m_Seen = true;
			R.m_Dir = (int)ToF(v[cDir]);
			if(ToF(v[cJ]) > 0.5f)
				R.m_Jump = true;
			float ac = ToF(v[cAC]);
			if(ac >= R.m_AimCert)
			{
				R.m_AimCert = ac;
				R.m_Aim = ToF(v[cA]);
			}
			if(ToF(v[cF]) > 0.5f && v[cW] == "grenade")
			{
				R.m_Fire = true;
				R.m_FireAim = ToF(v[cFA]);
				R.m_FireCert = ToF(v[cFC]);
			}
		}
		std::fclose(f);
	}
	// reference positions per label (smoothed +-1)
	std::vector<vec2> vTP(6000, vec2(0, 0));
	std::vector<bool> vTOk(6000, false);
	{
		FILE *f = std::fopen(Get("track", "").c_str(), "r");
		if(!f)
		{
			std::printf("cannot read track\n");
			return 1;
		}
		std::map<int, vec2> Raw;
		int k;
		float x, y;
		char aLine[256];
		while(std::fgets(aLine, sizeof(aLine), f))
			if(std::sscanf(aLine, "%d %f %f", &k, &x, &y) == 3 && k >= 0 && k < 6000)
				Raw[k] = vec2(x, y);
		std::fclose(f);
		for(auto &[L, P] : Raw)
		{
			vec2 S(0, 0);
			int n = 0;
			for(int d = -1; d <= 1; d++)
				if(Raw.count(L + d))
				{
					S += Raw[L + d];
					n++;
				}
			vTP[L] = S / (float)n;
			vTOk[L] = true;
		}
	}
	auto RefPos = [&](float L, vec2 &P) {
		int a = (int)std::floor(L);
		if(a < 0 || a + 1 >= 6000 || !vTOk[a] || !vTOk[a + 1])
			return false;
		float f = L - a;
		P = vTP[a] + (vTP[a + 1] - vTP[a]) * f;
		return true;
	};
	// progress on the reference line by projection near the expected label (for lagw)
	auto RefProj = [&](vec2 Q, float L0) {
		float BestD = 1e18f, BestL = L0;
		for(int a = (int)L0 - 40; a <= (int)L0 + 40; a++)
		{
			if(a < 0 || a + 1 >= 6000 || !vTOk[a] || !vTOk[a + 1])
				continue;
			vec2 A = vTP[a], B = vTP[a + 1];
			vec2 AB = B - A;
			float l2 = dot(AB, AB);
			float f = l2 > 0 ? std::clamp(dot(Q - A, AB) / l2, 0.0f, 1.0f) : 0;
			float d = distance(Q, A + AB * f);
			if(d < BestD)
			{
				BestD = d;
				BestL = a + f;
			}
		}
		return BestL;
	};

	std::vector<STasInput> vPre = ReadInputs(argv[2]);
	CTasGame Gm;
	Gm.Spawn(CTasGame::Map().m_vSpawns[0]);
	size_t i = 0;
	for(; i < vPre.size() && !Gm.HasGrenade(); i++)
		Gm.Step(vPre[i]);
	CFastG G0;
	G0.FromGame(Gm);
	for(; i < vPre.size(); i++)
		G0.Step(vPre[i]);
	{
		vec2 P;
		RefPos(G0.RaceTick() + Off, P);
		std::printf("start rt %d pos %.0f %.0f vel %.2f %.2f; reference label %.1f at %.0f %.0f (d %.1f)\n", G0.RaceTick(), G0.m_Pos.x, G0.m_Pos.y,
			G0.m_Core.m_Vel.x, G0.m_Core.m_Vel.y, G0.RaceTick() + Off, P.x, P.y, distance(P, G0.m_Pos));
	}

	auto AimTo = [](float Deg, STasInput &In) {
		In.m_TX = (int)std::lround(std::cos(Deg * pi / 180) * 10000);
		In.m_TY = (int)std::lround(std::sin(Deg * pi / 180) * 10000);
	};

	std::vector<std::vector<SNode>> vL;
	vL.push_back({{G0, vPre.empty() ? STasInput() : vPre.back(), -1, 0, 0}});
	int BestFin = -1, BestFinIdx = -1, FinLayer = -1;
	for(int t = 0; G0.RaceTick() + t < MaxT; t++)
	{
		const std::vector<SNode> &L = vL[t];
		const int Rt = L[0].m_G.RaceTick(); // all nodes share the tick
		const float Lab = Rt + 1 + Off; // label of the state after this step
		// reference facts near this step
		int LabI = (int)std::lround(Lab);
		bool JumpNear = false, FireNear = false;
		std::vector<float> vHookAim, vFireAim;
		bool FireUnsure = false;
		for(int d = -std::max(TolJ, TolF) - 1; d <= std::max(TolJ, TolF) + 1; d++)
		{
			int k = LabI + d;
			if(k < 0 || k >= (int)vRef.size())
				continue;
			const SRefTick &R = vRef[k];
			if(R.m_Jump && std::abs(d) <= TolJ)
				JumpNear = true;
			if(R.m_Fire && std::abs(d) <= TolF)
			{
				FireNear = true;
				if(R.m_FireCert >= 0.5f)
					vFireAim.push_back(R.m_FireAim);
				else
					FireUnsure = true;
			}
			if(std::abs(d) <= 1 && R.m_AimCert >= 0.5f)
			{
				vHookAim.push_back(R.m_Aim);
				if(R.m_Fire)
					vFireAim.push_back(R.m_Aim);
			}
		}
		std::vector<float> vHA; // hook press aims
		for(float a : vHookAim)
		{
			vHA.push_back(a);
			for(float p : vPert)
			{
				vHA.push_back(a + p);
				vHA.push_back(a - p);
			}
		}
		for(int r = 0; r < Ring; r++)
			vHA.push_back(360.0f * r / Ring);
		std::vector<float> vFA; // shot aims: the seen ones (with perturbations), then a ring (point-blank only)
		size_t NSeenFA = 0;
		if(FireNear)
		{
			for(float a : vFireAim)
			{
				vFA.push_back(a);
				for(float p : vFPert)
				{
					vFA.push_back(a + p);
					vFA.push_back(a - p);
				}
			}
			NSeenFA = vFA.size();
			(void)FireUnsure;
			for(int r = 0; r < FRing; r++)
				vFA.push_back(360.0f * r / FRing);
		}
		vec2 TP;
		bool HaveRef = RefPos(Lab, TP);

		if(Rt == DebugRt)
		{
			int NP = 0;
			for(const SNode &Nd : L)
				if(Nd.m_PendTau >= 0)
				{
					if(NP < 5)
					{
						std::vector<SRetro> vR;
						RetroFind(Nd.m_G, Nd.m_PendTau, Nd.m_PendPos, 50, vR);
						std::printf("debug rt %d: pending tau rt %d from %.0f %.0f, tee %.0f %.0f reload %d: %zu retro", Rt, Nd.m_PendTau - Nd.m_G.m_StartTick,
							Nd.m_PendPos.x, Nd.m_PendPos.y, Nd.m_G.m_Pos.x, Nd.m_G.m_Pos.y, Nd.m_G.m_ReloadTimer, vR.size());
						for(auto &r : vR)
							std::printf(" [%.0f,%.0f k %.1f]", r.m_Col.x, r.m_Col.y, length(r.m_Kick));
						std::printf("\n");
					}
					NP++;
				}
			std::printf("debug rt %d: %d/%zu nodes pending; fire near %d jump near %d; seen fire aims %zu of %zu\n", Rt, NP, L.size(), (int)FireNear, (int)JumpNear, NSeenFA, vFA.size());
			for(int q = 0; q < 3 && q < (int)L.size(); q++)
			{
				const SNode &Nd = L[q];
				int NPb = 0;
				float BestV = 0;
				for(size_t a = 0; a < vFA.size(); a++)
				{
					STasInput F = Nd.m_In;
					F.m_Fire = 1;
					F.m_Hook = 0;
					AimTo(vFA[a], F);
					CFastG G = Nd.m_G;
					G.Step(F);
					bool Alive = false;
					for(int p2 = 0; p2 < G.m_NumProj; p2++)
						if(G.m_aProj[p2].m_StartTick == Nd.m_G.m_Tick)
							Alive = true;
					if(!Alive)
					{
						NPb++;
						BestV = std::max(BestV, length(G.m_Core.m_Vel));
					}
				}
				std::printf("  node %d: pos %.0f %.0f v %.1f %.1f reload %d pend %d rank %.1f cost %.1f: %d point-blank aims, best |v| %.1f\n", q, Nd.m_G.m_Pos.x, Nd.m_G.m_Pos.y,
					Nd.m_G.m_Core.m_Vel.x, Nd.m_G.m_Core.m_Vel.y, Nd.m_G.m_ReloadTimer, Nd.m_PendTau >= 0 ? Nd.m_PendTau - Nd.m_G.m_StartTick : -1, Nd.m_Rank, Nd.m_Cost, NPb, BestV);
			}
		}
		std::vector<std::vector<SNode>> vPart(NumThreads);
		std::atomic<int> Next(0);
		auto Work = [&](int Th) {
			std::vector<SNode> &R = vPart[Th];
			std::vector<STasInput> vC;
			while(true)
			{
				int k = Next++;
				if(k >= (int)L.size())
					break;
				const SNode &Nd = L[k];
				const STasInput &P = Nd.m_In;
				vC.clear();
				const int T = Nd.m_G.m_Tick;
				const bool Pend = Nd.m_PendTau >= 0 && T - Nd.m_PendTau <= 95;
				const bool CanFire = Nd.m_G.m_ReloadTimer == 0 && FireNear; // a direct shot drops a reserved slot
				// plain inputs (and, at a reference shot, reserve the slot: no hook press, the shot's aim comes later)
				std::vector<STasInput> vNoPress;
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int J = 0; J < 2; J++)
					{
						if(J && (!JumpNear || P.m_Jump))
							continue;
						STasInput In = P;
						In.m_Dir = Dir;
						In.m_Jump = J;
						In.m_Fire = 0;
						In.m_Weapon = -1;
						if(P.m_Hook)
						{
							vC.push_back(In);
							vNoPress.push_back(In);
							In.m_Hook = 0;
							vC.push_back(In);
							vNoPress.push_back(In);
						}
						else
						{
							In.m_Hook = 0;
							vC.push_back(In);
							vNoPress.push_back(In);
							for(float a : vHA)
							{
								In.m_Hook = 1;
								AimTo(a, In);
								vC.push_back(In);
							}
						}
					}
				auto Emit = [&](CFastG &G, const STasInput &In, int PendTau, vec2 PendPos, int PatchTau, int16_t PTX, int16_t PTY) {
					if(G.m_Dead || G.m_Bad)
						return;
					float d = 0;
					if(HaveRef)
					{
						// distance to the reference within +-Warp labels of its clock (charged per label), dead zone Dz
						d = 1e9f;
						for(float w = -Warp; w <= Warp + 1e-3f; w += 0.5f)
						{
							vec2 RP;
							if(RefPos(Lab + w, RP))
								d = std::min(d, distance(G.m_Pos, RP) + WarpPen * std::abs(w));
						}
						d = std::min(std::max(0.0f, d - Dz), Cap);
					}
					float Cost = Nd.m_Cost + d * d / 100.0f;
					float Rank = Cost - (PendTau >= 0 ? PendB : 0) - (G.m_ReloadTimer == 0 && PendTau < 0 ? FreeB : 0);
					SNode C{G, In, k, Cost, Rank};
					C.m_PendTau = PendTau;
					C.m_PendPos = PendPos;
					C.m_PatchTau = PatchTau;
					C.m_PatchTX = PTX;
					C.m_PatchTY = PTY;
					R.push_back(std::move(C));
				};
				const int KeepTau = Pend ? Nd.m_PendTau : -1;
				for(const STasInput &In : vC)
				{
					CFastG G = Nd.m_G;
					G.Step(In);
					Emit(G, In, KeepTau, Nd.m_PendPos, -1, 0, 0);
				}
				if(CanFire)
				{
					// reserve the slot
					for(const STasInput &In : vNoPress)
					{
						if(Pend)
							break;
						CFastG G = Nd.m_G;
						G.Step(In);
						Emit(G, In, T, Nd.m_G.m_Pos, -1, 0, 0);
					}
					// direct shots: the seen aims (any outcome), the ring only as point-blank (explodes in this step)
					for(size_t a = 0; a < vFA.size(); a++)
						for(const STasInput &B : vNoPress)
						{
							STasInput F = B;
							F.m_Fire = 1;
							AimTo(vFA[a], F);
							CFastG G = Nd.m_G;
							G.Step(F);
							if(a >= NSeenFA)
							{
								bool Alive = false;
								for(int q = 0; q < G.m_NumProj; q++)
									if(G.m_aProj[q].m_StartTick == T)
										Alive = true;
								if(Alive)
									continue;
							}
							Emit(G, F, -1, vec2(0, 0), -1, 0, 0);
						}
				}
				if(Pend)
				{
					// resolve the reserved shot now (explodes in this step): any tick of the reserved window whose
					// input pressed no hook can be the shot's (positions from the lineage)
					std::vector<SRetro> vR;
					{
						std::vector<std::pair<int, vec2>> vTau;
						int j = t, ix = k;
						while(j > 0)
						{
							const SNode &A = vL[j][ix];
							const int TauA = G0.m_Tick + j - 1; // A's input is the step from this tick
							if(TauA < Nd.m_PendTau)
								break;
							const SNode &Pa = vL[j - 1][A.m_Parent];
							bool Press = A.m_In.m_Hook && !Pa.m_In.m_Hook;
							if(TauA <= Nd.m_PendTau + PendWin && !Press && !A.m_In.m_Fire)
								vTau.push_back({TauA, Pa.m_G.m_Pos});
							ix = A.m_Parent;
							j--;
						}
						// the node itself (its step from tick T is not taken yet: no)
						std::vector<SRetro> vOne;
						for(auto &[Ta, Ap] : vTau)
						{
							RetroFind(Nd.m_G, Ta, Ap, RetroK, vOne);
							for(auto &r : vOne)
							{
								SRetro r2 = r;
								r2.m_Tau = Ta;
								vR.push_back(r2);
							}
						}
						std::sort(vR.begin(), vR.end(), [](const SRetro &a, const SRetro &b) { return length(a.m_Kick) > length(b.m_Kick); });
						if((int)vR.size() > RetroK)
							vR.resize(RetroK);
					}
					for(const SRetro &Rr : vR)
						for(const STasInput &In : vNoPress)
						{
							if(In.m_Jump)
								continue;
							CFastG G = Nd.m_G;
							ApplyRetro(G, Rr.m_Tau, Rr);
							const bool Stack = FireNear && T - Rr.m_Tau >= 25 && G.m_ReloadTimer == 0;
							CFastG G1 = G;
							STasInput I2 = In;
							G1.Step(I2);
							Emit(G1, I2, -1, vec2(0, 0), Rr.m_Tau, Rr.m_TX, Rr.m_TY);
							// stacked: a point-blank / seen shot in the same step as the lob's explosion
							if(Stack)
								for(size_t a = 0; a < vFA.size(); a++)
								{
									STasInput F = In;
									F.m_Fire = 1;
									AimTo(vFA[a], F);
									CFastG G2 = G;
									G2.Step(F);
									if(a >= NSeenFA)
									{
										bool Alive = false;
										for(int q = 0; q < G2.m_NumProj; q++)
											if(G2.m_aProj[q].m_StartTick == T)
												Alive = true;
										if(Alive)
											continue;
									}
									Emit(G2, F, -1, vec2(0, 0), Rr.m_Tau, Rr.m_TX, Rr.m_TY);
								}
						}
				}
			}
		};
		{
			std::vector<std::thread> vT;
			for(int Th = 0; Th < NumThreads; Th++)
				vT.emplace_back(Work, Th);
			for(auto &Th : vT)
				Th.join();
		}
		std::vector<SNode> vAll;
		for(auto &P : vPart)
			for(auto &Nd : P)
				vAll.push_back(std::move(Nd));
		if(LagW > 0)
			for(SNode &Nd : vAll)
				Nd.m_Rank += LagW * (Lab - RefProj(Nd.m_G.m_Pos, Lab)) * 100.0f; // signed: ahead of the reference is good
		// finish?
		for(size_t k = 0; k < vAll.size(); k++)
			if(vAll[k].m_G.m_FinishTick >= 0)
			{
				if(BestFin < 0 || vAll[k].m_Rank < vAll[BestFinIdx].m_Rank)
				{
					BestFin = vAll[k].m_G.m_FinishTick - vAll[k].m_G.m_StartTick;
					BestFinIdx = k;
				}
			}
		std::sort(vAll.begin(), vAll.end(), [](const SNode &a, const SNode &b) { return a.m_Rank < b.m_Rank; });
		if(BestFin >= 0)
		{
			// the finishing node is the best-ranked finisher (re-find after the sort)
			for(size_t k = 0; k < vAll.size(); k++)
				if(vAll[k].m_G.m_FinishTick >= 0)
				{
					BestFinIdx = k;
					break;
				}
		}
		std::vector<SNode> vU;
		std::unordered_map<uint64_t, int> Seen;
		std::unordered_map<uint64_t, int> CellCnt;
		std::vector<size_t> vOver;
		for(SNode &Nd : vAll)
		{
			const CFastG &G = Nd.m_G;
			auto Q = [](float v, float q) { return (uint64_t)(int64_t)std::lround(v / q) & 0xffff; };
			uint64_t K = Q(G.m_Pos.x, Qp) | Q(G.m_Pos.y, Qp) << 16 | Q(G.m_Core.m_Vel.x, Qv) << 32 | Q(G.m_Core.m_Vel.y, Qv) << 48;
			int Hs = G.m_Core.m_HookState;
			uint64_t K2 = (uint64_t)(Hs + 2) * 1000003ull +
				      (Hs == HOOK_GRABBED || Hs == HOOK_FLYING ? (uint64_t)(G.m_Core.m_HookPos.x / 4) * 7919 + (uint64_t)(G.m_Core.m_HookPos.y / 4) : 0) +
				      (uint64_t)G.m_Core.m_Jumped * 31 + (Nd.m_In.m_Hook ? 17 : 0) + (uint64_t)G.m_ReloadTimer * 131071 + (uint64_t)(Nd.m_PendTau + 7) * 2246822519ull;
			for(int p = 0; p < G.m_NumProj; p++)
				K2 = K2 * 1315423911ull + (uint64_t)G.m_aProj[p].m_StartTick * 2654435761ull + (uint64_t)(int64_t)std::lround(std::atan2(G.m_aProj[p].m_Dir.y, G.m_aProj[p].m_Dir.x) * 2000);
			K ^= K2 * 0x9E3779B97F4A7C15ull;
			if(!Seen.emplace(K, 0).second)
				continue;
			// diversity: at most Quota states per coarse cell in the first pass, the rest fill up afterwards
			uint64_t C = Q(G.m_Pos.x, CellP) | Q(G.m_Pos.y, CellP) << 16 | Q(G.m_Core.m_Vel.x, CellV) << 32 | Q(G.m_Core.m_Vel.y, CellV) << 48;
			if(Quota > 0 && CellCnt[C]++ >= Quota)
			{
				vOver.push_back(&Nd - vAll.data());
				continue;
			}
			vU.push_back(std::move(Nd));
			if((int)vU.size() >= Beam)
				break;
		}
		for(size_t o = 0; o < vOver.size() && (int)vU.size() < Beam; o++)
			vU.push_back(std::move(vAll[vOver[o]]));
		std::stable_sort(vU.begin(), vU.end(), [](const SNode &a, const SNode &b) { return a.m_Rank < b.m_Rank; });
		if(BestFin >= 0)
		{
			std::printf("FINISH rt %d\n", BestFin);
			vL.push_back(std::move(vAll)); // keep the layer for the backtrack (BestFinIdx indexes vAll)
			FinLayer = t + 1;
			break;
		}
		if(vU.empty())
		{
			std::printf("rt %d: beam died\n", Rt + 1);
			FinLayer = -1;
			// write the best path of the previous layer for inspection
			vL.push_back({});
			break;
		}
		if((Rt + 1) % Every == 0)
		{
			const SNode &B = vU[0];
			std::printf("rt %d label %.1f: beam %zu/%zu cost %.0f pos %.0f %.0f ref %.0f %.0f d %.0f |v| %.1f proj %.1f\n", Rt + 1, Lab, vU.size(), vAll.size(), B.m_Cost,
				B.m_G.m_Pos.x, B.m_G.m_Pos.y, TP.x, TP.y, distance(B.m_G.m_Pos, TP), length(B.m_G.m_Core.m_Vel), RefProj(B.m_G.m_Pos, Lab));
			std::fflush(stdout);
		}
		vL.push_back(std::move(vU));
	}
	// backtrack: the finisher, else the best of the last non-empty layer
	int Layer = FinLayer > 0 ? FinLayer : (int)vL.size() - 1;
	while(Layer > 0 && vL[Layer].empty())
		Layer--;
	int Idx = FinLayer > 0 ? BestFinIdx : 0;
	std::vector<STasInput> vPath;
	std::vector<std::pair<int, std::pair<int16_t, int16_t>>> vPatch;
	for(int t = Layer; t > 0; t--)
	{
		const SNode &N = vL[t][Idx];
		vPath.push_back(N.m_In);
		if(N.m_PatchTau >= 0)
			vPatch.push_back({N.m_PatchTau, {N.m_PatchTX, N.m_PatchTY}});
		Idx = N.m_Parent;
	}
	std::reverse(vPath.begin(), vPath.end());
	for(auto &[Tau, A] : vPatch)
	{
		int ix = Tau - G0.m_Tick; // the step from tick Tau
		if(ix >= 0 && ix < (int)vPath.size())
		{
			vPath[ix].m_Fire = 1;
			vPath[ix].m_TX = A.first;
			vPath[ix].m_TY = A.second;
		}
	}
	std::vector<STasInput> vAllIn = vPre;
	vAllIn.insert(vAllIn.end(), vPath.begin(), vPath.end());
	WriteInputs(Out.c_str(), vAllIn);
	// verify on the exact game
	CTasGame V;
	V.Spawn(CTasGame::Map().m_vSpawns[0]);
	for(const STasInput &In : vAllIn)
		V.Step(In);
	std::printf("wrote %s (%zu inputs, last layer rt %d); finish %d\n", Out.c_str(), vAllIn.size(), G0.RaceTick() + Layer, BestFin);
	return 0;
}
