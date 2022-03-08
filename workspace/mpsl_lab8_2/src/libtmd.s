	.syntax unified
	.cpu cortex-m4
	.thumb

.text
	.global delay_ms
	.global fpu_enable

delay_ms:
	push {r0, r1, lr}
	ldr  r1, =4000
	muls r0, r1

delay_ms_loop:
	beq  delay_ms_end
	subs r0, 4
	b    delay_ms_loop

delay_ms_end:
	pop  {r0, r1, pc}

fpu_enable:
	push  {r0, r1, lr}
	ldr.w r0, =0xE000ED88
	ldr   r1, [r0]
	orr   r1, r1, #(0xF << 20)
	str   r1, [r0]
	dsb
	isb
	pop   {r0, r1, pc}
