#include <linux/module.h>
#include <linux/kernel.h>
//#include <linux/init.h>

int
init_module(void)
{
    printk(KERN_INFO "Hello world!\n");

    return 0;
}

void
cleanup_module(void)
{
    printk(KERN_INFO "Goodbye world!\n");
}

