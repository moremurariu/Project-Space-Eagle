#define private public
#define protected public
#include <game/client/prediction/entities/character.h>
#include <game/client/prediction/entities/projectile.h>
#include <game/client/prediction/gameworld.h>
#undef private
#undef protected

#include "nade.h"

#include <game/collision.h>
#include <game/mapitems.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

CTeamsCore CNadeG::ms_Teams;

static std::vector<uint8_t> gs_vSolid; // per tile: solid for projectiles (CheckPoint)
static std::vector<uint8_t> gs_vFreeze;
static std::vector<vec2> gs_vGrenades;
static int gs_W, gs_H;

int MapW() { return gs_W; }
int MapH() { return gs_H; }

void CNadeG::Init()
{
	const SMapInfo &M = CTasGame::Map();
	gs_W = M.m_W;
	gs_H = M.m_H;
	gs_vSolid.assign(gs_W * gs_H, 0);
	gs_vFreeze.assign(gs_W * gs_H, 0);
	gs_vGrenades.clear();
	for(int y = 0; y < gs_H; y++)
		for(int x = 0; x < gs_W; x++)
		{
			int T = M.Tile(x, y), F = M.Front(x, y);
			if(T == TILE_SOLID || T == TILE_NOHOOK)
				gs_vSolid[y * gs_W + x] = 1;
			if(T == TILE_FREEZE || F == TILE_FREEZE)
				gs_vFreeze[y * gs_W + x] = 1;
			if(T == ENTITY_OFFSET + ENTITY_WEAPON_GRENADE)
				gs_vGrenades.emplace_back(x * 32.0f + 16.0f, y * 32.0f + 16.0f);
			if(T != 0 && T != TILE_SOLID && T != TILE_NOHOOK && T != TILE_FREEZE && T < ENTITY_OFFSET)
			{
				std::printf("CNadeG: unsupported tile %d at %d,%d\n", T, x, y);
				std::exit(1);
			}
		}
	if(!M.m_vFront.empty())
	{
		std::printf("CNadeG: maps with a front layer are not supported\n");
		std::exit(1);
	}
}

bool AnySolid(float x0, float y0, float x1, float y1)
{
	// tiles that CheckPoint(round(x), round(y)) can see for points in the box (clamped like CCollision::GetTile)
	int tx0 = std::clamp((int)std::floor(x0 - 1) / 32 - 1, 0, gs_W - 1), tx1 = std::clamp((int)std::ceil(x1 + 1) / 32 + 1, 0, gs_W - 1);
	int ty0 = std::clamp((int)std::floor(y0 - 1) / 32 - 1, 0, gs_H - 1), ty1 = std::clamp((int)std::ceil(y1 + 1) / 32 + 1, 0, gs_H - 1);
	for(int y = ty0; y <= ty1; y++)
		for(int x = tx0; x <= tx1; x++)
			if(gs_vSolid[y * gs_W + x])
				return true;
	return false;
}

static inline int IntersectLineFast(vec2 Pos0, vec2 Pos1, vec2 *pCol, vec2 *pBefore)
{
	if(!AnySolid(std::min(Pos0.x, Pos1.x), std::min(Pos0.y, Pos1.y), std::max(Pos0.x, Pos1.x), std::max(Pos0.y, Pos1.y)))
	{
		*pCol = Pos1;
		*pBefore = Pos1;
		return 0;
	}
	return CTasGame::Collision()->IntersectLine(Pos0, Pos1, pCol, pBefore);
}

