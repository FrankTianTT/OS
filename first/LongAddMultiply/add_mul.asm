global _main

; 系统调用参数的顺序为
; %rdi %rsi %rdx %rcx
; %rbx通常由被调用者保护
; %rax作为返回值

section .data
    query_str       :   db  `Please input x and y:\n`
    query_strlen    :   equ $-query_str
    num_max_len     :   equ 100
    return_signal   :   db  `\n`
    negative_signal :   db  `-`
    space_signal    :   db  ` `

section .bss
    x_num_str       :   resb num_max_len
    y_num_str       :   resb num_max_len
    print_num_str   :   resb num_max_len
    x_num_s         :   resb 1          ; 0是正的，1是负的
    y_num_s         :   resb 1
    x_num_len       :   resb 1
    y_num_len       :   resb 1
    add_result_str	:   resb num_max_len
    char_input      :   resb 1
    char_output     :   resb 1
    
    
    single_mul_str  :   resb num_max_len
    mul_r1_str      :   resb num_max_len
    mul_r2_str      :   resb num_max_len
    mul_r3_str      :   resb num_max_len
    mul_r4_str      :   resb num_max_len
    mul_r5_str      :   resb num_max_len
    mul_r6_str      :   resb num_max_len
    shift_reg       :   resb num_max_len
    mul_result_str  :   resb num_max_len

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
    mov r8,     x_num_str
    mov r9,     x_num_len
    call        leading_zero
    ; 读取第二个数字
    mov r8,     y_num_str
    mov r9,     y_num_s
    mov r12, 	y_num_len
    call        scan
    mov r8,     y_num_str
    mov r9,     y_num_len
    call        leading_zero
    
    mov r8,     x_num_str
    mov r9,     y_num_str
    mov r13,    add_result_str
    call        bigadd
    mov r8,     add_result_str
    call print_num
    
    mov r8,     x_num_str
    mov r9,     y_num_str
    mov r13,    mul_result_str
    call bigmul
    mov r8,     mul_result_str
    call print_num
    
    ; 结束程序
    jmp        exit
    
    
;-----------------------------------------------------------;
; 过程scan，用于扫描一个最多为40位的整数                      
; parameter reg ; r8 str first addr                         ;
;               ; r9 num signal save addr                   ;
;               ; r12 num len save addr                     ;
; use reg       ; r10, r11, r12                             ;
; use memory    ; char_input                                ;
;               ; negative_signal                           ;
;               ; return_signal                             ;
scan:
    mov rsi,    char_input
    mov rdx,    1
    call        read
    mov r10b,   byte [char_input]
    cmp r10b,   byte [negative_signal]
    mov rcx,    long num_max_len
    jz          if_negative
    jnz         if_positive
back_scan:
    mov r10b,   [char_input]
    sub r10b,   48
    mov [r8],   r10b
scan_loop:
    mov rsi,    char_input
    mov rdx,    1
    call        read
    mov r10b,   byte [char_input]
    cmp r10b,   byte [return_signal]
    jz          exit_scan
    cmp r10b,   byte [space_signal]
    jz          exit_scan
    mov r11,    r8
    add r11,    num_max_len
    inc r11
    sub r11,    rcx
    sub r10b,   48
    mov [r11],  r10b
    loop        scan_loop
exit_scan:
    mov r10,    num_max_len
    inc r10
    sub r10,    rcx
    mov [r12],  r10b
    ret
if_negative:
    mov [r9],   byte 1
    mov rsi,    char_input
    mov rdx,    long 1
    call        read
    jmp         back_scan
if_positive:
    mov [r9],   byte 0
    jmp         back_scan


;-----------------------------------------------------------;
; 过程leading_zero, 用来给扫描到到数字补零
; parameter reg ; r8 str first addr                         ;
;               ; r9 num len save addr                      ;
; use reg       ; r10 num of mov bits                       ;
;               ; r11 now bit                               ;
leading_zero:
    mov r10b,   num_max_len
    sub r10b,   byte [r9]
    mov cl,     byte [r9]
mov_bits_loop:
    mov r11,    r8
    add r11,    rcx
    sub r11,    1
    mov r12b,   byte [r11]
    mov [r11+r10],  r12b
    loop        mov_bits_loop
    mov cl,     r10b
