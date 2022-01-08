/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.				*/
#include <new>
#include <iostream>
#include <engine/shared/config.h>
#include "player.h"
#include <engine/shared/network.h>
#include <engine/server/roundstatistics.h>


MACRO_ALLOC_POOL_ID_IMPL(CPlayer, MAX_CLIENTS)

IServer *CPlayer::Server() const { return m_pGameServer->Server(); }

CPlayer::CPlayer(CGameContext *pGameServer, int ClientID, int Team)
{
	m_pGameServer = pGameServer;
	m_ClientID = ClientID;
	m_Team = GameServer()->m_pController->ClampTeam(Team);
	Reset();
}

CPlayer::~CPlayer()
{
	delete m_pCharacter;
	m_pCharacter = 0;
}

void CPlayer::Reset()
{
	m_RespawnTick = Server()->Tick();
	m_DieTick = Server()->Tick();
	m_ScoreStartTick = Server()->Tick();
	m_pCharacter = 0;
	m_SpectatorID = SPEC_FREEVIEW;
	m_LastActionTick = Server()->Tick();
	m_LastActionMoveTick = Server()->Tick();
	m_TeamChangeTick = Server()->Tick();
	m_ClientVersion = 0;
	m_LastWhisperId = -1;

	m_LastEyeEmote = 0;
	m_DefEmote = EMOTE_NORMAL;
	m_OverrideEmote = 0;
	m_OverrideEmoteReset = -1;

/* INFECTION MODIFICATION START ***************************************/
	m_Authed = IServer::AUTHED_NO;
	m_ScoreRound = 0;
	m_ScoreMode = PLAYERSCOREMODE_SCORE;
	m_WinAsHuman = 0;
	m_class = PLAYERCLASS_NONE;
	m_LastHumanClass = PLAYERCLASS_NONE;
	m_InfectionTick = -1;
	m_NumberKills = 0;
	SetLanguage(Server()->GetClientLanguage(m_ClientID));
	for(int i=0; i<NB_PLAYERCLASS; i++)
	{
		m_knownClass[i] = false;

		int* idMap = Server()->GetIdMap(m_ClientID);
		for (int i = 1;i < VANILLA_MAX_CLIENTS;i++)
		{
			idMap[i] = -1;
		}
		idMap[0] = m_ClientID;
	}

	m_MapMenu = 0;
	m_MapMenuItem = -1;
	m_MapMenuTick = -1;
	m_HookProtectionAutomatic = true;

	m_PrevTuningParams = *m_pGameServer->Tuning();
	m_NextTuningParams = m_PrevTuningParams;
	m_IsInGame = false;

	for(unsigned int i=0; i<sizeof(m_LastHumanClasses)/sizeof(int); i++)
		m_LastHumanClasses[i] = PLAYERCLASS_INVALID;

/* INFECTION MODIFICATION END *****************************************/
}

void CPlayer::Tick()
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	// do latency stuff
	{
		IServer::CClientInfo Info;
		if(Server()->GetClientInfo(m_ClientID, &Info))
		{
			m_Latency.m_Accum += Info.m_Latency;
			m_Latency.m_AccumMax = maximum(m_Latency.m_AccumMax, Info.m_Latency);
			m_Latency.m_AccumMin = minimum(m_Latency.m_AccumMin, Info.m_Latency);
		}
		// each second
		if(Server()->Tick()%Server()->TickSpeed() == 0)
		{
			m_Latency.m_Avg = m_Latency.m_Accum/Server()->TickSpeed();
			m_Latency.m_Max = m_Latency.m_AccumMax;
			m_Latency.m_Min = m_Latency.m_AccumMin;
			m_Latency.m_Accum = 0;
			m_Latency.m_AccumMin = 1000;
			m_Latency.m_AccumMax = 0;
		}
	}

	if(!GameServer()->m_World.m_Paused)
	{
		if(!m_pCharacter && m_Team == TEAM_SPECTATORS && m_SpectatorID == SPEC_FREEVIEW)
			m_ViewPos -= vec2(clamp(m_ViewPos.x-m_LatestActivity.m_TargetX, -500.0f, 500.0f), clamp(m_ViewPos.y-m_LatestActivity.m_TargetY, -400.0f, 400.0f));

		if(!m_pCharacter && m_DieTick+Server()->TickSpeed()*3 <= Server()->Tick())
			m_Spawning = true;

		if(m_pCharacter)
		{
			if(m_pCharacter->IsAlive())
			{
				m_ViewPos = m_pCharacter->m_Pos;
			}
			else
			{
				delete m_pCharacter;
				m_pCharacter = 0;
			}
		}
		else if(m_Spawning && m_RespawnTick <= Server()->Tick())
			TryRespawn();
	}
	else
	{
		++m_RespawnTick;
		++m_DieTick;
		++m_ScoreStartTick;
		++m_LastActionTick;
		++m_LastActionMoveTick;
		++m_TeamChangeTick;
 	}
}

