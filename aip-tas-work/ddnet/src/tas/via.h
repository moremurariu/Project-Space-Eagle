// Post-pickup viability: can the tee get from a state to a goal point (the climb out of the grenade pocket) without
// freezing, and how soon? Small beam search on CFast guided by a geodesic distance field (4 px grid over positions
// whose tee box is free of solid tiles and whose centre is not in freeze). Used by ddsearch to reject pickups that
// only work as a dive into the freeze floor, and by viacheck to report the arrival tick.
#ifndef TAS_VIA_H
#define TAS_VIA_H

#include "fast.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <queue>
#include <unordered_set>
#include <vector>

class CViaField
{
public:
	enum
	{
		CELL = 4
	};
	int m_W = 0, m_H = 0;
	vec2 m_Goal = vec2(0, 0);
	std::vector<float> m_vDist; // px to the goal along free cells, 1e9 if unreachable

	void Build(vec2 Goal)
	{
		const SMapInfo &M = CTasGame::Map();
		m_Goal = Goal;
		m_W = M.m_W * 32 / CELL;
		m_H = M.m_H * 32 / CELL;
		std::vector<uint8_t> vFree(m_W * m_H, 0);
		auto Solid = [&](int px, int py) {
			int T = M.Tile(px / 32, py / 32);
			return T == 1 || T == 3;
		};
		for(int y = 0; y < m_H; y++)
			for(int x = 0; x < m_W; x++)
			{
				int px = x * CELL + CELL / 2, py = y * CELL + CELL / 2;
				int T = M.Tile(px / 32, py / 32);
				if(T == 9 || T == 1 || T == 3)
					continue;
				// tee box corners (14 px) must be outside solid tiles
				if(Solid(px - 14, py - 14) || Solid(px + 14, py - 14) || Solid(px - 14, py + 14) || Solid(px + 14, py + 14))
					continue;
				vFree[y * m_W + x] = 1;
			}
		m_vDist.assign(m_W * m_H, 1e9f);
		typedef std::pair<float, int> P;
		std::priority_queue<P, std::vector<P>, std::greater<P>> Q;
		int gx = std::clamp((int)Goal.x / CELL, 0, m_W - 1), gy = std::clamp((int)Goal.y / CELL, 0, m_H - 1);
		m_vDist[gy * m_W + gx] = 0;
		Q.push({0.0f, gy * m_W + gx});
		static const int DX[8] = {1, -1, 0, 0, 1, 1, -1, -1}, DY[8] = {0, 0, 1, -1, 1, -1, 1, -1};
		while(!Q.empty())
		{
			auto [D, I] = Q.top();
			Q.pop();
			if(D > m_vDist[I])
				continue;
			int x = I % m_W, y = I / m_W;
			for(int k = 0; k < 8; k++)
			{
				int nx = x + DX[k], ny = y + DY[k];
				if(nx < 0 || ny < 0 || nx >= m_W || ny >= m_H)
					continue;
				int J = ny * m_W + nx;
				if(!vFree[J])
					continue;
				float ND = D + (k < 4 ? CELL : CELL * 1.41421356f);
				if(ND < m_vDist[J])
				{
					m_vDist[J] = ND;
					Q.push({ND, J});
				}
			}
		}
	}
	float Dist(vec2 P) const
	{
		int x = std::clamp((int)P.x / CELL, 0, m_W - 1), y = std::clamp((int)P.y / CELL, 0, m_H - 1);
		float D = m_vDist[y * m_W + x];
		if(D >= 1e9f)
		{
			// a position the field calls blocked (e.g. touching a wall): use the best neighbour
			for(int dy = -3; dy <= 3; dy++)
				for(int dx = -3; dx <= 3; dx++)
				{
					int nx = std::clamp(x + dx, 0, m_W - 1), ny = std::clamp(y + dy, 0, m_H - 1);
					D = std::min(D, m_vDist[ny * m_W + nx] + CELL * (std::abs(dx) + std::abs(dy)));
				}
		}
		return D;
	}
};

