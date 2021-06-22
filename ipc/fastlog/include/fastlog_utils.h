#ifndef __fAStLOG_UTILS_H
#define __fAStLOG_UTILS_H 1

#ifndef __fAStLOG_H
#error "You can't include <fastlog_utils.h> directly, #include <fastlog.h> instead."
#endif

#ifdef FASTLOG_HAVE_LIBXML2

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>

#endif /*<FASTLOG_HAVE_LIBXML2>*/


#endif /*<__fAStLOG_UTILS_H>*/
