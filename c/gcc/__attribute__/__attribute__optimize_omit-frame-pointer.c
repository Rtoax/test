
#ifdef OFF
#define _frame_pointer	__attribute__((optimize("omit-frame-pointer")))
#else
#define _frame_pointer
#endif

_frame_pointer int foo()
{
	int i = 0;
	return i+1;
}
