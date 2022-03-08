	.syntax unified
	.cpu cortex-m4
	.thumb

.data
	password: .byte 0b1111
	.align

.text
	.global main
	.equ RCC_AHB2ENR  , 0x4002104C
	.equ GPIOB_MODER  , 0x48000400
	.equ GPIOB_OTYPER , 0x48000404
	.equ GPIOB_OSPEEDR, 0x48000408
	.equ GPIOB_PUPDR  , 0x4800040C
	.equ GPIOB_ODR    , 0x48000414
	.equ GPIOC_MODER  , 0x48000800
	.equ GPIOC_PUPDR  , 0x4800080C
	.equ GPIOC_IDR    , 0x48000810

main:
	b    gpio_init

gpio_init:
	mov  r0, 0x6
	ldr  r1, =RCC_AHB2ENR //enable gpio port BC
	str  r0, [r1]

	mov  r0, 0x1540 			//enable moder3~moder6
	ldr  r1, =GPIOB_MODER
	ldr  r2, [r1]
	and  r2, 0xFFFFC03F 		//clear pb3~pb6
	orr  r2, r0
	str  r2, [r1]

	mov r0, 0x00002A80		//set high speed , defulat value is 0x00000000 low speed
	ldr r1, =GPIOB_OSPEEDR
	str r0, [r1]

	ldr  r1, =GPIOC_MODER
	ldr  r0, [r1]
	ldr  r2, =0xF3FFFF00 	//clear pc13 to zero
	and  r0, r2
	str  r0, [r1]

	ldr  r1, =GPIOC_PUPDR
	ldr  r0, [r1]
	ldr  r2, =0b01010101   //pull-up
	and  r0, 0xFFFFFF00
	orr  r0, r2
	str  r0, [r1]

	ldr  r10, =GPIOB_ODR // leds : r10
	ldr  r11, =GPIOC_IDR // button : r11
	ldr  r12, =GPIOC_IDR // switch : r12

	mov  r0, 0b11111111
	strh r0, [r10]		//先關燈

	b    poll_button_init

poll_button_init:
	mov  r0, 0b11111111
	strh r0, [r10]		//先關燈

	mov  r8, 1
	mov  r9, 1
	movs r0, 0
poll_button://delay
	beq  button_restart
	ldr  r1, =0x1FFFF
	ands r1, r0, r1
	beq  button_check
	subs r0, 8
	b    poll_button

button_check:
	ldrh r1, [r11]	//r1=button read from :r11
	lsr  r1, 13		//p13
	and  r1, 0b1
	cmp  r1, r8
	mov  r8, r1		//(state:r8,r9)更新r8
	beq  button_confirm
	subs r0, 8
	b    poll_button

button_confirm:
	sub  r1, r8, r9
	cmp  r1, 1
	mov  r9, r8
	beq  read_switch
	subs r0, 8
	b    poll_button

button_restart:
	ldr  r0, =4000000
	movs r0, r0
	b    poll_button

read_switch:
	ldrh r1, [r12]      //r12:GPIOC_IDR
	mov  r5, r1         // debug
	and  r1, 0b1111		// mask
	eor  r1, 0b1111		// 取反
	ldr  r0, =password  //read pswd
	ldrb r0, [r0]

	cmp  r0, r1
	mov  r1, 0b01111000
	mov  r2, 0b0			//count

	beq  blink_3_times
	b    blink_1_times

blink_3_times:
	eor  r1, 0b11111111
	strh r1, [r10]     //r10:GPIOB_ODR
	add  r2, 0b1
	cmp  r2, 7
	beq  poll_button_init

	ldr  r0, =2000000
	blink_3_delay:
	beq  blink_3_times
	subs r0, 4
	b    blink_3_delay


blink_1_times:
	eor  r1, 0b11111111
	strh r1, [r10]
	add  r2, 0b1
	cmp  r2, 3
	beq  poll_button_init

	ldr  r0, =2000000
	blink_1_delay:
	beq  blink_1_times
	subs r0, 4
	b    blink_1_delay
