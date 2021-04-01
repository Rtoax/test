#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Slava Imameev ReCode by [RToax]");

const char* state_to_string(long state)
{
	switch (state)
	{
		case TASK_RUNNING: return "rinning";
		case TASK_INTERRUPTIBLE: return "irqable";
		case TASK_UNINTERRUPTIBLE: return "unirqable";
		case __TASK_STOPPED: return "stoped";
		case __TASK_TRACED: return "traced";
		case TASK_DEAD: return "dead";
		case TASK_WAKEKILL: return "wakekill";
		case TASK_WAKING: return "waking";
		case TASK_PARKED: return "parked";
		//case TASK_NOLOAD: return "TASK_NOLOAD";
		default: return "UNKNOWN";
	}
}

void list_from_task(struct task_struct *task)
{
	// tasklist_lock is not exported anymore, use RCU as p->tasks is updated wth list_add_tail_rcu(), see copy_process(),
	// you can use list_for_each_entry_rcu() instead the explicit RCU list traversing code below
	rcu_read_lock();
	{
		struct task_struct* p = task;

        
        printk(KERN_INFO "[RToax]%17s %6s %6s %3s %4s %4s %-20s\n", 
                                "Name", "PID", "TGID", "Pri","PriS","PriN", "State");
		do
		{
			struct list_head*  next;
			long               state = p->state; // the value is volatile and will be accessed twice, make a copy for consistency

			printk(KERN_INFO "[RToax]%17s %6d %6d %3d %4d %4d %2ld(%9s)\n", 
                                p->comm, p->pid, p->tgid,
                                p->prio, p->static_prio, p->normal_prio, 
                                state, state_to_string(state));

			next = rcu_dereference(p->tasks.next);
			p = list_entry(next, struct task_struct, tasks);
		}
		while (p != task);
	}
	rcu_read_unlock();
}

void list_tasks(void)
{
	list_from_task(&init_task);
}

static int __init _init(void)
{
	printk(KERN_INFO "[RToax]Hi from init\n");

	list_tasks();

	return 0;
}

static void __exit _exit(void)
{
	printk(KERN_INFO "[RToax]Bye!\n");
}

module_init(_init)
module_exit(_exit)
