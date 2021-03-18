#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int mipsCPUInfo(const char *search_string) {
   const char *file_name = "/proc/cpuinfo";
   /* Simple detection of MIPS DSP ASE at runtime for Linux.
   * It is based on /proc/cpuinfo, which reveals hardware configuration
   * to user-space applications. */

   char cpuinfo_line[256];

   FILE *f = NULL;
   if ((f = fopen (file_name, "r")) == NULL)
     return 0;

   while (fgets (cpuinfo_line, sizeof (cpuinfo_line), f) != NULL)
   {
     if (strstr (cpuinfo_line, search_string) != NULL)
     {
         fclose (f);
         return 1;
     }
   }

   fclose (f);

   /* Did not find string in the /proc/cpuinfo file. */
   return 0;
}
