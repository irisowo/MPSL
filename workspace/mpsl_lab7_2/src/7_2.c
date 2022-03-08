#include <stdint.h>
#include "stm32l476xx.h"
#include "core_cm4.h"


#define BIT_MASK(bit)				(1 << (bit))
#define SET_BIT(value,bit)			((value) |= BIT_MASK(bit))
#define CLEAR_BIT(value,bit)		((value) &= ~BIT_MASK(bit))
#define TEST_BIT(value,bit)			(((value) & BIT_MASK(bit)) ? 1 : 0)
#define SET_BITS(value, mask, newvalue)		(value = (value & ~mask) | (newvalue & mask))

int blink_cnt = 0;

int map[4][4] = {
		{1, 2, 3, 10},
		{4, 5, 6, 11},
		{7, 8, 9, 12},
		{15, 0, 14, 13}
};

void GPIO_init(void)
{
	// Enable AHB2 clock
	RCC->AHB2ENR |= 0b00000000000000000000000000000111; // A, B, C

	// PA5 - output, PA0 - alternative
	GPIOA->MODER &= 	0b11111111111111111111011111111110;
	SET_BITS(GPIOA->AFR[0], 0b1111, 0b0010); // PA0 : 的TIM12345:AF2
	GPIOA->OSPEEDR &=	0b11111111111111111111101111111110;

    GPIOC->MODER   &= 0b11111111111111111111111100000000; //use PC0123 for Yi(horizontal)
    GPIOC->MODER   |= 0b00000000000000000000000001010101;
    GPIOC->PUPDR   &= 0b11111111111111111111111100000000; //clear
    GPIOC->PUPDR   |= 0b00000000000000000000000001010101; //pup since we want 1 to be sent high level voltage
    GPIOC->OSPEEDR &= 0b11111111111111111111111100000000;
    GPIOC->OSPEEDR |= 0b00000000000000000000000001010101;
    GPIOC->ODR     |= 0b00000000000000000000000011110000;

    GPIOB->MODER   &= 0b11111111111111111111111100000000; //PB0123 for Xi output(straight)
    GPIOB->PUPDR   &= 0b11111111111111111111111100000000; //clear
    GPIOB->PUPDR   |= 0b00000000000000000000000010101010; //set pdown

	SET_BITS(GPIOB->AFR[1], 0b1111<<8, 0b0001<<8);// PB10 - alternate - buzzzzzer
}

void EXTI_config()
{
	// EXTI0~EXTI3 - PB0~PB3
	EXTI->IMR1 |= 0b1111;
	EXTI->RTSR1 |= 0b1111;

	RCC->APB2ENR |= 0b1; 						// enable SYSCFG
	SYSCFG->EXTICR[0] |= 0b0001000100010001;	// configure EXTI0~EXTI3 to use PB0~PB3
}

void NVIC_config()
{
	NVIC->ISER[0] |= 0b1111<<6; // interrupts 6~9 - EXTI0~EXTI3
}

int keypad(int triggered)
{
	int output = 0;

	for(int i = 0; i < 4; i++)
		GPIOC->BRR = (0b1 << (i));

	for(int col = 0; col < 4; col++)
	{
		GPIOC->BSRR = (0b1 << (col));

		int found = (GPIOB->IDR >> (0+triggered)) & 0b1;
		if(found == 0b1)
		{
			output = map[col][triggered];
			break;
		}
		else
			output = 0;

		GPIOC->BRR = (0b1 << (col));
	}

	for(int i = 0; i < 4; i++)
			GPIOC->BSRR = (0b1 << (i));

	return 2*output;
}

void EXTI0_IRQHandler()
{
	if((EXTI->PR1 & 0b0001) != 0)
	{
		EXTI->PR1 = 1 << 0;
		if(blink_cnt == 0)
			blink_cnt = keypad(0);
	}
}

void EXTI1_IRQHandler()
{
	if((EXTI->PR1 & 0b0010) != 0)
	{
		EXTI->PR1 = 1 << 1;
		if(blink_cnt == 0)
			blink_cnt = keypad(1);
	}
}

void EXTI2_IRQHandler()
{
	if((EXTI->PR1 & 0b0100) != 0)
	{
		EXTI->PR1 = 1 << 2;
		if(blink_cnt == 0)
			blink_cnt = keypad(2);
	}
}

void EXTI3_IRQHandler()
{
	if((EXTI->PR1 & 0b1000) != 0)
	{
		EXTI->PR1 = 1 << 3;
		if(blink_cnt == 0)
			blink_cnt = keypad(3);
	}
}

void SysTick_Handler()
{
	if(blink_cnt == 0)
	{
		GPIOA->BRR |= 0b1<<5;
		return;
	}
	else
	{
		if(blink_cnt%2 == 1)
		{
			GPIOA->BSRR |= 0b1<<5;
			if(blink_cnt > 0)
				blink_cnt--;
		}
		else
		{
			GPIOA->BRR |= 0b1<<5;
			if(blink_cnt > 0)
				blink_cnt--;
		}
	}

}



int main(void)
{
	GPIO_init();
	NVIC_config();
	EXTI_config();

	GPIOC->BSRR |= 0b1111<<0;

	SysTick_Config(2000000); // every half second assuming SYSCLK is 4MHz

	while(1);
}
