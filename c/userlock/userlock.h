/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation [spinlock]
 * Copyright(c) 2015 Intel Corporation  [rwlock]
 * Copyright(c) 2019 Arm Limited    [ticketlock]
 * Copyright(c) 2019 Arm Limited    [mcslock]
 * 
 *  ----------------------------------------------------------------------
 *  此文件移植自 DPDK-20.05，版权如上所示，使用建议。
 *      其中 "*" 表示建议使用程度
 *
 *  user_spinlock_t     自旋锁   *****
 *  user_rwlock_t       读写锁****
 *  user_mcslock_t      MCS锁**
 *  user_ticketlock_t   ticket锁*
 *  user_ticketlock_recursive_t 递归ticket锁*
 *
 *  荣涛  
 *  rongtao@sylincom.com
 */

#ifndef __USer_DpDK_LoCk_h
#define __USer_DpDK_LoCk_h 1

#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <syscall.h>
#include <assert.h>
#include <emmintrin.h>


/**
 * The user_spinlock_t type.
 */
typedef struct {
	volatile int locked; /**< lock status 0 = unlocked, 1 = locked */

#define USER_SPINLOCK_INITIALIZER { 0 } /* A static spinlock initializer. */
} user_spinlock_t;


/**
 * The user_rwlock_t type.
 *
 * cnt is -1 when write lock is held, and > 0 when read locks are held.
 */
typedef struct {
	volatile int32_t cnt; /**< -1 when W lock held, > 0 when R locks held. */

#define USER_RWLOCK_INITIALIZER { 0 }    /* A static rwlock initializer. */
} user_rwlock_t;


/**
 * The user_mcslock_t type.
 */
typedef struct user_mcslock {
	struct user_mcslock *next;
	int locked; /* 1 if the queue locked, 0 otherwise */
} user_mcslock_t;


/**
 * The user_ticketlock_t type.
 */
typedef union {
	uint32_t tickets;
	struct {
		uint16_t current;
		uint16_t next;
	} s;
#define USER_TICKETLOCK_INITIALIZER { 0 }    /* A static ticketlock initializer. */
} user_ticketlock_t;



/**
 * The user_ticketlock_recursive_t type.
 */
#define TICKET_LOCK_INVALID_ID -1

typedef struct {
	user_ticketlock_t tl; /**< the actual ticketlock */
	int user; /**< core id using lock, TICKET_LOCK_INVALID_ID for unused */
	unsigned int count; /**< count of time this lock has been called */
/* A static recursive ticketlock initializer. */
#define USER_TICKETLOCK_RECURSIVE_INITIALIZER {USER_TICKETLOCK_INITIALIZER, \
					      TICKET_LOCK_INVALID_ID, 0}
} user_ticketlock_recursive_t;



#ifndef unlikely
#define unlikely(x)    (__builtin_expect(!!(x), 0))
#endif

#ifndef likely
#define likely(x)    (__builtin_expect(!!(x), 1))
#endif



static inline void user_pause(void);

static inline void
user_wait_until_equal_16(volatile uint16_t *addr, uint16_t expected,
		int memorder);

static inline void
user_wait_until_equal_32(volatile uint32_t *addr, uint32_t expected,
		int memorder);

static inline void
user_wait_until_equal_64(volatile uint64_t *addr, uint64_t expected,
		int memorder);

/**
 * Initialize the spinlock to an unlocked state.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
user_spinlock_init(user_spinlock_t *sl) {
	sl->locked = 0;
}

static inline void
user_spinlock_lock(user_spinlock_t *sl) {
	int exp = 0;

	while (!__atomic_compare_exchange_n(&sl->locked, &exp, 1, 0,
				__ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
		while (__atomic_load_n(&sl->locked, __ATOMIC_RELAXED))
			user_pause();
		exp = 0;
	}
}

static inline int
user_spinlock_trylock (user_spinlock_t *sl) {
	int exp = 0;
	return __atomic_compare_exchange_n(&sl->locked, &exp, 1,
				0, /* disallow spurious failure */
				__ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
}


static inline void
user_spinlock_unlock (user_spinlock_t *sl) {
	__atomic_store_n(&sl->locked, 0, __ATOMIC_RELEASE);
}

/**
 * Test if the lock is taken.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the lock is currently taken; 0 otherwise.
 */
static inline int user_spinlock_is_locked (user_spinlock_t *sl) {
	return __atomic_load_n(&sl->locked, __ATOMIC_ACQUIRE);
}


