struct Foo
{
  int m;
};

/**
 *	sudo bpftrace -e 'struct Foo {int m;} uprobe:./a.out:func { $s = (struct Foo *) arg0; @x = $s->m; exit(); }'
 */
int func(struct Foo *foo)
{
  return foo->m;
}

int main()
{
  struct Foo foo;
  foo.m = 2;
  //while(1) {
	foo.m++;
	func(&foo);
  //}
  return 0;
}
