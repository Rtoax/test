#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init
init_module_0(void)
{
    printk(KERN_INFO "[RToax]Hello world!\n");

    return 0;
}

static void __exit
cleanup_module_0(void)
{
    printk(KERN_INFO "[RToax]Goodbye world!\n");
}

module_init(init_module_0);
module_exit(cleanup_module_0);
