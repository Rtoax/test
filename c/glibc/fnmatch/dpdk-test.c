#include "dpdk-fnmatch.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

// ./a.out "*.c*" ./

int main(int argc, char *argv[])
{
	char *pattern;
	DIR *dir;
 	struct dirent *entry;
	int ret;

	dir = opendir(argv[2]);	//打开指定路径

	pattern = argv[1];	
	
	//路径存在
	if(dir != NULL)
	{
		//逐个获取文件夹中文件
	    while( (entry = readdir(dir)) != NULL)
		{
	      ret = fnmatch(pattern, entry->d_name, FNM_PATHNAME|FNM_PERIOD);
	      if(ret == 0)			//符合pattern的结构
			{
	        printf("%s\n", entry->d_name);
	      }else if(ret == FNM_NOMATCH){
	        continue ;
	      }else
			{
	        printf("error file=%s\n", entry->d_name);
	    		}
	    	}

    	closedir(dir);
  	}
}

