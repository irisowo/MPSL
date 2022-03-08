#include <stdint.h>
#include "stm32l476xx.h"
#include "core_cm4.h"

#define BIT_MASK(bit)				(1 << (bit))
#define SET_BIT(value,bit)			((value) |= BIT_MASK(bit))
#define CLEAR_BIT(value,bit)		    ((value) &= ~BIT_MASK(bit))
#define TEST_BIT(value,bit)			(((value) & BIT_MASK(bit)) ? 1 : 0)
#define SET_BITS(value, mask, newvalue)		(value = (value & ~mask) | (newvalue & mask))

int state = 0;
uint32_t time_ms = 100;

void GPIO_init(void){
	// Enable AHB2 clock
	RCC->AHB2ENR = 0b1; // A

	// PA1 - output
	GPIOA->MODER   &= 0b11111111111111111111011111111111;
	GPIOA->OTYPER  &= 0b11111111111111111111001111111111;
	GPIOA->OSPEEDR &= 0b11111111111111111111101111111111;
	GPIOA->PUPDR   &= 0b11111111111111111111101111111111;//pull-up
}

void SystemClockConfig(){
	RCC->CR |= RCC_CR_HSION;
	while((RCC->CR & RCC_CR_HSION) == 0);

	SET_BITS(RCC->CFGR, 0b1111<<4, 0b1011<<4); 	// AHB prescaler : sclk/16
	SET_BITS(RCC->CFGR, 0b11, 0b01);			// System clock switch : HSI16
}

void SysTick_Handler(){
	if(time_ms == 0){
		time_ms = 100;
		state++;
	}
	else
		time_ms--;

	if(state == 1){ //off
		GPIOA->BRR |= 1<<5;
	}
	if(state == 2){ //light up
		GPIOA->BSRR |= 1<<5;
		state = 0;
	}
}

void SysTick_MYConfig(){
	 // Reload value
	  SysTick->LOAD  = (30000 & SysTick_LOAD_RELOAD_Msk)-1;

	  // set Priority for Cortex System Interrupts
	  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);

	  // SysTick當前数值寄存器
	  SysTick->VAL   = 0;

	  //Enable SysTick IRQ and SysTick Timer : [0]=1:enable [1]=1:tickint(數到0異常請求) [2]=1处理器时钟AHB
	  SysTick->CTRL  = 0b111;
}

int main(void){
	GPIO_init();
	SystemClockConfig();
	GPIOA->BSRR |= 1<<5;
	//SysTick_Config(30000);
	SysTick_MYConfig();
	while(1);
}

/*
SysTick_Config(uint32_t ticks)函数说明:
函数自动完成：重装载值的装载，时钟源选择，计数寄存器复位，中断优先级的设置(最低)，开中断，开始计数的工作。
brief  Initialize and start the SysTick counter and its interrupt.
param : ticks number of ticks between two interrupts

Initialize the system tick timer and its interrupt
and start the system tick timer/counter in free running mode
to generate periodical interrupts.

static __INLINE uint32_t SysTick_Config(uint32_t ticks)
{
  if (ticks > SysTick_LOAD_RELOAD_Msk)  return (1); // 重装载值要<0XFF FFFF,因为是24位的递减计数器

  SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1; // reload

  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1); // set Priority for Cortex System Interrupts

  SysTick->VAL   = 0; // Load the SysTick Counter Value

  //Enable SysTick IRQ and SysTick Timer
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;
  return (0);
}

 */