void CNadeG::FromGame(const CTasGame &G)
{
	CCharacter *pChr = G.Chr();
	m_Core = pChr->m_Core;
	m_Core.SetCoreWorld(nullptr, CTasGame::Collision(), &ms_Teams);
	m_Pos = pChr->m_Pos;
	m_PrevPos = pChr->m_PrevPos;
	m_Tick = G.m_Tick;
	m_Dead = G.Frozen();
	m_Bad = false;
	m_Fire = G.m_Fire;
	m_ReloadTimer = pChr->m_ReloadTimer;
	m_QueuedWeapon = pChr->m_QueuedWeapon;
	m_NumInputs = pChr->m_NumInputs;
	m_Input = pChr->m_Input;
	m_LatestInput = pChr->m_LatestInput;
	m_LatestPrevInput = pChr->m_LatestPrevInput;
	m_NumProj = 0;
	for(CEntity *pEnt = pChr->GameWorld()->FindFirst(CGameWorld::ENTTYPE_PROJECTILE); pEnt; pEnt = pEnt->TypeNext())
	{
		CProjectile *pProj = (CProjectile *)pEnt;
		if(pProj->m_MarkedForDestroy)
			continue;
		if(pProj->m_Type != WEAPON_GRENADE || !pProj->m_Explosive || pProj->m_Bouncing || pProj->m_Freeze || m_NumProj >= MAX_PROJ)
		{
			m_Bad = true;
			continue;
		}
		SNadeProj &P = m_aProj[m_NumProj++];
		P.m_Pos = pProj->m_Pos;
		P.m_Dir = pProj->m_Direction;
		P.m_StartTick = pProj->m_StartTick;
		P.m_LifeSpan = pProj->m_LifeSpan;
	}
}

bool CNadeG::Grounded() const
{
	return CTasGame::Collision()->IsOnGround(m_Core.m_Pos, CCharacterCore::PhysicalSize());
}

void CNadeG::DoWeaponSwitch()
{
	if(m_ReloadTimer != 0 || m_QueuedWeapon == -1)
		return;
	if(m_Core.m_aWeapons[WEAPON_NINJA].m_Got || !m_Core.m_aWeapons[m_QueuedWeapon].m_Got)
		return;
	if(m_QueuedWeapon == m_Core.m_ActiveWeapon)
		return;
	int W = m_QueuedWeapon;
	m_QueuedWeapon = -1;
	m_Core.m_ActiveWeapon = (W < WEAPON_HAMMER || W >= NUM_WEAPONS) ? -1 : W;
}

void CNadeG::HandleWeaponSwitch()
{
	if(m_NumInputs < 2)
		return;
	int WantedWeapon = m_Core.m_ActiveWeapon;
	if(m_QueuedWeapon != -1)
		WantedWeapon = m_QueuedWeapon;
	bool Anything = false;
	for(int i = 0; i < NUM_WEAPONS - 1; ++i)
		if(m_Core.m_aWeapons[i].m_Got)
			Anything = true;
	if(!Anything)
		return;
	if(m_LatestInput.m_WantedWeapon)
		WantedWeapon = m_Input.m_WantedWeapon - 1;
	if(WantedWeapon >= 0 && WantedWeapon < NUM_WEAPONS && WantedWeapon != m_Core.m_ActiveWeapon && m_Core.m_aWeapons[WantedWeapon].m_Got)
		m_QueuedWeapon = WantedWeapon;
	DoWeaponSwitch();
}

void CNadeG::FireWeapon(int GameTick)
{
	if(m_NumInputs < 2)
		return;
	if(m_ReloadTimer != 0)
		return;
	DoWeaponSwitch();
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
	const int W = m_Core.m_ActiveWeapon;
	bool FullAuto = W == WEAPON_GRENADE || W == WEAPON_SHOTGUN || W == WEAPON_LASER;
	bool WillFire = CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses;
	if(FullAuto && (m_LatestInput.m_Fire & 1) && W >= 0 && m_Core.m_aWeapons[W].m_Ammo)
		WillFire = true;
	if(!WillFire)
		return;
	if(W < 0 || !m_Core.m_aWeapons[W].m_Ammo)
		return;
	if(W != WEAPON_GRENADE)
	{
		m_Bad = true;
		return;
	}
	vec2 ProjStartPos = m_Pos + Direction * CCharacterCore::PhysicalSize() * 0.75f;
	if(m_NumProj >= MAX_PROJ)
	{
		m_Bad = true;
		return;
	}
	for(int i = m_NumProj; i > 0; i--)
		m_aProj[i] = m_aProj[i - 1];
	m_NumProj++;
	SNadeProj &P = m_aProj[0];
	P.m_Pos = ProjStartPos;
	P.m_Dir = Direction;
	P.m_StartTick = GameTick;
	P.m_LifeSpan = (int)(SERVER_TICK_SPEED * m_Core.m_Tuning.m_GrenadeLifetime);
	m_ReloadTimer = m_Core.m_Tuning.GetWeaponFireDelay(W) * SERVER_TICK_SPEED;
	m_LastFireStep = m_Tick;
}

