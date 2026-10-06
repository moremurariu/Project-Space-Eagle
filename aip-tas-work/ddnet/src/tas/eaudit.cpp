// eaudit: energy audit of a run (post-grenade, fast stepper): per tick, the change of kinetic energy v^2/2 split into
// explosions, hook, direction input (vs pressing along vx), jump, collisions and the rest (gravity, friction).
#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#undef private
#undef protected
#include "tasfast.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
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

// usage: eaudit MAP INPUTS FROM_RT [TO_RT] [SECTION=50] [EVENT_THRESHOLD=150]
static float En(vec2 V) { return 0.5f * dot(V, V); }
int main(int argc, const char **argv)
{
	if(argc < 4 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: eaudit MAP INPUTS FROM_RT [TO_RT] [SECTION=50] [EVENT=150]\n");
		return 1;
	}
	CFastG::Init();
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	const int From = std::atoi(argv[3]), To = argc > 4 ? std::atoi(argv[4]) : 100000, Sec = argc > 5 ? std::atoi(argv[5]) : 50;
	const float Ev = argc > 6 ? std::atof(argv[6]) : 150.0f;
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	const int Cut = From + 68;
	for(int i = 0; i < Cut && i < (int)vIn.size(); i++)
		G.Step(vIn[i]);
	static CTasFast F;
	F.FromGameFull(G);
	SStepAudit A;
	CFastG::ms_pAudit = &A;
	std::vector<SExplLog> vLog;
	if(getenv("EA_SURF"))
		CFastG::ms_pLog = &vLog;
	size_t NLog = 0;
	enum { EXPL, HOOK, DIR, JUMP, COLL, REST, NUM };
	const char *apN[NUM] = {"expl", "hook", "dir", "jump", "coll", "grav/rest"};
	double aSec[NUM] = {0}, aTot[NUM] = {0};
	int SecStart = From;
	std::printf("rt: energy terms per tick are d(v^2/2); sections of %d ticks\n", Sec);
	for(int i = Cut; i < (int)vIn.size(); i++)
	{
		const int Rt = i - 67;
		if(Rt > To || F.m_FinishTick >= 0)
			break;
		const int NP0 = F.NumProjectiles();
		F.Step(vIn[i]);
		if(getenv("EA_SHOTS") && F.NumProjectiles() > NP0)
		{
			// a new grenade this tick: where and when it explodes (all grenades in flight; the earliest is reported)
			vec2 E;
			int Tk;
			if(F.NextExplosion(E, Tk))
				std::printf("S %d te %d expl %.1f %.1f tee %.1f %.1f aim %d %d\n", Rt, Rt + (Tk - F.m_Tick), E.x, E.y, F.Pos().x, F.Pos().y, vIn[i].m_TX, vIn[i].m_TY);
		}
		for(; NLog < vLog.size(); NLog++)
		{
			// explosion surface: walk from the explosion point toward the tee; a freeze tile on the way = freeze-lined
			const SExplLog &L = vLog[NLog];
			int Frz = 0;
			const vec2 Dd = L.m_Tee - L.m_E;
			for(float f = 0; f <= 1.0f; f += 0.02f)
			{
				const vec2 P = L.m_E + Dd * f;
				const int T = CTasGame::Map().Tile((int)std::floor(P.x / 32), (int)std::floor(P.y / 32));
				if(T == TILE_FREEZE)
					Frz = 1;
			}
			const float Fl = length(L.m_Force), V0 = length(L.m_VelBefore);
			const float C = (Fl > 0 && V0 > 0.1f) ? dot(L.m_Force, L.m_VelBefore) / (Fl * V0) : 0;
			std::printf("X %d dist %.1f force %.2f cos %+.2f surf %s along %.2f\n", Rt, L.m_Dist, Fl, C, Frz ? "freeze-lined" : "bare", Fl * C);
		}
		float d[NUM];
		d[EXPL] = En(A.m_VExpl) - En(A.m_V0);
		auto TickWith = [&](auto Mod) {
			CCharacterCore C = A.m_CoreBeforeTick;
			Mod(C);
			C.Tick(true, true);
			return C.m_Vel;
		};
		vec2 Vnh = TickWith([](CCharacterCore &C) { C.m_HookState = HOOK_IDLE; C.m_Input.m_Hook = 0; });
		const float Vx = A.m_VExpl.x;
		vec2 Vda = TickWith([&](CCharacterCore &C) { C.m_Input.m_Direction = Vx > 1 ? 1 : (Vx < -1 ? -1 : C.m_Input.m_Direction); });
		vec2 Vnj = TickWith([](CCharacterCore &C) { C.m_Input.m_Jump = 0; });
		const float Et = En(A.m_VTick);
		d[HOOK] = Et - En(Vnh);
		d[DIR] = Et - En(Vda);
		d[JUMP] = Et - En(Vnj);
		d[REST] = Et - En(A.m_VExpl) - d[HOOK] - d[DIR] - d[JUMP];
		d[COLL] = En(A.m_VMove) - Et;
		for(int k = 0; k < NUM; k++)
		{
			aSec[k] += d[k];
			aTot[k] += d[k];
		}
		if(getenv("EA_HOOK") && length(A.m_VTick - Vnh) > 0.01f)
		{
			// hook effect this tick: velocity with the hook vs without, split into speed change and rotation
			const float S1 = length(A.m_VTick), S0 = length(Vnh);
			const float Ang = std::atan2(A.m_VTick.y, A.m_VTick.x) - std::atan2(Vnh.y, Vnh.x);
			float Rot = std::fabs(std::remainder(Ang, 2 * pi));
			std::printf("H %d |v| %.1f dspeed %+.3f rot %.4f (%.2f deg) hookstate %d dir %d\n", Rt, S0, S1 - S0, Rot, Rot * 180 / pi, F.HookState(), vIn[i].m_Dir);
		}
		if(getenv("EA_KICKS") && length(A.m_VExpl - A.m_V0) > 0.5f)
		{
			const vec2 K = A.m_VExpl - A.m_V0;
			const float V0 = length(A.m_V0), Kl = length(K);
			const float C = V0 > 0.1f ? dot(K, A.m_V0) / (Kl * V0) : 1.0f;
			std::printf("K %d |v0| %.1f kick %.2f (%.2f,%.2f) cos %+.2f dE %+.0f ideal %+.0f |v1| %.1f v0 (%.1f,%.1f) v1 (%.1f,%.1f)\n", Rt, V0, Kl, K.x, K.y, C,
				En(A.m_VExpl) - En(A.m_V0), (V0 + Kl) * (V0 + Kl) / 2 - V0 * V0 / 2, length(A.m_VExpl), A.m_V0.x, A.m_V0.y, A.m_VExpl.x, A.m_VExpl.y);
		}
		if(getenv("EA_TICKS"))
			std::printf("T %d %d %.2f %.2f %.2f %.2f %.2f %.2f %.2f %d %d\n", Rt, vIn[i].m_Hook, d[EXPL], d[HOOK], d[DIR], d[JUMP], d[COLL], d[REST], length(A.m_V0), F.HookState(), vIn[i].m_Dir);
		for(int k = 0; k < NUM; k++)
			if(k != REST && std::fabs(d[k]) >= Ev)
				std::printf("  rt %d %-5s %+7.0f  (|v| %.1f -> %.1f, pos %.0f %.0f, in %d %d %d %d)\n", Rt, apN[k], d[k], length(A.m_V0), length(A.m_VMove),
					F.Pos().x, F.Pos().y, vIn[i].m_Dir, vIn[i].m_Jump, vIn[i].m_Hook, vIn[i].m_Fire);
		if(Rt - SecStart + 1 >= Sec)
		{
			std::printf("SEC %d-%d:", SecStart, Rt);
			for(int k = 0; k < NUM; k++)
				std::printf(" %s %+.0f", apN[k], aSec[k]);
			std::printf("  |v| end %.1f\n", length(F.Vel()));
			for(double &x : aSec)
				x = 0;
			SecStart = Rt + 1;
		}
		if(F.m_Dead)
		{
			std::printf("DEAD at rt %d\n", Rt);
			break;
		}
	}
	std::printf("TOTAL:");
	for(int k = 0; k < NUM; k++)
		std::printf(" %s %+.0f", apN[k], aTot[k]);
	std::printf("\n");
	return 0;
}