void CPlayer::PostTick()
{
	// update latency value
	if(m_PlayerFlags&PLAYERFLAG_SCOREBOARD)
	{
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
				m_aActLatency[i] = GameServer()->m_apPlayers[i]->m_Latency.m_Min;
		}
	}

	// update view pos for spectators
	if(m_Team == TEAM_SPECTATORS && m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[m_SpectatorID])
		m_ViewPos = GameServer()->m_apPlayers[m_SpectatorID]->m_ViewPos;
}

void CPlayer::HandleTuningParams()
{
	if(!(m_PrevTuningParams == m_NextTuningParams))
	{
		if(m_IsReady)
		{
			CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
			int *pParams = (int *)&m_NextTuningParams;
			for(unsigned i = 0; i < sizeof(m_NextTuningParams)/sizeof(int); i++)
				Msg.AddInt(pParams[i]);
			Server()->SendMsg(&Msg, MSGFLAG_VITAL, GetCID());
		}
		
		m_PrevTuningParams = m_NextTuningParams;
	}
	
	m_NextTuningParams = *GameServer()->Tuning();
}

void CPlayer::HookProtection(bool Value, bool Automatic)
{
	if(m_HookProtection != Value)
	{
		m_HookProtection = Value;
		
		if(!m_HookProtectionAutomatic || !Automatic)
		{
			if(m_HookProtection)
				GameServer()->SendChatTarget_Localization(GetCID(), CHATCATEGORY_DEFAULT, _("Hook protection enabled"), NULL);
			else
				GameServer()->SendChatTarget_Localization(GetCID(), CHATCATEGORY_DEFAULT, _("Hook protection disabled"), NULL);
		}
	}
	
	m_HookProtectionAutomatic = Automatic;
}

void CPlayer::Snap(int SnappingClient)
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	int id = m_ClientID;
	if(SnappingClient != DemoClientID && !Server()->Translate(id, SnappingClient))
		return;

	CNetObj_ClientInfo *pClientInfo = static_cast<CNetObj_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_CLIENTINFO, id, sizeof(CNetObj_ClientInfo)));

	if(!pClientInfo)
		return;

	StrToInts(&pClientInfo->m_Name0, 4, Server()->ClientName(m_ClientID));
	
	int SnapScoreMode = PLAYERSCOREMODE_SCORE;
	if(GameServer()->GetPlayer(SnappingClient))
	{
		SnapScoreMode = GameServer()->m_apPlayers[SnappingClient]->GetScoreMode();
	}
	
/* INFECTION MODIFICATION STRAT ***************************************/
	int PlayerInfoScore = 0;
	
	StrToInts(&pClientInfo->m_Clan0, 3, GetClan(SnappingClient));
	if(GetTeam() == TEAM_SPECTATORS)
	{
	}
	else
	{
		if(SnapScoreMode == PLAYERSCOREMODE_TIME)
		{
			PlayerInfoScore = m_HumanTime/Server()->TickSpeed();
		}
		else
		{
			PlayerInfoScore = Server()->RoundStatistics()->PlayerScore(m_ClientID);
		}
	}
	
	pClientInfo->m_Country = Server()->ClientCountry(m_ClientID);

	if(
		GameServer()->GetPlayer(SnappingClient) && IsHuman() &&
		(
			(Server()->GetClientCustomSkin(SnappingClient) == 1 && SnappingClient == GetCID()) ||
			(Server()->GetClientCustomSkin(SnappingClient) == 2)
		)
	)
	{
		StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_CustomSkinName);
	}
	else StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
	
	pClientInfo->m_UseCustomColor = m_TeeInfos.m_UseCustomColor;
	pClientInfo->m_ColorBody = m_TeeInfos.m_ColorBody;
	pClientInfo->m_ColorFeet = m_TeeInfos.m_ColorFeet;
/* INFECTION MODIFICATION END *****************************************/

	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, id, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_Latency = SnappingClient == DemoClientID ? m_Latency.m_Min : GameServer()->m_apPlayers[SnappingClient]->m_aActLatency[m_ClientID];
	pPlayerInfo->m_Local = 0;
	pPlayerInfo->m_ClientID = id;
/* INFECTION MODIFICATION START ***************************************/
	pPlayerInfo->m_Score = PlayerInfoScore;