vec2 ExplosionKick(vec2 TeePos, vec2 E)
{
	const float Radius = 135.0f, InnerRadius = 48.0f;
	if(!(distance(TeePos, E) < Radius + CCharacterCore::PhysicalSize()))
		return vec2(0, 0);
	vec2 Diff = TeePos - E;
	vec2 ForceDir(0, 1);
	float l = length(Diff);
	if(l)
		ForceDir = normalize(Diff);
	l = 1 - std::clamp((l - InnerRadius) / (Radius - InnerRadius), 0.0f, 1.0f);
	float Dmg = 6.0f * l; // explosion_strength (default tuning)
	if(!(int)Dmg)
		return vec2(0, 0);
	return ForceDir * Dmg * 2;
}

void CNadeG::Explode(vec2 Pos, bool Lifetime)
{
	const float Radius = 135.0f, InnerRadius = 48.0f;
	if(!(distance(m_Pos, Pos) < Radius + CCharacterCore::PhysicalSize()))
	{
		if(m_pLog)
			m_pLog->push_back({m_Tick, Pos, m_Pos, m_Core.m_Vel, vec2(0, 0), distance(m_Pos, Pos), Lifetime});
		return;
	}
	vec2 Diff = m_Pos - Pos;
	vec2 ForceDir(0, 1);
	float l = length(Diff);
	if(l)
		ForceDir = normalize(Diff);
	l = 1 - std::clamp((l - InnerRadius) / (Radius - InnerRadius), 0.0f, 1.0f);
	float Strength = m_Core.m_Tuning.m_ExplosionStrength;
	float Dmg = Strength * l;
	if(m_pLog)
		m_pLog->push_back({m_Tick, Pos, m_Pos, m_Core.m_Vel, (int)Dmg ? ForceDir * Dmg * 2 : vec2(0, 0), length(Diff), Lifetime});
	if((int)Dmg)
		m_Core.m_Vel = m_Core.m_Vel + ForceDir * Dmg * 2;
}

static inline bool Clipped(vec2 P)
{
	return round_to_int(P.x) / 32 < -200 || round_to_int(P.x) / 32 > gs_W + 200 || round_to_int(P.y) / 32 < -200 || round_to_int(P.y) / 32 > gs_H + 200;
}

void CNadeG::TickProjectiles()
{
	const float Curvature = m_Core.m_Tuning.m_GrenadeCurvature, Speed = m_Core.m_Tuning.m_GrenadeSpeed;
	int Keep = 0;
	SNadeProj aKeep[MAX_PROJ];
	for(int i = 0; i < m_NumProj; i++)
	{
		SNadeProj &P = m_aProj[i];
		float Pt = (m_Tick - P.m_StartTick - 1) / (float)SERVER_TICK_SPEED;
		float Ct = (m_Tick - P.m_StartTick) / (float)SERVER_TICK_SPEED;
		vec2 PrevPos = CalcPos(P.m_Pos, P.m_Dir, Curvature, Speed, Pt);
		vec2 CurPos = CalcPos(P.m_Pos, P.m_Dir, Curvature, Speed, Ct);
		vec2 ColPos, NewPos;
		int Collide = IntersectLineFast(PrevPos, CurPos, &ColPos, &NewPos);
		if(P.m_LifeSpan > -1)
			P.m_LifeSpan--;
		if(Collide || Clipped(CurPos))
		{
			// server: CProjectile::Tick explodes once and returns (no lifetime explosion on top)
			if(P.m_LifeSpan == -1)
				m_DoubleRisk++;
			Explode(ColPos, false);
			continue;
		}
		if(P.m_LifeSpan == -1)
		{
			Explode(ColPos, true);
			continue;
		}
		aKeep[Keep++] = P;
	}
	for(int i = 0; i < Keep; i++)
		m_aProj[i] = aKeep[i];
	m_NumProj = Keep;
}

