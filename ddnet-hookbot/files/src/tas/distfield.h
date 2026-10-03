// Geodesic distance field for the TAS tools (extracted from tas.cpp).
#ifndef TAS_DISTFIELD_H
#define TAS_DISTFIELD_H
#include "sim.h"
#include <game/mapitems.h>
#include <algorithm>
#include <cmath>
#include <queue>
#include <vector>
inline int gs_DFStencil = 3;
struct SDistField
{
	static constexpr int CELL = 4;
	int m_W = 0, m_H = 0;
	std::vector<float> m_vD;
	const std::vector<float> *m_pTileCost = nullptr; // optional per-map-tile cost multiplier (vref / expected speed)

	static bool CenterOk(const SMapInfo &M, float x, float y)
	{
		// the tee box (28x28) must not overlap solid tiles, and its center must stay out of freeze
		const float r = 14.0f;
		float axs[2] = {x - r, x + r - 0.01f}, ays[2] = {y - r, y + r - 0.01f};
		for(float ax : axs)
			for(float ay : ays)
			{
				int t = M.Tile((int)std::floor(ax / 32), (int)std::floor(ay / 32));
				if(t == TILE_SOLID || t == TILE_NOHOOK)
					return false;
			}
		int cx = (int)std::floor(x / 32), cy = (int)std::floor(y / 32);
		int t = M.Tile(cx, cy), f = M.Front(cx, cy);
		if(t == TILE_FREEZE || f == TILE_FREEZE || t == TILE_DEATH || f == TILE_DEATH)
			return false;
		return true;
	}

	void Build(const SMapInfo &M, const std::vector<int> &vGoalTiles)
	{
		m_W = M.m_W * 32 / CELL;
		m_H = M.m_H * 32 / CELL;
		std::vector<uint8_t> vOk(m_W * m_H);
		for(int y = 0; y < m_H; y++)
			for(int x = 0; x < m_W; x++)
				vOk[y * m_W + x] = CenterOk(M, x * CELL + CELL / 2.0f, y * CELL + CELL / 2.0f);
		m_vD.assign(m_W * m_H, 1e9f);
		using QE = std::pair<float, int>;
		std::priority_queue<QE, std::vector<QE>, std::greater<QE>> Q;
		for(int y = 0; y < m_H; y++)
			for(int x = 0; x < m_W; x++)
			{
				int tx = x * CELL / 32, ty = y * CELL / 32;
				int t = M.Tile(tx, ty), f = M.Front(tx, ty);
				if(std::find(vGoalTiles.begin(), vGoalTiles.end(), t) != vGoalTiles.end() ||
					std::find(vGoalTiles.begin(), vGoalTiles.end(), f) != vGoalTiles.end())
				{
					m_vD[y * m_W + x] = 0;
					Q.push({0.f, y * m_W + x});
				}
			}
		// 16-neighbour stencil (knight moves too): the 8-neighbour one overestimates some diagonal
		// distances by up to 8%, which steers the search away from diagonal lines
		// stencil: all moves with |dx|,|dy| <= R and gcd 1 (R = 1: 8-neighbour, 2: 16, 3: 32)
		std::vector<int> aDx, aDy;
		std::vector<float> aC;
		const int R = gs_DFStencil;
		for(int pass = 0; pass < 2; pass++)
			for(int dy = -R; dy <= R; dy++)
				for(int dx = -R; dx <= R; dx++)
				{
					if(!dx && !dy)
						continue;
					int a = std::abs(dx), b = std::abs(dy);
					while(b)
					{
						int t = a % b;
						a = b;
						b = t;
					}
					if(a != 1)
						continue;
					bool Unit = std::abs(dx) <= 1 && std::abs(dy) <= 1;
					if(Unit != (pass == 0))
						continue;
					aDx.push_back(dx);
					aDy.push_back(dy);
					aC.push_back(std::sqrt((float)(dx * dx + dy * dy)));
				}
		const int NumN = (int)aDx.size();
		auto Ok = [&](int x, int y) { return x >= 0 && y >= 0 && x < m_W && y < m_H && vOk[y * m_W + x]; };
		while(!Q.empty())
		{
			auto [d, i] = Q.top();
			Q.pop();
			if(d > m_vD[i])
				continue;
			int x = i % m_W, y = i / m_W;
			for(int k = 0; k < NumN; k++)
			{
				int nx = x + aDx[k], ny = y + aDy[k];
				if(nx < 0 || ny < 0 || nx >= m_W || ny >= m_H)
					continue;
				int j = ny * m_W + nx;
				if(!vOk[j])
					continue;
				if(k >= 8)
				{
					// every cell the move passes through must be free
					bool Clear = true;
					const int Steps = 4 * std::max(std::abs(aDx[k]), std::abs(aDy[k]));
					for(int t = 1; t < Steps && Clear; t++)
					{
						float fx = x + 0.5f + aDx[k] * (float)t / Steps, fy = y + 0.5f + aDy[k] * (float)t / Steps;
						Clear = Ok((int)std::floor(fx), (int)std::floor(fy));
					}
					if(!Clear)
						continue;
				}
				float Cost = 1.0f;
				if(m_pTileCost)
				{
					const int MW = m_W * CELL / 32;
					const int tx0 = x * CELL / 32, ty0 = y * CELL / 32, tx1 = nx * CELL / 32, ty1 = ny * CELL / 32;
					Cost = 0.5f * ((*m_pTileCost)[ty0 * MW + tx0] + (*m_pTileCost)[ty1 * MW + tx1]);
				}
				float nd = d + aC[k] * CELL * Cost;
				if(nd < m_vD[j])
				{
					m_vD[j] = nd;
					Q.push({nd, j});
				}
			}
		}
	}

