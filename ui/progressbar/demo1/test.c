/**
 * program bar test program.
 *
 * @author chenxin <chenxin619315@gmail.com>
 */
#include "progress.h"
#include <unistd.h>
 
int main(int argc, char *argv[] )
{
    progress_t bar;
//    progress_init(&bar, "", 50, PROGRESS_NUM_STYLE);
    progress_init(&bar, "Meta ", 50, PROGRESS_CHR_STYLE);
//    progress_init(&bar, "", 50, PROGRESS_BGC_STYLE);
 
    int i;
    for ( i = 0; i <= 50; i++ ) {
        progress_show(&bar, i/50.0f);
        usleep(30000);
    }
    
    printf("\n+-Done\n");
    progress_reset(&bar, "Logs ");
    
    for ( i = 0; i <= 100; i++ ) {
        progress_show(&bar, i/100.0f);
        usleep(20000);
    }
    
    printf("\n+-Done\n");
    
    progress_destroy(&bar);
 
    return 0;
}

