// xbeam: small single-threaded beam search for short post-grenade continuations (physlab2 experiment tool).
// Progress = Teero-track label equivalent k_eff(x) (Teero's x(k) is monotone on the chosen label range);
// value = k_eff - rt + ESHARE * (|v| - 25) + KCREDIT * kick_ready_soon; dedup per (x/8, y/8, vx, vy, hook, reload) cell.
// usage: xbeam <map> prefix=FILE [lines=N] track=teero_track.txt k0=K k1=K beam=B maxticks=T out=FILE
//        [ywin=140] [eshare=0.05] [kcredit=1.0] [survive=8] [naims=16] [nfire=32]
// The output file = prefix lines + the best continuation up to the gate (x >= x(k1) within ywin of y(k1)).
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/entities/projectile.h>
#include <game/client/prediction/gameworld.h>
#include "../../ddnet/src/tas/sim.h"
#undef private
#undef protected

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct SNode
{
	std::unique_ptr<CTasGame> m_pG;
	int m_Parent = -1; // index into previous layer's history
	STasInput m_In;
	float m_Value = -1e9f;
};

struct SHist
{
	int m_Parent;
	STasInput m_In;
};

static std::vector<std::pair<float, float>> g_Track; // label k -> (x, y), index k - g_K0
static int g_K0 = 0, g_K1 = 0;
static float g_YWin = 140, g_EShare = 0.05f, g_KCredit = 0.0f;

static float g_JCredit = 0.0f;
static float g_LatPen = 0.02f, g_LatDz = 48.0f, g_Cell = 8.0f, g_VCell = 1.0f;
static float g_Lat = 0; // lateral distance of the last KEff call
static float KEff(vec2 P)
{
	// projection onto Teero's polyline (labels g_K0..g_K1): label equivalent + lateral distance
	int n = (int)g_Track.size();
	float BestD = 1e18f, BestK = g_K0;
	for(int i = 0; i + 1 < n; i++)
	{
		vec2 A(g_Track[i].first, g_Track[i].second), B(g_Track[i + 1].first, g_Track[i + 1].second);
		vec2 AB = B - A;
		float L2 = dot(AB, AB);
		float t = L2 > 0 ? dot(P - A, AB) / L2 : 0;
		float tc = std::clamp(t, 0.0f, 1.0f);
		if(i == 0 && t < 0)
			tc = t;
		if(i + 2 == n && t > 1)
			tc = t;
		vec2 Q = A + AB * tc;
		float D = distance(P, Q);
		if(D < BestD)
		{
			BestD = D;
			BestK = g_K0 + i + tc;
		}
	}
	g_Lat = BestD;
	return BestK;
}

static float YRef(float K)
{
	int i = std::clamp((int)K - g_K0, 0, (int)g_Track.size() - 1);
	return g_Track[i].second;
}

static STasInput Inp(int Dir, int Jump, int Hook, int Fire, float Deg)
{
	STasInput In;
	In.m_Dir = Dir;
	In.m_Jump = Jump;
	In.m_Hook = Hook;
	In.m_Fire = Fire;
	In.m_TX = (int16_t)std::lround(std::cos(Deg * pi / 180) * 1000);
	In.m_TY = (int16_t)std::lround(std::sin(Deg * pi / 180) * 1000);
	In.m_Weapon = WEAPON_GRENADE;
	return In;
}

