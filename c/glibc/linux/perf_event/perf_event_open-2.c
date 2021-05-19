/*
https://stackoverflow.com/questions/42088515/perf-event-open-how-to-monitoring-multiple-events

perf stat -e cycles,faults ls

*/
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <asm/unistd.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>

struct read_format {
    uint64_t nr;
    struct {
        uint64_t value;
        uint64_t id;
    } values[];
};


void do_malloc() {

    int i;
    char* ptr;
    int len = 2*1024*1024;
    ptr = malloc(len);
    
    mlock(ptr, len);
    
    for (i = 0; i < len; i++) {
        ptr[i] = (char) (i & 0xff); // pagefault
    }
    
    free(ptr);
}

void do_ls() {
    system("/bin/ls");
}

void do_something(int something) {
    
    switch(something) {
    case 1:
        do_ls();
        break;
    case 0:
    default:
        do_malloc();
        break;
    }
}


int create_hardware_perf(int grp_fd, enum perf_hw_id hw_ids, uint64_t *ioc_id)
{
    if(PERF_COUNT_HW_MAX <= hw_ids || hw_ids < 0) {
        printf("Unsupport enum perf_hw_id.\n");
        return -1;
    }
    
    struct perf_event_attr pea;
    
    memset(&pea, 0, sizeof(struct perf_event_attr));
    pea.type = PERF_TYPE_HARDWARE;
    pea.size = sizeof(struct perf_event_attr);
    pea.config = hw_ids;
    pea.disabled = 1;
    pea.exclude_kernel = 1;
    pea.exclude_hv = 1;
    pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
    int fd = syscall(__NR_perf_event_open, &pea, 0, -1, grp_fd>2?grp_fd:-1, 0);
    ioctl(fd, PERF_EVENT_IOC_ID, ioc_id);

    return fd;
}


int create_software_perf(int grp_fd, enum perf_sw_ids sw_ids, uint64_t *ioc_id)
{
    if(PERF_COUNT_SW_MAX <= sw_ids || sw_ids < 0) {
        printf("Unsupport enum perf_sw_ids.\n");
        return -1;
    }

    struct perf_event_attr pea;
    
    memset(&pea, 0, sizeof(struct perf_event_attr));
    pea.type = PERF_TYPE_SOFTWARE;
    pea.size = sizeof(struct perf_event_attr);
    pea.config = sw_ids;
    pea.disabled = 1;
    pea.exclude_kernel = 1;
    pea.exclude_hv = 1;
    pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
    int fd = syscall(__NR_perf_event_open, &pea, 0, -1, grp_fd>2?grp_fd:-1 /*!!!*/, 0);
    ioctl(fd, PERF_EVENT_IOC_ID, ioc_id);

    return fd;
}


int main(int argc, char* argv[]) 
{
    struct perf_event_attr pea;
    
    int group_fd, fd2, fd3, fd4, fd5;
    uint64_t id1, id2, id3, id4, id5;
    uint64_t val1, val2, val3, val4, val5;
    char buf[4096];
    struct read_format* rf = (struct read_format*) buf;
    int i;

    group_fd = create_hardware_perf(-1, PERF_COUNT_HW_CPU_CYCLES, &id1);
    
    fd2 = create_hardware_perf(group_fd, PERF_COUNT_HW_CACHE_MISSES, &id2);
    fd3 = create_software_perf(group_fd, PERF_COUNT_SW_PAGE_FAULTS, &id3);
    fd4 = create_software_perf(group_fd, PERF_COUNT_SW_CPU_CLOCK, &id4);
    fd5 = create_software_perf(group_fd, PERF_COUNT_SW_CPU_CLOCK, &id5);

    printf("ioctl %ld, %ld, %ld, %ld, %ld\n", id1, id2, id3, id4, id5);

    ioctl(group_fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
    ioctl(group_fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

    do_something(1);
    
    ioctl(group_fd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);


    read(group_fd, buf, sizeof(buf));
    for (i = 0; i < rf->nr; i++) {
        if (rf->values[i].id == id1) {
            val1 = rf->values[i].value;
        } else if (rf->values[i].id == id2) {
            val2 = rf->values[i].value;
        } else if (rf->values[i].id == id3) {
            val3 = rf->values[i].value;
        } else if (rf->values[i].id == id4) {
            val4 = rf->values[i].value;
        } else if (rf->values[i].id == id5) {
            val5 = rf->values[i].value;
        }
    }

    printf("cpu cycles:     %"PRIu64"\n", val1);
    printf("cache misses:   %"PRIu64"\n", val2);
    printf("page faults:    %"PRIu64"\n", val3);
    printf(" cpu clock:     %"PRIu64"\n", val4);
    printf("task clock:     %"PRIu64"\n", val5);

    close(group_fd);
    close(fd2);
    close(fd3);
    close(fd4);
    close(fd5);

    return 0;
}

