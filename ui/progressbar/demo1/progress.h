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

