/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
//#include <pthread_np.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/queue.h>
//#include <sys/thr.h>

//#include <rte_debug.h>
//#include <rte_atomic.h>
//#include <rte_launch.h>
#include <rte_log.h>
//#include <rte_memory.h>
#include <rte_per_lcore.h>
//#include <rte_eal.h>
#include <rte_lcore.h>
//#include <rte_eal_trace.h>

//#include "eal_private.h"
//#include "eal_thread.h"


RTE_DEFINE_PER_LCORE(unsigned, _lcore_id) = LCORE_ID_ANY;
//RTE_DEFINE_PER_LCORE(unsigned, _socket_id) = (unsigned)SOCKET_ID_ANY;


int rte_thread_getname(pthread_t id, char *name, size_t len)
{
	return pthread_getname_np(id, name, len);
}

