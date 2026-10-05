// fbeam: physlab4 beam search from a teleported entry state to the finish (final maze, Teero k~2380-2539).
// Progress = geodesic distance D to the finish tiles (4-px distance field, tee box vs solid, centre vs freeze),
// turned into time by a model T(D) (model=0: D/v0; model=1: Teero's own remaining time at that D).
// Ranking: S = t + tau + T(D(p_tau)), p_tau = best end point of short core-only rollouts (dir -1/0/1, hook kept /
// released); a rollout that touches freeze pays deathpen. Diversity: at most q states per position cell (cell px),
// fine dedup key (pos/keypos, vel/keyvel, hook, jumps, reload bucket, pending shot), states worse than the best by
// more than margin ticks are dropped.
// usage: fbeam <map> key=value...   (entry keys: tp=x,y,vx,vy tpk=K reload=N air=0|1 prefix=FILE)
//   beam=4000 q=6 cell=32 qvel=6 (quota key: pos/cell, vel/qvel) keypos=8 keyvel=2 angles=64 rothook=1 fire=1 fireangles=48 firelook=10 firetop=1000000
//   prefire=0 (also shots exploding up to prefire ticks later, judged neutrally) tau=8 deathpen=30 margin=25
//   model=1 v0=22 maxticks=220 out=results/fb (writes OUT.txt = inputs after the entry, OUT.trace)
//   gate=finish | gate=y<Y | gate=x>X ... (simple gates for sub-problems: first tick reaching it)
#include "flcommon.h"

#include <unordered_map>
#include <unordered_set>

struct SPar
{
	int m_Beam = 4000, m_Q = 6;
	float m_Cell = 32, m_KeyPos = 8, m_KeyVel = 2, m_QVel = 6;
	int m_Angles = 64, m_RotHook = 1;
	int m_Fire = 1, m_FireAngles = 48, m_FireLook = 10, m_FireTop = 1000000, m_Prefire = 0;
	int m_Tau = 8;
	float m_DeathPen = 30, m_Margin = 25;
	int m_Model = 1;
	float m_V0 = 22;
	float m_HNow = 0, m_VMax = 45;
	float m_LatPen = 0, m_LatDz = 64;
	float m_RlW = 0, m_GhostE = 0, m_ECap = 30;
	int m_EHgt = 0;
	float m_HybW = 0.5f;
	int m_Fill = 1; // fill the beam past the quota with the best remaining states
	int m_VKick = 1;
	float m_TrackW = 0, m_TrackCap = 96, m_TrackTie = 0.05f;
	int m_TrackOff = 0;
	float m_HookDedup = 0; // >0: one hook aim per hit point cell of this many px (0: per tile)
	float m_GateVW = 0; // gate value = tick - subtick - gatevw * (velocity along the gate normal)
	int m_GateWait = 0; // keep searching this many steps after the first gate hit
	std::string m_Pre; // post-entry inputs applied before the search (chaining) // rollouts may take one virtual point-blank kick to avoid freeze
	int m_MaxTicks = 220;
	std::string m_Out = "results/fb";
	std::string m_Gate = "finish";
	int m_Verbose = 1;
	int m_QExtra = 1; // quota key also separates jumps left / grenade loaded / shot in flight
	float m_MinKick = 5; // shots must kick at least this much (px/t)
	int m_JumpCred = 0; // ticks of credit per jump left (tie-break only)
	float m_RlCred = 0; // ticks of credit when the grenade is loaded
};
static SPar gs_P;
static std::vector<STasInput> gs_vPreIn;

// ---------- time model ----------
static std::vector<std::pair<float, float>> gs_vTD; // (D, Teero remaining ticks), D increasing
static void BuildTimeModel()
{
	FILE *f = std::fopen("../teero_track.txt", "r");
	char aLine[256];
	std::vector<std::pair<int, float>> v;
	while(f && std::fgets(aLine, sizeof(aLine), f))
	{
		int k;
		float x, y;
		if(std::sscanf(aLine, "%d %f %f", &k, &x, &y) == 3 && k >= 2300)
			v.push_back({k, gs_DF.Sample(vec2(x, y))});
	}
	if(f)
		std::fclose(f);
	// cumulative max from the end: D must grow going back in time (stalls keep the earliest time)
	float Mx = -1;
	for(int i = (int)v.size() - 1; i >= 0; i--)
	{
		if(v[i].second > 1e8f)
			continue;
		if(v[i].second > Mx + 0.5f)
		{
			Mx = v[i].second;
			gs_vTD.push_back({Mx, 2539.0f - v[i].first});
		}
	}
	std::sort(gs_vTD.begin(), gs_vTD.end());
}
static float TModel(float D)
{
	if(D > 1e8f)
		return 1e6f;
	if(gs_P.m_Model == 0 || gs_vTD.empty())
		return D / gs_P.m_V0;
	if(D <= gs_vTD.front().first)
		return gs_vTD.front().second * D / std::max(gs_vTD.front().first, 1.0f);
	if(D >= gs_vTD.back().first)
		return gs_vTD.back().second + (D - gs_vTD.back().first) / gs_P.m_V0;
	auto It = std::lower_bound(gs_vTD.begin(), gs_vTD.end(), std::make_pair(D, -1e9f));
	auto Hi = *It, Lo = *(It - 1);
	float a = (D - Lo.first) / std::max(Hi.first - Lo.first, 1e-3f);
	// where Teero's D stalls (several ticks at about the same D) take the earliest time
	return Lo.second + a * (Hi.second - Lo.second);
}

