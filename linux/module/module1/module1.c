/*************************************************************************
	> File Name: module1.c
	> Author: 
	> Mail: 
	> Created Time: 2017年12月18日 星期一 15时01分52秒
 ************************************************************************/

#include <linux/init.h>
#include <linux/module.h>

//许可证声明
MODULE_LICENSE("GPL");
//作者声明
MODULE_AUTHOR("zwj ReCode by [RToax]");
//模块功能描述
MODULE_DESCRIPTION("This module call str_len");

//外部函数参数声明
static char *string="linux";

//声明外部函数
extern int str_len(char *);

//加载模块时调用此函数
static int init_mod(void)
{
    int lenght = 0;
    lenght = str_len(string);
    printk(KERN_INFO "[RToax]Hello , i am module1!\n");
    printk(KERN_INFO "[RToax]Welcome to use the str_len function!\n");
    printk(KERN_INFO "[RToax]The lenght of string '%s' is :%d\n",string,lenght);

    return 0;
}

//卸载模块时调用此函数
static void exit_mod(void)
{
    printk(KERN_INFO "[RToax]Goodbye!\n");
}

module_param(string,charp,S_IRUGO);//定义模块参数
module_init(init_mod); //指定模块初始化函数
module_exit(exit_mod); //指定模块退出函数
