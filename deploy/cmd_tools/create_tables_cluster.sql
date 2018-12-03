/*!40101 SET NAMES utf8 */;

#
# Structure for table "tbl_cluster_account_1"
#

CREATE TABLE `tbl_cluster_account_1` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `Uin` bigint(10) DEFAULT '0',
  `SdkUserName` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `RoleName` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  `ServerID` int(10) DEFAULT '0',
  `ChannelID` varchar(64) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  `CreateTime` varchar(64) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `Uin` (`Uin`),
  INDEX `inx_SdkUserName`(`SdkUserName`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

CREATE TABLE `tbl_cluster_account_2` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `Uin` bigint(10) DEFAULT '0',
  `SdkUserName` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `RoleName` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  `ServerID` int(10) DEFAULT '0',
  `ChannelID` varchar(64) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  `CreateTime` varchar(64) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `Uin` (`Uin`),
  INDEX `inx_SdkUserName`(`SdkUserName`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

CREATE TABLE `tbl_cluster_account_3` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `Uin` bigint(10) DEFAULT '0',
  `SdkUserName` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `RoleName` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  `ServerID` int(10) DEFAULT '0',
  `ChannelID` varchar(64) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  `CreateTime` varchar(64) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `Uin` (`Uin`),
  INDEX `inx_SdkUserName`(`SdkUserName`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;


