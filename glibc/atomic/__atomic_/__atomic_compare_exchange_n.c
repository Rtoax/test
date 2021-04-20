#include <stdio.h>
#include <stdbool.h>

int t1() {

    bool a = false;
    bool b = false;
    printf("%d \n", __atomic_compare_exchange_n(&a, &b, true, 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED));
    printf("a= %d, b = %d\n", a, b);
}
int t2() {

    int a = 12;
    int b = 123;
    printf("%d \n", __atomic_compare_exchange_n(&a, &b, 1024, 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED));
    printf("a= %d, b = %d\n", a, b);
    printf("a= %d, b = %d\n", a, b);
    printf("%d \n", __atomic_compare_exchange_n(&a, &b, 1024, 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED));
    printf("a= %d, b = %d\n", a, b);
}


int t3() {

    int a = 12;
    int b = 123;
    while(!__atomic_compare_exchange_n(&a, &b, 1024, 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
        usleep(1);
    }
    printf("a= %d, b = %d\n", a, b);
}

int main() {
//    t1();
    t2();
//    t3();
}

