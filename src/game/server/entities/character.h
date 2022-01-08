/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.				*/
#ifndef GAME_SERVER_ENTITIES_CHARACTER_H
#define GAME_SERVER_ENTITIES_CHARACTER_H

#include <game/server/entity.h>
#include <game/server/infclass/entities/hero-flag.h>
#include <game/generated/server_data.h>
#include <game/generated/protocol.h>

#include <game/gamecore.h>

class CPortal;

enum
{
	WEAPON_GAME = -3, // team switching etc
	WEAPON_SELF = -2, // console kill command
	WEAPON_WORLD = -1, // death tiles etc
};

/* INFECTION MODIFICATION START ***************************************/
enum FREEZEREASON
{
	FREEZEREASON_FLASH = 0,
	FREEZEREASON_UNDEAD = 1,
	FREEZEREASON_INFECTION = 2,
};

#define GHOST_RADIUS 11
#define GHOST_SEARCHMAP_SIZE (2*GHOST_RADIUS+1)

enum TAKEDAMAGEMODE
{
	TAKEDAMAGEMODE_NOINFECTION=0,
	TAKEDAMAGEMODE_INFECTION,
	TAKEDAMAGEMODE_SELFHARM, // works like NOINFECTION but also harms the owner of the damage with 50%
};

/* INFECTION MODIFICATION END *****************************************/

class CCharacter : public CEntity
{
	class IConsole *m_pConsole;

	MACRO_ALLOC_POOL_ID()

public:
	class IConsole *Console() { return m_pConsole; }

	//character's size
	static const int ms_PhysSize = 28;

	CCharacter(CGameWorld *pWorld, IConsole *pConsole);
	~CCharacter() override;

	virtual void Reset();
	virtual void Destroy();
	virtual void Tick();
	virtual void TickDefered();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	bool IsGrounded();

	void SetWeapon(int W);
	void HandleWeaponSwitch();
	void DoWeaponSwitch();

	void HandleWeapons();
	void HandleNinja();
	void HandleWaterJump();

	void OnPredictedInput(CNetObj_PlayerInput *pNewInput);
	void OnDirectInput(CNetObj_PlayerInput *pNewInput);
	void ResetInput();
	virtual void FireWeapon();

	bool HasPortal();

	virtual void Die(int Killer, int Weapon);
	bool TakeDamage(vec2 Force, int Dmg, int From, int Weapon, TAKEDAMAGEMODE Mode);

	bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	bool Remove();

	bool IncreaseHealth(int Amount);
	bool IncreaseArmor(int Amount);
	bool IncreaseOverallHp(int Amount);
	void SetHealthArmor(int HealthAmount, int ArmorAmount);
	int GetHealthArmorSum();

	bool GiveWeapon(int Weapon, int Ammo);
	void SetActiveWeapon(int Weapon);

	void SetEmote(int Emote, int Tick);
	void NoAmmo();

	bool IsAlive() const { return m_Alive; }
	class CPlayer *GetPlayer() { return m_pPlayer; }

protected:
	// player controlling this character
	class CPlayer *m_pPlayer;

	bool m_Alive;

	// weapon info
	CEntity *m_apHitObjects[10];
	int m_NumObjectsHit;

	struct WeaponStat
	{
		int m_AmmoRegenStart;
		int m_Ammo;
		int m_Ammocost;
		bool m_Got;

	} m_aWeapons[NUM_WEAPONS];

	int m_ActiveWeapon;
	int m_LastWeapon;
	int m_QueuedWeapon;

	int m_ReloadTimer;
	int m_AttackTick;

	bool m_NeedFullHeal;
	int m_DamageTaken;

	int m_EmoteType;
	int m_EmoteStop;

	// last tick that the player took any action ie some input
	int m_LastAction;
	int m_LastNoAmmoSound;

public:
	// these are non-heldback inputs
	CNetObj_PlayerInput m_LatestPrevInput;
	CNetObj_PlayerInput m_LatestInput;

	// input
	CNetObj_PlayerInput m_PrevInput;
	CNetObj_PlayerInput m_Input;

protected:
	int m_NumInputs;
	int m_Jumped;

