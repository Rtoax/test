#include <syslog.h>
#include <stdio.h>
#include <errno.h>

//sudo watch -n1 tail /var/log/messages

int main()
{
	//syslog (LOG_MAKEPRI(LOG_LOCAL1, LOG_ERROR),
	//		"syslog\n");

	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("[jtsran]", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog (LOG_NOTICE, "[NOTICE] Program started by User %d", getuid ());
	syslog (LOG_INFO, "[Info] A tree falls in a forest");
	syslog (LOG_ERR, "[ERROR] A tree falls in a forest");

//    sleep(100);
    
	closelog ();

	return 0;
}
