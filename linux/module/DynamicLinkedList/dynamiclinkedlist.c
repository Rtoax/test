/**
 *   @file: dynamiclinkedlist.c
 *
 *   @date: Apr 04, 2017
 * @author: Mateusz Midor
 */
#include <linux/module.h>	// included for all kernel modules
#include <linux/kernel.h>	// included for KERN_INFO
#include <linux/init.h> 	// included for __init and __exit macros
#include <linux/list.h>
#include <linux/slab.h>		// kmalloc, kfree


// 1. define and init list head
LIST_HEAD(country_capital_list);

// 2. define list element type
struct country_capital {
    struct list_head list; // this makes country_capital valid linked-list element
    char country[20];
    char capital[20];
};

// 3. alloc and add elments to the list
static void fill_list(void) {
	const char* COUNTRIES[] = {"hiszpania", "wietnam", "peru"};
	const char* CAPITALS[] = {"madryt", "hanoi", "lima"};

	int i;
	struct country_capital *item;
	for (i = 0; i < sizeof(COUNTRIES) / sizeof(COUNTRIES[0]); i++) {
		// kmalloc - contiguous virtual addresses and physical addresses
		// vmalloc - contiguous virtual addresses, physical contiguous or not - allocated page by page, better chance to succeed
		item = kmalloc(sizeof(struct country_capital), GFP_KERNEL); // GFP_KERNEL is for ordinary kernel memory allocations
		strncpy(item->country, COUNTRIES[i], sizeof(item->country));
		strncpy(item->capital, CAPITALS[i], sizeof(item->capital));
		list_add_tail(&item->list, &country_capital_list);
	}
}

// 4. print list elements
static void print_list(void) {
    struct country_capital *entry;

    list_for_each_entry(entry, &country_capital_list, list)
        printk(KERN_INFO "%s - %s\n", entry->country, entry->capital);
}

// 5. cleanup the list and free it's elements
static void clean_list(void) {
    struct country_capital *entry;

    while (!list_empty(&country_capital_list)) {
    	entry = list_first_entry(&country_capital_list, struct country_capital, list);
    	printk(KERN_INFO "freeing %s\n", entry->country);
    	list_del(&entry->list);
    	kfree(entry);
    }
}


// #######################################################################
// HERE STARTS THE STANDARD MODULE CONFIG - SETUP - TEARDOWN
// #######################################################################

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateusz Midor");
MODULE_DESCRIPTION("A dynamic linux linked list example");
MODULE_VERSION("0.1");

static int __init lkm_init(void) {
    printk(KERN_INFO "Preparing dynamiclinkedlist module.\n");
    fill_list();
    print_list();
	return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit lkm_cleanup(void) {
	printk(KERN_INFO "Cleaning up dynamiclinkedlist module.\n\n");
	clean_list();
}


module_init(lkm_init);
module_exit(lkm_cleanup);
