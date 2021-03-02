/* 
 * Purpose:
 *   Check kernel syscall table against system map.
 *
 * Compile:
 *   gcc -O2 -Wall -o kern_check kern_check.c
 *
 * Usage (only the superuser can use this program !!!):
 *   kern_check [-v | --verbose] /path/to/System.map
 *
 * Diagnostics:
 *   Appropriate messages are printed to stderr if:
 *    - kernel modifications are found
 *    - kern_check thinks that you are using the wrong System.map
 *
 * Exit status: 
 *   success (0) if no kernel modifications are found
 *   error   (1) otherwise
 *
 * Example (sys_socketcall hijacked by a kernel module):
 *
 *   root#  kern_check /boot/System.map
 *   WARNING (kernel) 0xc841804c != 0xc0185ff4 (map) [sys_socketcall]
 *
 */

/*   Nov  7 21:22:11 CET 2006
 *
 *  - fix problem with 2.6.17+ kernel
 *
 *  Jun 16 22:17:32 CEST 2006
 *
 *  - fall back on mmap() if read() fails for /dev/kmem
 *  - syscall table for x86_64 (but reading from /dev/kmem fails for some reason)
 *
 *  Wed Jan 19 21:26:34 CET 2005
 *
 * - Fix off-by-one bug leading to a spurious error message 
 *   (noticed by M. Naeslund)
 *
 * Thu Jul 15 12:10:13 CEST 2004
 *
 * - remove the #include <linux/module.h>
 * - add 2.6 syscalls
 * - add code to fix syscall table (3 old syscalls) for 2.6
 * - don't rely on syscall 0 == sys_ni_syscall 
 * - break at syscall 255 for 2.4 (kernel table has only 256 entries)
 *
 * Thu Feb  5 16:51:13 CET 2004
 *
 * - check the proc filesystem lookup function to detect adore-ng
 *
 * Mon Sep 29 18:43:55 CEST 2003
 *
 * - fix bug (kaddr2 not used if kaddr == -1
 * - add suckit detection
 */

/* Copyright (C) 2001,2003,2004 Rainer Wichmann                            */
/*                                                                         */
/*  This program is free software; you can redistribute it                 */
/*  and/or modify                                                          */
/*  it under the terms of the GNU General Public License as                */
/*  published by                                                           */
/*  the Free Software Foundation; either version 2 of the License, or      */
/*  (at your option) any later version.                                    */
/*                                                                         */
/*  This program is distributed in the hope that it will be useful,        */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*  GNU General Public License for more details.                           */
/*                                                                         */
/*  You should have received a copy of the GNU General Public License      */
/*  along with this program; if not, write to the Free Software            */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */


/* for memmem */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/utsname.h>
#include <sys/mman.h>

static int have_warnings = EXIT_SUCCESS;
static int verbose = 0;
static int debug   = 0;
static int kvers_major = 2;
static int kvers_minor = 6;
static int kvers_micro = 15;


#define SH_MAXCALLS 512

/* 

the following two routines are taken from the SUCKIT rootkit, which
is distributed under the GPL:

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* SUCKIT v1.1c - New, singing, dancing, world-smashing rewtkit  *
* (c)oded by sd@sf.cz & devik@cdi.cz, 2001                      *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

struct idtr {
        unsigned short  limit;
        unsigned int    base;
} __attribute__ ((packed));

struct idt {
        unsigned short  off1;
        unsigned short  sel;
        unsigned char   none, flags;
        unsigned short  off2;
} __attribute__ ((packed));

/* simple fn which reads some bytes from /dev/kmem 
 */
unsigned long   loc_rkm_old (int fd, char *buf, size_t off, 
			     unsigned int size)
{
  if (lseek(fd, off, 0) != off)
    {
      // perror("loc_rkm_old: lseek");
      return 0;
    }
  if (read(fd, buf, size) != size)
    {
      // perror("loc_rkm_old: read");
      return 0;
    }
  return size;
}

/* simple fn which reads size bytes at offset off from /dev/kmem 
 *
 * This one uses mmap() rather than read(). However, on kernels
 * where read() works, mmap() will cause a segfault, so by default
 * we first try read(), and then mmap().
 */
unsigned long   loc_rkm (int fd, char * buf, size_t off, 
			 unsigned int size)
{
  size_t    moff, roff;
  size_t     sz = getpagesize(); /* unistd.h */

  char * kmap;

  unsigned long  ret_old = loc_rkm_old (fd, buf, off, size); 
  if (ret_old != 0)
    return ret_old;

  moff = ((size_t)(off/sz)) * sz;                 /* lower page boundary */
  roff = off - moff;    /* off relative to lower address of mmapped area */

  kmap = mmap(0, size+sz, PROT_READ, MAP_PRIVATE, fd, moff);/* sys/mman.h */
  
  if (kmap == MAP_FAILED)
    {
      perror("loc_rkm: mmap");
      return 0;
    }

  memcpy (buf, &kmap[roff], size);
      
  if (munmap(kmap, size) != 0)
    {
      perror("loc_rkm: munmap");
      return 0;
    }

  return size;
}

/* this fn tunnels out address of sys_call_table[] off int 80h */
#define INT80_LEN       128
#define SYSCALL_INTERRUPT       0x80
unsigned long   get_sct (unsigned long *i80)
{
  struct idtr     idtr;
  struct idt      idt;
  int             kmem;
  size_t          sys_call_off;
  char          * p;
  unsigned long   ret = 0;
  char            sc_asm[INT80_LEN];

  /* open kmem 
   */
  kmem = open("/dev/kmem", O_RDONLY, 0);
  if (kmem < 0) 
    return 0;
 
 /* well let's read IDTR 
  */
  asm("sidt %0" : "=m" (idtr));

  /* read-in IDT for 0x80 vector (syscall-gate) 
   */
  if (!loc_rkm(kmem, (char *)&idt, idtr.base + 8 * SYSCALL_INTERRUPT,
	       sizeof(idt)))
    return 0;

  sys_call_off = (idt.off2 << 16) | idt.off1;
  if (!loc_rkm(kmem, (char *)&sc_asm, sys_call_off, INT80_LEN))
    return 0;
  close(kmem);

  /* we have syscall routine address now, look for syscall table
     dispatch (indirect call) */
  
  p = memmem(sc_asm, INT80_LEN, "\xff\x14\x85", 3);
  if (p) {
    p += 3;
    memcpy(&ret, p, sizeof(unsigned long)); 
    *i80 = (unsigned long) (p - sc_asm + sys_call_off);
    return ret;
  }
  return ret;
}


int query_module(const char *name, int which, void *buf, size_t bufsize,
                 size_t *ret);

struct new_module_symbol
{
  unsigned long value;
  unsigned long name;
};

#define QM_SYMBOLS     4


/* x86_64 sys_call_table for kernel 2.6.8
 */
