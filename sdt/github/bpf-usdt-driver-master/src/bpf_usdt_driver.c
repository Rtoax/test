#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>		/* kmalloc() */
#include "bpf_usdt_driver.h"

MODULE_LICENSE("GPL");


int dev_major = 0;
int dev_minor = 0;
int dev_count = 1;

struct cdev cdev_;

BpfUsdtProbe *probesList[MAX_PROBES];

int bpf_usdt_open(struct inode *inode, struct file *filp);
int bpf_usdt_release(struct inode *inode, struct file *filp);
static long bpf_usdt_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations bpf_usdt_proc_ops = {
  .owner   = THIS_MODULE,
	.open    = bpf_usdt_open,
  .release = bpf_usdt_release,
  .unlocked_ioctl = bpf_usdt_ioctl
};

static long bpf_usdt_ioctl_add(void *ptr) {
  int i, result=0;
  BpfUsdtProbe *probe;
  for(i=0; i < MAX_PROBES; i++) {
    if(probesList[i] == NULL) {
      probe = kmalloc(sizeof(BpfUsdtProbe), GFP_KERNEL);
      copy_from_user(probe, ptr, sizeof(BpfUsdtProbe));
      probesList[i] = probe;
      printk(KERN_ALERT "Probe added: %s %d %s %s 0x%x\n", probe->module, probe->pid, probe->provider, probe->probe, probe->addr);
      result = i;
      break;
    }
  }
  return result;
}

static long bpf_usdt_ioctl_readall(void *ptr) {
  BpfUsdtProbe *availableProbes;
  int i, x, count=0;

  for(i=0; i < MAX_PROBES; i++) {
    if(probesList[i] != NULL) {
      count++;
    }
  }

  availableProbes = kmalloc(sizeof(BpfUsdtProbe) * count, GFP_KERNEL);

  x = 0;
  for(i=0; i < MAX_PROBES; i++) {
    if(probesList[i] != NULL) {
      memcpy(&(availableProbes[x]), probesList[i], sizeof(BpfUsdtProbe));
      x++;
    }
  }

  copy_to_user(ptr, availableProbes, sizeof(BpfUsdtProbe) * count);

  kfree(availableProbes);
  return count;
}

static long bpf_usdt_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
  long result=0;
  switch(cmd) {
    case BPF_USDT_ADD:
      result = bpf_usdt_ioctl_add((void *) arg);
      break;
    case BPF_USDT_DELETE:
      break;
    case BPF_USDT_READALL:
      result = bpf_usdt_ioctl_readall((void *)arg);
      break;
    default:
      return -ENOTTY;
  }
  return result;
}


int bpf_usdt_open(struct inode *inode, struct file *filp) {
  return 0;
}

int bpf_usdt_release(struct inode *inode, struct file *filp) {
  return 0;
}

static int __init bpf_usdt_init(void) {
  int i;
  int result;
  dev_t dev=0;

  // Create clear stateon
  for(i=0; i < MAX_PROBES; i++) {
    probesList[i] = NULL;
  }

  result = alloc_chrdev_region(&dev, dev_minor, dev_count, "bpf_usdt");
  if(result != 0) {
   printk(KERN_ALERT "Deu ruim\n");
   return result;
  }
  dev_major = MAJOR(dev);

  cdev_init(&cdev_, &bpf_usdt_proc_ops);
  cdev_.owner = THIS_MODULE;
  cdev_.ops = &bpf_usdt_proc_ops;
  result = cdev_add(&cdev_, dev, 1);
  if(result != 0) {
   printk(KERN_ALERT "Deu ruim 2\n");
   return result;
  }

  printk(KERN_ALERT "Hello, USDT: %d %d\n", dev_major, dev_minor);
  return 0;
}

static void bpf_usdt_exit(void) {
  int i;
  cdev_del(&cdev_);
  unregister_chrdev_region(MKDEV(dev_major, dev_minor), dev_count);
  // Create clear state
  for(i=0; i < MAX_PROBES; i++) {
    if(probesList[i] != NULL) {
      kfree(probesList[i]);
      probesList[i] = NULL;
    }
  }

  printk(KERN_ALERT "Goodbye, eBPF: %d %d\n", dev_major, dev_minor);
}

module_init(bpf_usdt_init);
module_exit(bpf_usdt_exit);
