// nadebound: relaxed feasibility check for "k gates" (grenades fired >= 26 ticks before the stack tick E that explode
// next to the tee at E), single block. Standalone (no DDNet code): build with  g++ -O2 -std=c++17 nadebound.cpp -o nadebound
//
// Coordinates: x relative to the block centre, h = height of the tee centre above the block top (standing: 15),
// j = ticks before E (fire step s = E - j + 1 fires from the position after step E - j, tau = j + 1 ... we use the
// grenade time tau = j directly with its sub-tick slack, see below).
//
// Tee model (the relaxation):
//   vertical: exact free fall (v += 0.5 per tick, h -= v) between events; events: landing on the block (end of the
//   window), or ONE air jump (v := -12) at any tick; optional pre-window state is free (any height, any speed).
//   horizontal: x(j) = xL + u*(j - jL) for a constant speed u of ANY size (an approach from any distance at any speed;
//   with air control alone |u| <= 5, faster approaches need explosion kicks), xL = x at landing / at E-1.
// Gate test (generous): the fire point lies within `slack` px of the circle of radius 21+20tc around
//   (Xe, 0.28 tc^2 above the top) for some tc in [tau-1, tau] and some Xe in [-16, 16] (any point of the block top).
// End: (a) "ground": the tee lands on the block at jL in [2, 25] (|xL| <= 30), or (b) "air": at j = 1 the tee is
//   15..45 px above the top over the block (|xL| <= 20) with the air jump unused in the window.
// For every tau triple (tau_a < tau_b < tau_c, spacing >= 25, tau_a >= 26, tau_c <= 101) the tool scans the remaining
// parameters (landing speed / height, air jump tick and pre-jump speed) and solves the x-part exactly (2-D linear
// feasibility in (xL, u)). It prints every feasible triple with the smallest |u| found.
//
// usage: nadebound [umax=1000] [k=3] [slack=2]
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static float gSlack = 2.0f;
static float gXe = 16.0f, gHElo = 0.0f, gHEhi = 0.0f; // explosion point: |x| <= gXe, height above the top in [gHElo, gHEhi]

// x-interval(s) of fire positions at height h for grenade time tau: |x - Xe| in [wlo, whi], Xe in [-16, 16]
struct SBand
{
	bool ok;
	float wlo, whi;
	float xe;
};
// explosion region (relative to the block centre / top): tile hits land on the top surface (|x| <= 16, height -1..0)
// or on a side face (|x| = 16, height -32..0); a lifetime explosion (tau = 101) can be anywhere within 48 px of the
// tee, which stands on the block (|x| <= 30, 13..48 above the top) -> |x| <= 78, height -35..96.
static SBand Band(float h, int tau, float &XeOut)
{
	SBand B{false, 1e9f, -1e9f, 16.0f};
	float hElo = tau == 101 ? -35.0f : -32.0f, hEhi = tau == 101 ? 96.0f : 1.0f;
	XeOut = tau == 101 ? 78.0f : 16.0f;
	B.xe = XeOut;
	const int NE = 12;
	for(int e = 0; e <= NE; e++)
	{
		float hE = hElo + (hEhi - hElo) * e / NE;
		for(int i = 0; i <= 16; i++)
		{
			float tc = tau - 1 + i / 16.0f;
			if(tau == 101 && i < 16)
				continue; // a lifetime explosion happens exactly at tau = 101
			float r = 21 + 20 * tc, c = 0.28f * tc * tc + hE;
			float dy = h - c;
			float rr = r + gSlack;
			if(std::fabs(dy) > rr)
				continue;
			float wmax = std::sqrt(rr * rr - dy * dy);
			float rl = std::max(0.0f, r - gSlack);
			float wmin = std::fabs(dy) >= rl ? 0.0f : std::sqrt(rl * rl - dy * dy);
			B.ok = true;
			B.wlo = std::min(B.wlo, wmin);
			B.whi = std::max(B.whi, wmax);
		}
	}
	return B;
}

