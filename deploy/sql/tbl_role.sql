# Host: 192.168.1.10  (Version: 5.6.14)
# Date: 2014-01-06 13:18:21
# Generator: MySQL-Front 5.3  (Build 4.72)

/*!40101 SET NAMES utf8 */;

#
# Structure for table "tbl_role"
#

DROP TABLE IF EXISTS `tbl_role`;
CREATE TABLE `tbl_role` (
  `id` int(10) NOT NULL AUTO_INCREMENT COMMENT '记录的ID',
  `Uin` bigint(20) unsigned NOT NULL,
  `RoleName` varchar(32) DEFAULT NULL,
  `LastLoginTime` int(10) unsigned DEFAULT NULL,
  `Level` tinyint(3) unsigned DEFAULT NULL,
  `RoleInfoBlob` mediumblob,
  `ItemInfoBlob` mediumblob,
  `AchievesInfoBlob` mediumblob,
  `MountInfoBlob` mediumblob,
  `HeroInfoBlob` mediumblob,
  `PetInfoBlob` mediumblob,
  `WeaponInfoBlob` mediumblob,
  PRIMARY KEY (`id`),
  UNIQUE KEY `Uin` (`Uin`),
  UNIQUE KEY `RoleName` (`RoleName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
