/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2017 Cavium, Inc
 */

#ifndef _RTE_PAUSE_X86_H_
#define _RTE_PAUSE_X86_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include "generic/rte_pause.h"
#include <assert.h>

#include <emmintrin.h>
static inline void rte_pause(void)
{
	_mm_pause();
}
static __rte_always_inline void
rte_wait_until_equal_16(volatile uint16_t *addr, uint16_t expected,
		int memorder)
{
	assert(memorder == __ATOMIC_ACQUIRE || memorder == __ATOMIC_RELAXED);

	while (__atomic_load_n(addr, memorder) != expected)
		rte_pause();
}

static __rte_always_inline void
rte_wait_until_equal_32(volatile uint32_t *addr, uint32_t expected,
		int memorder)
{
	assert(memorder == __ATOMIC_ACQUIRE || memorder == __ATOMIC_RELAXED);

	while (__atomic_load_n(addr, memorder) != expected)
		rte_pause();
}

static __rte_always_inline void
rte_wait_until_equal_64(volatile uint64_t *addr, uint64_t expected,
		int memorder)
{
	assert(memorder == __ATOMIC_ACQUIRE || memorder == __ATOMIC_RELAXED);

	while (__atomic_load_n(addr, memorder) != expected)
		rte_pause();
}

#ifdef __cplusplus
}
#endif

#endif /* _RTE_PAUSE_X86_H_ */

