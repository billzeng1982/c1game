CREATE TABLE IF NOT EXISTS `LevelAccumPassLog` (
 `PlayerID` BIGINT UNSIGNED NOT NULL  ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `LevelID` INT UNSIGNED   ,
 `FirstSweepTime` VARCHAR(32)   ,
 `Time2AccRegister` INT UNSIGNED   ,
 `AccumPassCnt` INT UNSIGNED   ,
 `AccumSweepCnt` INT UNSIGNED   ,
PRIMARY KEY(`PlayerID`))
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `LevelPassLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `LevelID` INT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `IsPass` TINYINT   ,
 `StartTime` VARCHAR(32)   ,
 `EndTime` VARCHAR(32)   ,
 `TimeCost` INT UNSIGNED   ,
 `MSkillUseNum` TINYINT   ,
 `PlayerLevel` SMALLINT UNSIGNED   ,
 `StarEvalResult` SMALLINT UNSIGNED   ,
 `General1` INT UNSIGNED   ,
 `General2` INT UNSIGNED   ,
 `General3` INT UNSIGNED   ,
 `General4` INT UNSIGNED   ,
 `General5` INT UNSIGNED   ,
 `General6` INT UNSIGNED   ,
 `MSkillID` INT UNSIGNED   ,
 `Time2AccRegister` INT UNSIGNED   ,
 `IsAutoBattle` TINYINT   ,
 `IsAccelerated` TINYINT   ,
 `MaxFramePerSecond` INT   ,
 `MinFramePerSecond` INT   ,
 `AverageFramePerSecond` INT   ,
 `ReservePara1` INT UNSIGNED   ,
 `ReservePara2` INT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `LevelSweepLog` (
 `DateTime` VARCHAR(32)   ,
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `LevelID` INT UNSIGNED   ,
 `SweepCnt` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `LotteryLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `Type` SMALLINT UNSIGNED   ,
 `ItemType` TINYINT   ,
 `ItemID` INT UNSIGNED   ,
 `ItemNum` INT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `LotteryCntLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `Type` TINYINT   ,
 `Item1Type` TINYINT   ,
 `Item1ID` INT UNSIGNED   ,
 `Item1Num` INT UNSIGNED   ,
 `Item2Type` TINYINT   ,
 `Item2ID` INT UNSIGNED   ,
 `Item2Num` INT UNSIGNED   ,
 `Item3Type` TINYINT   ,
 `Item3ID` INT UNSIGNED   ,
 `Item3Num` INT UNSIGNED   ,
 `Item4Type` TINYINT   ,
 `Item4ID` INT UNSIGNED   ,
 `Item4Num` INT UNSIGNED   ,
 `Item5Type` TINYINT   ,
 `Item5ID` INT UNSIGNED   ,
 `Item5Num` INT UNSIGNED   ,
 `Item6Type` TINYINT   ,
 `Item6ID` INT UNSIGNED   ,
 `Item6Num` INT UNSIGNED   ,
 `Item7Type` TINYINT   ,
 `Item7ID` INT UNSIGNED   ,
 `Item7Num` INT UNSIGNED   ,
 `Item8Type` TINYINT   ,
 `Item8ID` INT UNSIGNED   ,
 `Item8Num` INT UNSIGNED   ,
 `Item9Type` TINYINT   ,
 `Item9ID` INT UNSIGNED   ,
 `Item9Num` INT UNSIGNED   ,
 `Item10Type` TINYINT   ,
 `Item10ID` INT UNSIGNED   ,
 `Item10Num` INT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `AccountLoginLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `LoginTime` VARCHAR(32)   ,
 `ChannelName` VARCHAR(40)   ,
 `PhoneType` VARCHAR(128)   ,
 `DaySinceReg` SMALLINT UNSIGNED   ,
 `DaySinceSvrOpen` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `AccountLogoutLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `LoginTime` VARCHAR(32)   ,
 `LogoutTime` VARCHAR(32)   ,
 `ChannelName` VARCHAR(40)   ,
 `PhoneType` VARCHAR(128)   ,
 `LogoutReason` SMALLINT UNSIGNED   ,
 `DaySinceReg` SMALLINT UNSIGNED   ,
 `DaySinceSvrOpen` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `CreateNewLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `CreateTime` VARCHAR(32)   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `DiamondLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `ChgType` TINYINT   ,
 `ChgValue` INT UNSIGNED   ,
 `CurValue` INT UNSIGNED   ,
 `Approach` SMALLINT UNSIGNED   ,
 `DaySinceReg` SMALLINT UNSIGNED   ,
 `DaySinceSvrOpen` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `GoldLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `ChgType` TINYINT   ,
 `ChgValue` INT UNSIGNED   ,
 `CurValue` INT UNSIGNED   ,
 `Approach` SMALLINT UNSIGNED   ,
 `DaySinceReg` SMALLINT UNSIGNED   ,
 `DaySinceSvrOpen` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `YuanLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `ChgType` TINYINT   ,
 `ChgValue` INT UNSIGNED   ,
 `CurValue` INT UNSIGNED   ,
 `Approach` SMALLINT UNSIGNED   ,
 `DaySinceReg` SMALLINT UNSIGNED   ,
 `DaySinceSvrOpen` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `PropLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `PropID` INT UNSIGNED   ,
 `PropType` SMALLINT UNSIGNED   ,
 `ChgType` TINYINT   ,
 `ChgValue` INT UNSIGNED   ,
 `CurValue` INT UNSIGNED   ,
 `Approach` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `MSKillLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `MSkillID` INT UNSIGNED   ,
 `MSkillLv` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `GeneralLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `GeneralID` INT UNSIGNED   ,
 `OpType` SMALLINT UNSIGNED   ,
 `OpPara1` INT UNSIGNED   ,
 `OpPara2` INT UNSIGNED   ,
 `OpPara3` INT UNSIGNED   ,
 `GeneralPhase` TINYINT   ,
 `FameHallLvl` SMALLINT UNSIGNED   ,
 `GeneralStar` SMALLINT UNSIGNED   ,
 `CurrentHour` VARCHAR(32)   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `FightPVPRankLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `OldScore` INT UNSIGNED   ,
 `NewScore` INT UNSIGNED   ,
 `OldRank` SMALLINT UNSIGNED   ,
 `NewRank` SMALLINT UNSIGNED   ,
 `VipLvl` TINYINT   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `FriendLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `FriendID` BIGINT UNSIGNED   ,
 `FriendName` VARCHAR(40)   ,
 `OptType` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `PubMailLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `MailID` INT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `AwardLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `Method` SMALLINT UNSIGNED   ,
 `ItemType` TINYINT   ,
 `ItemID` INT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `TutorialStepLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `Flag` TINYINT   ,
 `TutorialStep` VARCHAR(40)   ,
 `DaySinceReg` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `GeneralGetLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `GeneralID` INT UNSIGNED   ,
 `Type` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `TaskLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DrawTime` VARCHAR(32)   ,
 `TaskID` INT UNSIGNED   ,
 `TaskType` VARCHAR(40)   ,
 `DaySinceReg` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `GuildLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `GuildID` BIGINT UNSIGNED   ,
 `OptType` SMALLINT UNSIGNED   ,
 `ReservePara1` VARCHAR(40)   ,
 `ReservePara2` BIGINT UNSIGNED   ,
 `ReservePara3` INT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `MatchLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `Ranking` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `StartTime` VARCHAR(32)   ,
 `FinishTime` VARCHAR(32)   ,
 `Result` TINYINT   ,
 `MatchType` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `RebornLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `GeneralID` INT UNSIGNED   ,
 `GeneralLevel` TINYINT   ,
 `RebornType` TINYINT   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `ItemPurchaseLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `ItemType` TINYINT   ,
 `ItemID` INT UNSIGNED   ,
 `ItemNum` INT UNSIGNED   ,
 `PriceType` TINYINT   ,
 `PriceValue` INT UNSIGNED   ,
 `PurchaseApproach` SMALLINT UNSIGNED   ,
 `CurrentHour` VARCHAR(32)   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `SerialNumLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `SerialNum` VARCHAR(9)   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `ActivityLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `LevelID` INT UNSIGNED   ,
 `StartTime` VARCHAR(32)   ,
 `FinishTime` VARCHAR(32)   ,
 `ReservePara1` INT UNSIGNED   ,
 `ReservePara2` INT UNSIGNED   ,
 `ReservePara3` INT UNSIGNED   ,
 `SecSinceReg` INT UNSIGNED   ,
 `EvaluateLvl` TINYINT   ,
 `HardLvl` TINYINT   ,
 `DaySinceReg` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `PayLog` (
 `ChannelName` VARCHAR(40)   ,
 `RoleID` BIGINT UNSIGNED   ,
 `UserID` VARCHAR(40)   ,
 `DateTime` VARCHAR(32)   ,
 `productID` INT UNSIGNED   ,
 `OrderID` VARCHAR(128)   ,
 `MoneyPurchased` INT UNSIGNED   ,
 `DaySinceSvrOpen` SMALLINT UNSIGNED   ,
 `DaySinceReg` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `CoinLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `Level` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `CoinType` TINYINT   ,
 `ChgType` TINYINT   ,
 `ChgValue` INT UNSIGNED   ,
 `CurValue` INT UNSIGNED   ,
 `Approach` SMALLINT UNSIGNED   ,
 `DaySinceReg` SMALLINT UNSIGNED   ,
 `DaySinceSvrOpen` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `MajestyLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `MajestyLvl` SMALLINT UNSIGNED   ,
 `SecSinceReg` INT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `VIPLvl` TINYINT   ,
 `DaySinceReg` SMALLINT UNSIGNED   ,
 `CurrentHour` VARCHAR(32)   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `PvPLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `CombatEffectiveness` INT UNSIGNED   ,
 `VIPLvl` TINYINT   ,
 `MajestyLvl` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `WinCount` SMALLINT UNSIGNED   ,
 `EloLvlID` TINYINT   ,
 `PvPType` VARCHAR(40)   ,
 `CurrentHour` VARCHAR(32)   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `MarketRefreshLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `DateTime` VARCHAR(32)   ,
 `ShopType` VARCHAR(40)   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `PromotionalActivityLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `MajestyLvl` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `CurrentHour` VARCHAR(32)   ,
 `ConsumeAward` TINYINT   ,
 `CombatEffectivenessAward` TINYINT   ,
 `CarnivalAward` TINYINT   ,
 `SevenDayAward` TINYINT   ,
 `New7DayDailyProgress` TINYINT   ,
 `DaySinceReg` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `ArenaLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `MajestyLvl` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `CombatEffectiveness` INT UNSIGNED   ,
 `VIPLvl` TINYINT   ,
 `Ranking` TINYINT   ,
 `CurrentHour` VARCHAR(32)   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `GuildBossLog` (
 `Uin` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `GuildID` BIGINT UNSIGNED   ,
 `MajestyLvl` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `Damage` INT UNSIGNED   ,
 `CombatEffectiveness` INT UNSIGNED   ,
 `BossID` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `CloneBattleLog` (
 `Uin` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `DateTime` VARCHAR(32)   ,
 `BossID` INT UNSIGNED   ,
 `MemCount` TINYINT   ,
 `WinCount` SMALLINT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `DailyChallengeLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `CombatEffectiveness` INT UNSIGNED   ,
 `VIPLvl` TINYINT   ,
 `MajestyLvl` SMALLINT UNSIGNED   ,
 `DateTime` VARCHAR(32)   ,
 `WinCount` SMALLINT UNSIGNED   ,
 `SiegeEquipment` SMALLINT UNSIGNED   ,
 `Buff1` TINYINT   ,
 `Buff2` TINYINT   ,
 `Buff3` TINYINT   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
 CREATE TABLE IF NOT EXISTS `CheatLog` (
 `PlayerID` BIGINT UNSIGNED   ,
 `AccountName` VARCHAR(40)   ,
 `RoleName` VARCHAR(40)   ,
 `DateTime` VARCHAR(32)   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
 CREATE TABLE IF NOT EXISTS `CltFightStats` (
 `DateTime` VARCHAR(32)   ,
 `DungeonID` INT   ,
 `PlayerName` VARCHAR(40)   ,
 `MasterSkillID` INT   ,
 `MasterSkillUsedTimes` INT   ,
 `GeneralID` INT   ,
 `GeneralName` VARCHAR(40)   ,
 `RepelTimes` INT   ,
 `DeadWithdrawTimes` INT   ,
 `ActiveWithdrawTimes` INT   ,
 `CastSkillTimes` INT   ,
 `SkillHitNum` INT   ,
 `TotalHealIn` INT   ,
 `SkillHealIn` INT   ,
 `BuffHealIn` INT   ,
 `TotalHealOut` INT   ,
 `SkillHealOut` INT   ,
 `BuffHealOut` INT   ,
 `TotalDamageIn` INT   ,
 `NomarlAtkDamageIn` INT   ,
 `SpearAtkDamageIn` INT   ,
 `ShootAtkDamageIn` INT   ,
 `RushAtkDamageIn` INT   ,
 `FaceAtkDamageIn` INT   ,
 `SkillAtkDamageIn` INT   ,
 `BuffDamageIn` INT   ,
 `AtkCityDamageIn` INT   ,
 `AmbushAtkDamageIn` INT   ,
 `TotalDamageOut` INT   ,
 `NomarlAtkDamageOut` INT   ,
 `SpearAtkDamageOut` INT   ,
 `ShootAtkDamageOut` INT   ,
 `RushAtkDamageOut` INT   ,
 `FaceAtkDamageOut` INT   ,
 `SkillAtkDamageOut` INT   ,
 `BuffDamageOut` INT   ,
 `AtkCityDamageOut` INT   ,
 `AmbushAtkDamageOut` INT   ,
 `RecoveryInCity` INT   ,
 `Winner` VARCHAR(40)   ,
 `GeneralStar` INT   ,
 `GeneralMaxHP` INT   ,
 `GeneralStrength` INT   ,
 `GeneralStrengthDef` INT   ,
 `GeneralWit` INT   ,
 `GeneralWitDef` INT   ,
 `BaseDamageAtkNormal` INT   ,
 `BaseDamageAtkCity` INT   ,
 `ArmyBaseValue` INT UNSIGNED   ,
 `IsAutoBattle` TINYINT UNSIGNED   ,
 `MaxFramePerSecond` INT   ,
 `MinFramePerSecond` INT   ,
 `AverageFramePerSecond` INT   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `CltFightStatsPVE` (
 `PlayerID` BIGINT UNSIGNED   ,
 `PlayerName` VARCHAR(40)   ,
 `PlayerLevel` INT UNSIGNED   ,
 `FLevelID` INT UNSIGNED   ,
 `FLevelType` TINYINT   ,
 `FLevelHardType` TINYINT   ,
 `IsPass` TINYINT UNSIGNED   ,
 `PassReason` TINYINT   ,
 `TimeStart` VARCHAR(32)   ,
 `TimeFinish` VARCHAR(32)   ,
 `StarEvalResult` TINYINT UNSIGNED   ,
 `StarEvalIDCount` TINYINT UNSIGNED   ,
 `StarEvalIDList_1` INT UNSIGNED  ,
 `StarEvalIDList_2` INT UNSIGNED  ,
 `StarEvalIDList_3` INT UNSIGNED  ,
 `FallPropsCount` TINYINT   ,
 `FallPropsList_1` INT UNSIGNED  ,
 `FallPropsList_2` INT UNSIGNED  ,
 `FallPropsList_3` INT UNSIGNED  ,
 `FallPropsNumList` MEDIUMBLOB    ,
 `KillTroopNum` TINYINT UNSIGNED   ,
 `KillTowerNum` TINYINT UNSIGNED   ,
 `KillBarrierNum` TINYINT UNSIGNED   ,
 `DeadTroopNum` TINYINT UNSIGNED   ,
 `GSkillUseNum` TINYINT UNSIGNED   ,
 `MSkillUseNum` TINYINT UNSIGNED   ,
 `CityHpSelf` TINYINT UNSIGNED   ,
 `CityHpEnemy` TINYINT UNSIGNED   ,
 `DamageOut` INT UNSIGNED   ,
 `DamageIn` INT UNSIGNED   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `CltFightStatsArray` (
 `CltFightStatsCNT` TINYINT UNSIGNED   ,
 `CltFightStatsList_1` MEDIUMBLOB   ,
 `CltFightStatsList_2` MEDIUMBLOB   ,
 `CltFightStatsList_3` MEDIUMBLOB   ,
 `CltFightStatsList_4` MEDIUMBLOB   ,
 `CltFightStatsList_5` MEDIUMBLOB   ,
 `CltFightStatsList_6` MEDIUMBLOB   ,
 `CltFightStatsList_7` MEDIUMBLOB   ,
 `CltFightStatsList_8` MEDIUMBLOB   ,
 `CltFightStatsList_9` MEDIUMBLOB   ,
 `CltFightStatsList_10` MEDIUMBLOB   ,
 `CltFightStatsList_11` MEDIUMBLOB   ,
 `CltFightStatsList_12` MEDIUMBLOB   )
 ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