// is there (xL, u) with xL in [xl0, xl1], |u| <= umax, and x_i = xL + u*n_i in sgn_i*[wlo_i - 16, whi_i + 16]?
static bool LineFeasible(int k, const float *n, const SBand *B, int SignMask, float xl0, float xl1, float umax, float &uOut)
{
	// constraints lo_i <= xL + u n_i <= hi_i ; xl0 <= xL <= xl1 ; -umax <= u <= umax
	float lo[8], hi[8], nn[8];
	int m = 0;
	for(int i = 0; i < k; i++)
	{
		float a = B[i].wlo - B[i].xe, b = B[i].whi + B[i].xe;
		if((SignMask >> i) & 1)
		{
			lo[m] = -b;
			hi[m] = -a;
		}
		else
		{
			lo[m] = a;
			hi[m] = b;
		}
		nn[m] = n[i];
		m++;
	}
	lo[m] = xl0;
	hi[m] = xl1;
	nn[m] = 0;
	m++;
	// eliminate xL: for all p,q: lo_p - u n_p <= hi_q - u n_q  ->  u (n_q - n_p) <= hi_q - lo_p
	float ulo = -umax, uhi = umax;
	for(int p = 0; p < m; p++)
		for(int q = 0; q < m; q++)
		{
			float a = nn[q] - nn[p], b = hi[q] - lo[p];
			if(a == 0)
			{
				if(b < 0)
					return false;
				continue;
			}
			if(a > 0)
				uhi = std::min(uhi, b / a);
			else
				ulo = std::max(ulo, b / a);
		}
	if(ulo > uhi)
		return false;
	uOut = std::fabs(ulo) < std::fabs(uhi) ? (ulo <= 0 && uhi >= 0 ? 0 : ulo) : uhi;
	if(ulo <= 0 && uhi >= 0)
		uOut = 0;
	return true;
}

// Lipschitz relaxation (no kicks): x_i in sgn_i*[wlo_i-16, whi_i+16], |x_i - x_j| <= vmax |n_i - n_j|, |xL| <= xl
static bool LipFeasible(int k, const float *n, const SBand *B, int SignMask, float xl, float vmax)
{
	// propagate an interval from the landing (n = 0) backwards through the gates in increasing n
	int ord[8];
	for(int i = 0; i < k; i++)
		ord[i] = i;
	std::sort(ord, ord + k, [&](int a, int b) { return n[a] < n[b]; });
	float lo = -xl, hi = xl, nprev = 0;
	for(int t = 0; t < k; t++)
	{
		int i = ord[t];
		float d = vmax * (n[i] - nprev);
		lo -= d;
		hi += d;
		float a = B[i].wlo - B[i].xe, b = B[i].whi + B[i].xe;
		float glo = ((SignMask >> i) & 1) ? -b : a, ghi = ((SignMask >> i) & 1) ? -a : b;
		lo = std::max(lo, glo);
		hi = std::min(hi, ghi);
		if(lo > hi)
			return false;
		nprev = n[i];
	}
	return true;
}

// one horizontal kick toward the block at backward time nk (from the landing). All gates on one side (x >= 0 = the
// tee's side; the mirror case is symmetric). Forward in time the distance to the block may shrink at <= vfast after
// the kick (approach) and <= vslow before it, and grow at <= vslow (air control) - so going back in time from the
// landing, x may grow by vfast per tick (n < nk) or vslow (n >= nk), and shrink by vslow.
static bool LipKickFeasible(int k, const float *n, const SBand *B, float xl, float vslow, float vfast, float nk)
{
	int ord[8];
	for(int i = 0; i < k; i++)
		ord[i] = i;
	std::sort(ord, ord + k, [&](int a, int b) { return n[a] < n[b]; });
	float lo = -xl, hi = xl, nprev = 0;
	for(int t = 0; t < k; t++)
	{
		int i = ord[t];
		float a0 = nprev, a1 = n[i];
		float fast = std::max(0.0f, std::min(a1, nk) - a0), slow = (a1 - a0) - fast;
		hi += vfast * fast + vslow * slow;
		lo -= vslow * (a1 - a0);
		float glo = B[i].wlo - B[i].xe, ghi = B[i].whi + B[i].xe;
		lo = std::max(lo, glo);
		hi = std::min(hi, ghi);
		if(lo > hi)
			return false;
		nprev = n[i];
	}
	return true;
}

