
int main()
{
#ifdef VOL
	volatile
#else
//#error define VOL to enable volatile.
#endif
	int a = 0;

	a = 1;
}