void CNadeG::Step(const STasInput &In)
{
	if((m_Fire & 1) != (In.m_Fire ? 1 : 0))
		m_Fire++;
	CNetObj_PlayerInput Input = {};
	Input.m_Direction = In.m_Dir;
	Input.m_TargetX = In.m_TX;
	Input.m_TargetY = In.m_TY;
	Input.m_Jump = In.m_Jump;
	Input.m_Fire = m_Fire;
	Input.m_Hook = In.m_Hook;
	Input.m_WantedWeapon = In.m_Weapon >= 0 ? In.m_Weapon + 1 : 0;
	Input.m_PlayerFlags = PLAYERFLAG_PLAYING;

	// OnDirectInput at the old tick
	m_NumInputs++;
	m_LatestPrevInput = m_LatestInput;
	m_LatestInput = Input;
	if(m_LatestInput.m_TargetX == 0 && m_LatestInput.m_TargetY == 0)
		m_LatestInput.m_TargetY = -1;
	if(m_NumInputs > 1)
	{
		HandleWeaponSwitch();
		FireWeapon(m_Tick);
	}
	m_LatestPrevInput = m_LatestInput;

	m_Tick++;
	m_Input = Input;
	if(m_Input.m_TargetX == 0 && m_Input.m_TargetY == 0)
		m_Input.m_TargetY = -1;

	// world tick: projectiles, then pickups, then the character
	if(m_NumProj)
		TickProjectiles();
	if(!m_Core.m_aWeapons[WEAPON_GRENADE].m_Got)
		for(const vec2 &G : gs_vGrenades)
			if(distance(G, m_Pos) < 20.0f + CCharacterCore::PhysicalSize())
			{
				m_Core.m_aWeapons[WEAPON_GRENADE].m_Got = true;
				m_Core.m_aWeapons[WEAPON_GRENADE].m_Ammo = -1;
			}

	m_Core.m_Input = m_Input;
	m_Core.Tick(true, true);
	if(m_ReloadTimer)
		m_ReloadTimer--;
	else
		FireWeapon(m_Tick);
	// DDRacePostCoreTick: jump rules
	if(m_Core.m_Jumps == -1 || m_Core.m_Jumps == 0)
		m_Core.m_Jumped |= 2;
	else if(m_Core.m_Jumps == 1 && m_Core.m_Jumped > 0)
		m_Core.m_Jumped |= 2;
	else if(m_Core.m_JumpedTotal < m_Core.m_Jumps - 1 && m_Core.m_Jumped > 1)
		m_Core.m_Jumped = 1;
	if((m_Core.m_Super || m_Core.m_EndlessJump) && m_Core.m_Jumped > 1)
		m_Core.m_Jumped = 1;
	m_PrevPos = m_Core.m_Pos;
	m_Core.Move();
	m_Core.Quantize();
	m_Pos = m_Core.m_Pos;
	int tx = std::clamp((int)m_Pos.x / 32, 0, gs_W - 1), ty = std::clamp((int)m_Pos.y / 32, 0, gs_H - 1);
	if(gs_vFreeze[ty * gs_W + tx])
		m_Dead = true;
}

// ---------------- grenade flight ----------------

vec2 AimDir(int TX, int TY)
{
	if(TX == 0 && TY == 0)
		TY = -1;
	return normalize(vec2(TX, TY));
}

SNadeFlight FlyGrenade(vec2 TeePos, vec2 Dir)
{
	const float Curvature = 7.0f, Speed = 1000.0f; // default grenade tuning
	vec2 Start = TeePos + Dir * CCharacterCore::PhysicalSize() * 0.75f;
	SNadeFlight F;
	int Life = 100;
	for(int k = 1; k <= 101; k++)
	{
		float Pt = (k - 1) / (float)SERVER_TICK_SPEED;
		float Ct = k / (float)SERVER_TICK_SPEED;
		vec2 PrevPos = CalcPos(Start, Dir, Curvature, Speed, Pt);
		vec2 CurPos = CalcPos(Start, Dir, Curvature, Speed, Ct);
		vec2 ColPos, NewPos;
		int Collide = IntersectLineFast(PrevPos, CurPos, &ColPos, &NewPos);
		Life--;
		if(Collide || Clipped(CurPos))
		{
			F.m_Tau = k;
			F.m_E = ColPos;
			F.m_Collide = Collide;
			return F;
		}
		if(Life == -1)
		{
			F.m_Tau = k;
			F.m_E = ColPos;
			F.m_Collide = false;
			return F;
		}
	}
	return F;
}

