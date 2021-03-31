#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main (void)
{    
   struct timespec ts = {5,0};

   printf("sleep five seconds\n");
   nanosleep(&ts, NULL);
   printf("end of sleep\n");

   return 0;
}
