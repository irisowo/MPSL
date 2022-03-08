/*
	.syntax unified
	.cpu cortex-m4
	.thumb

.data

	ans: .asciz "0112358132134558914423337761098715972584418167651094617711286574636875025121393196418317811514229832040134626921783093524578570288792274651493035224157817390881696324598699999999"
	len: .byte 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x4, 0x4, 0x4, 0x5, 0x5, 0x5, 0x5, 0x5, 0x6, 0x6, 0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x7, 0x7, 0x8, 0x8, 0x8, 0x8, 0x8

.text
	 .global main
	.equ	 RCC_AHB2ENR,	0x4002104C
	.equ	 GPIOA_MODER,	0x48000000
	.equ	 GPIOA_OSPEEDER,	0x48000008
	.equ	 GPIOA_PUPDR,	0x4800000C
	.equ	 GPIOA_IDR,		0x48000010
	.equ	 GPIOA_ODR,		0x48000014
	.equ	 GPIOA_BSRR,		0x48000018 //1
	.equ	 GPIOA_BRR,		0x48000028 //0

	//Din, CS, CLK offset
	.equ 	DIN,	0b100000 	//PA5
	.equ 	CS,		0b1000000	//PA6
	.equ 	CLK,	0b10000000	//PA7

	//max7219
	.equ	 DECODE,			0x19 //decode control
	.equ	 INTENSITY,		0x1A //brightness
	.equ	 SCAN_LIMIT,		0x1B //how many digits to display
	.equ	 SHUT_DOWN,		0x1C
	.equ	 DISPLAY_TEST,	0x1F

    //GPIOC for button
    .equ GPIOC_MODER  , 0x48000800
    .equ GPIOC_OTYPER ,	0x48000804
    .equ GPIOC_OSPEEDR,	0x48000808
    .equ GPIOC_PUPDR  ,	0x4800080c
    .equ GPIOC_IDR    , 0x48000810

    //other
    .equ one_sec, 1000000


main:
    //initialize button
	mov  r8, 0x0  //long press
	mov  r11, 0x1
	mov  r12, 0x1
	bl   gpio_init
	bl   max7219_init
display_ans:
	bl    clear//clear
	b start



gpio_init:
	//RCC_AHB2ENR: enable GPIOA
	mov r0, 0b101 //A  and C
	ldr r1, =RCC_AHB2ENR
	str r0, [r1]

	//GPIOA_MODER: PA7 6 5: output
	ldr r0, =0b010101 //7,6,5 =010101
	lsl r0, 10
	ldr r1, =GPIOA_MODER
	ldr r2, [r1]
	and r2, 0xFFFF03FF //r2= FFFF00.00.00.11FF :clear 7 6 5
	orrs r2, r2, r0 //7 6 5  --> output
	str r2, [r1]

	//GPIOA_OTYPER: push-pull (reset state)

	//=GPIOA_OSPEEDER
	mov r0, 0b101010
	lsl r0, 10  //PA7,6,5: fast speed
	ldr r1, =GPIOA_OSPEEDER
	ldr r2, [r1]
	and r2, 0xFFFF03FF
	orrs r2, r2, r0
	str r0, [r1]

	//GPIOC_MODER PC13
	ldr  r1, =GPIOC_MODER
	ldr  r2, [r1]
	and  r2, 0xF03FFFFFF //(F-0011-FFFFF)
	str  r2, [r1]

    //GPIOC_IDR
	ldr  r10, =GPIOC_IDR
	bx   lr

clear:
    push {r0, r1, lr}
	mov  r0, 0x8 //addr
	mov  r1, 0x0 //data
	bl   MAX7219Send
    init_7to0:
	cmp  r0, 0
	blt  clear_end
	bl   MAX7219Send
	subs r0, 1
	b init_7to0
clear_end:
    pop {r0, r1, PC}
    bx lr


start:
    mov r2, 0b0 //r2:00001111 with len 5 means nbbbb
    mov r3, 0b0 //r3=len
short_press:
	// initialize press button and press---------------------------------------------
	mov  r8, 0x0
	mov  r11, 0x1
	mov  r12, 0x1
	bl   check_button_init
	//short_press store 0
	lsl r2, 1
	and r2, 0xFFFFFFFE
	add r3, 1
	//-------------------------------------------------------------------------------
    bl clear
after_detect:
   subs r3, 8
   bgt start
   add r3, 8//sub加回來
   mov r4, r3
display_code:
   mov r0, r4 //addr=len-1
   mov r5, r2
   lsl r5, 1 //111 with len 3 : shift2
   lsr r5, r4
   ands r5, 1
   beq send_n
   mov r1, 0x1F
   bl   MAX7219Send
   b continue
   send_n:
   mov r1, 0x15
   bl   MAX7219Send
   continue:
   subs r4, 1
   bge display_code
   b short_press


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
	lsl r7, r6     //r7:ith-bit=1, others=0
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

max7219_init://TODO: Initialize max7219 registers

	push {r0, r1, LR}
	//---------------------FF:CODE B decode for digit 0-7
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
	//---------------------light up digit 0-7
	ldr r0, =SCAN_LIMIT
	ldr r1, =0x7
	bl MAX7219Send
	//---------------------normal operation
	ldr r0, =SHUT_DOWN
	ldr r1, =0x1
	bl MAX7219Send
	//---------------------
	pop {r0, r1, PC}
	BX LR

check_button_init:
	ldr  r0, =one_sec
	movs r0, r0 //?
	b    button_delay
button_delay:
	beq  check_button_init
	ldr  r1, =0x1FFFF
	ands r1, r0
	beq  check_button //branch many times
	subs r0, 0x8
	b    button_delay
check_button://(r11,r12)
	// r10 gpio button input, read to r1
	ldrh r1, [r10]
	lsr  r1, 13
	ands r1, 0b1
	beq  check_button_increment
    //--------------------------------
	cmp  r8, 17 // r8 detect long press
	bge  record_b
	mov  r8, 0
	cmp  r1, r11
	mov  r11, r1
	beq  same_press
	subs r0, 0x8
	b    button_delay
check_button_increment:
	add  r8, 1
	cmp  r1, r11
	mov  r11, r1
	beq  same_press
	subs r0, 0x8
	b    button_delay
same_press:
	subs r1, r11, r12
	cmp  r1, 1
	mov  r12, r11
	beq  button_end
	subs r0,0x8
	b    button_delay
button_end:
	bx   lr

record_b:
    push { r0, r1, lr}
    lsl r2, 1
    and r2, 0xFFFFFFFFE //(1110)
    orr r2, 0x1
    add r3, 1
    b after_detect
    pop {r0, r1, PC}
