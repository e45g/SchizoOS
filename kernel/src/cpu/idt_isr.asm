global isr_stub_table
global keyboard_stub
extern keyboard_handle
extern exception_handler

isr_stub_table:
%assign i 0
%rep 32
    dq isr_stub_%+i
%assign i i+1
%endrep

%macro regs_push 0
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
%endmacro

%macro regs_pop 0
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
%endmacro

keyboard_stub:
    regs_push
    call keyboard_handle
    regs_pop
    iretq

%macro isr_err_stub 1
isr_stub_%+%1:
    regs_push
    mov rdi, %1          ; vector number in rdi
    mov rsi, [rsp]       ; error code in rsir
    call exception_handler
    add rsp, 8
    regs_pop
    iretq
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push 0
    regs_push
    mov rdi, %1          ; 1st argument: vector number
    mov rsi, 0           ; 2nd argument: 0 for error code
    call exception_handler
    add rsp, 8           ; clean up the simulated error code
    regs_pop
    iretq
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31
