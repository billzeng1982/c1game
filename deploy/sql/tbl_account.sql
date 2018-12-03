# Host: 192.168.1.10  (Version: 5.6.14)
# Date: 2014-01-06 13:17:59
# Generator: MySQL-Front 5.3  (Build 4.72)

/*!40101 SET NAMES utf8 */;

#
# Structure for table "tbl_account"
#

DROP TABLE IF EXISTS `tbl_account`;
CREATE TABLE `tbl_account` (
  `id` int(10) NOT NULL AUTO_INCREMENT COMMENT '记录的ID',
  `Uid` int(10) DEFAULT '0' COMMENT '渠道ID',
  `Uin` bigint(10) DEFAULT '0' COMMENT 'user identity number',
  `AccountName` varchar(32) NOT NULL DEFAULT '' COMMENT '账户名',
  `PassWord` varchar(32) NOT NULL DEFAULT '' COMMENT '密码',
  PRIMARY KEY (`id`),
  UNIQUE KEY `AccountName` (`AccountName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
