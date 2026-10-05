// Geodesic distance field over the game layer (tile Dijkstra, 8-neighbour), used by lab's distinit/scorefile.
// (Reconstructed: the original distfield.h was not part of the saved snapshot.)
#ifndef TAS_DISTFIELD_H
#define TAS_DISTFIELD_H

#include "sim.h"

#include <game/mapitems.h>

#include <cmath>
#include <initializer_list>
#include <queue>
#include <vector>

inline int gs_DFStencil = 3;

struct SDistField
{
	int m_W = 0, m_H = 0;
	std::vector<float> m_vD; // pixels to the nearest target tile, through non-solid tiles

	static bool Solid(int T) { return T == TILE_SOLID || T == TILE_NOHOOK; }

	void Build(const SMapInfo &M, std::initializer_list<int> Targets)
	{
		m_W = M.m_W;
		m_H = M.m_H;
		m_vD.assign((size_t)m_W * m_H, 1e9f);
		using SItem = std::pair<float, int>;
		std::priority_queue<SItem, std::vector<SItem>, std::greater<SItem>> Q;
		for(int i = 0; i < m_W * m_H; i++)
			for(int T : Targets)
				if(M.m_vGame[i] == T)
				{
					m_vD[i] = 0;
					Q.push({0.0f, i});
				}
		const int aDx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
		const int aDy[8] = {0, 0, 1, -1, 1, -1, 1, -1};
		while(!Q.empty())
		{
			auto [D, i] = Q.top();
			Q.pop();
			if(D > m_vD[i])
				continue;
			int x = i % m_W, y = i / m_W;
			for(int k = 0; k < 8; k++)
			{
				int nx = x + aDx[k], ny = y + aDy[k];
				if(nx < 0 || ny < 0 || nx >= m_W || ny >= m_H || Solid(M.Tile(nx, ny)))
					continue;
				if(k >= 4 && (Solid(M.Tile(x + aDx[k], y)) || Solid(M.Tile(x, y + aDy[k]))))
					continue;
				float ND = D + (k < 4 ? 32.0f : 45.254834f);
				int j = ny * m_W + nx;
				if(ND < m_vD[j])
				{
					m_vD[j] = ND;
					Q.push({ND, j});
				}
			}
		}
	}
	float At(int x, int y) const
	{
		if(x < 0 || y < 0 || x >= m_W || y >= m_H)
			return 1e9f;
		return m_vD[y * m_W + x];
	}
	float Sample(vec2 P) const
	{
		float fx = P.x / 32.0f - 0.5f, fy = P.y / 32.0f - 0.5f;
		int x0 = (int)std::floor(fx), y0 = (int)std::floor(fy);
		float ax = fx - x0, ay = fy - y0;
		float aV[4] = {At(x0, y0), At(x0 + 1, y0), At(x0, y0 + 1), At(x0 + 1, y0 + 1)};
		float Best = 1e9f;
		for(float V : aV)
			Best = std::min(Best, V);
		for(float &V : aV)
			if(V > 1e8f)
				V = Best + 64.0f; // solid neighbours: extrapolate
		return (aV[0] * (1 - ax) + aV[1] * ax) * (1 - ay) + (aV[2] * (1 - ax) + aV[3] * ax) * ay;
	}
	vec2 Grad(vec2 P) const
	{
		const float h = 4.0f;
		vec2 G((Sample(P + vec2(h, 0)) - Sample(P - vec2(h, 0))) / (2 * h), (Sample(P + vec2(0, h)) - Sample(P - vec2(0, h))) / (2 * h));
		float L = length(G);
		return L > 1e-6f ? G / L : vec2(0, 0);
	}
};

#endif
