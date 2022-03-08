/*
for(int i=0;i<len-1;i++){
	for(int j=0;j<len-1-i;j++){
		if(a[j]<a[j+1]) swap(a[j],a[j+1])
	}
}
*/
/*
	.syntax unified
	.cpu cortex-m4
	.thumb
.data
	arr1: .word 0x19, 0x34, 0x14, 0x32, 0x52, 0x23, 0x61, 0x29
	arr2: .word 0x18, 0x17, 0x33, 0x16, 0xFA, 0x20, 0x55, 0xAC

.text
	.global main

do_sort:
	//TODO
	movs r1, #0 //i
	movs r8, #31// *r8:len-1
outer:
	movs r2, 0 //j
	subs r9, r8, r1 //r9:len-1-i
	inner:
		add r3, r0, r2 //r3=&a[j]
		ldrb r5, [r3]  //

		add r4, r3, #4 //*r4=&a[j+4]
		ldrb r6, [r4]  //

		cmp r5, r6 //if a[j]>a[j+4]
		bgt swap_end //no swap
		swap:
		strb r5, [r4]
		strb r6, [r3]
		swap_end:
		adds r2, r2, #4 //*j+=4
		cmp r2, r9
		blt inner
	inner_end:
	adds r1, r1,#4 //*i+=4
	cmp r1, #31
	blt outer

	b return


return:
	bx lr

main:
	ldr r0, =arr1
	bl do_sort
	ldr r0, =arr2
	bl do_sort

L: b L
*/
