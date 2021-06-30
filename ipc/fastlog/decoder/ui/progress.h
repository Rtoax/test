/**
 *  * linux terminal progress bar (no thread safe).
 *   *  @package progress.h.
 *    *
 *     * @author chenxin <chenxin619315@gmail.com>
 *      */
#ifndef progress_h
#define progress_h
 
#include <stdio.h>
 
typedef struct {
    char chr;       /*tip char*/
    char *title;    /*tip string*/
    int style;      /*progress style*/
    int max;        /*maximum value*/
    float offset;
    char *pro;
} progress_t;
 
#define PROGRESS_NUM_STYLE 0
#define PROGRESS_CHR_STYLE 1
#define PROGRESS_BGC_STYLE 2
 
extern void progress_init(progress_t *, char *, int, int);
 
extern void progress_show(progress_t *, float);
extern void progress_reset(progress_t *bar, char *title);

extern void progress_destroy(progress_t *);
 
#endif  /*ifndef*/

#if 0
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

#endif