// ---------- reference line (model=2): Teero's smoothed track, his own remaining time at the projected point ----------
static std::vector<vec2> gs_vRP;
static std::vector<float> gs_vRK;
static void LoadRef()
{
	FILE *f = std::fopen("../teero_track.txt", "r");
	char aLine[256];
	std::vector<std::pair<int, vec2>> v;
	while(f && std::fgets(aLine, sizeof(aLine), f))
	{
		int k;
		float x, y;
		if(std::sscanf(aLine, "%d %f %f", &k, &x, &y) == 3 && k >= 2340 && k <= 2539)
			v.push_back({k, vec2(x, y)});
	}
	if(f)
		std::fclose(f);
	const int N = v.size();
	for(int i = 0; i < N; i++)
	{
		vec2 S(0, 0);
		int n = 0;
		for(int d = -2; d <= 2; d++)
		{
			int j = std::clamp(i + d, 0, N - 1);
			S += v[j].second;
			n++;
		}
		gs_vRP.push_back(S / (float)n);
		gs_vRK.push_back(v[i].first);
	}
	// extend past the finish line (to the right along the finish row)
	gs_vRP.push_back(vec2(8900, 4640));
	gs_vRK.push_back(2542);
}
static int RefTrack(int Idx, vec2 P, vec2 V, float &Frac, float *pDist = nullptr)
{
	const float Sp = length(V);
	const float W = 40.0f * std::min(Sp / 8.0f, 1.0f);
	const int N = gs_vRP.size();
	int Lo = std::clamp(Idx - 2, 0, N - 2), Hi = std::clamp(Idx + 25, 0, N - 2);
	float Best = 1e30f, BestD = 0;
	int BestJ = Lo;
	Frac = 0;
	for(int j = Lo; j <= Hi; j++)
	{
		vec2 A = gs_vRP[j], B = gs_vRP[j + 1], AB = B - A;
		float L2 = dot(AB, AB);
		float t = L2 > 1e-6f ? std::clamp(dot(P - A, AB) / L2, 0.0f, 1.0f) : 0.0f;
		float d = distance(A + AB * t, P);
		float dd = d;
		if(W > 0 && L2 > 1e-6f && Sp > 1e-3f)
			dd += W * (1.0f - dot(AB, V) / (std::sqrt(L2) * Sp));
		if(dd < Best - 1e-3f)
		{
			Best = dd;
			BestD = d;
			BestJ = j;
			Frac = t;
		}
	}
	if(pDist)
		*pDist = BestD;
	return BestJ;
}
// time-indexed tracking (track=W): Teero's raw track position at race tick + trackoff
static std::vector<vec2> gs_vTrk(4000, vec2(-1, -1));
static void LoadTrk()
{
	FILE *f = std::fopen("../teero_track.txt", "r");
	char aLine[256];
	while(f && std::fgets(aLine, sizeof(aLine), f))
	{
		int k;
		float x, y;
		if(std::sscanf(aLine, "%d %f %f", &k, &x, &y) == 3 && k >= 0 && k < 4000)
			gs_vTrk[k] = vec2(x, y);
	}
	if(f)
		std::fclose(f);
}
static void TrkUpd(CTasGame &G)
{
	if(gs_P.m_TrackW <= 0)
		return;
	int K = Rt(G) + gs_P.m_TrackOff;
	if(K < 0 || K >= 4000 || gs_vTrk[K].x < 0)
		return;
	float d = std::min(distance(G.Pos(), gs_vTrk[K]), gs_P.m_TrackCap);
	G.m_TrackCost += gs_P.m_TrackW * d * d / 1000.0f;
}
static void Upd(CTasGame &G)
{
	float Fr;
	G.m_RefIdx = RefTrack(G.m_RefIdx, G.Pos(), G.Vel(), Fr);
}
static float RefT(int J, float Frac) { return 2539.0f - (gs_vRK[J] + Frac * (gs_vRK[J + 1] - gs_vRK[J])); }

static float PosValue(const CTasGame &G, vec2 P, vec2 V)
{
	if(gs_P.m_Model == 2 || gs_P.m_Model == 3)
	{
		float Frac, Dist;
		int J = RefTrack(G.m_RefIdx, P, V, Frac, &Dist);
		float T = RefT(J, Frac) + gs_P.m_LatPen * std::max(0.0f, Dist - gs_P.m_LatDz);
		if(gs_P.m_Model == 3)
			return gs_P.m_HybW * TModel(gs_DF.Sample(P)) + (1.0f - gs_P.m_HybW) * T;
		return T;
	}
	return TModel(gs_DF.Sample(P));
}

static bool SegFreeze(vec2 A, vec2 B)
{
	const SMapInfo &M = CTasGame::Map();
	float L = distance(A, B);
	int N = (int)(L / 4.0f) + 1;
	for(int i = 1; i <= N; i++)
	{
		vec2 P = mix(A, B, (float)i / N);
		int x = (int)P.x / 32, y = (int)P.y / 32;
		if(M.Tile(x, y) == TILE_FREEZE || M.Front(x, y) == TILE_FREEZE)
			return true;
	}
	return false;
}