char * callx_2p6[] = {
  	/* 000 */	"sys_read",
	/* 001 */	"sys_write",
	/* 002 */	"sys_open",
	/* 003 */	"sys_close",
	/* 004 */	"sys_newstat",
	/* 005 */	"sys_newfstat",
	/* 006 */	"sys_newlstat",
	/* 007 */	"sys_poll",
	/* 008 */	"sys_lseek",
	/* 009 */	"sys_mmap",
	/* 010 */	"sys_mprotect",
	/* 011 */	"sys_munmap",
	/* 012 */	"sys_brk",
	/* 013 */	"sys_rt_sigaction",
	/* 014 */	"sys_rt_sigprocmask",
	/* 015 */	"stub_rt_sigreturn",
	/* 016 */	"sys_ioctl",
	/* 017 */	"sys_pread64",
	/* 018 */	"sys_pwrite64",
	/* 019 */	"sys_readv",
	/* 020 */	"sys_writev",
	/* 021 */	"sys_access",
	/* 022 */	"sys_pipe",
	/* 023 */	"sys_select",
	/* 024 */	"sys_sched_yield",
	/* 025 */	"sys_mremap",
	/* 026 */	"sys_msync",
	/* 027 */	"sys_mincore",
	/* 028 */	"sys_madvise",
	/* 029 */	"sys_shmget",
	/* 030 */	"wrap_sys_shmat",
	/* 031 */	"sys_shmctl",
	/* 032 */	"sys_dup",
	/* 033 */	"sys_dup2",
	/* 034 */	"sys_pause",
	/* 035 */	"sys_nanosleep",
	/* 036 */	"sys_getitimer",
	/* 037 */	"sys_alarm",
	/* 038 */	"sys_setitimer",
	/* 039 */	"sys_getpid",
	/* 040 */	"sys_sendfile64",
	/* 041 */	"sys_socket",
	/* 042 */	"sys_connect",
	/* 043 */	"sys_accept",
	/* 044 */	"sys_sendto",
	/* 045 */	"sys_recvfrom",
	/* 046 */	"sys_sendmsg",
	/* 047 */	"sys_recvmsg",
	/* 048 */	"sys_shutdown",
	/* 049 */	"sys_bind",
	/* 050 */	"sys_listen",
	/* 051 */	"sys_getsockname",
	/* 052 */	"sys_getpeername",
	/* 053 */	"sys_socketpair",
	/* 054 */	"sys_setsockopt",
	/* 055 */	"sys_getsockopt",
	/* 056 */	"stub_clone",
	/* 057 */	"stub_fork",
	/* 058 */	"stub_vfork",
	/* 059 */	"stub_execve",
	/* 060 */	"sys_exit",
	/* 061 */	"sys_wait4",
	/* 062 */	"sys_kill",
	/* 063 */	"sys_uname",
	/* 064 */	"sys_semget",
	/* 065 */	"sys_semop",
	/* 066 */	"sys_semctl",
	/* 067 */	"sys_shmdt",
	/* 068 */	"sys_msgget",
	/* 069 */	"sys_msgsnd",
	/* 070 */	"sys_msgrcv",
	/* 071 */	"sys_msgctl",
	/* 072 */	"sys_fcntl",
	/* 073 */	"sys_flock",
	/* 074 */	"sys_fsync",
	/* 075 */	"sys_fdatasync",
	/* 076 */	"sys_truncate",
	/* 077 */	"sys_ftruncate",
	/* 078 */	"sys_getdents",
	/* 079 */	"sys_getcwd",
	/* 080 */	"sys_chdir",
	/* 081 */	"sys_fchdir",
	/* 082 */	"sys_rename",
	/* 083 */	"sys_mkdir",
	/* 084 */	"sys_rmdir",
	/* 085 */	"sys_creat",
	/* 086 */	"sys_link",
	/* 087 */	"sys_unlink",
	/* 088 */	"sys_symlink",
	/* 089 */	"sys_readlink",
	/* 090 */	"sys_chmod",
	/* 091 */	"sys_fchmod",
	/* 092 */	"sys_chown",
	/* 093 */	"sys_fchown",
	/* 094 */	"sys_lchown",
	/* 095 */	"sys_umask",
	/* 096 */	"sys_gettimeofday",
	/* 097 */	"sys_getrlimit",
	/* 098 */	"sys_getrusage",
	/* 099 */	"sys_sysinfo",
	/* 100 */	"sys_times",
	/* 101 */	"sys_ptrace",
	/* 102 */	"sys_getuid",
	/* 103 */	"sys_syslog",
	/* 104 */	"sys_getgid",
	/* 105 */	"sys_setuid",
	/* 106 */	"sys_setgid",
	/* 107 */	"sys_geteuid",
	/* 108 */	"sys_getegid",
	/* 109 */	"sys_setpgid",
	/* 110 */	"sys_getppid",
	/* 111 */	"sys_getpgrp",
	/* 112 */	"sys_setsid",
	/* 113 */	"sys_setreuid",
	/* 114 */	"sys_setregid",
	/* 115 */	"sys_getgroups",
	/* 116 */	"sys_setgroups",
	/* 117 */	"sys_setresuid",
	/* 118 */	"sys_getresuid",
	/* 119 */	"sys_setresgid",
	/* 120 */	"sys_getresgid",
	/* 121 */	"sys_getpgid",
	/* 122 */	"sys_setfsuid",
	/* 123 */	"sys_setfsgid",
	/* 124 */	"sys_getsid",
	/* 125 */	"sys_capget",
	/* 126 */	"sys_capset",
	/* 127 */	"sys_rt_sigpending",
	/* 128 */	"sys_rt_sigtimedwait",
	/* 129 */	"sys_rt_sigqueueinfo",
	/* 130 */	"stub_rt_sigsuspend",
	/* 131 */	"stub_sigaltstack",
	/* 132 */	"sys_utime",
	/* 133 */	"sys_mknod",
	/* 134 */	"sys_uselib",
	/* 135 */	"sys_personality",
	/* 136 */	"sys_ustat",
	/* 137 */	"sys_statfs",
	/* 138 */	"sys_fstatfs",
	/* 139 */	"sys_sysfs",
	/* 140 */	"sys_getpriority",
	/* 141 */	"sys_setpriority",
	/* 142 */	"sys_sched_setparam",
	/* 143 */	"sys_sched_getparam",
	/* 144 */	"sys_sched_setscheduler",
	/* 145 */	"sys_sched_getscheduler",
	/* 146 */	"sys_sched_get_priority_max",
	/* 147 */	"sys_sched_get_priority_min",
	/* 148 */	"sys_sched_rr_get_interval",
	/* 149 */	"sys_mlock",
	/* 150 */	"sys_munlock",
	/* 151 */	"sys_mlockall",
	/* 152 */	"sys_munlockall",
	/* 153 */	"sys_vhangup",
	/* 154 */	"sys_modify_ldt",
	/* 155 */	"sys_pivot_root",
	/* 156 */	"sys_sysctl",
	/* 157 */	"sys_prctl",
	/* 158 */	"sys_arch_prctl",
	/* 159 */	"sys_adjtimex",
	/* 160 */	"sys_setrlimit",
	/* 161 */	"sys_chroot",
	/* 162 */	"sys_sync",
	/* 163 */	"sys_acct",
	/* 164 */	"sys_settimeofday",
	/* 165 */	"sys_mount",
	/* 166 */	"sys_umount",
	/* 167 */	"sys_swapon",
	/* 168 */	"sys_swapoff",
	/* 169 */	"sys_reboot",
	/* 170 */	"sys_sethostname",
	/* 171 */	"sys_setdomainname",
	/* 172 */	"stub_iopl",
	/* 173 */	"sys_ioperm",
	/* 174 */	"sys_ni_syscall",
	/* 175 */	"sys_init_module",
	/* 176 */	"sys_delete_module",
	/* 177 */	"sys_ni_syscall",
	/* 178 */	"sys_ni_syscall",
	/* 179 */	"sys_quotactl",
	/* 180 */	"sys_nfsservctl",
	/* 181 */	"sys_ni_syscall",
	/* 182 */	"sys_ni_syscall",
	/* 183 */	"sys_ni_syscall",
	/* 184 */	"sys_ni_syscall",
	/* 185 */	"sys_ni_syscall",
	/* 186 */	"sys_gettid",
	/* 187 */	"sys_readahead",
	/* 188 */	"sys_setxattr",
	/* 189 */	"sys_lsetxattr",
	/* 190 */	"sys_fsetxattr",
	/* 191 */	"sys_getxattr",
	/* 192 */	"sys_lgetxattr",
	/* 193 */	"sys_fgetxattr",
	/* 194 */	"sys_listxattr",
	/* 195 */	"sys_llistxattr",
	/* 196 */	"sys_flistxattr",
	/* 197 */	"sys_removexattr",
	/* 198 */	"sys_lremovexattr",
	/* 199 */	"sys_fremovexattr",
	/* 200 */	"sys_tkill",
	/* 201 */	"sys_time64",
	/* 202 */	"sys_futex",
	/* 203 */	"sys_sched_setaffinity",
	/* 204 */	"sys_sched_getaffinity",
	/* 205 */	"sys_ni_syscall",
	/* 206 */	"sys_io_setup",
	/* 207 */	"sys_io_destroy",
	/* 208 */	"sys_io_getevents",
	/* 209 */	"sys_io_submit",
	/* 210 */	"sys_io_cancel",
	/* 211 */	"sys_ni_syscall",
	/* 212 */	"sys_lookup_dcookie",
	/* 213 */	"sys_epoll_create",
	/* 214 */	"sys_ni_syscall",
	/* 215 */	"sys_ni_syscall",
	/* 216 */	"sys_remap_file_pages",
	/* 217 */	"sys_getdents64",
	/* 218 */	"sys_set_tid_address",
	/* 219 */	"sys_restart_syscall",
	/* 220 */	"sys_semtimedop",
	/* 221 */	"sys_fadvise64",
	/* 222 */	"sys_timer_create",
	/* 223 */	"sys_timer_settime",
	/* 224 */	"sys_timer_gettime",
	/* 225 */	"sys_timer_getoverrun",
	/* 226 */	"sys_timer_delete",
	/* 227 */	"sys_clock_settime",
	/* 228 */	"sys_clock_gettime",
	/* 229 */	"sys_clock_getres",
	/* 230 */	"sys_clock_nanosleep",
	/* 231 */	"sys_exit_group",
	/* 232 */	"sys_epoll_wait",
	/* 233 */	"sys_epoll_ctl",
	/* 234 */	"sys_tgkill",
	/* 235 */	"sys_utimes",
	/* 236 */	"sys_ni_syscall",
	/* 237 */	"sys_mbind",
	/* 238 */	"sys_set_mempolicy",
	/* 239 */	"sys_get_mempolicy",
	/* 240 */	"sys_mq_open",
	/* 241 */	"sys_mq_unlink",
	/* 242 */	"sys_mq_timedsend",
	/* 243 */	"sys_mq_timedreceive",
	/* 244 */	"sys_mq_notify",
	/* 245 */	"sys_mq_getsetattr",
	/* 246 */	"sys_ni_syscall",
	/* 247 */	"sys_waitid",
  NULL
};