add_zeros_loop:
    mov r11,    r8
    add r11,    rcx
    sub r11,    1
    mov [r11],  byte 0
    loop        add_zeros_loop
    ret


;-----------------------------------------------------------;
; 过程bigadd, 用来大数相加
; parameter reg ; r8 first str first addr                   ;
;               ; r8 second str first addr                  ;
;               ; r13 result str first addr                 ;
; use reg       ; r10 num of now adding bit                 ;
;               ; r11 now adding bit of first num           ;
;               ; r12 now adding bit of second num          ;
;               ; r13                                       ;
;               ; r14 carray bit                            ;
bigadd:
    push r10
    push r11
    push r12
    push r13
    push r14
    push rcx
    mov rcx,    num_max_len
    mov r14,    long 0
bigadd_loop:
    mov r11,    long 0
    mov r12,    long 0
    mov r11b,   byte [r8+rcx-1]
    mov r12b,   byte [r9+rcx-1]
    add r11,    r12
    add r11,    r14
    mov r14,    long 0
    cmp r11,    9
    jle         back_bigadd_loop
    mov r14,    1
    sub r11,    10
back_bigadd_loop:
    mov r10,    r13
    add r10,    rcx
    sub r10,    1
    mov [r10],  r11b
    loop        bigadd_loop
    pop rcx
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    ret


;-----------------------------------------------------------;
; single_mul
; parameter reg ; r8 str first addr                         ;
;               ; r9 the number want to multiply(0~9)       ;
; use reg       ; r10 now bit                               ;
;               ; r11                                       ;
;               ; r12                                       ;
; use momery    ; mul_r1_str                                ;
;               ; mul_r2_str                                ;
single_mul:
    push r10
    push r11
    push r12
    push r13
    push rcx
    mov rcx,    long num_max_len
    mov r13,    mul_r2_str
    call set_zeros
single_mul_loop:
    mov r13,    mul_r1_str
    call set_zeros
    mov r10,    rcx
    sub r10,    1
    mov al,     byte [r8+r10]
    mul r9b
    mov rdx,     0
    mov r12,   long 10
    div r12
    mov [mul_r1_str+r10-1], al
    mov [mul_r1_str+r10],   dl
    ;--------------------------------------
    ; add
    push r8
    push r9
    push r13
    mov r8,     mul_r1_str
    mov r9,     mul_r2_str
    mov r13,    mul_r3_str
    call bigadd
    mov r8,     mul_r2_str
    mov r9,     mul_r3_str
    call copy
    pop r13
    pop r9
    pop r8
    ;--------------------------------------
    dec rcx
    jnz         single_mul_loop
    ;--------------------------------------
    ; save
    push r8
    push r9
    mov r8,     single_mul_str
    mov r9,     mul_r2_str
    call copy
    pop r9
    pop r8
    ;--------------------------------------
    pop rcx
    pop r13
    pop r12
    pop r11
    pop r10
    ret


;-----------------------------------------------------------;
; bigmul
; parameter reg ; r8 first str first addr                   ;
;               ; r9 second str first addr                  ;
;               ; r13 result str first addr                 ;
;               ; r10
; use memory    ; mul_r4_str                                ;
;               ; mul_r5_str                                ;
bigmul: 
    call b
    mov rcx,    num_max_len
    ;--------------------------------------
    ; call set_zeros
    push r13
    mov r13,    mul_r4_str
    call set_zeros
    pop r13
    ;--------------------------------------
bigmul_loop:
    mov r10,    rcx
    mov r11b,   byte [r9+r10-1]
    ;--------------------------------------
    ; call single_mul
    push r8
    push r9
    mov r9 ,    long 0
    mov r9b,    r11b
    call single_mul
    pop r9
    pop r8
    ;--------------------------------------
    ; call set_zeros
    ; clear mul_r5_str
    push r13
    mov r13,    mul_r5_str
    call set_zeros
    pop r13
    ;--------------------------------------
    ; call left_shift
    ; left shift single_mul
    push r8
    push r9
    mov r8, single_mul_str
    mov r9, num_max_len
    sub r9, rcx
    call left_shift
    mov r8, single_mul_str
    mov r9, shift_reg
    call copy
    pop r9
    pop r8
    ;--------------------------------------
    ; call bigadd
    ; mul_r5_str = mul_r4_str + single_mul_str
    push r8
    push r9
    push r13
    mov r8, mul_r4_str
    mov r9, single_mul_str
    mov r13,mul_r5_str
    call bigadd
    pop r13
    pop r9
    pop r8
    ;--------------------------------------
    ; call copy
    push r8
    push r9
    mov r8, mul_r4_str
    mov r9, mul_r5_str
    call copy
    pop r9
    pop r8
    ;--------------------------------------
    dec rcx
    jnz bigmul_loop
    mov r8, r13
    mov r9, mul_r4_str
    call copy
    