static void ToIntAim(vec2 d, int &TX, int &TY)
{
	float m = std::max(std::fabs(d.x), std::fabs(d.y));
	float s = 30000.0f / m;
	TX = round_to_int(d.x * s);
	TY = round_to_int(d.y * s);
}

bool AimPossible(vec2 P, int Tau, vec2 Q, float Slack)
{
	vec2 C = P + vec2(0, 0.28f * Tau * Tau);
	float r = 21.0f + 20.0f * Tau;
	float reach = 60.0f + 20.0f + 0.56f * Tau + Slack;
	return std::fabs(distance(C, Q) - r) <= reach;
}

static bool gs_OnlyHits = false;
static void TryAim(vec2 P, int Tau, vec2 Q, vec2 d, bool AllowLast, vec2 Want, SAim &Best, float &BestScore)
{
	int TX, TY;
	ToIntAim(d, TX, TY);
	vec2 Dir = AimDir(TX, TY);
	SNadeFlight F = FlyGrenade(P, Dir);
	if(F.m_Tau != Tau)
		return;
	if(F.m_Collide && Tau == 101 && !AllowLast)
		return;
	if(gs_OnlyHits && !F.m_Collide)
		return;
	vec2 K = ExplosionKick(Q, F.m_E);
	float S = dot(K, Want);
	if(S > BestScore)
	{
		BestScore = S;
		Best.m_Ok = true;
		Best.m_TX = TX;
		Best.m_TY = TY;
		Best.m_F = F;
		Best.m_Kick = K;
	}
}

