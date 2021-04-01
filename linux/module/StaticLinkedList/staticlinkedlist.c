/**
 *   @file: staticlinkedlist.c
 *
 *   @date: Apr 03, 2017
 * @author: Mateusz Midor
 */
#include <linux/module.h>	// included for all kernel modules
#include <linux/kernel.h>	// included for KERN_INFO
#include <linux/init.h> 	// included for __init and __exit macros
#include <linux/list.h>



// 1. define and init list head
LIST_HEAD(country_capital_list);

// 2. define list element type
struct country_capital {
    struct list_head list; // this makes country_capital valid linked-list element
    char *country;
    char *capital;
};

// 3. define some static list elements
struct country_capital c1 = {
    .country = "portugalia",
    .capital = "lizbona"
};

struct country_capital c2 = {
    .country = "kambodża",
    .capital = "phnompenh"
};

struct country_capital c3 = {
    .country = "meksyk",
    .capital = "tenochtitlan "
};


// 4. add elments to the list
static void fill_list(void) {
    list_add_tail(&c1.list, &country_capital_list);
    list_add_tail(&c2.list, &country_capital_list);
    list_add_tail(&c3.list, &country_capital_list);
}

// 5. print list elements
static void print_list(void) {
    struct country_capital *entry;

    list_for_each_entry(entry, &country_capital_list, list) {
        printk(KERN_INFO "[RToax]%s - %s\n", entry->country, entry->capital);}
}

// #######################################################################
// HERE STARTS THE STANDARD MODULE CONFIG - SETUP - TEARDOWN
// #######################################################################

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateusz Midor ReCode by [RToax]");
MODULE_DESCRIPTION("A linux statically allocated linked list example");
MODULE_VERSION("0.1");

static int __init hello_init(void) {
    printk(KERN_INFO "[RToax]Preparing linkedlist module.\n");
    fill_list();
    print_list();
	return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit hello_cleanup(void) {
	printk(KERN_INFO "[RToax]Cleaning up linkedlist module.\n\n");
}


module_init(hello_init);
module_exit(hello_cleanup);
