#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "../src/bpf_usdt_driver.h"

#define DEVICE_FILE_NAME "/dev/bpf_usdt"

int main() {
  int file_desc, ret_val;
  BpfUsdtProbe probe = {
    .module = "teste",
    .pid = 123,
    .provider = "lorem_ipsum",
    .probe = "dolore",
  };
  file_desc = open(DEVICE_FILE_NAME, 0);

  ret_val = ioctl(file_desc, BPF_USDT_ADD, &probe);
  if(ret_val < 0) {
    printf("Fudeu, error.\n");
  } else {
    printf("OMG, add OK\n");
  }

  return ret_val;
}
