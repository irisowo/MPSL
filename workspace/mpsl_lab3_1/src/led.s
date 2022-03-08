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


main:
	bl   gpio_init
	mov  r9, 0b00001100
	mov  r8, 0b0
	ldrb r7, =leds
	strb r9, [r7]
	b    loop

gpio_init:
	movs r0, 0x2 			//pB
	ldr  r1, =RCC_AHB2ENR
	str  r0, [r1]

	movs r0, 0x1540 		//0101010(pb3-6) 000000
	ldr  r1, =GPIOB_MODER
	ldr  r2, [r1]
	and  r2, 0xFFFFC03F //clear pb3-6 00000000111111
	orrs r2, r0
	str  r2, [r1]

	movs r0,0x2A80 			//default 0, change to 10101010000000
	ldr  r1, =GPIOB_OSPEEDR
	strh r0, [r1]



	ldr  r0, =GPIOB_ODR	 	// ***r0 : led data output value address!
	bx   lr

loop:
	bl   display_led

	ldr  r5, =2000000		//delay 1s block
	movs r5, r5
	bl   delay

	cmp  r8, 0b0 			// if(r8=0) move left
	it   eq
	lsleq  r9, r9, 0b1

	cmp  r8, 0b1			// if(r8=1) move right
	it   eq
	lsreq  r9, r9, 0b1

	cmp  r9, 0b11000000 	// change r8 0L-->1R
	it   eq
	moveq  r8, 0b1

	cmp  r9, 0b00001100 	// change r8 1R-->0L
	it   eq
	moveq  r8, 0b0

	b    loop

display_led:
	mvn  r5, r9  			//r5 = r9取反
	//mov r5, r9
	strh r5, [r0] 			//r5=0011
	bx   lr

delay:
	beq  delay_end
	subs r5, 4
	b delay
delay_end:
	bx   lr
