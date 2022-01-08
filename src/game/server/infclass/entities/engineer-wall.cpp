/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/vmath.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <game/server/infclass/classes/infcplayerclass.h>
#include <engine/server/roundstatistics.h>
#include <engine/shared/config.h>

#include "engineer-wall.h"
#include "infccharacter.h"

const float g_BarrierMaxLength = 300.0;
const float g_BarrierRadius = 0.0;

CEngineerWall::CEngineerWall(CGameContext *pGameContext, vec2 Pos1, vec2 Pos2, int Owner)
	: CInfCEntity(pGameContext, CGameWorld::ENTTYPE_ENGINEER_WALL, Pos1, Owner)
{
	if(distance(Pos1, Pos2) > g_BarrierMaxLength)
	{
		m_Pos2 = Pos1 + normalize(Pos2 - Pos1)*g_BarrierMaxLength;
	}
	else
	{
		m_Pos2 = Pos2;
	}
	m_LifeSpan = Server()->TickSpeed()*Config()->m_InfBarrierLifeSpan;
	GameWorld()->InsertEntity(this);
	m_EndPointID = Server()->SnapNewID();
	m_WallFlashTicks = 0;
}

CEngineerWall::~CEngineerWall()
{
	Server()->SnapFreeID(m_EndPointID);
}

void CEngineerWall::Tick()
{
	if(m_MarkedForDestroy) return;

	if (m_WallFlashTicks > 0) 
		m_WallFlashTicks--;

	m_LifeSpan--;
	
	if(m_LifeSpan < 0)
	{
		GameServer()->m_World.DestroyEntity(this);
	}
	else
	{
		// Find other players
		for(CInfClassCharacter *p = (CInfClassCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CInfClassCharacter *)p->TypeNext())
		{
			if(p->IsHuman()) continue;

			vec2 IntersectPos = closest_point_on_line(m_Pos, m_Pos2, p->m_Pos);
			float Len = distance(p->m_Pos, IntersectPos);
			if(Len < p->m_ProximityRadius+g_BarrierRadius)
			{
				OnZombieHit(p);
			}
		}
	}
}

void CEngineerWall::TickPaused()
{
	//~ ++m_EvalTick;
}

void CEngineerWall::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	// Laser dieing animation
	int LifeDiff = 0;
	if (m_WallFlashTicks > 0) // flash laser for a few ticks when zombie jumps
		LifeDiff = 5;
	else if (m_LifeSpan < 1*Server()->TickSpeed())
		LifeDiff = random_int(4, 5);
	else if (m_LifeSpan < 2*Server()->TickSpeed())
		LifeDiff = random_int(3, 5);
	else if (m_LifeSpan < 3*Server()->TickSpeed())
		LifeDiff = random_int(2, 4);
	else if (m_LifeSpan < 4*Server()->TickSpeed())
		LifeDiff = random_int(1, 3);
	else if (m_LifeSpan < 5*Server()->TickSpeed())
		LifeDiff = random_int(0, 2);
	else if (m_LifeSpan < 6*Server()->TickSpeed())
		LifeDiff = random_int(0, 1);
	else if (m_LifeSpan < 7*Server()->TickSpeed())
		LifeDiff = (random_prob(3.0f/4.0f)) ? 1 : 0;
	else if (m_LifeSpan < 8*Server()->TickSpeed())
		LifeDiff = (random_prob(5.0f/6.0f)) ? 1 : 0;
	else if (m_LifeSpan < 9*Server()->TickSpeed())
		LifeDiff = (random_prob(5.0f/6.0f)) ? 0 : -1;
	else if (m_LifeSpan < 10*Server()->TickSpeed())
		LifeDiff = (random_prob(5.0f/6.0f)) ? 0 : -1;
	else if (m_LifeSpan < 11*Server()->TickSpeed())
		LifeDiff = (random_prob(5.0f/6.0f)) ? -1 : -Server()->TickSpeed()*2;
	else
		LifeDiff = -Server()->TickSpeed()*2;
	
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		pObj->m_X = (int)m_Pos.x;
		pObj->m_Y = (int)m_Pos.y;
		pObj->m_FromX = (int)m_Pos2.x;
		pObj->m_FromY = (int)m_Pos2.y;
		pObj->m_StartTick = Server()->Tick()-LifeDiff;
	}
	if(!Server()->GetClientAntiPing(SnappingClient))
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_EndPointID, sizeof(CNetObj_Laser)));
		if(!pObj)
			return;
		
		vec2 Pos = m_Pos2;

		pObj->m_X = (int)Pos.x;
		pObj->m_Y = (int)Pos.y;
		pObj->m_FromX = (int)Pos.x;
		pObj->m_FromY = (int)Pos.y;
		pObj->m_StartTick = Server()->Tick();
	}
}

void CEngineerWall::OnZombieHit(CInfClassCharacter *pZombie)
{
	if(pZombie->GetPlayer())
	{
		if(pZombie->CanDie())
		{
			for(CInfClassCharacter *pHook = (CInfClassCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pHook; pHook = (CInfClassCharacter *)pHook->TypeNext())
			{
				if(
					pHook->GetPlayer() &&
					pHook->IsHuman() &&
					pHook->m_Core.m_HookedPlayer == pZombie->GetCID() &&
					pHook->GetCID() != m_Owner && //The engineer will get the point when the infected dies
					pZombie->m_LastFreezer != pHook->GetCID() //The ninja will get the point when the infected dies
				)
				{
					int ClientID = pHook->GetCID();
					Server()->RoundStatistics()->OnScoreEvent(ClientID, SCOREEVENT_HELP_HOOK_BARRIER, pHook->GetPlayerClass(), Server()->ClientName(ClientID), GameServer()->Console());
					GameServer()->SendScoreSound(pHook->GetCID());
				}
			}
		}

		if(pZombie->GetPlayerClass() != PLAYERCLASS_UNDEAD && pZombie->GetPlayerClass() != PLAYERCLASS_VOODOO)
		{
			int LifeSpanReducer = ((Server()->TickSpeed()*Config()->m_InfBarrierTimeReduce)/100);
			m_WallFlashTicks = 10;

			if(pZombie->GetPlayerClass() == PLAYERCLASS_GHOUL)
			{
				float Factor = pZombie->GetClass()->GetGhoulPercent();
				LifeSpanReducer += Server()->TickSpeed() * 5.0f * Factor;
			}

			m_LifeSpan -= LifeSpanReducer;
		}
	}

	pZombie->Die(m_Owner, WEAPON_HAMMER);
}
