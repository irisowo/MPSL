	.syntax unified
	.cpu cortex-m4
	.thumb

.data
	user_stack_bottom: .zero 128
	expr_result: .word 0


.text
	.global main
	postfix_expr: .asciz "2 -7 - - +"

// R0: address
// R1: ptr
// R2: value
// R3: pop, push
// R4: pop
// R5: ten
main:
	ldr	SP, =user_stack_bottom
	add	SP, #128

	ldr	R0, =postfix_expr
	mov	R1, #0

LOOP:
	ldrb	R2, [R0, R1]

	// \0
	cmp R2, #0
	it	eq
	bleq	END

	// space
	cmp R2, #32
	itt	eq
	addeq	R1, #1
	beq	LOOP

	// add
	cmp	R2, #43
	it	eq
	beq	ADD

	// sub
	cmp R2, #45
	it	eq
	beq	SUB

	b	NUM

ADD:
	pop	{R3, R4}
	bl check
	add	R3, R4
	push	{R3}
	add	R1, #1
	b	LOOP

SUB:
	add	R1, #1
	ldrb	R2, [R0, R1]
	cmp	R2, #32
	it	eq
	beq	SUB_OP
	cmp R2, #0
	it	eq
	beq	SUB_OP
	it	ne
	bne	SUB_NUM

SUB_NUM:
	bl	atoi
	sub	R12, #1
	mvn	R12, R12
	push	{R12}
	b	LOOP

SUB_OP:
	pop	{R3, R4}
	bl check
	sub	R4, R3
	push	{R4}
	b	LOOP

NUM:
	bl	atoi
	push	{R12}
	b	LOOP

END:
	pop	{R2}
	ldr	R0, =expr_result
	str	R2, [R0]
Error:
	mov R2, -1
	ldr	R0, =expr_result
	str	R2, [R0]
L:
	nop
	b	L

// R12: return value
// R5:  ten
atoi:
	mov	R12, #0
	mov	R5, #10
	ATOI_LOOP:
		ldrb	R2, [R0, R1]
		add	R1, #1
		// break
		cmp	R2, #32
		it	eq
		bxeq	LR
		cmp	R2, #0
		it	eq
		bxeq	LR

		mul	R12, R5
		sub	R2, #48
		add R12, R2
		b ATOI_LOOP
	bx	LR

check:
    cmp r3, 48
    blt Error
    cmp r3, 57
    bgt Error
    cmp r4, 48
    blt Error
    cmp r4, 57
    bgt Error
    bx lr
