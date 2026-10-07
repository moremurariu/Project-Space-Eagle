// ddsearch: beam search to the grenade pickup on the exact fast stepper (CFast).
//
//   ddsearch MAP best=FILE cut=RT [key=value ...]
//
// Starts from the state of the best run (inputs from spawn) at race tick RT (negative: before the start line) and
// searches to the grenade pickup. The best run's own continuation always stays in the beam (incumbent), so a search
// can only find an earlier pickup or nothing. States are ranked by a time model against the best run (reference):
//   est = rt + (T_ref - t_match) + hnow * (1/v_tan - 1/v_ref) - ge * (E - E_ref) + lp * max(0, lat - latdz)
// t_match: fractional reference tick of the nearest reference point (tracked forward), v_tan: displacement per tick
// along the reference tangent (x scaled by the velocity ramp), E = |v|^2 - y.
// A kept state must survive `survive` ticks under at least one simple continuation (dir, hook kept/released, jump).
// Hook aims: `angles` evenly spaced, plus rotation-pulse aims (rothook=1: +-30 deg around the velocity normals in
// 3 deg steps; rothook=2: the exact edge of the region where the hook pull is accepted above the speed limit,
// found by bisection, and 4 aims slightly inside it).
// Keys: beam angles hnow ge jw lp latdz threads seed jitter out cellpos cellvel abort rothook survive incumbent slack
//       dumpat=RT [dumpk=K] (stop when the beam reaches RT and write the best-ranked state's inputs to OUT, and up to
//       K-1 more distinct top states to OUT.1 ...)
//       via=0|1 (default 0; 1: a pickup only counts if a small search gets from there to the climb point 5254,2176 without
//       freezing; viamax viabeam viatry) goal=pickup|climb (climb: minimise pickup + ticks to the climb point; climbextra)
//       gateend=1 [gater=R] (goal: the reference's last position instead of the pickup; for post-pickup checks)
//       geogoal=X,Y [geol=L geos=S geor=R geoge=G geovmin=V geojump=1 geovref=VX,VY geovtol=T] (short windows: rank by race tick + geodesic distance from the position
//       extrapolated L ticks ahead to the gate point / S; stop when within R px of it; writes the inputs to OUT)
//       aimfile=FILE [aimwin=W] (another player's aim per race tick "rt degrees" (y down): his aims around the matched
//       reference time, +-W ticks and +-0.5 deg, are added to the hook fire options)
//       ref=FILE (reference track "race_tick x y" per line on our race clock, e.g. another player's run; replaces the
//       best run's positions as the time model's reference inside its time range)
//       prefix=FILE (start at the end of FILE instead of a cut of the best run; reference = best, no incumbent unless
//       transplant=1: the best run's inputs replayed from its most similar state are kept in the beam)
// Writes OUT (inputs from spawn to the pickup) when it finds an earlier pickup, verified on CTasGame with the fast
// collision paths off (plain DDNet prediction code).
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected

#include "fast.h"
#include "tasio.h"
#include "via.h"

#include <game/collision.h>
#include <game/gamecore.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct SRefPt
{
	vec2 m_Pos;
	vec2 m_Vel;
	float m_E;
	float m_VTan; // displacement per tick along the tangent
	vec2 m_Tan;
};

static std::vector<SRefPt> gs_vRef; // index = race tick of the reference run
static int gs_RefEnd; // reference pickup race tick
static int gs_RefStart; // reference start tick (reference index i = race tick i + 1 - start)

static int gs_Beam = 8000;
static int gs_Angles = 64;
static float gs_HNow = 600.0f;
static float gs_Ge = 0.02f;
static float gs_Lp = 0.0f;
static float gs_Jw = 0.0f; // energy credit for an unused air jump (jw=144 ~ its impulse)
static float gs_LatDz = 24.0f;
static float gs_LatQ = 0.0f; // latq: + latq * lateral distance^2 (follow the reference's path closely)
static int gs_Shadow = 0; // shadow=N: add the best run's inputs at the matched reference index (offsets 1-N .. N)
static std::vector<STasInput> gs_vShadowIn;
static int gs_ShadowShift = 0; // shadowfile=FILE shadowshift=S: shadow inputs from another run whose track is shifted by S ticks in ref=
static float gs_VelW = 0.0f;
static int gs_VelSym = 0; // velsym=1: velw also penalises being faster than the reference // velw: + velw * (sideways^2 + slower^2) velocity difference to the reference
static int gs_Threads = 1;
static float gs_Jitter = 0.0f;
static float gs_CellPos = 8.0f;
static float gs_CellVel = 1.0f;
static float gs_Abort = 25.0f; // give up when the best estimate is this many ticks behind the reference
static int gs_RotHook = 1;
static std::vector<float> gs_vAimRef; // aimfile: another player's aim (radians) per reference race tick (index = race tick + 100)
static int gs_AimWin = 3; // add rotation-pulse hook aims (towards the reference tangent)

static float RampScale(vec2 Vel)
{
	return VelocityRamp(length(Vel) * 50, CTuningParams::DEFAULT.m_VelrampStart, CTuningParams::DEFAULT.m_VelrampRange, CTuningParams::DEFAULT.m_VelrampCurvature);
}

struct SNode
{
	int m_Parent;
	STasInput m_In;
};

struct SState
{
	CFast m_F;
	int m_Node; // index into the node array of its layer
	int m_RefIdx; // tracked reference index
	float m_Score;
	int m_LastHook;
	int m_LastJump;
	float m_Acc = 0.0f; // track mode: decayed sum of squared position errors
};

static int gs_Geo = 0; // geogoal=X,Y: rank by race tick + geodesic distance to a gate point, stop at the gate
static vec2 gs_GeoGoal;
static float gs_GeoL = 4.0f, gs_GeoS = 30.0f, gs_GeoR = 48.0f;
static float gs_GeoGe = 0.0f, gs_GeoVMin = 0.0f; // geo mode: energy credit (with jw), minimum speed at the gate
static int gs_GeoMax = 250; // geo mode: give up after this many ticks
static int gs_GeoJump = 0; // geo mode: the gate needs an unused air jump
static vec2 gs_GeoVRef = vec2(0, 0); // geo mode: reference velocity at the gate (geovref=vx,vy)
static float gs_GeoVTol = 1e9f; // geo mode: required speed along geovref's direction >= |geovref| - geovtol
static CViaField gs_GeoField;

static bool AtGoal(const CFast &F);

