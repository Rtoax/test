#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <pcap.h>

#include <pcap/funcattrs.h>

//gcc can_set_rfmon_test.c -lpcap


static const char *program_name;


/* VARARGS */
static void error(const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, "%s: ", program_name);
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

int main(int argc, char **argv)
{
	const char *cp;
	pcap_t *pd;
	char ebuf[PCAP_ERRBUF_SIZE];
	int status;

	if ((cp = strrchr(argv[0], '/')) != NULL)
		program_name = cp + 1;
	else
		program_name = argv[0];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <device>\n", program_name);
		return 2;
	}

	pd = pcap_create(argv[1], ebuf);
	if (pd == NULL)
		error("%s", ebuf);
	status = pcap_can_set_rfmon(pd);
	if (status < 0) {
		if (status == PCAP_ERROR)
			error("%s: pcap_can_set_rfmon failed: %s", argv[1],
			    pcap_geterr(pd));
		else
			error("%s: pcap_can_set_rfmon failed: %s", argv[1],
			    pcap_statustostr(status));
	}
	printf("%s: Monitor mode %s be set\n", argv[1], status ? "can" : "cannot");
	return 0;
}