int main(int argc, const char **argv)
{
	if(argc < 2 || !CTasGame::LoadMap(argv[1]))
		return 1;
	std::string Prefix, Track = "../teero_track.txt", Out = "xbeam_out.txt";
	int Lines = 1 << 30, Beam = 2000, MaxTicks = 120, Survive = 8, NAims = 16, NFire = 32;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "prefix") Prefix = V;
		else if(K == "lines") Lines = std::stoi(V);
		else if(K == "track") Track = V;
		else if(K == "k0") g_K0 = std::stoi(V);
		else if(K == "k1") g_K1 = std::stoi(V);
		else if(K == "beam") Beam = std::stoi(V);
		else if(K == "maxticks") MaxTicks = std::stoi(V);
		else if(K == "out") Out = V;
		else if(K == "ywin") g_YWin = std::stof(V);
		else if(K == "eshare") g_EShare = std::stof(V);
		else if(K == "kcredit") g_KCredit = std::stof(V);
		else if(K == "survive") Survive = std::stoi(V);
		else if(K == "naims") NAims = std::stoi(V);
		else if(K == "latpen") g_LatPen = std::stof(V);
		else if(K == "jcredit") g_JCredit = std::stof(V);
		else if(K == "cell") g_Cell = std::stof(V);
		else if(K == "vcell") g_VCell = std::stof(V);
		else if(K == "latdz") g_LatDz = std::stof(V);
		else if(K == "nfire") NFire = std::stoi(V);
		else
		{
			std::printf("unknown key %s\n", K.c_str());
			return 1;
		}
	}
	{
		std::map<int, std::pair<float, float>> M;
		FILE *f = std::fopen(Track.c_str(), "r");
		int k;
		float x, y;
		while(f && std::fscanf(f, "%d %f %f", &k, &x, &y) == 3)
			M[k] = {x, y};
		if(f)
			std::fclose(f);
		// smoothed (5-tick moving average) Teero positions for labels k0..k1
		for(int kk = g_K0; kk <= g_K1; kk++)
		{
			float sx = 0, sy = 0;
			int c = 0;
			for(int j = kk - 2; j <= kk + 2; j++)
				if(M.count(j))
				{
					sx += M[j].first;
					sy += M[j].second;
					c++;
				}
			g_Track.push_back({sx / c, sy / c});
		}
	}
	const float XG = g_Track.back().first, YG = g_Track.back().second;
	// prefix
	std::vector<STasInput> vPre;
	{
		FILE *f = std::fopen(Prefix.c_str(), "r");
		int d, j, h, fi, tx, ty, w;
		while(f && (int)vPre.size() < Lines && std::fscanf(f, "%d %d %d %d %d %d %d", &d, &j, &h, &fi, &tx, &ty, &w) == 7)
		{
			STasInput In;
			In.m_Dir = d;
			In.m_Jump = j;
			In.m_Hook = h;
			In.m_Fire = fi;
			In.m_TX = tx;
			In.m_TY = ty;
			In.m_Weapon = w;
			vPre.push_back(In);
		}
		if(f)
			std::fclose(f);
	}
	auto pStart = std::make_unique<CTasGame>();
	pStart->Spawn(CTasGame::Map().m_vSpawns[0]);
	for(auto &In : vPre)
		pStart->Step(In);
	STasInput Last = vPre.back();
	std::printf("start rt %d pos %.0f %.0f vel %.2f %.2f keff %.2f gate x %.0f y %.0f\n", pStart->m_Tick - pStart->m_StartTick, pStart->Pos().x,
		pStart->Pos().y, pStart->Vel().x, pStart->Vel().y, KEff(pStart->Pos()), XG, YG);

	std::vector<std::vector<SHist>> vHist; // per layer
	std::vector<SNode> vCur(1);
	vCur[0].m_pG = std::move(pStart);
	vCur[0].m_In = Last;
	vCur[0].m_Parent = -1;
	vHist.push_back({{-1, Last}});
	std::vector<std::unique_ptr<CTasGame>> vPool;
	auto Get = [&]() {
		if(vPool.empty())
			return std::make_unique<CTasGame>();
		auto p = std::move(vPool.back());
		vPool.pop_back();
		return p;
	};
	int BestLayer = -1, BestIdx = -1;
	float BestT = 1e9f;
	for(int L = 1; L <= MaxTicks; L++)
	{
		std::vector<SNode> vNext;
		std::unordered_map<uint64_t, int> mCell;
		for(int ni = 0; ni < (int)vCur.size(); ni++)
		{
			SNode &N = vCur[ni];
			const CTasGame &G = *N.m_pG;
			std::vector<STasInput> vAct;
			const bool HookHeld = N.m_In.m_Hook;
			const int Hs = G.HookState();
			const bool CanJump = !(G.Jumped() & 2) || G.Grounded();
			const bool CanFire = G.ReloadTimer() == 0;
			float VDeg = std::atan2(G.Vel().y, G.Vel().x) * 180 / pi;
			for(int d = -1; d <= 1; d++)
			{
				// keep/hold hook state
				if(HookHeld && (Hs == HOOK_GRABBED || Hs == HOOK_FLYING))
					vAct.push_back([&] { STasInput I = N.m_In; I.m_Dir = d; I.m_Jump = 0; I.m_Fire = 0; I.m_Weapon = WEAPON_GRENADE; return I; }());
				vAct.push_back(Inp(d, 0, 0, 0, 0)); // release / no hook
				if(!HookHeld)
					for(int a = 0; a < NAims; a++)
						vAct.push_back(Inp(d, 0, 1, 0, 360.0f * a / NAims + 3.0f));
				if(CanJump)
				{
					STasInput I = HookHeld ? N.m_In : Inp(d, 0, 0, 0, 0);
					I.m_Dir = d;
					I.m_Jump = 1;
					I.m_Fire = 0;
					I.m_Weapon = WEAPON_GRENADE;
					vAct.push_back(I);
				}
			}
			if(CanFire)
			{
				int d = G.Vel().x >= 0 ? 1 : -1;
				for(int a = 0; a < NFire; a++)
				{
					float Deg = 360.0f * a / NFire + VDeg * 0; // absolute aims
					STasInput I = (HookHeld && (Hs == HOOK_GRABBED || Hs == HOOK_FLYING)) ? N.m_In : Inp(d, 0, 0, 0, 0);
					I.m_Dir = d;
					I.m_Jump = 0;
					I.m_Fire = 1;
					I.m_Weapon = WEAPON_GRENADE;
					I.m_TX = (int16_t)std::lround(std::cos(Deg * pi / 180) * 1000);
					I.m_TY = (int16_t)std::lround(std::sin(Deg * pi / 180) * 1000);
					// a shot on a hook-launch tick would re-aim the hook: only fire while holding/not hooking
					vAct.push_back(I);
				}
			}
			for(const STasInput &I : vAct)
			{
				auto pC = Get();
				pC->CopyFrom(G);
				// last-input bookkeeping for the hook (sim tracks m_LastHook itself)
				pC->Step(I);
				if(pC->Frozen() || pC->EnteredFreeze() || pC->m_StartTick == -2)
				{
					vPool.push_back(std::move(pC));
					continue;
				}
				vec2 P = pC->Pos(), V = pC->Vel();
				float KE = KEff(P);
				float Lat = g_Lat;
				// gate: projection past the last label, within ywin of the line
				if(KE >= g_K1 && Lat < g_YWin)
				{
					float KPrev = KEff(G.Pos());
					float Frac = std::clamp((KE - g_K1) / std::max(0.05f, KE - KPrev), 0.0f, 1.0f);
					float T = (pC->m_Tick - pC->m_StartTick) - Frac;
					if(T < BestT)
					{
						BestT = T;
						BestLayer = L;
						BestIdx = (int)vHist.size() > L ? -1 : -2; // patched below
						// store this child so the path can be rebuilt
						vNext.push_back(SNode{std::move(pC), ni, I, 1e9f});
						BestIdx = (int)vNext.size() - 1;
					}
					else
						vPool.push_back(std::move(pC));
					continue;
				}
				// stay near Teero's line
				if(Lat > g_YWin * 1.5f)
				{
					vPool.push_back(std::move(pC));
					continue;
				}
				if(Survive > 0)
				{
					vec2 aPos[64];
					bool Ok = false;
					for(int d = 1; d >= -1 && !Ok; d--)
						Ok = pC->Rollout(Survive, d, false, aPos) == Survive || pC->Rollout(Survive, d, true, aPos) == Survive;
					if(!Ok)
					{
						vPool.push_back(std::move(pC));
						continue;
					}
				}
				int Rt = pC->m_Tick - pC->m_StartTick;
				float Vl = length(V);
				int Rl = pC->ReloadTimer();
				float Value = KE - Rt + g_EShare * (Vl - 25) - g_LatPen * std::max(0.0f, Lat - g_LatDz) +
					      g_JCredit * ((pC->Jumped() & 2) ? 0.0f : 1.0f) + g_KCredit * std::max(0.0f, 1.0f - Rl / 10.0f);
				// cell dedup
				uint64_t Key = ((uint64_t)(int)(P.x / g_Cell) & 0xffff) | (((uint64_t)(int)(P.y / g_Cell) & 0xffff) << 16) |
					       (((uint64_t)(int)std::floor(V.x / g_VCell) & 0xff) << 32) | (((uint64_t)(int)std::floor(V.y / g_VCell) & 0xff) << 40) |
					       ((uint64_t)(pC->HookState() + 1) << 48) | ((uint64_t)std::min(Rl, 25) << 52) | ((uint64_t)pC->NumProjectiles() << 58) |
					       ((uint64_t)(pC->Jumped() & 3) << 61);
				auto It = mCell.find(Key);
				if(It != mCell.end())
				{
					if(vNext[It->second].m_Value >= Value)
					{
						vPool.push_back(std::move(pC));
						continue;
					}
					vPool.push_back(std::move(vNext[It->second].m_pG));
					vNext[It->second] = SNode{std::move(pC), ni, I, Value};
					continue;
				}
				mCell[Key] = (int)vNext.size();
				vNext.push_back(SNode{std::move(pC), ni, I, Value});
			}
		}
		// history for this layer (parents index into vCur order = previous layer's history indices)
		std::vector<SHist> vH;
		// keep best Beam by value (the gate node, if any, is kept separately)
		std::vector<int> vIdx;
		for(int i = 0; i < (int)vNext.size(); i++)
			if(vNext[i].m_pG)
				vIdx.push_back(i);
		int GateNode = (BestLayer == L) ? BestIdx : -1;
		std::sort(vIdx.begin(), vIdx.end(), [&](int a, int b) { return vNext[a].m_Value > vNext[b].m_Value; });
		std::vector<SNode> vKeep;
		for(int i : vIdx)
		{
			if((int)vKeep.size() >= Beam && i != GateNode)
			{
				vPool.push_back(std::move(vNext[i].m_pG));
				continue;
			}
			if(i == GateNode)
				BestIdx = (int)vKeep.size();
			vH.push_back({vNext[i].m_Parent, vNext[i].m_In});
			vKeep.push_back(std::move(vNext[i]));
		}
		// translate parent indices: vCur[ni] corresponds to vHist.back()[ni]
		vHist.push_back(vH);
		for(auto &N : vCur)
			if(N.m_pG)
				vPool.push_back(std::move(N.m_pG));
		vCur = std::move(vKeep);
		if(!vCur.empty())
		{
			int b = 0;
			for(int i = 0; i < (int)vCur.size(); i++)
				if(vCur[i].m_Value > vCur[b].m_Value && vCur[i].m_Value < 1e8f)
					b = i;
			const CTasGame &G = *vCur[b].m_pG;
			if(L % 5 == 0)
				std::printf("L %d rt %d n %zu best value %.2f pos %.0f %.0f vel %.1f %.1f reload %d keff %.2f gateT %.2f\n", L, G.m_Tick - G.m_StartTick, vCur.size(),
					vCur[b].m_Value, G.Pos().x, G.Pos().y, G.Vel().x, G.Vel().y, G.ReloadTimer(), KEff(G.Pos()), BestT);
		}
		std::fflush(stdout);
		// stop when the best gate time can't be beaten any more
		if(BestT < 1e8f && (vCur.empty() || L > BestLayer + 3))
			break;
		if(vCur.empty())
			break;
	}
	if(BestLayer < 0)
	{
		std::printf("NOGATE\n");
		return 0;
	}
	// rebuild
	std::vector<STasInput> vPath;
	int Idx = BestIdx;
	for(int L = BestLayer; L >= 1; L--)
	{
		vPath.push_back(vHist[L][Idx].m_In);
		Idx = vHist[L][Idx].m_Parent;
	}
	std::reverse(vPath.begin(), vPath.end());
	FILE *f = std::fopen(Out.c_str(), "w");
	for(auto &In : vPre)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	for(auto &In : vPath)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
	std::printf("GATE rt %.2f (layer %d) -> %s\n", BestT, BestLayer, Out.c_str());
	return 0;
}
