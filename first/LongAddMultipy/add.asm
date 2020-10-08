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
    quote_signal    :   db  `'`
    return_signal   :   db  `\n`
    ask_signal      :   db  `?`
    negative_signal :   db  `-`

section .bss
    x_num_str       :   resb 40
    y_num_str       :   resb 40
    print_num_str    :   resb 40
    x_num_s         :   resb 1          ; 0是正的，1是负的
    y_num_s         :   resb 1
    x_num_len       :   resb 1
    y_num_len       :   resb 1
    r_num_str	    :   resb 40
    r_num_s		:resb 1
    r_num_len		: resb 1
    char_input      :   resb 1
    char_output      :   resb 1

section .text

_main:
    ; 输出询问语句
    mov rsi,    query_str
    mov rdx,    query_strlen
    call write
    

    ; 读取第一个数字
    mov r8,     x_num_str
    mov r9,     x_num_s
    mov r12, 	x_num_len
    call        scan
    mov r8, x_num_str
    mov r9, x_num_len
    call leading_zero
    mov r8, x_num_str
    mov r9, 40
    call print_num
    ; 读取第二个数字
    mov r8,     y_num_str
    mov r9,     y_num_s
    mov r12, 	y_num_len
    call        scan
    mov r8, y_num_str
    mov r9, y_num_len
    call leading_zero
    mov r8, y_num_str
    mov r9, 40
    call print_num
    
    mov r8,     x_num_str
    mov r9, y_num_str
    mov r13, r_num_str
    call bigadd
    
    mov r8, r_num_str
    mov r9, 40
    call print_num
    
    jmp        exit

; r8 str first addr;
; r9 num signal
; r12 num len
; r10 use, 
; r11 str now addr, char_input读数
; r12 
scan:
    mov rsi,    char_input
    mov rdx,    1
    call        read
    mov r10b,   byte [char_input]
    cmp r10b,   byte [negative_signal]
    mov rcx, long 40
    jz  if_negative
    jnz if_positive
back_scan:
    mov r10b,    [char_input]
    sub r10b, 48
    mov [r8],   r10b
scan_loop:
    mov rsi,    char_input
    mov rdx,    1
    call        read
    mov r10b,   byte [char_input]
    cmp r10b,   byte [return_signal]
    jz exit_scan
    mov r11, r8
    add r11, 41
    sub r11, rcx
    sub r10b, 48
    mov [r11],   r10b
    loop scan_loop
exit_scan:
    mov r10, long 41
    sub r10, rcx
    mov byte [r12], r10b
    ret

if_negative:
    mov byte [r9],   1
    mov rsi,    char_input
    mov rdx,    long 1
    call        read
    jmp back_scan

if_positive:
    mov byte [r9],   0
    jmp back_scan


; r8 str first addr
; r9 num len
; r10 mov bits
; r11 now bit
leading_zero:
    mov r10b, 40
    sub r10b, byte [r9]
    mov cl, byte [r9]
mov_bits_loop:
    mov r11, r8
    add r11, rcx
    sub r11, 1
    mov r12b, byte [r11]
    mov [r11+r10], r12b
    loop mov_bits_loop
    mov cl, byte r10b
add_zeros_loop:
    mov r11, r8
    add r11, rcx
    sub r11, 1
    mov byte [r11], 0
    loop add_zeros_loop
    ret
	
	
; r8 first num(int not ascii)
; r9 second num(int not ascii)
; r13 result num(int not ascii)
; r10 now add bit
; r11 x bit; r12 y bit(int not ascii)
; r14 carry bit
bigadd:
    mov rcx, long 40
    mov r14, long 0
bigadd_loop:
    mov r11, long 0
    mov r12, long 0
    mov r11b , byte [r8+rcx-1]
    mov r12b , byte [r9+rcx-1]
    add r11, r12
    add r11, r14
    mov r14, long 0
    cmp r11, 9
    jle back_bigadd_loop
    mov r14, 1
    sub r11, 10
back_bigadd_loop:
    mov r10, r13
    add r10,rcx
    sub r10, 1
    mov byte [r10], r11b
    loop bigadd_loop
    ret


; r8 str first addr
; r9 num len(imm)
; r10 now bit
; r11 use
; r12 none zero bit
print_num:
    mov r12, long 0
    mov cl, r9b
print_num_loop:
    mov r10, r9
    sub r10, rcx
    add r10, 1
    mov r11b, byte[r8+r10]
    mov byte[print_num_str+r10], r11b
    add byte[print_num_str+r10], 48
    cmp r11b, 0
    jz back_print_num_loop
    cmp r12, 0
    jnz back_print_num_loop
    mov r12, r10
back_print_num_loop:
    loop print_num_loop
    mov rsi, print_num_str
    add rsi,r12
    mov rdx, r9
    sub rdx,r12
    call write
    mov rsi, return_signal
    mov rdx, long 1
    call write
    ret

; 将字符地址存入rsi, 字符长度存入rdx后调用
write:
    push rcx
    mov rax,    1                              ; 系统调用号
    mov rdi,    1                                       ; 表示stdout
    syscall                                             ; 系统调用
    pop rcx
    ret

; 将字符地址存入rsi, 字符长度存入drx后调用
read:
    push rcx
    mov rax,    0                               ; 系统调用号
    mov rdi,    0                                       ; 表示stdin
    syscall                                             ; 系统调用
    pop rcx
    ret

exit:
    mov rax,    60                               ; 系统调用号
    xor rdi, rdi
    syscall
    
b:
    ret
