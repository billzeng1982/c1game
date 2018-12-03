# Host: 192.168.1.10  (Version: 5.6.14)
# Date: 2014-01-06 13:18:21
# Generator: MySQL-Front 5.3  (Build 4.72)

/*!40101 SET NAMES utf8 */;

#
# Structure for table "tbl_mail"
#

DROP TABLE IF EXISTS `tbl_mail_1`;
CREATE TABLE `tbl_mail_1` (
  `Uin` bigint(20) unsigned NOT NULL,
  `MailBoxBlob` blob,
  PRIMARY KEY (`Uin`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
DROP TABLE IF EXISTS `tbl_mail_2`;
CREATE TABLE `tbl_mail_2` (
  `Uin` bigint(20) unsigned NOT NULL,
  `MailBoxBlob` blob,
  PRIMARY KEY (`Uin`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
DROP TABLE IF EXISTS `tbl_mail_3`;
CREATE TABLE `tbl_mail_3` (
  `Uin` bigint(20) unsigned NOT NULL,
  `MailBoxBlob` blob,
  PRIMARY KEY (`Uin`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
