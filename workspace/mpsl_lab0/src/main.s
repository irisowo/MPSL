    .syntax unified
    .cpu cortex-m4
    .thumb
.data
	X: .word 100
	Z: .word 100
	str: .asciz "Hello World!"
.text
	.global main
	.equ AA, 0x55
	//X: .word 100
main:
	ldr r1, =X  //r1=&X
	ldr r0, [r1] //r0=*r1=100
	movs r2, #AA //r2=85
	adds r2, r2, r0 //r2=185
	str r2, [r1] // 把r2寫到位址r1(0x20000000)
	ldr r1, =str //r1=儲存str的位址
	ldrh r2, [r1] //r2=*r1 /lleH=6c6c6548 -->1819043144

L:B L
