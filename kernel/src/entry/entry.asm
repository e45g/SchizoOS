extern kmain

section .text
global _start

_start:
    cli
    mov rsp, kernel_stack_top
    call kmain

    cli
    hlt
    jmp $


section .bss
align 4096
kernel_stack_bottom:
    resq 2048  ; 16KB stack
kernel_stack_top:
