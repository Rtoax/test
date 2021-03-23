; ----------------------------------------------------------------------------------------
; 仅使用系统调用来输出 "Hello, World" 到控制台。 这个程序仅在 64 位的 Linux 下运行。
; 如何编译执行:
;
;     nasm -felf64 hello.asm && ld hello.o && ./a.out
; ----------------------------------------------------------------------------------------

        global  _start

        section .text
_start:
        ; write(1, message, 13)
        mov     rax, 1                  ; 1 号系统调用是写操作 
        mov     rdi, 1                  ; 1 号文件系统调用是标准输出 
        mov     rsi, message            ; 输出字符串的地址 
        mov     rdx, 13                 ; 字符串的长度 
        syscall                         ; 调用系统执行写操作 

        ; exit(0)
        mov     eax, 60                 ; 60 号系统调用是退出 
        xor     rdi, rdi                ; 0 号系统调用作为退出 
        syscall                         ; 调用系统执行退出 
		section .data
message:
        db      "Hello, World", 10      ; 注意到最后的换行 

