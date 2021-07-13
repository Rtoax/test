/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   page_mapper.c
 * Author: root
 *
 * Created on November 15, 2017, 12:40 AM
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>	
#include <linux/slab.h>		
#include <linux/fs.h>		
#include <linux/errno.h>	
#include <linux/types.h>	
#include <linux/fcntl.h>	
#include <linux/mm.h>

#include "./utils/log.h"
#include "chardev/chardev.h"

#include "page_mapper.h"

static struct page_mapper_dev *pm_device;

/**
 * @brief Register kernel module. Kernel call's it on insmod
 *
 * @return 0 on success
 *	   -1 error
 */
static int mod_init(void)
{
    int rc;
    
    /// Allocate memory for main device struct
    pm_device = kzalloc(sizeof(struct page_mapper_dev), GFP_KERNEL);
    if (!pm_device)
    {
	LOG_ERROR("Error: kzalloc failed. Cant allocate memory\n");

	return -1;
    }
    
    /// create character device
    rc = create_char_device(pm_device);
    if (rc == -1)
    {
        LOG_ERROR("Error: create_char_device failed\n");

	return -1;
    }
    
    LOG("Module successfully loaded\n");
    
    return 0;
}

/**
 * @brief Unregister kernel module. Kernel calls it on rmmod
 *
 * @return void
 */
static void mod_exit(void)
{
    destroy_char_device(pm_device);
    
    /// free mem
    if (pm_device)
        kfree(pm_device);
    
    LOG("Module unloaded\n");
}


MODULE_LICENSE("GPL");

module_init(mod_init);
module_exit(mod_exit);