
##特别注意字符集的选择
use c1game;

CREATE TABLE IF NOT EXISTS `tbl_account` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `Uin` bigint(10) DEFAULT '0',
  `AccountType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `AccountName` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `PassWord` varchar(32) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  `ChannelID` varchar(64) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  `CreateTime` varchar(64) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  `BanTime` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `AccountName` (`AccountName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;



CREATE TABLE IF NOT EXISTS `tbl_asyncpvp_player` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `Uin` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `DataBlob` blob,
  PRIMARY KEY (`id`),
  UNIQUE KEY `Uin` (`Uin`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_asyncpvp_team` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `Uin` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `TeamBlob` blob,
  PRIMARY KEY (`id`),
  UNIQUE KEY `Uin` (`Uin`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_clonebattle_team` (
  `Id` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `TeamBlob` blob,
  PRIMARY KEY (`Id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_friend` (
  `Uin` bigint(20) unsigned NOT NULL,
  `Name` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `ApplyBlob` mediumblob,
  `AgreeBlob` mediumblob,
  `SendBlob` mediumblob,
  `RecvBlob` mediumblob,
  `PlayerBlob` mediumblob,
  PRIMARY KEY (`Uin`),
  UNIQUE KEY `Name` (`Name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_guild` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `GuildId` bigint(20) unsigned NOT NULL,
  `Name` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `MemberBlob` mediumblob,
  `GlobalBlob` mediumblob,
  `ApplyBlob` mediumblob,
  `ReplayBlob` mediumblob,
  `BossBlob` mediumblob,
  `ScienceBlob` mediumblob,
  `DelFlag` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `GuildId` (`GuildId`),
  KEY `Name` (`Name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_guildplayer` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `Uin` bigint(20) unsigned NOT NULL,
  `GuildId` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `ApplyBlob` mediumblob,
  PRIMARY KEY (`id`),
  UNIQUE KEY `Uin` (`Uin`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_mail_pri` (
  `Uin` bigint(20) unsigned NOT NULL,
  `PriSeq` bigint(20) unsigned NOT NULL,
  `PubSeq` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `MailBoxPriBlob` mediumblob,
  PRIMARY KEY (`Uin`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_mail_pub` (
  `Id` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `DelFlag` tinyint(3) unsigned DEFAULT '0',
  `StartTime` bigint(20) unsigned DEFAULT NULL,
  `EndTime` bigint(20) unsigned DEFAULT NULL,
  `MailDataBlob` mediumblob,
  PRIMARY KEY (`Id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_message` (
  `Uin` bigint(20) unsigned NOT NULL,
  `Channel` tinyint(3) unsigned NOT NULL,
  `DelFlag` tinyint(3) unsigned DEFAULT '0',
  `Version` int(10) unsigned DEFAULT '0',
  `MessageBoxBlob` mediumblob,
  PRIMARY KEY (`Uin`,`Channel`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_mine_ore` (
  `Uid` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `OreBlob` blob,
  PRIMARY KEY (`Uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_mine_player` (
  `Uin` bigint(20) unsigned NOT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `PlayerBlob` blob,
  PRIMARY KEY (`Uin`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `tbl_role` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `Uin` bigint(20) unsigned NOT NULL,
  `RoleName` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `FirstLoginTime` int(10) unsigned DEFAULT NULL,
  `LastLoginTime` int(10) unsigned DEFAULT NULL,
  `BlackRoomTime` int(10) unsigned NOT NULL DEFAULT '0',
  `BagSeq` int(10) unsigned DEFAULT NULL,
  `Version` int(10) unsigned DEFAULT '0',
  `MajestyBlob` mediumblob,
  `EquipBlob` mediumblob,
  `GCardBlob` mediumblob,
  `PropsBlob` mediumblob,
  `ItemsBlob` mediumblob,
  `ELOBlob` mediumblob,
  `TaskBlob` mediumblob,
  `MSkillBlob` mediumblob,
  `EquipPageBlob` mediumblob,
  `PveBlob` mediumblob,
  `MiscBlob` mediumblob,
  `GuildBlob` mediumblob,
  `TacticsBlob` mediumblob,
  PRIMARY KEY (`id`),
  UNIQUE KEY `Uin` (`Uin`),
  KEY `RoleName` (`RoleName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
