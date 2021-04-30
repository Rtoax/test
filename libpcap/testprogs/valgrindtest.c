#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pcap/funcattrs.h>

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(_AIX) || defined(sun)
/* OS with BPF - use BPF */
#define USE_BPF
#elif defined(linux)
/* Linux - use socket filters */
#define USE_SOCKET_FILTERS
#else
#error "Unknown platform or platform that doesn't support Valgrind"
#endif

#if defined(USE_BPF)

#include <sys/ioctl.h>
#include <net/bpf.h>

/*
 * Make "pcap.h" not include "pcap/bpf.h"; we are going to include the
 * native OS version, as we're going to be doing our own ioctls to
 * make sure that, in the uninitialized-data tests, the filters aren't
 * checked by libpcap before being handed to BPF.
 */
#define PCAP_DONT_INCLUDE_PCAP_BPF_H

#elif defined(USE_SOCKET_FILTERS)

#include <sys/socket.h>
#include <linux/types.h>
#include <linux/filter.h>

#endif

/*
 * Squelch a warning.
 *
 * We include system headers to be able to directly set the filter to
 * a program with uninitialized content, to make sure what we're testing
 * is Valgrind's checking of the system call to set the filter, and we
 * also include <pcap.h> to open the device in the first place, and that
 * means that we may get collisions between their definitions of
 * BPF_STMT and BPF_JUMP - and do, in fact, get them on Linux (the
 * definitons may be semantically the same, but that's not sufficient to
 * avoid the warnings, as the preprocessor doesn't know that u_short is
 * just unsigned short).
 *
 * So we undefine BPF_STMT and BPF_JUMP to avoid the warning.
 */
#undef BPF_STMT
#undef BPF_JUMP
#include <pcap.h>

static char *program_name;

/* Forwards */
static void PCAP_NORETURN usage(void);
static void PCAP_NORETURN error(const char *, ...);
static void warning(const char *, ...);

/*
 * On Windows, we need to open the file in binary mode, so that
 * we get all the bytes specified by the size we get from "fstat()".
 * On UNIX, that's not necessary.  O_BINARY is defined on Windows;
 * we define it as 0 if it's not defined, so it does nothing.
 */
#ifndef O_BINARY
#define O_BINARY	0
#endif

