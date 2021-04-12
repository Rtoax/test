	.file	"compare.c"
	.text
	.globl	compare
	.type	compare, @function
compare:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	vmovss	%xmm0, -4(%rbp)
	vmovss	%xmm1, -8(%rbp)
	vmovss	-4(%rbp), %xmm0
	vucomiss	-8(%rbp), %xmm0
	jbe	.L6
	movl	$0, %eax
	jmp	.L4
.L6:
	movl	$1, %eax
.L4:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	compare, .-compare
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-39)"
	.section	.note.GNU-stack,"",@progbits