/**
 * Initialize the rwlock to an unlocked state.
 *
 * @param rwl
 *   A pointer to the rwlock structure.
 */
static inline void
user_rwlock_init(user_rwlock_t *rwl)
{
	rwl->cnt = 0;
}

/**
 * Take a read lock. Loop until the lock is held.
 *
 * @param rwl
 *   A pointer to a rwlock structure.
 */
static inline void
user_rwlock_read_lock(user_rwlock_t *rwl)
{
	int32_t x;
	int success = 0;

	while (success == 0) {
		x = __atomic_load_n(&rwl->cnt, __ATOMIC_RELAXED);
		/* write lock is held */
		if (x < 0) {
			user_pause();
			continue;
		}
		success = __atomic_compare_exchange_n(&rwl->cnt, &x, x + 1, 1,
					__ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
	}
}

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice.
 *
 * try to take a read lock.
 *
 * @param rwl
 *   A pointer to a rwlock structure.
 * @return
 *   - zero if the lock is successfully taken
 *   - -EBUSY if lock could not be acquired for reading because a
 *     writer holds the lock
 */
static inline int
user_rwlock_read_trylock(user_rwlock_t *rwl)
{
	int32_t x;
	int success = 0;

	while (success == 0) {
		x = __atomic_load_n(&rwl->cnt, __ATOMIC_RELAXED);
		/* write lock is held */
		if (x < 0)
			return -EBUSY;
		success = __atomic_compare_exchange_n(&rwl->cnt, &x, x + 1, 1,
					__ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
	}

	return 0;
}

/**
 * Release a read lock.
 *
 * @param rwl
 *   A pointer to the rwlock structure.
 */
static inline void
user_rwlock_read_unlock(user_rwlock_t *rwl)
{
	__atomic_fetch_sub(&rwl->cnt, 1, __ATOMIC_RELEASE);
}

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice.
 *
 * try to take a write lock.
 *
 * @param rwl
 *   A pointer to a rwlock structure.
 * @return
 *   - zero if the lock is successfully taken
 *   - -EBUSY if lock could not be acquired for writing because
 *     it was already locked for reading or writing
 */
static inline int
user_rwlock_write_trylock(user_rwlock_t *rwl)
{
	int32_t x;

	x = __atomic_load_n(&rwl->cnt, __ATOMIC_RELAXED);
	if (x != 0 || __atomic_compare_exchange_n(&rwl->cnt, &x, -1, 1,
			      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED) == 0)
		return -EBUSY;

	return 0;
}

/**
 * Take a write lock. Loop until the lock is held.
 *
 * @param rwl
 *   A pointer to a rwlock structure.
 */
static inline void
user_rwlock_write_lock(user_rwlock_t *rwl)
{
	int32_t x;
	int success = 0;

	while (success == 0) {
		x = __atomic_load_n(&rwl->cnt, __ATOMIC_RELAXED);
		/* a lock is held */
		if (x != 0) {
			user_pause();
			continue;
		}
		success = __atomic_compare_exchange_n(&rwl->cnt, &x, -1, 1,
					__ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
	}
}

/**
 * Release a write lock.
 *
 * @param rwl
 *   A pointer to a rwlock structure.
 */
static inline void
user_rwlock_write_unlock(user_rwlock_t *rwl)
{
	__atomic_store_n(&rwl->cnt, 0, __ATOMIC_RELEASE);
}



/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice
 *
 * Take the MCS lock.
 *
 * @param msl
 *   A pointer to the pointer of a MCS lock.
 *   When the lock is initialized or declared, the msl pointer should be
 *   set to NULL.
 * @param me
 *   A pointer to a new node of MCS lock. Each CPU/thread acquiring the
 *   lock should use its 'own node'.
 */
static inline void
user_mcslock_lock(user_mcslock_t **msl, user_mcslock_t *me)
{
	user_mcslock_t *prev;

	/* Init me node */
	__atomic_store_n(&me->locked, 1, __ATOMIC_RELAXED);
	__atomic_store_n(&me->next, NULL, __ATOMIC_RELAXED);

	/* If the queue is empty, the exchange operation is enough to acquire
	 * the lock. Hence, the exchange operation requires acquire semantics.
	 * The store to me->next above should complete before the node is
	 * visible to other CPUs/threads. Hence, the exchange operation requires
	 * release semantics as well.
	 */
	prev = __atomic_exchange_n(msl, me, __ATOMIC_ACQ_REL);
	if (likely(prev == NULL)) {
		/* Queue was empty, no further action required,
		 * proceed with lock taken.
		 */
		return;
	}
	__atomic_store_n(&prev->next, me, __ATOMIC_RELAXED);

	/* The while-load of me->locked should not move above the previous
	 * store to prev->next. Otherwise it will cause a deadlock. Need a
	 * store-load barrier.
	 */
	__atomic_thread_fence(__ATOMIC_ACQ_REL);
	/* If the lock has already been acquired, it first atomically
	 * places the node at the end of the queue and then proceeds
	 * to spin on me->locked until the previous lock holder resets
	 * the me->locked using mcslock_unlock().
	 */
	while (__atomic_load_n(&me->locked, __ATOMIC_ACQUIRE))
		user_pause();
}

/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice
 *
 * Release the MCS lock.
 *
 * @param msl
 *   A pointer to the pointer of a MCS lock.
 * @param me
 *   A pointer to the node of MCS lock passed in user_mcslock_lock.
 */
static inline void
user_mcslock_unlock(user_mcslock_t **msl, user_mcslock_t *me)
{
	/* Check if there are more nodes in the queue. */
	if (likely(__atomic_load_n(&me->next, __ATOMIC_RELAXED) == NULL)) {
		/* No, last member in the queue. */
		user_mcslock_t *save_me = __atomic_load_n(&me, __ATOMIC_RELAXED);

		/* Release the lock by setting it to NULL */
		if (likely(__atomic_compare_exchange_n(msl, &save_me, NULL, 0,
				__ATOMIC_RELEASE, __ATOMIC_RELAXED)))
			return;

		/* Speculative execution would be allowed to read in the
		 * while-loop first. This has the potential to cause a
		 * deadlock. Need a load barrier.
		 */
		__atomic_thread_fence(__ATOMIC_ACQUIRE);
		/* More nodes added to the queue by other CPUs.
		 * Wait until the next pointer is set.
		 */
		while (__atomic_load_n(&me->next, __ATOMIC_RELAXED) == NULL)
			user_pause();
	}

	/* Pass lock to next waiter. */
	__atomic_store_n(&me->next->locked, 0, __ATOMIC_RELEASE);
}

/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice
 *
 * Try to take the lock.
 *
 * @param msl
 *   A pointer to the pointer of a MCS lock.
 * @param me
 *   A pointer to a new node of MCS lock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline int
user_mcslock_trylock(user_mcslock_t **msl, user_mcslock_t *me)
{
	/* Init me node */
	__atomic_store_n(&me->next, NULL, __ATOMIC_RELAXED);

	/* Try to lock */
	user_mcslock_t *expected = NULL;

	/* The lock can be taken only when the queue is empty. Hence,
	 * the compare-exchange operation requires acquire semantics.
	 * The store to me->next above should complete before the node
	 * is visible to other CPUs/threads. Hence, the compare-exchange
	 * operation requires release semantics as well.
	 */
	return __atomic_compare_exchange_n(msl, &expected, me, 0,
			__ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
}

/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice
 *
 * Test if the lock is taken.
 *
 * @param msl
 *   A pointer to a MCS lock node.
 * @return
 *   1 if the lock is currently taken; 0 otherwise.
 */
static inline int
user_mcslock_is_locked(user_mcslock_t *msl)
{
	return (__atomic_load_n(&msl, __ATOMIC_RELAXED) != NULL);
}



/**
 * Initialize the ticketlock to an unlocked state.
 *
 * @param tl
 *   A pointer to the ticketlock.
 */
static inline void
user_ticketlock_init(user_ticketlock_t *tl)
{
	__atomic_store_n(&tl->tickets, 0, __ATOMIC_RELAXED);
}

/**
 * Take the ticketlock.
 *
 * @param tl
 *   A pointer to the ticketlock.
 */
static inline void
user_ticketlock_lock(user_ticketlock_t *tl)
{
	uint16_t me = __atomic_fetch_add(&tl->s.next, 1, __ATOMIC_RELAXED);
	user_wait_until_equal_16(&tl->s.current, me, __ATOMIC_ACQUIRE);
}

/**
 * Release the ticketlock.
 *
 * @param tl
 *   A pointer to the ticketlock.
 */
static inline void
user_ticketlock_unlock(user_ticketlock_t *tl)
{
	uint16_t i = __atomic_load_n(&tl->s.current, __ATOMIC_RELAXED);
	__atomic_store_n(&tl->s.current, i + 1, __ATOMIC_RELEASE);
}

/**
 * Try to take the lock.
 *
 * @param tl
 *   A pointer to the ticketlock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline int
user_ticketlock_trylock(user_ticketlock_t *tl)
{
	user_ticketlock_t old, new;
	old.tickets = __atomic_load_n(&tl->tickets, __ATOMIC_RELAXED);
	new.tickets = old.tickets;
	new.s.next++;
	if (old.s.next == old.s.current) {
		if (__atomic_compare_exchange_n(&tl->tickets, &old.tickets,
		    new.tickets, 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
			return 1;
	}

	return 0;
}

/**
 * Test if the lock is taken.
 *
 * @param tl
 *   A pointer to the ticketlock.
 * @return
 *   1 if the lock is currently taken; 0 otherwise.
 */
static inline int
user_ticketlock_is_locked(user_ticketlock_t *tl)
{
	user_ticketlock_t tic;
	tic.tickets = __atomic_load_n(&tl->tickets, __ATOMIC_ACQUIRE);
	return (tic.s.current != tic.s.next);
}


/**
 * Initialize the recursive ticketlock to an unlocked state.
 *
 * @param tlr
 *   A pointer to the recursive ticketlock.
 */
static inline void
user_ticketlock_recursive_init(user_ticketlock_recursive_t *tlr)
{
	user_ticketlock_init(&tlr->tl);
	__atomic_store_n(&tlr->user, TICKET_LOCK_INVALID_ID, __ATOMIC_RELAXED);
	tlr->count = 0;
}

/**
 * Take the recursive ticketlock.
 *
 * @param tlr
 *   A pointer to the recursive ticketlock.
 */
static inline void
user_ticketlock_recursive_lock(user_ticketlock_recursive_t *tlr)
{
	int id = syscall(__NR_gettid);

	if (__atomic_load_n(&tlr->user, __ATOMIC_RELAXED) != id) {
		user_ticketlock_lock(&tlr->tl);
		__atomic_store_n(&tlr->user, id, __ATOMIC_RELAXED);
	}
	tlr->count++;
}

/**
 * Release the recursive ticketlock.
 *
 * @param tlr
 *   A pointer to the recursive ticketlock.
 */
static inline void
user_ticketlock_recursive_unlock(user_ticketlock_recursive_t *tlr)
{
	if (--(tlr->count) == 0) {
		__atomic_store_n(&tlr->user, TICKET_LOCK_INVALID_ID,
				 __ATOMIC_RELAXED);
		user_ticketlock_unlock(&tlr->tl);
	}
}

/**
 * Try to take the recursive lock.
 *
 * @param tlr
 *   A pointer to the recursive ticketlock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline int
user_ticketlock_recursive_trylock(user_ticketlock_recursive_t *tlr)
{
	int id = syscall(__NR_gettid);

	if (__atomic_load_n(&tlr->user, __ATOMIC_RELAXED) != id) {
		if (user_ticketlock_trylock(&tlr->tl) == 0)
			return 0;
		__atomic_store_n(&tlr->user, id, __ATOMIC_RELAXED);
	}
	tlr->count++;
	return 1;
}

static inline void user_pause(void) {
	_mm_pause();
}

static inline void
user_wait_until_equal_16(volatile uint16_t *addr, uint16_t expected,
		int memorder)
{
	assert(memorder == __ATOMIC_ACQUIRE || memorder == __ATOMIC_RELAXED);

	while (__atomic_load_n(addr, memorder) != expected)
		user_pause();
}

static inline void
user_wait_until_equal_32(volatile uint32_t *addr, uint32_t expected,
		int memorder)
{
	assert(memorder == __ATOMIC_ACQUIRE || memorder == __ATOMIC_RELAXED);

	while (__atomic_load_n(addr, memorder) != expected)
		user_pause();
}

static inline void
user_wait_until_equal_64(volatile uint64_t *addr, uint64_t expected,
		int memorder)
{
	assert(memorder == __ATOMIC_ACQUIRE || memorder == __ATOMIC_RELAXED);

	while (__atomic_load_n(addr, memorder) != expected)
		user_pause();
}

#endif  /*<__USer_DpDK_LoCk_h>*/
