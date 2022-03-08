	.syntax unified
	.cpu cortex-m4
	.thumb
.data
	result: .word 0
	max_size: .word 0
.text
	m: .word 0x4E
	n: .word 0x82
	@m: .word 0x27
	@n: .word 0x41
	.global main

main:
	ldr r0, #m
	ldr r1, #n
	@push {r0}
	@push {r1}
	movs r5, #1
	movs r6, #0
	BL GCD
	//store r5 to result[r2]
	ldr r2, =result
	str r5, [r2]
	//store r6 to max_size[r3]
	ldr r3, =max_size
	str r6, [r3]
L: b L

GCD:
	//push {lr}
    // r5 = GCD(r0,r1)
	cmp r0, #0
	beq if_a0_rb;
	cmp r1, #0
	beq if_b0_ra;

	ands r2,r0,1
	ands r3,r1,1
	adds r4,r2,r3
	beq  if_a0_b0

	cmp  r2,#0
	beq  if_a0
	cmp  r3,#0
	beq  if_b0

	b else

if_a0_rb:
	movs r5, r1
	bx lr
if_b0_ra:
	movs r5, r0
	bx lr
if_a0_b0:
	@push {r0}
	@push {r1}
	push {lr}
	adds r6, #4//@
	lsr r0,1  // a/2
	lsr r1,1  // b/2
	bl GCD
	lsl r5, 1 // 2*GCD(a/2,a/b)
	pop {lr}
	@pop {r1}
	@pop {r0}
	bx lr
if_a0:
	@push {r0}
	@push {r1}
	push {lr}
	adds r6, #4//@
	lsr r0,1
	bl GCD
	pop {lr}
	@pop {r1}
	@pop {r0}
	bx lr
if_b0:
	@push {r0}
	@push {r1}
	push {lr}
	adds r6, #4//@
	lsr r1, 1
	bl GCD
	pop {lr}
	@pop {r1}
	@pop {r0}
	bx lr

else: // return GCD(abs(a - b), min(a, b))
	@push {r0}
	@push {r1}
	push {lr}
	adds r6, #4 //@
	subs r2,r0,r1
	bpl no_abs
	subs r2, r1, r0
	movs r1, r0 //b換成a(min)
	movs r0, r2 //a換成b-a
no_abs:
	movs r0, r2 //a換成a-b,b比較小不變
	bl GCD
	pop {lr}
	@pop {r1}
	@pop {r0}
	BX LR
