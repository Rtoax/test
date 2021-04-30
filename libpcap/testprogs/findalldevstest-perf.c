
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>

#include <pcap.h>

#include "pcap/funcattrs.h"
#include "portability.h"

int main(int argc, char **argv)
{
  pcap_if_t *alldevs;
  int exit_status = 0;
  char errbuf[PCAP_ERRBUF_SIZE+1];

  struct rusage start_rusage, end_rusage;
  struct timeval ktime, utime, tottime;

  if (getrusage(RUSAGE_SELF, &start_rusage) == -1) {
    fprintf(stderr, "getrusage() fails at start\n");
    exit(1);
  }
  for (int i = 0; i < 500; i++)
  {
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
      fprintf(stderr,"Error in pcap_findalldevs: %s\n",errbuf);
      exit(1);
    }
    pcap_freealldevs(alldevs);
  }

  if (getrusage(RUSAGE_SELF, &end_rusage) == -1) {
    fprintf(stderr, "getrusage() fails at end\n");
    exit(1);
  }
  timersub(&end_rusage.ru_stime, &start_rusage.ru_stime, &ktime);
  timersub(&end_rusage.ru_utime, &start_rusage.ru_utime, &utime);
  timeradd(&ktime, &utime, &tottime);
  printf("Total CPU secs: kernel %g, user %g, total %g\n",
      (double)ktime.tv_sec + ((double)ktime.tv_usec / 1000000.0),
      (double)utime.tv_sec + ((double)utime.tv_usec / 1000000.0),
      (double)tottime.tv_sec + ((double)tottime.tv_usec / 1000000.0));
  exit(exit_status);
}