// track=FILE ("race_tick x y", any spacing, e.g. another player's run on our race clock): rank by how closely the tee
// follows that trajectory in time (target = the track at race tick rt + trackoff): decayed sum of squared position
// errors (trackdecay) + trackw * squared error of the position extrapolated tracklook ticks ahead. Stops at
// race tick trackend and dumps the best states like dumpat.
static std::vector<vec2> gs_vTrack; // index = race tick + 100 (target already shifted by trackoff)
static std::vector<vec2> gs_vTrackVel; // optional target velocities (track lines "race_tick x y vx vy")
static float gs_TrackOff = 0.0f, gs_TrackDecay = 0.9f, gs_TrackW = 0.5f, gs_TrackV = 0.0f; // trackv: weight of squared velocity errors
static int gs_TrackLook = 4;
static vec2 TrackAt(int Rt)
{
	int k = std::clamp(Rt + 100, 0, (int)gs_vTrack.size() - 1);
	return gs_vTrack[k];
}

static float Score(SState &S, std::mt19937 *pRng)
{
	if(!gs_vTrack.empty())
	{
		const CFast &F = S.m_F;
		vec2 P = F.m_Core.m_Pos, V = F.m_Core.m_Vel;
		int Rt = F.RaceTick();
		vec2 T = TrackAt(Rt);
		S.m_Acc = S.m_Acc * gs_TrackDecay + dot(P - T, P - T);
		if(gs_TrackV > 0 && !gs_vTrackVel.empty())
		{
			vec2 Dv = V - gs_vTrackVel[std::clamp(Rt + 100, 0, (int)gs_vTrackVel.size() - 1)];
			S.m_Acc += gs_TrackV * dot(Dv, Dv);
		}
		vec2 Vd(V.x * RampScale(V), V.y);
		vec2 L = P + Vd * (float)gs_TrackLook - TrackAt(Rt + gs_TrackLook);
		S.m_RefIdx = Rt + (int)std::lround(gs_TrackOff) + gs_RefStart - 1; // for aimfile: the tracked player's time
		float Est = S.m_Acc + gs_TrackW * dot(L, L);
		if(pRng && gs_Jitter > 0)
			Est += std::uniform_real_distribution<float>(-gs_Jitter, gs_Jitter)(*pRng);
		return Est;
	}
	if(gs_Geo)
	{
		const CFast &F = S.m_F;
		vec2 P = F.m_Core.m_Pos, V = F.m_Core.m_Vel;
		float D = std::min(gs_GeoField.Dist(P + V * gs_GeoL), gs_GeoField.Dist(P) + gs_GeoL * gs_GeoS);
		float Est = (float)F.RaceTick() + D / gs_GeoS;
		if(gs_GeoGe > 0)
		{
			float E = dot(V, V) - P.y + (!(F.m_Core.m_Jumped & 2) ? gs_Jw : 0.0f);
			Est -= gs_GeoGe * E;
		}
		if(pRng && gs_Jitter > 0)
			Est += std::uniform_real_distribution<float>(-gs_Jitter, gs_Jitter)(*pRng);
		return Est;
	}
	const CFast &F = S.m_F;
	vec2 P = F.m_Core.m_Pos, V = F.m_Core.m_Vel;
	int N = (int)gs_vRef.size();
	int Lo = std::max(0, S.m_RefIdx - 8), Hi = std::min(N - 2, S.m_RefIdx + 60);
	float BestD = 1e18f;
	float BestT = S.m_RefIdx;
	int BestI = S.m_RefIdx;
	for(int i = Lo; i <= Hi; i++)
	{
		vec2 A = gs_vRef[i].m_Pos, B = gs_vRef[i + 1].m_Pos;
		vec2 AB = B - A;
		float L2 = dot(AB, AB);
		float u = L2 > 0 ? std::clamp(dot(P - A, AB) / L2, 0.0f, 1.0f) : 0.0f;
		vec2 Q = A + AB * u;
		float D = dot(P - Q, P - Q);
		if(D < BestD)
		{
			BestD = D;
			BestT = i + u;
			BestI = i;
		}
	}
	S.m_RefIdx = BestI;
	const SRefPt &R = gs_vRef[BestI];
	float Lat = std::sqrt(BestD);
	float Ramp = RampScale(V);
	float VTan = V.x * Ramp * R.m_Tan.x + V.y * R.m_Tan.y;
	float E = dot(V, V) - P.y;
	// an unused air jump is worth about its impulse squared (vy = -12 from rest); grounded refills both
	if(!(F.m_Core.m_Jumped & 2))
		E += gs_Jw;
	float Rt = (float)F.RaceTick();
	float Est = Rt + (gs_RefEnd - (BestT + 1 - gs_RefStart));
	Est += gs_HNow * (1.0f / std::max(VTan, 1.0f) - 1.0f / std::max(R.m_VTan, 1.0f));
	Est -= gs_Ge * (E - R.m_E);
	if(gs_Lp > 0 && Lat > gs_LatDz)
		Est += gs_Lp * (Lat - gs_LatDz);
	if(gs_LatQ > 0)
		Est += gs_LatQ * BestD;
	if(gs_VelW > 0)
	{
		// follow the reference's velocity: penalise sideways and slower deviations (not faster ones)
		vec2 Dv = V - R.m_Vel;
		float Along = dot(Dv, R.m_Tan);
		vec2 Perp = Dv - R.m_Tan * Along;
		Est += gs_VelW * (dot(Perp, Perp) + (Along < 0 || gs_VelSym ? Along * Along : 0.0f));
	}
	if(pRng && gs_Jitter > 0)
		Est += std::uniform_real_distribution<float>(-gs_Jitter, gs_Jitter)(*pRng);
	return Est;
}

static uint64_t CellKey(const SState &S)
{
	const CCharacterCore &C = S.m_F.m_Core;
	int64_t px = (int64_t)std::floor(C.m_Pos.x / gs_CellPos), py = (int64_t)std::floor(C.m_Pos.y / gs_CellPos);
	int64_t vx = (int64_t)std::floor(C.m_Vel.x / gs_CellVel), vy = (int64_t)std::floor(C.m_Vel.y / gs_CellVel);
	int64_t hs = C.m_HookState + 2;
	int64_t hx = 0, hy = 0;
	if(C.m_HookState == HOOK_GRABBED || C.m_HookState == HOOK_FLYING)
	{
		hx = (int64_t)std::floor(C.m_HookPos.x / 16);
		hy = (int64_t)std::floor(C.m_HookPos.y / 16);
	}
	uint64_t h = 1469598103934665603ull;
	auto Mix = [&](int64_t v) { h = (h ^ (uint64_t)v) * 1099511628211ull; };
	Mix(px);
	Mix(py);
	Mix(vx);
	Mix(vy);
	Mix(hs);
	Mix(hx);
	Mix(hy);
	Mix(C.m_Jumped);
	Mix(S.m_LastJump);
	Mix(S.m_LastHook);
	return h;
}