struct SViaState
{
	CFast m_F;
	int m_LastHook;
	int m_LastJump;
	float m_Score;
};

// Returns the number of ticks needed to come within R px of the field's goal (or -1 if no state of the beam gets there
// within MaxTicks without freezing).
inline int ViaTicks(const CViaField &Field, const CFast &Start, int LastHook, int LastJump, int MaxTicks, int Beam, float R)
{
	std::vector<std::pair<int, int>> vAims;
	for(int a = 0; a < 32; a++)
	{
		float Ang = (a + 0.5f) / 32 * 2 * pi;
		vAims.emplace_back((int)std::lround(std::cos(Ang) * 1000), (int)std::lround(std::sin(Ang) * 1000));
	}
	std::vector<SViaState> vBeam(1);
	vBeam[0].m_F = Start;
	vBeam[0].m_LastHook = LastHook;
	vBeam[0].m_LastJump = LastJump;
	if(distance(Start.m_Core.m_Pos, Field.m_Goal) < R)
		return 0;
	std::vector<SViaState> vNext;
	std::vector<STasInput> vIn;
	for(int t = 1; t <= MaxTicks; t++)
	{
		vNext.clear();
		std::unordered_set<uint64_t> Seen;
		for(const SViaState &S : vBeam)
		{
			const CCharacterCore &C = S.m_F.m_Core;
			vIn.clear();
			for(int Dir = -1; Dir <= 1; Dir++)
				for(int J = 0; J <= 1; J++)
				{
					if(J && (C.m_Jumped & 1))
						continue;
					STasInput In;
					In.m_Dir = Dir;
					In.m_Jump = J;
					In.m_Fire = 0;
					In.m_Weapon = -1;
					In.m_TX = 0;
					In.m_TY = -1;
					if(S.m_LastHook)
					{
						In.m_Hook = 1;
						vIn.push_back(In);
						In.m_Hook = 0;
						vIn.push_back(In);
					}
					else
					{
						In.m_Hook = 0;
						vIn.push_back(In);
						In.m_Hook = 1;
						for(auto &A : vAims)
						{
							In.m_TX = A.first;
							In.m_TY = A.second;
							vIn.push_back(In);
						}
					}
				}
			for(const STasInput &In : vIn)
			{
				SViaState N;
				N.m_F = S.m_F;
				N.m_F.Step(In);
				if(N.m_F.m_Dead)
					continue;
				if(distance(N.m_F.m_Core.m_Pos, Field.m_Goal) < R)
					return t;
				const CCharacterCore &NC = N.m_F.m_Core;
				uint64_t K = ((uint64_t)(int)(NC.m_Pos.x / 4) << 40) ^ ((uint64_t)(int)(NC.m_Pos.y / 4) << 20) ^ ((uint64_t)(int)(NC.m_Vel.x + 64) << 10) ^
					     (uint64_t)(int)(NC.m_Vel.y + 64) ^ ((uint64_t)NC.m_HookState << 60) ^ ((uint64_t)In.m_Hook << 58) ^ ((uint64_t)NC.m_Jumped << 56);
				if(!Seen.insert(K).second)
					continue;
				N.m_LastHook = In.m_Hook;
				N.m_LastJump = In.m_Jump;
				// geodesic distance after a short ballistic look-ahead (speed toward the goal counts)
				vec2 Ahead = NC.m_Pos + NC.m_Vel * 3.0f;
				N.m_Score = std::min(Field.Dist(Ahead) + 20.0f, Field.Dist(NC.m_Pos) + 60.0f);
				vNext.push_back(N);
			}
		}
		if(vNext.empty())
			return -1;
		int Keep = std::min((int)vNext.size(), Beam);
		std::partial_sort(vNext.begin(), vNext.begin() + Keep, vNext.end(), [](const SViaState &a, const SViaState &b) { return a.m_Score < b.m_Score; });
		vNext.resize(Keep);
		vBeam.swap(vNext);
	}
	return -1;
}

#endif
