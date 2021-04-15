/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation.
 * Copyright(c) 2013 6WIND S.A.
 */

#ifndef _RTE_CYCLES_X86_64_H_
#define _RTE_CYCLES_X86_64_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <rte_common.h>
//#include <rte_config.h>

static inline uint64_t
rte_rdtsc(void)
{
	union {
		uint64_t tsc_64;
		RTE_STD_C11
		struct {
			uint32_t lo_32;
			uint32_t hi_32;
		};
	} tsc;

	asm volatile("rdtsc" :
		     "=a" (tsc.lo_32),
		     "=d" (tsc.hi_32));
	return tsc.tsc_64;
}

static inline uint64_t
rte_rdtsc_precise(void)
{
	rte_mb();
	return rte_rdtsc();
}

static inline uint64_t
rte_get_tsc_cycles(void) { return rte_rdtsc(); }

#ifdef __cplusplus
}
#endif

#endif /* _RTE_CYCLES_X86_64_H_ */

