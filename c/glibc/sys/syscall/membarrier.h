#include <unistd.h>
#include <sys/syscall.h>
#include <linux/membarrier.h>

// /usr/include/bits/syscall.h:784:#ifdef __NR_membarrier
// /usr/include/bits/syscall.h:785:# define SYS_membarrier __NR_membarrier
// /usr/include/SDL2/SDL_atomic.h:178:#define SDL_MemoryBarrierRelease()   __cpu_membarrier()
// /usr/include/SDL2/SDL_atomic.h:179:#define SDL_MemoryBarrierAcquire()   __cpu_membarrier()

enum membarrier_cmd {
	MEMBARRIER_CMD_QUERY					= 0,
	MEMBARRIER_CMD_GLOBAL					= (1 << 0),
	MEMBARRIER_CMD_GLOBAL_EXPEDITED				= (1 << 1),
	MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED		= (1 << 2),
	MEMBARRIER_CMD_PRIVATE_EXPEDITED			= (1 << 3),
	MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED		= (1 << 4),
	MEMBARRIER_CMD_PRIVATE_EXPEDITED_SYNC_CORE		= (1 << 5),
	MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED_SYNC_CORE	= (1 << 6),

	/* Alias for header backward compatibility. */
	MEMBARRIER_CMD_SHARED			= MEMBARRIER_CMD_GLOBAL,
};
    
enum membarrier_cmd_flag {
    MEMBARRIER_CMD_FLAG_CPU     = (1 << 0),
};


int membarrier(int cmd, unsigned int flags, int cpu_id);


/**
 * sys_membarrier - issue memory barriers on a set of threads
 * @cmd:   Takes command values defined in enum membarrier_cmd.
 * @flags: Currently needs to be 0. For future extensions.
 *
 * If this system call is not implemented, -ENOSYS is returned. If the
 * command specified does not exist, not available on the running
 * kernel, or if the command argument is invalid, this system call
 * returns -EINVAL. For a given command, with flags argument set to 0,
 * if this system call returns -ENOSYS or -EINVAL, it is guaranteed to
 * always return the same value until reboot. In addition, it can return
 * -ENOMEM if there is not enough memory available to perform the system
 * call.
 *
 * All memory accesses performed in program order from each targeted thread
 * is guaranteed to be ordered with respect to sys_membarrier(). If we use
 * the semantic "barrier()" to represent a compiler barrier forcing memory
 * accesses to be performed in program order across the barrier, and
 * smp_mb() to represent explicit memory barriers forcing full memory
 * ordering across the barrier, we have the following ordering table for
 * each pair of barrier(), sys_membarrier() and smp_mb():
 *
 * The pair ordering is detailed as (O: ordered, X: not ordered):
 *
 *                        barrier()   smp_mb() sys_membarrier()
 *        barrier()          X           X            O
 *        smp_mb()           X           O            O
 *        sys_membarrier()   O           O            O
 */
SYSCALL_DEFINE2(membarrier, int, cmd, int, flags)
{
	if (unlikely(flags))
		return -EINVAL;
	switch (cmd) {
	case MEMBARRIER_CMD_QUERY:
	{
		int cmd_mask = MEMBARRIER_CMD_BITMASK;

		if (tick_nohz_full_enabled())
			cmd_mask &= ~MEMBARRIER_CMD_GLOBAL;
		return cmd_mask;
	}
	case MEMBARRIER_CMD_GLOBAL:
		/* MEMBARRIER_CMD_GLOBAL is not compatible with nohz_full. */
		if (tick_nohz_full_enabled())
			return -EINVAL;
		if (num_online_cpus() > 1)
			synchronize_rcu();
		return 0;
	case MEMBARRIER_CMD_GLOBAL_EXPEDITED:
		return membarrier_global_expedited();
	case MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED:
		return membarrier_register_global_expedited();
	case MEMBARRIER_CMD_PRIVATE_EXPEDITED:
		return membarrier_private_expedited(0);
	case MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED:
		return membarrier_register_private_expedited(0);
	case MEMBARRIER_CMD_PRIVATE_EXPEDITED_SYNC_CORE:
		return membarrier_private_expedited(MEMBARRIER_FLAG_SYNC_CORE);
	case MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED_SYNC_CORE:
		return membarrier_register_private_expedited(MEMBARRIER_FLAG_SYNC_CORE);
	default:
		return -EINVAL;
	}
}