;-----------------------------------------------------------;
; 过程copy r9 -> r8
; parameter reg ; r8 str first addr                         ;
;               ; r9 str first addr                         ;
; use reg       ; r10 now bit                               ;
copy:
    push rcx
    push r10
    mov rcx,    long num_max_len
copy_loop:
    mov r10b,   [r9+rcx-1]
    mov [r8+rcx-1], r10b
    loop   copy_loop
    pop r10
    pop rcx
    ret

;-----------------------------------------------------------;
; 过程left_shift
; parameter reg ; r8 str first addr                         ;
;               ; r9 num of shift bit
; use reg       ; r10 now shift bit
left_shift:
    push r10
    push r11
    push rcx
    mov rcx,    num_max_len
    sub rcx,    r9
    ;--------------------------------------
    ; call set_zeros
    push r13
    mov r13,    shift_reg
    call set_zeros
    pop r13
    ;--------------------------------------
left_shift_loop:
    mov r10,    r9
    add r10,    num_max_len
    sub r10,    rcx
    mov r11b,   byte [r8+r10]
    sub r10,    r9
    mov [shift_reg+r10],    r11b
    loop left_shift_loop
    pop rcx
    pop r11
    pop r10
    ret
    

;-----------------------------------------------------------;
; 过程print_num, 输出数字(不含前导零)
; parameter reg ; r8 str first addr                         ;
; use reg       ; r10 now bit                               ;
;               ; r11                                       ;
;               ; r12 num of the first none zero bit        ;
; use momery    ; return_signal                             ;
print_num:
    push r10
    push r11
    push r12
    push rcx
    mov r12,    long 0
    mov rcx,    long num_max_len
print_num_loop:
    mov r10,    long num_max_len
    sub r10,    rcx
    mov r11b,   byte [r8+r10]
    mov byte[print_num_str+r10],    r11b
    add byte[print_num_str+r10],    48
    cmp r11b,   0
    jz          back_print_num_loop
    cmp r12,    0
    jnz         back_print_num_loop
    mov r12,    r10
back_print_num_loop:
    loop        print_num_loop
    ; following code deal with all zero
    cmp r12,    0
    jnz         print_ascii_num
    mov r11b,   byte [r8+num_max_len-1]
    cmp r11b,   byte 0
    jz          print_zero
    jnz         print_ascii_num
print_ascii_num: 
    mov rsi,    print_num_str
    add rsi,    r12
    mov rdx,    long num_max_len
    sub rdx,    r12
    call        write
    jmp         exit_print_num
print_zero:
    mov [char_output],  byte 48
    mov rsi,    char_output
    mov rdx,    long 1
    call        write
exit_print_num:
    mov rsi,    return_signal
    mov rdx,    long 1
    call        write
    pop rcx
    pop r12
    pop r11
    pop r10
    ret


;-----------------------------------------------------------;
; 过程write
write:
    push rcx
    mov rax,    1                              ; 系统调用号
    mov rdi,    1                                       ; 表示stdout
    syscall                                             ; 系统调用
    pop rcx
    ret


;-----------------------------------------------------------;
; 过程read
read:
    push rcx
    mov rax,    0                               ; 系统调用号
    mov rdi,    0                                       ; 表示stdin
    syscall                                             ; 系统调用
    pop rcx
    ret


;-----------------------------------------------------------;
; set_zeros
; parameter reg ; r13 str first addr                        ;
; use reg       ; r9                                        ;
set_zeros:
    push rcx
    push r9
    mov rcx,    long num_max_len
set_zeros_loop:
    mov r9,     long num_max_len
    sub r9,     rcx
    mov [r13+r9],byte 0
    loop        set_zeros_loop
    pop r9
    pop rcx
    ret

;-----------------------------------------------------------;
exit:
    mov rax,    60                               ; 系统调用号
    xor rdi, rdi
    syscall


;-----------------------------------------------------------;
b:
    ret