/* INFECTION MODIFICATION END *****************************************/
	pPlayerInfo->m_Team = m_Team;

	if(m_ClientID == SnappingClient)
		pPlayerInfo->m_Local = 1;

	if(m_ClientID == SnappingClient && m_Team == TEAM_SPECTATORS)
	{
		CNetObj_SpectatorInfo *pSpectatorInfo = static_cast<CNetObj_SpectatorInfo *>(Server()->SnapNewItem(NETOBJTYPE_SPECTATORINFO, m_ClientID, sizeof(CNetObj_SpectatorInfo)));
		if(!pSpectatorInfo)
			return;

		pSpectatorInfo->m_SpectatorID = m_SpectatorID;
		pSpectatorInfo->m_X = m_ViewPos.x;
		pSpectatorInfo->m_Y = m_ViewPos.y;
	}
}

void CPlayer::OnDisconnect(int Type, const char *pReason)
{
	KillCharacter();

	if(Server()->ClientIngame(m_ClientID))
	{
		if(Type == CLIENTDROPTYPE_BAN)
		{
			GameServer()->SendChatTarget_Localization(-1, CHATCATEGORY_PLAYER, _("{str:PlayerName} has been banned ({str:Reason})"),
				"PlayerName", Server()->ClientName(m_ClientID),
				"Reason", pReason,
				NULL);
		}
		else if(Type == CLIENTDROPTYPE_KICK)
		{
			GameServer()->SendChatTarget_Localization(-1, CHATCATEGORY_PLAYER, _("{str:PlayerName} has been kicked ({str:Reason})"),
				"PlayerName", Server()->ClientName(m_ClientID),
				"Reason", pReason,
				NULL);
		}
		else if(pReason && *pReason)
		{
			GameServer()->SendChatTarget_Localization(-1, CHATCATEGORY_PLAYER, _("{str:PlayerName} has left the game ({str:Reason})"),
				"PlayerName", Server()->ClientName(m_ClientID),
				"Reason", pReason,
				NULL);
		}
		else
		{
			GameServer()->SendChatTarget_Localization(-1, CHATCATEGORY_PLAYER, _("{str:PlayerName} has left the game"),
				"PlayerName", Server()->ClientName(m_ClientID),
				NULL);
		}
	}
}

void CPlayer::OnPredictedInput(CNetObj_PlayerInput *NewInput)
{
	// skip the input if chat is active
	if((m_PlayerFlags&PLAYERFLAG_CHATTING) && (NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING))
		return;

	if(m_pCharacter)
		m_pCharacter->OnPredictedInput(NewInput);
}

void CPlayer::OnDirectInput(CNetObj_PlayerInput *NewInput)
{
	if(NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING)
	{
		// skip the input if chat is active
		if(m_PlayerFlags&PLAYERFLAG_CHATTING)
			return;

		// reset input
		if(m_pCharacter)
			m_pCharacter->ResetInput();

		m_PlayerFlags = NewInput->m_PlayerFlags;
 		return;
	}

	m_PlayerFlags = NewInput->m_PlayerFlags;

	if(m_pCharacter)
		m_pCharacter->OnDirectInput(NewInput);

	if(!m_pCharacter && m_Team != TEAM_SPECTATORS && (NewInput->m_Fire&1))
		m_Spawning = true;

	// check for activity
	if(NewInput->m_Direction || m_LatestActivity.m_TargetX != NewInput->m_TargetX ||
		m_LatestActivity.m_TargetY != NewInput->m_TargetY || NewInput->m_Jump ||
		NewInput->m_Fire&1 || NewInput->m_Hook)
	{
		m_LatestActivity.m_TargetX = NewInput->m_TargetX;
		m_LatestActivity.m_TargetY = NewInput->m_TargetY;
		m_LastActionTick = Server()->Tick();
		if (NewInput->m_Direction || NewInput->m_Jump || NewInput->m_Hook)
			m_LastActionMoveTick = Server()->Tick();
	}
}

CCharacter *CPlayer::GetCharacter()
{
	if(m_pCharacter && m_pCharacter->IsAlive())
		return m_pCharacter;
	return 0;
}

void CPlayer::KillCharacter(int Weapon)
{
	if(m_pCharacter)
	{
		m_pCharacter->Die(m_ClientID, Weapon);
		delete m_pCharacter;
		m_pCharacter = 0;
	}
}

void CPlayer::Respawn()
{
	if(m_Team != TEAM_SPECTATORS)
		m_Spawning = true;
}

void CPlayer::SetTeam(int Team, bool DoChatMsg)
{
	KillCharacter();

	m_Team = Team;
	m_LastActionTick = Server()->Tick();
	m_LastActionMoveTick = Server()->Tick();
	m_SpectatorID = SPEC_FREEVIEW;
	// we got to wait 0.5 secs before respawning
	m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;

	if(Team == TEAM_SPECTATORS)
	{
		// update spectator modes
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->m_SpectatorID == m_ClientID)
				GameServer()->m_apPlayers[i]->m_SpectatorID = SPEC_FREEVIEW;
		}
	}
}

