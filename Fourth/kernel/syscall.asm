
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0
_NR_milli_sleep 	equ 1
_NR_print_str       equ 2
_NR_print_int 		equ 3
_NR_P				equ 4
_NR_V				equ 5
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global  milli_sleep
global  print_str
global  print_int
global	P
global	V

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

milli_sleep:
	mov	eax, _NR_milli_sleep
	mov ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret

print_str:
	mov	eax, _NR_print_str
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int	INT_VECTOR_SYS_CALL
	ret

print_int:
	mov	eax, _NR_print_int
	mov ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret

P:
	mov	eax, _NR_P
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int	INT_VECTOR_SYS_CALL
	ret

V:
	mov	eax, _NR_V
	mov ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret