#ifndef __MOTOR1_H
#define __MOTOR1_H

#include "stm32l476xx.h"
#include "core_cm4.h"
#include "lib.h"

#define IN1(a) 					\
if(a){ 							\
GPIOB->ODR |= (0b01<< 4);		\
}else{							\
GPIOB->ODR &= (0b0<< 4);		\
}

#define IN2(a) 					\
if(a){ 							\
GPIOB->ODR |= (0b01<< 5);		\
}else{							\
GPIOB->ODR &= (0b0<< 5);		\
}

#define IN3(a) 					\
if(a){ 							\
GPIOA->ODR |= (0b01<< 6);		\
}else{							\
GPIOA->ODR &= (0b0<< 6);		\
}

#define IN4(a) 					\
if(a){ 							\
GPIOA->ODR |= (0b01<< 7);		\
}else{							\
GPIOA->ODR &= (0b0<< 7);		\
}


void Motor_12_Config(void){  //定义初始化函数
	RCC->AHB2ENR = 0b011;//A B
	//IN1, IN2
	for(int i=4;i<=5;i++){
		MODER_SETTING(GPIOB, i, MODER_O);
		PUPDR_SETTING(GPIOB, i, PUPDR_PD);
		OSPEEDR_SETTING(GPIOB, i, OSPEEDR_HIGH );
		OTYPER_SETTING(GPIOB, i, OTYPER_PP);
	}
	//IN3, IN4
	for(int i=6;i<=7;i++){
		MODER_SETTING(GPIOA, i, MODER_O);
		PUPDR_SETTING(GPIOA, i, PUPDR_PD);
		OSPEEDR_SETTING(GPIOA, i, OSPEEDR_HIGH );
		OTYPER_SETTING(GPIOA, i, OTYPER_PP);
	}
}


void Motor_1_STOP(void){
	IN1(0);
	IN2(0);
}

void Motor_1_PRun(void){
	IN1(0);
	IN2(1);
}

void Motor_1_NRun(void){
	IN2(0);
	IN1(1);
}

void Motor_2_STOP(void){
	IN3(0);
	IN4(0);
}

void Motor_2_PRun(void){
	IN3(0);
	IN4(1);
}

void Motor_2_NRun(void){
	IN4(0);
	IN3(1);
}


#endif