/* i386 sys_call_table for kernel 2.4.x
 */
char * callz_2p4[] = {
    "sys_ni_syscall",    /* 0 - old setup() system call*/
    "sys_exit",
    "sys_fork",
    "sys_read",
    "sys_write",
    "sys_open",        /* 5 */
    "sys_close",
    "sys_waitpid",
    "sys_creat",
    "sys_link",
    "sys_unlink",        /* 10 */
    "sys_execve",
    "sys_chdir",
    "sys_time",
    "sys_mknod",
    "sys_chmod",        /* 15 */
    "sys_lchown16",
    "sys_ni_syscall",                /* old break syscall holder */
    "sys_stat",
    "sys_lseek",
    "sys_getpid",        /* 20 */
    "sys_mount",
    "sys_oldumount",
    "sys_setuid16",
    "sys_getuid16",
    "sys_stime",        /* 25 */
    "sys_ptrace",
    "sys_alarm",
    "sys_fstat",
    "sys_pause",
    "sys_utime",        /* 30 */
    "sys_ni_syscall",                /* old stty syscall holder */
    "sys_ni_syscall",                /* old gtty syscall holder */
    "sys_access",
    "sys_nice",
    "sys_ni_syscall",    /* 35 */        /* old ftime syscall holder */
    "sys_sync",
    "sys_kill",
    "sys_rename",
    "sys_mkdir",
    "sys_rmdir",        /* 40 */
    "sys_dup",
    "sys_pipe",
    "sys_times",
    "sys_ni_syscall",                /* old prof syscall holder */
    "sys_brk",        /* 45 */
    "sys_setgid16",
    "sys_getgid16",
    "sys_signal",
    "sys_geteuid16",
    "sys_getegid16",    /* 50 */
    "sys_acct",
    "sys_umount",                    /* recycled never used  phys() */
    "sys_ni_syscall",                /* old lock syscall holder */
    "sys_ioctl",
    "sys_fcntl",        /* 55 */
    "sys_ni_syscall",                /* old mpx syscall holder */
    "sys_setpgid",
    "sys_ni_syscall",                /* old ulimit syscall holder */
    "sys_olduname",
    "sys_umask",        /* 60 */
    "sys_chroot",
    "sys_ustat",
    "sys_dup2",
    "sys_getppid",
    "sys_getpgrp",        /* 65 */
    "sys_setsid",
    "sys_sigaction",
    "sys_sgetmask",
    "sys_ssetmask",
    "sys_setreuid16",    /* 70 */
    "sys_setregid16",
    "sys_sigsuspend",
    "sys_sigpending",
    "sys_sethostname",
    "sys_setrlimit",    /* 75 */
    "sys_old_getrlimit",
    "sys_getrusage",
    "sys_gettimeofday",
    "sys_settimeofday",
    "sys_getgroups16",    /* 80 */
    "sys_setgroups16",
    "old_select",
    "sys_symlink",
    "sys_lstat",
    "sys_readlink",        /* 85 */
    "sys_uselib",
    "sys_swapon",
    "sys_reboot",
    "old_readdir",
    "old_mmap",        /* 90 */
    "sys_munmap",
    "sys_truncate",
    "sys_ftruncate",
    "sys_fchmod",
    "sys_fchown16",        /* 95 */
    "sys_getpriority",
    "sys_setpriority",
    "sys_ni_syscall",                /* old profil syscall holder */
    "sys_statfs",
    "sys_fstatfs",        /* 100 */
    "sys_ioperm",
    "sys_socketcall",
    "sys_syslog",
    "sys_setitimer",
    "sys_getitimer",    /* 105 */
    "sys_newstat",
    "sys_newlstat",
    "sys_newfstat",
    "sys_uname",
    "sys_iopl",        /* 110 */
    "sys_vhangup",
    "sys_ni_syscall",    /* old idle system call */
    "sys_vm86old",
    "sys_wait4",
    "sys_swapoff",        /* 115 */
    "sys_sysinfo",
    "sys_ipc",
    "sys_fsync",
    "sys_sigreturn",
    "sys_clone",        /* 120 */
    "sys_setdomainname",
    "sys_newuname",
    "sys_modify_ldt",
    "sys_adjtimex",
    "sys_mprotect",        /* 125 */
    "sys_sigprocmask",
    "sys_create_module",
    "sys_init_module",
    "sys_delete_module",
    "sys_get_kernel_syms",    /* 130 */
    "sys_quotactl",
    "sys_getpgid",
    "sys_fchdir",
    "sys_bdflush",
    "sys_sysfs",        /* 135 */
    "sys_personality",
    "sys_ni_syscall",    /* for afs_syscall */
    "sys_setfsuid16",
    "sys_setfsgid16",
    "sys_llseek",        /* 140 */
    "sys_getdents",
    "sys_select",
    "sys_flock",
    "sys_msync",
    "sys_readv",        /* 145 */
    "sys_writev",
    "sys_getsid",
    "sys_fdatasync",
    "sys_sysctl",
    "sys_mlock",        /* 150 */
    "sys_munlock",
    "sys_mlockall",
    "sys_munlockall",
    "sys_sched_setparam",
    "sys_sched_getparam",  /* 155 */
    "sys_sched_setscheduler",
    "sys_sched_getscheduler",
    "sys_sched_yield",
    "sys_sched_get_priority_max",
    "sys_sched_get_priority_min", /* 160 */
    "sys_sched_rr_get_interval",
    "sys_nanosleep",
    "sys_mremap",
    "sys_setresuid16",
    "sys_getresuid16",    /* 165 */
    "sys_vm86",
    "sys_query_module",
    "sys_poll",
    "sys_nfsservctl",
    "sys_setresgid16",    /* 170 */
    "sys_getresgid16",
    "sys_prctl",
    "sys_rt_sigreturn",
    "sys_rt_sigaction",
    "sys_rt_sigprocmask",    /* 175 */
    "sys_rt_sigpending",
    "sys_rt_sigtimedwait",
    "sys_rt_sigqueueinfo",
    "sys_rt_sigsuspend",
    "sys_pread",        /* 180 */
    "sys_pwrite",
    "sys_chown16",
    "sys_getcwd",
    "sys_capget",
    "sys_capset",      /* 185 */
    "sys_sigaltstack",
    "sys_sendfile",
    "sys_getpmsg",        /* streams1 */
    "sys_putpmsg",        /* streams2 */
    "sys_vfork",      /* 190 */
    "sys_getrlimit",
    "sys_mmap2",
    "sys_truncate64",
    "sys_ftruncate64",
    "sys_stat64",        /* 195 */
    "sys_lstat64",
    "sys_fstat64",
    "sys_lchown",
    "sys_getuid",
    "sys_getgid",        /* 200 */
    "sys_geteuid",
    "sys_getegid",
    "sys_setreuid",
    "sys_setregid",
    "sys_getgroups",    /* 205 */
    "sys_setgroups",
    "sys_fchown",
    "sys_setresuid",
    "sys_getresuid",
    "sys_setresgid",    /* 210 */
    "sys_getresgid",
    "sys_chown",
    "sys_setuid",
    "sys_setgid",
    "sys_setfsuid",        /* 215 */
    "sys_setfsgid",
    "sys_pivot_root",
    "sys_mincore",
    "sys_madvise",
    "sys_getdents64",    /* 220 */
    "sys_fcntl64",
    "sys_tux",           /* reserved for TUX */
    "sys_security",
    "sys_gettid",
    "sys_readahead",     /* 225 */
    "sys_setxattr",
    "sys_lsetxattr",
    "sys_fsetxattr",
    "sys_getxattr",
    "sys_lgetxattr",     /* 230 */
    "sys_fgetxattr",
    "sys_listxattr",
    "sys_llistxattr",
    "sys_flistxattr",
    "sys_removexattr",   /* 235 */
    "sys_lremovexattr",
    "sys_fremovexattr",
    "sys_tkill",
    "sys_sendfile64",
    "sys_futex",         /* 240 */
    "sys_sched_setaffinity",
    "sys_sched_getaffinity",
    "sys_set_thread_area",   /* new syscalls in 2.6 */
    "sys_get_thread_area",   
    "sys_io_setup",      /* 245 */
    "sys_io_destroy",        
    "sys_io_getevents",      
    "sys_io_submit",         
    "sys_io_cancel",         
    "sys_fadvise64",     /* 250 */
    "sys_ni_syscall",    
    "sys_exit_group",        
    "sys_lookup_dcookie",    
    "sys_epoll_create",      
    "sys_epoll_ctl",     /* 255 */
    "sys_epoll_wait",    
    "sys_remap_file_pages",  
    "sys_set_tid_address",   
    "sys_timer_create",  
    "sys_timer_settime", /* 260 */
    "sys_timer_gettime",     
    "sys_timer_getoverrun",   
    "sys_timer_delete", 
    "sys_clock_settime", 
    "sys_clock_gettime", /* 265 */
    "sys_clock_getres", 
    "sys_clock_nanosleep", 
    "sys_statfs64",         
    "sys_fstatfs64", 
    "sys_tgkill",        /* 270 */
    "sys_utimes",   
    "sys_fadvise64_64",
    "sys_vserver",
    NULL
};



