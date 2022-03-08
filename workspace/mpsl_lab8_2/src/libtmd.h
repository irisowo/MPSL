#ifndef LIBTMD_H_
#define LIBTMD_H_

extern void delay_ms(unsigned);
extern void fpu_enable();

#include "stm32l476xx.h"

#ifndef GPIO_PIN_0
#define GPIO_PIN_0   ((uint16_t) 0x0001)
#define GPIO_PIN_1   ((uint16_t) 0x0002)
#define GPIO_PIN_2   ((uint16_t) 0x0004)
#define GPIO_PIN_3   ((uint16_t) 0x0008)
#define GPIO_PIN_4   ((uint16_t) 0x0010)
#define GPIO_PIN_5   ((uint16_t) 0x0020)
#define GPIO_PIN_6   ((uint16_t) 0x0040)
#define GPIO_PIN_7   ((uint16_t) 0x0080)
#define GPIO_PIN_8   ((uint16_t) 0x0100)
#define GPIO_PIN_9   ((uint16_t) 0x0200)
#define GPIO_PIN_10  ((uint16_t) 0x0400)
#define GPIO_PIN_11  ((uint16_t) 0x0800)
#define GPIO_PIN_12  ((uint16_t) 0x1000)
#define GPIO_PIN_13  ((uint16_t) 0x2000)
#define GPIO_PIN_14  ((uint16_t) 0x4000)
#define GPIO_PIN_15  ((uint16_t) 0x8000)
#define GPIO_PIN_ALL ((uint16_t) 0xFFFF)
#endif

#ifndef SET_BIT
#define SET_BIT(REG, BIT) ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT) ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT) ((REG) & (BIT))
#define CLEAR_REG(REG) ((REG) = (0x0))
#define WRITE_REG(REG, VAL) ((REG) = (VAL))
#define READ_REG(REG) ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
#endif

void GPIO_SetPinLow(GPIO_TypeDef *GPIOX, uint16_t GPIO_PIN_Y) {
	GPIOX->BRR = GPIO_PIN_Y;
}

void GPIO_SetPinHigh(GPIO_TypeDef *GPIOX, uint16_t GPIO_PIN_Y) {
	GPIOX->BSRR = GPIO_PIN_Y;
}

int GPIO_ReadInputDataBit(GPIO_TypeDef *GPIOX, uint16_t GPIO_PIN_Y) {
	return GPIOX->IDR & GPIO_PIN_Y;
}

#endif
