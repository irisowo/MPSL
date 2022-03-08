#ifndef LIBADC_H_
#define LIBADC_H_

#include "stm32l476xx.h"

float resistor = 0.0f;

void SysTick_UserConfig(float sec) {
	SysTick->CTRL |= 0x00000004;
	SysTick->LOAD = (uint32_t) (sec * 4000000.0f);
	SysTick->VAL = 0;
	SysTick->CTRL |= 0x00000003;
}

void SysTick_Handler() {
	ADC1->CR |= ADC_CR_ADSTART; // start adc conversion
}

void ADC1_2_IRQHandler() {
	while (!(ADC1->ISR & ADC_ISR_EOC)); // wait for conversion complete
	float voltage = (float) ADC1->DR / 4096.0f * 5.0f;
	resistor = (5500.0f - 1100.0f * voltage) / voltage;
}

void ADC1_Init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
	GPIOC->MODER |= 0b11; // analog mode
	GPIOC->ASCR |= 1; // connect analog switch to ADC input
	ADC1->CFGR &= ~ADC_CFGR_RES; // 12-bit resolution
	ADC1->CFGR &= ~ADC_CFGR_CONT; // disable continuous conversion
	ADC1->CFGR &= ~ADC_CFGR_ALIGN; // right align
	ADC123_COMMON->CCR &= ~ADC_CCR_DUAL; // independent mode
	ADC123_COMMON->CCR &= ~ADC_CCR_CKMODE; // clock mode: hclk / 1
	ADC123_COMMON->CCR |= 1 << ADC_CCR_CKMODE_Pos;
	ADC123_COMMON->CCR &= ~ADC_CCR_PRESC; // prescaler: div 1
	ADC123_COMMON->CCR &= ~ADC_CCR_MDMA; // disable dma
	ADC123_COMMON->CCR &= ~ADC_CCR_DELAY; // delay: 5 adc clk cycle
	ADC123_COMMON->CCR |= 4 << ADC_CCR_DELAY_Pos;
	ADC1->SQR1 &= ~(ADC_SQR1_SQ1 << 6); // channel: 1, rank: 1
	ADC1->SQR1 |= (1 << 6);
	ADC1->SMPR1 &= ~(ADC_SMPR1_SMP0 << 3); // adc clock cycle: 12.5
	ADC1->SMPR1 |= (2 << 3);
	ADC1->CR &= ~ADC_CR_DEEPPWD; // turn off power
	ADC1->CR |= ADC_CR_ADVREGEN; // enable adc voltage regulator
	for (int i = 0; i <= 1000; ++i); // wait for regulator start up
	ADC1->IER |= ADC_IER_EOCIE; // enable end of conversion interrupt
	NVIC_EnableIRQ(ADC1_2_IRQn);
	ADC1->CR |= ADC_CR_ADEN; // enable adc
	while (!(ADC1->ISR & ADC_ISR_ADRDY)); // wait for adc start up
}

#endif