/* i386 sys_call_table for kernel 2.2.x
 */
char * callz_2p2[]={
  "sys_ni_syscall",        /* 0 */
  "sys_exit",
  "sys_fork",
  "sys_read",
  "sys_write",
  "sys_open",              /* 5 */
  "sys_close",
  "sys_waitpid", 
  "sys_creat",
  "sys_link",
  "sys_unlink",              /* 10 */
  "sys_execve",
  "sys_chdir",
  "sys_time",
  "sys_mknod",
  "sys_chmod",              /* 15 */
  "sys_lchown",
  "sys_ni_syscall",
  "sys_stat",
  "sys_lseek",
  "sys_getpid",              /* 20 */
  "sys_mount",
  "sys_oldumount", 
  "sys_setuid",
  "sys_getuid",
  "sys_stime",              /* 25 */
  "sys_ptrace",
  "sys_alarm",
  "sys_fstat",
  "sys_pause",
  "sys_utime",              /* 30 */
  "sys_ni_syscall",
  "sys_ni_syscall",
  "sys_access",
  "sys_nice",
  "sys_ni_syscall",              /* 35 */
  "sys_sync",
  "sys_kill",
  "sys_rename",
  "sys_mkdir",
  "sys_rmdir",              /* 40 */
  "sys_dup",
  "sys_pipe",
  "sys_times",
  "sys_ni_syscall",
  "sys_brk",              /* 45 */
  "sys_setgid",
  "sys_getgid",
  "sys_signal",
  "sys_geteuid",
  "sys_getegid",              /* 50 */
  "sys_acct",
  "sys_umount",
  "sys_ni_syscall",
  "sys_ioctl",
  "sys_fcntl",              /* 55 */
  "sys_ni_syscall",
  "sys_setpgid",
  "sys_ni_syscall",
  "sys_olduname",
  "sys_umask",              /* 60 */
  "sys_chroot",
  "sys_ustat",
  "sys_dup2",
  "sys_getppid",
  "sys_getpgrp",              /* 65 */
  "sys_setsid",
  "sys_sigaction",
  "sys_sgetmask",
  "sys_ssetmask",
  "sys_setreuid",              /* 70 */
  "sys_setregid",
  "sys_sigsuspend",
  "sys_sigpending",
  "sys_sethostname",
  "sys_setrlimit",              /* 75 */
  "sys_getrlimit",
  "sys_getrusage",
  "sys_gettimeofday",
  "sys_settimeofday",
  "sys_getgroups",              /* 80 */
  "sys_setgroups",
  "old_select",
  "sys_symlink",
  "sys_lstat",
  "sys_readlink",              /* 85 */
  "sys_uselib",
  "sys_swapon",
  "sys_reboot",
  "old_readdir",
  "old_mmap",              /* 90 */
  "sys_munmap",
  "sys_truncate",
  "sys_ftruncate",
  "sys_fchmod",
  "sys_fchown",              /* 95 */
  "sys_getpriority",
  "sys_setpriority",
  "sys_ni_syscall",
  "sys_statfs",
  "sys_fstatfs",              /* 100 */
  "sys_ioperm",
  "sys_socketcall",
  "sys_syslog",
  "sys_setitimer",
  "sys_getitimer",              /* 105 */
  "sys_newstat",
  "sys_newlstat",
  "sys_newfstat",
  "sys_uname",
  "sys_iopl",              /* 110 */
  "sys_vhangup",
  "sys_idle",
  "sys_vm86old",
  "sys_wait4",
  "sys_swapoff",              /* 115 */
  "sys_sysinfo",
  "sys_ipc",
  "sys_fsync",
  "sys_sigreturn",
  "sys_clone",              /* 120 */
  "sys_setdomainname",
  "sys_newuname",
  "sys_modify_ldt",
  "sys_adjtimex",
  "sys_mprotect",              /* 125 */
  "sys_sigprocmask",
  "sys_create_module",
  "sys_init_module",
  "sys_delete_module",
  "sys_get_kernel_syms", /* 130 */
  "sys_quotactl",
  "sys_getpgid",
  "sys_fchdir",
  "sys_bdflush",
  "sys_sysfs",              /* 135 */
  "sys_personality",
  "sys_ni_syscall",
  "sys_setfsuid",
  "sys_setfsgid",
  "sys_llseek",              /* 140 */
  "sys_getdents",
  "sys_select",
  "sys_flock",
  "sys_msync",
  "sys_readv",              /* 145 */
  "sys_writev",
  "sys_getsid",
  "sys_fdatasync",
  "sys_sysctl",
  "sys_mlock",              /* 150 */
  "sys_munlock",
  "sys_mlockall",
  "sys_munlockall",
  "sys_sched_setparam", 
  "sys_sched_getparam",  /* 155 */
  "sys_sched_setscheduler",
  "sys_sched_getscheduler",
  "sys_sched_yield",
  "sys_sched_get_priority_max",
  "sys_sched_get_priority_min", /* 160 */
  "sys_sched_rr_get_interval",
  "sys_nanosleep",
  "sys_mremap",
  "sys_setresuid",
  "sys_getresuid",              /* 165 */
  "sys_vm86",
  "sys_query_module",
  "sys_poll",
  "sys_nfsservctl", 
  "sys_setresgid",              /* 170 */
  "sys_getresgid",
  "sys_prctl",
  "sys_rt_sigreturn",
  "sys_rt_sigaction",
  "sys_rt_sigprocmask", /* 175 */
  "sys_rt_sigpending",
  "sys_rt_sigtimedwait",
  "sys_rt_sigqueueinfo",
  "sys_rt_sigsuspend",
  "sys_pread",              /* 180 */
  "sys_pwrite",
  "sys_chown",
  "sys_getcwd",
  "sys_capget",
  "sys_capset",              /* 185 */
  "sys_sigaltstack",
  "sys_sendfile",
  "sys_ni_syscall",
  "sys_ni_syscall",
  "sys_vfork",              /* 190 */
  NULL
};



