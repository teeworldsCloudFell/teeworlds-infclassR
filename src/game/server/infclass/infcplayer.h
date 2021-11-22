#ifndef GAME_SERVER_INFCLASS_PLAYER_H
#define GAME_SERVER_INFCLASS_PLAYER_H

#include <game/gamecore.h>

class CGameContext;
class CInfClassCharacter;
class CInfClassGameController;
class CInfClassPlayerClass;

// We actually have to include player.h after all this stuff above.
#include <game/server/player.h>

enum class DO_INFECTION
{
	NO,
	REGULAR,
	FORCED,
};

class CInfClassPlayer : public CPlayer
{
	MACRO_ALLOC_POOL_ID()

public:
	CInfClassPlayer(CInfClassGameController *pGameController, int ClientID, int Team);
	~CInfClassPlayer() override;

	CInfClassGameController *GameController();

	void TryRespawn() override;

	void Tick() override;
	int GetDefaultEmote() const override;

	void HandleInfection();

	CInfClassCharacter *GetCharacter();
	CInfClassPlayerClass *GetCharacterClass() { return m_pInfcPlayerClass; }
	const CInfClassPlayerClass *GetCharacterClass() const { return m_pInfcPlayerClass; }
	void SetCharacterClass(CInfClassPlayerClass *pClass);

	void SetClass(int newClass) override;

	void Infect(CPlayer* pInfectiousPlayer);
	void StartInfection(bool force = false, CPlayer* pInfectiousPlayer = nullptr);

	int MapMenu() { return (m_Team != TEAM_SPECTATORS) ? m_MapMenu : 0; };
	void OpenMapMenu(int Menu);
	void CloseMapMenu();
	bool MapMenuClickable();

	float GetGhoulPercent() const;
	void IncreaseGhoulLevel(int Diff);
	int GetGhoulLevel() const { return m_GhoulLevel; }

	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const { return CPlayer::Server(); };

public:
	int m_MapMenuItem = -1;

protected:
	const char *GetClan(int SnappingClient = -1) const override;

	CInfClassGameController *m_pGameController = nullptr;
	CInfClassPlayerClass *m_pInfcPlayerClass = nullptr;

	DO_INFECTION m_DoInfection = DO_INFECTION::NO;
	int m_InfectiousPlayerCID = -1;

	int m_MapMenu = 0;
	int m_MapMenuTick = -1;

	int m_GhoulLevel = 0;
	int m_GhoulLevelTick = 0;
};

template<int FLAGS>
class CInfClassPlayerIterator : public CPlayerIterator<FLAGS>
{
public:
	CInfClassPlayerIterator(CPlayer **ppPlayers) :
		CPlayerIterator<FLAGS>(ppPlayers)
	{
	}

	CInfClassPlayer *Player() { return static_cast<CInfClassPlayer *> (CPlayerIterator<FLAGS>::Player()); }
};

#endif // GAME_SERVER_INFCLASS_PLAYER_H
