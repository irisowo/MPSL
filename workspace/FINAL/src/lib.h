#ifndef LIB_H
#define LIB_H

#define MODER_I 0b00
#define MODER_O 0b01
#define MODER_AF 0b10
#define MODER_A 0b11

#define OTYPER_PP 0b0
#define OTYPER_OD 0b1

#define OSPEEDR_LOW       0b00
#define OSPEEDR_MEDIUM    0b01
#define OSPEEDR_HIGH      0b10
#define OSPEEDR_VERY_HIGH 0b11

#define PUPDR_NO_PP      0b00
#define PUPDR_PU         0b01
#define PUPDR_PD         0b10
#define PUPDR_RESERVED   0b11

#define MODER_SETTING(BASE, PIN, VAL)            \
		(BASE)->MODER &= ~(0b11 << ((PIN)*2));   \
		(BASE)->MODER |= ((VAL) << ((PIN)*2));   \

#define PUPDR_SETTING(BASE, PIN, VAL)            \
		(BASE)->PUPDR &= ~(0b11 << ((PIN)*2));   \
		(BASE)->PUPDR |= ((VAL) << ((PIN)*2));   \

#define OSPEEDR_SETTING(BASE, PIN, VAL)          \
		(BASE)->OSPEEDR &= ~(0b11 << ((PIN)*2)); \
		(BASE)->OSPEEDR |= ((VAL) << ((PIN)*2)); \

#define OTYPER_SETTING(BASE, PIN, VAL)           \
		(BASE)->OTYPER &= ~(0b1  << (PIN));      \
		(BASE)->OTYPER |= ((VAL) << (PIN));      \


#endif