// virtual point-blank kick directions at P: aims whose ray meets a solid within 60 px (freeze is transparent);
// returns the kick vectors (opposite to the aim, explosion strength by distance)
static int VirtualKicks(vec2 P, vec2 *pOut)
{
	const SMapInfo &M = CTasGame::Map();
	int n = 0;
	for(int a = 0; a < 24; a++)
	{
		float Ang = 2 * pi * a / 24;
		vec2 D(std::cos(Ang), std::sin(Ang));
		for(float r = 21.0f; r <= 64.0f; r += 2.0f)
		{
			vec2 Q = P + D * r;
			int T = M.Tile((int)std::floor(Q.x / 32), (int)std::floor(Q.y / 32));
			if(T == TILE_SOLID)
			{
				float f = 1.0f - std::clamp((r - 48.0f) / 87.0f, 0.0f, 1.0f);
				pOut[n++] = -D * (12.0f * f);
				break;
			}
		}
	}
	return n;
}

// core-only rollout like CTasGame::Rollout, but when the tee would touch freeze and the grenade is loaded by then,
// it may take one virtual point-blank kick (best of the available directions by the end value) and go on.
// Returns the ticks survived; pPos gets the positions.
static int RolloutK(const CTasGame &G, int Ticks, int Dir, bool KeepHook, bool AllowKick, vec2 *pPos, float (*Value)(const CTasGame &, vec2, vec2))
{
	CCharacterCore Core = G.Chr()->m_Core;
	Core.SetCoreWorld(nullptr, CTasGame::Collision(), &G.Chr()->GameWorld()->m_Teams);
	CNetObj_PlayerInput In = G.Chr()->m_Input;
	In.m_Direction = Dir;
	In.m_Jump = G.m_LastJump;
	In.m_Hook = KeepHook ? G.m_LastHook : 0;
	In.m_Fire = 0;
	const int Reload = G.HasGrenade() ? G.ReloadTimer() : 1000;
	for(int t = 0; t < Ticks; t++)
	{
		CCharacterCore Save = Core;
		Core.m_Input = In;
		vec2 Prev = Core.m_Pos;
		Core.Tick(true);
		Core.Move();
		Core.Quantize();
		if(!SegFreeze(Prev, Core.m_Pos))
		{
			pPos[t] = Core.m_Pos;
			continue;
		}
		if(!AllowKick || Reload > t)
			return t;
		// try virtual kicks applied before this tick's move
		vec2 aK[24];
		int nK = VirtualKicks(Save.m_Pos, aK);
		float BestV = 1e30f;
		int BestN = t;
		vec2 aBest[64], aTry[64];
		for(int k = 0; k < nK; k++)
		{
			CCharacterCore C2 = Save;
			C2.m_Vel += aK[k];
			int u = t;
			for(; u < Ticks; u++)
			{
				C2.m_Input = In;
				vec2 Pr = C2.m_Pos;
				C2.Tick(true);
				C2.Move();
				C2.Quantize();
				if(SegFreeze(Pr, C2.m_Pos))
					break;
				aTry[u] = C2.m_Pos;
			}
			if(u <= t)
				continue;
			float V = Value(G, aTry[u - 1], u > t + 1 ? aTry[u - 1] - aTry[u - 2] : C2.m_Vel) - u + (u < Ticks ? 1000.0f : 0.0f);
			if(V < BestV)
			{
				BestV = V;
				BestN = u;
				for(int w = t; w < u; w++)
					aBest[w] = aTry[w];
			}
		}
		for(int w = t; w < BestN; w++)
			pPos[w] = aBest[w];
		return BestN;
	}
	return Ticks;
}

static float PosValue(const CTasGame &G, vec2 P, vec2 V);

static float Score(const CTasGame &G, vec2 *pEnd = nullptr)
{
	const int t = Rt(G);
	vec2 aPos[64];
	const int Tau = std::min(gs_P.m_Tau, 64);
	float Best = 1e9f;
	const bool Hooking = G.m_LastHook;
	for(int k = 0; k < (Hooking ? 2 : 1); k++)
		for(int d = -1; d <= 1; d++)
		{
			int n = gs_P.m_VKick ? RolloutK(G, Tau, d, k == 0, true, aPos, PosValue) : G.Rollout(Tau, d, k == 0, aPos);
			vec2 P = n > 0 ? aPos[n - 1] : G.Pos();
			vec2 Vend = n > 1 ? aPos[n - 1] - aPos[n - 2] : G.Vel();
			float S = t + n + PosValue(G, P, Vend) + (n < Tau ? gs_P.m_DeathPen * (1.0f - 0.5f * n / Tau) : 0.0f);
			if(gs_P.m_HNow > 0 && n == Tau && Tau >= 4)
			{
				// speed term: the next hnow px at the D-rate of the rollout's last 3 ticks instead of v0
				float VD = (gs_DF.Sample(aPos[Tau - 4]) - gs_DF.Sample(aPos[Tau - 1])) / 3.0f;
				S += gs_P.m_HNow * (1.0f / std::clamp(VD, 5.0f, gs_P.m_VMax) - 1.0f / gs_P.m_V0);
			}
			if(S < Best)
			{
				Best = S;
				if(pEnd)
					*pEnd = P;
			}
		}
	if(gs_P.m_JumpCred)
		Best -= gs_P.m_JumpCred * JumpsLeft(G);
	if(gs_P.m_RlCred > 0 && G.ReloadTimer() == 0)
		Best -= gs_P.m_RlCred;
	Best += gs_P.m_RlW * G.ReloadTimer();
	if(gs_P.m_GhostE > 0)
	{
		// capped kinetic-energy credit (|v| above vcap is not worth more)
		float V2 = std::min(dot(G.Vel(), G.Vel()), gs_P.m_ECap * gs_P.m_ECap);
		if(gs_P.m_EHgt)
			V2 -= G.Pos().y - 4000.0f;
		Best -= gs_P.m_GhostE * V2;
	}
	return Best;
}