typedef struct _smap_entry {
  unsigned long addr;
  char          name[64];
} smap_entry;

unsigned long get_symbol_from_systemmap (char * systemmap, 
					 char * symbol, char flag)
{
  FILE * fp;
  char buf[512], addr[16], * p;
  unsigned long retval = 0;

  fp = fopen (systemmap, "r");

  if (!fp)
    {
      fprintf(stderr, "error opening <%s>\n", systemmap);
      perror("fill_smap: fopen");
      return 0;
    }
  while (fgets(buf, 512, fp) != NULL)
    {
      if (buf[9] != flag)
	continue;

      p = strchr(buf, '\n');
      if (p != NULL)
	*p = '\0';

      if (0 != strcmp(&buf[11], symbol))
	continue;

      addr[0] = '0'; addr[1] = 'x'; addr[2] = '\0';
      strncat(&addr[2], buf, 8);

      retval = strtoul(addr, NULL, 0);
      if (retval == ULONG_MAX)
	{
	  perror("fill_smap");
	  return -1;
	}
    }
  fclose(fp);
  return retval;
}

int fill_smap(char * systemmap, smap_entry * sh_smap, int num)
{
  FILE * fp;
  char buf[512], addr[16], name[128];
  int  i, j, count = 0;

  fp = fopen (systemmap, "r");

  if (!fp)
    {
      fprintf(stderr, "error opening <%s>\n", systemmap);
      perror("fill_smap: fopen");
      return -1;
    }

  while (fgets(buf, 512, fp) != NULL)
    {

      if (buf[9] != 'T')
	continue;
      if ( (buf[11]!='s' || buf[12]!='y' || buf[13]!='s' || buf[14]!='_') &&
	   (buf[11]!='o' || buf[12]!='l' || buf[13]!='d' || buf[14]!='_'))
	continue;

      for (i = 0; i < num; ++i)
	{
	  for (j = 0; j < 128; ++j)
	    {
	      if (buf[11+j] == '\n' || buf[11+j] == '\0')
		{
		  name[j] = '\0';
		  break;
		}
	      name[j] = buf[11+j];
	    }


	  if (0 == strcmp(name, sh_smap[i].name)) 
	    {
      
	      /* --- copy symbol address ---
	       */
	      addr[0] = '0'; addr[1] = 'x'; addr[2] = '\0';
	      strncat(&addr[2], buf, 8);
	      addr[10] = '\0';
	      sh_smap[i].addr = strtoul(addr, NULL, 0);
	      if (sh_smap[i].addr == ULONG_MAX)
		{
		  perror("fill_smap");
		  return -1;
		}

	      ++count;
	      /* break; */
      	    }
	}
    }
  fclose(fp);
  return count;
}


