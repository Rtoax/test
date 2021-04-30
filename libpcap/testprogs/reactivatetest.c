#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <pcap/funcattrs.h>

/* Forwards */
static void PCAP_NORETURN error(PCAP_FORMAT_STRING(const char *), ...) PCAP_PRINTFLIKE(1,2);

int
main(void)
{
	char ebuf[PCAP_ERRBUF_SIZE];
	pcap_t *pd;
	int status = 0;

	pd = pcap_open_live("lo0", 65535, 0, 1000, ebuf);
	if (pd == NULL) {
		pd = pcap_open_live("lo", 65535, 0, 1000, ebuf);
		if (pd == NULL) {
			error("Neither lo0 nor lo could be opened: %s",
			    ebuf);
		}
	}
	status = pcap_activate(pd);
	if (status != PCAP_ERROR_ACTIVATED) {
		if (status == 0)
			error("pcap_activate() of opened pcap_t succeeded");
		else if (status == PCAP_ERROR)
			error("pcap_activate() of opened pcap_t failed with %s, not PCAP_ERROR_ACTIVATED",
			    pcap_geterr(pd));
		else
			error("pcap_activate() of opened pcap_t failed with %s, not PCAP_ERROR_ACTIVATED",
			    pcap_statustostr(status));
	}
	return 0;
}

/* VARARGS */
static void
error(const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, "reactivatetest: ");
	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (*fmt) {
		fmt += strlen(fmt);
		if (fmt[-1] != '\n')
			(void)fputc('\n', stderr);
	}
	exit(1);
	/* NOTREACHED */
}
