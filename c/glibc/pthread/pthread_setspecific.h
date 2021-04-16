#include <pthread.h>

void* pthread_getspecific(pthread_key_t key);
	//返回：指向线程特定数据的指针（有可能是个空指针）
	
/**
 * pthread_setspecific
 *
 *      使用该函数可以为指定线程特定数据键设置线程特定绑定。
 *
 */
int pthread_setspecific(pthread_key_t key, const void *value);
	//返回：若成功则为0，若出错则为正的Exxx值  
	


