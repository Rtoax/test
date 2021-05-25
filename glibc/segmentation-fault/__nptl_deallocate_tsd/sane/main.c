#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <malloc.h>
#include <sane/sane.h>

#define PTHREAD_STACK_MIN	16384

void* scan_thread(void *arg) {
    
    SANE_Status status;

    status = sane_init(NULL, NULL);
    assert(status == SANE_STATUS_GOOD);

    const SANE_Device** device_list = NULL;
    status = sane_get_devices(&device_list, false);
    assert(status == SANE_STATUS_GOOD);
	int i;

    for(i = 0; device_list[i] != NULL; ++i){
        printf("%s\n", device_list[i]->name);
    }

    sane_exit();
}

int main()
{    
	pthread_t t;
    pthread_attr_t attr;
    void *stackAddr = NULL;
    int paseSize = getpagesize();
    size_t stacksize = paseSize*4;
    
    pthread_attr_init(&attr);
    posix_memalign(&stackAddr, paseSize, stacksize);
    pthread_attr_setstack(&attr, stackAddr, stacksize);
    
	pthread_create(&t, NULL, scan_thread, NULL);
    pthread_join(t, NULL);
    return 0;
}
