divert(-1)
#
# Sendmail 8 configuration file for lists.openbsd.org
#
# This machine handles all mail for openbsd.{org,com,net}
#

divert(0)dnl
VERSIONID(`$OpenBSD: openbsd-lists.mc,v 1.7 2001/11/29 18:47:58 millert Exp $')
OSTYPE(openbsd)dnl
dnl
dnl Advertise ourselves as ``openbsd.org''
define(`confSMTP_LOGIN_MSG', `openbsd.org Sendmail $v/$Z/millert ready willing and able at $b')dnl
dnl
dnl Define relays, since not everyone uses internet addresses, even now
define(`UUCP_RELAY', `rutgers.edu')dnl
define(`BITNET_RELAY', `interbit.cren.net')dnl
define(`DECNET_RELAY', `vaxf.colorado.edu')dnl
dnl
dnl Override some default values
define(`confPRIVACY_FLAGS', `authwarnings, nobodyreturn')dnl
define(`confTRY_NULL_MX_LIST', `True')dnl
define(`confMAX_HOP', `30')dnl
dnl
dnl Some broken nameservers will return SERVFAIL (a temporary failure)
dnl on T_AAAA (IPv6) lookups.
define(`confBIND_OPTS', `WorkAroundBrokenAAAA')dnl
dnl
dnl Keep host status on disk between sendmail runs in the .hoststat dir
define(`confHOST_STATUS_DIRECTORY', `.hoststat')dnl
define(`confTO_HOSTSTATUS', `1h')dnl
dnl
dnl Always use fully qualified domains
FEATURE(always_add_domain)
dnl
dnl Need to add domo and mailman as "trusted users" to rewrite From lines
define(`confTRUSTED_USERS', `domo mailman')dnl
dnl
dnl Wait a day before sending mail about deferred messages
define(`confTO_QUEUEWARN', `1d')dnl
dnl
dnl Wait 4 days before giving up and bouncing the message
define(`confTO_QUEUERETURN', `4d')dnl
dnl
dnl SSL certificate paths
define(`CERT_DIR', `MAIL_SETTINGS_DIR`'certs')dnl
define(`confCACERT_PATH', `CERT_DIR')dnl
define(`confCACERT', `CERT_DIR/mycert.pem')dnl
define(`confSERVER_CERT', `CERT_DIR/mycert.pem')dnl
define(`confSERVER_KEY', `CERT_DIR/mykey.pem')dnl
define(`confCLIENT_CERT', `CERT_DIR/mycert.pem')dnl
define(`confCLIENT_KEY', `CERT_DIR/mykey.pem')dnl
dnl
dnl Make mail appear to be from openbsd.org
MASQUERADE_AS(openbsd.org)
FEATURE(masquerade_envelope)
dnl
dnl Need this for OpenBSD mailing lists
FEATURE(stickyhost)dnl
FEATURE(virtusertable)dnl
dnl
dnl Spam blocking features
FEATURE(access_db)dnl
FEATURE(blacklist_recipients)dnl
dnl FEATURE(dnsbl, `rbl.maps.vix.com', `Rejected - see http://www.mail-abuse.org/rbl/')dnl
dnl FEATURE(dnsbl, `dul.maps.vix.com', `Dialup - see http://www.mail-abuse.org/dul/')dnl
dnl FEATURE(dnsbl, `relays.mail-abuse.org', `Open spam relay - see http://www.mail-abuse.org/rss/')dnl
dnl
dnl List the mailers we support
FEATURE(`no_default_msa')dnl
MAILER(local)dnl
MAILER(smtp)dnl
DAEMON_OPTIONS(`Family=inet, address=0.0.0.0, Name=MTA')dnl
DAEMON_OPTIONS(`Family=inet6, address=::, Name=MTA6, M=O')dnl
DAEMON_OPTIONS(`Family=inet, address=0.0.0.0, Port=587, Name=MSA, M=E')dnl
DAEMON_OPTIONS(`Family=inet6, address=::, Port=587, Name=MSA6, M=O, M=E')dnl
CLIENT_OPTIONS(`Family=inet6, Address=::')dnl
CLIENT_OPTIONS(`Family=inet, Address=0.0.0.0')dnl
dnl
dnl Finally, we have the local cf-style goo
LOCAL_CONFIG
# Treat mail to openbsd.{org,net,com} as local
Cw openbsd.org
Cw openbsd.net
Cw openbsd.com
Cw openssh.org
Cw anonopenbsd.cs.colorado.edu
#
#  Regular expression to reject:
#    * numeric-only localparts from aol.com and msn.com
#    * localparts starting with a digit from juno.com
#    * localparts longer than 20 characters from aol.com
#
Kcheckaddress regex -a@MATCH
   ^([0-9]+<@(aol|msn)\.com|[0-9][^<]*<@juno\.com|.{20}[^<]+<@aol\.com)\.?>

#
#  Names that won't be allowed in a To: line (local-part and domains)
#
C{RejectToLocalparts}		friend you user
C{RejectToDomains}		public.com the-internet.com

LOCAL_RULESETS
#
# Header checks
#
HTo: $>CheckTo
HMessage-Id: $>CheckMessageId
HSubject: $>Check_Subject
HX-Spanska: $>Spanska

#
# Melissa worm detection (done in Check_Subject)
# See http://www.cert.org/advisories/CA-99-04-Melissa-Macro-Virus.html
#
D{MPat}Important Message From
D{MMsg}This message may contain the Melissa virus; see http://www.cert.org/advisories/CA-99-04-Melissa-Macro-Virus.html

#
# ILOVEYOU worm detection (done in Check_Subject)
# See http://www.datafellows.com/v-descs/love.htm
#
D{ILPat}ILOVEYOU
D{ILMsg}This message may contain the ILOVEYOU virus; see http://www.datafellows.com/v-descs/love.htm

#
# Life stages worm detection (done in Check_Subject)
# See http://www.f-secure.com/v-descs/stages.htm
#
D{LSPat}Fw: Life stages
D{LSMsg}This message may contain the Life stages virus; see http://www.f-secure.com/v-descs/stages.htm

#
# Reject some mail based on To: header
#
SCheckTo
R$={RejectToLocalparts}@$*	$#error $: "553 Header error"
R$*@$={RejectToDomains}		$#error $: "553 Header error"

#
# Enforce valid Message-Id to help stop spammers
#
SCheckMessageId
R< $+ @ $+ >			$@ OK
R$*				$#error $: 553 Header Error

#
# Happy99 worm detection
#
SSpanska
R$*				$#error $: "553 Your system is probably infected by the Happy99 worm; see http://www.symantec.com/avcenter/venc/data/happy99.worm.html"

#
# Check Subject line for worm/virus telltales
#
SCheck_Subject
R${MPat} $*			$#error $: 553 ${MMsg}
RRe: ${MPat} $*			$#error $: 553 ${MMsg}
R${ILPat}			$#error $: 553 ${ILMsg}
RRe: ${ILPat}			$#error $: 553 ${ILMsg}
R${LSPat}			$#error $: 553 ${LSMsg}
RRe: ${LSPat}			$#error $: 553 ${LSMsg}
