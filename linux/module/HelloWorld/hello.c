/**
 * @file    hello.c
 * @author  Derek Molloy, modified Mateusz Midor
 * @date    4 April 2015, Nov 3, 2016
 * @version 0.1
 * @brief  An introductory "Hello World!" loadable kernel module (LKM) that can display a message
 * in the /var/log/kern.log file when the module is loaded and removed. The module can accept an
 * argument when it is loaded -- the name, which appears in the kernel log files.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
*/

#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Derek Molloy [ReCode by RToax]");
MODULE_DESCRIPTION("A Simple Hello World module");
MODULE_VERSION("0.1");

static char *name = "[RToax]"; // An example LKM argument -- default value is "world"
module_param( name, charp, S_IRUGO);// Param desc. charp = char ptr, S_IRUGO can be read/not changed
MODULE_PARM_DESC(name, "[RToax]The name to display in /var/log/kern.log");	// parameter description

static int __init hello_init(void) {
	printk(KERN_INFO "[RToax]Hello %s!\n", name);
	return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit hello_cleanup(void) {
	printk(KERN_INFO "[RToax]Cleaning up hello module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
