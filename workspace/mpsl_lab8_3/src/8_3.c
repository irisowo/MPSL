
#include "stm32l476xx.h"
#include "core_cm4.h"
#include "lib.h"

int freq = 500;
int counting = 0;



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

//UART
int UART_Transmit(uint8_t *arr, uint32_t size) {
	char *str = arr;
	int ret = 0;
	for (int i = 0; str[i] && i < size; i ++) {
		while ((USART1->ISR & USART_ISR_TXE) == 0);
		USART1->TDR = str[i];
		ret ++;
	}
	while ((USART1->ISR & USART_ISR_TXE) == 0);
	return ret;
}

char buf[300];

char receive_char() {
	while (!(USART1->ISR & USART_ISR_RXNE));
	USART1->ISR = USART1->ISR & ~USART_ISR_RXNE;
	//USART_DR和received shift register之間包含一個緩衝器(RDR)。
	return USART1->RDR & 0xFF;
}

void read_cmd() {
	int ptr = 0;
	char c;
	do {
		c = receive_char();
		if (c=='\x7F')//backspace
			if(ptr>0) { buf[--ptr] = '\0';}
		else
			buf[ptr++] = c;
		UART_Transmit((uint8_t*)&c, 1);
	} while (c != '\n');
	buf[ptr++] = '\0';
	UART_Transmit((uint8_t*)"\n", 2);
}

void init_UART() {
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	// CR1
	USART1->CR1 &= ~(USART_CR1_M | USART_CR1_PS | USART_CR1_PCE | USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8);
	USART1->CR1 |= (USART_CR1_TE | USART_CR1_RE);

	// CR2
	USART1->CR2 &= ~(USART_CR2_STOP);

	// CR3
	USART1->CR3 &= ~(USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_ONEBIT);

	// BRR
	USART1->BRR &= ~(0xFF);
	USART1->BRR |= (1000000L / 9600L) & 0xFFFF ;

	// Turn off Other Mode, turn off clk since it's asynchronous
	USART1->CR2 &= ~(USART_CR2_LINEN | USART_CR2_CLKEN);
	USART1->CR3 &= ~(USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);

	// Enable UART
	USART1->CR1 |= (USART_CR1_UE);
	// USART1->ISR |= USART_ISR_RXNE;

	// NVIC_EnableIRQ(USART1_IRQn);
}

void init_ADC(){
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
	// ADC123_COMMON->CCR |= ADC_CCR_CKMODE_0;

	// ADC1->CFGR |= ADC_CFGR_CONT; // continue
	ADC1->CFGR &= ~(ADC_CFGR_CONT); // continue
	ADC1->CFGR |= 0 << 3;
	ADC1->CFGR &= ~ADC_CFGR_ALIGN;

	//ADC123_COMMON->CCR |= 11 << 18;
	ADC123_COMMON->CCR |= 1 << 16; // ckmode
	ADC123_COMMON->CCR |= 4 << 8; // delay

	ADC1->SQR1 |= 0<<0;
	ADC1->SQR1 |= 5<<6;

	ADC1->SMPR1 |= 2<<15;//ADC clock cycle

	ADC1->CR &= ~ADC_CR_DEEPPWD;
	ADC1->CR |= ADC_CR_ADVREGEN;

	for (int i=0; i<200; i++);

	ADC1->IER |= ADC_IER_EOCIE;
	NVIC_EnableIRQ(ADC1_2_IRQn);

//	ADC1->CFGR |= 4<<6;
//	ADC1->CFGR |= ADC_CFGR_EXTEN_0;

	ADC1->CR |= ADC_CR_ADEN;
	while (!(ADC1->ISR & ADC_ISR_ADRDY));
	ADC1->CR |= ADC_CR_ADSTART;
}

int ADC_data = 1;

void ADC1_2_IRQHandler(){
	// UART_Transmit((uint8_t*)"Scan\r\n", 20);
	NVIC_ClearPendingIRQ(ADC1_2_IRQn);
	for (int i=0; i<(1<<8); i++);
	ADC_data = ADC1->DR;
	ADC1->ISR |= ADC_ISR_EOC; //
	UART_Transmit_Number(ADC_data);

	NVIC_ClearPendingIRQ(ADC1_2_IRQn);
}

void UART_Transmit_Number(int n) {
	int dig[12] = {0};
	for (int i = 0; i < 12; i ++) {
		dig[i] = n % 10;
		n /= 10;
	}
	for (int i = 11; i >= 0; i --) {
		char c = '0' + dig[i];
		UART_Transmit((uint8_t*)&c, 1);
	}
	UART_Transmit((uint8_t*)"\n", 2);
}

void set_clock() {
	// Set system clock as MSI
	RCC->CFGR &= ~3;

	// HPRE -> 1MHz
	RCC->CFGR &= ~(0xF << 4);
	RCC->CFGR |= 11 << 4;

	// enable HSION
	RCC->CR |= 1 << 8;

	// Set system clock as PLL
	RCC->CFGR |= 1;
}

void systick_enable() {
	SysTick->CTRL |= 1;
	int delay=1000000;
	while(delay--);
}

void systick_disable() {
	SysTick->CTRL &= ~1;
}

void systick_config() {
	SysTick->CTRL |= 7;
	SysTick->LOAD = 500000;
	 NVIC_SetPriority(SysTick_IRQn, 1);
	 NVIC_SetPriority(ADC1_2_IRQn, 0);
}

void SysTick_Handler() {
	ADC1->CR |= ADC_CR_ADSTART;
	// UART_Transmit((uint8_t*)"Systick\r\n", 10);
}

int main() {
	gpio_init();
	set_clock();
	systick_config();
	init_UART();
	init_ADC();
	systick_disable();

	UART_Transmit((uint8_t*)"Start\015",7 );
	//debug
	GPIOA->ODR |= (1 << 5);

	while (1) {
		UART_Transmit((uint8_t*)"> ", 2);
		read_cmd();
		// UART_Transmit((uint8_t*)buf, 50);
		if (strncmp(buf, "showid", 6) == 0 && buf[6]=='\0') {
			UART_Transmit((uint8_t*)"0616086\n", 15);
		} else if (strncmp(buf, "light", 5) == 0) {
			systick_enable();
			char c;
			do {
				c = receive_char();
			} while (c != 'q');
			systick_disable();
		} else if (strncmp(buf, "led off", 7) == 0) {
			GPIOA->ODR |= (1 << 5);
		} else if (strncmp(buf, "led on", 6) == 0) { //active low
			GPIOA->ODR &= ~(1 << 5);
		}else {
			if(buf[0]!='\0')
				UART_Transmit((uint8_t*)"Unknown Command\r\n", 20);
		}
	}
}

