// seg: segment search after the start line, guided by a reference line (Teero's track).
// Energy beam like `pre` (per-cell dedup + global dominance), but progress is the position along the reference
// line, so it works on any part of the route. States are ranked by the estimated total race time
//   race_tick + T_rem(s, E_eff) + H * (1/v_now - 1/v_E)
// where T_rem(s, E) is the time from arc position s to the gate along the reference line at the speed the energy
// gives there (v = sqrt(E + y_ref), displacement ramp applied), v_now the current speed along the line and H = hnow.
//
// usage: seg <map> prefix=FILE ref=teero_track.txt key=value...
//   gate=K (Teero tick on the reference line) | gate=grenade (pickup) | gate=finish     out=PREFIX (writes PREFIX0.txt)
//   gate=box:x0,x1,y0,y1[,vymax[,K]] (first tick inside the box with the grenade and vy <= vymax; earliest wins;
//     K = Teero tick of the box for the time model, default: the reference point nearest the box centre)
//   beam=20000 threads=1 maxticks=600 angles=64 hnow=300 dirmode=all|tan (tan: only hold towards the line's x)
//   pjc=130 pgc=160 (energy credit for an unused air / ground jump)  gcred=0 (credit for a loaded grenade)
//   fire=1 fireangles=48 firelook=12 (grenade shots, judged after the explosion)  survive=20
//   ghost=1|2 (2: the speed term compares our time over the next hnow px with the reference's own time there)
//   ghost=3 vcap=1.25 vmin=8 brake=3 brakepen=3: like 2, but our speed over the next hnow px is capped at vcap x the
//     reference's local speed, and states that can't brake to those caps in time are penalized
//   track=W [trackoff=N trackcap=96 tracktie=0.05]: rank by W x accumulated squared distance (px^2/1000, capped) to
//     Teero's position at race tick + trackoff (default: the start state's offset), + tracktie x the time model
//   prefire=1: shots still in flight after firelook ticks are judged as if not fired (no held-input look), and
//     states are told apart by their pending explosion (use with padaims=N for long pre-fire aims)
//   padref=R: also keep pre-fire aims whose explosion lands within R px of the reference line ~that many ticks ahead (turns)
//   plan=FILE [planforce=1]: shot plan, lines "t0 t1 ex ey [r]" (fire on race ticks t0..t1 so that the grenade
//     explodes within r px of (ex,ey); exact aims, pre-fires included) or "t0 t1 free" (point-blank shots allowed);
//     no other shots; planforce drops states that skipped a target shot (use with prefire=1, padtop for the states);
//     optional explosion race tick: "t0 t1 ex ey r te tol"; planrv=W [planacc=1.5]: credit up to 4W ticks for a grenade
//     in flight whose explosion point the tee can still reach (rendezvous); planref=1: t0 t1 are Teero ticks on the
//     reference line (the shot is allowed where he fired), te is the flight time in ticks; planfree=1 [planlock=25]:
//     ordinary point-blank shots also outside the plan windows, except in the planlock ticks before a target shot
//   jitter=J seed=N: +-J ticks of deterministic per-state score noise (different seeds = different searches)
//   crashw=W [crashn=8]: penalty W ticks per unit of v^2 lost against solid tiles over the next N ticks (current
//     inputs, hook released)
//   firealign=c: only grenade shots whose kick (opposite to the aim) has cos >= c with the line direction
//   quota=Q [qcell=32 qvel=6]: beam diversity, at most Q states per coarse position/velocity cell first, then the best
//     of the rest (keeps the beam alive in maze sections; physlab4)
//   shotplan=teero/catalog/shots.tsv [planpre=8 planpost=6 plangap=25 planr=96]: keep the reload for Teero's kick
//     slots: shots only near his slots, when his next slot is >= plangap ticks away, or as pre-fires for it
//   shotref=teero/catalog/shots.tsv shotbonus=3 shotrad=64: a shot whose explosion lands within shotrad px of one of
//     Teero's explosion points (later than the last one matched) earns shotbonus ticks
//   erel=1: (ghost 1-3) the energy credit counts our energy minus the reference's own energy at the matched point
//   kickmin=K (segf only): drop states in which a grenade explodes with a kick under K px/t (max 12: explosion within
//     48 px; 10 ~ within 62 px) - get close to the surface first (Teero's mean kick is 11.7, ours was 10.6)
//   pfcred=W (segf, with prefire=1 padaims=N): a pre-fire is credited W x the time its explosion saves on a held-input
//     look (look with the grenade vs the same look without it) until it explodes
//   retro=K [retrominf=2 retromaxf=30 retrotop=all retrorad=90] (segf): retro shots - a grenade's flight doesn't depend
//     on the tee, so for each state the search looks back along its own history for a tick where it held a loaded
//     grenade (no shot since, no hook start that tick) from which some aim explodes next tick right next to it; the
//     shot is patched into that ancestor's input (exact: the tee's motion until the explosion is unchanged) and the
//     state with the grenade in flight is expanded too (up to K best kicks). Pre-fires / double kicks without guessing.
//     retroafter=1: also for states that fired since (the retro shot 25+ ticks before that shot, exploding after it)
//   loadres=F: reserve F x beam places for the best-ranked states that hold a loaded grenade (horizon effect: a kick
//     saved for the next bend looks worse than firing now until the bend)
//   firemax=N: drop states that keep a loaded grenade (reload 0) for more than N ticks (kick as often as possible)
//   kfut=W [kfutn=25]: while the reload is in (kready, kfutn], credit W x the point-blank kick available where a
//     ballistic flight puts the tee when the reload is back (be next to a surface when the grenade is loaded)
//   kcredit=0 kready=4: energy credit (x kcredit) for the best point-blank kick along the line when reload <= kready
//   ghost=4 [e4w=1]: Teero's remaining time + (time to the next sink at our energy - same at Teero's own energy
//     there, from the energy table) + hnow x (1/v_now - 1/v_energy); needs sinks=
//   inc=FILE: incumbent (a run from spawn whose first inputs are the prefix): its continuation is kept in the beam
//     every step (so with gate=finish the result is never later than it); SEG_TRACK=FILE (env): write the prefix's
//     positions as a reference track ("k x y", k = race tick + 3 like Teero's labels) and exit
//   latpen=0 latdz=8 latk0= latk1=: ticks of penalty per px of distance from the line beyond latdz (Teero ticks latk0..latk1)
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "sim.h"
#ifdef SEG_FAST
#include "tasfast.h"
using CGameT = CTasFast;
#else
using CGameT = CTasGame;
#endif

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <cctype>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
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

