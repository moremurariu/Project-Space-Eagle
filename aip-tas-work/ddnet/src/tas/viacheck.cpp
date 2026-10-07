// viacheck MAP INPUTS [goalx goaly maxticks beam radius]: replays INPUTS to the grenade pickup and reports how many
// ticks a small search needs from there to the goal point (default: the climb out of the grenade pocket) without freezing.
#include "via.h"
#include "tasio.h"
#include <game/collision.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>

int main(int argc, const char **argv)
{
	if(argc < 3)
	{
		std::printf("usage: viacheck MAP INPUTS [goalx goaly maxticks beam radius]\n");
		return 1;
	}
	vec2 Goal(argc > 4 ? atof(argv[3]) : 5254.0f, argc > 4 ? atof(argv[4]) : 2176.0f);
	int MaxTicks = argc > 5 ? atoi(argv[5]) : 80;
	int Beam = argc > 6 ? atoi(argv[6]) : 4000;
	float R = argc > 7 ? atof(argv[7]) : 48.0f;
	if(!CTasGame::LoadMap(argv[1]))
		return 1;
	CFast::Init();
	CCollision::ms_FastPaths = true;
	std::vector<STasInput> vIn = ReadInputs(argv[2]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	CFast F;
	F.FromGame(G);
	size_t i = 0;
	for(; i < vIn.size(); i++)
	{
		F.Step(vIn[i]);
		if(F.m_Dead)
		{
			std::printf("dies at input %d\n", (int)i + 1);
			return 1;
		}
		if(F.m_Got)
			break;
	}
	if(!F.m_Got)
	{
		std::printf("no pickup\n");
		return 1;
	}
	auto T0 = std::chrono::steady_clock::now();
	CViaField Field;
	Field.Build(Goal);
	int Pickup = F.RaceTick();
	int T = ViaTicks(Field, F, vIn[i].m_Hook, vIn[i].m_Jump, MaxTicks, Beam, R);
	double Sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - T0).count();
	std::printf("pickup race tick %d, pos %.0f %.0f vel %.2f %.2f jumped %d hook %d; goal %.0f %.0f: %s", Pickup, F.m_Core.m_Pos.x, F.m_Core.m_Pos.y,
		F.m_Core.m_Vel.x, F.m_Core.m_Vel.y, F.m_Core.m_Jumped, F.m_Core.m_HookState, Goal.x, Goal.y, T < 0 ? "NOT REACHED" : "reached");
	if(T >= 0)
		std::printf(" after %d ticks (race tick %d)", T, Pickup + T);
	std::printf(" [%.1fs]\n", Sec);
	return T < 0 ? 2 : 0;
}
