	.syntax unified
	.cpu cortex-m4
	.thumb

.data
    result: .byte 0

.text
    .global main
    //result=0x00000001=16
    @.equ X, 0x55AA
    @.equ Y, 0xAA55

    //result=0x00000000=0
    @.equ X, 0xAA55
    @.equ Y, 0xAA55

    //result=0x00000005=5
    @.equ X, 0x1234
    @.equ Y, 0x5678

 	//result=0x00000006=6
    .equ X, 0xABCD
    .equ Y, 0xEFAB

hamm:
    //TODO
	eors r3, r0, r1
	mov r5,0  //count=0
	cmp r3, #0
	beq return

    while:
		and r4, r3, #1 //r4 = last bit 1 or not
		add r5, r5, r4
		lsr r3, r3, #1 //r3>>1
		cmp r3, #0
		bne while

	return:
		bx lr
main:
    ldr r0, =X //This code will cause assemble error. Why? And how to fix.
    ldr r1, =Y
    ldr r2, =result
    bl hamm
    strb r5, [r2]

L: b L
