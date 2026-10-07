// x_tig: Teero-input guided tracking search. A reference run is known only from a video: per-frame inputs read from it
// (direction, jump, cursor aim, shots with their aim, each with a certainty) and the tee's tracked position (label per
// tick, +-10 px). This search reproduces it in the exact stepper: the beam follows the tracked positions in time
// (label = our race tick + off) and its branching is cut down to what the video allows: jumps and shots only near the
// ticks they were seen, new hooks aimed where the cursor was (a few degrees around it, plus a coarse ring), shots
// aimed where they were seen (fine perturbations; a full ring when the aim was not readable). Dir is free.
// usage: x_tig <map> <prefix> csv=FILE track=FILE off=11.5 [beam=3000] [cap=160] [tolj=1] [tolf=1] [ring=16]
//        [pert=3,6] [fpert=0.5,1,2,4] [fring=64] [qp=2] [qv=0.5] [maxt=2700] [threads=4] [out=PATH] [every=25]
//        [lagw=0] (adds lagw * ticks behind the reference line, by projection, to the tracking cost)
//        [hooks=FILE tolh=1] his hook timeline from the video (teero/hooks/teero_hooks.csv: k,state,anchor_x,anchor_y
//        on the track's clock; state G grabbed, F flying, - none): the hook is held / pressed only where he hooks
//        (within tolh ticks), released where he does not, and new hooks aim at his anchor (+-0.5..4 deg)
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
	float m_Lab = 0; // place on the reference line (label, by projection)
	float m_ShI = 0; // place on the shadow path (step index)
	bool m_Sh = false; // made by one of the shadow run's own inputs (kept past the quota, see ShKeep)
	int m_PendTau = -1; // reserved shot: fired from m_PendPos in the step from tick m_PendTau, aim decided at its explosion
	vec2 m_PendPos = vec2(0, 0);
	int m_PatchTau = -1; // this step resolved the reserved shot: fire + aim go into the input of the step from that tick
	int16_t m_PatchTX = 0, m_PatchTY = 0;
};

