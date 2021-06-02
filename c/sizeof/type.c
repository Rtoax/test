#include <stdio.h>

#define S(type) printf("sizeof("#type") = %d\n", sizeof(type))

int main()
{
	S(int);
	S(long int);
	S(long);
	S(long long int);
	S(long long);

	S(short);
	S(short int);

	S(float);
	//$(long float);
	S(double);
	S(long double);

	S(char);
	//S(long char);
	
	//S(unsigned double);

}
