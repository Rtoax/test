.text

.globl _va_call

_va_call:
	.word 0
	callg *8(ap), *4(ap)
	ret
