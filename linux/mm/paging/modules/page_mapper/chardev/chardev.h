/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   chardev.h
 * Author: root
 *
 * Created on November 15, 2017, 3:13 AM
 */

#ifndef CHARDEV_H
#define CHARDEV_H


#include <linux/fs.h>

#include "../page_mapper.h"

#define MAJOR_NUMBER 0   ///dynamic major by default
#define DEV_NAME "mem2"  /// name of inode in /dev


extern void destroy_char_device(struct page_mapper_dev *pm_dev);
extern int create_char_device(struct page_mapper_dev *pm_dev);



#endif /* CHARDEV_H */

