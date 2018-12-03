
## 公会远征 表结构

use c1game;
CREATE TABLE IF NOT EXISTS `tbl_guild_expedition_guild` (
  `GuildId` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `GuildBlob` mediumblob,
  PRIMARY KEY (`GuildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_guild_expedition_player` (
  `Uin` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `PlayerBlob` mediumblob,
  PRIMARY KEY (`Uin`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;