static int gs_Incumbent = 1;
static int gs_Transplant = 0; // prefix mode: incumbent = the best run's inputs replayed from the prefix (most similar state)
static int gs_Via = 0; // 1: a pickup only counts if the tee can get from there to the climb point (gs_ViaGoal) without freezing
static vec2 gs_ViaGoal(5254.0f, 2176.0f);
static int gs_ViaMax = 70, gs_ViaBeam = 1500, gs_ViaTry = 40;
static int gs_GoalClimb = 0; // 1: minimise pickup + ticks to the climb point instead of the pickup tick
static int gs_ClimbExtra = 8; // goal=climb: keep searching this many ticks after the first viable pickup
static CViaField gs_ViaField;
static int gs_GateEnd = 0; // 1: the goal is the reference's last position (within gs_GateR), not the pickup
static float gs_GateR = 48.0f;
static vec2 gs_GatePos;
static int gs_DumpAt = 1 << 30;
static int gs_DumpK = 1; // dumpk: number of distinct top states to dump (OUT, OUT.1, ...) // write the best-ranked state's inputs when the beam reaches this race tick, then stop
static int gs_Slack = 0; // also accept pickups up to this many ticks after the reference (exploration, with incumbent=0)
static int gs_Survive = 8; // ticks a kept state must survive under at least one simple continuation (0 = off)

// true if some simple continuation (dir -1/0/1, hook kept or released, optional immediate jump) survives Ticks ticks
static bool Survives(const SState &S, int Ticks)
{
	if(Ticks <= 0)
		return true;
	const CCharacterCore &C = S.m_F.m_Core;
	for(int J = 0; J <= 1; J++)
		for(int H = (S.m_LastHook ? 1 : 0); H >= 0; H--)
			for(int Dir : {0, 1, -1})
			{
				if(J && ((C.m_Jumped & 1) && S.m_LastJump))
					continue;
				CFast F = S.m_F;
				bool Ok = true;
				for(int t = 0; t < Ticks; t++)
				{
					STasInput In;
					In.m_Dir = Dir;
					In.m_Jump = (J && t == 0) ? 1 : 0;
					In.m_Hook = H;
					In.m_Fire = 0;
					In.m_Weapon = -1;
					In.m_TX = 0;
					In.m_TY = -1;
					F.Step(In);
					if(F.m_Dead)
					{
						Ok = false;
						break;
					}
					if(F.m_Got && !gs_Via)
						break; // the pickup is the goal; what happens after it does not matter
				}
				if(Ok)
					return true;
			}
	return false;
}

static bool AtGoal(const CFast &F)
{
	if(gs_Geo)
	{
		float Lr = length(gs_GeoVRef);
		bool VelOk = Lr <= 0 || dot(F.m_Core.m_Vel, gs_GeoVRef / Lr) >= Lr - gs_GeoVTol;
		return distance(F.m_Core.m_Pos, gs_GeoGoal) < gs_GeoR && length(F.m_Core.m_Vel) >= gs_GeoVMin && VelOk &&
		       (!gs_GeoJump || !(F.m_Core.m_Jumped & 2));
	}
	if(gs_GateEnd)
		return F.m_Got && distance(F.m_Core.m_Pos, gs_GatePos) < gs_GateR;
	return F.m_Got;
}