static void WriteInputs(const char *pPath, const std::vector<STasInput> &v)
{
	FILE *f = std::fopen(pPath, "w");
	if(!f)
		return;
	for(const auto &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
}

struct SParams
{
	int m_Beam = 20000;
	int m_Angles = 64;
	int m_MaxTicks = 600;
	int m_Threads = 1;
	float m_CPos = 16, m_CVel = 2;
	int m_Dom = 1;
	std::string m_Out = "runs/seg/s";
	std::string m_Prefix;
	std::string m_Ref = "teero_track.txt";
	std::string m_Gate = "finish";
	int m_GateK = -1;
	float m_HNow = 300;
	int m_DirTan = 0;
	float m_PJC = 130, m_PGC = 160;
	float m_GCred = 0;
	int m_Fire = 1;
	int m_FireAngles = 32;
	float m_FireRange = 120; // only shots that hit a solid tile this close (point-blank kicks)
	int m_FireAllDirs = 0;
	std::string m_Plan; // shot plan file
	int m_PlanForce = 0;
	int m_PlanRef = 0;
	float m_Jitter = 0; int m_Seed = 0; // jitter=J seed=N: uniform +-J ticks of score noise (stochastic beam)
	float m_CrashW = 0; int m_CrashN = 8; // crashw=W crashn=N: W ticks per unit of v^2 lost to collisions in the next N ticks
	int m_PlanFree = 0, m_PlanLock = 25; // planfree=1: point-blank shots also outside the plan, except planlock ticks before a target shot // planref=1: plan windows in Teero ticks on the reference line, te = flight time
	float m_PlanRv = 0, m_PlanAcc = 1.5f; // rendezvous credit for grenades in flight (planrv=, planacc=)
	float m_FireAlign = -2; // firealign=c: only shots whose kick has cos >= c with the line direction (-2 = off) // shots with every direction input (default: only towards the line)
	int m_PendLook = 1; // judge every state with a grenade in flight after its explosion (pre-fired shots)
	int m_PadAims = 0; // coarse angle count for edge-refined (pre-fire) shot aims, 0 = off
	int m_PadTop = 300; // only the best this many beam states get them
	int m_PadRange = 30; // pre-fired shots may explode up to this many ticks later
	float m_PadRef = 0; // also keep pre-fire aims exploding within this many px of the reference line ~that many ticks ahead
	int m_SurvEvery = 2; // every this many steps, beam states that can't survive `survive` ticks stop breeding (0 = off)
	int m_FireLook = 12;
	int m_Survive = 20;
	float m_HookRange = 420;
	int m_Quant = 0; // speed terms use the whole-pixel displacement per tick (positions are rounded every tick)
	std::string m_Inc; // incumbent run (inputs from spawn, starting with the prefix): its continuation always stays in the beam
	std::string m_Imit; // Teero's per-frame inputs (csv: s_since_start, dir, jump_arrow, aim_tx, aim_ty): restrict
	                    // dir / jump / hook aims to his within +-ImitW ticks (hook press timing stays free)
	float m_ImitOff = -3.2f; // race tick = s_since_start * 50 + ImitOff
	int m_ImitW = 2;
	int m_ImitHooks = 0; // also keep the normal hook targets
	int m_ImitDir = 1; // restrict dir (0: all dirs)
	int m_RotHook = 0; // add rotation-pulse hook aims (first-tick grab, boundary of |v| growth, turn towards the reference tangent)
	int m_HookLA = 0; // score a flying / held hook by its predicted first applied pull within this many ticks
	float m_HookIdle = 0.02f; // ... a hook predicted to never pull costs this (ticks)
	int m_HookDedup = 1; // keep one hook aim per hit tile
	int m_Prefire = 0; // judge shots still in flight neutrally, separate states by their explosion point
	std::string m_ShotRef; // Teero's shot catalog (tsv)
	float m_ShotBonus = 3, m_ShotRad = 64;
	std::string m_ShotPlan; // Teero's shot catalog: keep the reload for his kick slots
	float m_PlanPre = 8, m_PlanPost = 6, m_PlanGap = 25, m_PlanR = 96;
	int m_GateWait = 2;
	float m_TrackBack = 0, m_TrackFwd = 30;
	int m_Horizon = 300; // the time model looks this many Teero ticks past the gate
	std::string m_Sinks; // comma-separated Teero ticks of energy sinks
	int m_CommitK = -1; // also write OUTc.txt: the result cut at the first input that reaches this Teero tick
	int m_Ghost = 0; // time model: the reference's own remaining time (+ current speed over hnow px)
	float m_GhostE = 0; // ghost model: ticks of credit per unit of energy
	float m_GhostSink = 0; // ghost model: the energy credit fades over this many px before the next sink
	int m_Quiet = 0;
	float m_VCap = 1.25f, m_VMin = 8, m_Brake = 3, m_BrakePen = 3; // ghost=3
	float m_GhostE4 = 1; // ghost=4: weight of the energy time difference
	float m_LatPen = 0, m_LatDz = 8, m_LatK0 = -1e9f, m_LatK1 = 1e9f; // distance-from-line penalty
	float m_TrackW = 0; // tracking mode: rank by accumulated squared distance to Teero's position at the same (offset) tick
	int m_TrackOff = -100000; // Teero tick minus our race tick (default: from the start state)
	float m_TrackCap = 96, m_TrackTie = 0.05f;
	int m_Quota = 0; // diversity quota per coarse cell (0 = off; physlab4)
	float m_QCell = 32, m_QVel = 6;
	float m_KCredit = 0; // energy credit for the point-blank kick available when the grenade is (nearly) loaded
	int m_KReady = 4;
	float m_KickMin = 0; // kickmin=K (segf): drop states in which a grenade explodes with a kick weaker than K px/t
	int m_Retro = 0; // retro=K (segf): up to K retro shots per state (fired by an ancestor holding the grenade)
	int m_RetroMinF = 2, m_RetroMaxF = 30, m_RetroTop = 1000000, m_RetroAfter = 0; // retroafter: also behind the state's own last shot
	float m_RetroRad = 90;
	float m_PfCred = 0; // pfcred=W (segf, prefire=1): pre-fire credit from held-input looks with / without the grenade
	float m_LoadRes = 0; // loadres=F: reserve F x beam places for the best states holding a loaded grenade
	float m_BoxE = 0; // boxe=L: box gate value = race tick - L x (v^2 - y) (L large: most energy at the box)
	float m_ERel = 0; // erel=1: ghost energy credit relative to the reference's own energy at the matched point
	int m_FireMax = -1; // firemax=N: drop states that keep a loaded grenade for more than N ticks
	float m_KFut = 0; // kfut=W: credit W x the kick available where the tee flies to by the time the reload is back
	int m_KFutN = 25; // (only while the reload is <= kfutn and > kready)
	int m_TpK = -1; // diagnostics: teleport after the prefix (tp=x,y,vx,vy tpk=K tpreload=N)
	int m_TpReload = -1;
	int m_TpLine = -1; // tpline=N: teleport after N prefix lines, then replay the rest of the prefix
	vec2 m_TpPos, m_TpVel;
};
static SParams gs_P;

#ifdef SEG_FAST
static const CCharacterCore &CoreOf(const CTasFast &G) { return G.m_Core; }
#else
static const CCharacterCore &CoreOf(const CTasGame &G) { return G.Chr()->m_Core; }
#endif

static int JumpsLeft(const CGameT &G) { return G.Grounded() ? 2 : ((G.Jumped() & 2) ? 0 : 1); }

// ---- reference line ----
struct SRef
{
	std::vector<vec2> m_vP; // smoothed points
	std::vector<float> m_vK; // Teero tick of each point
	std::vector<float> m_vL; // cumulative arc length
	int m_GateIdx = 0; // end of the segment (search gate)
	int m_EndIdx = 0; // end of the time model (gate + horizon)
	std::vector<int> m_vSinkIdx; // energy sinks (U-turns): energy only counts up to the next one
	// time-to-gate table over (point, energy)
	static constexpr float E0 = -2600, DE = 10;
	static constexpr int NE = 700;
	std::vector<float> m_vT;
	float Disp(float v) const { return v > 11 ? v * std::pow(1.4f, -(50 * v - 550) / 2000.0f) : v; }
	bool Load(const char *pPath)
	{
		std::vector<std::pair<int, vec2>> vRaw;
		FILE *f = std::fopen(pPath, "r");
		if(!f)
			return false;
		char aLine[256];
		while(std::fgets(aLine, sizeof(aLine), f))
		{
			int t;
			float x, y;
			if(std::sscanf(aLine, "%d %f %f", &t, &x, &y) == 3 && t >= 0)
				vRaw.push_back({t, vec2(x, y)});
		}
		std::fclose(f);
		const int N = vRaw.size();
		for(int i = 0; i < N; i++)
		{
			vec2 S(0, 0);
			int n = 0;
			for(int k = -3; k <= 3; k++)
			{
				int j = std::clamp(i + k, 0, N - 1);
				S += vRaw[j].second;
				n++;
			}
			m_vP.push_back(S / (float)n);
			m_vK.push_back((float)vRaw[i].first);
		}
		m_vL.assign(N, 0);
		for(int i = 1; i < N; i++)
			m_vL[i] = m_vL[i - 1] + distance(m_vP[i - 1], m_vP[i]);
		return N > 2;
	}
	void Build(int GateIdx, int EndIdx)
	{
		m_GateIdx = std::clamp(GateIdx, 1, (int)m_vP.size() - 1);
		m_EndIdx = std::clamp(EndIdx, m_GateIdx, (int)m_vP.size() - 1);
		const int N = m_vP.size();
		const int TabEnd = N - 1;
		m_vT.assign((size_t)N * NE, 0);
		for(int e = 0; e < NE; e++)
		{
			float E = E0 + e * DE;
			float Acc = 0;
			for(int i = N - 1; i >= 0; i--)
			{
				if(i < TabEnd)
				{
					float Ds = m_vL[i + 1] - m_vL[i];
					float Y = 0.5f * (m_vP[i].y + m_vP[i + 1].y);
					Acc += Ds / Disp(std::sqrt(std::max(E + Y, 25.0f)));
				}
				m_vT[(size_t)i * NE + e] = Acc;
			}
		}
	}
	// closest point near Idx: returns the segment index and the fraction along it
	// the velocity breaks ties where the line runs back along itself (narrow shafts): segments that point
	// against the motion are penalized
	int Track(int Idx, vec2 P, vec2 Vel, float &Frac) const
	{
		const float Sp = length(Vel);
		const float W = 40.0f * std::min(Sp / 8.0f, 1.0f);
		const int N = m_vP.size();
		int Lo = std::max(0, Idx - (int)gs_P.m_TrackBack), Hi = std::min(N - 2, Idx + (int)gs_P.m_TrackFwd);
		float Best = 1e30f;
		int BestJ = Idx;
		Frac = 0;
		for(int j = Lo; j <= Hi; j++)
		{
			vec2 A = m_vP[j], B = m_vP[j + 1], AB = B - A;
			float L2 = dot(AB, AB);
			float t = L2 > 1e-6f ? std::clamp(dot(P - A, AB) / L2, 0.0f, 1.0f) : 0.0f;
			float d = distance(A + AB * t, P);
			if(W > 0 && L2 > 1e-6f)
				d += W * (1.0f - dot(AB, Vel) / (std::sqrt(L2) * Sp));
			if(d < Best - 1e-3f)
			{
				Best = d;
				BestJ = j;
				Frac = t;
			}
		}
		return BestJ;
	}
	// first point where the energy part of the estimate stops: the next sink after I, or the evaluation end
	int EnergyEnd(int I) const
	{
		for(int S : m_vSinkIdx)
			if(S > I)
				return std::min(S, m_EndIdx);
		return m_EndIdx;
	}
	float TAt(int I, float E) const
	{
		I = std::clamp(I, 0, (int)m_vP.size() - 1);
		float fe = std::clamp((E - E0) / DE, 0.0f, (float)NE - 1.001f);
		int e = (int)fe;
		float b = fe - e;
		return (1 - b) * m_vT[(size_t)I * NE + e] + b * m_vT[(size_t)I * NE + e + 1];
	}
	float TRem(int I, float Frac, float E) const
	{
		const int N = m_vP.size();
		I = std::clamp(I, 0, N - 2);
		float fe = std::clamp((E - E0) / DE, 0.0f, (float)NE - 1.001f);
		int e = (int)fe;
		float b = fe - e;
		auto At = [&](int ii) { return (1 - b) * m_vT[(size_t)ii * NE + e] + b * m_vT[(size_t)ii * NE + e + 1]; };
		return (1 - Frac) * At(I) + Frac * At(I + 1);
	}
	std::vector<float> m_vET; // ghost=4: the reference's own energy per point (from its smoothed speed)
	void PrepET()
	{
		const int N = m_vP.size();
		std::vector<float> vRaw(N);
		for(int j = 0; j < N; j++)
		{
			float v = InvDisp(VRef(j));
			vRaw[j] = v * v - m_vP[j].y;
		}
		m_vET.resize(N);
		for(int j = 0; j < N; j++)
		{
			float S = 0;
			int n = 0;
			for(int d = -5; d <= 5; d++)
			{
				int i = std::clamp(j + d, 0, N - 1);
				S += vRaw[i];
				n++;
			}
			m_vET[j] = S / n;
		}
	}
	std::vector<float> m_vVc, m_vVcv; // ghost=3: displacement cap and the matching speed per point
	void PrepCaps(float VCap, float VMin)
	{
		const int N = m_vP.size();
		m_vVc.resize(N);
		m_vVcv.resize(N);
		for(int j = 0; j < N; j++)
		{
			m_vVc[j] = std::max(VCap * VRef(j), VMin);
			m_vVcv[j] = InvDisp(m_vVc[j]);
		}
	}
	// speed whose displacement per tick is D
	float InvDisp(float D) const
	{
		float Lo = 0, Hi = 200;
		for(int i = 0; i < 40; i++)
		{
			float M = 0.5f * (Lo + Hi);
			if(Disp(M) < D)
				Lo = M;
			else
				Hi = M;
		}
		return Lo;
	}
	// reference speed (px per tick) around point I, smoothed over +-2 points
	float VRef(int I) const
	{
		const int N = m_vL.size();
		int A = std::clamp(I - 2, 0, N - 1), B = std::clamp(I + 3, 0, N - 1);
		return B > A ? (m_vL[B] - m_vL[A]) / (m_vK[B] - m_vK[A]) : 10.0f;
	}
	// reference tick at arc length L (clamped to the line)
	float KAtL(float L) const
	{
		const int N = m_vL.size();
		if(L >= m_vL[N - 1])
			return m_vK[N - 1];
		int J = std::upper_bound(m_vL.begin(), m_vL.end(), L) - m_vL.begin();
		J = std::clamp(J, 1, N - 1);
		float Seg = m_vL[J] - m_vL[J - 1];
		float t = Seg > 1e-4f ? (L - m_vL[J - 1]) / Seg : 0.0f;
		return m_vK[J - 1] + t * (m_vK[J] - m_vK[J - 1]);
	}
	vec2 Tangent(int I) const
	{
		I = std::clamp(I, 0, (int)m_vP.size() - 2);
		vec2 D = m_vP[I + 1] - m_vP[I];
		float l = length(D);
		return l > 1e-3f ? D / l : vec2(1, 0);
	}
	float SpeedAt(int I, float E) const
	{
		I = std::clamp(I, 0, (int)m_vP.size() - 1);
		return Disp(std::sqrt(std::max(E + m_vP[I].y, 25.0f)));
	}
};
static SRef gs_Ref;
static int gs_GrenIdx = 1 << 30; // reference point of the grenade pickup

// exact broad phase: true only if no solid / unhookable tile lies within the box P +- R (false when unsure)
static bool NoSolidNear(vec2 P, float R)
{
	const CCollision *pC = CTasGame::Collision();
	if(!pC->m_FastOk || !CCollision::ms_FastPaths)
		return false;
	const SMapInfo &M = CTasGame::Map();
	if(P.x - R < 64 || P.y - R < 64 || P.x + R > M.m_W * 32 - 64 || P.y + R > M.m_H * 32 - 64)
		return false; // SMapInfo::Tile treats the outside of the map as solid
	return !pC->FastAnySolid(P.x - R, P.y - R, P.x + R, P.y + R);
}

// energy a point-blank kick could add now (best of 32 aims whose shot hits a solid tile within ~90 px; kicks
// against the line's direction don't count)
static float KickPotentialAt(vec2 P, vec2 V, vec2 Tg)
{
	const SMapInfo &M = CTasGame::Map();
	if(NoSolidNear(P, 92.0f)) // the rays below end within 89 px
		return 0;
	static const std::vector<vec2> s_vDir = [] {
		std::vector<vec2> v;
		for(int a = 0; a < 32; a++)
		{
			float Ang = 2 * pi * a / 32;
			v.emplace_back(std::cos(Ang), std::sin(Ang));
		}
		return v;
	}();
	float Sp = length(V);
	float Ramp = Sp * 50 > 550 ? std::pow(1.4f, -(Sp * 50 - 550) / 2000.0f) : 1.0f;
	vec2 Q = P + V * Ramp; // where the tee is when the explosion is applied
	float Best = 0;
	for(int a = 0; a < 32; a++)
	{
		const vec2 D = s_vDir[a];
		vec2 S0 = P + D * 21.0f;
		vec2 E;
		bool Hit = false;
		for(float r = 0; r < 70.0f; r += 4.0f)
		{
			E = S0 + D * r;
			int t = M.Tile((int)std::floor(E.x / 32), (int)std::floor(E.y / 32));
			if(t == TILE_SOLID || t == TILE_NOHOOK)
			{
				Hit = true;
				break;
			}
		}
		if(!Hit)
			continue;
		vec2 Diff = Q - E;
		float l = length(Diff);
		if(l < 1e-3f)
			continue;
		float Str = 6.0f * (1.0f - std::clamp((l - 48.0f) / 87.0f, 0.0f, 1.0f));
		if(Str < 1.0f)
			continue;
		vec2 K = Diff / l * Str * 2.0f;
		if(dot(K, Tg) <= 0)
			continue;
		vec2 V2 = V + K;
		Best = std::max(Best, dot(V2, V2) - Sp * Sp);
	}
	return Best;
}
static float KickPotential(const CGameT &G) { return KickPotentialAt(G.Pos(), G.Vel(), gs_Ref.Tangent(G.m_RefIdx)); }

// kfut: the point-blank kick available where a ballistic flight (velocity ramp, gravity, no collisions) puts the tee
// when the reload is back (0 if that point is inside a solid tile)
static float FutureKick(const CGameT &G)
{
	const int R = G.ReloadTimer();
	vec2 P = G.Pos(), V = G.Vel();
	for(int t = 0; t < R; t++)
	{
		V.y += 0.5f;
		float Sp = length(V);
		float Ramp = Sp * 50 > 550 ? std::pow(1.4f, -(Sp * 50 - 550) / 2000.0f) : 1.0f;
		P += V * Ramp;
	}
	const SMapInfo &M = CTasGame::Map();
	if(M.Tile((int)std::floor(P.x / 32), (int)std::floor(P.y / 32)) == TILE_SOLID || M.Tile((int)std::floor(P.x / 32), (int)std::floor(P.y / 32)) == TILE_NOHOOK)
		return 0;
	float Frac;
	int I = gs_Ref.Track(G.m_RefIdx, P, V, Frac);
	return KickPotentialAt(P, V, gs_Ref.Tangent(I));
}

static float EffEnergy(const CGameT &G)
{
	vec2 V = G.Vel();
	float E = dot(V, V) - G.Pos().y;
	if(gs_P.m_KCredit > 0 && G.HasGrenade() && G.ReloadTimer() <= gs_P.m_KReady)
		E += gs_P.m_KCredit * KickPotential(G) * (1.0f - (float)G.ReloadTimer() / (gs_P.m_KReady + 1));
	else if(gs_P.m_KFut > 0 && G.HasGrenade() && G.ReloadTimer() <= gs_P.m_KFutN)
		E += gs_P.m_KFut * FutureKick(G);
	int J = JumpsLeft(G);
	if(J >= 1)
		E += gs_P.m_PJC;
	if(J == 2)
		E += gs_P.m_PGC;
	if(gs_P.m_GCred > 0 && G.HasGrenade())
		E += gs_P.m_GCred * (1.0f - std::min(G.ReloadTimer(), 25) / 25.0f);
	return E;
}

// optional penalty for the distance from the reference line (ticks per px beyond latdz, only between Teero ticks latk0..latk1)
static float LatPen(const CGameT &G, int I, float Frac)
{
	if(gs_P.m_LatPen <= 0)
		return 0;
	const int N = gs_Ref.m_vP.size();
	if(gs_Ref.m_vK[I] < gs_P.m_LatK0 || gs_Ref.m_vK[I] > gs_P.m_LatK1)
		return 0;
	vec2 A = gs_Ref.m_vP[I], B = gs_Ref.m_vP[std::min(I + 1, N - 1)];
	float d = distance(G.Pos(), A + (B - A) * Frac);
	return gs_P.m_LatPen * std::max(0.0f, d - gs_P.m_LatDz);
}

// reference shots (Teero's catalog): a shot of ours that explodes near one of his explosion points gets a bonus
static std::vector<vec2> gs_vShotRef;
static void LoadShotRef(const char *pPath)
{
	FILE *f = std::fopen(pPath, "r");
	if(!f)
		return;
	char aLine[1024];
	while(std::fgets(aLine, sizeof(aLine), f))
	{
		int n;
		float k, gap, x, y;
		// n, race_tick, gap (may be empty), tile x, tile y
		if(std::sscanf(aLine, "%d\t%f\t%f\t%f\t%f", &n, &k, &gap, &x, &y) == 5)
			gs_vShotRef.push_back(vec2(x * 32, y * 32));
		else if(std::sscanf(aLine, "%d\t%f\t\t%f\t%f", &n, &k, &x, &y) == 4)
			gs_vShotRef.push_back(vec2(x * 32, y * 32));
	}
	std::fclose(f);
}
static void ShotBonus(const CGameT &Parent, CGameT &G, const STasInput &In)
{
	if(gs_vShotRef.empty() || !In.m_Fire || Parent.ReloadTimer() > 0 || G.ReloadTimer() == 0)
		return;
	vec2 E;
	int Te;
	if(!G.NextExplosion(E, Te))
		return;
	for(int i = std::max(G.m_LastShot + 1, 0); i < (int)gs_vShotRef.size(); i++)
		if(distance(E, gs_vShotRef[i]) < gs_P.m_ShotRad)
		{
			G.m_Bonus += gs_P.m_ShotBonus;
			G.m_LastShot = i;
			return;
		}
}

// shot plan (Teero's catalog): a shot is allowed near one of his kick slots (our reference tick within
// [k - planpre, k + planpost]), when his next kick is at least plangap reference ticks away (the reload is back by
// then), or when it explodes within planr px of his next explosion point (a pre-fire for that kick)
struct SPlanShot
{
	float m_K;
	vec2 m_E;
};
static std::vector<SPlanShot> gs_vPlan;
static void LoadShotPlan(const char *pPath)
{
	FILE *f = std::fopen(pPath, "r");
	if(!f)
	{
		std::printf("cannot read %s\n", pPath);
		std::exit(1);
	}
	char aLine[1024];
	while(std::fgets(aLine, sizeof(aLine), f))
	{
		// tab-separated: n, race_tick, gap (may be empty), expl_tile_x, expl_tile_y, ...
		std::vector<std::string> vF;
		std::string Cur;
		for(const char *c = aLine; *c && *c != '\n'; c++)
		{
			if(*c == '\t')
			{
				vF.push_back(Cur);
				Cur.clear();
			}
			else
				Cur += *c;
		}
		vF.push_back(Cur);
		if(vF.size() < 5 || vF[0].empty() || !std::isdigit((unsigned char)vF[0][0]))
			continue;
		gs_vPlan.push_back({std::stof(vF[1]), vec2(std::stof(vF[3]) * 32, std::stof(vF[4]) * 32)});
	}
	std::fclose(f);
}
static bool ShotPlanOk(const CGameT &Tmp, const STasInput &In, bool Fired)
{
	if(gs_vPlan.empty() || !In.m_Fire || !Fired)
		return true;
	const float K = gs_Ref.m_vK[std::clamp(Tmp.m_RefIdx, 0, (int)gs_Ref.m_vK.size() - 1)];
	for(const auto &P : gs_vPlan)
	{
		if(P.m_K < K - gs_P.m_PlanPost)
			continue;
		// P = the next kick slot not yet passed
		if(K >= P.m_K - gs_P.m_PlanPre || P.m_K - K >= gs_P.m_PlanGap)
			return true;
		vec2 E;
		int Te;
		return Tmp.NextExplosion(E, Te) && distance(E, P.m_E) < gs_P.m_PlanR;
	}
	return true;
}

// tracking mode: squared distance (capped) to Teero's smoothed position at our race tick + trackoff
static std::vector<int> gs_vTickIdx; // Teero tick -> reference point
static float TrackInc(const CGameT &G)
{
	if(gs_P.m_TrackW <= 0 || !G.m_Started)
		return 0;
	int K = G.m_Tick - G.m_StartTick + gs_P.m_TrackOff;
	if(K < 0 || K >= (int)gs_vTickIdx.size() || gs_vTickIdx[K] < 0)
		return 0;
	float d = std::min(distance(G.Pos(), gs_Ref.m_vP[gs_vTickIdx[K]]), gs_P.m_TrackCap);
	return gs_P.m_TrackW * d * d / 1000.0f;
}

// estimated total race time of a (started) state; lower is better
// current progress per tick along the tangent
static float AlongSpeed(vec2 V, vec2 Tg, float MinV)
{
	float Sp = length(V);
	if(gs_P.m_Quant)
	{
		float Rm = Sp > 11 ? gs_Ref.Disp(Sp) / Sp : 1.0f;
		vec2 D(std::round(V.x * Rm), std::round(V.y));
		return std::max(dot(D, Tg), MinV);
	}
	float Along = Sp > 1e-3f ? dot(V, Tg) / Sp : 0.0f;
	return std::max(gs_Ref.Disp(Sp) * Along, MinV);
}

static float EstTotal(const CGameT &G, float *pEe = nullptr)
{
	float Frac;
	int I = gs_Ref.Track(G.m_RefIdx, G.Pos(), G.Vel(), Frac);
	float Ee = EffEnergy(G);
	if(pEe)
		*pEe = Ee;
	if(gs_P.m_Ghost)
	{
		// race the reference: its remaining time from our point, the next H px at our own speed
		const int N = gs_Ref.m_vK.size();
		float K = gs_Ref.m_vK[I] + Frac * (gs_Ref.m_vK[std::min(I + 1, N - 1)] - gs_Ref.m_vK[I]);
		float T = gs_Ref.m_vK[gs_Ref.m_EndIdx] - K;
		vec2 V = G.Vel();
		float Sp = length(V);
		vec2 Tg = gs_Ref.Tangent(I);
		float Along = Sp > 1e-3f ? dot(V, Tg) / Sp : 0.0f;
		float Vn = AlongSpeed(V, Tg, 3.0f);
		if(gs_P.m_Ghost == 4)
		{
			// energy relative to the reference's own energy here, worth its time difference up to the next sink
			int J = gs_Ref.EnergyEnd(I);
			if(J > I)
			{
				float Et = gs_Ref.m_vET[I] + Frac * (gs_Ref.m_vET[std::min(I + 1, N - 1)] - gs_Ref.m_vET[I]);
				float Tours = gs_Ref.TRem(I, Frac, Ee) - gs_Ref.TAt(J, Ee);
				float Tref = gs_Ref.TRem(I, Frac, Et) - gs_Ref.TAt(J, Et);
				T += gs_P.m_GhostE4 * (Tours - Tref);
				float Ve = gs_Ref.SpeedAt(I, Ee);
				float Rest = gs_Ref.m_vL[J] - (gs_Ref.m_vL[I] + Frac * (gs_Ref.m_vL[std::min(I + 1, N - 1)] - gs_Ref.m_vL[I]));
				float H = std::min(gs_P.m_HNow, std::max(Rest, 0.0f));
				T += H * (1.0f / Vn - 1.0f / std::max(Ve, 3.0f));
			}
			return (float)(G.m_Tick - G.m_StartTick) + T + LatPen(G, I, Frac);
		}
		if(gs_P.m_Ghost == 3)
		{
			// the next hnow px at our speed, but never faster than vcap x the reference's own speed there; states
			// that can't brake (at `brake` px/t^2) to those caps in time pay `brakepen` ticks per px/t of excess
			float L0 = gs_Ref.m_vL[I] + Frac * (gs_Ref.m_vL[std::min(I + 1, N - 1)] - gs_Ref.m_vL[I]);
			const float Vb = gs_Ref.Disp(Sp);
			float Tn = 0, Dist = 0, Vallow = 1e9f;
			for(int j = I; j < N - 1 && Dist < gs_P.m_HNow; j++)
			{
				float A = std::max(gs_Ref.m_vL[j], L0), B = gs_Ref.m_vL[j + 1];
				float Ds = std::min(B - A, gs_P.m_HNow - Dist);
				if(Ds <= 0)
					continue;
				const float Vc = gs_Ref.m_vVc[j];
				Tn += Ds / std::min(Vn, Vc);
				// braking in velocity space: ticks to get from |v| down to the speed whose displacement is Vc,
				// travelling about the mean displacement meanwhile
				const float Vcv = gs_Ref.m_vVcv[j];
				if(Sp > Vcv)
				{
					float Tb = (Sp - Vcv) / gs_P.m_Brake;
					float Db = Tb * 0.5f * (Vb + Vc);
					if(Db > Dist)
						Vallow = std::min(Vallow, Vb - (Db - Dist) / std::max(Tb, 1.0f));
				}
				Dist += Ds;
			}
			T = gs_Ref.m_vK[gs_Ref.m_EndIdx] - gs_Ref.KAtL(L0 + Dist) + Tn;
			if(Vallow < 1e8f)
				T += gs_P.m_BrakePen * (Vb - Vallow);
		}
		else if(gs_P.m_Ghost == 2)
		{
			// the next hnow px at our speed instead of the reference's own time over the same stretch
			float L = gs_Ref.m_vL[I] + Frac * (gs_Ref.m_vL[std::min(I + 1, N - 1)] - gs_Ref.m_vL[I]);
			T += gs_P.m_HNow / Vn - (gs_Ref.KAtL(L + gs_P.m_HNow) - K);
		}
		else
		{
			float Vt = std::max(gs_Ref.m_vL[std::min(I + 1, N - 1)] - gs_Ref.m_vL[I], 3.0f); // reference px per tick here
			T += gs_P.m_HNow * (1.0f / Vn - 1.0f / Vt);
		}
		float Ge = gs_P.m_GhostE;
		if(gs_P.m_GhostSink > 0 && !gs_Ref.m_vSinkIdx.empty())
		{
			// energy is only worth something until the next sink (U-turn) takes it
			int J = gs_Ref.EnergyEnd(I);
			float Dist = gs_Ref.m_vL[J] - gs_Ref.m_vL[I];
			Ge *= std::clamp(Dist / gs_P.m_GhostSink, 0.0f, 1.0f);
		}
		float Eref = 0;
		if(gs_P.m_ERel > 0)
			Eref = gs_P.m_ERel * (gs_Ref.m_vET[I] + Frac * (gs_Ref.m_vET[std::min(I + 1, N - 1)] - gs_Ref.m_vET[I]));
		T -= Ge * (Ee - Eref); // optional energy credit (ticks per unit; erel: relative to the reference's energy here)
		return (float)(G.m_Tick - G.m_StartTick) + T + LatPen(G, I, Frac);
	}
	// energy part up to the next sink (or the evaluation end), Teero's own time after it
	int J = gs_Ref.EnergyEnd(I);
	float T;
	if(J <= I)
		T = gs_Ref.m_vK[gs_Ref.m_EndIdx] - (gs_Ref.m_vK[I] + Frac * (gs_Ref.m_vK[std::min(I + 1, (int)gs_Ref.m_vK.size() - 1)] - gs_Ref.m_vK[I]));
	else
		T = gs_Ref.TRem(I, Frac, Ee) - gs_Ref.TAt(J, Ee) + (gs_Ref.m_vK[gs_Ref.m_EndIdx] - gs_Ref.m_vK[J]);
	if(gs_P.m_HNow > 0 && J > I)
	{
		vec2 V = G.Vel();
		float Sp = length(V);
		vec2 Tg = gs_Ref.Tangent(I);
		float Along = Sp > 1e-3f ? dot(V, Tg) / Sp : 0.0f;
		float Vn = AlongSpeed(V, Tg, 5.0f);
		float Ve = gs_Ref.SpeedAt(I, Ee);
		float Rest = gs_Ref.m_vL[J] - (gs_Ref.m_vL[I] + Frac * (gs_Ref.m_vL[std::min(I + 1, (int)gs_Ref.m_vL.size() - 1)] - gs_Ref.m_vL[I]));
		float H = std::min(gs_P.m_HNow, std::max(Rest, 0.0f));
		T += H * (1.0f / Vn - 1.0f / Ve);
	}
	return (float)(G.m_Tick - G.m_StartTick) + T + LatPen(G, I, Frac);
}

template<class TG>
static void UpdateTrack(TG &G)
{
	if(!G.m_Started)
		return;
	float Frac;
	G.m_RefIdx = gs_Ref.Track(G.m_RefIdx, G.Pos(), G.Vel(), Frac);
}

static int64_t CellKey(const CGameT &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	int64_t k = (int64_t)std::floor(P.x / gs_P.m_CPos) + 64;
	k = k * 1024 + ((int64_t)std::floor(P.y / gs_P.m_CPos) + 64);
	k = k * 128 + ((int64_t)std::floor(V.x / gs_P.m_CVel) + 64);
	k = k * 128 + ((int64_t)std::floor(V.y / gs_P.m_CVel) + 64);
	int hs = G.HookState();
	int h = hs == 0 || hs == -1 ? 0 : (hs == 5 ? 2 : 1);
	k = k * 3 + h;
	k = k * 3 + JumpsLeft(G);
	k = k * 2 + (G.m_LastJump ? 1 : 0);
	if(G.HasGrenade())
		k = k * 8 + std::min(G.ReloadTimer(), 28) / 4 + (G.NumProjectiles() > 0 ? 64 : 0);
	if(gs_P.m_Prefire && G.NumProjectiles() > 0)
	{
		// pre-fired shots with different targets are different states
		vec2 E;
		int Te;
		if(G.NextExplosion(E, Te))
		{
			k = k * 1031 + ((int64_t)std::floor(E.x / 24) & 1023);
			k = k * 1031 + ((int64_t)std::floor(E.y / 24) & 1023);
			k = k * 67 + ((Te - G.m_Tick) & 63);
		}
	}
	if(h)
	{
		vec2 A = G.HookPos();
		k = k * 64 + ((int64_t)std::floor(A.x / 64) & 63);
		k = k * 64 + ((int64_t)std::floor(A.y / 64) & 63);
	}
	return k;
}

static void HookTargets(const CGameT &G, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.Pos();
	const float Range = gs_P.m_HookRange + 4.0f * length(G.Vel());
	if(NoSolidNear(P, Range + 2.0f))
		return;
	std::vector<int> vSeen;
	for(int a = 0; a < gs_P.m_Angles; a++)
	{
		float Ang = 2 * pi * a / gs_P.m_Angles;
		vec2 D(std::cos(Ang), std::sin(Ang));
		int Hit = -1;
		for(float r = 42.0f; r < Range; r += 4.0f)
		{
			vec2 Q = P + D * r;
			int tx = (int)std::floor(Q.x / 32), ty = (int)std::floor(Q.y / 32);
			int T = M.Tile(tx, ty);
			if(T == TILE_SOLID)
			{
				Hit = ty * M.m_W + tx;
				break;
			}
			if(T == TILE_NOHOOK)
				break;
		}
		if(Hit < 0 || (gs_P.m_HookDedup && std::find(vSeen.begin(), vSeen.end(), Hit) != vSeen.end()))
			continue;
		vSeen.push_back(Hit);
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

// shot directions whose ray meets a solid tile within range (explosions far away don't matter)
static void FireTargets(const CGameT &G, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.Pos();
	for(int a = 0; a < gs_P.m_FireAngles; a++)
	{
		float Ang = 2 * pi * a / gs_P.m_FireAngles;
		vec2 D(std::cos(Ang), std::sin(Ang));
		bool Hit = false;
		for(float r = 8.0f; r < gs_P.m_FireRange; r += 6.0f)
		{
			vec2 Q = P + D * r;
			if(M.Tile((int)std::floor(Q.x / 32), (int)std::floor(Q.y / 32)) == TILE_SOLID)
			{
				Hit = true;
				break;
			}
		}
		if(!Hit)
			continue;
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

// edge-refined shot aims: scan coarse angles, bisect where the explosion point or time jumps (side faces and
// corners of blocks), keep aims whose explosion comes 4..padrange ticks later near where the tee is heading
static void PadAims(const CGameT &G, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	static thread_local CGameT s_T;
	const int N = gs_P.m_PadAims;
	struct SShot
	{
		bool m_Ok;
		vec2 m_E;
		int m_T;
	};
	auto Shoot = [&](float Ang) {
		SShot R{false, vec2(0, 0), 0};
		s_T.CopyFrom(G);
		STasInput In;
		In.m_Dir = 0;
		In.m_Hook = G.m_LastHook;
		In.m_Jump = G.m_LastJump;
		In.m_Fire = 1;
		In.m_Weapon = 3;
		In.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
		In.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
		if(!In.m_TX && !In.m_TY)
			In.m_TY = -1;
		s_T.Step(In);
		R.m_Ok = s_T.NextExplosion(R.m_E, R.m_T);
		R.m_T -= G.m_Tick;
		return R;
	};
	const vec2 P = G.Pos(), V = G.Vel();
	auto Useful = [&](const SShot &S) {
		if(!S.m_Ok || S.m_T < 4 || S.m_T > gs_P.m_PadRange)
			return false;
		vec2 Pred = P + V * (S.m_T * 0.85f);
		if(distance(Pred, S.m_E) < 140.0f)
			return true;
		if(gs_P.m_PadRef > 0 && G.m_Started)
		{
			// near the reference line where it will be about S.m_T ticks from here (turns: the straight-line guess fails)
			const int N = gs_Ref.m_vK.size();
			const float K0 = gs_Ref.m_vK[std::clamp(G.m_RefIdx, 0, N - 1)];
			for(int i = std::max(G.m_RefIdx, 0); i < N && gs_Ref.m_vK[i] <= K0 + 1.4f * S.m_T + 3; i++)
				if(gs_Ref.m_vK[i] >= K0 + 0.6f * S.m_T && distance(gs_Ref.m_vP[i], S.m_E) < gs_P.m_PadRef)
					return true;
		}
		return false;
	};
	auto Emit = [&](float Ang) {
		int16_t TX = (int16_t)std::lround(std::cos(Ang) * 1000), TY = (int16_t)std::lround(std::sin(Ang) * 1000);
		if(!TX && !TY)
			TY = -1;
		for(auto &q : vOut)
			if(q.first == TX && q.second == TY)
				return;
		vOut.push_back({TX, TY});
	};
	std::vector<SShot> vS(N);
	for(int a = 0; a < N; a++)
		vS[a] = Shoot(2 * pi * a / N);
	for(int a = 0; a < N; a++)
	{
		const SShot &A = vS[a], &B = vS[(a + 1) % N];
		bool Jump = A.m_Ok != B.m_Ok || distance(A.m_E, B.m_E) > 40.0f || std::abs(A.m_T - B.m_T) > 2;
		if(!Jump)
		{
			if(Useful(A))
				Emit(2 * pi * a / N);
			continue;
		}
		if(!Useful(A) && !Useful(B))
			continue;
		float Lo = 2 * pi * a / N, Hi = 2 * pi * (a + 1) / N;
		SShot SL = A;
		for(int k = 0; k < 8; k++)
		{
			float Mid = 0.5f * (Lo + Hi);
			SShot M = Shoot(Mid);
			if(M.m_Ok == SL.m_Ok && distance(M.m_E, SL.m_E) <= 40.0f && std::abs(M.m_T - SL.m_T) <= 2)
			{
				Lo = Mid;
				SL = M;
			}
			else
				Hi = Mid;
		}
		SShot SH = Shoot(Hi);
		if(Useful(SL))
			Emit(Lo);
		if(Useful(SH))
			Emit(Hi);
	}
}

struct SImitTick
{
	int m_Dir = 0, m_Jump = 0, m_Known = 0;
	int16_t m_TX = 1000, m_TY = 0;
};
static std::vector<SImitTick> gs_vImit; // index = race tick + 100
static void LoadImit(const char *pPath)
{
	FILE *f = std::fopen(pPath, "r");
	if(!f)
	{
		std::printf("cannot read %s\n", pPath);
		std::exit(1);
	}
	char aLine[512];
	std::fgets(aLine, sizeof(aLine), f); // header: frame,video_s,s_since_start,A_left,D_right,dir,jump_arrow,aim_angle_deg,aim_tx,aim_ty,...
	gs_vImit.assign(1400, SImitTick());
	std::vector<float> vBest(1400, 1e9f);
	while(std::fgets(aLine, sizeof(aLine), f))
	{
		int Frame, A, D, Dir, J, TX, TY;
		float Vs, S, Ang;
		if(std::sscanf(aLine, "%d,%f,%f,%d,%d,%d,%d,%f,%d,%d", &Frame, &Vs, &S, &A, &D, &Dir, &J, &Ang, &TX, &TY) != 10)
			continue;
		float Rt = S * 50.0f + gs_P.m_ImitOff;
		int R = (int)std::lround(Rt);
		for(int r = R - 1; r <= R + 1; r++)
		{
			int i = r + 100;
			if(i < 0 || i >= (int)gs_vImit.size())
				continue;
			float d = std::fabs(Rt - r);
			if(d < vBest[i])
			{
				vBest[i] = d;
				gs_vImit[i].m_Dir = Dir;
				gs_vImit[i].m_Jump = J;
				gs_vImit[i].m_TX = (int16_t)TX;
				gs_vImit[i].m_TY = (int16_t)TY;
				gs_vImit[i].m_Known = 1;
			}
		}
	}
	std::fclose(f);
}
static const SImitTick *ImitAt(int Rt)
{
	int i = Rt + 100;
	if(i < 0 || i >= (int)gs_vImit.size() || !gs_vImit[i].m_Known)
		return nullptr;
	return &gs_vImit[i];
}

// velocity right after this tick's gravity / jump / direction input (before the hook)
static vec2 PreHookVel(const CGameT &G, int Dir, int Jump)
{
	vec2 V = G.Vel();
	V.y += 0.5f;
	if(Jump)
	{
		if(G.Grounded())
			V.y = -13.2f;
		else if(!(G.Jumped() & 2))
			V.y = -12.0f;
	}
	const bool Gr = G.Grounded();
	const float Acc = Gr ? 2.0f : 1.5f, Max = Gr ? 10.0f : 5.0f;
	if(Dir == 0)
		V.x *= Gr ? 0.5f : 0.95f;
	else if(Dir > 0 && V.x <= Max)
		V.x = std::min(V.x + Acc, Max);
	else if(Dir < 0 && V.x >= -Max)
		V.x = std::max(V.x - Acc, -Max);
	return V;
}

static vec2 HookPull(vec2 To, int Dir)
{
	vec2 H = normalize(To) * 3.0f;
	if(H.y > 0)
		H.y *= 0.3f;
	H.x *= ((H.x < 0 && Dir < 0) || (H.x > 0 && Dir > 0)) ? 0.95f : 0.75f;
	return H;
}

// Rotation pulse: a hook fired at a solid tile 47..122 px away grabs and pulls in the same tick along the aim. Above
// 15 px/t the pull only applies if |v| does not grow; the best aim sits on that boundary and turns v the most towards
// the reference tangent. Up to 3 aims: the boundary one, 1 and 3 degrees inside it.
static void RotHookAims(const CGameT &G, int Dir, int Jump, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	const vec2 P = G.Pos();
	if(NoSolidNear(P, 124.0f)) // nothing within the 122 px reach
		return;
	const vec2 V = PreHookVel(G, Dir, Jump);
	const vec2 Tg = gs_Ref.Tangent(G.m_RefIdx);
	const float L0 = length(V);
	static const std::vector<vec2> s_vDir = [] {
		std::vector<vec2> v;
		for(int i = -720; i < 720; i++)
		{
			float Ang = i * pi / 720.0f;
			v.emplace_back(std::cos(Ang), std::sin(Ang));
		}
		return v;
	}();
	auto DirOf = [&](int i) {
		if(i >= -720 && i < 720)
			return s_vDir[i + 720];
		float Ang = i * pi / 720.0f;
		return vec2(std::cos(Ang), std::sin(Ang));
	};
	auto Reach = [&](int i) {
		vec2 D = DirOf(i);
		for(float r = 42.0f; r <= 122.0f; r += 1.0f)
		{
			vec2 Q = P + D * r;
			int T = M.Tile((int)std::floor(Q.x / 32), (int)std::floor(Q.y / 32));
			if(T == TILE_SOLID)
				return r > 47.0f;
			if(T == TILE_NOHOOK)
				return false;
		}
		return false;
	};
	auto NewV = [&](int i) { return V + HookPull(DirOf(i), Dir); };
	auto Ok = [&](int i) { float Ln = length(NewV(i)); return Ln < 15.0f - 0.01f || Ln < L0 - 0.004f; };
	float Best = dot(V, Tg) + 0.02f;
	int BestI = -100000;
	for(int i = -720; i < 720; i++)
	{
		if(!Ok(i))
			continue;
		float A = dot(NewV(i), Tg);
		if(A <= Best || !Reach(i))
			continue;
		Best = A;
		BestI = i;
	}
	if(BestI == -100000)
		return;
	for(int k = 0; k < 3; k++)
	{
		int i = BestI;
		if(k)
		{
			int Off = k == 1 ? 4 : 12;
			int i1 = BestI + Off, i2 = BestI - Off;
			bool Ok1 = Ok(i1) && Reach(i1), Ok2 = Ok(i2) && Reach(i2);
			if(Ok1 && (!Ok2 || dot(NewV(i1), Tg) > dot(NewV(i2), Tg)))
				i = i1;
			else if(Ok2)
				i = i2;
			else
				continue;
		}
		float Ang = i * pi / 720.0f;
		int16_t TX = (int16_t)std::lround(std::cos(Ang) * 1000), TY = (int16_t)std::lround(std::sin(Ang) * 1000);
		if(TX || TY)
			vOut.push_back({TX, TY});
	}
}

// A hook in flight or held whose pull has not applied yet: predict (ballistic tee, straight hook) the first tick its
// pull would apply; returns false if none within m_HookLA ticks, else the predicted position and velocities.
static bool HookLookahead(const CGameT &G, int Dir, vec2 &PosOut, vec2 &VFree, vec2 &VPull)
{
	const CCharacterCore &C = CoreOf(G);
	const int HS = C.m_HookState;
	if(HS != HOOK_FLYING && HS != HOOK_GRABBED)
		return false;
	const SMapInfo &M = CTasGame::Map();
	vec2 P = C.m_Pos, V = C.m_Vel, A = C.m_HookPos, D = C.m_HookDir;
	bool Grabbed = HS == HOOK_GRABBED;
	for(int n = 0; n < gs_P.m_HookLA; n++)
	{
		V.y += 0.5f;
		if(!Grabbed)
		{
			vec2 NA = A + D * 80.0f;
			if(distance(P, NA) > 380.0f)
				return false;
			bool Hit = false;
			for(float r = 0; r <= 80.0f; r += 2.0f)
			{
				vec2 Q = A + D * r;
				int T = M.Tile((int)std::floor(Q.x / 32), (int)std::floor(Q.y / 32));
				if(T == TILE_NOHOOK)
					return false;
				if(T == TILE_SOLID)
				{
					A = Q;
					Hit = true;
					break;
				}
			}
			if(!Hit)
				A = NA;
			Grabbed = Hit;
		}
		if(Grabbed && distance(A, P) > 46.0f)
		{
			vec2 NV = V + HookPull(A - P, Dir);
			float Ln = length(NV);
			if(Ln < 15.0f || Ln < length(V))
			{
				PosOut = P;
				VFree = V;
				VPull = NV;
				return true;
			}
		}
		float L = length(V);
		float Rm = L > 11 ? gs_Ref.Disp(L) / L : 1.0f;
		P.x += V.x * Rm;
		P.y += V.y;
	}
	return false;
}

// next explosion of a projectile in flight, cached in the state (projectiles ignore the tee in solo)
static bool PendExpl(CGameT &G, vec2 &E, int &T)
{
	if(G.NumProjectiles() <= 0)
		return false;
	if(G.m_PendFire != G.m_Fire || G.m_PendT <= G.m_Tick)
	{
		G.m_PendFire = G.m_Fire;
		if(!G.NextExplosion(G.m_PendE, G.m_PendT))
		{
			G.m_PendT = -1;
			return false;
		}
	}
	if(G.m_PendT <= G.m_Tick)
		return false;
	E = G.m_PendE;
	T = G.m_PendT;
	return true;
}

// shot plan (plan=FILE): lines "t0 t1 ex ey [r]" = fire on race ticks t0..t1 so that the grenade explodes within r px
// (default 40) of (ex,ey) (aims found by exact simulation: coarse scan + golden-section refinement; pre-fires included),
// "t0 t1 free" = ordinary point-blank shots allowed on t0..t1. With a plan, no other shot is generated (any direction
// input). planforce=1 drops states that did not fire in a target entry's window (reload still 0 at t1+1).
struct SPlanEntry
{
	int m_T0, m_T1;
	vec2 m_E;
	float m_R;
	bool m_Free;
	int m_Te = -1, m_Tol = 2; // optional explosion race tick (and tolerance)
};
static std::vector<SPlanEntry> gs_vPlanE;
static void LoadPlan(const char *pPath)
{
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
		SPlanEntry P{0, 0, vec2(0, 0), 40.0f, false, -1, 2};
		char aW[64] = "";
		if(std::sscanf(aLine, "%d %d %63s", &P.m_T0, &P.m_T1, aW) < 3)
			continue;
		if(std::string(aW) == "free")
			P.m_Free = true;
		else if(std::sscanf(aLine, "%d %d %f %f %f %d %d", &P.m_T0, &P.m_T1, &P.m_E.x, &P.m_E.y, &P.m_R, &P.m_Te, &P.m_Tol) < 4)
			continue;
		gs_vPlanE.push_back(P);
	}
	std::fclose(f);
}
static void PlanAims(const CGameT &G, const SPlanEntry &P, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	static thread_local CGameT s_T;
	auto Dist = [&](float Ang) {
		s_T.CopyFrom(G);
		STasInput In;
		In.m_Dir = 0;
		In.m_Hook = G.m_LastHook;
		In.m_Jump = 0;
		In.m_Fire = 1;
		In.m_Weapon = 3;
		In.m_TX = (int16_t)std::lround(std::cos(Ang) * 1000);
		In.m_TY = (int16_t)std::lround(std::sin(Ang) * 1000);
		if(!In.m_TX && !In.m_TY)
			In.m_TY = -1;
		s_T.Step(In);
		vec2 E;
		int T;
		if(!s_T.NextExplosion(E, T))
			return 1e9f;
		float D = distance(E, P.m_E);
		if(P.m_Te >= 0)
		{
			// planref: m_Te is the flight time (explosion tick - fire tick); else the explosion race tick
			const int Want = gs_P.m_PlanRef ? G.m_Tick + 1 + P.m_Te : G.m_StartTick + P.m_Te;
			D += 8.0f * std::max(0, std::abs(T - Want) - P.m_Tol);
		}
		return D;
	};
	const int N = 72;
	float aD[N];
	for(int a = 0; a < N; a++)
		aD[a] = Dist(2 * pi * a / N);
	for(int a = 0; a < N; a++)
	{
		const float d = aD[a], dl = aD[(a + N - 1) % N], dr = aD[(a + 1) % N];
		if(d > 300.0f || d > dl || d > dr)
			continue;
		// golden-section search on [a-1, a+1]
		float Lo = 2 * pi * (a - 1) / N, Hi = 2 * pi * (a + 1) / N;
		const float g = 0.618034f;
		float x1 = Hi - g * (Hi - Lo), x2 = Lo + g * (Hi - Lo), f1 = Dist(x1), f2 = Dist(x2);
		for(int k = 0; k < 22; k++)
		{
			if(f1 < f2)
			{
				Hi = x2;
				x2 = x1;
				f2 = f1;
				x1 = Hi - g * (Hi - Lo);
				f1 = Dist(x1);
			}
			else
			{
				Lo = x1;
				x1 = x2;
				f1 = f2;
				x2 = Lo + g * (Hi - Lo);
				f2 = Dist(x2);
			}
		}
		const float Best = f1 < f2 ? x1 : x2;
		if(std::min(f1, f2) > P.m_R)
			continue;
		int16_t TX = (int16_t)std::lround(std::cos(Best) * 1000), TY = (int16_t)std::lround(std::sin(Best) * 1000);
		if(!TX && !TY)
			TY = -1;
		std::pair<int16_t, int16_t> Q{TX, TY};
		if(std::find(vOut.begin(), vOut.end(), Q) == vOut.end())
			vOut.push_back(Q);
	}
}

static void GenActions(const CGameT &G, const STasInput &Prev, std::vector<STasInput> &vOut, bool Pad = false)
{
	vOut.clear();
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vHooks, s_vFire;
	const bool Hooking = G.m_LastHook;
	if(!Hooking)
		HookTargets(G, s_vHooks);
	const bool CanJump = !G.m_LastJump && (G.Grounded() || !(G.Jumped() & 2));
	const int Weapon = G.HasGrenade() ? 3 : -1;
	const bool CanFire = gs_P.m_Fire && G.HasGrenade() && G.ActiveWeapon() == 3 && G.ReloadTimer() == 0 && !Prev.m_Fire;
	if(!gs_vPlanE.empty())
	{
		s_vFire.clear();
		if(CanFire)
		{
			// planref: windows are Teero ticks on the reference line (where he fired), else our race ticks
			const int Rt = gs_P.m_PlanRef ? (int)std::lround(gs_Ref.m_vK[std::clamp(G.m_RefIdx, 0, (int)gs_Ref.m_vK.size() - 1)]) : G.m_Tick + 1 - G.m_StartTick;
			bool Free = false;
			static thread_local std::vector<std::pair<int16_t, int16_t>> s_vPA;
			for(const auto &P : gs_vPlanE)
				if(Rt >= P.m_T0 && Rt <= P.m_T1)
				{
					if(P.m_Free)
						Free = true;
					else if(Pad)
					{
						s_vPA.clear();
						PlanAims(G, P, s_vPA);
						for(auto &q : s_vPA)
							if(std::find(s_vFire.begin(), s_vFire.end(), q) == s_vFire.end())
								s_vFire.push_back(q);
					}
				}
			if(!Free && gs_P.m_PlanFree)
			{
				// hybrid: ordinary shots anywhere except in the planlock ticks before a planned target shot
				bool Active = false, Locked = false;
				for(const auto &P : gs_vPlanE)
				{
					if(Rt >= P.m_T0 && Rt <= P.m_T1)
						Active = true;
					if(!P.m_Free && Rt < P.m_T0 && Rt >= P.m_T0 - gs_P.m_PlanLock)
						Locked = true;
				}
				Free = !Active && !Locked;
			}
			if(Free)
			{
				s_vPA.clear();
				FireTargets(G, s_vPA);
				for(auto &q : s_vPA)
					if(std::find(s_vFire.begin(), s_vFire.end(), q) == s_vFire.end())
						s_vFire.push_back(q);
			}
		}
	}
	else if(CanFire)
	{
		FireTargets(G, s_vFire);
		if(Pad && gs_P.m_PadAims > 0)
		{
			static thread_local std::vector<std::pair<int16_t, int16_t>> s_vPad;
			PadAims(G, s_vPad);
			if(getenv("SEG_PADDBG") && !s_vPad.empty())
				std::printf("pad: tick %d pos %.0f %.0f %zu aims\n", G.m_Tick, G.Pos().x, G.Pos().y, s_vPad.size());
			for(auto &q : s_vPad)
				if(std::find(s_vFire.begin(), s_vFire.end(), q) == s_vFire.end())
					s_vFire.push_back(q);
		}
	}
	// imitation: Teero's dirs / jumps / aims around this race tick
	bool Imit = !gs_vImit.empty() && G.m_Started;
	bool ImitDir[3] = {false, false, false};
	bool ImitJump = false;
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vImitAims;
	if(Imit)
	{
		const int Rt = G.m_Tick - G.m_StartTick + 1;
		s_vImitAims.clear();
		int Known = 0;
		for(int d = -gs_P.m_ImitW; d <= gs_P.m_ImitW; d++)
		{
			const SImitTick *p = ImitAt(Rt + d);
			if(!p)
				continue;
			Known++;
			ImitDir[p->m_Dir + 1] = true;
			if(p->m_Jump)
				ImitJump = true;
			std::pair<int16_t, int16_t> A{p->m_TX, p->m_TY};
			if(std::find(s_vImitAims.begin(), s_vImitAims.end(), A) == s_vImitAims.end())
				s_vImitAims.push_back(A);
		}
		if(!Known)
			Imit = false;
		else if(!gs_P.m_ImitHooks)
			s_vHooks.clear();
		if(Imit && !Hooking)
			for(auto &A : s_vImitAims)
				if(std::find(s_vHooks.begin(), s_vHooks.end(), A) == s_vHooks.end())
					s_vHooks.push_back(A);
	}
	int DirLo = -1, DirHi = 1;
	if(gs_P.m_DirTan)
	{
		vec2 Tg = gs_Ref.Tangent(G.m_RefIdx);
		if(Tg.x > 0.3f)
			DirLo = DirHi = 1;
		else if(Tg.x < -0.3f)
			DirLo = DirHi = -1;
	}
	for(int Dir = DirLo; Dir <= DirHi; Dir++)
		for(int Jump = 0; Jump <= (CanJump ? 1 : 0); Jump++)
		{
			if(Imit && gs_P.m_ImitDir && !ImitDir[Dir + 1])
				continue;
			if(Imit && Jump && !ImitJump)
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
			{
				for(auto [TX, TY] : s_vHooks)
				{
					STasInput H = In;
					H.m_Hook = 1;
					H.m_TX = TX;
					H.m_TY = TY;
					vOut.push_back(H);
				}
				if(gs_P.m_RotHook)
				{
					static thread_local std::vector<std::pair<int16_t, int16_t>> s_vRot;
					RotHookAims(G, Dir, Jump, s_vRot);
					for(auto [TX, TY] : s_vRot)
					{
						STasInput H = In;
						H.m_Hook = 1;
						H.m_TX = TX;
						H.m_TY = TY;
						vOut.push_back(H);
					}
				}
			}
			// shots keep the hook as it is (the hook's direction is fixed at launch)
			int FireDir = 1;
			{
				vec2 Tg = gs_Ref.Tangent(G.m_RefIdx);
				FireDir = Tg.x >= 0 ? 1 : -1;
			}
			if(CanFire && Jump == 0 && (gs_P.m_FireAllDirs || !gs_vPlanE.empty() || Dir == FireDir))
				for(auto [TX, TY] : s_vFire)
				{
					if(gs_P.m_FireAlign > -1.5f)
					{
						// the kick points opposite to the aim: keep only shots that push along the line
						vec2 Tg = gs_Ref.Tangent(G.m_RefIdx);
						vec2 A(TX, TY);
						if(dot(-normalize(A), Tg) < gs_P.m_FireAlign)
							continue;
					}
					STasInput F = In;
					F.m_Fire = 1;
					F.m_TX = TX;
					F.m_TY = TY;
					vOut.push_back(F);
				}
		}
}

static bool Survives(const CGameT &G, int N)
{
	if(G.Frozen() || G.EnteredFreeze())
		return false;
	vec2 aPos[128];
	N = std::min(N, 128);
	for(int d = 1; d >= -1; d--)
	{
		if(G.Rollout(N, d, true, aPos) == N)
			return true;
		if(G.m_LastHook && G.Rollout(N, d, false, aPos) == N)
			return true;
	}
	static thread_local std::vector<std::pair<int16_t, int16_t>> s_vHooks;
	HookTargets(G, s_vHooks);
	for(auto [TX, TY] : s_vHooks)
		for(int d = 1; d >= -1; d--)
			if(G.RolloutHook(N, d, TX, TY, aPos) == N)
				return true;
	return false;
}

static int64_t QuotaKey(const CGameT &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	int64_t k = ((int64_t)std::floor(P.x / gs_P.m_QCell) + 1000) * 100000 + ((int64_t)std::floor(P.y / gs_P.m_QCell) + 1000);
	k = (k * 1000 + ((int64_t)std::floor(V.x / gs_P.m_QVel) + 500)) * 1000 + ((int64_t)std::floor(V.y / gs_P.m_QVel) + 500);
	return k * 8 + (G.ReloadTimer() == 0 ? 1 : 0) + (G.NumProjectiles() > 0 ? 2 : 0) + ((G.Jumped() & 2) ? 4 : 0);
}

struct SRetro
{
	int m_Step = -1; // retro shot: vHist step of the input that fires it (-1: none)
	int16_t m_TX = 0, m_TY = 0;
	vec2 m_P0 = vec2(0, 0), m_Dir = vec2(0, 0);
	int m_StartTick = 0, m_Reload = 0, m_Life = 0;
};
struct SPatch
{
	int m_Step = -1;
	int16_t m_TX = 0, m_TY = 0;
};
struct SAux
{
	vec2 m_Pos; // CCharacter::m_Pos after the step (a shot in the next tick starts here)
	uint8_t m_Flags; // 1: holds a loaded grenade, 2: hook idle (a hook press next tick would aim)
};

struct SCand
{
	float m_S; // estimated total (lower better)
	int m_Parent;
	STasInput m_In;
	int64_t m_Key;
	uint64_t m_Hash;
	float m_Ee;
	int m_Rt;
	int m_Ref;
	int64_t m_QKey = 0; // coarse position/velocity cell for the diversity quota
	bool m_Inc = false; // the incumbent's own continuation
	bool m_Loaded = false; // has the grenade with the reload at 0 (loadres)
	SRetro m_R; // retro shot applied to the parent first
};

#ifdef SEG_FAST
// retro shot: the grenade the ancestor fired is put in flight (newest first), with the reload it would have now
static void ApplyRetro(CGameT &G, const SRetro &R)
{
	// entity list order: newest first
	int At = 0;
	while(At < G.m_NumProj && G.m_aProj[At].m_StartTick > R.m_StartTick)
		At++;
	for(int k = G.m_NumProj; k > At; k--)
		G.m_aProj[k] = G.m_aProj[k - 1];
	G.m_NumProj++;
	SFastProj &P = G.m_aProj[At];
	P.m_Pos = R.m_P0;
	P.m_Dir = R.m_Dir;
	P.m_StartTick = R.m_StartTick;
	P.m_LifeSpan = R.m_Life;
	G.m_ReloadTimer = R.m_Reload;
	// the patched input pressed and released fire once more
	G.m_Fire += 2;
	G.m_Input.m_Fire += 2;
	G.m_LatestInput.m_Fire += 2;
	G.m_LatestPrevInput.m_Fire += 2;
	G.m_Core.m_Input.m_Fire += 2;
	G.m_RetroMin = G.m_Tick + 1;
}

// retro shots for state G (beam node idxG of vHist step sG): walk back while the state held a loaded grenade; from
// each ancestor A (fire in the next input N: no hook start there) solve the aim whose grenade reaches a solid point
// next to G in the tick after G, check its whole flight exactly (no earlier collision), keep the best K kicks
static void RetroFind(const CGameT &G, int sG, int idxG, const std::vector<std::vector<std::pair<int, STasInput>>> &vHist,
	const std::vector<std::vector<SAux>> &vAux, std::vector<SRetro> &vOut)
{
	vOut.clear();
	if(!(G.HasGrenade() && G.m_Core.m_ActiveWeapon == WEAPON_GRENADE && G.NumProjectiles() < CFastG::MAX_PROJ))
		return;
	const SMapInfo &M = CTasGame::Map();
	const vec2 Q = G.m_Pos, V = G.Vel();
	const int t = G.m_Tick;
	// solid points around the tee
	vec2 aE[24];
	int NE = 0;
	for(int a = 0; a < 24; a++)
	{
		const float Ang = 2 * pi * a / 24;
		const vec2 D(std::cos(Ang), std::sin(Ang));
		for(float r = 6.0f; r < gs_P.m_RetroRad; r += 3.0f)
		{
			vec2 P = Q + D * r;
			if(M.Tile((int)std::floor(P.x / 32), (int)std::floor(P.y / 32)) == TILE_SOLID)
			{
				aE[NE++] = P;
				break;
			}
		}
	}
	if(!NE)
		return;
	const float Curv = G.m_Core.m_Tuning.m_GrenadeCurvature, Speed = G.m_Core.m_Tuning.m_GrenadeSpeed;
	const float Vg = Speed / SERVER_TICK_SPEED, Cg = Curv / 10000.0f * Vg * Vg, R0 = CCharacterCore::PhysicalSize() * 0.75f;
	const float Strength = G.m_Core.m_Tuning.m_ExplosionStrength;
	vec2 Tg = gs_Ref.Tangent(std::clamp(G.m_RefIdx, 0, (int)gs_Ref.m_vP.size() - 1));
	struct SC
	{
		SRetro R;
		float V;
		vec2 C;
	};
	static thread_local std::vector<SC> s_vC;
	s_vC.clear();
	int sN = sG, iN = idxG;
	// G may have fired one shot itself recently (reload running): then the retro shot must come from before that
	// shot's tick Ts, 25+ ticks earlier (its reload over in time); G keeps its own reload
	int Ts = -1;
	if(!(vAux[sG][idxG].m_Flags & 1))
	{
		if(gs_P.m_RetroAfter <= 0)
			return;
		int d = 0;
		while(sN >= 1 && !(vAux[sN][iN].m_Flags & 1) && d <= gs_P.m_RetroMaxF)
		{
			iN = vHist[sN][iN].first;
			sN--;
			d++;
		}
		if(sN < 1 || !(vAux[sN][iN].m_Flags & 1) || d > gs_P.m_RetroMaxF)
			return;
		Ts = t - d + 1; // tick of the node created by the shot's step
	}
	for(int d = sG - sN; d <= gs_P.m_RetroMaxF && sN >= 1; d++)
	{
		const SAux &N = vAux[sN][iN];
		if(!(N.m_Flags & 1))
			break;
		const int iA = vHist[sN][iN].first;
		const SAux &A = vAux[sN - 1][iA];
		const STasInput &InN = vHist[sN][iN].second;
		const int TickA = t - d - 1;
		if(TickA < G.m_RetroMin || !(A.m_Flags & 1))
			break;
		if(d >= 1 && d >= gs_P.m_RetroMinF && !InN.m_Fire && !((A.m_Flags & 2) && InN.m_Hook) && (Ts < 0 || t - d + 25 <= Ts))
		{
			const int TauT = t + 1 - TickA;
			for(int e = 0; e < NE; e++)
			{
				const vec2 W = aE[e] - A.m_Pos;
				auto F = [&](float Tau) { return length(vec2(W.x, W.y - Cg * Tau * Tau)) - R0 - Vg * Tau; };
				float Lo = TauT - 1.0f, Hi = (float)TauT;
				if(!(F(Lo) > 0 && F(Hi) <= 0))
					continue;
				for(int it = 0; it < 24; it++)
				{
					float Mid = 0.5f * (Lo + Hi);
					if(F(Mid) > 0)
						Lo = Mid;
					else
						Hi = Mid;
				}
				vec2 Dn = normalize(vec2(W.x, W.y - Cg * Hi * Hi));
				int16_t TX = (int16_t)std::lround(Dn.x * 1000), TY = (int16_t)std::lround(Dn.y * 1000);
				if(!TX && !TY)
					TY = -1;
				const vec2 Dir = normalize(vec2(TX, TY));
				const vec2 P0 = A.m_Pos + Dir * R0;
				// the exact flight (as CFastG::TickProjectiles): first collision in the tick after G
				bool Ok = false;
				vec2 Col;
				for(int Tau = 1; Tau <= TauT; Tau++)
				{
					vec2 Prev = CalcPos(P0, Dir, Curv, Speed, (Tau - 1) / (float)SERVER_TICK_SPEED);
					vec2 Cur = CalcPos(P0, Dir, Curv, Speed, Tau / (float)SERVER_TICK_SPEED);
					if(Cur.x < 0 || Cur.y < 0 || Cur.x >= M.m_W * 32 || Cur.y >= M.m_H * 32)
						break;
					vec2 NewPos;
					int Collide = CTasGame::Collision()->IntersectLine(Prev, Cur, &Col, &NewPos);
					if(Collide)
					{
						Ok = Tau == TauT;
						break;
					}
				}
				if(!Ok)
					continue;
				// the kick (as CFastG::Explode, at G's position)
				if(!(distance(Q, Col) < 135.0f + CCharacterCore::PhysicalSize()))
					continue;
				vec2 Diff = Q - Col;
				float l = length(Diff);
				vec2 Fd = l ? normalize(Diff) : vec2(0, 1);
				l = 1 - std::clamp((l - 48.0f) / (135.0f - 48.0f), 0.0f, 1.0f);
				float Dmg = Strength * l;
				if(!(int)Dmg)
					continue;
				vec2 Fk = Fd * Dmg * 2;
				float Val = dot(Fk, Tg) + (length(V + Fk) - length(V));
				bool Dup = false;
				for(auto &C : s_vC)
					if(distance(C.C, Col) < 6.0f)
					{
						Dup = true;
						if(Val > C.V)
							C = {SRetro{sN, TX, TY, P0, Dir, TickA, Ts < 0 ? std::max(0, 24 - d) : G.ReloadTimer(), 100 - (t - TickA)}, Val, Col};
						break;
					}
				if(!Dup)
					s_vC.push_back({SRetro{sN, TX, TY, P0, Dir, TickA, Ts < 0 ? std::max(0, 24 - d) : G.ReloadTimer(), 100 - (t - TickA)}, Val, Col});
			}
		}
		sN--;
		iN = iA;
	}
	std::sort(s_vC.begin(), s_vC.end(), [](const SC &a, const SC &b) { return a.V > b.V; });
	for(int k = 0; k < (int)s_vC.size() && k < gs_P.m_Retro; k++)
		vOut.push_back(s_vC[k].R);
}
#endif

struct SGate
{
	float m_V = 1e30f; // lower better
	int m_Step = -1, m_Parent = -1;
	STasInput m_In;
	int m_Rt = 0;
	float m_Ee = 0;
	vec2 m_Pos, m_Vel;
	SRetro m_R;
};

static float gs_aBox[6] = {0, 0, 0, 0, 1e9f, -1}; // gate=box:x0,x1,y0,y1[,vymax[,K]] (with the grenade)
static bool gs_Box = false;

static bool AtGate(const CGameT &G)
{
	if(gs_Box)
	{
		vec2 P = G.Pos(), V = G.Vel();
		return G.HasGrenade() && P.x >= gs_aBox[0] && P.x <= gs_aBox[1] && P.y >= gs_aBox[2] && P.y <= gs_aBox[3] && V.y <= gs_aBox[4];
	}
	if(gs_P.m_Gate == "finish")
		return G.m_FinishTick >= 0;
	if(gs_P.m_Gate == "grenade")
		return G.HasGrenade();
	return G.m_RefIdx >= gs_Ref.m_GateIdx;
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: seg <map> prefix=FILE ref=TRACK gate=K|grenade|finish key=value...\n");
		return 1;
	}
#ifdef SEG_FAST
	CFastG::Init();
#endif
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
		{
			std::printf("bad arg %s\n", argv[i]);
			return 1;
		}
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "beam") gs_P.m_Beam = std::stoi(V);
		else if(K == "angles") gs_P.m_Angles = std::stoi(V);
		else if(K == "maxticks") gs_P.m_MaxTicks = std::stoi(V);
		else if(K == "threads") gs_P.m_Threads = std::stoi(V);
		else if(K == "cpos") gs_P.m_CPos = std::stof(V);
		else if(K == "cvel") gs_P.m_CVel = std::stof(V);
		else if(K == "dom") gs_P.m_Dom = std::stoi(V);
		else if(K == "out") gs_P.m_Out = V;
		else if(K == "prefix") gs_P.m_Prefix = V;
		else if(K == "ref") gs_P.m_Ref = V;
		else if(K == "gate") gs_P.m_Gate = V;
		else if(K == "hnow") gs_P.m_HNow = std::stof(V);
		else if(K == "dirmode") gs_P.m_DirTan = V == "tan";
		else if(K == "pjc") gs_P.m_PJC = std::stof(V);
		else if(K == "pgc") gs_P.m_PGC = std::stof(V);
		else if(K == "gcred") gs_P.m_GCred = std::stof(V);
		else if(K == "fire") gs_P.m_Fire = std::stoi(V);
		else if(K == "fireangles") gs_P.m_FireAngles = std::stoi(V);
		else if(K == "firerange") gs_P.m_FireRange = std::stof(V);
		else if(K == "plan") gs_P.m_Plan = V;
		else if(K == "planrv") gs_P.m_PlanRv = std::stof(V);
		else if(K == "planacc") gs_P.m_PlanAcc = std::stof(V);
		else if(K == "planfree") gs_P.m_PlanFree = std::stoi(V);
		else if(K == "planlock") gs_P.m_PlanLock = std::stoi(V);
		else if(K == "jitter") gs_P.m_Jitter = std::stof(V);
		else if(K == "seed") gs_P.m_Seed = std::stoi(V);
		else if(K == "crashw") gs_P.m_CrashW = std::stof(V);
		else if(K == "crashn") gs_P.m_CrashN = std::stoi(V);
		else if(K == "planref") gs_P.m_PlanRef = std::stoi(V);
		else if(K == "planforce") gs_P.m_PlanForce = std::stoi(V);
		else if(K == "firealign") gs_P.m_FireAlign = std::stof(V);
		else if(K == "firealldirs") gs_P.m_FireAllDirs = std::stoi(V);
		else if(K == "pendlook") gs_P.m_PendLook = std::stoi(V);
		else if(K == "survevery") gs_P.m_SurvEvery = std::stoi(V);
		else if(K == "padaims") gs_P.m_PadAims = std::stoi(V);
		else if(K == "padtop") gs_P.m_PadTop = std::stoi(V);
		else if(K == "padrange") gs_P.m_PadRange = std::stoi(V);
		else if(K == "padref") gs_P.m_PadRef = std::stof(V);
		else if(K == "firelook") gs_P.m_FireLook = std::stoi(V);
		else if(K == "survive") gs_P.m_Survive = std::stoi(V);
		else if(K == "hookrange") gs_P.m_HookRange = std::stof(V);
		else if(K == "hookdedup") gs_P.m_HookDedup = std::stoi(V);
		else if(K == "rothook") gs_P.m_RotHook = std::stoi(V);
		else if(K == "imit") gs_P.m_Imit = V;
		else if(K == "inc") gs_P.m_Inc = V;
		else if(K == "imitoff") gs_P.m_ImitOff = std::stof(V);
		else if(K == "imitw") gs_P.m_ImitW = std::stoi(V);
		else if(K == "imithooks") gs_P.m_ImitHooks = std::stoi(V);
		else if(K == "imitdir") gs_P.m_ImitDir = std::stoi(V);
		else if(K == "quant") gs_P.m_Quant = std::stoi(V);
		else if(K == "hookla") gs_P.m_HookLA = std::stoi(V);
		else if(K == "hookidle") gs_P.m_HookIdle = std::stof(V);
		else if(K == "prefire") gs_P.m_Prefire = std::stoi(V);
		else if(K == "shotref") gs_P.m_ShotRef = V;
		else if(K == "shotbonus") gs_P.m_ShotBonus = std::stof(V);
		else if(K == "shotrad") gs_P.m_ShotRad = std::stof(V);
		else if(K == "shotplan") gs_P.m_ShotPlan = V;
		else if(K == "planpre") gs_P.m_PlanPre = std::stof(V);
		else if(K == "planpost") gs_P.m_PlanPost = std::stof(V);
		else if(K == "plangap") gs_P.m_PlanGap = std::stof(V);
		else if(K == "planr") gs_P.m_PlanR = std::stof(V);
		else if(K == "gatewait") gs_P.m_GateWait = std::stoi(V);
		else if(K == "quiet") gs_P.m_Quiet = std::stoi(V);
		else if(K == "horizon") gs_P.m_Horizon = std::stoi(V);
		else if(K == "sinks") gs_P.m_Sinks = V;
		else if(K == "commitk") gs_P.m_CommitK = std::stoi(V);
		else if(K == "ghost") gs_P.m_Ghost = std::stoi(V);
		else if(K == "ghoste") gs_P.m_GhostE = std::stof(V);
		else if(K == "vcap") gs_P.m_VCap = std::stof(V);
		else if(K == "e4w") gs_P.m_GhostE4 = std::stof(V);
		else if(K == "latpen") gs_P.m_LatPen = std::stof(V);
		else if(K == "tpk") gs_P.m_TpK = std::stoi(V);
		else if(K == "tpreload") gs_P.m_TpReload = std::stoi(V);
		else if(K == "tpline") gs_P.m_TpLine = std::stoi(V);
		else if(K == "quota") gs_P.m_Quota = std::stoi(V);
		else if(K == "qcell") gs_P.m_QCell = std::stof(V);
		else if(K == "qvel") gs_P.m_QVel = std::stof(V);
		else if(K == "kcredit") gs_P.m_KCredit = std::stof(V);
		else if(K == "track") gs_P.m_TrackW = std::stof(V);
		else if(K == "trackoff") gs_P.m_TrackOff = std::stoi(V);
		else if(K == "trackcap") gs_P.m_TrackCap = std::stof(V);
		else if(K == "tracktie") gs_P.m_TrackTie = std::stof(V);
		else if(K == "kready") gs_P.m_KReady = std::stoi(V);
		else if(K == "kfut") gs_P.m_KFut = std::stof(V);
		else if(K == "firemax") gs_P.m_FireMax = std::stoi(V);
		else if(K == "erel") gs_P.m_ERel = std::stof(V);
		else if(K == "boxe") gs_P.m_BoxE = std::stof(V);
		else if(K == "loadres") gs_P.m_LoadRes = std::stof(V);
		else if(K == "pfcred") gs_P.m_PfCred = std::stof(V);
		else if(K == "retro") gs_P.m_Retro = std::stoi(V);
		else if(K == "retrominf") gs_P.m_RetroMinF = std::stoi(V);
		else if(K == "retromaxf") gs_P.m_RetroMaxF = std::stoi(V);
		else if(K == "retrotop") gs_P.m_RetroTop = std::stoi(V);
		else if(K == "retroafter") gs_P.m_RetroAfter = std::stoi(V);
		else if(K == "retrorad") gs_P.m_RetroRad = std::stof(V);
		else if(K == "kickmin") gs_P.m_KickMin = std::stof(V);
		else if(K == "kfutn") gs_P.m_KFutN = std::stoi(V);
		else if(K == "tp") std::sscanf(V.c_str(), "%f,%f,%f,%f", &gs_P.m_TpPos.x, &gs_P.m_TpPos.y, &gs_P.m_TpVel.x, &gs_P.m_TpVel.y);
		else if(K == "latdz") gs_P.m_LatDz = std::stof(V);
		else if(K == "latk0") gs_P.m_LatK0 = std::stof(V);
		else if(K == "latk1") gs_P.m_LatK1 = std::stof(V);
		else if(K == "vmin") gs_P.m_VMin = std::stof(V);
		else if(K == "brake") gs_P.m_Brake = std::stof(V);
		else if(K == "brakepen") gs_P.m_BrakePen = std::stof(V);
		else if(K == "ghostsink") gs_P.m_GhostSink = std::stof(V);
		else
		{
			std::printf("unknown option %s\n", K.c_str());
			return 1;
		}
	}
	if(!gs_Ref.Load(gs_P.m_Ref.c_str()))
	{
		std::printf("cannot read %s\n", gs_P.m_Ref.c_str());
		return 1;
	}
	// gate on the reference line: a Teero tick, or the end for grenade/finish gates
	int GateIdx = (int)gs_Ref.m_vP.size() - 1;
	{
		const SMapInfo &M = CTasGame::Map();
		// the route's grenade is the first one on the reference line (the finish room has more pickups)
		float Best = 1e30f;
		for(int y = 0; y < M.m_H; y++)
			for(int x = 0; x < M.m_W; x++)
				if(M.Tile(x, y) == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
				{
					vec2 Gp(x * 32 + 16, y * 32 + 16);
					int BestI = -1;
					float BestD = 1e30f;
					for(int i = 0; i < (int)gs_Ref.m_vP.size(); i++)
						if(distance(gs_Ref.m_vP[i], Gp) < BestD)
						{
							BestD = distance(gs_Ref.m_vP[i], Gp);
							BestI = i;
						}
					if(BestD < 200 && BestI >= 0 && BestI < Best)
					{
						Best = BestI;
						gs_GrenIdx = BestI;
					}
				}
	}
	if(gs_P.m_Gate.rfind("box:", 0) == 0)
	{
		gs_Box = true;
		std::sscanf(gs_P.m_Gate.c_str() + 4, "%f,%f,%f,%f,%f,%f", &gs_aBox[0], &gs_aBox[1], &gs_aBox[2], &gs_aBox[3], &gs_aBox[4], &gs_aBox[5]);
		vec2 C((gs_aBox[0] + gs_aBox[1]) / 2, (gs_aBox[2] + gs_aBox[3]) / 2);
		float Best = 1e30f;
		for(int i = 0; i < (int)gs_Ref.m_vP.size(); i++)
			if(gs_aBox[5] >= 0 ? gs_Ref.m_vK[i] >= gs_aBox[5] && Best > 1e29f : distance(gs_Ref.m_vP[i], C) < Best)
			{
				Best = distance(gs_Ref.m_vP[i], C);
				GateIdx = i;
			}
	}
	else if(gs_P.m_Gate != "finish" && gs_P.m_Gate != "grenade")
	{
		int K = std::stoi(gs_P.m_Gate);
		for(int i = 0; i < (int)gs_Ref.m_vK.size(); i++)
			if(gs_Ref.m_vK[i] >= K)
			{
				GateIdx = i;
				break;
			}
	}
	else if(gs_P.m_Gate == "grenade")
	{
		// the reference point closest to the grenade pickup
		const SMapInfo &M = CTasGame::Map();
		vec2 Gp(0, 0);
		for(int y = 0; y < M.m_H; y++)
			for(int x = 0; x < M.m_W; x++)
				if(M.Tile(x, y) == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
					Gp = vec2(x * 32 + 16, y * 32 + 16);
		float Best = 1e30f;
		for(int i = 0; i < (int)gs_Ref.m_vP.size(); i++)
			if(distance(gs_Ref.m_vP[i], Gp) < Best)
			{
				Best = distance(gs_Ref.m_vP[i], Gp);
				GateIdx = i;
			}
	}
	int EndIdx = GateIdx;
	while(EndIdx + 1 < (int)gs_Ref.m_vK.size() && gs_Ref.m_vK[EndIdx + 1] <= gs_Ref.m_vK[GateIdx] + gs_P.m_Horizon)
		EndIdx++;
	gs_Ref.Build(GateIdx, EndIdx);
	gs_Ref.PrepCaps(gs_P.m_VCap, gs_P.m_VMin);
	gs_Ref.PrepET();
	if(!gs_P.m_Imit.empty())
		LoadImit(gs_P.m_Imit.c_str());
	if(!gs_P.m_Plan.empty())
	{
		LoadPlan(gs_P.m_Plan.c_str());
		std::printf("plan: %zu entries\n", gs_vPlanE.size());
	}
	if(!gs_P.m_ShotPlan.empty())
	{
		LoadShotPlan(gs_P.m_ShotPlan.c_str());
		std::printf("shotplan: %zu kick slots\n", gs_vPlan.size());
	}
	if(!gs_P.m_ShotRef.empty())
	{
		LoadShotRef(gs_P.m_ShotRef.c_str());
		std::printf("shotref: %zu reference explosions\n", gs_vShotRef.size());
	}
	{
		size_t p = 0;
		while(p < gs_P.m_Sinks.size())
		{
			size_t q = gs_P.m_Sinks.find(',', p);
			if(q == std::string::npos)
				q = gs_P.m_Sinks.size();
			int K = std::stoi(gs_P.m_Sinks.substr(p, q - p));
			for(int i = 0; i < (int)gs_Ref.m_vK.size(); i++)
				if(gs_Ref.m_vK[i] >= K)
				{
					gs_Ref.m_vSinkIdx.push_back(i);
					break;
				}
			p = q + 1;
		}
		std::sort(gs_Ref.m_vSinkIdx.begin(), gs_Ref.m_vSinkIdx.end());
	}

	auto t0 = std::chrono::steady_clock::now();
	std::vector<STasInput> vPrefix = ReadInputs(gs_P.m_Prefix.c_str());
	std::vector<std::unique_ptr<CGameT>> vBeam;
	std::vector<STasInput> vPrev;
	std::vector<char> vDoomed;
	{
		auto G = std::make_unique<CTasGame>();
		G->Spawn(CTasGame::Map().m_vSpawns[0]);
		FILE *pTrack = getenv("SEG_TRACK") ? std::fopen(getenv("SEG_TRACK"), "w") : nullptr;
		auto DoTp = [&]() {
			// diagnostics only: teleport and set the race clock to Teero tick tpk
			G->SetState(gs_P.m_TpPos, gs_P.m_TpVel);
			G->m_Tick += 4000; // m_StartTick must stay >= 0 or CheckRace never records the finish (physlab4)
			G->Chr()->m_Core.m_HookState = HOOK_IDLE;
			G->Chr()->m_Core.m_HookTick = 0;
			G->m_LastHook = 0;
			G->m_StartTick = G->m_Tick - gs_P.m_TpK;
			if(gs_P.m_TpReload >= 0)
				G->Chr()->m_ReloadTimer = gs_P.m_TpReload;
			for(int i = 0; i < (int)gs_Ref.m_vK.size(); i++)
				if(gs_Ref.m_vK[i] >= gs_P.m_TpK - 3)
				{
					G->m_RefIdx = i;
					break;
				}
			UpdateTrack(*G);
		};
		int PrefixIdx = 0;
		for(const auto &In : vPrefix)
		{
			if(gs_P.m_TpK >= 0 && gs_P.m_TpLine >= 0 && PrefixIdx == gs_P.m_TpLine)
				DoTp();
			PrefixIdx++;
			G->Step(In);
			UpdateTrack(*G);
			if(pTrack && G->m_Started && G->m_StartTick >= 0)
				std::fprintf(pTrack, "%d %.2f %.2f\n", G->m_Tick - G->m_StartTick + 3, G->Pos().x, G->Pos().y);
		}
		if(pTrack)
		{
			std::fclose(pTrack);
			std::printf("track written (finish rt %d)\n", G->m_FinishTick >= 0 ? G->m_FinishTick - G->m_StartTick : -1);
			return 0;
		}
		if(!G->m_Started)
		{
			std::printf("the prefix must end after the start line\n");
			return 1;
		}
		if(gs_P.m_TpK >= 0 && (gs_P.m_TpLine < 0 || gs_P.m_TpLine >= (int)vPrefix.size()))
			DoTp(); // after the whole prefix
#ifdef SEG_FAST
		auto pF = std::make_unique<CTasFast>();
		pF->FromGameFull(*G);
		if(pF->m_Bad)
		{
			std::printf("segf: the prefix state has something CFastG does not model\n");
			return 1;
		}
		auto &GT = pF;
#else
		auto &GT = G;
#endif
		float Ee;
		float S = EstTotal(*GT, &Ee);
		std::printf("start: rt %d pos %.0f %.0f ref idx %d (teero tick %.0f) gate idx %d (tick %.0f) est total %.1f Ee %.0f gren idx %d has %d\n", GT->m_Tick - GT->m_StartTick, GT->Pos().x, GT->Pos().y,
			GT->m_RefIdx, gs_Ref.m_vK[GT->m_RefIdx], gs_Ref.m_GateIdx, gs_Ref.m_vK[gs_Ref.m_GateIdx], S, Ee, gs_GrenIdx, (int)GT->HasGrenade());
		gs_vTickIdx.assign(4000, -1);
		for(int i = 0; i < (int)gs_Ref.m_vK.size(); i++)
		{
			int K = (int)gs_Ref.m_vK[i];
			if(K >= 0 && K < 4000 && gs_vTickIdx[K] < 0)
				gs_vTickIdx[K] = i;
		}
		if(gs_P.m_TrackOff == -100000)
			gs_P.m_TrackOff = (int)std::lround(gs_Ref.m_vK[GT->m_RefIdx]) - (GT->m_Tick - GT->m_StartTick);
		if(gs_P.m_TrackW > 0)
			std::printf("tracking: offset %d (teero tick = race tick + offset)\n", gs_P.m_TrackOff);
		vPrev.push_back(vPrefix.empty() ? STasInput{} : vPrefix.back());
		vBeam.push_back(std::move(GT));
	}
	std::vector<STasInput> vInc;
	int IncIdx = -1;
	if(!gs_P.m_Inc.empty())
	{
		vInc = ReadInputs(gs_P.m_Inc.c_str());
		auto Same = [](const STasInput &a, const STasInput &b) {
			return a.m_Dir == b.m_Dir && a.m_Jump == b.m_Jump && a.m_Hook == b.m_Hook && a.m_Fire == b.m_Fire && a.m_TX == b.m_TX && a.m_TY == b.m_TY && a.m_Weapon == b.m_Weapon;
		};
		bool Ok = vInc.size() > vPrefix.size();
		for(size_t k = 0; Ok && k < vPrefix.size(); k++)
			Ok = Same(vInc[k], vPrefix[k]);
		if(!Ok || gs_P.m_TpK >= 0)
		{
			std::printf("inc: the incumbent must continue the prefix (and no tp)\n");
			return 1;
		}
		IncIdx = 0;
		std::printf("incumbent: %zu inputs (%zu after the prefix)\n", vInc.size(), vInc.size() - vPrefix.size());
	}
	auto SameIn = [](const STasInput &a, const STasInput &b) {
		return a.m_Dir == b.m_Dir && a.m_Jump == b.m_Jump && a.m_Hook == b.m_Hook && a.m_Fire == b.m_Fire && a.m_TX == b.m_TX && a.m_TY == b.m_TY && a.m_Weapon == b.m_Weapon && a.m_Commit == b.m_Commit;
	};
	std::vector<std::vector<std::pair<int, STasInput>>> vHist;
	std::vector<std::vector<SAux>> vAux; // per vHist node: position / loaded / hook idle (retro)
	std::vector<std::vector<SPatch>> vHistR; // per vHist node: retro shot patched into an ancestor's input
	std::unordered_map<int64_t, float> Dom;
	SGate BestGate;
	std::mutex GateMx;
	int FirstGateStep = -1;
	const int NT = std::max(1, gs_P.m_Threads);

	for(int Step = 0; Step < gs_P.m_MaxTicks && !vBeam.empty(); Step++)
	{
		if(FirstGateStep >= 0 && Step > FirstGateStep + gs_P.m_GateWait)
			break;
		std::vector<std::vector<SCand>> vTC(NT);
		std::atomic<int> Next{0};
		auto Worker = [&](int T) {
			CGameT Tmp, Look;
			std::vector<STasInput> vActs;
			while(true)
			{
				int i = Next.fetch_add(1);
				if(i >= (int)vBeam.size())
					break;
				if(!vDoomed.empty() && vDoomed[i])
					continue;
				const CGameT &G = *vBeam[i];
				GenActions(G, vPrev[i], vActs, i < gs_P.m_PadTop);
				const bool IsInc = i == IncIdx && vPrefix.size() + Step < vInc.size();
				STasInput IncIn;
				if(IsInc)
				{
					IncIn = vInc[vPrefix.size() + Step];
					bool Have = false;
					for(const auto &A : vActs)
						Have |= SameIn(A, IncIn);
					if(!Have)
						vActs.push_back(IncIn);
				}
				auto Child = [&](const CGameT &Src, const STasInput &In, const SRetro *pR) {
					Tmp.CopyFrom(Src);
#ifdef SEG_FAST
					static thread_local std::vector<SExplLog> s_vExpl;
					s_vExpl.clear();
					if(gs_P.m_KickMin > 0)
						CFastG::ms_pLog = &s_vExpl;
#endif
					Tmp.Step(In);
					if(Tmp.Frozen() || Tmp.EnteredFreeze() || Tmp.m_StartTick == -2)
					{
						if(getenv("SEG_DBG") && Step < 30)
							std::printf("drop step %d frz %d entered %d start %d pos %.0f %.0f\n", Step, Tmp.Frozen(), Tmp.EnteredFreeze(), Tmp.m_StartTick, Tmp.Pos().x, Tmp.Pos().y);
						return;
					}
					UpdateTrack(Tmp);
					Tmp.m_TrackCost += TrackInc(Tmp);
					ShotBonus(Src, Tmp, In);
#ifdef SEG_FAST
					if(Tmp.NumProjectiles() == 0)
						Tmp.m_PendCredit = 0;
#endif
					Tmp.m_ReadyTicks = Tmp.HasGrenade() && Tmp.ReloadTimer() == 0 ? Src.m_ReadyTicks + 1 : 0;
					if(gs_P.m_FireMax >= 0 && Tmp.m_ReadyTicks > gs_P.m_FireMax && !(!pR && IsInc && SameIn(In, IncIn)))
						return;
					if(!ShotPlanOk(Tmp, In, Src.ReloadTimer() == 0 && Tmp.ReloadTimer() > 0))
						return;
					if(gs_P.m_PlanForce && Tmp.ReloadTimer() == 0)
					{
						const int RtF = Tmp.m_Tick - Tmp.m_StartTick;
						bool Missed = false;
						for(const auto &P : gs_vPlanE)
							if(!P.m_Free && RtF == P.m_T1 + 1)
								Missed = true;
						if(Missed)
							return;
					}
					if(!Tmp.HasGrenade() && Tmp.m_RefIdx > gs_GrenIdx + 3)
						return; // passed the pickup without the grenade
					const int Rt = Tmp.m_Tick - Tmp.m_StartTick;
					if(AtGate(Tmp))
					{
						float Ee;
						float V = gs_P.m_Gate == "finish" ? (float)(Tmp.m_FinishTick - Tmp.m_StartTick) : EstTotal(Tmp, &Ee);
						if(gs_Box)
							V = (float)Rt + 0.01f * Tmp.Vel().y - gs_P.m_BoxE * (dot(Tmp.Vel(), Tmp.Vel()) - Tmp.Pos().y);
						// (EstTotal includes the time model's horizon beyond the gate, so energy at the gate counts)
						if(gs_P.m_Gate == "finish")
							Ee = EffEnergy(Tmp);
						std::lock_guard<std::mutex> L(GateMx);
						if(V < BestGate.m_V - 1e-4f && (gs_P.m_Gate == "finish" || Survives(Tmp, gs_P.m_Survive)))
						{
							BestGate.m_V = V;
							BestGate.m_Step = Step;
							BestGate.m_Parent = i;
							BestGate.m_R = pR ? *pR : SRetro{};
							BestGate.m_In = In;
							BestGate.m_Rt = gs_P.m_Gate == "finish" ? Tmp.m_FinishTick - Tmp.m_StartTick : Rt;
							BestGate.m_Ee = Ee;
							BestGate.m_Pos = Tmp.Pos();
							BestGate.m_Vel = Tmp.Vel();
						}
						return;
					}
#ifdef SEG_FAST
					auto WeakKick = [&]() {
						for(const auto &L : s_vExpl)
							if(length(L.m_Force) < gs_P.m_KickMin)
								return true;
						return false;
					};
					if(gs_P.m_KickMin > 0 && WeakKick())
					{
						CFastG::ms_pLog = nullptr;
						return;
					}
#endif
					const CGameT *pEval = &Tmp;
					if(gs_P.m_TrackW <= 0 && (In.m_Fire || (gs_P.m_PendLook && !gs_P.m_Prefire)) && Tmp.NumProjectiles() > 0)
					{
						// judge a shot after its explosion
						Look.CopyFrom(Tmp);
						STasInput L = In;
						L.m_Fire = 0;
						bool Dead = false;
						for(int k = 0; k < gs_P.m_FireLook && Look.NumProjectiles() > 0; k++)
						{
							Look.Step(L);
							UpdateTrack(Look);
							if(Look.Frozen() || Look.EnteredFreeze() || Look.m_StartTick == -2)
							{
								Dead = true;
								break;
							}
						}
#ifdef SEG_FAST
						if(gs_P.m_KickMin > 0 && WeakKick())
							Dead = true;
#endif
						// (the incumbent's own continuation is never dropped: a held-input look is only a guess)
						const bool IncChild = !pR && IsInc && SameIn(In, IncIn);
						if(Dead && !IncChild)
						{
#ifdef SEG_FAST
							CFastG::ms_pLog = nullptr;
#endif
							return;
						}
						// prefire: a shot still in flight after the look is judged as if not fired yet
						pEval = Dead || (gs_P.m_Prefire && Look.NumProjectiles() > 0) ? &Tmp : &Look;
#ifdef SEG_FAST
						if(gs_P.m_PfCred > 0 && pEval == &Tmp && Src.ReloadTimer() == 0 && Tmp.ReloadTimer() > 0)
						{
							// pfcred: fly the held-input look on to the explosion, and the same look without the
							// grenade; the time it saves is credited until it explodes
							int k = gs_P.m_FireLook;
							bool DeadA = false;
							for(; k < gs_P.m_PadRange + 2 && Look.NumProjectiles() > 0; k++)
							{
								Look.Step(L);
								UpdateTrack(Look);
								if(Look.Frozen() || Look.EnteredFreeze() || Look.m_StartTick == -2)
								{
									DeadA = true;
									break;
								}
							}
							float C = 0;
							if(!DeadA && Look.NumProjectiles() == 0)
							{
								static thread_local CGameT s_LB;
								s_LB.CopyFrom(Tmp);
								s_LB.m_NumProj = 0;
								bool DeadB = false;
								for(int j = 0; j < k; j++)
								{
									s_LB.Step(L);
									UpdateTrack(s_LB);
									if(s_LB.Frozen() || s_LB.EnteredFreeze() || s_LB.m_StartTick == -2)
									{
										DeadB = true;
										break;
									}
								}
								C = DeadB ? 0.0f : std::max(0.0f, EstTotal(s_LB) - EstTotal(Look));
							}
							Tmp.m_PendCredit = gs_P.m_PfCred * C;
						}
#endif
					}
#ifdef SEG_FAST
					CFastG::ms_pLog = nullptr;
#endif
					float Ee;
					float S = EstTotal(*pEval, &Ee);
#ifdef SEG_FAST
					if(pEval == &Tmp && Tmp.NumProjectiles() > 0)
						S -= Tmp.m_PendCredit;
#endif
					if(gs_P.m_HookLA > 0 && pEval == &Tmp && Tmp.HookState() >= HOOK_FLYING)
					{
						vec2 Pp, Vf, Vp;
						if(HookLookahead(Tmp, In.m_Dir, Pp, Vf, Vp))
						{
							// value change the predicted pull makes (evaluated at the predicted point)
							Look.CopyFrom(Tmp);
							Look.SetState(Pp, Vf);
							float Sf = EstTotal(Look);
							Look.SetState(Pp, Vp);
							S += EstTotal(Look) - Sf;
						}
						else
							S += gs_P.m_HookIdle;
					}
					if(gs_P.m_Jitter > 0)
					{
						// stochastic beam: deterministic per (seed, state) noise on the score
						uint64_t h = Tmp.Hash() ^ (0x9E3779B97F4A7C15ull * (uint64_t)(gs_P.m_Seed + 1));
						h ^= h >> 33;
						h *= 0xff51afd7ed558ccdull;
						h ^= h >> 33;
						S += gs_P.m_Jitter * ((h & 0xffffff) / (float)0xffffff - 0.5f) * 2.0f;
					}
					if(gs_P.m_CrashW > 0)
						S += gs_P.m_CrashW * Tmp.CrashLoss(gs_P.m_CrashN); // v^2 about to be lost against walls/ceilings
					if(gs_P.m_PlanRv > 0 && Tmp.NumProjectiles() > 0)
					{
						// rendezvous credit: a grenade in flight is worth up to 4 x planrv ticks if the tee can still be
						// next to its explosion point when it goes off (ballistic guess +- what control can change)
						vec2 E;
						int Te;
						if(PendExpl(Tmp, E, Te))
						{
							const int dt = Te - Tmp.m_Tick;
							if(dt >= 1 && dt <= 60)
							{
								vec2 P = Tmp.Pos(), V = Tmp.Vel();
								float Sp = length(V);
								float Rm = Sp > 11 ? gs_Ref.Disp(Sp) / Sp : 1.0f;
								vec2 Pp(P.x + V.x * Rm * dt, P.y + V.y * dt + 0.25f * dt * (dt + 1));
								float Miss = std::max(0.0f, distance(Pp, E) - 0.5f * gs_P.m_PlanAcc * dt * dt - 40.0f);
								S -= gs_P.m_PlanRv * (4.0f - std::min(Miss, 400.0f) / 100.0f);
							}
						}
					}
					if(gs_P.m_TrackW > 0)
						S = Tmp.m_TrackCost + gs_P.m_TrackTie * S;
					S -= Tmp.m_Bonus;
					vTC[T].push_back({S, i, In, CellKey(Tmp), Tmp.Hash(), Ee, Rt, Tmp.m_RefIdx, QuotaKey(Tmp)});
					vTC[T].back().m_Loaded = Tmp.HasGrenade() && Tmp.ReloadTimer() == 0;
					if(pR)
						vTC[T].back().m_R = *pR;
					if(!pR && IsInc && SameIn(In, IncIn))
						vTC[T].back().m_Inc = true;
				};
				for(const auto &In : vActs)
					Child(G, In, nullptr);
#ifdef SEG_FAST
				if(gs_P.m_Retro > 0 && Step >= 1 && i < gs_P.m_RetroTop && !IsInc)
				{
					// retro shots: a grenade fired from an ancestor that was holding it, exploding next to us next tick
					static thread_local std::vector<SRetro> s_vR;
					static thread_local CGameT s_Gr;
					RetroFind(G, Step - 1, i, vHist, vAux, s_vR);
					for(const auto &R : s_vR)
					{
						s_Gr.CopyFrom(G);
						ApplyRetro(s_Gr, R);
						for(const auto &In : vActs)
							if(!(In.m_Fire && s_Gr.ReloadTimer() > 0))
								Child(s_Gr, In, &R);
					}
				}
#endif
			}
		};
		std::vector<std::thread> vTh;
		for(int T = 1; T < NT; T++)
			vTh.emplace_back(Worker, T);
		Worker(0);
		for(auto &Th : vTh)
			Th.join();
		if(BestGate.m_Step >= 0 && FirstGateStep < 0)
			FirstGateStep = Step;

		std::vector<SCand> vAll;
		for(auto &v : vTC)
			vAll.insert(vAll.end(), v.begin(), v.end());
		std::sort(vAll.begin(), vAll.end(), [](const SCand &a, const SCand &b) { return a.m_S != b.m_S ? a.m_S < b.m_S : a.m_Hash < b.m_Hash; });
		std::unordered_set<uint64_t> Seen;
		std::unordered_set<int64_t> Cells;
		std::vector<SCand> vSel;
		if(gs_P.m_LoadRes > 0)
		{
			// loadres: the best states that keep a loaded grenade get loadres x beam places first, so waiting for a
			// better kick (e.g. a turning kick at the next bend) survives the ticks where firing now looks better
			const int Res = (int)(gs_P.m_LoadRes * gs_P.m_Beam);
			for(const auto &C : vAll)
			{
				if((int)vSel.size() >= Res)
					break;
				if(!C.m_Loaded || !Seen.insert(C.m_Hash).second || !Cells.insert(C.m_Key).second)
					continue;
				vSel.push_back(C);
			}
		}
		if(gs_P.m_Quota <= 0)
			for(const auto &C : vAll)
			{
				if((int)vSel.size() >= gs_P.m_Beam)
					break;
				if(!Seen.insert(C.m_Hash).second || !Cells.insert(C.m_Key).second)
					continue;
				if(gs_P.m_Dom)
				{
					auto it = Dom.find(C.m_Key);
					if(it != Dom.end() && it->second <= C.m_S + 0.5f)
						continue;
				}
				vSel.push_back(C);
			}
		else
		{
			// diversity quota (physlab4): at most `quota` states per coarse position/velocity cell in a first pass,
			// then the beam is filled with the best of the rest
			std::unordered_map<int64_t, int> QCnt;
			std::vector<char> vTaken(vAll.size(), 0);
			for(int Pass = 0; Pass < 2; Pass++)
				for(size_t ci = 0; ci < vAll.size(); ci++)
				{
					const auto &C = vAll[ci];
					if((int)vSel.size() >= gs_P.m_Beam)
						break;
					if(vTaken[ci] || Seen.count(C.m_Hash) || Cells.count(C.m_Key))
						continue;
					if(gs_P.m_Dom)
					{
						auto it = Dom.find(C.m_Key);
						if(it != Dom.end() && it->second <= C.m_S + 0.5f)
							continue;
					}
					if(Pass == 0)
					{
						int &Q = QCnt[C.m_QKey];
						if(Q >= gs_P.m_Quota)
							continue;
						Q++;
					}
					Seen.insert(C.m_Hash);
					Cells.insert(C.m_Key);
					vTaken[ci] = 1;
					vSel.push_back(C);
				}
		}
		if(IncIdx >= 0)
		{
			IncIdx = -1;
			const SCand *pInc = nullptr;
			for(const auto &C : vAll)
				if(C.m_Inc)
					pInc = &C;
			if(pInc)
			{
				for(size_t k = 0; k < vSel.size() && IncIdx < 0; k++)
					if(vSel[k].m_Hash == pInc->m_Hash)
						IncIdx = (int)k; // the same physical state is kept (the incumbent's inputs continue it)
				if(IncIdx < 0)
				{
					if((int)vSel.size() >= gs_P.m_Beam && !vSel.empty())
						vSel.back() = *pInc;
					else
						vSel.push_back(*pInc);
					IncIdx = (int)vSel.size() - 1;
				}
			}
		}
		for(const auto &C : vSel)
		{
			float &D = Dom.try_emplace(C.m_Key, 1e30f).first->second;
			D = std::min(D, C.m_S);
		}
		std::vector<std::unique_ptr<CGameT>> vNew(vSel.size());
		std::vector<STasInput> vNewPrev(vSel.size());
		std::atomic<int> Next2{0};
		auto Mat = [&]() {
			while(true)
			{
				int k = Next2.fetch_add(1);
				if(k >= (int)vSel.size())
					break;
				vNew[k] = std::make_unique<CGameT>();
				vNew[k]->CopyFrom(*vBeam[vSel[k].m_Parent]);
#ifdef SEG_FAST
				if(vSel[k].m_R.m_Step >= 0)
					ApplyRetro(*vNew[k], vSel[k].m_R);
#endif
				vNew[k]->Step(vSel[k].m_In);
				UpdateTrack(*vNew[k]);
				vNew[k]->m_TrackCost += TrackInc(*vNew[k]);
				ShotBonus(*vBeam[vSel[k].m_Parent], *vNew[k], vSel[k].m_In);
				vNew[k]->m_ReadyTicks = vNew[k]->HasGrenade() && vNew[k]->ReloadTimer() == 0 ? vBeam[vSel[k].m_Parent]->m_ReadyTicks + 1 : 0;
				vNewPrev[k] = vSel[k].m_In;
			}
		};
		std::vector<std::thread> vTh2;
		for(int T = 1; T < NT; T++)
			vTh2.emplace_back(Mat);
		Mat();
		for(auto &Th : vTh2)
			Th.join();
		std::vector<std::pair<int, STasInput>> H(vSel.size());
		for(size_t k = 0; k < vSel.size(); k++)
			H[k] = {vSel[k].m_Parent, vSel[k].m_In};
		vHist.push_back(std::move(H));
		{
			std::vector<SAux> Ax(vSel.size());
			std::vector<SPatch> Pr(vSel.size());
			for(size_t k = 0; k < vSel.size(); k++)
			{
				const CGameT &N = *vNew[k];
#ifdef SEG_FAST
				Ax[k].m_Pos = N.m_Pos;
				Ax[k].m_Flags = (N.HasGrenade() && N.ReloadTimer() == 0 && N.m_Core.m_ActiveWeapon == WEAPON_GRENADE && N.m_NumInputs >= 2 ? 1 : 0) |
						(N.m_Core.m_HookState == HOOK_IDLE ? 2 : 0);
#else
				Ax[k].m_Pos = N.Pos();
				Ax[k].m_Flags = 0;
#endif
				Pr[k] = {vSel[k].m_R.m_Step, vSel[k].m_R.m_TX, vSel[k].m_R.m_TY};
			}
			vAux.push_back(std::move(Ax));
			vHistR.push_back(std::move(Pr));
		}
		vBeam = std::move(vNew);
		vPrev = std::move(vNewPrev);
		vDoomed.assign(vBeam.size(), 0);
		if(gs_P.m_SurvEvery > 0 && Step % gs_P.m_SurvEvery == 0)
		{
			std::atomic<int> Next3{0}, NDoomed{0};
			auto Chk = [&]() {
				while(true)
				{
					int k = Next3.fetch_add(1);
					if(k >= (int)vBeam.size())
						break;
					if(k != IncIdx && !Survives(*vBeam[k], gs_P.m_Survive))
					{
						vDoomed[k] = 1;
						NDoomed++;
					}
				}
			};
			std::vector<std::thread> vTh3;
			for(int T = 1; T < NT; T++)
				vTh3.emplace_back(Chk);
			Chk();
			for(auto &Th : vTh3)
				Th.join();
			if(NDoomed.load() == (int)vBeam.size())
				vDoomed.assign(vBeam.size(), 0); // all doomed: keep searching anyway
		}
		if(!gs_P.m_Quiet && (Step % 10 == 0 || getenv("SEG_DBG")) && !vSel.empty())
		{
			const SCand &B = vSel[0];
			double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
			std::printf("step %d beam %zu cands %zu best est %.1f rt %d ref %d (teero %.0f) Ee %.0f (%.0fs)\n", Step, vBeam.size(), vAll.size(), B.m_S, B.m_Rt, B.m_Ref,
				gs_Ref.m_vK[std::clamp(B.m_Ref, 0, (int)gs_Ref.m_vK.size() - 1)], B.m_Ee, Sec);
			std::fflush(stdout);
		}
	}
	if(BestGate.m_Step < 0)
	{
		std::printf("NOGATE\n");
		return 0;
	}
	std::vector<STasInput> vRun;
	std::vector<SPatch> vPatch;
	vRun.push_back(BestGate.m_In);
	if(BestGate.m_R.m_Step >= 0)
		vPatch.push_back({BestGate.m_R.m_Step, BestGate.m_R.m_TX, BestGate.m_R.m_TY});
	int Idx = BestGate.m_Parent;
	for(int s = BestGate.m_Step - 1; s >= 0; s--)
	{
		vRun.push_back(vHist[s][Idx].second);
		if(vHistR[s][Idx].m_Step >= 0)
			vPatch.push_back(vHistR[s][Idx]);
		Idx = vHist[s][Idx].first;
	}
	std::reverse(vRun.begin(), vRun.end());
	for(const auto &P : vPatch)
	{
		// retro shot: the ancestor fires the grenade with this aim
		vRun[P.m_Step].m_Fire = 1;
		vRun[P.m_Step].m_TX = P.m_TX;
		vRun[P.m_Step].m_TY = P.m_TY;
	}
	if(!vPatch.empty())
		std::printf("retro shots: %zu\n", vPatch.size());
	std::vector<STasInput> vAllIn = vPrefix;
	vAllIn.insert(vAllIn.end(), vRun.begin(), vRun.end());
	std::string Out = gs_P.m_Out + "0.txt";
	WriteInputs(Out.c_str(), vAllIn);
	std::printf("GATE rt %d value %.2f Ee %.0f pos %.0f %.0f vel %.2f %.2f -> %s (%zu inputs)\n", BestGate.m_Rt, BestGate.m_V, BestGate.m_Ee, BestGate.m_Pos.x, BestGate.m_Pos.y,
		BestGate.m_Vel.x, BestGate.m_Vel.y, Out.c_str(), vAllIn.size());
	if(getenv("SEG_DUMP"))
	{
		// per-tick trajectory of the result (after the prefix; teleport applied like the search)
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		for(const auto &In : vPrefix)
		{
			G.Step(In);
			UpdateTrack(G);
		}
		if(gs_P.m_TpK >= 0)
		{
			G.SetState(gs_P.m_TpPos, gs_P.m_TpVel);
			G.Chr()->m_Core.m_HookState = HOOK_IDLE;
			G.Chr()->m_Core.m_HookTick = 0;
			G.m_LastHook = 0;
			G.m_StartTick = G.m_Tick - gs_P.m_TpK;
			if(gs_P.m_TpReload >= 0)
				G.Chr()->m_ReloadTimer = gs_P.m_TpReload;
			for(int i = 0; i < (int)gs_Ref.m_vK.size(); i++)
				if(gs_Ref.m_vK[i] >= gs_P.m_TpK - 3)
				{
					G.m_RefIdx = i;
					break;
				}
			UpdateTrack(G);
		}
		for(const auto &In : vRun)
		{
			G.Step(In);
			UpdateTrack(G);
			vec2 P = G.Pos(), V = G.Vel();
			std::printf("D rt %d ref %.0f pos %.0f %.0f vel %.2f %.2f |v| %.2f E %.0f in %d %d %d %d %d %d hook %d jumped %d gr %d reload %d proj %d\n", G.m_Tick - G.m_StartTick,
				gs_Ref.m_vK[G.m_RefIdx], P.x, P.y, V.x, V.y, length(V), dot(V, V) - P.y, In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, G.HookState(), G.Jumped(),
				(int)G.Grounded(), G.ReloadTimer(), G.NumProjectiles());
		}
	}
	if(gs_P.m_CommitK >= 0)
	{
		int CIdx = (int)gs_Ref.m_vK.size() - 1;
		for(int i = 0; i < (int)gs_Ref.m_vK.size(); i++)
			if(gs_Ref.m_vK[i] >= gs_P.m_CommitK)
			{
				CIdx = i;
				break;
			}
		CTasGame G;
		G.Spawn(CTasGame::Map().m_vSpawns[0]);
		size_t n = 0;
		for(; n < vAllIn.size(); n++)
		{
			G.Step(vAllIn[n]);
			UpdateTrack(G);
			if(n + 1 > vPrefix.size() && G.m_RefIdx >= CIdx)
			{
				n++;
				break;
			}
		}
		std::vector<STasInput> vC(vAllIn.begin(), vAllIn.begin() + std::min(n, vAllIn.size()));
		std::string OutC = gs_P.m_Out + "c.txt";
		WriteInputs(OutC.c_str(), vC);
		std::printf("COMMIT %zu inputs rt %d ref %d (teero %.0f) -> %s\n", vC.size(), G.m_Tick - G.m_StartTick, G.m_RefIdx, gs_Ref.m_vK[G.m_RefIdx], OutC.c_str());
	}
	return 0;
}
