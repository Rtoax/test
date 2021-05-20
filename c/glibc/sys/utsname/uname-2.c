//https://stackoverflow.com/questions/9358681/how-can-i-get-the-processor-id-of-the-current-process-in-c-in-linux
#include<stdio.h>
#include<sys/utsname.h>

int main()
{

char hostname[1024];
struct utsname userinfo;
if(uname(&userinfo)>=0)
{
  printf("\n***** System Details ******\n");
  printf("System Name    : %s\n",userinfo.sysname);
  printf("System Node    : %s\n",userinfo.nodename);
  printf("System Release : %s\n",userinfo.release);
  printf("System Version : %s\n",userinfo.version);
  printf("System Machine : %s\n",userinfo.machine);
}
else
 printf("\nSystem details fetch failed..\n");



if(gethostname(&hostname,1024)==0)
{
  printf("Hostname : %s\n",hostname);
}
else
 printf("\nHostname details fetch failed..\n");

return 0;
}
