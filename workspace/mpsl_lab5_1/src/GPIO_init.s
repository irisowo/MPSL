	.syntax unified
	.cpu cortex-m4
	.thumb

.text
	.global max7219_init
	.global max7219_send
	.global GPIO_init

	.equ RCC_AHB2ENR,  0x4002104C

	//max7219
	.equ	DECODE_MODE,	0x19 //decode control
	.equ	INTENSITY,		0x1A //brightness
	.equ	SCAN_LIMIT,		0x1B //how many digits to display
	.equ	SHUT_DOWN,		0x1C
	.equ	DISPLAY_TEST,	0x1F

	//Din, CS, CLK offset
	.equ 	DIN,	0b100000 	//PA5(0x20)
	.equ	CS,		0b1000000	//PA6(0x40)
	.equ	CLK,	0b10000000	//PA7(0x80)

	.equ    GPIOA_BASE,   0x48000000
	.equ    BSRR_OFFSET,  0x18 //1
	.equ    BRR_OFFSET,   0x28 //0

GPIO_init:
	push {r0, r1, r2, lr}

    //RCC_AHB2ENR
	mov r0, 0b1
	ldr r1, =RCC_AHB2ENR
	str r0, [r1]

	//GPIOA_MODER: PA7 6 5: output
	ldr r0, =0b010101
	lsl r0, 10
	ldr r1, =GPIOA_BASE //GPIOA_MODER
	ldr r2, [r1]
	and r2, 0xFFFF03FF //clear 7 6 5
	orrs r2, r2, r0 //7 6 5  --> output
	str r2, [r1]

    //GPIOA_OTYPER
	add  r1, 0x4
	ldr  r2, [r1]
	and  r2, 0xFFFFFF1F //(...0001...)
	str  r2, [r1]

	//GPIOA_OSPEEDR
	mov r0, 0b101010
	lsl r0, 10   //PA7,6,5: fast speed
	add  r1, 0x4 //GPIOA_OSPEEDER
	ldr r2, [r1]
	and r2, 0xFFFF03FF
	orrs r2, r2, r0
	str r0, [r1]

	pop  {r0, r1, r2, pc}

max7219_init:
	push {r0, r1, lr}
	//---------------------FF:CODE B decode for digit 0-7
	ldr  r0, =DECODE_MODE
	ldr  r1, =0xFF
	bl   max7219_send
	//---------------------0:normal operation
	ldr  r0, =DISPLAY_TEST
	ldr  r1, =0x0
	bl   max7219_send
	//---------------------21/32 (brightness)
	ldr  r0, =INTENSITY
	ldr  r1, =0xA
	bl   max7219_send
    //---------------------light up digit 0-7
	ldr  r0, =SCAN_LIMIT
	ldr  r1, =0x7
	bl   max7219_send
    //---------------------normal operation
	ldr  r0, =SHUT_DOWN
	ldr  r1, =0x1
	bl   max7219_send
    //---------------------
	pop  {r0, r1, pc}

max7219_send: //parameter:(r0, r1) is (addr,data)
	push {r0, r1, r2, r3, r4, r5, r6, r7, r8, lr}
	//set r0=DIN
	lsl	r0, 8  // D15-D8=addr
	add r0, r1 // D7-D0=data
	ldr  r1, =GPIOA_BASE
	ldr  r2, =CS
	ldr  r3, =DIN
	ldr  r4, =CLK
	ldr  r5, =BSRR_OFFSET //1
	ldr  r6, =BRR_OFFSET  //0
	ldr  r7, =0x0F // 第r7位, count from 15 to 0

    send_loop:
	mov  r8, #0x1
	lsl  r8, r7 //r8:ith-bit
	//------CLK->0-----------------------
	str  r4, [r1, r6]
	tst  r0, r8 //ANDS but just update flags
	it eq
	streq r3, [r1, r6] //DIN -> 0
	it ne
	strne r3, [r1,r5] //DIN -> 1
	str  r4, [r1, r5]
	//-------CLK->1----------------------
	subs r7, #0x1
	bge  send_loop //if(r7>=0) r7--
	str  r2, [r1, r6] // CS -> 0
	str  r2, [r1, r5] // CS -> 1
	pop  {r0, r1, r2, r3, r4, r5, r6, r7, r8, pc}

.end
