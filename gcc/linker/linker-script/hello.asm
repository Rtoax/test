section .data
	msg: db "hello, world",'\n'

section .text
	global _start
  
_start:
	; write(1, msg, 13)
	mov    rax, 1
	mov    rdi, 1
	mov    rsi, msg
	mov    rdx, 14
	syscall

	; exit(0)
	mov    rax, 60
	mov	    rdi, 0
	syscall