struct SHint
{
	bool m_JumpNear = false, m_FireNear = false;
	std::vector<float> m_vHA, m_vFA;
	size_t m_NSeenFA = 0;
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
	// his hook timeline (video): per label state and anchor
	const std::string HooksFile = Get("hooks", "");
	const bool HaveHooks = !HooksFile.empty();
	const int TolH = std::stoi(Get("tolh", "1"));
	std::vector<char> vHkS(6000, 0);
	std::vector<vec2> vHkA(6000, vec2(0, 0));
	if(HaveHooks)
	{
		FILE *f = std::fopen(HooksFile.c_str(), "r");
		if(!f)
		{
			std::printf("cannot read hooks\n");
			return 1;
		}
		char aLine[512];
		std::fgets(aLine, sizeof(aLine), f);
		int N = 0;
		while(std::fgets(aLine, sizeof(aLine), f))
		{
			std::vector<std::string> v = SplitCsv(aLine);
			if(v.size() < 4)
				continue;
			int k = std::atoi(v[0].c_str());
			if(k < 0 || k >= 6000 || v[1].empty())
				continue;
			vHkS[k] = v[1][0];
			if(v[1][0] == 'G' && !v[2].empty())
				vHkA[k] = vec2(ToF(v[2]), ToF(v[3]));
			N++;
		}
		std::fclose(f);
		std::printf("hooks: %d labels from %s (tolh %d)\n", N, HooksFile.c_str(), TolH);
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

	// projection onto the reference line within a label window; distance out
	auto RefProjW = [&](vec2 Q, float L0, float L1, float *pDist) {
		float BestD = 1e18f, BestL = L0;
		for(int a = (int)std::floor(L0); a <= (int)std::ceil(L1); a++)
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
		if(pDist)
			*pDist = BestD;
		return BestL;
	};
	const bool ProjMode = Get("mode", "proj") == "proj";
	// shadow run (e.g. the incumbent): its inputs at the matched place of its own path are extra candidates
	std::vector<STasInput> vSh;
	std::vector<vec2> vShP; // position after step i (index = step from the shadow's grenade pickup)
	std::vector<vec2> vShV; // velocity after step i
	int ShRt0 = 0; // race tick after shadow step 0
	size_t ShI0 = 0; // index of the first post-pickup input
	if(Kv.count("shadow"))
	{
		vSh = ReadInputs(Kv["shadow"].c_str());
		CTasGame Gs;
		Gs.Spawn(CTasGame::Map().m_vSpawns[0]);
		size_t j = 0;
		for(; j < vSh.size() && !Gs.HasGrenade(); j++)
			Gs.Step(vSh[j]);
		CFastG Fs;
		Fs.FromGame(Gs);
		ShI0 = j;
		for(; j < vSh.size(); j++)
		{
			Fs.Step(vSh[j]);
			if(vShP.empty())
				ShRt0 = Fs.RaceTick();
			vShP.push_back(Fs.m_Pos);
			vShV.push_back(Fs.m_Core.m_Vel);
			if(Fs.m_FinishTick >= 0)
				break;
		}
		std::printf("shadow %s: %zu post-pickup steps from rt %d\n", Kv["shadow"].c_str(), vShP.size(), ShRt0);
	}
	// place on the shadow path (step index, fractional) near I0
	auto ShProj = [&](vec2 Q, float I0) {
		float BestD = 1e18f, BestI = I0;
		for(int a = (int)I0 - 3; a <= (int)I0 + 12; a++)
		{
			if(a < 0 || a + 1 >= (int)vShP.size())
				continue;
			vec2 A = vShP[a], B = vShP[a + 1];
			vec2 AB = B - A;
			float l2 = dot(AB, AB);
			float f = l2 > 0 ? std::clamp(dot(Q - A, AB) / l2, 0.0f, 1.0f) : 0;
			float d = distance(Q, A + AB * f);
			if(d < BestD)
			{
				BestD = d;
				BestI = a + f;
			}
		}
		return BestI;
	};
	const int SurvH = std::stoi(Get("surv", "25")), SurvM = std::stoi(Get("survm", "3"));
	const float EW = std::stof(Get("ew", "0"));
	const float SW = std::stof(Get("sw", "0"));
	const float Jitter = std::stof(Get("jitter", "0"));
	const int ShKeep = std::stoi(Get("shkeep", "300")); // shadow-input children kept past the quota (when a shadow is given)
	const float VW = std::stof(Get("vw", "0")), VDz = std::stof(Get("vdz", "5")); // velocity-match weight (px per px/t)
	const int Seed = std::stoi(Get("seed", "0")); // ticks per px/t of ramped speed along the route
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
	vL[0][0].m_Lab = RefProjW(G0.m_Pos, G0.RaceTick() + Off - 30, G0.RaceTick() + Off + 30, nullptr);
	if(!vShP.empty())
		vL[0][0].m_ShI = ShProj(G0.m_Pos, std::max(0, G0.RaceTick() - ShRt0 - 10));
	std::printf("root label %.1f (clock label %.1f)\n", vL[0][0].m_Lab, G0.RaceTick() + Off);
	int BestFin = -1, BestFinIdx = -1, FinLayer = -1;
	for(int t = 0; G0.RaceTick() + t < MaxT; t++)
	{
		const std::vector<SNode> &L = vL[t];
		const int Rt = L[0].m_G.RaceTick(); // all nodes share the tick
		const float Lab = Rt + 1 + Off; // label of the state after this step
		// reference facts near a label: jump / shot ticks, hook aims, shot aims
		auto MakeHint = [&](int LabI) {
			SHint H;
			std::vector<float> vHookAim, vFireAim;
			for(int d = -std::max(TolJ, TolF) - 1; d <= std::max(TolJ, TolF) + 1; d++)
			{
				int k = LabI + d;
				if(k < 0 || k >= (int)vRef.size())
					continue;
				const SRefTick &R = vRef[k];
				if(R.m_Jump && std::abs(d) <= TolJ)
					H.m_JumpNear = true;
				if(R.m_Fire && std::abs(d) <= TolF)
				{
					H.m_FireNear = true;
					if(R.m_FireCert >= 0.5f)
						vFireAim.push_back(R.m_FireAim);
				}
				if(std::abs(d) <= 1 && R.m_AimCert >= 0.5f)
				{
					vHookAim.push_back(R.m_Aim);
					if(R.m_Fire)
						vFireAim.push_back(R.m_Aim);
				}
			}
			for(float a : vHookAim)
			{
				H.m_vHA.push_back(a);
				for(float p : vPert)
				{
					H.m_vHA.push_back(a + p);
					H.m_vHA.push_back(a - p);
				}
			}
			for(int r = 0; r < Ring; r++)
				H.m_vHA.push_back(360.0f * r / Ring);
			if(H.m_FireNear)
			{
				for(float a : vFireAim)
				{
					H.m_vFA.push_back(a);
					for(float p : vFPert)
					{
						H.m_vFA.push_back(a + p);
						H.m_vFA.push_back(a - p);
					}
				}
				H.m_NSeenFA = H.m_vFA.size();
				for(int r = 0; r < FRing; r++)
					H.m_vFA.push_back(360.0f * r / FRing);
			}
			return H;
		};
		// per label (position-indexed mode: each node's own place on the reference line) or one for the layer (clock)
		std::map<int, SHint> HintCache;
		if(ProjMode)
		{
			for(const SNode &Nd : L)
			{
				int LI = (int)std::lround(Nd.m_Lab + 1);
				if(!HintCache.count(LI))
					HintCache[LI] = MakeHint(LI);
			}
		}
		else
			HintCache[(int)std::lround(Lab)] = MakeHint((int)std::lround(Lab));
		const SHint &H0 = HintCache.begin()->second;
		const bool JumpNear = H0.m_JumpNear, FireNear = H0.m_FireNear;
		const std::vector<float> &vFA = H0.m_vFA;
		const size_t NSeenFA = H0.m_NSeenFA;
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
				const SHint &Hn0 = ProjMode ? HintCache.at((int)std::lround(Nd.m_Lab + 1)) : H0;
				SHint Hn = Hn0;
				std::vector<STasInput> vShIn; // the shadow's own inputs near here (taken as they are)
				std::vector<std::pair<CFastG, STasInput>> ShFire; // the shadow's shots, stepped
				if(!vShP.empty())
				{
					int i0 = (int)std::lround(Nd.m_ShI) + 1;
					for(int di = -2; di <= 2; di++)
					{
						int i = i0 + di;
						if(i < 1 || i >= (int)vShP.size())
							continue;
						const STasInput &A = vSh[ShI0 + i], &Ap = vSh[ShI0 + i - 1];
						float Ang = std::atan2((float)A.m_TY, (float)A.m_TX) * 180.0f / pi;
						if(A.m_Hook && !Ap.m_Hook)
							Hn.m_vHA.insert(Hn.m_vHA.begin(), Ang);
						if(A.m_Jump && !Ap.m_Jump)
							Hn.m_JumpNear = true;
						if(A.m_Fire && !Ap.m_Fire)
						{
							if(!Hn.m_FireNear)
							{
								Hn.m_FireNear = true;
								Hn.m_vFA.clear();
								Hn.m_NSeenFA = 0;
								for(int r = 0; r < FRing; r++)
									Hn.m_vFA.push_back(360.0f * r / FRing);
							}
							Hn.m_vFA.insert(Hn.m_vFA.begin(), Ang);
							Hn.m_NSeenFA++;
						}
						if(std::abs(di) <= 1)
							vShIn.push_back(A);
					}
				}
				const bool JumpNear = Hn.m_JumpNear, FireNear = Hn.m_FireNear;
				const std::vector<float> &vHA = Hn.m_vHA, &vFA = Hn.m_vFA;
				const size_t NSeenFA = Hn.m_NSeenFA;
				const int T = Nd.m_G.m_Tick;
				const bool Pend = Nd.m_PendTau >= 0 && T - Nd.m_PendTau <= 95;
				const bool CanFire = Nd.m_G.m_ReloadTimer == 0 && FireNear; // a direct shot drops a reserved slot
				// his hooks near this label: may hold / press (he hooks), may release (he does not), his next anchor
				bool HkOn = true, HkOff = true, HkAnc = false;
				vec2 HkA(0, 0);
				if(HaveHooks)
				{
					const int LabI = (int)std::lround(ProjMode ? Nd.m_Lab + 1 : Lab);
					HkOn = HkOff = false;
					for(int d = -TolH; d <= TolH; d++)
					{
						int kk = LabI + d;
						char c = kk >= 0 && kk < 6000 ? vHkS[kk] : 0;
						if(c == 'G' || c == 'F')
							HkOn = true;
						else if(c == '-')
							HkOff = true;
						else
							HkOn = HkOff = true;
					}
					for(int kk = std::max(0, LabI - TolH); kk < std::min(6000, LabI + 9); kk++)
						if(vHkS[kk] == 'G')
						{
							HkA = vHkA[kk];
							HkAnc = true;
							break;
						}
				}
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
							if(HkOn)
							{
								vC.push_back(In);
								vNoPress.push_back(In);
							}
							if(HkOff)
							{
								In.m_Hook = 0;
								vC.push_back(In);
								vNoPress.push_back(In);
							}
						}
						else
						{
							In.m_Hook = 0;
							if(HkOff || !HkOn)
							{
								vC.push_back(In);
								vNoPress.push_back(In);
							}
							if(!HaveHooks)
							{
								for(float a : vHA)
								{
									In.m_Hook = 1;
									AimTo(a, In);
									vC.push_back(In);
								}
							}
							else if(HkOn)
							{
								std::vector<float> vA;
								if(HkAnc)
								{
									const vec2 Q = Nd.m_G.m_Pos;
									const float B = std::atan2(HkA.y - Q.y, HkA.x - Q.x) * 180.0f / pi;
									for(float o : {0.0f, 0.5f, -0.5f, 1.0f, -1.0f, 2.0f, -2.0f, 4.0f, -4.0f})
										vA.push_back(B + o);
								}
								for(size_t a = 0; a < vHA.size() && a < 3; a++)
									vA.push_back(vHA[a]);
								for(float a : vA)
								{
									In.m_Hook = 1;
									AimTo(a, In);
									vC.push_back(In);
								}
							}
						}
					}
				auto Emit = [&](CFastG &G, const STasInput &In, int PendTau, vec2 PendPos, int PatchTau, int16_t PTX, int16_t PTY) {
					if(G.m_Dead || G.m_Bad)
						return;
					float d = 0;
					float CLab = Nd.m_Lab + 1;
					if(ProjMode)
					{
						CLab = RefProjW(G.m_Pos, Nd.m_Lab - 3, Nd.m_Lab + 12, &d);
						d = std::min(std::max(0.0f, d - Dz), Cap);
					}
					else if(HaveRef)
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
					if(VW > 0 && ProjMode)
					{
						// velocity match: our displacement per tick (horizontal ramped) vs the reference's at our place
						vec2 A0, A1;
						if(RefPos(CLab - 2, A0) && RefPos(CLab + 2, A1))
						{
							vec2 V = G.m_Core.m_Vel;
							float Lv = length(V) * 50;
							float Ramp = Lv < 550 ? 1.0f : 1.0f / std::pow(1.4f, (Lv - 550) / 2000.0f);
							float dv = distance(vec2(V.x * Ramp, V.y), (A1 - A0) * 0.25f);
							float e = std::max(0.0f, dv - VDz);
							d = std::sqrt(d * d + VW * e * e);
						}
					}
					float Cost = Nd.m_Cost + d * d / 100.0f;
					float Rank = Cost - (PendTau >= 0 ? PendB : 0) - (G.m_ReloadTimer == 0 && PendTau < 0 ? FreeB : 0);
					if(ProjMode)
						Rank += LagW * (Rt + 1 + Off - CLab) * 100.0f; // signed: ahead of the reference is good
					if(SW > 0 && ProjMode)
					{
						// speed along the reference route, horizontal part as actually moved (velocity ramp on |v|)
						vec2 V = G.m_Core.m_Vel;
						float Lv = length(V) * 50;
						float Ramp = Lv < 550 ? 1.0f : 1.0f / std::pow(1.4f, (Lv - 550) / 2000.0f);
						vec2 A0, A1;
						if(RefPos(CLab - 2, A0) && RefPos(CLab + 2, A1) && distance(A0, A1) > 1)
						{
							vec2 Dir = normalize(A1 - A0);
							Rank -= SW * dot(vec2(V.x * Ramp, V.y), Dir) * LagW * 100.0f;
						}
					}
					if(EW > 0)
					{
						// energy credit (ticks per unit of v^2 - y, as x_ds' egain), in the same units as the lag term
						vec2 V = G.m_Core.m_Vel;
						Rank -= EW * (dot(V, V) - G.m_Pos.y) * LagW * 100.0f;
					}
					if(Jitter > 0)
					{
						// deterministic tie-breaking noise (seeded): different runs keep different equal-rank states
						uint64_t h = (uint64_t)Seed * 0x9E3779B97F4A7C15ull ^ (uint64_t)(int64_t)std::lround(G.m_Pos.x * 4) * 0xC2B2AE3D27D4EB4Full ^
							     (uint64_t)(int64_t)std::lround(G.m_Pos.y * 4) * 0x165667B19E3779F9ull ^ (uint64_t)(int64_t)std::lround(G.m_Core.m_Vel.x * 64) * 0x27D4EB2F165667C5ull ^
							     (uint64_t)(int64_t)std::lround(G.m_Core.m_Vel.y * 64);
						h ^= h >> 31;
						h *= 0xBF58476D1CE4E5B9ull;
						h ^= h >> 29;
						Rank += Jitter * (float)(h & 0xffff) / 65535.0f;
					}
					SNode C{G, In, k, Cost, Rank};
					C.m_Lab = CLab;
					C.m_ShI = vShP.empty() ? 0 : ShProj(G.m_Pos, Nd.m_ShI);
					C.m_PendTau = PendTau;
					C.m_PendPos = PendPos;
					C.m_PatchTau = PatchTau;
					C.m_PatchTX = PTX;
					C.m_PatchTY = PTY;
					R.push_back(std::move(C));
				};
				const int KeepTau = Pend ? Nd.m_PendTau : -1;
				const size_t NShIn = vShIn.size(); // the shadow inputs go first into vC
				for(const STasInput &A : vShIn)
				{
					// the shadow input as it is (its exact aim and shot); a shot only with the reload free, and it
					// drops a reserved slot (no second shot inside one reload)
					STasInput In = A;
					In.m_Weapon = -1;
					if(In.m_Jump && P.m_Jump)
						In.m_Jump = 0;
					if(In.m_Fire)
					{
						if(Nd.m_G.m_ReloadTimer == 0 && !P.m_Fire)
						{
							CFastG G = Nd.m_G;
							G.Step(In);
							ShFire.push_back({G, In});
						}
						In.m_Fire = 0;
					}
					vC.push_back(In);
				}
				for(size_t ci = 0; ci < vC.size(); ci++)
				{
					const STasInput &In = vC[ci];
					CFastG G = Nd.m_G;
					G.Step(In);
					size_t Before = R.size();
					Emit(G, In, KeepTau, Nd.m_PendPos, -1, 0, 0);
					if(ci < NShIn && R.size() > Before)
						R.back().m_Sh = true;
				}
				for(auto &[G, In] : ShFire)
				{
					Emit(G, In, -1, vec2(0, 0), -1, 0, 0);
					if(!R.empty() && R.back().m_Parent == k)
						R.back().m_Sh = true;
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
		if(LagW > 0 && !ProjMode)
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
		// survival: the best-ranked SurvM*Beam candidates must survive SurvH ticks under at least one simple policy
		// (hold or release the hook, any dir; a new hook in one of 8 directions; a jump; a point-blank shot in one of
		// 16 directions). Candidates beyond that are not checked (a policy this simple misses clever escapes).
		std::vector<uint8_t> vDoomed(vAll.size(), 0);
		if(SurvH > 0)
		{
			const size_t M = std::min(vAll.size(), (size_t)SurvM * Beam);
			std::atomic<size_t> NextS(0);
			auto Surv = [&]() {
				while(true)
				{
					size_t q = NextS++;
					if(q >= M)
						break;
					const SNode &Nd = vAll[q];
					bool Ok = false;
					for(int Pol = 0; Pol < 31 && !Ok; Pol++)
					{
						CFastG G = Nd.m_G;
						STasInput In = Nd.m_In;
						In.m_Jump = 0;
						In.m_Fire = 0;
						STasInput First = In;
						if(Pol < 6)
						{
							In.m_Dir = Pol % 3 - 1;
							if(Pol >= 3)
								In.m_Hook = 0;
							First = In;
						}
						else if(Pol < 14)
						{
							In.m_Dir = 0;
							In.m_Hook = 0;
							First = In;
						}
						else if(Pol == 14)
						{
							if(Nd.m_In.m_Jump)
								continue;
							First.m_Jump = 1;
							In.m_Dir = 0;
						}
						else
						{
							if(G.m_ReloadTimer != 0 || Nd.m_In.m_Fire)
								continue;
							First.m_Fire = 1;
							AimTo(22.5f * (Pol - 15), First);
						}
						bool Dead = false;
						for(int h = 0; h < SurvH; h++)
						{
							if(Pol >= 6 && Pol < 14 && h == 1)
							{
								In.m_Hook = 1;
								AimTo(45.0f * (Pol - 6), In);
							}
							G.Step(h == 0 ? First : In);
							if(G.m_FinishTick >= 0)
								break;
							if(G.m_Dead)
							{
								Dead = true;
								break;
							}
						}
						Ok = !Dead;
					}
					vDoomed[q] = !Ok;
				}
			};
			std::vector<std::thread> vT;
			for(int Th = 0; Th < NumThreads; Th++)
				vT.emplace_back(Surv);
			for(auto &Th : vT)
				Th.join();
		}
		std::vector<SNode> vU;
		std::unordered_map<uint64_t, int> Seen;
		std::unordered_map<uint64_t, int> CellCnt;
		std::vector<size_t> vOver;
		bool AllDoomed = true;
		for(size_t q = 0; q < vAll.size() && AllDoomed; q++)
			AllDoomed = vDoomed[q] != 0;
		if(AllDoomed)
			std::fill(vDoomed.begin(), vDoomed.end(), 0); // nothing passes the simple policies: keep the best anyway
		auto Key = [&](const SNode &Nd) {
			const CFastG &G = Nd.m_G;
			auto Q = [](float v, float q) { return (uint64_t)(int64_t)std::lround(v / q) & 0xffff; };
			uint64_t K = Q(G.m_Pos.x, Qp) | Q(G.m_Pos.y, Qp) << 16 | Q(G.m_Core.m_Vel.x, Qv) << 32 | Q(G.m_Core.m_Vel.y, Qv) << 48;
			int Hs = G.m_Core.m_HookState;
			uint64_t K2 = (uint64_t)(Hs + 2) * 1000003ull +
				      (Hs == HOOK_GRABBED || Hs == HOOK_FLYING ? (uint64_t)(G.m_Core.m_HookPos.x / 4) * 7919 + (uint64_t)(G.m_Core.m_HookPos.y / 4) : 0) +
				      (uint64_t)G.m_Core.m_Jumped * 31 + (Nd.m_In.m_Hook ? 17 : 0) + (uint64_t)G.m_ReloadTimer * 131071 + (uint64_t)(Nd.m_PendTau + 7) * 2246822519ull;
			for(int p = 0; p < G.m_NumProj; p++)
				K2 = K2 * 1315423911ull + (uint64_t)G.m_aProj[p].m_StartTick * 2654435761ull + (uint64_t)(int64_t)std::lround(std::atan2(G.m_aProj[p].m_Dir.y, G.m_aProj[p].m_Dir.x) * 2000);
			return K ^ (K2 * 0x9E3779B97F4A7C15ull);
		};
		if(ShKeep > 0)
		{
			int NSh = 0;
			for(SNode &Nd : vAll)
			{
				if(NSh >= ShKeep)
					break;
				if(!Nd.m_Sh)
					continue;
				if(vDoomed[&Nd - vAll.data()])
				{
					// the shadow run survived where it goes: no survival check for its own inputs, but only on (nearly)
					// its own state - far from it, its inputs can steer straight into freeze
					int i = (int)std::lround(Nd.m_ShI);
					if(i < 0 || i >= (int)vShP.size() || distance(Nd.m_G.m_Pos, vShP[i]) > 3.0f || distance(Nd.m_G.m_Core.m_Vel, vShV[i]) > 1.5f)
						continue;
				}
				if(!Seen.emplace(Key(Nd), 0).second)
					continue;
				vU.push_back(Nd);
				vDoomed[&Nd - vAll.data()] = 1; // taken
				NSh++;
			}
		}
		for(SNode &Nd : vAll)
		{
			if(vDoomed[&Nd - vAll.data()])
				continue;
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
			std::printf("rt %d label %.1f: beam %zu/%zu cost %.0f pos %.0f %.0f ref %.0f %.0f d %.0f |v| %.1f proj %.1f lag %.1f\n", Rt + 1, Lab, vU.size(), vAll.size(), B.m_Cost,
				B.m_G.m_Pos.x, B.m_G.m_Pos.y, TP.x, TP.y, distance(B.m_G.m_Pos, TP), length(B.m_G.m_Core.m_Vel), B.m_Lab, Lab - B.m_Lab);
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
