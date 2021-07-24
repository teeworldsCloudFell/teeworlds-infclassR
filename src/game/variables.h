/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_VARIABLES_H
#define GAME_VARIABLES_H
#undef GAME_VARIABLES_H // this file will be included several times

// server
MACRO_CONFIG_INT(SvWarmup, sv_warmup, 0, 0, 0, CFGFLAG_SERVER, "Number of seconds to do warmup before round starts")
MACRO_CONFIG_STR(SvMotd, sv_motd, 900, "", CFGFLAG_SERVER, "Message of the day to display for the clients")
MACRO_CONFIG_STR(SvMaprotation, sv_maprotation, 768, "infc_lunaroutpost infc_skull infc_warehouse infc_damascus infc_eidalfitr infc_newdust infc_hardcorepit infc_normandie infc_deathdealer infc_bamboo3 infc_halfdust infc_warehouse2 infc_malinalli_k9f infc_canyon", CFGFLAG_SERVER, "Maps to rotate between")
MACRO_CONFIG_INT(SvRoundsPerMap, sv_rounds_per_map, 8, 1, 100, CFGFLAG_SERVER, "Number of rounds on each map before rotating")
MACRO_CONFIG_INT(SvScorelimit, sv_scorelimit, 0, 0, 1000, CFGFLAG_SERVER, "Score limit (0 disables)")
MACRO_CONFIG_INT(SvTimelimit, sv_timelimit, 0, 0, 1000, CFGFLAG_SERVER, "Time limit in minutes (0 disables)")
MACRO_CONFIG_STR(SvGametype, sv_gametype, 32, "mod", CFGFLAG_SERVER, "Game type (dm, tdm, ctf)")
MACRO_CONFIG_INT(SvTournamentMode, sv_tournament_mode, 0, 0, 1, CFGFLAG_SERVER, "Tournament mode. When enabled, players join the server as spectators")
MACRO_CONFIG_INT(SvSpamprotection, sv_spamprotection, 1, 0, 1, CFGFLAG_SERVER, "Spam protection")

MACRO_CONFIG_INT(SvSpectatorSlots, sv_spectator_slots, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Number of slots to reserve for spectators")
MACRO_CONFIG_INT(SvTeambalanceTime, sv_teambalance_time, 1, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before autobalancing teams")
MACRO_CONFIG_INT(SvInactiveKickTime, sv_inactivekick_time, 3, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before taking care of inactive players")
MACRO_CONFIG_INT(SvInactiveKick, sv_inactivekick, 1, 0, 2, CFGFLAG_SERVER, "How to deal with inactive players (0=move to spectator, 1=move to free spectator slot/kick, 2=kick)")

MACRO_CONFIG_INT(SvStrictSpectateMode, sv_strict_spectate_mode, 0, 0, 1, CFGFLAG_SERVER, "Restricts information in spectator mode")
MACRO_CONFIG_INT(SvVoteSpectate, sv_vote_spectate, 0, 0, 1, CFGFLAG_SERVER, "Allow voting to move players to spectators")
MACRO_CONFIG_INT(SvVoteSpectateRejoindelay, sv_vote_spectate_rejoindelay, 3, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before a player can rejoin after being moved to spectators by vote")
MACRO_CONFIG_INT(SvVoteKick, sv_vote_kick, 0, 0, 1, CFGFLAG_SERVER, "Allow voting to kick players")
MACRO_CONFIG_INT(SvVoteKickMin, sv_vote_kick_min, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Minimum number of players required to start a kick vote")
MACRO_CONFIG_INT(SvVoteKickBantime, sv_vote_kick_bantime, 5, 0, 1440, CFGFLAG_SERVER, "The time to ban a player if kicked by vote. 0 makes it just use kick")

MACRO_CONFIG_INT(SvMapUpdateRate, sv_mapupdaterate, 5, 1, 100, CFGFLAG_SERVER, "(Tw32) real id <-> vanilla id players map update rate")

MACRO_CONFIG_INT(SvSkinStealAction, sv_skinstealaction, 0, 0, 1, CFGFLAG_SERVER, "How to punish skin stealing (currently only 1 = force pinky)")

//Server Bots
MACRO_CONFIG_INT(SvBotSlots, sv_bot_slots, 2, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Number of slots to reserve for bots")
MACRO_CONFIG_INT(SvBotAllowHook, sv_bot_allow_hook, 1, 0, 1, CFGFLAG_SERVER, "Bots are allowed to hook")
MACRO_CONFIG_INT(SvBotAllowMove, sv_bot_allow_move, 1, 0, 1, CFGFLAG_SERVER, "Bots are allowed to move")
MACRO_CONFIG_INT(SvBotAllowFire, sv_bot_allow_fire, 1, 0, 1, CFGFLAG_SERVER, "Bots fire")
MACRO_CONFIG_INT(SvBotAccuracyError, sv_bot_accuracy, 1, 0, 1, CFGFLAG_SERVER, "Random accuracy error for bots")

MACRO_CONFIG_INT(SvBotAlwaysEnable, sv_bot_always_enable, 1, 0, 2, CFGFLAG_SERVER, "Make the bots stay(=1)/play(=2) even without clients")

MACRO_CONFIG_INT(SvBotSmoothPath, sv_bot_smooth_path, 1, 0, 10, CFGFLAG_SERVER, "Bot path smooth iterations")
MACRO_CONFIG_INT(SvBotDrawTarget, sv_bot_draw_target, 0, 0, 1, CFGFLAG_SERVER, "Show bot target")
MACRO_CONFIG_INT(SvBotEngineDrawGraph, sv_botengine_draw_graph, 0, 0, 1, CFGFLAG_SERVER, "Draw graph")

// debug
#ifdef CONF_DEBUG // this one can crash the server if not used correctly
	MACRO_CONFIG_INT(DbgDummies, dbg_dummies, 0, 0, 15, CFGFLAG_SERVER, "")
#endif
#endif
