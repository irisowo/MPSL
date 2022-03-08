#include <stdint.h>
#include "stm32l476xx.h"
#include "core_cm4.h"


#define BIT_MASK(bit)				(1 << (bit))
#define SET_BIT(value,bit)			((value) |= BIT_MASK(bit))
#define CLEAR_BIT(value,bit)		    ((value) &= ~BIT_MASK(bit))
#define TEST_BIT(value,bit)			(((value) & BIT_MASK(bit)) ? 1 : 0)
#define SET_BITS(value, mask, newvalue)		(value = (value & ~mask) | (newvalue & mask))

extern void max7219_init();
extern void max7219_send(unsigned char address, unsigned char data);
extern void max7219_GPIO_init();

int ALARM = 0;
int timer_initial = 0;

int map[4][4] = {
		{1, 2, 3, 10},
		{4, 5, 6, 11},
		{7, 8, 9, 12},
		{15, 16, 14, 13}
};

void GPIO_init(void)
{
	// Enable AHB2 clock
	RCC->AHB2ENR |= 0b00000000000000000000000000000111; // A, B, C

	GPIOA->MODER &= (~(0b11 << 2));
	GPIOA->MODER |= ( 0b10 << 2);

	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL1; // PA0: TIM2_CH1: AF1
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL1_0; // PA0: TIM2_CH1: AF1

    GPIOC->MODER   &= 0b11110011111111111111111100000000; //use PC0123 for Yi(horizontal)
    GPIOC->MODER   |= 0b0000000000000000000000001010101;
    GPIOC->PUPDR   &= 0b11110011111111111111111100000000; //clear
    GPIOC->PUPDR   |= 0b00000000000000000000000001010101; //pup since we want 1 to be sent high level voltage
    GPIOC->OSPEEDR &= 0b11111111111111110000001100000000;
    GPIOC->OSPEEDR |= 0b00000000000000001010100001010101;
    GPIOC->ODR     |= 0b00000000000000000000000011110000;

    GPIOB->MODER   &= 0b11111111111111111111111100000000; //PB0123 for Xi output(straight)
    GPIOB->PUPDR   &= 0b11111111111111111111111100000000; //clear
    GPIOB->PUPDR   |= 0b00000000000000000000000010101010; //set pdown

	SET_BITS(GPIOB->AFR[1], 0b1111<<8, 0b0001<<8);// PB10 - alternate - buzzzzzer
}

void EXTI_config()
{
	RCC->APB2ENR |= 0b01; 						// enable SYSCFG

	SYSCFG->EXTICR[0] |= 0b0001000100010001;	// offset:0x08, configure EXTI0~EXTI3 to use PB0~PB3
	//SYSCFG->EXTICR[3] |= 0b00100000;    // offset:0x14, configure EXTI13 to use PC13
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC ;

	// set GPIO as rising trigger
	EXTI->RTSR1 |= 0b1111;
	EXTI->RTSR1 |= EXTI_RTSR1_RT13;

	// Enable GPIO interrupt : EXTI0~EXTI3
	EXTI->IMR1 |= 0b1111;
	EXTI->IMR1 |= EXTI_IMR1_IM13;

}

void NVIC_config()
{
	NVIC->ISER[0] |= 0b1111<<6; // interrupts 6~9 - EXTI0~EXTI3
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	NVIC_SetPriority(EXTI15_10_IRQn, 0);
}

void Timer_init(TIM_TypeDef *timer)
{
 RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
 //enable auto-reload preload
 timer->CR1 |= TIM_CR1_ARPE;
 //Freq: HCLK / (prescaler + 1) / arr
 timer->PSC = (uint32_t)399;
 timer->ARR = (uint32_t) 99;
 // Capture/Compare 2 output enable(PA1-TIM2-ch2)
 timer->CCER |= TIM_CCER_CC2E;
 // [6:4] 0111: PWM(when upcounting and TIMx_CNT<TIMx_CCR, active)
 timer->CCMR1 |= ( (TIM_CCMR1_OC2M & 0xFFFFFF8F) | (0b01110000) );
 timer->CCR2 = 10;
 // reinit counter
 timer->EGR = TIM_EGR_UG;
 //preload enable
 timer->CCMR1 |= TIM_CCMR1_OC2PE;

}

