/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_PROJECTILE_H
#define GAME_SERVER_ENTITIES_PROJECTILE_H

#include <game/server/entity.h>

class CProjectile : public CEntity
{
public:
	CProjectile(CGameWorld *pGameWorld, int Type, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, bool Explosive, float Force, int SoundImpact, int Weapon, TAKEDAMAGEMODE TakeDamageMode = TAKEDAMAGEMODE_NOINFECTION);

	vec2 GetPos(float Time);
	void FillInfo(CNetObj_Projectile *pProj);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	int GetOwner() const;

private:
	vec2 m_Direction;
	int m_LifeSpan;
	int m_Owner;
	int m_Type;
	int m_Damage;
	int m_SoundImpact;
	int m_Weapon;
	float m_Force;
	int m_StartTick;
	bool m_Explosive;
	
/* INFECTION MODIFICATION START ***************************************/
	bool m_IsFlashGrenade;
	int m_FlashRadius = 0;
	vec2 m_StartPos;
	TAKEDAMAGEMODE m_TakeDamageMode;
	
public:
	void FlashGrenade();
	void SetFlashRadius(int Radius);
/* INFECTION MODIFICATION END *****************************************/
};

#endif