long my_address (char *sym_name)
{
  struct new_module_symbol *buf;
  size_t ret, j, size;
  long   retval;
  char   *p;

  buf = (struct new_module_symbol *) malloc (sizeof(struct new_module_symbol));

  if (buf == NULL)
    {
      fprintf(stderr, "Out of memory\n");
      return -1;
    }

  size = sizeof(struct new_module_symbol);

  while (0 != query_module(NULL, QM_SYMBOLS, buf,
                           size,
                           &ret))
    {
      if (errno == ENOSPC)
        {
          free(buf);
          size = ret;
          buf  = (struct new_module_symbol *) malloc (size);
          if (buf == NULL)
            {
	      fprintf(stderr, "Out of memory\n");
              return -1;
            }
        }
      else
        {
	  perror("kern_check: query_module");
          free(buf);
          return -1;
        }
    }

  for (j = 0; j < ret; ++j)
    {
      p = (char *) buf; p += buf[j].name;
      if(strstr(p, sym_name))
        {
          retval = (long) buf[j].value;
          free(buf);
          return retval;
        }
    }

  free(buf);
  return -1;
}


void usage(int flag)
{
  printf("\n");
  printf("Usage: kern_check [-v | --verbose] /path/to/System.map\n");
  printf("       kern_check [-h | --help]\n");
  printf("\n");
  printf("       You need superuser privileges to use this program,\n");
  printf("       because only the superuser can read from /dev/kmem.\n");
  printf("\n");
  exit(flag);
}

#if 0
#define __KERNEL__
#include <linux/types.h>
#undef  __KERNEL__
#endif

/* 
 * from include/linux/fs/proc_fs.h 

typedef int (read_proc_t)(char *page, char **start, off_t off,
                          int count, int *eof, void *data);
typedef int (write_proc_t)(char *file, const char *buffer,
                           unsigned long count, void *data);
typedef int (get_info_t)(char *, char **, off_t, int);
 */

struct inode_operations {
  int (*create) (int *,int *,int);
  int * (*lookup) (int *,int *);
  int (*link) (int *,int *,int *);
  int (*unlink) (int *,int *);
  int (*symlink) (int *,int *,const char *);
  int (*mkdir) (int *,int *,int);
  int (*rmdir) (int *,int *);
  int (*mknod) (int *,int *,int,int);
  int (*rename) (int *, int *,
		 int *, int *);
  int (*readlink) (int *, char *,int);
  int (*follow_link) (int *, int *);
  void (*truncate) (int *);
  int (*permission) (int *, int);
  int (*revalidate) (int *);
  /*
    int (*setattr) (int *, int *);
    int (*getattr) (int *, int *);
    int (*setxattr) (int *, const char *, void *, size_t, int);
    ssize_t (*getxattr) (int *, const char *, void *, size_t);
    ssize_t (*listxattr) (int *, char *, size_t);
    int (*removexattr) (int *, const char *);
  */
};

struct file_operations {
  int (*create) (int *,int *,int);
};

struct proc_dir_entry_old {
  unsigned short low_ino;
  unsigned short namelen;
  const char * name;
  mode_t mode;
  nlink_t nlink;
  uid_t uid;
  gid_t gid;
  unsigned long size; 
  struct inode_operations * proc_iops;
  struct file_operations * proc_fops;
  /*
  get_info_t *get_info;
  struct module *owner;
  struct proc_dir_entry *next, *parent, *subdir;
  void *data;
  read_proc_t *read_proc;
  write_proc_t *write_proc;
  atomic_t count;         
  int deleted;  
  */          
};

struct proc_dir_entry_2_17 {
  unsigned short low_ino;
  unsigned short namelen;
  const char * name;
  mode_t mode;
  nlink_t nlink;
  uid_t uid;
  gid_t gid;

  /* size is loff_t in 2.6.17+ kernels */
  unsigned long dummy; 

  unsigned long size; 
  struct inode_operations * proc_iops;
  struct file_operations * proc_fops;
  /*
  get_info_t *get_info;
  struct module *owner;
  struct proc_dir_entry *next, *parent, *subdir;
  void *data;
  read_proc_t *read_proc;
  write_proc_t *write_proc;
  atomic_t count;         
  int deleted;  
  */          
};

