/*
#include "stm32l476xx.h"

#define X0 11
#define X1 12
#define X2 13
#define X3 14
#define Y0 3
#define Y1 4
#define Y2 5
#define Y3 6
#define LED_PIN 0 // PA 0

unsigned int x_pin[4] = {X0, X1, X2, X3};
unsigned int y_pin[4] = {Y0, Y1, Y2, Y3};


void gpio_init();
int keypad_scan();
void led_init();
void timer_init();
void timer_start();

int main() {
	gpio_init();
	led_init();
	timer_init();
	timer_start();
	int prev_char = -1;
	int counter = 0;
	while(1) {
//		int input_char = 14;
		int input_char = keypad_scan();
		if (input_char == -1 || input_char == prev_char){
			prev_char = input_char;
			continue;
		}
		if (input_char == 14)
			counter = (counter + 5 > 90 ) ? 90 : counter + 5;
		else if (input_char == 15)
			counter = (counter - 5 < 10 ) ? 10 : counter - 5;

		TIM2->CCR1 = (uint32_t)counter;
		prev_char = input_char;
	}

	return 0;
}
// pb 12 13 14 15 (column) output
// pb 3 4  5 6 (row) input
void gpio_init() {
	// enable A and B
	RCC->AHB2ENR = 3;

	// B output
	int B_moder = 0;
	for(int i = 0; i < 4; i++) {
		B_moder = B_moder | (0x01 << (x_pin[i] * 2));
	}
	// B input
	for(int i = 0; i < 4; i++) {
		B_moder = B_moder | (0x00 << (y_pin[i] * 2));
	}
	GPIOB->MODER = B_moder;

	// B speed
	// x speed
	int B_speed = 0;
	for(int i = 0; i < 4; i++) {
		B_speed = B_speed | (0x01 << (x_pin[i] * 2));
	}

	// y speed
	for(int i = 0; i < 4; i++) {
		B_speed = B_speed | (0x01 << (y_pin[i] * 2));
	}
	GPIOB->OSPEEDR = B_speed;

	// PUPDR
	int B_PUPDR = 0;
	for(int i = 0; i < 4; i++) {
		B_PUPDR = B_PUPDR | (0x01 << (x_pin[i] * 2));
	}
	for(int i = 0; i < 4; i++) {
		B_PUPDR = B_PUPDR | (0x11 << (y_pin[i] * 2));
	}
	GPIOB->PUPDR = B_PUPDR;

	// ODR
	int B_ODR = 0;
	for(int i = 0; i < 4; i++) {
		B_ODR = B_ODR | (0x01 << x_pin[i]);
	}
	GPIOB->ODR = B_ODR;
	return;
}


int keypad_scan() {
	// X0
	GPIOB->ODR = 0xFFFFFFFF;
	GPIOB->ODR = ~(0x01 << X0);
	if((GPIOB->IDR & (0x01 << Y0))== 0)return 13; // 13
	if((GPIOB->IDR & (0x01 << Y1))== 0)return 14; // 14
	if((GPIOB->IDR & (0x01 << Y2))== 0)return 0; // 0
	if((GPIOB->IDR & (0x01 << Y3))== 0)return 15; // 15
	// X1
	GPIOB->ODR = ~(0x01 << X1);
	if((GPIOB->IDR & (0x01 << Y0))== 0)return 12; // 12
	if((GPIOB->IDR & (0x01 << Y1))== 0)return 9; // 9
	if((GPIOB->IDR & (0x01 << Y2))== 0)return 8; // 8
	if((GPIOB->IDR & (0x01 << Y3))== 0)return 7; // 7
	// X2
	GPIOB->ODR = ~(0x01 << X2);
	if((GPIOB->IDR & (0x01 << Y0))== 0)return 11; // 11
	if((GPIOB->IDR & (0x01 << Y1))== 0)return 6; // 6
	if((GPIOB->IDR & (0x01 << Y2))== 0)return 5; // 5
	if((GPIOB->IDR & (0x01 << Y3))== 0)return 4; // 4
	// X3
	GPIOB->ODR = ~(0x01 << X3);
	if((GPIOB->IDR & (0x01 << Y0))== 0)return 10; // 10
	if((GPIOB->IDR & (0x01 << Y1))== 0)return 3; // 3
	if((GPIOB->IDR & (0x01 << Y2))== 0)return 2; // 2
	if((GPIOB->IDR & (0x01 << Y3))== 0)return 1; // 1

	return -1;
}

#define MODER_AF 0b10

void led_init(){
	RCC->AHB2ENR |=	RCC_AHB2ENR_GPIOAEN;
	GPIOA->MODER &= (~(0b11 << LED_PIN * 2));
	GPIOA->MODER |= (MODER_AF << LED_PIN * 2);

	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL0; // PA0: TIM2_CH1: AF1
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL0_0; // PA0: TIM2_CH1: AF1
	return;
}

void timer_init() {
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	TIM2->CR1 &= ~TIM_CR1_CMS; // Edge-Aligned
	// Channel 1
	//TIM2->PSC = (uint32_t) (DO-1) ;
	TIM2->ARR = (uint32_t) (100-1);
	TIM2->CCR1 = (uint32_t) (50-1); // Channel 1

	TIM2->CCMR1 |= ( TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 ); // 0110
	TIM2->CR1 |= TIM_CR1_ARPE;
	TIM2->EGR = TIM_EGR_UG;
	TIM2->CCER = TIM_CCER_CC1E; // TURN ON // Channel 1
	return;
}

void timer_start() {
	TIM2->CR1 |= TIM_CR1_CEN;
	return;
}
*/