void CPlayer::TryRespawn()
{
	vec2 SpawnPos;

/* INFECTION MODIFICATION START ***************************************/
	if(!GameServer()->m_pController->PreSpawn(this, &SpawnPos))
		return;
/* INFECTION MODIFICATION END *****************************************/

	m_Spawning = false;
	m_pCharacter = new(m_ClientID) CCharacter(&GameServer()->m_World, GameServer()->Console());
	m_pCharacter->Spawn(this, SpawnPos);
	if(GetClass() != PLAYERCLASS_NONE)
		GameServer()->CreatePlayerSpawn(SpawnPos);
}

int CPlayer::GetDefaultEmote() const
{
	if(m_OverrideEmoteReset >= 0)
		return m_OverrideEmote;

	return m_DefEmote;
}

void CPlayer::OverrideDefaultEmote(int Emote, int Tick)
{
	m_OverrideEmote = Emote;
	m_OverrideEmoteReset = Tick;
	m_LastEyeEmote = Server()->Tick();
}

bool CPlayer::CanOverrideDefaultEmote() const
{
	return true;
}

/* INFECTION MODIFICATION START ***************************************/
int CPlayer::GetClass() const
{
	return m_class;
}

int CPlayer::LastHumanClass() const
{
	if(m_LastHumanClass == PLAYERCLASS_NONE)
	{
		return PLAYERCLASS_MEDIC; // if old class was not set, it defaults to medic
	}

	return m_LastHumanClass;
}

void CPlayer::Infect(CPlayer *pInfectiousPlayer)
{
	StartInfection(/* force */ false, pInfectiousPlayer);
}

void CPlayer::StartInfection(bool force, CPlayer *pInfectiousPlayer)
{
	if(!force && IsZombie())
		return;
	
	m_DoInfection = true;
	m_InfectiousPlayerCID = pInfectiousPlayer ? pInfectiousPlayer->GetCID() : -1;
}

bool CPlayer::IsZombie() const
{
	return (m_class > END_HUMANCLASS);
}

bool CPlayer::IsActuallyZombie() const
{
	return IsZombie() && !IsSpectator();
}

bool CPlayer::IsHuman() const
{
	return !(m_class > END_HUMANCLASS);
}

bool CPlayer::IsSpectator() const
{
	return GetTeam() == TEAM_SPECTATORS;
}

bool CPlayer::IsKnownClass(int c)
{
	return m_knownClass[c];
}

int CPlayer::GetScoreMode()
{
	return m_ScoreMode;
}

void CPlayer::SetScoreMode(int Mode)
{
	m_ScoreMode = Mode;
}

int CPlayer::GetNumberKills()
{
	if( GetClass() == PLAYERCLASS_NONE )
		return 0;
	else
		return m_NumberKills;
}

void CPlayer::IncreaseNumberKills()
{
	m_NumberKills++;
}

void CPlayer::ResetNumberKills()
{
	m_NumberKills = 0;
}

const char *CPlayer::GetClan(int SnappingClient) const
{
	return Server()->ClientClan(m_ClientID);
}

const char* CPlayer::GetLanguage()
{
	return m_aLanguage;
}

void CPlayer::SetLanguage(const char* pLanguage)
{
	str_copy(m_aLanguage, pLanguage, sizeof(m_aLanguage));
}
void CPlayer::OpenMapMenu(int Menu)
{
	m_MapMenu = Menu;
	m_MapMenuTick = 0;
}

void CPlayer::CloseMapMenu()
{
	m_MapMenu = 0;
	m_MapMenuTick = -1;
}

bool CPlayer::MapMenuClickable()
{
	return (m_MapMenu > 0 && (m_MapMenuTick > Server()->TickSpeed()/2));
}

float CPlayer::GetGhoulPercent() const
{
	return clamp(m_GhoulLevel/static_cast<float>(g_Config.m_InfGhoulStomachSize), 0.0f, 1.0f);
}

void CPlayer::IncreaseGhoulLevel(int Diff)
{
	int NewGhoulLevel = m_GhoulLevel + Diff;
	if(NewGhoulLevel < 0)
		NewGhoulLevel = 0;
	if(NewGhoulLevel > g_Config.m_InfGhoulStomachSize)
		NewGhoulLevel = g_Config.m_InfGhoulStomachSize;
	
	m_GhoulLevel = NewGhoulLevel;
}

/* INFECTION MODIFICATION END *****************************************/
