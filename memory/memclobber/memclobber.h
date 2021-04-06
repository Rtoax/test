#ifndef __MeMCloBBER_H
#define __MeMCloBBER_H 1

#include <malloc.h>
#include <mcheck.h>


enum mclobber_status {
	MCLOBBER_OK,			//OK
	MCLOBBER_FREE,			//Double Free 错误
	MCLOBBER_HEAD,			//堆内存块的头部越界
	MCLOBBER_TAIL,			//堆内存块的尾部越界
};



static const char *const msafe_status_str[] = {
    "Memory OK",
    "Memory Double Free",
    "Memory was Clobbered Before The Block",
    "Memory was Clobbered After The Block",
    "Uknown Msafe Status",
};


#endif /*<__MeMCloBBER_H>*/
