#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "../src/bpf_usdt_driver.h"

#define DEVICE_FILE_NAME "/dev/bpf_usdt"

int main() {
  int file_desc, ret_val, i;
  BpfUsdtProbe probes[MAX_PROBES];
  file_desc = open(DEVICE_FILE_NAME, 0);

  ret_val = ioctl(file_desc, BPF_USDT_READALL, probes);
  if(ret_val < 0) {
    printf("Fudeu\n");
    return ret_val;
  }

  for(i=0; i < ret_val; i++) {
    printf("OMG: %s %d %s %s\n", probes[i].module, probes[i].pid, probes[i].provider, probes[i].probe);
  }
  return 0;
}
