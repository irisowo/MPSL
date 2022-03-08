#include "stm32l476xx.h"
#include "core_cm4.h"
#include "lib.h"

int freq = 500;
int counting = 0;

// GPIO
char buf[300];

void read_button() {
	int cnt = 0;
	while(1) {
		int button = GPIOC->IDR & (1 << 13);
		if (button == 0) { // 0
			cnt ++;
		} else if (cnt > (1 << 13)) { // 1 trigger
			cnt = 0;
			break;
		} else { // 1
			cnt = 0;
		}
	}
}


void gpio_init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	MODER_SETTING(GPIOA, 9, MODER_AF); //TX
	MODER_SETTING(GPIOA, 10, MODER_AF); //RX

	MODER_SETTING(GPIOC, 13, MODER_I) //Button
	OSPEEDR_SETTING(GPIOC, 13, OSPEEDR_HIGH)
	PUPDR_SETTING(GPIOC, 13, PUPDR_PU)

	MODER_SETTING(GPIOA, 5, MODER_O);
	MODER_SETTING(GPIOA, 0, MODER_A);

	GPIOA->ODR &= ~(1 << 5);
	GPIOA->AFR[1] |= (7 << 4) + (7 << 8);
	GPIOA->ASCR |= (1<<0);
}

/* uart */
int UART_Transmit(uint8_t *arr, uint32_t size) {
	char *str = arr;
	int ret = 0;
	for (int i = 0; str[i] && i < size; i ++) {
		while ((USART1->ISR & USART_ISR_TXE) == 0); // wait until 1: data is transferred to the shift register
		USART1->TDR = str[i]; //USART transmit data register
		ret ++;
	}
	while ((USART1->ISR & USART_ISR_TXE) == 0);
	return ret;
}


char receive_char() {
	while (!(USART1->ISR & USART_ISR_RXNE));
	// USART1->RQR |= USART_RQR_RXFRQ;
	USART1->ISR = USART1->ISR & ~USART_ISR_RXNE;
	return USART1->RDR & 0xFF;
}

void read_cmd() {
	int ptr = 0;
	char c;
	do {
		c = receive_char();
		buf[ptr++] = c;
		UART_Transmit((uint8_t*)&c, 1);
	} while (c != '\n' && c != '\r');
	buf[ptr++] = '\0';
	//UART_Transmit((uint8_t*)"\r\n", 2);
}

void init_UART() {
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	// CR1
	//CR1_M set word length = 8bit,
	USART1->CR1 &= ~(USART_CR1_M | USART_CR1_PS | USART_CR1_PCE | USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8);
	USART1->CR1 |= (USART_CR1_TE | USART_CR1_RE);

	// CR2-stop bit
	USART1->CR2 &= ~(USART_CR2_STOP);

	// CR3
	USART1->CR3 &= ~(USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_ONEBIT);

	// BRR
    // In case of oversampling 16 : USARTDIV = Fck / Tx/Rx baud
	USART1->BRR &= ~(0xFF);
	USART1->BRR |= (1000000L / 9600L) & 0xFFFF ;
	//USART1->BRR |= (2000000L / 115200L) & 0xFFFF ;

	// Turn off Other Mode, turn off clk since it's asynchronous
	USART1->CR2 &= ~(USART_CR2_LINEN | USART_CR2_CLKEN);
	USART1->CR3 &= ~(USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);

	// Enable UART
	USART1->CR1 |= (USART_CR1_UE);
	// USART1->ISR |= USART_ISR_RXNE;
	// NVIC_EnableIRQ(USART1_IRQn);
}

void set_clock() {
	// Set system clock as MSI
	RCC->CFGR &= ~3;

	// HPRE -> 1MHz
	RCC->CFGR &= ~(0xF << 4);
	RCC->CFGR |= 0b1011 << 4; // AHB prescaler 16
	//RCC->CFGR |= 0b1010 << 4; // AHB prescaler 8

	// enable HSION
	RCC->CR |= 1 << 8;

	// Set system clock as PLL
	RCC->CFGR |= 1;
}


void systick_config() {
	SysTick->CTRL |= 7;
	SysTick->LOAD = 500000;
	NVIC_SetPriority(SysTick_IRQn, 1);
	NVIC_SetPriority(ADC1_2_IRQn, 0);
}

void SysTick_Handler() {
	ADC1->CR |= ADC_CR_ADSTART;
}

int main() {
	gpio_init();
	set_clock();
	systick_config();
	init_UART();

	while (1) {
		read_button();
		UART_Transmit((uint8_t*)"Hello World!\r\n", 14);
	}
}