int keypad(int triggered)
{
	int output = 0;

	for(int i = 0; i < 4; i++)
		GPIOC->BRR = (0b1 << (i));

	for(int col = 0; col < 4; col++){
		GPIOC->BSRR = (0b1 << (col));
		int found = (GPIOB->IDR >> (0+triggered)) & 0b1;
		if(found == 0b1){
			output = map[col][triggered];
			break;
		}
		else{
			output = 0;
		}
		GPIOC->BRR = (0b1 << (col));
	}

	for(int i = 0; i < 4; i++)
			GPIOC->BSRR = (0b1 << (i));

	return output;
}

void show_time(int time){
	if(time>=10){
		max7219_send(2,time/10);
		max7219_send(1,time%10);
	}
	else{
		max7219_send(2,0xF);
		max7219_send(1,time);
	}
}

void EXTI0_IRQHandler()
{
	if((EXTI->PR1 & 0b0001) != 0)
	{
		EXTI->PR1 = 1 << 0;
		if(timer_initial == 0){
			timer_initial = keypad(0);
			show_time(timer_initial);
		}
		//EXTI->PR1 = 1 << 0;
	}
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
}

void EXTI1_IRQHandler()
{
	if((EXTI->PR1 & 0b0010) != 0)
	{
		EXTI->PR1 = 1 << 1;
		if(timer_initial == 0){
			timer_initial = keypad(1);
			if(timer_initial==16){
				ALARM=1;
				TIM2->CR1 |= TIM_CR1_CEN;
				timer_initial=0;
				show_time(0);
			}
			else
				show_time(timer_initial);
		}
	}
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
}

void EXTI2_IRQHandler()
{
	if((EXTI->PR1 & 0b0100) != 0)
	{
		EXTI->PR1 = 1 << 2;
		if(timer_initial == 0){
			timer_initial = keypad(2);
			show_time(timer_initial);
		}
	}
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
}

void EXTI3_IRQHandler()
{
	if((EXTI->PR1 & 0b1000 ) != 0)
	{
		EXTI->PR1 = 1 << 3;
		if(timer_initial == 0){
			timer_initial = keypad(3);
			show_time(timer_initial);
		}

	}
	NVIC_ClearPendingIRQ(EXTI3_IRQn);
}


int press_button (void){
	static int debounce = 0;
	if( (GPIOC->IDR & 0b0010000000000000) == 0){ // pressed
	    debounce = debounce >= 1 ? 1 : debounce+1 ;
	    return 1;
	}
	else if( debounce >= 1 ){
	    debounce = 0;
	    return 1;
	}
	return 0;
}

void EXTI15_10_IRQHandler()
{
	EXTI->PR1 |= EXTI_PR1_PIF13;
	//while (1){
		if (ALARM == 1 && press_button()){
			TIM2->CR1 &= ~TIM_CR1_CEN;
			//break;
		}
	//}
}


void SysTick_Handler()
{
	if(timer_initial == 0){
		//if(press_button()) TIM2->CR1 &= ~TIM_CR1_CEN;
		return;
	}
	else{
		if(timer_initial == 1){
			ALARM = 1;
			TIM2->CR1 |= TIM_CR1_CEN;
			timer_initial --;
			show_time(timer_initial);
		}
		else{
			ALARM=0;
			if(timer_initial > 1) {
				timer_initial--;
			}
			show_time(timer_initial);

		}

	}
}

void display_clr(){
    for(int i = 0;i <= 8;i++){
        max7219_send(i,0xF);
    }
}


int main(void)
{
	GPIO_init();
	max7219_init();
	max7219_GPIO_init();
	display_clr();

	NVIC_config();
	EXTI_config();
	TIM_TypeDef *timer = TIM2;
	Timer_init(timer);

	GPIOC->BSRR |= 0b1111<<0;

	SysTick_Config(4000000);

	while(1);
}
