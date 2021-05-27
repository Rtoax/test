#include <stdio.h>

#define V1	0x0001
#define V2	0x0010
#define V3	0xf000
#define V4	0xf001
#define VN	0xffff

void switch_case(int v)
{
	switch(v) {
		case V1:printf("V1\n");break;
		case V2:printf("V2\n");break;
		case V3 ... VN:
			printf("V3 ... VN\n");
			break;
		default:
			break;
	}
}

int main()
{
	switch_case(V1);		
	switch_case(V2);
	switch_case(V3);
	switch_case(V4);
	switch_case(VN);
}
