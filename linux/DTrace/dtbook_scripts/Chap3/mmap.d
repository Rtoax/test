#!/usr/sbin/dtrace -s

#pragma D option flowindent

syscall::mmap:entry
{
	self->flag = 1;
}
fbt:::
/self->flag/
{
}
syscall::mmap:return
/self->flag/
{
	self->flag = 0;
	exit(0);
}
