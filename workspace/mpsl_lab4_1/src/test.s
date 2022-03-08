/*
	.syntax unified
	.cpu cortex-m4
	.thumb
.data
	//arr: .byte 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	arr: .byte 0x1C, 0x0D, 0x15, 0x19, 0x1C, 0x0D, 0x15, 0x19
.text
	.global main
	//GPIO
	.equ	RCC_AHB2ENR,	0x4002104C
	.equ	GPIOA_MODER,	0x48000000
	.equ	GPIOA_OTYPER,	0x48000004
	.equ	GPIOA_OSPEEDER,	0x48000008
	.equ	GPIOA_PUPDR,	0x4800000C
	.equ	GPIOA_IDR,		0x48000010
	.equ	GPIOA_ODR,		0x48000014
	.equ	GPIOA_BSRR,		0x48000018 //set bit -> 1
	.equ	GPIOA_BRR,		0x48000028 //clear bit -> 0

	//Din, CS, CLK offset
	.equ 	DIN,	0b100000 	//PA5
	.equ	CS,		0b1000000	//PA6
	.equ	CLK,	0b10000000	//PA7

	//max7219
	.equ	DECODE,			0x19 //decode control
	.equ	INTENSITY,		0x1A //brightness
	.equ	SCAN_LIMIT,		0x1B //how many digits to display
	.equ	SHUT_DOWN,		0x1C
	.equ	DISPLAY_TEST,	0x1F

	//timer
	.equ	 one_sec,		3000000

main:
    BL   GPIO_init
    BL   max7219_init

DisplayDigit:
    mov r0, 0x1  //r0=addr
	mov r12, 0x0 //r12=i
	ldr r3, =arr //arr
    display_loop:
	ldrb r1, [r3,r12] //r1=data=arr[i]
	bl MAX7219Send    //send
	BL max7219_init   //light up the r12th bulb
	//-----------------delay-------------
	ldr r10, =one_sec
	bl delay
	//----------------turn off-------------
	mov r1, 0x0
	bl MAX7219Send
	//----------------if(i<8) i++ else i=0
	add r12, 1//i++
	add r0, 0x1
	cmp r12, 0x8
	blt display_loop
	b	DisplayDigit


GPIO_init://TODO: Initialize three GPIO pins as output for max7219 DIN, CS and CLK

	//RCC_AHB2ENR: enable GPIOA
	mov r0, 0b1
	ldr r1, =RCC_AHB2ENR
	str r0, [r1]

	//GPIOA_MODER: PA7,6,5: output
	ldr r0, =0b010101
	lsl r0, 10
	ldr r1, =GPIOA_MODER
	ldr r2, [r1]
	and r2, 0xFFFF03FF //clear 7 6 5 (0000-0011)
	orrs r2, r2, r0    //7 6 5  --> output
	str r2, [r1]

	//GPIOA_OTYPER
	//GPIO_OSPEEDR: high speed
	mov r0, 0b101010
	lsl r0, 10 //PA7,6,5 as high speed
	ldr r1, =GPIOA_OSPEEDER
	ldr r2, [r1]
	and r2, 0xFFFF03FF//clear 7 6 5 (0000-0011)
	orrs r2, r2, r0
	str r0, [r1]

	BX LR //back to loop

max7219_init://TODO: Initialize max7219 registers

	push {r0, r1, LR}
	//---------------------0:NO DECODE
	ldr r0, =DECODE
	ldr r1, =0x0
	bl MAX7219Send
	//---------------------0:normal operation
	ldr r0, =DISPLAY_TEST
	ldr r1, =0x0
	bl MAX7219Send
	//---------------------21/32 (brightness)
	ldr r0, =INTENSITY
	ldr r1, =0xA
	bl MAX7219Send
	//---------------------only light up digit 0
	ldr r0, =SCAN_LIMIT
	mov r1, r12
	bl MAX7219Send
	//---------------------normal operation
	ldr r0, =SHUT_DOWN
	ldr r1, =0x1
	bl MAX7219Send
	//---------------------
	pop {r0, r1, PC}
	BX LR

MAX7219Send:	//parameter:(r0, r1) is (addr,data)
	push {r0, r1, r2, r3, r4, r5, r6, r7, LR}
	//set r0=din
	lsl	r0, 8  // D15-D8=addr
	add r0, r1 // D7-D0=data
	ldr r1, =DIN
	ldr r2, =CS
	ldr r3, =CLK
	ldr r4, =GPIOA_BSRR //-> 1
	ldr r5, =GPIOA_BRR  //-> 0
	ldr r6, =0xF //sending nth bit
	//b send_loop
    send_loop://sending the nth bit of r0
	mov r7, 0b1
	lsl r7, r6
	//------CLK->0-----------------------
	str r3, [r5]
	tst r0, r7    //ANDS but just update flags
	it eq
	streq r1, [r5] //din -> 0, send clear bit
	it ne
	strne r1, [r4] //din -> 1
	str r3, [r4]
	//-------CLK->1-----------------------
	subs r6, 0b1
	bge send_loop
	str r2, [r5] //CS -> 0
	str r2, [r4] //CS -> 1
	pop {r0, r1, r2, r3, r4, r5, r6, r7, PC}
	BX LR

delay: //TODO: delay 1sec
	beq  delay_end
	subs r10, 0x4
	b    delay
    delay_end:
	bx   lr
*/
