global _main

; 系统调用参数的顺序为
; %rdi %rsi %rdx %rcx
; %rbx通常由被调用者保护
; %rax作为返回值

%macro pusha 0
    push    rbx
    push    rcx
    push    rdx
    push    rsi
    push    rdi
%endmacro

%macro popa 0
    pop     rdi
    pop     rsi
    pop     rdx
    pop     rcx
    pop     rbx
%endmacro

section .data
    query_str       :   db  `Please input x and y:\n`
    query_strlen    :   equ $-query_str
    quote_signal    :   equ  `'`
    return_signal   :   equ  `\n`
    ask_signal      :   equ  `?`
    negative_signal :   equ  `-`

section .bss
    x_num_str       :   resb 20
    y_num_str       :   resb 20
    x_num_s         :   resb 1          ; 0是正的，1是负的
    y_num_s         :   resb 1
    x_num_len       :   resb 1
    y_num_len       :   resb 1
    char_input      :   resb 1

section .text

_main:
    ; 输出询问语句
    mov rsi,    query_str
    mov rdx,    query_strlen
    call write
    

    ; 读取第一个数字
    mov r8,     x_num_str
    mov r9,     x_num_s
    call        scan_20bits
    call        exit


    ; 读取第二个数字
    mov r8,     y_num_str
    mov r9,     y_num_s
    call        scan_20bits
    call        exit

    mov r8,     r8
    mov r9,     20
    call        write
    call        exit

; 将num首地址存入r8, 符号存入r9, r10保存位数, char_input读数
scan_20bits:
    pusha
    mov r10,    0
    mov rsi,    char_input
    mov rdi,    1
    call        read
    mov r12b,   [char_input]
    cmp r12b,   negative_signal
    jz  if_negative
    jnz if_positive
    inc r10
    call scan_loop
    popa
    ret

if_negative:
    mov [r9],   long 1
    mov rsi,    char_input
    mov rdi,    1
    call        read
    mov r12,    [char_input]
    mov [r8],   r12
    ret

if_positive:
    mov [r9],   long 0
    mov r12,    [char_input]
    mov [r8],   r12
    ret

scan_loop:
    cmp r10,    20
    jz  close_scan_loop
    mov rsi,    char_input
    mov rdi,    1
    call        read
    mov r12,    [char_input+r10]
    mov [r8],   r12
    inc r10
    call scan_loop
close_scan_loop:
    ret

; 将字符地址存入rsi, 字符长度存入drx后调用
write:
    pusha
    mov rax,    0x2000004                               ; 系统调用号
    mov rdi,    1                                       ; 表示stdout
    syscall                                             ; 系统调用
    popa
    ret

; 将字符地址存入rsi, 字符长度存入drx后调用
read:
    pusha
    mov rax,    0x2000003                               ; 系统调用号
    mov rdi,    0                                       ; 表示stdin
    syscall                                             ; 系统调用
    popa
    ret

exit:
    mov rax,    0x2000001                               ; 系统调用号
    syscall
    ret