#include <stdio.h>

#include <s-keys.h>

struct static_key key = STATIC_KEY_INIT_FALSE;

int main (int argc, char *argv[])
{
	jump_label_init();

	if (static_key_false(&key))
		printf("...\n");
	else
		printf("%d\n", argc);

    
    //在 static_key_slow_inc 执行前， main的 反汇编是这样的
    //Dump of assembler code for function main:
    //   0x00000000004007bd <+0>: push   %rbx
    //   0x00000000004007be <+1>: mov    %edi,%ebx
    //   0x00000000004007c0 <+3>: callq  0x4009c2 <jump_label_init>
    //   0x00000000004007c5 <+8>: nopl   0x0(%rax,%rax,1)
    //   0x00000000004007ca <+13>:    mov    %ebx,%esi
    //   ...
    //   0x00000000004007ec <+47>:    callq  0x4008d9 <static_key_slow_inc>
    //   0x00000000004007f1 <+52>:    nopl   0x0(%rax,%rax,1)
    //   0x00000000004007f6 <+57>:    mov    %ebx,%esi
    //   ...
    //   0x0000000000400818 <+91>:    callq  0x40093a <static_key_slow_dec>
    //   0x000000000040081d <+96>:    mov    $0x6020d0,%edi
    //   0x0000000000400822 <+101>:   callq  0x40093a <static_key_slow_dec>
    //   0x0000000000400827 <+106>:   nopl   0x0(%rax,%rax,1)
    //   0x000000000040082c <+111>:   mov    %ebx,%esi
    //   ...
    //   0x000000000040084f <+146>:   retq   
    //End of assembler dump.
	static_key_slow_inc(&key);

    //在 static_key_slow_inc 执行后， main的 反汇编是这样的(注意 被 jmp 替换掉的 nop 指令)
    //Dump of assembler code for function main:
    //   0x00000000004007bd <+0>: push   %rbx
    //   0x00000000004007be <+1>: mov    %edi,%ebx
    //   0x00000000004007c0 <+3>: callq  0x4009c2 <jump_label_init>
    //   0x00000000004007c5 <+8>: jmpq   0x4007dd <main+32>
    //   0x00000000004007ca <+13>:    mov    %ebx,%esi
    //   ...
    //   0x00000000004007ec <+47>:    callq  0x4008d9 <static_key_slow_inc>
    //   0x00000000004007f1 <+52>:    jmpq   0x400809 <main+76>
    //   0x00000000004007f6 <+57>:    mov    %ebx,%esi
    //   ...
    //   0x0000000000400818 <+91>:    callq  0x40093a <static_key_slow_dec>
    //   0x000000000040081d <+96>:    mov    $0x6020d0,%edi
    //   0x0000000000400822 <+101>:   callq  0x40093a <static_key_slow_dec>
    //   0x0000000000400827 <+106>:   jmpq   0x40083f <main+130>
    //   0x000000000040082c <+111>:   mov    %ebx,%esi
    //   ...
    //   0x000000000040084f <+146>:   retq   
    //End of assembler dump.

	if (static_key_false(&key))
		printf("...\n");
	else
		printf("%d\n", argc);

	static_key_slow_dec(&key);
	static_key_slow_dec(&key);

	if (static_key_false(&key))
		printf("...\n");
	else
		printf("%d\n", argc);

	return 0;
}


