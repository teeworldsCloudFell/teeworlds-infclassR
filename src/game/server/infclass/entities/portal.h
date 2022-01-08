/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_PORTAL_H
#define GAME_SERVER_ENTITIES_PORTAL_H

#include "infcentity.h"

class CPortal : public CInfCEntity
{
public:
	enum
	{
		NUM_HINT = 12,
		NUM_SIDE = 12,
		NUM_IDS = NUM_HINT + NUM_SIDE,
	};

	enum PortalType
	{
		Disconnected,
		In,
		Out,
	};
	CPortal(CGameContext *pGameContext, vec2 CenterPos, int Owner, PortalType Type);
	~CPortal() override;

	void Snap(int SnappingClient) override;
	void Reset() override;
	void TickPaused() override;
	void Tick() override;

	int GetNewEntitySound() const;

	PortalType GetPortalType() const;
	void ConnectPortal(CPortal *anotherPortal);
	void Disconnect();
	CPortal *GetAnotherPortal() const;
	float GetRadius() const { return m_Radius; }

	void TakeDamage(int Dmg, int From, int Weapon, int Mode);
	void Explode(int From);

protected:
	void StartParallelsVisualEffect();
	void StartMeridiansVisualEffect();
	void MoveParallelsParticles();
	void MoveMeridiansParticles();
	void TeleportCharacters();
	float GetSpeedMultiplier();
	void PrepareAntipingParticles(vec2 *Output);

protected:
	// visual
	const float m_ParticleStartSpeed = 1.1f;
	const float m_ParticleAcceleration = 1.01f;
	int m_ParticleStopTickTime; // when X time is left stop creating particles - close animation

	int m_IDs[NUM_IDS];
	vec2 m_ParticlePos[NUM_IDS];
	vec2 m_ParticleVec[NUM_HINT];
	float m_Radius = 0;
	float m_Angle = 0;

	PortalType m_PortalType = PortalType::Disconnected;
	CPortal *m_AnotherPortal = nullptr;

	int m_StartTick;
	int m_ConnectedTick = 0;
};

#endif // GAME_SERVER_ENTITIES_PORTAL_H
