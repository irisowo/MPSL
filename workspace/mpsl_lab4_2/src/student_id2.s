	.syntax unified
	.cpu cortex-m4
	.thumb

.data
	student_id: .byte 0xF, 0, 6, 1, 6, 0, 8, 6
	//~~~*\(>.<)/*~~~~

.text
	 .global main
	.equ	RCC_AHB2ENR,	0x4002104C
	.equ	GPIOA_MODER,	0x48000000
	.equ	GPIOA_OSPEEDER,	0x48000008
	.equ	GPIOA_PUPDR,	0x4800000C
	.equ	GPIOA_IDR,		0x48000010
	.equ	GPIOA_ODR,		0x48000014
	.equ	GPIOA_BSRR,		0x48000018 //1
	.equ	GPIOA_BRR,		0x48000028 //0

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

main:
    BL   GPIO_init
    BL   max7219_init
    //TODO: display your student id on 7-Seg LED
    BL	Display_student_ID
L:  B L

Display_student_ID:
	mov r0, 0x8 //r0:digit  ~~~*\(>.<)/*~~~~
	mov r2, 0x0 //r2:i=0
	ldr r3, =student_id
    display_loop:
	ldrb r1, [r3,r2]
	bl MAX7219Send   //r0:0xX7~0xX1, r1=student_id1[i]
	adds r2, r2, 1   //i++
	subs r0, r0, 0b1 //digit -1
	beq Display_student_ID
	bgt display_loop

GPIO_init:
	//TODO: Initialize three GPIO pins as output for max7219 DIN, CS and CLK
	//RCC_AHB2ENR: enable GPIOA
	mov r0, 0b1
	ldr r1, =RCC_AHB2ENR
	str r0, [r1]

	//GPIOA_MODER: PA7 6 5: output
	ldr r0, =0b010101
	lsl r0, 10
	ldr r1, =GPIOA_MODER
	ldr r2, [r1]
	and r2, 0xFFFF03FF //clear 7 6 5
	orrs r2, r2, r0 //7 6 5  --> output
	str r2, [r1]

	//GPIOA_OTYPER
	//GPIO_OSPEEDR
	mov r0, 0b101010
	lsl r0, 10  //PA7,6,5: high speed
	ldr r1, =GPIOA_OSPEEDER
	ldr r2, [r1]
	and r2, 0xFFFF03FF
	orrs r2, r2, r0
	str r0, [r1]

	BX LR


max7219_init://TODO: Initialize max7219 registers

	push {r0, r1, LR}
	//---------------------FF:CODE B decode for digit 0-7
	ldr r0, =DECODE
	ldr r1, =0xFF
	bl MAX7219Send
	//---------------------0:normal operation
	ldr r0, =DISPLAY_TEST
	ldr r1, =0x0
	bl MAX7219Send
	//---------------------21/32 (brightness)
	ldr r0, =INTENSITY
	ldr r1, =0xA
	bl MAX7219Send
	//---------------------light up digit 0-6
	ldr r0, =SCAN_LIMIT
	ldr r1, =0x7//+blank  ~~~*\(>.<)/*~~~~
	bl MAX7219Send
	//---------------------normal operation
	ldr r0, =SHUT_DOWN
	ldr r1, =0x1
	bl MAX7219Send
	//---------------------
	pop {r0, r1, PC}
	BX LR


MAX7219Send://parameter:(r0, r1) is (addr,data)
	push {r0, r1, r2, r3, r4, r5, r6, r7, LR}
	//set r0=din
	lsl	r0, 8  // D15-D8=addr
	add r0, r1 // D7-D0=data
	ldr r1, =DIN
	ldr r2, =CS
	ldr r3, =CLK
	ldr r4, =GPIOA_BSRR //-> 1
	ldr r5, =GPIOA_BRR  //-> 0
	ldr r6, =0xF // 第r6位, count from 15 to 0
	//b send_loop
    send_loop:
	mov r7, 1
	lsl r7, r6     //r7:ith-bit
	//------CLK->0-----------------------
	str r3, [r5]
	tst r0, r7    //ANDS but just update flags
	it eq
	streq r1, [r5] //din -> 0
	it ne
	strne r1, [r4] //din -> 1
	str r3, [r4]
	//-------CLK->1----------------------
	subs r6, 0x1
	bge send_loop  //if(r6>=0) r6--
	str r2, [r5]   //CS -> 0
	str r2, [r4]   //CS -> 1
	pop {r0, r1, r2, r3, r4, r5, r6, r7, PC}
	BX LR

