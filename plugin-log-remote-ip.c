#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <bglibs/systime.h>
#include <bglibs/msg.h>
#include "mailfront.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

static const char *mainprog = "mailfront";
static const char *thisprog = "plugin-log-remote-ip";
static str msgheader;

static const char *linkproto;
static const char *local_host;
static str local_ip;
static const char *remote_host;
static str remote_ip;

static const char *senderusername;
static const char *senderdomain;
static const char *senderloghash;
static unsigned long thispid;

static int srand_called=0;

void eprintf_logname()
{
	if (!thispid) {
		thispid = (unsigned long)getpid();
	}
	eprintf("%s-%s[%lu]: ", mainprog, thisprog, thispid);
}

void generate_random_string(char *s, unsigned long len)
{
	/* be sure to have called srand exactly one time */
	if(!srand_called)
	{
		srand( (unsigned) time( (time_t *) 0 ) * getpid());
		srand_called=1;
	}
	const char *chars = "abcdefghijklmnopqrstuvwxyz"
			    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			    "0123456789";
	unsigned int max = strlen( chars );
	unsigned long i = 0L;

	for ( ; i < len; ++i ) {
		s[ i ] = chars[ rand() % max ];
	}
	s[ i ] = '\0';
}

static int add_msgheader(str *s)
{
	const char *tmp;
	const char *l1;
	const char *l2;
	char myhash[34];
	generate_random_string(myhash, 32);
	senderloghash = myhash;
	if (!thispid) {
		thispid = (unsigned long)getpid();
	}
	char mypid[sizeof(thispid) + 1];
	sprintf(mypid, "%lu", thispid);
	if ((tmp = session_getenv("TRACK_AUTH_SENDER_HEADER")) != 0 && *tmp)
		l1 = tmp;
	else {
		l1 = "X-AntiAbuse: This header was added to track abuse, "
		     "please include it with any abuse report";
	}
	if ((tmp = session_getenv("TRACK_AUTH_SENDER")) != 0 && *tmp)
		l2 = tmp;
	else {
		l2 = "X-AntiAbuse: ";
	}
	if (!str_cat2s(s, l1, "\n")) return 0;
	if (!str_cat4s(s, l2, "Primary Hostname  - ", local_host, "\n"))
		return 0;
	if (remote_host) {
		if (!str_cat4s(s, l2, "Sender Hostname   - ",
		    remote_host, "\n")) {
			return 0;
		}
	}
	if (!str_cat4s(s, l2, "Sender IP Address - ",
	    (char *)getprotoenv("REMOTEIP"), "\n")) {
		return 0;
	}
	if (!str_cat4s(s, l2, "Sender Username   - ", senderusername, "\n"))
		return 0;
	if (!str_cat4s(s, l2, "Sender Domain     - ", senderdomain, "\n"))
		return 0;
	if (!str_cat3s(s, l2, "Sender Address    - ", senderusername))
		return 0;
	if (!str_cat3s(s, "@", senderdomain, "\n")) return 0;
	if (!str_cat4s(s, l2, "Log Hash          - ", senderloghash, "\n"))
		return 0;
	if (!str_cat4s(s, l2, "Log Pid           - ", (char *)mypid, "\n"))
		return 0;
	if (!str_cat3s(s, l2, "authenticated_id  - ", senderusername))
		return 0;
	if (!str_cat3s(s, "@", senderdomain, "\n")) return 0;
	return 1;
}

static int str_copyip(str *s, const char *ip, int is_ipv6)
{
	s->len = 0;
	if (ip != 0) {
		if (is_ipv6 && !str_copys(s, "IPv6:"))
			return 0;
		return str_cats(s, ip);
	}
	return 1;
}

static const response* sender(str* r, str* params)
{
  senderusername = session_getstr("auth_user");
  senderdomain = session_getstr("auth_domain");
  eprintf_logname();
  if (session_getnum("authenticated", 0) && senderusername && senderdomain)
  {
      eprintf("Auth remote ip = %s\n",remote_ip.s);
  }else{
      eprintf("Strange remote ip = %s\n",remote_ip.s);
  }


  (void)r;
  return 0;
  (void)params;
}

static const response *init(void)
{
	int is_ipv6;

	linkproto = getprotoenv(0);
	is_ipv6 = linkproto != 0 && strcasecmp(linkproto, "TCP6") == 0;
	if (!str_copyip(&local_ip, getprotoenv("LOCALIP"), is_ipv6))
		return &resp_oom;
	if (!str_copyip(&remote_ip, getprotoenv("REMOTEIP"), is_ipv6))
		return &resp_oom;
	local_host = getprotoenv("LOCALHOST");
	remote_host = getprotoenv("REMOTEHOST");
	if (!thispid) {
		thispid = (unsigned long)getpid();
	}

	return 0;
}

static const response *data_start(int fd)
{
	msgheader.len = 0;
	if (session_getnum("authenticated", 0)) {
		senderusername = session_getstr("auth_user");
		senderdomain = session_getstr("auth_domain");
		if (!thispid) {
			thispid = (unsigned long)getpid();
		}

		if (senderusername && senderdomain
		    && add_msgheader(&msgheader) && senderloghash) {
			eprintf_logname();
			eprintf("Log Hash = %s\n", senderloghash);
			eprintf_logname();
			eprintf("authenticated_id = %s@%s\n",
				senderusername, senderdomain);
		}
		else{
			eprintf_logname();
    	    eprintf("no username and domain\n");
			return &resp_internal;
		}
	}
	else {
		//if(remote_ip!=null){
        eprintf("unauthenticated remote ip = %s\n",remote_ip.s);
        //}
		return 0;		
	}
	return backend_data_block(msgheader.s, msgheader.len);
	(void)fd;
}

struct plugin plugin = {
	.version = PLUGIN_VERSION,
	.init = init,
	.sender = sender,
	.data_start = data_start,
};