static int64_t FineKey(const CTasGame &G)
{
	vec2 P = G.Pos(), V = G.Vel();
	int64_t k = (int64_t)std::floor(P.x / gs_P.m_KeyPos) + 64;
	k = k * 2048 + ((int64_t)std::floor(P.y / gs_P.m_KeyPos) + 64);
	k = k * 256 + ((int64_t)std::floor(V.x / gs_P.m_KeyVel) + 128);
	k = k * 256 + ((int64_t)std::floor(V.y / gs_P.m_KeyVel) + 128);
	int hs = G.HookState();
	int h = hs == HOOK_IDLE || hs == HOOK_RETRACTED ? 0 : (hs == HOOK_GRABBED ? 2 : 1);
	k = k * 3 + h;
	k = k * 3 + JumpsLeft(G);
	k = k * 2 + (G.m_LastJump ? 1 : 0);
	k = k * 8 + std::min(G.ReloadTimer(), 28) / 4;
	if(G.NumProjectiles() > 0)
	{
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
static int64_t PosCell(vec2 P, vec2 V, int Extra)
{
	int64_t k = ((int64_t)std::floor(P.x / gs_P.m_Cell) + 1000) * 100000 + ((int64_t)std::floor(P.y / gs_P.m_Cell) + 1000);
	if(gs_P.m_QVel > 0)
		k = (k * 1000 + ((int64_t)std::floor(V.x / gs_P.m_QVel) + 500)) * 1000 + ((int64_t)std::floor(V.y / gs_P.m_QVel) + 500);
	return k * 16 + Extra;
}

static void HookTargets(const CTasGame &G, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	vec2 P = G.Pos();
	const float Range = 380.0f + 2.0f * length(G.Vel());
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
				if(gs_P.m_HookDedup > 0)
					Hit = ((int)std::floor(Q.y / gs_P.m_HookDedup)) * 100000 + (int)std::floor(Q.x / gs_P.m_HookDedup);
				else
					Hit = ty * M.m_W + tx;
				break;
			}
			if(T == TILE_NOHOOK)
				break;
		}
		if(Hit < 0 || std::find(vSeen.begin(), vSeen.end(), Hit) != vSeen.end())
			continue;
		vSeen.push_back(Hit);
		int16_t TX = (int16_t)std::lround(D.x * 1000), TY = (int16_t)std::lround(D.y * 1000);
		if(!TX && !TY)
			TY = -1;
		vOut.push_back({TX, TY});
	}
}

