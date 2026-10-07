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

// x-interval(s) of fire positions at height h for grenade time tau: |x - Xe| in [wlo, whi], Xe in [-16, 16]
struct SBand
{
	bool ok;
	float wlo, whi;
};
static SBand Band(float h, int tau)
{
	SBand B{false, 1e9f, -1e9f};
	for(int i = 0; i <= 16; i++)
	{
		float tc = tau - 1 + i / 16.0f;
		float r = 21 + 20 * tc, c = 0.28f * tc * tc;
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
		float a = B[i].wlo - 16, b = B[i].whi + 16;
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
		float a = B[i].wlo - 16, b = B[i].whi + 16;
		float glo = ((SignMask >> i) & 1) ? -b : a, ghi = ((SignMask >> i) & 1) ? -a : b;
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
	for(int i = 1; i < argc; i++)
	{
		if(!std::strncmp(argv[i], "umax=", 5))
			UMax = std::atof(argv[i] + 5);
		if(!std::strncmp(argv[i], "k=", 2))
			K = std::atoi(argv[i] + 2);
		if(!std::strncmp(argv[i], "slack=", 6))
			gSlack = std::atof(argv[i] + 6);
	}
	std::vector<std::vector<int>> vT;
	if(K == 3)
	{
		for(int a = 26; a <= 101; a++)
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
	std::printf("%zu tau sets, umax %.1f, slack %.1f\n", vT.size(), UMax, gSlack);
	int Feasible = 0, LipFeasibleSets = 0;
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
							for(int i = 0; i < k && !Bad; i++)
							{
								int j = T[i];
								float h;
								if(Mode == 0)
								{
									float vL = ip * 0.05f; // 0 .. 120
									int d = j - jL;
									h = 15 + d * vL - 0.25f * d * (d - 1);
								}
								else if(Mode == 1)
								{
									float vL = -12 + 0.5f * (jJ - jL);
									float vp = -100 + ip * 0.25f; // speed at the jump tick before the jump (down +)
									if(j <= jJ)
									{
										int d = j - jL;
										h = 15 + d * vL - 0.25f * d * (d - 1);
									}
									else
									{
										int dJ = jJ - jL;
										float hJ = 15 + dJ * vL - 0.25f * dJ * (dJ - 1);
										int d = j - jJ;
										h = hJ + d * vp - 0.25f * d * (d - 1);
									}
								}
								else
								{
									float v1 = -20 + ip * 0.05f; // -20 .. 100
									float h1 = 15 + ih;
									int d = j - 1;
									h = h1 + d * v1 - 0.25f * d * (d - 1);
								}
								if(h < -140) // beside and below the block top: even the air jump can't bring it back
								{
									Bad = true;
									break;
								}
								B[i] = Band(h, j);
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
								float u;
								float xl = Mode == 2 ? 20.0f : 30.0f;
								if(LineFeasible(k, n, B, Sm, -xl, xl, UMax, u))
								{
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
			std::printf("  smallest |u| %.2f  (%s)\n", SetBestU, aDesc);
		}
	}
	std::printf("no-kick model (|vx| <= 5, any path): %d of %zu tau sets feasible\n", LipFeasibleSets, vT.size());
	std::printf("constant-speed approach model: %d of %zu tau sets feasible; smallest approach speed |u| over all: %.2f px/tick\n", Feasible, vT.size(), BestU);
	return 0;
}
