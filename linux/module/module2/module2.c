/*************************************************************************
	> File Name: module2.c
	> Author: 
	> Mail: 
	> Created Time: 2017年12月18日 星期一 14时48分31秒
 ************************************************************************/

#include <linux/init.h>
#include <linux/module.h>

//许可证声明
MODULE_LICENSE("GPL");
//作者声明
MODULE_AUTHOR("zwj ReCode by [RToax]");
//模块功能描述
MODULE_DESCRIPTION("[RToax]This module provide a function to module1");

//计算字符串长度
int str_len(char *str)
{
    int count = 0;
    while(*str != '\0')
    {
        count++;
        str++;
    }

    return count;
}

//加载模块时调用此函数
static int init_mod(void)
{
    printk(KERN_INFO "[RToax]Hello,this module was install successfully!\n");
    return 0;
}

//卸载模块时，调用此函数
static void exit_mod(void)
{
    printk(KERN_INFO "[RToax]Goodbye,the module was uninstalled!\n");
}

//导出str_len函数，其他模块便可调用
EXPORT_SYMBOL(str_len);
//指定模块初始化函数init_mode()
module_init(init_mod);
//指定模块退出函数exit_mode()
module_exit(exit_mod);
