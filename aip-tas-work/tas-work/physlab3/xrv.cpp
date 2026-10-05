// xrv: rendezvous beam for the exit double kick (physlab2 experiment tool).
// The prefix must end right after the pre-fire step (one grenade in flight). The tool reads the pending
// explosion (tick TE, point E) and beam-searches hook/dir inputs so that the tee is close to E on the tick
// before TE; then it fires the point-blank shot (several aims) on the step before TE or on TE itself and
// scores by the estimated race tick at x = XF after a 14-tick dir-1 tail.
// usage: xrv <map> prefix=FILE [beam=3000] [naims=24] [xf=5900] [out=FILE] [cell=6]
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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

static float g_VNom = 13.5f, g_VxW = 0.0f, g_DY = 4.0f, g_DX = 40.0f;
static int g_VxTicks = 4;
struct SNode
{
	std::unique_ptr<CTasGame> m_pG;
	int m_Parent;
	STasInput m_In;
	float m_Value;
};
struct SHist
{
	int m_Parent;
	STasInput m_In;
};

static STasInput Inp(int Dir, int Hook, int Fire, float Deg)
{
	STasInput In;
	In.m_Dir = Dir;
	In.m_Jump = 0;
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
	std::string Prefix, Out = "xrv_out.txt";
	int Beam = 3000, NAims = 24;
	float XF = 5900, Cell = 6;
	for(int i = 2; i < argc; i++)
	{
		std::string A = argv[i];
		size_t Eq = A.find('=');
		std::string K = A.substr(0, Eq), V = A.substr(Eq + 1);
		if(K == "prefix") Prefix = V;
		else if(K == "beam") Beam = std::stoi(V);
		else if(K == "naims") NAims = std::stoi(V);
		else if(K == "xf") XF = std::stof(V);
		else if(K == "out") Out = V;
		else if(K == "cell") Cell = std::stof(V);
		else if(K == "vnom") g_VNom = std::stof(V);
		else if(K == "vxw") g_VxW = std::stof(V);
		else if(K == "vxticks") g_VxTicks = std::stoi(V);
		else if(K == "dy") g_DY = std::stof(V);
		else if(K == "dx") g_DX = std::stof(V);
	}
	std::vector<STasInput> vPre;
	{
		FILE *f = std::fopen(Prefix.c_str(), "r");
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
			vPre.push_back(In);
		}
		if(f)
			std::fclose(f);
	}
	auto pStart = std::make_unique<CTasGame>();
	pStart->Spawn(CTasGame::Map().m_vSpawns[0]);
	for(auto &In : vPre)
		pStart->Step(In);
	vec2 E;
	int TE;
	if(!pStart->NextExplosion(E, TE))
	{
		std::printf("no grenade in flight\n");
		return 1;
	}
	const int Start = pStart->m_Tick, SRt = Start - pStart->m_StartTick;
	const int Layers = TE - 1 - Start; // last layer = state before the explosion step
	std::printf("start rt %d pos %.0f %.0f; explosion at rt %d (%.1f, %.1f); layers %d\n", SRt, pStart->Pos().x, pStart->Pos().y,
		TE - pStart->m_StartTick, E.x, E.y, Layers);
	// target: just right of the explosion point, a little below it (kick slightly up)
	const vec2 Target(E.x + g_DX, E.y + g_DY);
	std::vector<std::vector<SHist>> vHist;
	std::vector<SNode> vCur;
	vCur.push_back(SNode{std::move(pStart), -1, vPre.back(), 0});
	vHist.push_back({{-1, vPre.back()}});
	for(int L = 1; L <= Layers; L++)
	{
		std::vector<SNode> vNext;
		std::unordered_map<uint64_t, int> mCell;
		const int Rem = Layers - L;
		for(int ni = 0; ni < (int)vCur.size(); ni++)
		{
			const CTasGame &G = *vCur[ni].m_pG;
			const STasInput &Last = vCur[ni].m_In;
			std::vector<STasInput> vAct;
			const int Hs = G.HookState();
			for(int d = -1; d <= 1; d++)
			{
				if(Last.m_Hook && (Hs == HOOK_GRABBED || Hs == HOOK_FLYING))
				{
					STasInput I = Last;
					I.m_Dir = d;
					I.m_Fire = 0;
					I.m_Jump = 0;
					vAct.push_back(I);
				}
				vAct.push_back(Inp(d, 0, 0, 0));
				if(!Last.m_Hook)
					for(int a = 0; a < NAims; a++)
						vAct.push_back(Inp(d, 1, 0, 180.0f + 180.0f * (a + 0.5f) / NAims)); // upper half aims
			}
			for(const STasInput &I : vAct)
			{
				auto pC = std::make_unique<CTasGame>();
				pC->CopyFrom(G);
				pC->Step(I);
				if(pC->Frozen() || pC->EnteredFreeze())
					continue;
				vec2 P = pC->Pos(), V = pC->Vel();
				float D = distance(P, Target);
				// on-schedule value: distance still to cover vs what ~13.5 px/t covers in the remaining ticks
				float Value = -std::fabs(D - g_VNom * Rem) - 0.3f * std::max(0.0f, D - 15.0f * Rem) + (Rem <= g_VxTicks ? g_VxW * V.x : 0.0f);
				uint64_t Key = ((uint64_t)(int)(P.x / Cell) & 0xffff) | (((uint64_t)(int)(P.y / Cell) & 0xffff) << 16) |
					       (((uint64_t)(int)std::floor(V.x) & 0xff) << 32) | (((uint64_t)(int)std::floor(V.y) & 0xff) << 40) |
					       ((uint64_t)(pC->HookState() + 1) << 48) | ((uint64_t)I.m_Hook << 52);
				auto It = mCell.find(Key);
				if(It != mCell.end())
				{
					if(vNext[It->second].m_Value >= Value)
						continue;
					vNext[It->second] = SNode{std::move(pC), ni, I, Value};
					continue;
				}
				mCell[Key] = (int)vNext.size();
				vNext.push_back(SNode{std::move(pC), ni, I, Value});
			}
		}
		std::sort(vNext.begin(), vNext.end(), [](const SNode &a, const SNode &b) { return a.m_Value > b.m_Value; });
		if((int)vNext.size() > Beam)
			vNext.resize(Beam);
		std::vector<SHist> vH;
		for(auto &N : vNext)
			vH.push_back({N.m_Parent, N.m_In});
		vHist.push_back(vH);
		vCur = std::move(vNext);
		if(vCur.empty())
		{
			std::printf("beam died at layer %d\n", L);
			return 0;
		}
		const CTasGame &B = *vCur[0].m_pG;
		std::printf("L %d rt %d n %zu best %.1f pos %.0f %.0f vel %.2f %.2f\n", L, B.m_Tick - B.m_StartTick, vCur.size(), vCur[0].m_Value, B.Pos().x,
			B.Pos().y, B.Vel().x, B.Vel().y);
	}
	// final: point-blank shot options for each final state (fire on the step before TE or on TE), then tail
	float BestS = 1e9f;
	int BestI = -1;
	std::vector<STasInput> BestTail;
	CTasGame H;
	for(int i = 0; i < (int)vCur.size(); i++)
	{
		const CTasGame &G = *vCur[i].m_pG;
		vec2 P = G.Pos();
		if(P.x < 5281 || P.x > 5320 || std::fabs(P.y - E.y) > 40)
			continue;
		for(int When = 0; When < 2; When++) // 0: fire now (explodes with the pre-fire if within 41 px), 1: fire the step before (not possible here)
		{
			if(When == 1)
				continue;
			float Base = std::atan2(std::clamp(P.y - 4, 1922.0f, 1950.0f) - P.y, 5248.0f - P.x) * 180 / pi;
			for(int da = -12; da <= 12; da += 3)
				for(int Hk = 0; Hk < 2; Hk++)
				{
					H.CopyFrom(G);
					std::vector<STasInput> vT;
					STasInput I = vCur[i].m_In;
					if(!Hk || !(I.m_Hook && (G.HookState() == HOOK_GRABBED || G.HookState() == HOOK_FLYING)))
						I = Inp(1, 0, 1, Base + da);
					else
					{
						I.m_Dir = 1;
						I.m_Fire = 1;
						I.m_TX = (int16_t)std::lround(std::cos((Base + da) * pi / 180) * 1000);
						I.m_TY = (int16_t)std::lround(std::sin((Base + da) * pi / 180) * 1000);
					}
					vT.push_back(I);
					H.Step(I);
					bool Dead = H.Frozen() || H.EnteredFreeze();
					for(int k = 0; k < 15 && !Dead; k++)
					{
						STasInput J = Inp(1, 0, 0, 0);
						vT.push_back(J);
						H.Step(J);
						Dead = H.Frozen() || H.EnteredFreeze();
					}
					if(Dead || H.Vel().x < 5)
						continue;
					float S = (H.m_Tick - H.m_StartTick) + (XF - H.Pos().x) / H.Vel().x;
					if(S < BestS)
					{
						BestS = S;
						BestI = i;
						BestTail = vT;
					}
				}
		}
	}
	if(BestI < 0)
	{
		std::printf("no final state in the zone\n");
		return 0;
	}
	std::vector<STasInput> vPath;
	int Idx = BestI;
	for(int L = Layers; L >= 1; L--)
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
	for(auto &In : BestTail)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
	std::printf("BEST est rt at x=%.0f: %.2f -> %s\n", XF, BestS, Out.c_str());
	return 0;
}
