/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   page_mapper.h
 * Author: root
 *
 * Created on November 15, 2017, 12:02 AM
 */
#ifndef PAGE_MAPPER_H
#define PAGE_MAPPER_H

#include <linux/cdev.h>

#define MODULE_NAME "page-mapper"

/// device main struct
struct page_mapper_dev
{
    struct class *cdev_class; /// needed for udev
    struct device *sysfs_dev; /// sysfs dev
    struct cdev chardev;	  /// char device structure
    u16 major_number; /// major mod number
    u32 minor_number; /// minor mod number
};


#endif  /* PAGE_MAPPER_H */
