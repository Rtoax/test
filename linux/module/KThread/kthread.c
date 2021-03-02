/**
 * @file    kthread.c
 * @date    08.05.2017
 * @author  Mateusz Midor
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>


struct task_struct *task;
char *THREAD_MESSAGE = "Hello from MyThread!";


int thread_function(void *data) {
    printk(KERN_INFO "%s\n", (char*) data);

    while (!kthread_should_stop()) {
        schedule();
    }

    printk(KERN_INFO "MyThread says goodbye\n");
    return 0;
}

static int kernel_init(void) {
    printk(KERN_INFO "--------------------------------------------");
    task = kthread_run(&thread_function, (void *)THREAD_MESSAGE, "MyThread");
    printk(KERN_INFO "Kernel Thread : %s\n", task->comm);
    return 0;
}

static void kernel_exit(void) {
    kthread_stop(task);
}

module_init(kernel_init);
module_exit(kernel_exit);
MODULE_AUTHOR("MM");
MODULE_LICENSE("GPL");