int main(int argc, char **argv)
{
	float UMax = 1000;
	int K = 3;
	int AMin = 26, AMax = 101;
	for(int i = 1; i < argc; i++)
	{
		if(!std::strncmp(argv[i], "umax=", 5))
			UMax = std::atof(argv[i] + 5);
		if(!std::strncmp(argv[i], "k=", 2))
			K = std::atoi(argv[i] + 2);
		if(!std::strncmp(argv[i], "slack=", 6))
			gSlack = std::atof(argv[i] + 6);
		if(!std::strncmp(argv[i], "xe=", 3))
			gXe = std::atof(argv[i] + 3);
		if(!std::strncmp(argv[i], "helo=", 5))
			gHElo = std::atof(argv[i] + 5);
		if(!std::strncmp(argv[i], "hehi=", 5))
			gHEhi = std::atof(argv[i] + 5);
		if(!std::strncmp(argv[i], "amin=", 5))
			AMin = std::atoi(argv[i] + 5);
		if(!std::strncmp(argv[i], "amax=", 5))
			AMax = std::atoi(argv[i] + 5);
	}
	std::vector<std::vector<int>> vT;
	if(K == 3)
	{
		for(int a = AMin; a <= std::min(AMax, 101); a++)
			for(int b = a + 25; b <= 101; b++)
				for(int c = b + 25; c <= 101; c++)
					vT.push_back({a, b, c});
	}
	else if(K == 2)
	{
		for(int a = 26; a <= 101; a++)
			for(int b = a + 25; b <= 101; b++)
				vT.push_back({a, b});
	}
	else
		vT.push_back({26, 51, 76, 101});
	if(getenv("TRIPLE"))
	{
		vT.clear();
		std::vector<int> t;
		int a, b, c;
		std::sscanf(getenv("TRIPLE"), "%d,%d,%d", &a, &b, &c);
		vT.push_back({a, b, c});
	}
	std::printf("%zu tau sets, umax %.1f, slack %.1f, explosion |x| <= %.0f, height %.0f..%.0f\n", vT.size(), UMax, gSlack, gXe, gHElo, gHEhi);
	int Feasible = 0, LipFeasibleSets = 0;
	float GMinPhi[4] = {1e9f, 1e9f, 1e9f, 1e9f};
	int KickSets = 0;
	float GKickPhi = 1e9f;
	float BestU = 1e9;
	for(const auto &T : vT)
	{
		int k = (int)T.size();
		float n[8];
		SBand B[8];
		float SetBestU = 1e9;
		char aDesc[256] = "";
		long LipCount = 0;
		char LipDesc[256] = "";
		float MinPhi[4] = {1e9f, 1e9f, 1e9f, 1e9f};
		char PhiDesc[256] = "";
		float KickPhi = 1e9f;
		char KickDesc[256] = ""; // by horizontal kicks needed: ceil((|u|-5)/12)
		// (a) ground end, no air jump: landing at jL with speed vL (down); h(j) = 15 + d vL - 0.25 d (d - 1), d = j - jL
		// (b) ground end with an air jump at jJ (> jL): after the jump vy = -12 -> landing speed vL = -12 + 0.5 (jJ - jL)
		//     (must be > 0); before the jump (j > jJ) free fall with speed vp at the jump tick
		// (c) air end: h(1) = h1 in [15, 45] with speed v1 (down), no air jump in the window
		for(int Mode = 0; Mode < 3; Mode++)
		{
			for(int jL = (Mode == 2 ? 1 : 2); jL <= (Mode == 2 ? 1 : 25); jL++)
			{
				if(jL >= T[0])
					break;
				for(int jJ = (Mode == 1 ? jL + 24 : 0); jJ <= (Mode == 1 ? 101 : 0); jJ++)
				{
					int nP = Mode == 1 ? 801 : 2401;
					for(int ip = 0; ip < nP; ip++)
					{
						for(int ih = 0; ih < (Mode == 2 ? 31 : 1); ih++)
						{
							bool Bad = false;
							float PhiC = 0; // vertical energy h + v^2 at the earliest gate (largest tau)
							for(int i = 0; i < k && !Bad; i++)
							{
								int j = T[i];
								float h;
								float vj = 0; // downward speed at j
								if(Mode == 0)
								{
									float vL = ip * 0.05f; // 0 .. 120
									int d = j - jL;
									h = 15 + d * vL - 0.25f * d * (d - 1);
									vj = vL - 0.5f * d;
								}
								else if(Mode == 1)
								{
									float vL = -12 + 0.5f * (jJ - jL);
									float vp = -100 + ip * 0.25f; // speed at the jump tick before the jump (down +)
									if(j <= jJ)
									{
										int d = j - jL;
										h = 15 + d * vL - 0.25f * d * (d - 1);
										vj = vL - 0.5f * d;
									}
									else
									{
										int dJ = jJ - jL;
										float hJ = 15 + dJ * vL - 0.25f * dJ * (dJ - 1);
										int d = j - jJ;
										h = hJ + d * vp - 0.25f * d * (d - 1);
										vj = vp - 0.5f * d;
									}
								}
								else
								{
									float v1 = -20 + ip * 0.05f; // -20 .. 100
									float h1 = 15 + ih;
									int d = j - 1;
									h = h1 + d * v1 - 0.25f * d * (d - 1);
									vj = v1 - 0.5f * d;
								}
								if(h < -140) // beside and below the block top: even the air jump can't bring it back
								{
									Bad = true;
									break;
								}
								if(i == k - 1)
									PhiC = h + vj * vj;
								float XeTmp;
								B[i] = Band(h, j, XeTmp);
								n[i] = (float)(j - jL);
								if(!B[i].ok)
									Bad = true;
							}
							if(Bad)
								continue;
							for(int Sm = 0; Sm < (1 << k); Sm++)
							{
								if(LipFeasible(k, n, B, Sm, Mode == 2 ? 20.0f : 30.0f, 5.0f))
								{
									LipCount++;
									if(!LipDesc[0])
										std::snprintf(LipDesc, sizeof(LipDesc), "mode %d jL %d jJ %d ip %d ih %d signs %d", Mode, jL, jJ, ip, ih, Sm);
								}
								{
									float xlk = Mode == 2 ? 20.0f : 30.0f;
									// earliest kick allowed: its fire (101 ticks earlier) must be >= 25 before the first gate's fire
									int jkMin = std::max(0, T[k - 1] - 76);
									for(int jk = jkMin; jk <= 110; jk += 2)
										if(Sm == 0 && LipKickFeasible(k, n, B, xlk, 5.0f, 17.0f, (float)(jk - jL)))
										{
											if(PhiC < KickPhi)
											{
												KickPhi = PhiC;
												std::snprintf(KickDesc, sizeof(KickDesc), "mode %d jL %d jJ %d ip %d ih %d signs %d kick at j=%d", Mode, jL, jJ, ip, ih, Sm, jk);
											}
											break;
										}
								}
								float u;
								float xl = Mode == 2 ? 20.0f : 30.0f;
								if(LineFeasible(k, n, B, Sm, -xl, xl, UMax, u))
								{
									int kh = std::fabs(u) <= 5 ? 0 : (int)std::ceil((std::fabs(u) - 5) / 12);
									if(kh < 4 && PhiC < MinPhi[kh])
									{
										MinPhi[kh] = PhiC;
										if(kh == 1)
											std::snprintf(PhiDesc, sizeof(PhiDesc), "mode %d jL %d jJ %d ip %d ih %d signs %d u %.2f", Mode, jL, jJ, ip, ih, Sm, u);
									}
									if(std::fabs(u) < SetBestU)
									{
										SetBestU = std::fabs(u);
										std::snprintf(aDesc, sizeof(aDesc), "mode %d jL %d jJ %d ip %d ih %d signs %d u %.2f", Mode, jL, jJ, ip, ih, Sm, u);
									}
								}
							}
						}
					}
				}
			}
		}
		if(KickPhi < 1e9f)
		{
			KickSets++;
			GKickPhi = std::min(GKickPhi, KickPhi);
			std::printf("ONE-KICK (speed <=5 before, <=17 after a kick at any allowed time) feasible:");
			for(int t : T)
				std::printf(" %d", t);
			std::printf("  min vertical energy %.0f (%s)\n", KickPhi, KickDesc);
		}
		if(LipCount)
		{
			LipFeasibleSets++;
			std::printf("NO-KICK (|vx|<=5) feasible:");
			for(int t : T)
				std::printf(" %d", t);
			std::printf("  (%ld hits, e.g. %s)\n", LipCount, LipDesc);
		}
		if(SetBestU < 1e9)
		{
			Feasible++;
			BestU = std::min(BestU, SetBestU);
			std::printf("feasible:");
			for(int t : T)
				std::printf(" %d", t);
			std::printf("  smallest |u| %.2f  (%s)  min vertical energy h+v^2 at the first gate by kicks needed: %.0f %.0f %.0f %.0f\n", SetBestU, aDesc, MinPhi[0], MinPhi[1], MinPhi[2], MinPhi[3]);
			for(int q = 0; q < 4; q++)
				GMinPhi[q] = std::min(GMinPhi[q], MinPhi[q]);
			std::printf("   min-energy 1-kick solution: %s\n", PhiDesc);
		}
	}
	std::printf("no-kick model (|vx| <= 5, any path): %d of %zu tau sets feasible\n", LipFeasibleSets, vT.size());
	std::printf("one-horizontal-kick model (<=5 before, <=17 after): %d of %zu tau sets feasible, min vertical energy at the first gate %.0f px\n", KickSets, vT.size(), GKickPhi);
	std::printf("constant-speed model, min vertical energy h+v^2 (px above the top) at the first gate, by horizontal kicks needed (0,1,2,3): %.0f %.0f %.0f %.0f\n", GMinPhi[0], GMinPhi[1], GMinPhi[2], GMinPhi[3]);
	std::printf("constant-speed approach model: %d of %zu tau sets feasible; smallest approach speed |u| over all: %.2f px/tick\n", Feasible, vT.size(), BestU);
	return 0;
}
