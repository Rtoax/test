/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   log.h
 * Author: root
 *
 * Created on November 15, 2017, 12:21 AM
 */

#ifndef LOG_H
#define LOG_H

/// macro for printk
#define LOG(fmt, args...) printk(KERN_INFO MODULE_NAME " %s(): " fmt, __FUNCTION__, ## args)
#define LOG_ERROR(fmt, args...) printk(KERN_ERR MODULE_NAME " Error: %s(): " fmt, __FUNCTION__, ## args)

#endif /* LOG_H */

