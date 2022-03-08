.syntax unified
.cpu cortex-m4
.thumb
.text
	.global main
	@.equ N, -1
	.equ N,  3
	@.equ N, 3
	@.equ N, 21
	@.equ N, 47
	@.equ N, 100
	@.equ N, 102
fib:
 	//check 0
 	cmp r0, #0
 	beq zero
 	//r1 :check <1 || >
 	subs r1, r0, #1
 	bmi out_of_range
 	subs r1, r1, #100
 	bpl out_of_range

 	//fib
 	mov r2, #1 //r2 : a(n-2)
 	mov r3, #1 //r3 : a(n-1)
 	mov r4, #1
 	subs r0, r0, #1 //if n=1, return
 	beq return
 	while:
 		subs r0, r0, #1 //if n=2 return
 		beq return
 		adds r4, r3, r2
 		bvs overflow
 		mov r2, r3
 		mov r3, r4
 		b while
zero:
	movs  r4, #0
	b return

out_of_range:
	movs  r4, #-1
	b return

overflow:
	mov r4, #-2
	b return

return:
	bx lr

main:
	movs R0, #N
	bl fib

L: b L
