.PHONY: all clean

SERVICES = GuildExpeditionSvr ClusterDBSvr ZoneSvr NetConn FightSvr AccountSvr AsyncPvpSvr RoleSvr MatchSvr RankSvr ClusterGate ReplaySvr GuildSvr MessageSvr MailSvr MailDBSvr FriendSvr XiYouSDKSvr SdkDMSvr SerialNumSvr MiscSvr CloneBattleSvr LogQuerySvr IdipAgentSvr MineSvr MineDBSvr ClusterAccountSvr ZoneHttpConn

all:
	@for dir in $(SERVICES); do\
	( $(MAKE) -C $$dir all ) done

clean:
	@for dir in $(SERVICES); do\
	( $(MAKE) -C $$dir clean ) done
