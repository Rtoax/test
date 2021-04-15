#include <stdio.h>
#include <string.h>
#include <fnmatch.h>  
#include <sys/types.h>  
#include <dirent.h>  

static int lookup(const char *dir, const char *arg)
{
   DIR *dirp;
   struct dirent *dp;
//char name[1024];

   if ((dirp = opendir(dir)) == NULL) {
       perror(dir);
       return -1;
       }

   while ((dp = readdir(dirp)) != NULL) {
   if (fnmatch(arg, dp->d_name, 0) == 0){
    printf("%s\n", dp->d_name);
        }
     }
    (void) closedir(dirp);
   return 0;
}

void main()
{
	char dirname[1024];
	char filedir[1024]="/root";
	char filename[1024]="a_*";
	sprintf(dirname,"%s",lookup(filedir,filename));
	printf("%s",dirname);
	//lookup(filedir,filename);
}
