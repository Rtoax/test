#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <sane/sane.h>

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

int main() {
    
	pthread_t t;
	pthread_create(&t, NULL, scan_thread, NULL);
    pthread_join(t, NULL);
    return 0;
}
