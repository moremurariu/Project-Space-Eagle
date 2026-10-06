// nadeaim: aim helper. Replays PREFIX (input file) to get a state, then for a grenade fired at the next step (from the
// tee's current position) prints, for every explosion tick tau in [tau0, tau1], the best aim that explodes right under
// a given target point at exactly that tick (SolveAim), and the exact flight.
// usage: nadeaim MAP PREFIX TX TY [tau0=1] [tau1=101] [allowlast=0] [onlyhits=0]
//   (TX, TY) = where the tee will be when the grenade explodes (CCharacter::m_Pos), e.g. 4816 12772
//   allowlast=1 also accepts tile hits on the last lifetime tick (the server explodes those once, the client
//   prediction code that CTasGame/CFastG run explodes them twice)
// Also: nadeaim MAP PREFIX fly AIMX AIMY  -> exact flight of a grenade fired with that aim from the current position.
#include "nade.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, const char **argv)
{
	if(argc < 5 || !CTasGame::LoadMap(argv[1]))
	{
		std::printf("usage: nadeaim MAP PREFIX TX TY [tau0] [tau1] [allowlast]\n       nadeaim MAP PREFIX fly AIMX AIMY\n");
		return 1;
	}
	CNadeG::Init();
	std::vector<STasInput> vIn = ReadInputFile(argv[2]);
	CTasGame G;
	G.Spawn(CTasGame::Map().m_vSpawns[0]);
	CNadeG F;
	F.FromGame(G);
	for(const STasInput &In : vIn)
		F.Step(In);
	vec2 P = F.m_Pos;
	std::printf("state after %zu steps: tee %.0f %.0f vel %.3f %.3f reload %d\n", vIn.size(), P.x, P.y, F.m_Core.m_Vel.x, F.m_Core.m_Vel.y, F.m_ReloadTimer);
	if(!std::strcmp(argv[3], "fly"))
	{
		int ax = std::atoi(argv[4]), ay = argc > 5 ? std::atoi(argv[5]) : 0;
		SNadeFlight Fl = FlyGrenade(P, AimDir(ax, ay));
		std::printf("aim %d %d: explodes %d ticks after the fire step's old tick at %.2f %.2f (%s)\n", ax, ay, Fl.m_Tau, Fl.m_E.x, Fl.m_E.y,
			Fl.m_Collide ? (Fl.m_Tau == 101 ? "tile hit on the LAST lifetime tick: server 1 explosion, prediction 2" : "tile hit") : "lifetime end");
		return 0;
	}
	vec2 T(std::atof(argv[3]), std::atof(argv[4]));
	int t0 = argc > 5 ? std::atoi(argv[5]) : 1, t1 = argc > 6 ? std::atoi(argv[6]) : 101;
	bool AllowLast = argc > 7 && std::atoi(argv[7]);
	bool OnlyHits = argc > 8 && std::atoi(argv[8]);
	for(int tau = t0; tau <= t1; tau++)
	{
		SAim A = SolveAim(P, tau, T, AllowLast, vec2(0, -1), OnlyHits);
		if(!A.m_Ok)
			continue;
		std::printf("tau %3d: aim %6d %6d -> explosion at %.2f %.2f (%s), kick on the target %.2f %.2f\n", tau, A.m_TX, A.m_TY, A.m_F.m_E.x, A.m_F.m_E.y,
			A.m_F.m_Collide ? (tau == 101 ? "hit on the last lifetime tick" : "hit") : "lifetime", A.m_Kick.x, A.m_Kick.y);
	}
	return 0;
}