int check_adore_ng (char * systemmap)
{
  int kd;
  int result;
  unsigned long proc_root;
  unsigned long proc_root_iops;
  unsigned long proc_root_lookup;
  struct inode_operations proc_root_inode;

  struct proc_dir_entry_old  proc_root_dir_old;
  struct proc_dir_entry_2_17 proc_root_dir_new;

  int flag = 0;

  if (kvers_major > 2 || (kvers_major == 2 && kvers_minor > 6) ||
      (kvers_major == 2 && kvers_minor == 6 && kvers_micro >= 17))
    {
      if (debug)
	printf("Using code for 2.6.17+ kernel\n");
      flag = 1;
    }


  proc_root =  get_symbol_from_systemmap (systemmap, 
					  "proc_root", 'D');
  if (proc_root == 0) 
    {
      proc_root =  get_symbol_from_systemmap (systemmap, 
					      "proc_root", 'd');
    }

  proc_root_lookup =  get_symbol_from_systemmap (systemmap, 
						 "proc_root_lookup", 't');
  if (proc_root_lookup == 0) 
    {
      proc_root_lookup =  get_symbol_from_systemmap (systemmap, 
						     "proc_root_lookup", 'T');
    }

  proc_root_iops =  get_symbol_from_systemmap (systemmap, 
					       "proc_root_inode_operations", 
					       'd');
  if (proc_root_iops == 0) 
    {
      proc_root_iops = get_symbol_from_systemmap (systemmap, 
						  "proc_root_inode_operations",
						  'D');
    }

  if (debug)
    {
      fprintf(stdout,  "proc_root        %#lx\n", proc_root);
      fprintf(stdout,  "proc_root_iops   %#lx\n", proc_root_iops);
      fprintf(stdout,  "proc_root_lookup %#lx\n", proc_root_lookup);
    }

  if (proc_root == 0)
    {
      fprintf(stderr, "Failed to get symbol 'proc_root' from system map %s\n",
	      systemmap);
      return -1;
    }
  if (proc_root_iops == 0)
    {
      fprintf(stderr, "Failed to get symbol 'proc_root_inode_operations' from system map %s\n",
	      systemmap);
      return -1;
    }
  if (proc_root_lookup == 0)
    {
      fprintf(stderr, "Failed to get symbol 'proc_root_lookup' from system map %s\n",
	      systemmap);
      return -1;
    }
      
  kd = open("/dev/kmem", O_RDONLY, 0);

  if (kd < 0)
    {
      perror("kern_check: open(/dev/kmem)");
      return -1;
    }

  if (flag == 0)
    result = loc_rkm(kd, (char *) &proc_root_dir_old, proc_root, sizeof(proc_root_dir_old));
  else
    result = loc_rkm(kd, (char *) &proc_root_dir_new, proc_root, sizeof(proc_root_dir_new));
    
      
  if (!result)
    {
      perror("kern_check: read proc_root");
      return -1;
    }

  if (!loc_rkm(kd, (char *) &proc_root_inode, proc_root_iops, sizeof(proc_root_inode)))
    {
      perror("kern_check: read proc_root_iops");
      return -1;
    }

  close(kd);

  if (debug)
    fprintf(stdout, "proc_root_iops.lookup, proc_root_lookup: %p %#lx\n", 
	    proc_root_inode.lookup, proc_root_lookup);
  if ( (unsigned long) *proc_root_inode.lookup != proc_root_lookup)
    {
      fprintf(stderr, "WARNING: Adore-ng detected !!!\n");
      fprintf(stderr, "WARNING: You have: proc_root_iops.lookup: %p \n", 
	      proc_root_inode.lookup);
      fprintf(stderr, "WARNING: Correct : proc_root_lookup:      %#lx\n", 
		proc_root_lookup);
      have_warnings = EXIT_FAILURE;
    }

  if (flag == 0)
    {
      if (debug)
	{
	  fprintf(stdout, "(a)      %#lx\n", proc_root_dir_old.size);
	  fprintf(stdout, "(b)      %#lx\n",   
		  (unsigned long)* &proc_root_dir_old.proc_iops);
	}
      
      if ( ( ((unsigned long) * &proc_root_dir_old.proc_iops) != proc_root_iops) &&
	   (proc_root_dir_old.size != proc_root_iops))
	{
	  fprintf(stderr, "WARNING: Proc VFS modification (e.g. adore-ng) detected !!!\n");
	  fprintf(stderr, "WARNING: proc_root_inode_operations not found in proc_root.\n");
	  have_warnings = EXIT_FAILURE;
	}
    }
  else
    {
      if (debug)
	{
	  fprintf(stdout, "(a)      %#lx\n", proc_root_dir_new.size);
	  fprintf(stdout, "(b)      %#lx\n",   
		  (unsigned long)* &proc_root_dir_new.proc_iops);
	}
      
      if ( ( ((unsigned long) * &proc_root_dir_new.proc_iops) != proc_root_iops) &&
	   (proc_root_dir_new.size != proc_root_iops))
	{
	  fprintf(stderr, "WARNING: Proc VFS modification (e.g. adore-ng) detected !!!\n");
	  fprintf(stderr, "WARNING: proc_root_inode_operations not found in proc_root.\n");
	  have_warnings = EXIT_FAILURE;
	}
    }

  return 0;
}

#define MACH_I386   0
#define MACH_X86_64 1

