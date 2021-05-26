/**
 *  证明CPU乱序执行，并使用内存屏障消除乱序执行的影响
 *  rtoax 荣涛
 *  2021年5月26日
 */
#include <stdio.h>
#include <pthread.h>


#ifdef HAVE_BARRIER
static void  inline mrwbarrier() { asm volatile("mfence":::"memory"); }
static void  inline mrbarrier()  { asm volatile("lfence":::"memory"); }
static void  inline mwbarrier()  { asm volatile("sfence":::"memory"); }
#else
#define mrwbarrier()
#define mrbarrier()
#define mwbarrier()
#warning define HAVE_BARRIER to solve Out-of-Order Exec.
#endif

volatile int x, y, a, b;

void *task1(void *arg)
{
	a = 1;
    mrwbarrier();
	x = b;
	pthread_exit(NULL);
}

void *task2(void *arg)
{
	b = 1;
    mrwbarrier();
	y = a;
	pthread_exit(NULL);
}

int main()
{
	int i, j;
	pthread_t tasks[2];

	i = 10000;
    
	while(i>=0) {
		x = y = a = b = 0;

		pthread_create(&tasks[0], NULL, task1, NULL);
		pthread_create(&tasks[1], NULL, task2, NULL);

		pthread_join(tasks[0], NULL);
		pthread_join(tasks[1], NULL);

		if(x ==0 && y == 0) {
			printf("CPU Out of Order Exec Happend.\n");
		}
	}

}
