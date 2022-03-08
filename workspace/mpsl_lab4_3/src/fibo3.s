/*
	.syntax unified
	.cpu cortex-m4
	.thumb

.data
	ans: .asciz "0112358132134558914423337761098715972584418167651094617711286574636875025121393196418317811514229832040134626921783093524578570288792274651493035224157817390881696324598699999999"
	len: .byte 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x4, 0x4, 0x4, 0x5, 0x5, 0x5, 0x5, 0x5, 0x6, 0x6, 0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x7, 0x7, 0x8, 0x8, 0x8, 0x8, 0x8

.text
	//.global main
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

    //GPIOC for button
    .equ GPIOC_MODER  , 0x48000800
    .equ GPIOC_OTYPER ,	0x48000804
    .equ GPIOC_OSPEEDR,	0x48000808
    .equ GPIOC_PUPDR  ,	0x4800080c
    .equ GPIOC_IDR    , 0x48000810

    //other
    .equ one_sec, 4000000


main:
    //initialize button
	mov  r8, 0x0  //long press
	mov  r11, 0x1
	mov  r12, 0x1

	bl   gpio_init
	bl   max7219_init

display_ans:
	ldr  r2, =len
	ldr  r3, =ans
	b    display_init

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

	add  r10, r1, 0x10
	bx   lr

display_init:
	mov  r0, 0x8
	mov  r1, 0x0F
	bl   MAX7219Send
    init_7to0:
	cmp  r0, 0x0
	beq  init_end
	subs r0, 0x1
	bl   MAX7219Send
	b init_7to0
    init_end:
    ldrb r0, [r2]

display_byte:
    //read data(r1) and send -------------------------------------
	ldrb r1, [r3]
	sub  r1, 0x30 // char to digit
	bl   MAX7219Send
	subs r0, 0x1  //(r0最大位數讀到最低位)
	add  r3, 0x1
	bne  display_byte
display_next_byte:
    // checkpoint: if not len[39], i++----------------------------------------------
	ldr  r0, =len
	add  r0, 40  // r0=&len[39]
	cmp  r2, r0  // r2=&len[i]
	it   eq      // if(len[i]==len[39]) reset
	bleq reset
	add  r2, 0x1 // else i++
	// initialize press button and press---------------------------------------------
	mov  r8, 0x0
	mov  r11, 0x1
	mov  r12, 0x1
	//bl   check_button_init
	bl check_button
	//-------------------------------------------------------------------------------
	b    display_init
reset:
	ldr  r2, =len
	ldr  r3, =ans
	sub  r2, 0x1
	bx   lr

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
	movs r0, r0
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
	cmp  r8, 31 // r8 detect long press
	bge  main
	mov  r8, 0
	cmp  r1, r11
	mov  r11, r1
	beq  same_press
	subs r0, 8
	b    button_delay
check_button_increment:
	add  r8, 1
	cmp  r1, r11
	mov  r11, r1
	beq  same_press
	subs r0, 8
	b    button_delay
same_press:
	subs r1, r11, r12
	cmp  r1, 1
	mov  r12, r11
	beq  button_end
	subs r0, 8
	b    button_delay
button_end:
	bx   lr


check_button: //check every cycle, and accumulate 1
    ldr r5, [r10] //fetch the data from button
    lsr r5, r5, #13
    ands r5, r5, 0x1 //filter the signal
    it eq
    addseq r12, r12 ,#1 //accumulate until the threshol

    cmp r5, #1 //not stable, go back to accumulate again
    it eq
    moveq r12, #0

	ldr r11, =ans_digit

    push {r10}
    ldr r10, =one_sec //trial and error
    cmp r12, r10 //threshold achieved BREAKDOWN!, r6 flag rises use 1 for slo mo debug
	it eq
	movseq r6, #1
    pop {r10}

    push {r10}
    ldr r10, =one_sec
    cmp r12, r10
    it eq
    movseq r6, #2
    pop {r10}

    cmp r6, #1
    it eq
    ldrbeq r11, [r11,r4] //get current fibonacci digit this r0 will decrease

    cmp r6, #1
    it eq
    addeq r4, r4, 0x1//go to next fibonacci digit

    cmp r6, #1
    it eq
    addeq r7, r7, r11 //move to the start of next fibonacci digit, by increment the digit of current fibonacci number

    cmp r6, #2
    it eq
    moveq r7, #0 //move to the start of next fibonacci digit, by increment the digit of current fibonacci number

    cmp r6, #2
    it eq
    moveq r4, #0//case 2 reset all fibonacci pointers

    cmp r4, #40
    it ge
    movsge r4, #40

    cmp r4, #40
    it ge
    movsge r7, #170 //for -1

	mov r6, #0

    b check_end