int main(int argc, char *argv[])
{
  int i, count, kd, bad;
  int which;
  int machine;
  char * p;
  char * systemmap = NULL;
  smap_entry sh_smap[SH_MAXCALLS];
  struct utsname utbuf;

  unsigned long kaddr;
  unsigned long kaddr2;
  unsigned long kmem_call_table[512];

  unsigned long dispatch = 0;
  unsigned long kaddr3   = 0;

  unsigned long addr_ni_syscall = 0;

  if (argc > 1)
    {
      if (argv[1][0] == '-')
	{
	  if (strcmp(argv[1], "-h") == 0 ||  strcmp(argv[1], "--help") == 0)
	    usage(EXIT_SUCCESS);
	  else if (argc > 2 && (strcmp(argv[1], "-v") == 0 || 
				strcmp(argv[1], "--verbose") == 0))
	    {
	      systemmap = argv[2];
	      verbose = 1;
	    }
	  else 
	    usage(EXIT_FAILURE);
	}
      else
	systemmap = argv[1];
    }
  else
    usage(EXIT_FAILURE);
      
      

  if (0 != uname(&utbuf))
    {
      perror("kern_check: utname");
      exit (EXIT_FAILURE);
    }

  if      (strncmp(utbuf.release, "2.2", 3) == 0)
    which = 2;
  else if (strncmp(utbuf.release, "2.4", 3) == 0)
    which = 4;
  else if (strncmp(utbuf.release, "2.6", 3) == 0)
    which = 6;
  else
    {
      printf("kern_check: kernel %s not supported\n", utbuf.release);
      exit (EXIT_FAILURE);
    }

  kvers_major = atoi(utbuf.release);
  p = strchr(utbuf.release,'.');
  if (!p) {
      printf("kern_check: kernel %s: cannot determine minor version\n", 
	     utbuf.release);
      exit (EXIT_FAILURE);
  }
  ++p; kvers_minor = atoi(p);
  p = strchr(p,'.');
  if (!p) {
      printf("kern_check: kernel %s: cannot determine micro version\n", 
	     utbuf.release);
      exit (EXIT_FAILURE);
  }
  ++p; kvers_micro = atoi(p);
  if (debug)
    {
      printf ("kernel version: %d.%d.%d\n", 
	      kvers_major, kvers_minor, kvers_micro);
    }
  
  if (utbuf.machine[0] != 'i' || utbuf.machine[2] != '8' || 
      utbuf.machine[3] != '6')
    {
      if (utbuf.machine[0] != 'x' || utbuf.machine[1] != '8' || 
      utbuf.machine[2] != '6') 
	{
	  printf("kern_check: machine %s not supported\n", utbuf.machine);
	  exit (EXIT_FAILURE);
	}
      else
	{
	  machine = MACH_X86_64;
	}
    }
  else
    {
      machine = MACH_I386;
    }

  /* --- initialize the system call table
   */
  for (i = 0; i < SH_MAXCALLS; ++i)
    {
      if (which == 6 && machine == MACH_X86_64)
	{
	  if (callx_2p6[i] == NULL)
	    break;
	  strcpy(sh_smap[i].name, callx_2p6[i]);
	}
      else if (which == 2)
	{
	  if (callz_2p2[i] == NULL)
	    break;
	  strcpy(sh_smap[i].name, callz_2p2[i]);
	}
      else
	{
	  if (callz_2p4[i] == NULL)
	    break;
	  strcpy(sh_smap[i].name, callz_2p4[i]);
	}
      sh_smap[i].addr    = 0UL;
    }
  if (which == 6 && machine == MACH_I386) /* fix syscall map for 2.6 */
    {
      strcpy(sh_smap[0].name,   "sys_restart_syscall");
      strcpy(sh_smap[180].name, "sys_pread64");
      strcpy(sh_smap[181].name, "sys_pwrite64");
    }

  count = i;

  if ( fill_smap(systemmap, sh_smap, count) < 0)
    exit (EXIT_FAILURE);

  for (i = 0; i < count; ++i)
    {
      if (0 == strcmp(sh_smap[i].name, "sys_ni_syscall"))
	{
	  addr_ni_syscall = sh_smap[i].addr;
	  break;
	}
    }

  for (i = 0; i < count; ++i)
    {
      if (sh_smap[i].addr == 0UL)
	{
	  if (verbose > 0)
	    fprintf(stdout, "** unknown syscall **: [%s]\n", sh_smap[i].name);
	  strcpy(sh_smap[i].name, "sys_ni_syscall");
	  sh_smap[i].addr = addr_ni_syscall;
	}
    }

  if (which == 6) {
    kaddr = -1;
  } else {
    kaddr = my_address ("sys_call_table");
  }

  kaddr2 =  get_symbol_from_systemmap (systemmap, 
				       "sys_call_table", 'D');

  if (kaddr2 == 0) 
    {
      kaddr2 =  get_symbol_from_systemmap (systemmap, 
					   "sys_call_table", 'd');
    }
  if (kaddr2 == 0) 
    {
      kaddr2 =  get_symbol_from_systemmap (systemmap, 
					   "sys_call_table", 'R');
    }
  if (kaddr2 == 0) 
    {
      kaddr2 =  get_symbol_from_systemmap (systemmap, 
					   "sys_call_table", 'r');
    }

  /* from suckit
   */
  kaddr3 = get_sct(&dispatch);

  if (verbose > 0)
    fprintf(stdout, "(kernel) %#lx   %#lx (map)  %#lx (int 80h) [%s]\n",
	    kaddr,
	    kaddr2,
	    kaddr3,
	    "sys_call_table");

  if ((kaddr == (unsigned int) -1) && (kaddr2 == 0) && (kaddr3 == 0))
    {
      perror("kern_check: my_address()");
      fprintf(stderr, "ERROR: no sys_call_table address in %s found\n",
	      systemmap);
      fprintf(stderr, "ERROR: no sys_call_table address via int 80h found\n");
      fprintf(stderr, "ERROR: cannot check the system call table\n");
      exit (EXIT_FAILURE);
    }

  if ( (kaddr == (unsigned int) -1) )
    {
      if (verbose > 0)
	{
	  fprintf(stderr, 
		  "sys_call_table address not exported by the kernel,\n");
	  fprintf(stderr, "using the address from %s\n", systemmap);
	}
      kaddr = kaddr2;
    }
  else if ( kaddr2 != 0 && kaddr != kaddr2 )
    {
      fprintf(stderr, 
	      "WARNING: (kernel) %#lx != %#lx (map)  [%s]\n",
	      kaddr,
	      kaddr2,
	      "sys_call_table");
      have_warnings = EXIT_FAILURE;
    }
  else if ( kaddr3 != 0 && kaddr != kaddr3 )
    {
      fprintf(stderr, 
	      "WARNING: (kernel) %#lx != %#lx (int 80h)  [%s]\n",
	      kaddr,
	      kaddr3,
	      "sys_call_table");
      have_warnings = EXIT_FAILURE;
    }

  /* here, kaddr should be eq kaddr2 */
  if (kaddr3 == 0)
    {
      fprintf(stderr, 
	      "Could not determine sys_call_table[] address from int 80h\n");
    }
  else if ( (kaddr3 != 0) && (kaddr2 != 0) && (kaddr3 != kaddr2) )
    {
      fprintf(stderr, 
	      "WARNING: (int 80h) %#lx != %#lx (map)  [%s]\n",
	      kaddr3,
	      kaddr2,
	      "sys_call_table");
      fprintf(stderr, 
	      "WARNING: This indicates the presence of the SuckIT rootkit.");
      have_warnings = EXIT_FAILURE;
      kaddr = kaddr3;
    }
  else 
    {
      kaddr = kaddr3;
    }


  kd = open("/dev/kmem", O_RDONLY, 0);

  if (kd < 0)
    {
      perror("kern_check: open(/dev/kmem)");
      exit (EXIT_FAILURE);
    }

  if (!loc_rkm(kd, (char *) &kmem_call_table, kaddr, sizeof(kmem_call_table)))
    {
      perror("kern_check: read kmem_call_table");
      exit (EXIT_FAILURE);
    }

  close(kd);

  bad = 0;

  for (i = 0; i < count; ++i)
    {
      if (sh_smap[i].name == NULL || sh_smap[i].addr == 0UL)
        break;

      if (verbose > 0)
	fprintf(stdout, "(kernel) %#lx   %#lx (map)  [%03d][%s]\n",
		kmem_call_table[i],
		sh_smap[i].addr,
		i,
		sh_smap[i].name);

      if (sh_smap[i].addr != kmem_call_table[i])
	{
	  fprintf(stderr, 
		  "WARNING: (kernel) %#lx != %#lx (map)  [%03d][%s]\n",
		  kmem_call_table[i],
		  sh_smap[i].addr,
		  i,
		  sh_smap[i].name);
	  have_warnings = EXIT_FAILURE;
	  ++bad;
	}

      /* 2.6 syscall table is too large for 2.4 kernel
       */
      if ((which < 6) && (i > 254))
	break;
    }

  if (bad > (count/2))
    {
      fprintf(stderr, "-------------------------------------------------------------------------\n");
      fprintf(stderr, "ERROR: You have a very large number of incorrect syscall addresses. This\n");
      fprintf(stderr, "ERROR: indicates that you are using the wrong System.map file, i.e. one\n");
      fprintf(stderr, "ERROR: that does not correspond to your currently running kernel.\n");
      fprintf(stderr, "ERROR: If this is the case, you should ignore all warnings\n");
      fprintf(stderr, "ERROR: and look for the correct System.map file first.\n");
      fprintf(stderr, "-------------------------------------------------------------------------\n");
    }

  if (0 != check_adore_ng(systemmap))
    {
      fprintf(stderr, "ERROR: check of /proc filesystem failed.\n");
      have_warnings = EXIT_FAILURE;
    }

  exit (have_warnings);
}


