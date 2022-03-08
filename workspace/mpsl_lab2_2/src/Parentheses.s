/*
	.syntax unified
	.cpu cortex-m4
	.thumb

.data
	infix_expr: .asciz "{-99+ [ 10 + 2 -0] }"

	user_stack_bottom: .zero 128

.text
	.global main
	//move infix_expr here. Please refer to the question below.
	@infix_expr: .ascii "{-99+ [ 10 + 20-0] }"
main:
	//stack_init
	ldr sp, =user_stack_bottom
	add SP, #128
	//r0 = infix_addr
	LDR R0, =infix_expr
	BL pare_check
	cmp R0, -1
	it ne
	movne R0, #0
L: B L


pare_check: //TODO
	mov r1, #0 //ri:i
LOOP:
	//check '\0'
	ldrb r2, [r0,r1]
	cmp r2, #0
	beq pare_end
	//space
	cmp r2, #57
	itt ls       //unsigned<=
	addls r1,#1  //i++
	bls LOOP

	cmp r2, #91		//  [
	itttt eq
	moveq r3, r2 	//  |
	pusheq {r3}		//  |
	addeq r1,#1		//  |
	beq LOOP		    //  |

	cmp r2, #123	//  {
	ittt eq
	moveq r3, r2 	//  |
	pusheq {r3}	 	//  |
	beq  next	 	//  |

	cmp r2, #93		//  2-1
	ite eq			//  if(r2==']')
	popeq {r3}		//  	pop()
	bne continue 	//  else continue
					//
	cmp r3, #91		//  if( r2==']'	&& (r3=='[' )
	itt ne			//		if not match
	movne r0, #-1	//  		r0=-1
	bne pare_end	//  		end
	@cmp r3, #91
	itt eq			//  	if match
	addeq r1,#1 	//  		i+=1
	beq LOOP		//  		LOOP

continue:
	cmp r2, #125	// 2-2
	ite eq			// if(r2=='}')
	popeq {r3}		// 		pop()
	bne next		// else continue
					//
	cmp r3, #123	// if( r2=='}' && r3=='{' )
	itt ne			//		if not match
	movne r0, #-1	//  		r0=-1
	bne pare_end 	//  		end
	@cmp r3, #123
	itt eq			//  	if match
	addeq r1,#1 	//  		i+=1
	beq LOOP		//  		LOOP

next:
	add r1,#1
	b LOOP

pare_end:
	BX LR
*/