SAim SolveAim(vec2 P, int Tau, vec2 Q, bool AllowLast, vec2 Want, bool OnlyHits)
{
	gs_OnlyHits = OnlyHits;
	SAim Best;
	float BestScore = 0.0f;
	if(!AimPossible(P, Tau, Q, 40.0f))
		return Best;
	// 1) tile hits: aim at sample points on exposed tile faces near Q, entering during (Tau-1, Tau]
	const float Reach = 120.0f;
	int tx0 = std::clamp((int)((Q.x - Reach) / 32), 0, gs_W - 1), tx1 = std::clamp((int)((Q.x + Reach) / 32), 0, gs_W - 1);
	int ty0 = std::clamp((int)((Q.y - Reach) / 32), 0, gs_H - 1), ty1 = std::clamp((int)((Q.y + Reach) / 32), 0, gs_H - 1);
	struct STarget
	{
		vec2 m_X;
		float m_Score;
	};
	std::vector<STarget> vT;
	auto Solid = [&](int x, int y) { return x >= 0 && y >= 0 && x < gs_W && y < gs_H && gs_vSolid[y * gs_W + x]; };
	for(int ty = ty0; ty <= ty1; ty++)
		for(int tx = tx0; tx <= tx1; tx++)
		{
			if(!Solid(tx, ty))
				continue;
			// faces: top, bottom, left, right (exposed only); sample every 2 px, 0.5 px inside
			const float X0 = tx * 32.0f, Y0 = ty * 32.0f;
			for(int f = 0; f < 4; f++)
			{
				int nx = tx + (f == 2 ? -1 : f == 3 ? 1 : 0), ny = ty + (f == 0 ? -1 : f == 1 ? 1 : 0);
				if(Solid(nx, ny))
					continue;
				for(float s = 0.25f; s < 32.0f; s += 2.0f)
				{
					vec2 X;
					if(f == 0)
						X = vec2(X0 + s, Y0 + 0.25f);
					else if(f == 1)
						X = vec2(X0 + s, Y0 + 31.75f);
					else if(f == 2)
						X = vec2(X0 + 0.25f, Y0 + s);
					else
						X = vec2(X0 + 31.75f, Y0 + s);
					float Sc = dot(ExplosionKick(Q, X), Want);
					if(Sc > 0.5f)
						vT.push_back({X, Sc});
				}
			}
		}
	std::sort(vT.begin(), vT.end(), [](const STarget &a, const STarget &b) { return a.m_Score > b.m_Score; });
	int Tried = 0;
	for(const STarget &T : vT)
	{
		if(T.m_Score <= BestScore)
			break;
		if(Tried++ > 60)
			break;
		// find continuous times tc in [Tau-1.2, Tau+0.2] where the grenade passes through T
		auto Fn = [&](float tc) {
			vec2 D = T.m_X - P - vec2(0, 0.28f * tc * tc);
			return length(D) - (21.0f + 20.0f * tc);
		};
		const int N = 12;
		float a = Tau - 1.2f, fa = Fn(a);
		for(int i = 1; i <= N; i++)
		{
			float b = Tau - 1.2f + 1.4f * i / N, fb = Fn(b);
			if((fa <= 0) != (fb <= 0))
			{
				float lo = a, hi = b, flo = fa;
				for(int it = 0; it < 30; it++)
				{
					float m = 0.5f * (lo + hi), fm = Fn(m);
					if((fm <= 0) == (flo <= 0))
					{
						lo = m;
						flo = fm;
					}
					else
						hi = m;
				}
				float tc = 0.5f * (lo + hi);
				vec2 D = T.m_X - P - vec2(0, 0.28f * tc * tc);
				vec2 d = normalize(D);
				TryAim(P, Tau, Q, d, AllowLast, Want, Best, BestScore);
				// small perturbations (integer aim / chord effects)
				float ang = std::atan2(d.y, d.x);
				for(float e : {-4e-4f, -2e-4f, -1e-4f, 1e-4f, 2e-4f, 4e-4f})
					TryAim(P, Tau, Q, vec2(std::cos(ang + e), std::sin(ang + e)), AllowLast, Want, Best, BestScore);
			}
			a = b;
			fa = fb;
		}
	}
	// 2) lifetime explosion at Tau == 101 anywhere in the air
	if(Tau == 101)
	{
		vec2 C = P + vec2(0, 0.28f * 101 * 101);
		for(float dd : {20.0f, 30.0f, 40.0f, 48.0f, 60.0f})
		{
			vec2 Ideal = Q - Want * dd;
			vec2 d0 = normalize(Ideal - C);
			float ang0 = std::atan2(d0.y, d0.x);
			for(int i = -8; i <= 8; i++)
			{
				float ang = ang0 + i * 0.004f;
				TryAim(P, Tau, Q, vec2(std::cos(ang), std::sin(ang)), AllowLast, Want, Best, BestScore);
			}
		}
	}
	// 3) local refinement of the best aim angle (exact flights): maximise the kick along Want
	if(Best.m_Ok)
	{
		vec2 d0 = AimDir(Best.m_TX, Best.m_TY);
		float a0 = std::atan2(d0.y, d0.x);
		for(float Step = 2e-3f; Step > 2e-5f; Step *= 0.5f)
		{
			bool Improved = true;
			while(Improved)
			{
				Improved = false;
				for(float sgn : {-1.0f, 1.0f})
				{
					float a = a0 + sgn * Step;
					float Before = BestScore;
					TryAim(P, Tau, Q, vec2(std::cos(a), std::sin(a)), AllowLast, Want, Best, BestScore);
					if(BestScore > Before + 1e-6f)
					{
						vec2 d = AimDir(Best.m_TX, Best.m_TY);
						a0 = std::atan2(d.y, d.x);
						Improved = true;
					}
				}
			}
		}
	}
	return Best;
}

std::vector<STasInput> ReadInputFile(const char *pPath)
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

bool WriteInputFile(const char *pPath, const std::vector<STasInput> &v)
{
	FILE *f = std::fopen(pPath, "w");
	if(!f)
		return false;
	for(const STasInput &In : v)
		std::fprintf(f, "%d %d %d %d %d %d %d\n", In.m_Dir, In.m_Jump, In.m_Hook, In.m_Fire, In.m_TX, In.m_TY, In.m_Weapon);
	std::fclose(f);
	return true;
}
