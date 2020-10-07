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
    x_num_str       :   resb 21
    y_num_str       :   resb 21
    x_num_s         :   resb 1          ; 0是正的，1是负的
    y_num_s         :   resb 1
    x_num_len       :   resb 1
    y_num_len       :   resb 1
    char_input      :   resb 1
    byte_output    :   resb 1
    now_loc  : 	resq 1

section .text

_main:
    ; 输出询问语句
    mov rsi,    query_str
    mov rdx,    query_strlen
    call write
    
    mov rsi, x_num_str
    mov rdx, 21    
    call read
    
    mov rsi, y_num_str
    mov rdx, 21    
    call read

    mov rsi, x_num_str
    mov rdx, 21    
    call write
    
    mov rsi, y_num_str
    mov rdx, 21    
    call write

    jmp exit



; 将字符地址存入rsi, 字符长度存入drx后调用
write:
    push rcx
    mov rax,    1                               ; 系统调用号
    mov rdi,    1                                       ; 表示stdout
    syscall                                             ; 系统调用
    pop rcx
    ret

; 将字符地址存入rsi, 字符长度存入drx后调用
read:
    push rcx
    mov rax,  	0                               ; 系统调用号
    mov rdi, 	0                                       ; 表示stdin
    syscall                                             ; 系统调用
    pop rcx
    ret

exit:
    mov rax,    1                               ; 系统调用号
    syscall
    
b:	
	ret
