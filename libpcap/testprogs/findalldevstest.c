#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <pcap.h>

#include <pcap/funcattrs.h>

static int ifprint(pcap_if_t *d);
/* From tcptraceroute */
#define IPTOSBUFFERS	12
static char *iptos(bpf_u_int32 in)
{
	static char output[IPTOSBUFFERS][3*4+3+1];
	static short which;
	u_char *p;

	p = (u_char *)&in;
	which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
	sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return output[which];
}


int main(int argc, char **argv)
{
  pcap_if_t *alldevs;
  pcap_if_t *d;
  bpf_u_int32 net, mask;
  int exit_status = 0;
  char errbuf[PCAP_ERRBUF_SIZE+1];

  if (pcap_findalldevs(&alldevs, errbuf) == -1)
  {
    fprintf(stderr,"Error in pcap_findalldevs: %s\n",errbuf);
    exit(1);
  }
  for(d=alldevs;d;d=d->next)
  {
    if (!ifprint(d))
      exit_status = 2;
  }

  if (alldevs != NULL)
  {
    if (pcap_lookupnet(alldevs->name, &net, &mask, errbuf) < 0)
    {
      /*
       * XXX - this doesn't distinguish between "a real error
       * occurred" and "this interface doesn't *have* an IPv4
       * address".  The latter shouldn't be treated as an error.
       *
       * We look for the interface name, followed by a colon and
       * a space, and, if we find it,w e see if what follows it
       * is "no IPv4 address assigned".
       */
      size_t devnamelen = strlen(alldevs->name);
      if (strncmp(errbuf, alldevs->name, devnamelen) == 0 &&
          strncmp(errbuf + devnamelen, ": ", 2) == 0 &&
          strcmp(errbuf + devnamelen + 2, "no IPv4 address assigned") == 0)
        printf("Preferred device is not on an IPv4 network\n");
      else {
        fprintf(stderr,"Error in pcap_lookupnet: %s\n",errbuf);
        exit_status = 2;
      }
    }
    else
    {
      printf("Preferred device is on network: %s/%s\n",iptos(net), iptos(mask));
    }
  }

  pcap_freealldevs(alldevs);
  exit(exit_status);
}

static int ifprint(pcap_if_t *d)
{
  pcap_addr_t *a;
  char ipv4_buf[INET_ADDRSTRLEN];
  char ipv6_buf[INET6_ADDRSTRLEN];
  const char *sep;
  int status = 1; /* success */

  printf("%s\n",d->name);
  if (d->description)
    printf("\tDescription: %s\n",d->description);
  printf("\tFlags: ");
  sep = "";
  if (d->flags & PCAP_IF_UP) {
    printf("%sUP", sep);
    sep = ", ";
  }
  if (d->flags & PCAP_IF_RUNNING) {
    printf("%sRUNNING", sep);
    sep = ", ";
  }
  if (d->flags & PCAP_IF_LOOPBACK) {
    printf("%sLOOPBACK", sep);
    sep = ", ";
  }
  if (d->flags & PCAP_IF_WIRELESS) {
    printf("%sWIRELESS", sep);
    switch (d->flags & PCAP_IF_CONNECTION_STATUS) {

    case PCAP_IF_CONNECTION_STATUS_UNKNOWN:
      printf(" (association status unknown)");
      break;

    case PCAP_IF_CONNECTION_STATUS_CONNECTED:
      printf(" (associated)");
      break;

    case PCAP_IF_CONNECTION_STATUS_DISCONNECTED:
      printf(" (not associated)");
      break;

    case PCAP_IF_CONNECTION_STATUS_NOT_APPLICABLE:
      break;
    }
  } else {
    switch (d->flags & PCAP_IF_CONNECTION_STATUS) {

    case PCAP_IF_CONNECTION_STATUS_UNKNOWN:
      printf(" (connection status unknown)");
      break;

    case PCAP_IF_CONNECTION_STATUS_CONNECTED:
      printf(" (connected)");
      break;

    case PCAP_IF_CONNECTION_STATUS_DISCONNECTED:
      printf(" (disconnected)");
      break;

    case PCAP_IF_CONNECTION_STATUS_NOT_APPLICABLE:
      break;
    }
  }
  sep = ", ";
  printf("\n");

  for(a=d->addresses;a;a=a->next) {
    if (a->addr != NULL)
      switch(a->addr->sa_family) {
      case AF_INET:
        printf("\tAddress Family: AF_INET\n");
        if (a->addr)
          printf("\t\tAddress: %s\n",
            inet_ntop(AF_INET,
               &((struct sockaddr_in *)(a->addr))->sin_addr,
               ipv4_buf, sizeof ipv4_buf));
        if (a->netmask)
          printf("\t\tNetmask: %s\n",
            inet_ntop(AF_INET,
               &((struct sockaddr_in *)(a->netmask))->sin_addr,
               ipv4_buf, sizeof ipv4_buf));
        if (a->broadaddr)
          printf("\t\tBroadcast Address: %s\n",
            inet_ntop(AF_INET,
               &((struct sockaddr_in *)(a->broadaddr))->sin_addr,
               ipv4_buf, sizeof ipv4_buf));
        if (a->dstaddr)
          printf("\t\tDestination Address: %s\n",
            inet_ntop(AF_INET,
               &((struct sockaddr_in *)(a->dstaddr))->sin_addr,
               ipv4_buf, sizeof ipv4_buf));
        break;
#ifdef INET6
      case AF_INET6:
        printf("\tAddress Family: AF_INET6\n");
        if (a->addr)
          printf("\t\tAddress: %s\n",
            inet_ntop(AF_INET6,
               ((struct sockaddr_in6 *)(a->addr))->sin6_addr.s6_addr,
               ipv6_buf, sizeof ipv6_buf));
        if (a->netmask)
          printf("\t\tNetmask: %s\n",
            inet_ntop(AF_INET6,
              ((struct sockaddr_in6 *)(a->netmask))->sin6_addr.s6_addr,
               ipv6_buf, sizeof ipv6_buf));
        if (a->broadaddr)
          printf("\t\tBroadcast Address: %s\n",
            inet_ntop(AF_INET6,
              ((struct sockaddr_in6 *)(a->broadaddr))->sin6_addr.s6_addr,
               ipv6_buf, sizeof ipv6_buf));
        if (a->dstaddr)
          printf("\t\tDestination Address: %s\n",
            inet_ntop(AF_INET6,
              ((struct sockaddr_in6 *)(a->dstaddr))->sin6_addr.s6_addr,
               ipv6_buf, sizeof ipv6_buf));
        break;
#endif
      default:
        printf("\tAddress Family: Unknown (%d)\n", a->addr->sa_family);
        break;
      }
    else
    {
      fprintf(stderr, "\tWarning: a->addr is NULL, skipping this address.\n");
      status = 0;
    }
  }
  printf("\n");
  return status;
}


