#ifndef BPF_USDT_DRIVER_H_
#define BPF_USDT_DRIVER_H_

#include <linux/types.h>
#include <linux/ioctl.h>


#define MAX_PROBES 10
#define STR_LENGTH 256

typedef struct BpfUsdtProbe_ {
  char module[STR_LENGTH];
  pid_t pid;
  char provider[STR_LENGTH];
  char probe[STR_LENGTH];
  unsigned long addr;
} BpfUsdtProbe;

  // ************ //
 // IOCTL macros //
// ************ //
#define BPF_USDT_IOC_MAGIC  'U'

// #define BPF_USDT_IOCRESET    _IO(BPF_USDT_IOC_MAGIC, 0)

#define BPF_USDT_ADD      _IOW(BPF_USDT_IOC_MAGIC,  1, int)
#define BPF_USDT_DELETE   _IOW(BPF_USDT_IOC_MAGIC,   2, int)
#define BPF_USDT_READALL  _IOR(BPF_USDT_IOC_MAGIC,  3, int)

#endif