int main(int argc, const char **argv)
{
	if(argc < 2)
	{
		std::printf("usage: ddsearch MAP best=FILE cut=RT [beam= angles= hnow= ge= lp= latdz= threads= seed= jitter= out= cellpos= cellvel= abort= rothook=]\n");
		return 1;
	}
	std::string BestFile, PrefixFile, RefFile, TrackFile, ShadowFile, Out = "ddsearch_out.txt";
	int TrackEnd = 1 << 30;
	int Cut = 100;
	unsigned Seed = 1;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		const char *v = V.c_str();
		if(K == "best")
			BestFile = V;
		else if(K == "cut")
			Cut = atoi(v);
		else if(K == "beam")
			gs_Beam = atoi(v);
		else if(K == "angles")
			gs_Angles = atoi(v);
		else if(K == "hnow")
			gs_HNow = atof(v);
		else if(K == "ge")
			gs_Ge = atof(v);
		else if(K == "geogoal")
		{
			gs_Geo = 1;
			std::sscanf(v, "%f,%f", &gs_GeoGoal.x, &gs_GeoGoal.y);
		}
		else if(K == "geoge")
			gs_GeoGe = atof(v);
		else if(K == "geomax")
			gs_GeoMax = atoi(v);
		else if(K == "geovref")
			std::sscanf(v, "%f,%f", &gs_GeoVRef.x, &gs_GeoVRef.y);
		else if(K == "geovtol")
			gs_GeoVTol = atof(v);
		else if(K == "geovmin")
			gs_GeoVMin = atof(v);
		else if(K == "geojump")
			gs_GeoJump = atoi(v);
		else if(K == "geol")
			gs_GeoL = atof(v);
		else if(K == "geos")
			gs_GeoS = atof(v);
		else if(K == "geor")
			gs_GeoR = atof(v);
		else if(K == "transplant")
			gs_Transplant = atoi(v);
		else if(K == "jw")
			gs_Jw = atof(v);
		else if(K == "lp")
			gs_Lp = atof(v);
		else if(K == "latdz")
			gs_LatDz = atof(v);
		else if(K == "threads")
			gs_Threads = atoi(v);
		else if(K == "seed")
			Seed = atoi(v);
		else if(K == "jitter")
			gs_Jitter = atof(v);
		else if(K == "out")
			Out = V;
		else if(K == "cellpos")
			gs_CellPos = atof(v);
		else if(K == "cellvel")
			gs_CellVel = atof(v);
		else if(K == "abort")
			gs_Abort = atof(v);
		else if(K == "rothook")
			gs_RotHook = atoi(v);
		else if(K == "via")
			gs_Via = atoi(v);
		else if(K == "viamax")
			gs_ViaMax = atoi(v);
		else if(K == "viabeam")
			gs_ViaBeam = atoi(v);
		else if(K == "viatry")
			gs_ViaTry = atoi(v);
		else if(K == "goal")
			gs_GoalClimb = V == "climb";
		else if(K == "climbextra")
			gs_ClimbExtra = atoi(v);
		else if(K == "gateend")
			gs_GateEnd = atoi(v);
		else if(K == "gater")
			gs_GateR = atof(v);
		else if(K == "track")
			TrackFile = V;
		else if(K == "trackoff")
			gs_TrackOff = atof(v);
		else if(K == "trackdecay")
			gs_TrackDecay = atof(v);
		else if(K == "trackw")
			gs_TrackW = atof(v);
		else if(K == "tracklook")
			gs_TrackLook = atoi(v);
		else if(K == "latq")
			gs_LatQ = atof(v);
		else if(K == "shadow")
			gs_Shadow = atoi(v);
		else if(K == "shadowfile")
			ShadowFile = V;
		else if(K == "shadowshift")
			gs_ShadowShift = atoi(v);
		else if(K == "velsym")
			gs_VelSym = atoi(v);
		else if(K == "velw")
			gs_VelW = atof(v);
		else if(K == "trackv")
			gs_TrackV = atof(v);
		else if(K == "trackend")
			TrackEnd = atoi(v);
		else if(K == "ref")
			RefFile = V;
		else if(K == "aimwin")
			gs_AimWin = atoi(v);
		else if(K == "aimfile")
		{
			FILE *pA = std::fopen(v, "r");
			int k;
			float a;
			while(pA && std::fscanf(pA, "%d %f", &k, &a) == 2)
			{
				if(k + 100 < 0)
					continue;
				if((int)gs_vAimRef.size() <= k + 100)
					gs_vAimRef.resize(k + 101, std::nanf(""));
				gs_vAimRef[k + 100] = a * pi / 180.0f;
			}
			if(pA)
				std::fclose(pA);
			std::printf("aims from %s: %zu ticks\n", v, gs_vAimRef.size());
		}
		else if(K == "dumpk")
			gs_DumpK = atoi(v);
		else if(K == "dumpat")
			gs_DumpAt = atoi(v);
		else if(K == "prefix")
			PrefixFile = V;
		else if(K == "slack")
			gs_Slack = atoi(v);
		else if(K == "incumbent")
			gs_Incumbent = atoi(v);
		else if(K == "survive")
			gs_Survive = atoi(v);
		else
		{
			std::printf("unknown key %s\n", K.c_str());
			return 1;
		}
	}
	if(!CTasGame::LoadMap(argv[1]))
		return 1;
	CFast::Init();
	CCollision::ms_FastPaths = true;
	std::vector<STasInput> vBest = ReadInputs(BestFile.c_str());
	gs_vShadowIn = ShadowFile.empty() ? vBest : ReadInputs(ShadowFile.c_str());
	if(vBest.empty())
	{
		std::printf("no inputs in %s\n", BestFile.c_str());
		return 1;
	}

	// replay the reference: states after every input, its start tick and pickup
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	CFast F;
	F.FromGame(G);
	std::vector<CFast> vStates; // state after input i
	std::vector<int> vLastHook, vLastJump;
	for(size_t i = 0; i < vBest.size(); i++)
	{
		F.Step(vBest[i]);
		vStates.push_back(F);
		vLastHook.push_back(vBest[i].m_Hook);
		vLastJump.push_back(vBest[i].m_Jump);
		if(F.m_Got && !gs_GateEnd)
			break;
		if(F.m_Dead)
		{
			std::printf("reference run dies at input %d\n", (int)i);
			return 1;
		}
	}
	if(!F.m_Got)
	{
		std::printf("reference run does not reach the grenade\n");
		return 1;
	}
	gs_RefStart = F.m_StartTick;
	CFast::ms_AssumedStart = gs_RefStart;
	gs_RefEnd = F.RaceTick();
	gs_GatePos = F.m_Core.m_Pos;
	if(gs_GateEnd)
		std::printf("goal: within %.0f px of %.0f %.0f (reference there at race tick %d)\n", gs_GateR, gs_GatePos.x, gs_GatePos.y, gs_RefEnd);
	// reference index i = state after input i, race tick i + 1 - start
	for(size_t i = 0; i < vStates.size(); i++)
	{
		SRefPt R;
		R.m_Pos = vStates[i].m_Core.m_Pos;
		R.m_Vel = vStates[i].m_Core.m_Vel;
		R.m_E = dot(R.m_Vel, R.m_Vel) - R.m_Pos.y;
		gs_vRef.push_back(R);
	}
	if(!RefFile.empty())
	{
		// external reference track ("race_tick x y" per line, race ticks on our clock, any spacing): replaces the
		// reference positions / velocities inside its time range; after its end the last point is held
		std::vector<std::array<float, 3>> vT;
		FILE *pF = std::fopen(RefFile.c_str(), "r");
		float a, b, c;
		while(pF && std::fscanf(pF, "%f %f %f", &a, &b, &c) == 3)
			vT.push_back({a, b, c});
		if(pF)
			std::fclose(pF);
		if(vT.size() < 10)
		{
			std::printf("bad reference track %s\n", RefFile.c_str());
			return 1;
		}
		auto At = [&](float Rt) {
			if(Rt <= vT.front()[0])
				return vec2(vT.front()[1], vT.front()[2]);
			if(Rt >= vT.back()[0])
				return vec2(vT.back()[1], vT.back()[2]);
			size_t j = 1;
			while(vT[j][0] < Rt)
				j++;
			float u = (Rt - vT[j - 1][0]) / std::max(1e-6f, vT[j][0] - vT[j - 1][0]);
			return vec2(vT[j - 1][1] + (vT[j][1] - vT[j - 1][1]) * u, vT[j - 1][2] + (vT[j][2] - vT[j - 1][2]) * u);
		};
		auto Smooth = [&](float Rt) {
			vec2 Sum(0, 0);
			for(int k = -2; k <= 2; k++)
				Sum += At(Rt + k);
			return Sum / 5.0f;
		};
		int Used = 0;
		gs_vRef.resize(vStates.size() + 400);
		for(int i = 0; i < (int)gs_vRef.size(); i++)
		{
			float Rt = (float)(i + 1 - gs_RefStart);
			if(Rt < vT.front()[0])
				continue;
			SRefPt R;
			R.m_Pos = Smooth(Rt);
			R.m_Vel = (Smooth(Rt + 1) - Smooth(Rt - 1)) / 2.0f;
			R.m_E = dot(R.m_Vel, R.m_Vel) - R.m_Pos.y;
			gs_vRef[i] = R;
			Used++;
		}
		std::printf("reference track %s: %zu points, race ticks %.1f .. %.1f\n", RefFile.c_str(), vT.size(), vT.front()[0], vT.back()[0]);
	}
	if(!TrackFile.empty())
	{
		std::vector<std::array<float, 3>> vT;
		std::vector<std::array<float, 3>> vTV; // race tick, vx, vy (5-column track files)
		FILE *pF = std::fopen(TrackFile.c_str(), "r");
		char aLine[256];
		while(pF && std::fgets(aLine, sizeof(aLine), pF))
		{
			float a, b, c, d, e;
			int n = std::sscanf(aLine, "%f %f %f %f %f", &a, &b, &c, &d, &e);
			if(n >= 3)
				vT.push_back({a, b, c});
			if(n == 5)
				vTV.push_back({a, d, e});
		}
		if(pF)
			std::fclose(pF);
		if(vTV.size() == vT.size() && !vTV.empty())
		{
			// velocities are only meaningful at the track's own (integer) race ticks
			for(int Rt = -100; Rt < 2000; Rt++)
			{
				float Want = Rt + gs_TrackOff;
				size_t j = 0;
				while(j + 1 < vTV.size() && vTV[j + 1][0] <= Want)
					j++;
				gs_vTrackVel.push_back(vec2(vTV[j][1], vTV[j][2]));
			}
		}
		if(vT.size() < 10)
		{
			std::printf("bad track %s\n", TrackFile.c_str());
			return 1;
		}
		auto At = [&](float Rt) {
			if(Rt <= vT.front()[0])
				return vec2(vT.front()[1], vT.front()[2]);
			if(Rt >= vT.back()[0])
				return vec2(vT.back()[1], vT.back()[2]);
			size_t j = 1;
			while(vT[j][0] < Rt)
				j++;
			float u = (Rt - vT[j - 1][0]) / std::max(1e-6f, vT[j][0] - vT[j - 1][0]);
			return vec2(vT[j - 1][1] + (vT[j][1] - vT[j - 1][1]) * u, vT[j - 1][2] + (vT[j][2] - vT[j - 1][2]) * u);
		};
		for(int Rt = -100; Rt < 2000; Rt++)
			gs_vTrack.push_back(At(Rt + gs_TrackOff));
		gs_DumpAt = std::min(TrackEnd, (int)vT.back()[0] - 2);
		gs_Incumbent = 0;
		std::printf("track %s (offset %+.2f): race ticks %.1f .. %.1f, dump at %d\n", TrackFile.c_str(), gs_TrackOff, vT.front()[0], vT.back()[0], gs_DumpAt);
	}
	int CutInput = Cut + gs_RefStart - 1;
	if(CutInput < 0 || CutInput >= (int)vStates.size() - 1)
		CutInput = -1;
	// tangents and tangential speeds (central differences)
	int N = (int)gs_vRef.size();
	for(int i = 0; i < N; i++)
	{
		vec2 A = gs_vRef[std::max(0, i - 1)].m_Pos, B = gs_vRef[std::min(N - 1, i + 1)].m_Pos;
		vec2 T = B - A;
		float L = length(T);
		gs_vRef[i].m_Tan = L > 0 ? T / L : vec2(1, 0);
		vec2 V = gs_vRef[i].m_Vel;
		gs_vRef[i].m_VTan = V.x * RampScale(V) * gs_vRef[i].m_Tan.x + V.y * gs_vRef[i].m_Tan.y;
	}
	std::vector<STasInput> vHead; // inputs before the search (prefix mode: the prefix file, else the best run up to the cut)
	CFast StartF;
	int StartHook = 0, StartJump = 0, StartRef = 0;
	if(!PrefixFile.empty())
	{
		vHead = ReadInputs(PrefixFile.c_str());
		CTasGame G2;
		G2.Spawn(CTasGame::Map().m_vSpawns[0]);
		StartF.FromGame(G2);
		for(const STasInput &In : vHead)
		{
			StartF.Step(In);
			if(StartF.m_Dead || (StartF.m_Got && !gs_GateEnd))
			{
				std::printf("prefix dies or picks up the grenade\n");
				return 1;
			}
		}
		StartHook = vHead.back().m_Hook;
		StartJump = vHead.back().m_Jump;
		// reference index: nearest reference point within 80 race ticks
		float BestD = 1e30f;
		for(int i = 0; i < (int)gs_vRef.size(); i++)
		{
			int RtI = i + 1 - gs_RefStart;
			if(std::abs(RtI - StartF.RaceTick()) > 80)
				continue;
			float Dd = distance(gs_vRef[i].m_Pos, StartF.m_Core.m_Pos);
			if(Dd < BestD)
			{
				BestD = Dd;
				StartRef = i;
			}
		}
		gs_Incumbent = 0;
		Cut = StartF.RaceTick();
		std::printf("prefix %s: %d inputs, race tick %d, reference index %d (%.0f px)\n", PrefixFile.c_str(), (int)vHead.size(), Cut, StartRef, BestD);
	}
	else
	{
		if(CutInput < 0)
		{
			std::printf("cut %d not in the run\n", Cut);
			return 1;
		}
		vHead.assign(vBest.begin(), vBest.begin() + CutInput + 1);
		StartF = vStates[CutInput];
		StartHook = vLastHook[CutInput];
		StartJump = vLastJump[CutInput];
		StartRef = CutInput;
	}
	if(gs_Geo)
	{
		gs_GeoField.Build(gs_GeoGoal);
		gs_Via = 0;
		gs_Incumbent = 0;
		gs_Slack = 100000; // the gate is not the pickup: no reference deadline
		std::printf("geo goal %.0f %.0f (radius %.0f, look-ahead %.0f ticks, speed %.0f)\n", gs_GeoGoal.x, gs_GeoGoal.y, gs_GeoR, gs_GeoL, gs_GeoS);
	}
	if(gs_Via && !gs_GateEnd)
	{
		gs_ViaField.Build(gs_ViaGoal);
		int RefVia = ViaTicks(gs_ViaField, vStates.back(), vBest[vStates.size() - 1].m_Hook, vBest[vStates.size() - 1].m_Jump, gs_ViaMax, gs_ViaBeam, 48.0f);
		std::printf("reference: climb point %.0f %.0f reached %d ticks after the pickup%s\n", gs_ViaGoal.x, gs_ViaGoal.y, RefVia, RefVia < 0 ? " (NOT VIABLE)" : "");
	}
	std::printf("reference pickup at race tick %d; cut at race tick %d (input %d); beam %d angles %d hnow %.0f ge %.3f lp %.3f seed %u\n",
		gs_RefEnd, Cut, CutInput + 1, gs_Beam, gs_Angles, gs_HNow, gs_Ge, gs_Lp, Seed);
	std::fflush(stdout);

	// hook aims: evenly spaced, as integer targets at radius 1000
	std::vector<std::pair<int, int>> vAims;
	for(int a = 0; a < gs_Angles; a++)
	{
		float Ang = (a + 0.5f) / gs_Angles * 2 * pi;
		vAims.emplace_back((int)std::lround(std::cos(Ang) * 1000), (int)std::lround(std::sin(Ang) * 1000));
	}

	// incumbent sequence: the best run's own continuation, or (prefix mode, transplant=1) the best run's inputs
	// replayed from the prefix's end, starting at the best run's most similar state
	std::vector<CFast> vIncF;
	std::vector<STasInput> vIncIn;
	if(PrefixFile.empty() && gs_Incumbent)
	{
		for(int k = CutInput + 1; k < (int)vStates.size(); k++)
		{
			vIncF.push_back(vStates[k]);
			vIncIn.push_back(vBest[k]);
		}
	}
	else if(!PrefixFile.empty() && gs_Transplant)
	{
		int BestJ = -1;
		float BestD = 1e30f;
		const CCharacterCore &C0 = StartF.m_Core;
		for(int j = 0; j + 1 < (int)vStates.size(); j++)
		{
			if(vStates[j].m_StartTick < 0 || std::abs(vStates[j].RaceTick() - StartF.RaceTick()) > 40)
				continue;
			const CCharacterCore &C = vStates[j].m_Core;
			float Dd = distance(C.m_Pos, C0.m_Pos) + 4.0f * distance(C.m_Vel, C0.m_Vel);
			if((C.m_Jumped & 2) != (C0.m_Jumped & 2))
				Dd += 1000.0f;
			if((C.m_HookState == HOOK_GRABBED) != (C0.m_HookState == HOOK_GRABBED) || vBest[j].m_Hook != StartHook)
				Dd += 300.0f;
			if(Dd < BestD)
			{
				BestD = Dd;
				BestJ = j;
			}
		}
		if(BestJ >= 0)
		{
			CFast T = StartF;
			for(int k = BestJ + 1; k < (int)vBest.size(); k++)
			{
				T.Step(vBest[k]);
				if(T.m_Dead)
					break;
				vIncF.push_back(T);
				vIncIn.push_back(vBest[k]);
				if(T.m_Got)
					break;
			}
			std::printf("transplant: best run from input %d (race tick %d, match %.1f): %zu ticks%s\n", BestJ + 1, vStates[BestJ].RaceTick(), BestD,
				vIncF.size(), !vIncF.empty() && vIncF.back().m_Got ? (" -> pickup at race tick " + std::to_string(vIncF.back().RaceTick())).c_str() : " (dies)");
		}
	}
	int IncBeamIdx = vIncF.empty() ? -1 : 0;

	std::vector<std::vector<SNode>> vLayers; // kept nodes per search tick
	std::vector<SState> vBeam(1);
	vBeam[0].m_F = StartF;
	vBeam[0].m_Node = -1;
	vBeam[0].m_RefIdx = StartRef;
	vBeam[0].m_LastHook = StartHook;
	vBeam[0].m_LastJump = StartJump;
	vBeam[0].m_Score = Score(vBeam[0], nullptr);

	struct SCand
	{
		float m_Score;
		uint64_t m_Key;
		int m_Parent; // beam index
		int m_RefIdx;
		float m_Acc;
		STasInput m_In;
	};

	auto T0 = std::chrono::steady_clock::now();
	int IncNode = -1; // node of the incumbent in the previous layer
	int IncRefIdx = StartRef;
	int FoundRt = -1, FoundVia = -1, FoundTotal = 1 << 30, FoundLayer = -1;
	SNode FoundNode;
	SState Found;
	for(int Step = 0;; Step++)
	{
		int RtNow = vBeam[0].m_F.RaceTick() + 1;
		if(FoundRt >= 0 && RtNow > FoundRt + (gs_GoalClimb ? gs_ClimbExtra : 0))
			break;
		if(gs_Geo && FoundRt < 0 && RtNow > Cut + gs_GeoMax)
		{
			std::printf("RESULT none (gate not reached within %d ticks)\n", gs_GeoMax);
			return 2;
		}
		if(FoundRt < 0 && RtNow >= gs_RefEnd + gs_Slack)
		{
			std::printf("RESULT none (no pickup before race tick %d)\n", gs_RefEnd);
			return 2;
		}
		std::vector<std::vector<SCand>> vCand(gs_Threads);
		std::atomic<int> Next(0);
		std::mutex GotMutex;
		struct SPick
		{
			SState m_S;
			STasInput m_In;
		};
		std::vector<SPick> vPicks;
		auto Worker = [&](int Th) {
			std::mt19937 TRng(Seed * 7919 + Step * 104729 + Th);
			auto &Cand = vCand[Th];
			std::vector<STasInput> vIn;
			for(;;)
			{
				int i = Next.fetch_add(1);
				if(i >= (int)vBeam.size())
					break;
				const SState &S = vBeam[i];
				const CCharacterCore &C = S.m_F.m_Core;
				bool Grounded = CTasGame::Collision()->IsOnGround(C.m_Pos, CCharacterCore::PhysicalSize());
				bool CanJump = !(C.m_Jumped & 1) && (Grounded || !(C.m_Jumped & 2));
				vIn.clear();
				for(int Dir = -1; Dir <= 1; Dir++)
					for(int J = 0; J <= (CanJump ? 1 : 0); J++)
					{
						STasInput In;
						In.m_Dir = Dir;
						In.m_Jump = J;
						In.m_Fire = 0;
						In.m_Weapon = -1;
						In.m_TX = 0;
						In.m_TY = -1;
						if(S.m_LastHook)
						{
							In.m_Hook = 1;
							vIn.push_back(In);
							In.m_Hook = 0;
							vIn.push_back(In);
						}
						else
						{
							In.m_Hook = 0;
							vIn.push_back(In);
							In.m_Hook = 1;
							for(auto &A : vAims)
							{
								In.m_TX = A.first;
								In.m_TY = A.second;
								vIn.push_back(In);
							}
							if(!gs_vAimRef.empty())
							{
								// the other player's aims around the matched reference time
								int RtRef = S.m_RefIdx + 1 - gs_RefStart;
								for(int dk = -gs_AimWin; dk <= gs_AimWin; dk++)
								{
									int k = RtRef + dk + 100;
									if(k < 0 || k >= (int)gs_vAimRef.size() || std::isnan(gs_vAimRef[k]))
										continue;
									for(float Off : {0.0f, -0.009f, 0.009f})
									{
										float Ang = gs_vAimRef[k] + Off;
										In.m_TX = (int)std::lround(std::cos(Ang) * 10000);
										In.m_TY = (int)std::lround(std::sin(Ang) * 10000);
										vIn.push_back(In);
									}
								}
							}
							if(gs_RotHook == 2)
							{
								// rotation pulses: aims at the edge of the region where the pull is accepted
								// (|v + dv| < |v| above the hook speed limit), and a few slightly inside it
								vec2 V = C.m_Vel;
								float Sp = length(V);
								auto Accepted = [&](float Ang) {
									vec2 Hv(std::cos(Ang) * 3.0f, std::sin(Ang) * 3.0f);
									if(Hv.y > 0)
										Hv.y *= 0.3f;
									if((Hv.x < 0 && Dir < 0) || (Hv.x > 0 && Dir > 0))
										Hv.x *= 0.95f;
									else
										Hv.x *= 0.75f;
									float L = length(V + Hv);
									return L < 15.0f || L < Sp;
								};
								float Base = std::atan2(V.y, V.x);
								if(Sp > 13.0f)
									for(int sg = -1; sg <= 1; sg += 2)
									{
										// from the normal (accepted only by chance) towards the back (accepted): find the edge
										float A0 = Base + sg * (pi / 2 - 0.6f), A1 = Base + sg * (pi - 0.05f);
										if(Accepted(A0) || !Accepted(A1))
											continue;
										for(int it = 0; it < 30; it++)
										{
											float M = (A0 + A1) / 2;
											if(Accepted(M))
												A1 = M;
											else
												A0 = M;
										}
										for(float Off : {0.0005f, 0.003f, 0.01f, 0.03f, 0.08f})
										{
											float Ang = A1 + sg * Off;
											// integer aims at radius 10000 resolve ~0.006 degrees
											In.m_TX = (int)std::lround(std::cos(Ang) * 10000);
											In.m_TY = (int)std::lround(std::sin(Ang) * 10000);
											vIn.push_back(In);
										}
									}
							}
							else if(gs_RotHook)
							{
								// fine aims near the normals of the velocity (rotation pulses): +-30 degrees in 3 degree steps
								vec2 V = C.m_Vel;
								float Base = std::atan2(V.y, V.x);
								for(int sg = -1; sg <= 1; sg += 2)
									for(int k = -10; k <= 10; k++)
									{
										float Ang = Base + sg * pi / 2 + k * pi / 60;
										In.m_TX = (int)std::lround(std::cos(Ang) * 1000);
										In.m_TY = (int)std::lround(std::sin(Ang) * 1000);
										vIn.push_back(In);
									}
							}
						}
					}
				if(gs_Shadow > 0)
				{
					// shadow: the reference run's own inputs at the matched reference point (the input that followed it,
					// and its neighbours), so a state that is close to the reference can repeat what the reference did
					for(int dk = 1 - gs_Shadow; dk <= gs_Shadow; dk++)
					{
						int k = S.m_RefIdx + 1 + dk - gs_ShadowShift;
						if(k < 0 || k >= (int)gs_vShadowIn.size())
							continue;
						STasInput In = gs_vShadowIn[k];
						In.m_Fire = 0;
						In.m_Weapon = -1;
						if(In.m_Hook && !S.m_LastHook && In.m_TX == 0 && In.m_TY == -1)
							continue;
						vIn.push_back(In);
					}
				}
				if(i == IncBeamIdx && Step < (int)vIncIn.size())
					vIn.push_back(vIncIn[Step]);
				for(const STasInput &In : vIn)
				{
					SState N2;
					N2.m_F = S.m_F;
					N2.m_F.Step(In);
					if(N2.m_F.m_Dead)
						continue;
					N2.m_RefIdx = S.m_RefIdx;
					N2.m_LastHook = In.m_Hook;
					N2.m_LastJump = In.m_Jump;
					N2.m_Node = -1;
					N2.m_Acc = S.m_Acc;
					float Sc = Score(N2, &TRng);
					if(AtGoal(N2.m_F))
					{
						std::lock_guard<std::mutex> Lock(GotMutex);
						SPick Pk;
						Pk.m_S = N2;
						Pk.m_S.m_Node = i; // parent beam index
						Pk.m_S.m_Score = Sc;
						Pk.m_In = In;
						vPicks.push_back(Pk);
						continue;
					}
					SCand Cd;
					Cd.m_Score = Sc;
					Cd.m_Key = CellKey(N2);
					Cd.m_Parent = i;
					Cd.m_RefIdx = N2.m_RefIdx;
					Cd.m_Acc = N2.m_Acc;
					Cd.m_In = In;
					Cand.push_back(Cd);
				}
			}
		};
		std::vector<std::thread> vTh;
		for(int t = 0; t < gs_Threads; t++)
			vTh.emplace_back(Worker, t);
		for(auto &T : vTh)
			T.join();
		if(!vPicks.empty())
		{
			std::sort(vPicks.begin(), vPicks.end(), [](const SPick &a, const SPick &b) { return a.m_S.m_Score < b.m_S.m_Score; });
			int Tried = 0;
			for(const SPick &Pk : vPicks)
			{
				if(Tried >= gs_ViaTry)
					break;
				int Rt = Pk.m_S.m_F.RaceTick();
				int Via = 0;
				if(gs_Via && !gs_GateEnd)
				{
					Tried++;
					Via = ViaTicks(gs_ViaField, Pk.m_S.m_F, Pk.m_In.m_Hook, Pk.m_In.m_Jump, gs_ViaMax, gs_ViaBeam, 48.0f);
					if(Via < 0)
						continue;
				}
				int Total = gs_GoalClimb ? Rt + Via : Rt;
				if(FoundRt < 0 || Total < FoundTotal)
				{
					FoundRt = Rt;
					FoundVia = Via;
					FoundTotal = Total;
					Found = Pk.m_S;
					SNode Nd;
					Nd.m_Parent = vBeam[Pk.m_S.m_Node].m_Node;
					Nd.m_In = Pk.m_In;
					FoundLayer = (int)vLayers.size();
					FoundNode = Nd;
					std::printf("viable pickup at race tick %d, climb point after %d more (total %d)\n", Rt, Via, Rt + Via);
					std::fflush(stdout);
				}
				if(!gs_GoalClimb)
					break;
			}
			if(FoundRt >= 0 && (!gs_GoalClimb || RtNow >= FoundRt + gs_ClimbExtra))
				break;
		}
		std::vector<SCand> vAll;
		size_t Total = 0;
		for(auto &C : vCand)
			Total += C.size();
		if(!Total)
		{
			std::printf("RESULT none (beam died at step %d)\n", Step);
			return 2;
		}
		vAll.reserve(Total);
		for(auto &C : vCand)
			vAll.insert(vAll.end(), C.begin(), C.end());
		vCand.clear();
		// dedup by cell, keep the best score per cell
		std::unordered_map<uint64_t, int> Cell;
		Cell.reserve(Total * 2);
		std::vector<int> vKeep;
		for(int i = 0; i < (int)vAll.size(); i++)
		{
			auto It = Cell.find(vAll[i].m_Key);
			if(It == Cell.end())
			{
				Cell[vAll[i].m_Key] = (int)vKeep.size();
				vKeep.push_back(i);
			}
			else if(vAll[i].m_Score < vAll[vKeep[It->second]].m_Score)
				vKeep[It->second] = i;
		}
		size_t Cells = vKeep.size();
		// re-simulate candidates in score order and check survival, in chunks, until the beam is full
		int Pool = std::min((int)vKeep.size(), gs_Survive > 0 ? 2 * gs_Beam : gs_Beam);
		std::partial_sort(vKeep.begin(), vKeep.begin() + Pool, vKeep.end(), [&](int a, int b) { return vAll[a].m_Score < vAll[b].m_Score; });
		std::vector<SState> vNew(Pool);
		std::vector<SNode> Layer(Pool);
		std::vector<uint8_t> vAlive(Pool, 0);
		std::vector<SState> vSel;
		std::vector<SNode> LayerSel;
		int Done = 0;
		while(Done < Pool && (int)vSel.size() < gs_Beam)
		{
			int Need = gs_Beam - (int)vSel.size();
			int ChunkEnd = std::min(Pool, Done + Need + Need / 8 + 64);
			std::atomic<int> Next2(Done);
			auto Worker2 = [&](int) {
				for(;;)
				{
					int k = Next2.fetch_add(1);
					if(k >= ChunkEnd)
						break;
					const SCand &Cd = vAll[vKeep[k]];
					const SState &P = vBeam[Cd.m_Parent];
					SState &N2 = vNew[k];
					N2.m_F = P.m_F;
					N2.m_F.Step(Cd.m_In);
					N2.m_RefIdx = Cd.m_RefIdx;
					N2.m_Acc = Cd.m_Acc;
					N2.m_LastHook = Cd.m_In.m_Hook;
					N2.m_LastJump = Cd.m_In.m_Jump;
					N2.m_Score = Cd.m_Score;
					N2.m_Node = k;
					Layer[k].m_Parent = P.m_Node;
					Layer[k].m_In = Cd.m_In;
					vAlive[k] = Survives(N2, gs_Survive);
				}
			};
			vTh.clear();
			for(int t = 0; t < gs_Threads; t++)
				vTh.emplace_back(Worker2, t);
			for(auto &T : vTh)
				T.join();
			for(int k = Done; k < ChunkEnd && (int)vSel.size() < gs_Beam; k++)
				if(vAlive[k])
				{
					vNew[k].m_Node = (int)vSel.size();
					LayerSel.push_back(Layer[k]);
					vSel.push_back(vNew[k]);
				}
			Done = ChunkEnd;
		}
		// incumbent: the reference run's own continuation always stays in the beam
		IncBeamIdx = -1;
		if(Step < (int)vIncF.size() && !vIncF[Step].m_Got)
		{
			SState R;
			R.m_F = vIncF[Step];
			R.m_RefIdx = vBeam.empty() ? 0 : vBeam[0].m_RefIdx;
			// track the reference index from the incumbent's own previous one
			R.m_RefIdx = IncRefIdx;
			R.m_LastHook = vIncIn[Step].m_Hook;
			R.m_LastJump = vIncIn[Step].m_Jump;
			R.m_Score = Score(R, nullptr);
			IncRefIdx = R.m_RefIdx;
			SNode Nd;
			Nd.m_Parent = IncNode;
			Nd.m_In = vIncIn[Step];
			R.m_Node = (int)vSel.size();
			IncNode = R.m_Node;
			IncBeamIdx = (int)vSel.size();
			LayerSel.push_back(Nd);
			vSel.push_back(R);
		}
		if(vSel.empty())
		{
			std::printf("RESULT none (no surviving state at step %d)\n", Step);
			return 2;
		}
		vLayers.push_back(std::move(LayerSel));
		vBeam.swap(vSel);
		const SState &B = vBeam[0];
		if(Step % 25 == 0)
		{
			double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
			std::printf("rt %d best est %.2f (ref idx %d) pos %.0f %.0f |v| %.1f cand %zu cells %zu [%.0fs]\n", B.m_F.RaceTick(), B.m_Score, B.m_RefIdx,
				B.m_F.m_Core.m_Pos.x, B.m_F.m_Core.m_Pos.y, length(B.m_F.m_Core.m_Vel), Total, Cells, Sec);
			if(!gs_vTrack.empty())
				std::printf("   target %.0f %.0f err %.1f\n", TrackAt(B.m_F.RaceTick()).x, TrackAt(B.m_F.RaceTick()).y, distance(TrackAt(B.m_F.RaceTick()), B.m_F.m_Core.m_Pos));
			std::fflush(stdout);
		}
		if(B.m_F.RaceTick() >= gs_DumpAt)
		{
			// the best-ranked state, and up to dumpk-1 more that are at least 24 px away from all dumped ones
			std::vector<int> vPick;
			for(int k = 0; k < (int)vBeam.size() && (int)vPick.size() < gs_DumpK; k++)
			{
				bool Far = true;
				for(int j : vPick)
					if(distance(vBeam[j].m_F.m_Core.m_Pos, vBeam[k].m_F.m_Core.m_Pos) < 24.0f)
						Far = false;
				if(Far)
					vPick.push_back(k);
			}
			for(int n = 0; n < (int)vPick.size(); n++)
			{
				const SState &S = vBeam[vPick[n]];
				std::vector<STasInput> vP;
				int Nd = S.m_Node;
				for(int L = (int)vLayers.size() - 1; L >= 0 && Nd >= 0; L--)
				{
					vP.push_back(vLayers[L][Nd].m_In);
					Nd = vLayers[L][Nd].m_Parent;
				}
				std::reverse(vP.begin(), vP.end());
				std::vector<STasInput> vD = vHead;
				vD.insert(vD.end(), vP.begin(), vP.end());
				std::string Name = n == 0 ? Out : Out + "." + std::to_string(n);
				WriteInputs(Name.c_str(), vD);
				std::printf("RESULT dump est %.2f at rt %d (ref idx %d) -> %s\n", S.m_Score, S.m_F.RaceTick(), S.m_RefIdx, Name.c_str());
			}
			return 0;
		}
		if(gs_vTrack.empty() && B.m_Score > gs_RefEnd + gs_Abort)
		{
			std::printf("RESULT none (abort: best estimate %.1f at rt %d)\n", B.m_Score, B.m_F.RaceTick());
			return 2;
		}
	}

	// reconstruct inputs
	std::vector<STasInput> vPath;
	vPath.push_back(FoundNode.m_In);
	int Node = FoundNode.m_Parent;
	for(int L = FoundLayer - 1; L >= 0 && Node >= 0; L--)
	{
		vPath.push_back(vLayers[L][Node].m_In);
		Node = vLayers[L][Node].m_Parent;
	}
	std::reverse(vPath.begin(), vPath.end());
	std::vector<STasInput> vOut = vHead;
	vOut.insert(vOut.end(), vPath.begin(), vPath.end());

	// verify on the plain DDNet prediction code (fast collision paths off)
	CCollision::ms_FastPaths = false;
	CTasGame V;
	V.Spawn(CTasGame::Map().m_vSpawns[0]);
	int VerRt = -1;
	bool Bad = false;
	for(size_t i = 0; i < vOut.size(); i++)
	{
		V.Step(vOut[i]);
		if(V.Frozen() || V.EnteredFreeze() || V.m_StartTick == -2)
		{
			Bad = true;
			break;
		}
		float VLr = length(gs_GeoVRef);
		bool VerGoal = gs_Geo ? (i + 1 == vOut.size() && distance(V.Pos(), gs_GeoGoal) < gs_GeoR && length(V.Vel()) >= gs_GeoVMin &&
						(VLr <= 0 || dot(V.Vel(), gs_GeoVRef / VLr) >= VLr - gs_GeoVTol) && (!gs_GeoJump || !(V.Jumped() & 2))) :
				   gs_GateEnd ? (i + 1 == vOut.size() && V.HasGrenade() && distance(V.Pos(), gs_GatePos) < gs_GateR) : V.HasGrenade();
		if(VerGoal)
		{
			VerRt = V.m_Tick - V.m_StartTick;
			break;
		}
	}
	double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
	std::printf("RESULT %d verified %d (bad %d) via %d total %d cut %d [%.0fs]\n", FoundRt, VerRt, Bad, FoundVia, FoundRt + FoundVia, Cut, Sec);
	if(!Bad && VerRt == FoundRt)
	{
		WriteInputs(Out.c_str(), vOut);
		std::printf("wrote %s\n", Out.c_str());
	}
	return 0;
}
