#pragma once

#include "strutil.h"

#define ROLE_TABLE_NAME 				"tbl_role"
#define ACCOUNT_TABLE_NAME 			"tbl_account"
#define MAIL_PUB_TABLE_NAME 			"tbl_mail_pub"
#define MAIL_PRI_TABLE_NAME 			"tbl_mail_pri"
#define GUILD_TABLE_NAME 			"tbl_guild"
#define GUILD_PLAYER_TABLE_NAME 		"tbl_guildplayer"
#define MESSAGE_TABLE_NAME 			"tbl_message"
#define FRIEND_TABLE_NAME 			"tbl_friend"
#define ASYNCPVP_PLAYER_TABLE_NAME		"tbl_asyncpvp_player"
#define ASYNCPVP_TEAM_TABLE_NAME		"tbl_asyncpvp_team"
#define MINE_ORE_TABLE_NAME			"tbl_mine_ore"
#define MINE_PLAYER_TABLE_NAME		"tbl_mine_player"
#define CLONEBATTLE_TEAM_TABLE_NAME	"tbl_clonebattle_team"
#define CLUSTER_ACCOUNT_TABLE_NAME		"tbl_cluster_account"
#define GUILD_EXPEDITION_GUILD_TABLE	"tbl_guild_expedition_guild"
#define GUILD_EXPEDITION_PLAYER_TABLE	"tbl_guild_expedition_player"

#define SQL_ADD_DELIMITER( Sql, SqlSize ) StrCat( Sql, SqlSize, ", " )

