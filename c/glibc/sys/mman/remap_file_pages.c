#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <memory.h>

#define LEN         8192
#define TEST_FILE   "text.out"

//mmap是将一个文件中的连续部分映射到虚拟内存中的一块连续的区域，它做的映射是线性映射，
//如果需要将一个文件中的不同部分以不同的顺序映射到虚拟内存中的连续区域，则需要使用多个
//mmap映射，从消耗的资源来看这样做代价比较昂贵（因为分配的vm_area_struct多了）。
//内核提供了一个系统调用叫做remap_file_pages，通过这个函数可以实现非线性映射，同时不需
//要分配更多的vm_area_struct。下面先看一下这个系统调用是怎么用的，然后说一下非线性映射
//在内核中是怎么实现的。

//本示例程序：
//将一个文件text的第一页和第二页颠倒映射，text文件的前4096个字节全部为字母a，后4096个字节全部为字母b。
//运行结果：
//    [rongtao@localhost mman]$ gcc remap_file_pages.c 
//    [rongtao@localhost mman]$ ./a.out 
//    aaaaaaaa
//    bbbbbbbb
//    aaaabbbb
//    bbbbbbbb
//    aaaaaaaa
//    bbbbaaaa

void gen_test_file() {
    FILE *fp = fopen(TEST_FILE, "w");
    int i;
    char ch = 'a';

    for(i=0;i<4096;i++)
        fwrite(&ch, 1, 1, fp);

    ch = 'b';
    for(i=0;i<4096;i++)
        fwrite(&ch, 1, 1, fp);
    
    fclose(fp);
}

void remove_test_file() {
    unlink(TEST_FILE);
}

int main(void)
{
    gen_test_file();

    int i, fd, ret;
    char str[LEN] = {0};
    char path[256] = {0};
    char* start = NULL;
    unsigned int sum = 0;

    fd = open(TEST_FILE, O_RDWR);

    ////先执行线性映射，映射文件text的前两页。
    start = mmap(NULL, LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(start == MAP_FAILED) {
        perror("mmap failed");
        return;
    }

    memcpy(str, start, 8);
    printf("%s\n", str);

    memcpy(str, start + 4096, 8);
    printf("%s\n", str);

    memcpy(str, start + 4096 - 4, 8);
    printf("%s\n", str);

    //将文件的第二页重新映射到start位置
    ret = remap_file_pages(start, LEN >> 1, 0, 1, MAP_SHARED);
    if (ret < 0) {
        perror("remap file pages failed1");
        munmap(start, LEN);
        close(fd);
        return 0;
    }
    //将文件的第一页重新映射到start+4096的位置。
    ret = remap_file_pages(start + 4096, LEN >> 1, 0, 0, MAP_SHARED);
    if (ret < 0) {
        perror("remap file pages failed2");
        munmap(start, LEN);
        close(fd);
        return 0;
    }

    memcpy(str, start, 8);
    printf("%s\n", str);

    memcpy(str, start + 4096, 8);
    printf("%s\n", str);

    memcpy(str, start + 4096 - 4, 8);
    printf("%s\n", str);

    munmap(start, LEN);
    close(fd);

    remove_test_file();
    
    return 0;
}

