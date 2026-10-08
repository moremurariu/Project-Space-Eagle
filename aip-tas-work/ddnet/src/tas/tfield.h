// Velocity-aware time-to-go field for the post-grenade part (see x_tfield.cpp): T(x, y, heading, speed) = ticks to the
// finish under a reduced point-mass model of the tee (gravity, the horizontal speed ramp, lossless hook turning up to
// a calibrated lateral acceleration, hook / direction braking, an average kick acceleration, freeze and solid as walls).
// Loaded by x_ds (tfield=FILE) to rank beam states by t + T instead of the lag behind the incumbent at the same
// geodesic distance plus an energy credit: the field knows which speed can be carried through the next turns.
#ifndef TAS_TFIELD_H
#define TAS_TFIELD_H

#include <base/vmath.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

struct STField
{
	int m_C = 16; // cell size (px)
	int m_NH = 24, m_NV = 16; // heading bins, speed levels
	float m_DV = 5; // speed step (px/t)
	int m_W = 0, m_H = 0, m_NC = 0;
	std::vector<int> m_vIdx; // cell -> compact index (-1: not a legal tee centre)
	std::vector<float> m_vT; // [compact cell][heading][speed]
	static constexpr float INF = 1e6f;

	float &At(int c, int h, int k) { return m_vT[((size_t)c * m_NH + h) * m_NV + k]; }
	float At(int c, int h, int k) const { return m_vT[((size_t)c * m_NH + h) * m_NV + k]; }
	int Cell(int cx, int cy) const { return cx < 0 || cy < 0 || cx >= m_W || cy >= m_H ? -1 : m_vIdx[cy * m_W + cx]; }

	// multilinear interpolation; cells that are not legal or unreached are skipped (renormalized)
	float Value(vec2 P, vec2 V) const
	{
		float fx = P.x / m_C - 0.5f, fy = P.y / m_C - 0.5f;
		int x0 = (int)std::floor(fx), y0 = (int)std::floor(fy);
		float wx = fx - x0, wy = fy - y0;
		float Sp = length(V);
		float Th = std::atan2(V.y, V.x);
		if(Th < 0)
			Th += 2 * pi;
		float fh = Th / (2 * pi) * m_NH;
		int h0 = (int)std::floor(fh);
		float wh = fh - h0;
		h0 %= m_NH;
		int h1 = (h0 + 1) % m_NH;
		float fv = std::clamp(Sp / m_DV, 0.0f, (float)(m_NV - 1) - 1e-4f);
		int k0 = (int)std::floor(fv);
		float wv = fv - k0;
		int k1 = std::min(k0 + 1, m_NV - 1);
		float Sum = 0, WSum = 0;
		for(int dy = 0; dy <= 1; dy++)
			for(int dx = 0; dx <= 1; dx++)
			{
				int c = Cell(x0 + dx, y0 + dy);
				if(c < 0)
					continue;
				float w = (dx ? wx : 1 - wx) * (dy ? wy : 1 - wy);
				if(w <= 0)
					continue;
				float a = At(c, h0, k0), b = At(c, h0, k1), d = At(c, h1, k0), e = At(c, h1, k1);
				float m = std::max(std::max(a, b), std::max(d, e));
				if(m >= INF * 0.5f)
				{
					// partly unreached corner: use the reached values only
					float s = 0, ws = 0;
					float aw[4] = {(1 - wh) * (1 - wv), (1 - wh) * wv, wh * (1 - wv), wh * wv};
					float av[4] = {a, b, d, e};
					for(int q = 0; q < 4; q++)
						if(av[q] < INF * 0.5f)
						{
							s += aw[q] * av[q];
							ws += aw[q];
						}
					if(ws < 0.25f)
						continue;
					Sum += w * s / ws;
					WSum += w;
					continue;
				}
				float v = (1 - wh) * ((1 - wv) * a + wv * b) + wh * ((1 - wv) * d + wv * e);
				Sum += w * v;
				WSum += w;
			}
		if(WSum > 0.05f)
			return Sum / WSum;
		// no reached cell around (e.g. the tee grazes a block): the best nearby cell within 2 cells
		float Best = INF;
		int cx = (int)std::floor(P.x / m_C), cy = (int)std::floor(P.y / m_C);
		for(int dy = -2; dy <= 2; dy++)
			for(int dx = -2; dx <= 2; dx++)
			{
				int c = Cell(cx + dx, cy + dy);
				if(c < 0)
					continue;
				float v = (1 - wh) * ((1 - wv) * At(c, h0, k0) + wv * At(c, h0, k1)) + wh * ((1 - wv) * At(c, h1, k0) + wv * At(c, h1, k1));
				if(v < INF * 0.5f)
					Best = std::min(Best, v + 0.5f * (std::abs(dx) + std::abs(dy)));
			}
		return Best;
	}

	bool Save(const char *pPath) const
	{
		FILE *f = std::fopen(pPath, "wb");
		if(!f)
			return false;
		int aHdr[6] = {m_C, m_NH, m_NV, m_W, m_H, m_NC};
		std::fwrite(aHdr, sizeof(aHdr), 1, f);
		std::fwrite(&m_DV, sizeof(float), 1, f);
		std::fwrite(m_vIdx.data(), sizeof(int), m_vIdx.size(), f);
		std::fwrite(m_vT.data(), sizeof(float), m_vT.size(), f);
		std::fclose(f);
		return true;
	}
	bool Load(const char *pPath)
	{
		FILE *f = std::fopen(pPath, "rb");
		if(!f)
			return false;
		int aHdr[6];
		bool Ok = std::fread(aHdr, sizeof(aHdr), 1, f) == 1 && std::fread(&m_DV, sizeof(float), 1, f) == 1;
		if(Ok)
		{
			m_C = aHdr[0];
			m_NH = aHdr[1];
			m_NV = aHdr[2];
			m_W = aHdr[3];
			m_H = aHdr[4];
			m_NC = aHdr[5];
			m_vIdx.resize((size_t)m_W * m_H);
			m_vT.resize((size_t)m_NC * m_NH * m_NV);
			Ok = std::fread(m_vIdx.data(), sizeof(int), m_vIdx.size(), f) == m_vIdx.size() &&
			     std::fread(m_vT.data(), sizeof(float), m_vT.size(), f) == m_vT.size();
		}
		std::fclose(f);
		return Ok;
	}
};

#endif