	void BuildFromPoint(const SMapInfo &M, vec2 P)
	{
		// goal = cells within pickup range of P
		SMapInfo Tmp = M;
		int tx = (int)(P.x / 32), ty = (int)(P.y / 32);
		const uint8_t GOAL = 250;
		Tmp.m_vGame[ty * Tmp.m_W + tx] = GOAL;
		Build(Tmp, {GOAL});
	}

	float At(int x, int y) const
	{
		x = std::clamp(x, 0, m_W - 1);
		y = std::clamp(y, 0, m_H - 1);
		return m_vD[y * m_W + x];
	}
	// bilinear sample; unreachable cells are 1e9, so avoid mixing them in
	float Sample(vec2 p) const
	{
		float fx = p.x / CELL - 0.5f, fy = p.y / CELL - 0.5f;
		int x0 = (int)std::floor(fx), y0 = (int)std::floor(fy);
		float ax = fx - x0, ay = fy - y0;
		float a[4] = {At(x0, y0), At(x0 + 1, y0), At(x0, y0 + 1), At(x0 + 1, y0 + 1)};
		float w[4] = {(1 - ax) * (1 - ay), ax * (1 - ay), (1 - ax) * ay, ax * ay};
		float s = 0, ws = 0, mn = 1e9f;
		for(int k = 0; k < 4; k++)
		{
			mn = std::min(mn, a[k]);
			if(a[k] < 1e8f)
			{
				s += a[k] * w[k];
				ws += w[k];
			}
		}
		if(ws <= 0)
			return 1e9f;
		return s / ws;
	}
	vec2 Grad(vec2 p) const
	{
		const float h = 6.0f;
		float dx = Sample(p + vec2(h, 0)) - Sample(p - vec2(h, 0));
		float dy = Sample(p + vec2(0, h)) - Sample(p - vec2(0, h));
		if(std::fabs(dx) > 1e6f || std::fabs(dy) > 1e6f)
			return vec2(0, 0);
		vec2 g(dx, dy);
		float l = length(g);
		return l > 1e-6f ? g / l : vec2(0, 0);
	}
};

#endif
