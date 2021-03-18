; https://stackoverflow.com/questions/44613592/shared-library-injection-dl-relocate-object-segfaults
; 

global inject_library
global nullsub

section .data
section .text

inject_library:
; rdi -> Pointer to malloc()
; rsi -> Pointer to free()
; rdx -> Pointer to dlopen()
; rcx -> Size of the path to the .so to load

; Create a new stack frame
push rbp
sub rsp, 0x8
; Do stuff
; Fix the stack pointer
add rsp, 0x8

; Save rbx because we're using it as scratch space
push rbx
; Save addresses of free & dlopen on the stack
push rsi
push rdx

; Move the pointer to malloc into rbx
mov rbx, rdi
; Move the size of the path as the first argument to malloc
mov rdi, rcx
; Call malloc(so_path_size)
call rbx
; Stop so that we can see what's happening from the injector process
int 0x3

; Move the pointer to dlopen into rbx
pop rbx
; Move the malloc'd space (now containing the path) to rdi for the first argument
mov rdi, rax
; Push rax because it'll be overwritten
push rax
; Second argument to dlopen (RTLD_NOW)
mov rsi, 0x2
; Call dlopen(path_to_library, RTLD_NOW)
call rbx
; Pass control to the injector
int 0x3

; Finally, begin free-ing the malloc'd area
pop rdi
; Get the address of free into rbx
pop rbx
; Call free(path_to_library)
call rbx

; Restore rbx
pop rbx

; Destory the stack frame
pop rbp

; We're done
int 0x3
retn

nullsub:
retn

