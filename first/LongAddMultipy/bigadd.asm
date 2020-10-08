gloabl start
section .data
sta	db 200 dump(?)
sum	db 6 dump(?)
coa	db 9,2,5,6,2		;;datas
cob	db 3,4,9,4,1
;	1 2 7 5 0  3
sps	dw ? 	
start:	
	mov sps,sp
	mov sp,offset sta
	add sp,20
	
	mov ax,5	;传参
	call addf	;	调用大数加法
	mov ax,di[bx]
	;--------------------show
	mov bx,offset sum
	mov ah,02h	;传参int21h
	mov cx,5	;显示计数器
	mov dl,[bx]	;分类讨论第一位，是否为0
	inc bx		;
	cmp dl,0
	je lp1		;第一位是0的话就不显示，直接开始循环
	add dl,'0'
	int 21h
	
lp1:	
	mov dl,[bx]
	add dl,'0'
	inc bx
	int 21h
	
	loop lp1
	
	mov sp,sps;整个程序的结束
	ret	;---------end show
addf:	;ax保存位数 函数————————————————————————proc	大数

	add si,ax
	add di,ax
	mov cx,ax
	dec si
	mov bl,0
	
lp:	mov dl,coa[si]
	mov al,cob[si]
	add dl,al
	add dl,bl
	
	mov bl,0	;bl为进位
	cmp dl,10
	jb s
	mov bl,1
	sub dl,10
s:	mov sum[di],dl
	
	dec si
	dec di
	loop lp
	
	mov sum[di],0
	add sum[di],bl
	ret	;———————
