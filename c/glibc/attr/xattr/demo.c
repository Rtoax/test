#include<stdio.h>
#include<attr/xattr.h>
#include<sys/types.h>
#include<string.h>

int main(int argc, char *argv[])
{
    char *file = argv[1];

    char list[1024]={0};
    char value[1024]={0};
    size_t size,size2;
    int j=0;
    int i=0;
    int m=0;
    int k=0;
    char ch[100] = {0};
    char ch1[100]={0},ch2[100]={0};
    
    if(file) {
        strncpy(ch, file, 100);
    } else {
        printf("please input path or path+file:");
        scanf("%s",ch);
    }

    for(;j<100;j++) {
        
        if('\0'==ch[j])break;
        ch1[j]=ch[j];
    }
    
    printf("listxattr: %s\n", ch1);
    
    size = listxattr(ch1,list,1024);
    printf("listxattr: %d\n", size);

    for(;i<size;i++) {
        
        printf("%c",list[i]);
        
        if('\0'==list[i]) {
            
            size2 = getxattr(ch1,ch2,value,1024);
            printf("    %s\n",value);
            memset(ch2,0,100);
            k=0;
            continue;
        }
        ch2[k]=list[i];
        k++;
    }

    return 1;
}
