	.syntax unified
	.cpu cortex-m4
	.thumb

.data
	leds: .byte 0
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
	.equ GPIOC_IDR    , 0x48000810


main:
	bl   gpio_init
	mov  r9, 0b00001100 		//r9 on/off leds
	ldrb r8, =leds
	strb r9, [r8]
	mov  r8,  0b0				//r8 left/right
	mov  r7,  0b0 				//r7 run/stop
	mov  r11, 0b1
	mov  r12, 0b1
	b    loop

gpio_init:
	movs r0, 0x6				//BC
	ldr  r1, =RCC_AHB2ENR
	str  r0, [r1]

	movs r0, 0x1540 			//0101010(pb3-6) 000000
	ldr  r1, =GPIOB_MODER
	ldr  r2, [r1]
	and  r2, 0xFFFFC03F     	//clear pb3-6 00000000111111
	orrs r2, r0
	str  r2, [r1]

	ldr  r1, =GPIOC_MODER
	ldr  r0, [r1]
	ldr  r2, =0xF3FFFFFF
	and  r0, r2
	str  r0, [r1]

	movs r0, 0x2A80 			//change to high speed
	ldr  r1, =GPIOB_OSPEEDR
	strh r0, [r1]

	ldr  r0,  =GPIOB_ODR		//r0 : leds' output value address
	ldr  r10, =GPIOC_IDR		//r10 : button
	bx   lr

loop://亮燈-->delay-->button
	bl   display_led

	ldr  r5, =2000000
	movs r5, r5
	bl   delay

	cmp r7, 0b1					//if r7==1(stop), stop
	beq loop

	cmp  r8, 0b0 				// if(r8=0) shift left
	it   eq
	lsleq  r9, r9, 0b1

	cmp  r8, 0b1				// if(r8=1) shift right
	it   eq
	lsreq  r9, r9, 0b1

	cmp  r9, 0b11000000  		// change to going right
	it   eq
	moveq  r8, 0b1

	cmp  r9, 0b00001100  		// change to going left
	it   eq
	moveq  r8, 0b0

	b    loop

display_led:
	mvn  r5, r9  				// r5 = r9取反
	//mov  r5, r9
	strh r5, [r0]
	bx   lr

//在delay期間讀button--------------------------------------------

//每次跑都會check_button
delay:
	beq  delay_end
	ldr  r1, =0xFFFF // 2^15/4
	ands r1, r5, r1
	beq  check_button
	subs r5, 4
	b    delay
//第一次r1-->r11
check_button:
	ldrh r1, [r10]			// r10 input data reg
	lsr  r1, 13				// p13
	and  r1, 0x1			// filter the signal
	cmp  r1, r11
	mov  r11, r1
	beq  button_confirmed
	//subs r5, 4
	b    delay
//第二次r12
button_confirmed:
	subs r1, r11, r12
	cmp  r1, 1
	mov  r12, r11
	beq  switch
	subs r5, 4
	b    delay

switch:
	eor  r7, 0b1  		// r7:run/stop取反
	b    delay

delay_end:
	bx   lr
