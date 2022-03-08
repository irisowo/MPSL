	.syntax unified
	.cpu cortex-m4
	.thumb
.data
	result: .zero 8

.text
	.global main
	@.equ X, 0x12345678
	@.equ Y, 0xABCDEF00
	.equ X, 0xFFFFFFFF
	.equ Y, 0xFFFFFFFF
	.equ MASK, 0xFFFF


main:
	LDR R0, =X
	LDR R1, =Y
	LDR R2, =result
	LDR R3, =MASK
	BL kara_mul

L: B L

kara_mul:
	mov r4, r0
	mov r5, r0
	mov r6, r1
	mov r7, r1
	lsr r4, 16 		//r4:XL
	and r5, r3 		//r5:XR
	lsr r6, 16 		//r6:YL
	and r7, r3		//r7:YR

	mul r8, r4, r6		//r8:X​LY​L​
	add r9, r4, r5		//r9:(X​L​+X​R)
	add r10, r6, r7		//r10:(Y​L+​Y​R)​
	mul r11, r5, r7		//r11:XRYR

	//r9 = [ (X​L​+X​R)​(Y​L+​Y​R)​-(X​LY​​L​+X​RX​​R)​ ]
	mov r12, #0
	//r12r9 = (X​L​+X​R)​(Y​L+​Y​R)​
	umull r9, r12, r9, r10 //r12r9
	cmp r9, r8
	bgt continue1
	sub r12, #1
	continue1:
	subs r9, r9, r8

	cmp r9, r8
	bgt continue2
	sub r12, #1
	continue2:
	subs r9, r9,  r11


	mov r10, r9
	lsr r10, #16	//r10 = r9L

	lsl r12, #16
	add r10, r12

	and r9,  r3		//r9 = r9R
	lsl r9, #16

	 //r10r9 =  (c379aaa)(42d20800) | (fffffffe)(00000001)
	 adds r9, r9, r11 //r9=31-0
	 adc  r10, r10, r8 //r10=64-32

	 strd r9, r10, [r2]
	BX LR
