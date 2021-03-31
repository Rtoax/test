#include <stdio.h>

//void gotoxy(int x, int y){printf("%c[%d;%df", 0x1B, y, x);}

void memshow(const char *prefix, void *ptr, ssize_t size, int reverse)
{
//  format:  
//	0x0000:  4500 0034 fcd2 4000 4006 3fef 7f00 0001
//	0x0010:  7f00 0001 988e 268f 2b67 f87a 7d08 8461
//	0x0020:  8010 01f8 fe28 0000 0101 080a a355 7aed
    unsigned char *byte = ((unsigned char *)ptr)+(reverse?size:0);
    unsigned char *line_hdr=NULL;
    int i=0, j=0, cnt=0;
    int line=0;
    for(i=0;i<size;i++){
        if(cnt%16==0) {
            line=printf("%s0x%04x:  ", prefix, cnt);
            line_hdr = byte;
        }

        printf("%02x", *byte);
        
        if(cnt%2)printf(" ");
        byte += reverse?(-1):(1);

        cnt++;
        if(cnt%16==0) {
            printf(" | ");
            for(j=0;j<16;j++){
                if(i+j<size){
                    char _ch = *line_hdr;
                    if(isalnum(_ch) || isalpha(_ch) || isascii(_ch) || isdigit(_ch) || isspace(_ch))
                        printf("%c", _ch);
                    else printf("%c", '.');
                    line_hdr++;
                }
            }
            printf("\n");
            fflush(stdout);
        }
    }
    printf("\n");
    fflush(stdout);
}

#ifdef TEST
int main()
{
    char str[] = {
        "1234567890abcdefghijklmnopqrstuvwxyz:;,."
    };

    memshow(">>  ", str, sizeof(str), 0);
    memshow(">>>>", str, sizeof(str), 1);
}
#endif
