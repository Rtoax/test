#include <linux/module.h>
#include <linux/kernel.h>
//#include <linux/init.h>

int
init_module_1(void)
{
    printk(KERN_INFO "[RToax]Hello world!\n");

    return 0;
}

void
cleanup_module_1(void)
{
    printk(KERN_INFO "[RToax]Goodbye world!\n");
}

module_init(init_module_1);
module_exit(cleanup_module_1);

