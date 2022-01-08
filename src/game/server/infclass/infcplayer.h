#ifndef GAME_SERVER_INFCLASS_PLAYER_H
#define GAME_SERVER_INFCLASS_PLAYER_H

#include <game/gamecore.h>

class CGameContext;
class CInfClassCharacter;
class CInfClassGameController;
class CInfClassPlayerClass;

// We actually have to include player.h after all this stuff above.
#include <game/server/player.h>

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

	CInfClassCharacter *GetCharacter();
	CInfClassPlayerClass *GetCharacterClass() { return m_pInfcPlayerClass; }
	const CInfClassPlayerClass *GetCharacterClass() const { return m_pInfcPlayerClass; }
	void SetCharacterClass(CInfClassPlayerClass *pClass);

	void SetClass(int newClass) override;

	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const { return CPlayer::Server(); };

protected:
	const char *GetClan(int SnappingClient = -1) const override;

	CInfClassGameController *m_pGameController = nullptr;
	CInfClassPlayerClass *m_pInfcPlayerClass = nullptr;
};

#endif // GAME_SERVER_INFCLASS_PLAYER_H
