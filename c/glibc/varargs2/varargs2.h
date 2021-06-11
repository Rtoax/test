#ifndef VARARGS2_H
#define VARARGS2_H

#define va_stack(name, max) int name[max+1]
#define va_stack_init(stack) stack[0] = 0
#define va_push(stack, arg, type) *(type *)(&stack[stack[0]+1]) = (arg), \
				stack[0] += (sizeof(type) + sizeof(int) - 1) / sizeof(int)

#ifdef __STDC__
extern int va_call(int (*)(), int *);
#endif

#endif
