#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define THREAD_HANDLE			pthread_t
#define THREAD_FUNC_ARG_TYPE		void *
#define THREAD_FUNC_RETURN_TYPE	void *
#include <errno.h>
#include <sys/types.h>

#include <pcap.h>

#include <pcap/funcattrs.h>


static char *program_name;

/* Forwards */
static void countme(u_char *, const struct pcap_pkthdr *, const u_char *);
static void PCAP_NORETURN usage(void);
static void PCAP_NORETURN error(const char *, ...);
static void warning(const char *, ...);
static char *copy_argv(char **);

static pcap_t *pd;

static void
catch_sigusr1(int sig)
{
	printf("Got SIGUSR1\n");
}

static void
sleep_secs(int secs)
{
	unsigned secs_remaining;

	if (secs <= 0)
		return;
	secs_remaining = secs;
	while (secs_remaining != 0)
		secs_remaining = sleep(secs_remaining);
}

static THREAD_FUNC_RETURN_TYPE
capture_thread_func(THREAD_FUNC_ARG_TYPE arg)
{
	char *device = arg;
	int packet_count;
	int status;
	struct sigaction action;
	sigset_t mask;

	sigemptyset(&mask);
	action.sa_handler = catch_sigusr1;
	action.sa_mask = mask;
	action.sa_flags = 0;
	if (sigaction(SIGUSR1, &action, NULL) == -1)
		error("Can't catch SIGUSR1: %s", strerror(errno));

	printf("Listening on %s\n", device);
	for (;;) {
		packet_count = 0;
		status = pcap_dispatch(pd, -1, countme,
		    (u_char *)&packet_count);
		if (status < 0)
			break;
		if (status != 0) {
			printf("%d packets seen, %d packets counted after pcap_dispatch returns\n",
			    status, packet_count);
		} else
			printf("No packets seen by pcap_dispatch\n");
	}
	if (status == PCAP_ERROR_BREAK) {
		/*
		 * We got interrupted, so perhaps we didn't
		 * manage to finish a line we were printing.
		 * Print an extra newline, just in case.
		 */
		putchar('\n');
		printf("Loop got broken\n");
	}
	(void)fflush(stdout);
	if (status == PCAP_ERROR) {
		/*
		 * Error.  Report it.
		 */
		(void)fprintf(stderr, "%s: pcap_dispatch: %s\n",
		    program_name, pcap_geterr(pd));
	}
	return 0;
}

int
main(int argc, char **argv)
{
	register int op;
	register char *cp, *cmdbuf, *device;
	int do_wakeup = 1;
	pcap_if_t *devlist;
	bpf_u_int32 localnet, netmask;
	struct bpf_program fcode;
	char ebuf[PCAP_ERRBUF_SIZE];
	int status;
	THREAD_HANDLE capture_thread;
	void *retval;

	device = NULL;
	if ((cp = strrchr(argv[0], '/')) != NULL)
		program_name = cp + 1;
	else
		program_name = argv[0];

	opterr = 0;
	while ((op = getopt(argc, argv, "i:n")) != -1) {
		switch (op) {

		case 'i':
			device = optarg;
			break;

		case 'n':
			do_wakeup = 0;
			break;

		default:
			usage();
			/* NOTREACHED */
		}
	}

	if (device == NULL) {
		if (pcap_findalldevs(&devlist, ebuf) == -1)
			error("%s", ebuf);
		if (devlist == NULL)
			error("no interfaces available for capture");
		device = strdup(devlist->name);
		pcap_freealldevs(devlist);
	}
	*ebuf = '\0';
	pd = pcap_create(device, ebuf);
	if (pd == NULL)
		error("%s", ebuf);
	status = pcap_set_snaplen(pd, 65535);
	if (status != 0)
		error("%s: pcap_set_snaplen failed: %s",
			    device, pcap_statustostr(status));
	status = pcap_set_timeout(pd, 5*60*1000);
	if (status != 0)
		error("%s: pcap_set_timeout failed: %s",
		    device, pcap_statustostr(status));
	status = pcap_activate(pd);
	if (status < 0) {
		/*
		 * pcap_activate() failed.
		 */
		error("%s: %s\n(%s)", device,
		    pcap_statustostr(status), pcap_geterr(pd));
	} else if (status > 0) {
		/*
		 * pcap_activate() succeeded, but it's warning us
		 * of a problem it had.
		 */
		warning("%s: %s\n(%s)", device,
		    pcap_statustostr(status), pcap_geterr(pd));
	}
	if (pcap_lookupnet(device, &localnet, &netmask, ebuf) < 0) {
		localnet = 0;
		netmask = 0;
		warning("%s", ebuf);
	}
	cmdbuf = copy_argv(&argv[optind]);

	if (pcap_compile(pd, &fcode, cmdbuf, 1, netmask) < 0)
		error("%s", pcap_geterr(pd));

	if (pcap_setfilter(pd, &fcode) < 0)
		error("%s", pcap_geterr(pd));

	status = pthread_create(&capture_thread, NULL, capture_thread_func,
	    device);
	if (status != 0)
		error("Can't create capture thread: %s", strerror(status));
	sleep_secs(60);
	printf("Doing pcap_breakloop()\n");
	pcap_breakloop(pd);
	if (do_wakeup) {
		/*
		 * Force a wakeup in the capture thread.
		 *
		 * On some platforms, with some devices,, pcap_breakloop()
		 * can't do that itself.  On Windows, poke the device's
		 * event handle; on UN*X, send a SIGUSR1 to the thread.
		 */
		printf("Sending SIGUSR1\n");
		status = pthread_kill(capture_thread, SIGUSR1);
		if (status != 0)
			warning("Can't interrupt capture thread: %s",
			strerror(status));
	}

	/*
	 * Now wait for the capture thread to terminate.
	 */
	status = pthread_join(capture_thread, &retval);
	if (status != 0)
		error("Wait for thread termination failed: %s",
		    strerror(status));

	pcap_close(pd);
	pcap_freecode(&fcode);
	exit(status == -1 ? 1 : 0);
}

static void
countme(u_char *user, const struct pcap_pkthdr *h, const u_char *sp)
{
	int *counterp = (int *)user;

	(*counterp)++;
}

static void
usage(void)
{
	(void)fprintf(stderr, "Usage: %s [ -n ] [ -i interface ] [ expression ]\n",
	    program_name);
	exit(1);
}

/* VARARGS */
static void
error(const char *fmt, ...)
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

/* VARARGS */
static void
warning(const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, "%s: WARNING: ", program_name);
	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (*fmt) {
		fmt += strlen(fmt);
		if (fmt[-1] != '\n')
			(void)fputc('\n', stderr);
	}
}

/*
 * Copy arg vector into a new buffer, concatenating arguments with spaces.
 */
static char *
copy_argv(register char **argv)
{
	register char **p;
	register size_t len = 0;
	char *buf;
	char *src, *dst;

	p = argv;
	if (*p == 0)
		return 0;

	while (*p)
		len += strlen(*p++) + 1;

	buf = (char *)malloc(len);
	if (buf == NULL)
		error("copy_argv: malloc");

	p = argv;
	dst = buf;
	while ((src = *p++) != NULL) {
		while ((*dst++ = *src++) != '\0')
			;
		dst[-1] = ' ';
	}
	dst[-1] = '\0';

	return buf;
}