static vec2 PreHookVel(const CTasGame &G, int Dir, int Jump)
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
// rotation pulse aims: first-tick grab (solid 47..122 px away), boundary of |v| growth, turn towards -grad D
static void RotHookAims(const CTasGame &G, int Dir, int Jump, std::vector<std::pair<int16_t, int16_t>> &vOut)
{
	vOut.clear();
	const SMapInfo &M = CTasGame::Map();
	const vec2 P = G.Pos(), V = PreHookVel(G, Dir, Jump);
	const vec2 Tg = -gs_DF.Grad(P);
	if(length(Tg) < 0.5f)
		return;
	const float L0 = length(V);
	auto Reach = [&](int i) {
		float Ang = i * pi / 360.0f;
		vec2 D(std::cos(Ang), std::sin(Ang));
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
	auto NewV = [&](int i) { float Ang = i * pi / 360.0f; return V + HookPull(vec2(std::cos(Ang), std::sin(Ang)), Dir); };
	auto Ok = [&](int i) { float Ln = length(NewV(i)); return Ln < 15.0f - 0.01f || Ln < L0 - 0.004f; };
	float Best = dot(V, Tg) + 0.02f;
	int BestI = -100000;
	for(int i = -360; i < 360; i++)
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
	float Ang = BestI * pi / 360.0f;
	int16_t TX = (int16_t)std::lround(std::cos(Ang) * 1000), TY = (int16_t)std::lround(std::sin(Ang) * 1000);
	if(TX || TY)
		vOut.push_back({TX, TY});
}

// analytic shot preview (CProjectile path, grenade speed 20 px/t, curvature 0.28 n^2): explosion tick offset n
// (1 = in the fire step) and point; the predicted kick on a ballistic tee (kick uses the tee position before
// the n-th move). Returns the kick strength (0 if none within maxn).
static float ShotPreview(const CTasGame &G, vec2 Dir, int MaxN, int &N, vec2 &EP)
{
	const vec2 P0 = G.Pos();
	const vec2 S = P0 + Dir * 21.0f;
	vec2 Prev = S;
	for(int T = 1; T <= MaxN; T++)
	{
		float d = 20.0f * T;
		vec2 Cur(S.x + Dir.x * d, S.y + Dir.y * d + 0.0007f * d * d);
		vec2 Col, Before;
		if(CTasGame::Collision()->IntersectLine(Prev, Cur, &Col, &Before))
		{
			N = T;
			EP = Col;
			// tee position before the T-th move (ballistic, ramped x)
			vec2 P = P0, V = G.Vel();
			for(int k = 1; k < T; k++)
			{
				V.y += 0.5f;
				float L = length(V);
				float Rm = L > 11 ? std::pow(1.4f, -(L * 50 - 550) / 2000.0f) : 1.0f;
				P.x += V.x * Rm;
				P.y += V.y;
			}
			float l = distance(P, EP);
			float f = 1.0f - std::clamp((l - 48.0f) / 87.0f, 0.0f, 1.0f);
			return (int)(6.0f * f) ? 12.0f * f : 0.0f;
		}
		Prev = Cur;
	}
	N = -1;
	return 0;
}

struct SCand
{
	float m_S;
	int m_Parent;
	STasInput m_In;
	int64_t m_Key, m_Cell;
	uint64_t m_Hash;
};

static vec2 GateNormal()
{
	const std::string &g = gs_P.m_Gate;
	if(g == "finish" || g.rfind("x>", 0) == 0)
		return vec2(1, 0);
	if(g == "under" || g == "room" || g.rfind("x<", 0) == 0)
		return vec2(-1, 0);
	if(g == "gap" || g.rfind("y>", 0) == 0)
		return vec2(0, 1);
	if(g == "col")
		return vec2(0, -1);
	return vec2(0, -1);
}
static float GateLine()
{
	const std::string &g = gs_P.m_Gate;
	if(g == "finish")
		return 8822.67f;
	if(g == "under")
		return -9050;
	if(g == "room")
		return -9000;
	if(g == "gap")
		return 4352;
	if(g == "col")
		return -3900;
	float v = std::atof(g.c_str() + 2);
	return g[1] == '<' ? -v : v;
}
static bool AtGate(const CTasGame &G)
{
	if(gs_P.m_Gate == "finish")
		return G.m_FinishTick >= 0;
	float v = std::atof(gs_P.m_Gate.c_str() + 2);
	vec2 P = G.Pos();
	if(gs_P.m_Gate == "col")
		return P.y < 3900 && P.x > 8470 && P.x < 8580;
	if(gs_P.m_Gate == "under")
		return P.x < 9050 && P.x > 8700 && P.y > 3870 && P.y < 4070;
	if(gs_P.m_Gate == "gap")
		return P.y > 4352 && P.x > 9200;
	if(gs_P.m_Gate == "room")
		return P.x < 9000 && P.y > 4410;
	if(gs_P.m_Gate.rfind("y<", 0) == 0)
		return P.y < v;
	if(gs_P.m_Gate.rfind("y>", 0) == 0)
		return P.y > v;
	if(gs_P.m_Gate.rfind("x<", 0) == 0)
		return P.x < v;
	if(gs_P.m_Gate.rfind("x>", 0) == 0)
		return P.x > v;
	return false;
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: fbeam <map> key=value...\n");
		return 1;
	}
	SEntry E;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		if(Eq == std::string::npos)
			continue;
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(EntryArg(E, K, V))
			continue;
		if(K == "beam") gs_P.m_Beam = std::stoi(V);
		else if(K == "q") gs_P.m_Q = std::stoi(V);
		else if(K == "cell") gs_P.m_Cell = std::stof(V);
		else if(K == "qvel") gs_P.m_QVel = std::stof(V);
		else if(K == "qextra") gs_P.m_QExtra = std::stoi(V);
		else if(K == "minkick") gs_P.m_MinKick = std::stof(V);
		else if(K == "keypos") gs_P.m_KeyPos = std::stof(V);
		else if(K == "keyvel") gs_P.m_KeyVel = std::stof(V);
		else if(K == "angles") gs_P.m_Angles = std::stoi(V);
		else if(K == "rothook") gs_P.m_RotHook = std::stoi(V);
		else if(K == "fire") gs_P.m_Fire = std::stoi(V);
		else if(K == "fireangles") gs_P.m_FireAngles = std::stoi(V);
		else if(K == "firelook") gs_P.m_FireLook = std::stoi(V);
		else if(K == "firetop") gs_P.m_FireTop = std::stoi(V);
		else if(K == "prefire") gs_P.m_Prefire = std::stoi(V);
		else if(K == "tau") gs_P.m_Tau = std::stoi(V);
		else if(K == "deathpen") gs_P.m_DeathPen = std::stof(V);
		else if(K == "margin") gs_P.m_Margin = std::stof(V);
		else if(K == "model") gs_P.m_Model = std::stoi(V);
		else if(K == "v0") gs_P.m_V0 = std::stof(V);
		else if(K == "hnow") gs_P.m_HNow = std::stof(V);
		else if(K == "latpen") gs_P.m_LatPen = std::stof(V);
		else if(K == "rlw") gs_P.m_RlW = std::stof(V);
		else if(K == "vkick") gs_P.m_VKick = std::stoi(V);
		else if(K == "track") gs_P.m_TrackW = std::stof(V);
		else if(K == "trackcap") gs_P.m_TrackCap = std::stof(V);
		else if(K == "tracktie") gs_P.m_TrackTie = std::stof(V);
		else if(K == "trackoff") gs_P.m_TrackOff = std::stoi(V);
		else if(K == "hookdedup") gs_P.m_HookDedup = std::stof(V);
		else if(K == "gatevw") gs_P.m_GateVW = std::stof(V);
		else if(K == "gatewait") gs_P.m_GateWait = std::stoi(V);
		else if(K == "pre") gs_P.m_Pre = V;
		else if(K == "fill") gs_P.m_Fill = std::stoi(V);
		else if(K == "ghoste") gs_P.m_GhostE = std::stof(V);
		else if(K == "ecap") gs_P.m_ECap = std::stof(V);
		else if(K == "ehgt") gs_P.m_EHgt = std::stoi(V);
		else if(K == "hybw") gs_P.m_HybW = std::stof(V);
		else if(K == "latdz") gs_P.m_LatDz = std::stof(V);
		else if(K == "vmax") gs_P.m_VMax = std::stof(V);
		else if(K == "maxticks") gs_P.m_MaxTicks = std::stoi(V);
		else if(K == "out") gs_P.m_Out = V;
		else if(K == "gate") gs_P.m_Gate = V;
		else if(K == "verbose") gs_P.m_Verbose = std::stoi(V);
		else if(K == "jumpcred") gs_P.m_JumpCred = std::stoi(V);
		else if(K == "rlcred") gs_P.m_RlCred = std::stof(V);
		else
		{
			std::printf("unknown key %s\n", K.c_str());
			return 1;
		}
	}
	auto t0 = std::chrono::steady_clock::now();
	BuildDF();
	BuildTimeModel();
	LoadRef();
	LoadTrk();
	if(getenv("TMDBG"))
	{
		for(float D = 0; D <= 4000; D += 250)
			std::printf("T(%.0f) = %.1f\n", D, TModel(D));
		std::printf("table %zu entries, first %.0f %.1f last %.0f %.1f\n", gs_vTD.size(), gs_vTD.front().first, gs_vTD.front().second, gs_vTD.back().first, gs_vTD.back().second);
	}
	std::vector<std::unique_ptr<CTasGame>> vBeam;
	std::vector<STasInput> vPrev;
	{
		auto G = std::make_unique<CTasGame>();
		MakeEntry(*G, E);
		{
			float Fr;
			G->m_RefIdx = 0;
			G->m_RefIdx = RefTrack(0, G->Pos(), G->Vel(), Fr);
			for(int it = 0; it < 10; it++)
				G->m_RefIdx = RefTrack(G->m_RefIdx, G->Pos(), G->Vel(), Fr);
		}
		std::vector<STasInput> vPre0 = gs_P.m_Pre.empty() ? std::vector<STasInput>() : ReadInputs(gs_P.m_Pre.c_str());
		for(const auto &I : vPre0)
		{
			G->Step(I);
			Upd(*G);
		}
		gs_vPreIn = vPre0;
		std::printf("entry k %d pos %.0f %.0f v %.2f %.2f reload %d air %d D %.0f T %.1f score %.1f\n", Rt(*G), G->Pos().x, G->Pos().y, G->Vel().x, G->Vel().y, G->ReloadTimer(), E.m_Air,
			gs_DF.Sample(G->Pos()), TModel(gs_DF.Sample(G->Pos())), Score(*G));
		STasInput P0 = gs_vPreIn.empty() ? MkIn(1, 0, 0, 0, 0) : gs_vPreIn.back();
		vPrev.push_back(P0);
		vBeam.push_back(std::move(G));
	}
	std::vector<std::vector<std::pair<int, STasInput>>> vHist;
	int BestStep = -1, BestParent = -1, BestRt = 1 << 30, FirstGate = -1;
	float BestV = 1e30f;
	vec2 BestVel(0, 0), BestPos(0, 0);
	STasInput BestIn;
	CTasGame Tmp, Look, Base, Look2;
	std::vector<std::pair<int16_t, int16_t>> vHooks, vRot, vFire;
	std::vector<SCand> vC;
	for(int Step = 0; Step < gs_P.m_MaxTicks && !vBeam.empty(); Step++)
	{
		vC.clear();
		for(int i = 0; i < (int)vBeam.size(); i++)
		{
			const CTasGame &G = *vBeam[i];
			const STasInput &Prev = vPrev[i];
			const bool Hooking = G.m_LastHook;
			const bool CanJump = !G.m_LastJump && (G.Grounded() || !(G.Jumped() & 2));
			const bool CanFire = gs_P.m_Fire && G.HasGrenade() && G.ReloadTimer() == 0 && i < gs_P.m_FireTop;
			if(!Hooking)
				HookTargets(G, vHooks);
			if(CanFire)
			{
				vFire.clear();
				for(int a = 0; a < gs_P.m_FireAngles; a++)
				{
					float Ang = 2 * pi * a / gs_P.m_FireAngles;
					int16_t TX = (int16_t)std::lround(std::cos(Ang) * 1000), TY = (int16_t)std::lround(std::sin(Ang) * 1000);
					int N;
					vec2 EP;
					float Kick = ShotPreview(G, normalize(vec2(TX, TY)), std::max(gs_P.m_FireLook, gs_P.m_Prefire) + 1, N, EP);
					if(N < 0)
						continue;
					if(N <= 1 && Kick < gs_P.m_MinKick - 1.5f)
						continue; // later explosions: the look decides (the tee may be hooked / turning)
					vFire.push_back({TX, TY});
				}
			}
			auto Emit = [&](const STasInput &In, const CTasGame &Child, const CTasGame &Eval) {
				if(AtGate(Child))
				{
					int R = gs_P.m_Gate == "finish" ? Child.m_FinishTick - Child.m_StartTick : Rt(Child);
					vec2 Nrm = GateNormal();
					float Disp = std::fabs(dot(Child.Pos() - G.Pos(), Nrm));
					float Past = dot(Child.Pos(), Nrm) - GateLine();
					float Frac = gs_P.m_Gate == "finish" ? 0.0f : std::clamp(Disp > 1e-3f ? Past / Disp : 0.0f, 0.0f, 1.0f);
					float V = R - Frac - gs_P.m_GateVW * dot(Child.Vel(), Nrm);
					if(V < BestV)
					{
						BestV = V;
						BestRt = R;
						BestStep = Step;
						BestParent = i;
						BestIn = In;
						BestVel = Child.Vel();
						BestPos = Child.Pos();
					}
					return;
				}
				// a grenade in flight that explodes soon: judge the state after its explosion (holding this input)
				const CTasGame *pE = &Eval;
				if(Eval.NumProjectiles() > 0)
				{
					vec2 EP;
					int ET;
					if(Eval.NextExplosion(EP, ET) && ET - Eval.m_Tick <= gs_P.m_FireLook)
					{
						Look2.CopyFrom(Eval);
						STasInput L = In;
						L.m_Fire = 0;
						for(int k = 0; k < 40 && Look2.NumProjectiles() > 0; k++)
						{
							Look2.Step(L);
							Upd(Look2);
							if(Dead(Look2))
								return;
						}
						pE = &Look2;
					}
				}
				SCand C;
				C.m_S = Score(*pE);
				if(gs_P.m_TrackW > 0)
					C.m_S = Child.m_TrackCost + gs_P.m_TrackTie * C.m_S;
				C.m_Parent = i;
				C.m_In = In;
				C.m_Key = FineKey(Child);
				C.m_Cell = PosCell(Child.Pos(), Child.Vel(), gs_P.m_QExtra ? (JumpsLeft(Child) * 4 + (Child.ReloadTimer() == 0 ? 1 : 0) + (Child.NumProjectiles() > 0 ? 2 : 0)) : 0);
				C.m_Hash = Child.Hash();
				vC.push_back(C);
			};
			for(int Dir = -1; Dir <= 1; Dir++)
				for(int Jump = 0; Jump <= (CanJump ? 1 : 0); Jump++)
				{
					STasInput In;
					In.m_Dir = Dir;
					In.m_Jump = Jump;
					In.m_Weapon = 3;
					In.m_TX = Prev.m_TX;
					In.m_TY = Prev.m_TY;
					In.m_Hook = Hooking;
					// base action
					Base.CopyFrom(G);
					Base.Step(In); Upd(Base); TrkUpd(Base);
					bool BaseOk = !Dead(Base);
					if(BaseOk)
						Emit(In, Base, Base);
					std::vector<STasInput> vAlt;
					if(Hooking)
					{
						STasInput R = In;
						R.m_Hook = 0;
						vAlt.push_back(R);
					}
					else
					{
						for(auto [TX, TY] : vHooks)
						{
							STasInput H = In;
							H.m_Hook = 1;
							H.m_TX = TX;
							H.m_TY = TY;
							vAlt.push_back(H);
						}
						if(gs_P.m_RotHook)
						{
							RotHookAims(G, Dir, Jump, vRot);
							for(auto [TX, TY] : vRot)
							{
								STasInput H = In;
								H.m_Hook = 1;
								H.m_TX = TX;
								H.m_TY = TY;
								vAlt.push_back(H);
							}
						}
					}
					for(const auto &A : vAlt)
					{
						Tmp.CopyFrom(G);
						Tmp.Step(A); Upd(Tmp); TrkUpd(Tmp);
						if(!Dead(Tmp))
							Emit(A, Tmp, Tmp);
					}
					if(CanFire && !Jump)
					{
						for(auto [TX, TY] : vFire)
						{
							STasInput F = In;
							F.m_Fire = 1;
							F.m_TX = TX;
							F.m_TY = TY;
							Tmp.CopyFrom(G);
							Tmp.Step(F); Upd(Tmp); TrkUpd(Tmp);
							if(Dead(Tmp))
								continue;
							if(Tmp.NumProjectiles() == 0)
							{
								// exploded in the fire step: keep only real kicks
								if(!BaseOk || length(Tmp.Vel() - Base.Vel()) > gs_P.m_MinKick * 0.8f)
									Emit(F, Tmp, Tmp);
								continue;
							}
							vec2 EP;
							int ET;
							if(!Tmp.NextExplosion(EP, ET))
								continue;
							int n = ET - Tmp.m_Tick;
							if(n > gs_P.m_FireLook)
							{
								if(n <= gs_P.m_Prefire)
									Emit(F, Tmp, Tmp); // judged as if not fired; the key separates it
								continue;
							}
							// look: hold the same input (no fire) until the explosion
							Look.CopyFrom(Tmp);
							STasInput L = F;
							L.m_Fire = 0;
							bool Dd = false;
							vec2 PBefore = Look.Pos();
							for(int k = 0; k < n && Look.NumProjectiles() > 0; k++)
							{
								PBefore = Look.Pos();
								Look.Step(L); Upd(Look);
								if(Dead(Look))
								{
									Dd = true;
									break;
								}
							}
							if(Dd || distance(PBefore, EP) > 48.0f + 87.0f * (1.0f - gs_P.m_MinKick / 12.0f))
								continue;
							if(AtGate(Look))
								continue;
							Emit(F, Tmp, Look);
						}
					}
				}
		}
		if(BestStep >= 0 && FirstGate < 0)
			FirstGate = Step;
		if(FirstGate >= 0 && Step >= FirstGate + gs_P.m_GateWait)
			break;
		// selection
		std::sort(vC.begin(), vC.end(), [](const SCand &a, const SCand &b) { return a.m_S != b.m_S ? a.m_S < b.m_S : a.m_Hash < b.m_Hash; });
		std::unordered_set<uint64_t> Seen;
		std::unordered_set<int64_t> Keys;
		std::unordered_map<int64_t, int> CellCnt;
		std::vector<SCand> vSel;
		const float Top = vC.empty() ? 0 : vC[0].m_S;
		std::vector<char> vTaken(vC.size(), 0);
		for(size_t ci = 0; ci < vC.size(); ci++)
		{
			const auto &C = vC[ci];
			if((int)vSel.size() >= gs_P.m_Beam || C.m_S > Top + gs_P.m_Margin)
				break;
			if(Seen.count(C.m_Hash) || Keys.count(C.m_Key))
				continue;
			int &Cnt = CellCnt[C.m_Cell];
			if(Cnt >= gs_P.m_Q)
				continue;
			Seen.insert(C.m_Hash);
			Keys.insert(C.m_Key);
			Cnt++;
			vSel.push_back(C);
			vTaken[ci] = 1;
		}
		// second pass: fill the beam with the best of the rest (quota off, dedup on)
		for(size_t ci = 0; ci < vC.size() && gs_P.m_Fill; ci++)
		{
			const auto &C = vC[ci];
			if((int)vSel.size() >= gs_P.m_Beam || C.m_S > Top + gs_P.m_Margin)
				break;
			if(vTaken[ci] || Seen.count(C.m_Hash) || Keys.count(C.m_Key))
				continue;
			Seen.insert(C.m_Hash);
			Keys.insert(C.m_Key);
			vSel.push_back(C);
		}
		std::vector<std::unique_ptr<CTasGame>> vNew(vSel.size());
		std::vector<STasInput> vNewPrev(vSel.size());
		std::vector<std::pair<int, STasInput>> H(vSel.size());
		for(size_t k = 0; k < vSel.size(); k++)
		{
			vNew[k] = std::make_unique<CTasGame>();
			vNew[k]->CopyFrom(*vBeam[vSel[k].m_Parent]);
			vNew[k]->Step(vSel[k].m_In); Upd(*vNew[k]); TrkUpd(*vNew[k]);
			vNewPrev[k] = vSel[k].m_In;
			H[k] = {vSel[k].m_Parent, vSel[k].m_In};
		}
		vHist.push_back(std::move(H));
		vBeam = std::move(vNew);
		vPrev = std::move(vNewPrev);
		if(gs_P.m_Verbose && !vSel.empty() && (Step % 5 == 0 || gs_P.m_Verbose > 1))
		{
			const CTasGame &B = *vBeam[0];
			double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
			std::printf("step %d k %d beam %zu cands %zu cells %zu best S %.1f pos %.0f %.0f v %.1f %.1f D %.0f (%.0fs)\n", Step, Rt(B), vBeam.size(), vC.size(), CellCnt.size(), vSel[0].m_S,
				B.Pos().x, B.Pos().y, B.Vel().x, B.Vel().y, gs_DF.Sample(B.Pos()), Sec);
			std::fflush(stdout);
		}
	}
	if(BestStep < 0)
	{
		std::printf("NOGATE (writing the best last state's path)\n");
		while(!vHist.empty() && vHist.back().empty())
			vHist.pop_back();
		if(vHist.empty())
			return 0;
		BestStep = (int)vHist.size() - 1;
		BestIn = vHist[BestStep][0].second;
		BestParent = vHist[BestStep][0].first;
		BestRt = -1;
	}
	std::vector<STasInput> vRun;
	vRun.push_back(BestIn);
	int Idx = BestParent;
	for(int s = BestStep - 1; s >= 0; s--)
	{
		vRun.push_back(vHist[s][Idx].second);
		Idx = vHist[s][Idx].first;
	}
	std::reverse(vRun.begin(), vRun.end());
	vRun.insert(vRun.begin(), gs_vPreIn.begin(), gs_vPreIn.end());
	std::string Out = gs_P.m_Out + ".txt";
	WriteInputs(Out.c_str(), vRun);
	double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
	std::printf("GATE k %d (%d ticks from the entry k %d; Teero finish 2539) value %.2f pos %.0f %.0f vel %.2f %.2f -> %s (%.0fs)\n", BestRt, BestRt - E.m_K, E.m_K, BestV, BestPos.x, BestPos.y, BestVel.x, BestVel.y, Out.c_str(), Sec);
	// trace
	std::string TrPath = gs_P.m_Out + ".trace";
	FILE *f = std::fopen(TrPath.c_str(), "w");
	CTasGame G;
	MakeEntry(G, E);
	std::fprintf(f, "# entry tp=%.2f,%.2f,%.3f,%.3f tpk=%d reload=%d air=%d prefix=%s\n", E.m_P.x, E.m_P.y, E.m_V.x, E.m_V.y, E.m_K, E.m_Reload, E.m_Air, E.m_Prefix.c_str());
	PrintState(G, nullptr, f);
	for(const auto &In : vRun)
	{
		G.Step(In);
		std::fprintf(f, "D %.0f ", gs_DF.Sample(G.Pos()));
		PrintState(G, &In, f);
	}
	std::fclose(f);
	return 0;
}