static char *
read_infile(char *fname)
{
	register int i, fd, cc;
	register char *cp;
	struct stat buf;

	fd = open(fname, O_RDONLY|O_BINARY);
	if (fd < 0)
		error("can't open %s: %s", fname, pcap_strerror(errno));

	if (fstat(fd, &buf) < 0)
		error("can't stat %s: %s", fname, pcap_strerror(errno));

	/*
	 * _read(), on Windows, has an unsigned int byte count and an
	 * int return value, so we can't handle a file bigger than
	 * INT_MAX - 1 bytes (and have no reason to do so; a filter *that*
	 * big will take forever to compile).  (The -1 is for the '\0' at
	 * the end of the string.)
	 */
	if (buf.st_size > INT_MAX - 1)
		error("%s is larger than %d bytes; that's too large", fname,
		    INT_MAX - 1);
	cp = malloc((u_int)buf.st_size + 1);
	if (cp == NULL)
		error("malloc(%d) for %s: %s", (u_int)buf.st_size + 1,
			fname, pcap_strerror(errno));
	cc = (int)read(fd, cp, (u_int)buf.st_size);
	if (cc < 0)
		error("read %s: %s", fname, pcap_strerror(errno));
	if (cc != buf.st_size)
		error("short read %s (%d != %d)", fname, cc, (int)buf.st_size);

	close(fd);
	/* replace "# comment" with spaces */
	for (i = 0; i < cc; i++) {
		if (cp[i] == '#')
			while (i < cc && cp[i] != '\n')
				cp[i++] = ' ';
	}
	cp[cc] = '\0';
	return (cp);
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

#define INSN_COUNT	17

int
main(int argc, char **argv)
{
	char *cp, *device;
	int op;
	int dorfmon, useactivate;
	char ebuf[PCAP_ERRBUF_SIZE];
	char *infile;
	const char *cmdbuf;
	pcap_if_t *devlist;
	pcap_t *pd;
	int status = 0;
	int pcap_fd;
#if defined(USE_BPF)
	struct bpf_program bad_fcode;
	struct bpf_insn uninitialized[INSN_COUNT];
#elif defined(USE_SOCKET_FILTERS)
	struct sock_fprog bad_fcode;
	struct sock_filter uninitialized[INSN_COUNT];
#endif
	struct bpf_program fcode;

	device = NULL;
	dorfmon = 0;
	useactivate = 0;
	infile = NULL;

	if ((cp = strrchr(argv[0], '/')) != NULL)
		program_name = cp + 1;
	else
		program_name = argv[0];

	opterr = 0;
	while ((op = getopt(argc, argv, "aF:i:I")) != -1) {
		switch (op) {

		case 'a':
			useactivate = 1;
			break;

		case 'F':
			infile = optarg;
			break;

		case 'i':
			device = optarg;
			break;

		case 'I':
			dorfmon = 1;
			useactivate = 1;	/* required for rfmon */
			break;

		default:
			usage();
			/* NOTREACHED */
		}
	}

	if (device == NULL) {
		/*
		 * No interface specified; get whatever pcap_lookupdev()
		 * finds.
		 */
		if (pcap_findalldevs(&devlist, ebuf) == -1)
			error("%s", ebuf);
		if (devlist == NULL)
			error("no interfaces available for capture");
		device = strdup(devlist->name);
		pcap_freealldevs(devlist);
	}

	if (infile != NULL) {
		/*
		 * Filter specified with "-F" and a file containing
		 * a filter.
		 */
		cmdbuf = read_infile(infile);
	} else {
		if (optind < argc) {
			/*
			 * Filter specified with arguments on the
			 * command line.
			 */
			cmdbuf = copy_argv(&argv[optind+1]);
		} else {
			/*
			 * No filter specified; use an empty string, which
			 * compiles to an "accept all" filter.
			 */
			cmdbuf = "";
		}
	}

	if (useactivate) {
		pd = pcap_create(device, ebuf);
		if (pd == NULL)
			error("%s: pcap_create() failed: %s", device, ebuf);
		status = pcap_set_snaplen(pd, 65535);
		if (status != 0)
			error("%s: pcap_set_snaplen failed: %s",
			    device, pcap_statustostr(status));
		status = pcap_set_promisc(pd, 1);
		if (status != 0)
			error("%s: pcap_set_promisc failed: %s",
			    device, pcap_statustostr(status));
		if (dorfmon) {
			status = pcap_set_rfmon(pd, 1);
			if (status != 0)
				error("%s: pcap_set_rfmon failed: %s",
				    device, pcap_statustostr(status));
		}
		status = pcap_set_timeout(pd, 1000);
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
	} else {
		*ebuf = '\0';
		pd = pcap_open_live(device, 65535, 1, 1000, ebuf);
		if (pd == NULL)
			error("%s", ebuf);
		else if (*ebuf)
			warning("%s", ebuf);
	}

	pcap_fd = pcap_fileno(pd);

	/*
	 * Try setting a filter with an uninitialized bpf_program
	 * structure.  This should cause valgrind to report a
	 * problem.
	 *
	 * We don't check for errors, because it could get an
	 * error due to a bad pointer or count.
	 */
#if defined(USE_BPF)
	ioctl(pcap_fd, BIOCSETF, &bad_fcode);
#elif defined(USE_SOCKET_FILTERS)
	setsockopt(pcap_fd, SOL_SOCKET, SO_ATTACH_FILTER, &bad_fcode,
	    sizeof(bad_fcode));
#endif

	/*
	 * Try setting a filter with an initialized bpf_program
	 * structure that points to an uninitialized program.
	 * That should also cause valgrind to report a problem.
	 *
	 * We don't check for errors, because it could get an
	 * error due to a bad pointer or count.
	 */
#if defined(USE_BPF)
	bad_fcode.bf_len = INSN_COUNT;
	bad_fcode.bf_insns = uninitialized;
	ioctl(pcap_fd, BIOCSETF, &bad_fcode);
#elif defined(USE_SOCKET_FILTERS)
	bad_fcode.len = INSN_COUNT;
	bad_fcode.filter = uninitialized;
	setsockopt(pcap_fd, SOL_SOCKET, SO_ATTACH_FILTER, &bad_fcode,
	    sizeof(bad_fcode));
#endif

	/*
	 * Now compile a filter and set the filter with that.
	 * That should *not* cause valgrind to report a
	 * problem.
	 */
	if (pcap_compile(pd, &fcode, cmdbuf, 1, 0) < 0)
		error("can't compile filter: %s", pcap_geterr(pd));
	if (pcap_setfilter(pd, &fcode) < 0)
		error("can't set filter: %s", pcap_geterr(pd));

	pcap_close(pd);
	exit(status < 0 ? 1 : 0);
}

static void
usage(void)
{
	(void)fprintf(stderr, "%s, with %s\n", program_name,
	    pcap_lib_version());
	(void)fprintf(stderr,
	    "Usage: %s [-aI] [ -F file ] [ -i interface ] [ expression ]\n",
	    program_name);
	exit(1);
}
