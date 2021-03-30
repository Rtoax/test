#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

struct vty_struct {
	char name[64];
	int fd;

};

#define VTY_FD	1
struct vty_struct VTY = {
	.name = "rtoax",
	.fd = VTY_FD,
};

#if 1
#define my_assert(exp) {\
	if(!(exp)) {\
		printf("assert.\n");\
	}\
}
#else
#define my_assert(exp) assert(exp)
#endif

#define CHECK_VTY() do{\
	if(VTY.fd != VTY_FD) {\
		my_assert(0 && "vty 被篡改了!\n"); \
	} \
}while(0)

struct vty_struct * vty_create(char *name, int fd)
{
	struct vty_struct *vty = malloc(sizeof(struct vty_struct));
	assert(vty && "malloc failed.");
	strncpy(vty->name, name, sizeof(vty->name));
	vty->fd = fd;

	return vty;
}

int vty_print(struct vty_struct *vty, const char *fmt, ...) 
{
	vty->fd = 2;
	va_list args;
	int len = 0;
	char buf[1040] = {0};

	va_start(args, fmt);
	len = sprintf(buf, "\033[0m");
	len = vsnprintf(&buf[len], 1023, fmt, args);
	va_end(args);

	return write(vty->fd, buf, strlen(buf)+1);
}



int main() {
	
	vty_print(&VTY, "Hello %s\n", "World!");
	CHECK_VTY();
	vty_print(&VTY, "%22lu.%-2lu %17lu.%-2lu\r\n", 1L, 2L, 3L, -4L);

	struct vty_struct *vty = vty_create("rtoax2", 2);
	vty_print(vty, "Hello %s\n", "World!");
}
