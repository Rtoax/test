#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/timer.h>

#include <linux/kernel.h>

struct timer_list stimer;

static void 
time_handler(unsigned long data)
{
	mod_timer(&stimer, jiffies + HZ);
	printk("current jiffies is %ld\n", jiffies);
}

static int __init timer_init(void)
{
	printk("My module worked!\n");
	init_timer(&stimer);
	stimer.data = 0;
	stimer.expires = jiffies + HZ;
	stimer.function = time_handler;
	add_timer(&stimer);
	return 0;
}

static void
__exit timer_exit(void)
{
	printk("Unloading my module.\n");
	del_timer(&stimer);
	return;
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_AUTHOR("fyf");
MODULE_LICENSE("GPL");