	int m_DamageTakenTick;

	int m_Health;
	int m_Armor;

/* INFECTION MODIFICATION START ***************************************/
	//Dart
public:
	int m_DartLifeSpan;
	vec2 m_DartDir;
	int m_DartLeft;
	int m_DartOldVelAmount;
	
	int m_WaterJumpLifeSpan;

	// the player core for the physics
	CCharacterCore m_Core;
	
private:
/* INFECTION MODIFICATION END *****************************************/

	// info for dead reckoning
	int m_ReckoningTick; // tick that we are performing dead reckoning From
	CCharacterCore m_SendCore; // core that we should send
	CCharacterCore m_ReckoningCore; // the dead reckoning core

/* INFECTION MODIFICATION START ***************************************/
protected:
	int m_AirJumpCounter;
	bool m_FirstShot;
	vec2 m_FirstShotCoord;
	int m_HookDmgTick;
	int m_InvisibleTick;
	bool m_IsInvisible;
	int m_HealTick;
	int m_BonusTick;
	int m_InfZoneTick;
	int m_ProtectionTick;
	
	int m_FlagID;
	int m_HeartID;
	int m_BarrierHintID;
	array<int> m_BarrierHintIDs;
	int m_CursorID;
	int m_AntiFireTime;
	
	bool m_IsFrozen;
	int m_FrozenTime;
	bool m_IsInSlowMotion; //LooperClass changes here
	FREEZEREASON m_FreezeReason;
	int m_InAirTick;
	
	char m_GhostSearchMap[GHOST_SEARCHMAP_SIZE*GHOST_SEARCHMAP_SIZE];
	
	vec2 m_SpawnPosition;

	CHeroFlag* m_pHeroFlag;
	bool m_canOpenPortals = false;
	CPortal *m_pPortalIn = nullptr;
	CPortal *m_pPortalOut = nullptr;

public:
	int m_PositionLockTick;
	bool m_PositionLocked;
	bool m_PositionLockAvailable;
	bool m_HasWhiteHole;
	bool m_HasIndicator;
	bool m_HasStunGrenade;
	int m_BroadcastWhiteHoleReady; // used to broadcast "WhiteHole ready" for a short period of time
	int m_LoveTick;
	int m_HallucinationTick;
	int m_SlipperyTick;
	int m_SlowMotionTick; //LooperClass changes here
	int m_LastFreezer;
	int m_HookMode;
	int m_InWater;
	int m_NinjaLevel = 0;
	int m_NinjaVelocityBuff;
	int m_NinjaStrengthBuff;
	int m_NinjaAmmoBuff;
	int m_NinjaAmmoRegenReduction = 0;
	int m_NinjaExtraDarts = 0;
	int m_NinjaExtraFlashRadius = 0;
	int m_RefreshTime;
	//Mercenary
	int m_TurretCount;
	int m_ResetKillsTime;

public:
	void DestroyChildEntities();
	void ClassSpawnAttributes();
	vec2 GetDirection() const;
	int GetPlayerClass() const;

	bool IsZombie() const;
	bool IsHuman() const;
	void RemoveAllGun();
	void SetAntiFire();
	void Freeze(float Time, int Player, FREEZEREASON Reason);
	bool IsFrozen() const;
	bool IsInSlowMotion() const; //LooperClass changes here
	void SlowMotionEffect(float duration);	//LooperClass changes here
	void Unfreeze();
	bool IsInLove() const;
	void LoveEffect();
	void HallucinationEffect();
	void SlipperyEffect();
	void GrantSpawnProtection();
	int GetInfWeaponID(int WID) const;
	bool FindPortalPosition(vec2 Pos, vec2& Res);
	bool FindWitchSpawnPosition(vec2& Res);
	void SaturateVelocity(vec2 Force, float MaxSpeed);
	bool HasPassenger() const;
	CCharacter *GetPassenger();
	int GetInfZoneTick();

/* INFECTION MODIFICATION END *****************************************/

	CCharacterCore *Core() { return &m_Core; }
	virtual void PreCoreTick() { }
	virtual void PostCoreTick() { }
};

#endif
