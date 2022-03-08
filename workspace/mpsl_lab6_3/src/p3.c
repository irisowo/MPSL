/*
#include "stm32l476xx.h"

void display (int data, int len);
extern void GPIO_init();
extern void max7219_send(unsigned char address, unsigned char data);

#define ROW_MAX 4
#define COL_MAX 4
int check = -1;
int duty_cycle = 0;
float freq = 0;
unsigned int keypad_value[4][4] ={{1,2,3,10},
                            	  {4,5,6,11},
								  {7,8,9,12},
								  {15,0,14,13}};

//HCLK / (prescaler + 1) / 100 =333 -->PSC~120-1

void keypad_init(){
    GPIO_init(); //initiate 7-seg
    RCC->AHB2ENR   |= 0b00000000000000000000000000000111; //safely initialize again

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
}

#define MODER_AF 01
void GPIO_init_AF(){
	RCC->AHB2ENR |=	RCC_AHB2ENR_GPIOAEN;
	GPIOA->MODER &= 0xFFFFFFFC;
	GPIOA->MODER |= (MODER_AF);

	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL0; // //AFR[0] LOW
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL0_0; // PA0: TIM2_CH1: AF1
	return;
}


void Timer_init(){
	//Timer off
	TIM2->CR1 &= ~TIM_CR1_CEN;
	//TIM2 enable
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	// Edge-Aligned
	TIM2->CR1 &= ~TIM_CR1_CMS;
	//PSC
	TIM2->PSC = (uint32_t) 3999 ;
	//Reload value
	TIM2->ARR = (uint32_t) 99;
	TIM2->CR1 |= TIM_CR1_ARPE;  //ARPE enable
	//count down
	TIM2->CR1 |= TIM_CR1_DIR;   //down
	//enable ch1 output
	TIM2->CCER = TIM_CCER_CC1E;
	//PWM1 0110
	TIM2->CCMR1 &= 0xFFFFFCFF;
	TIM2->CCMR1 |= ( TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 );
	// enable output compare 1 preload register
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE;
	//set CCR's count value
	TIM2->CCR1 = (uint32_t) 10;
	 //re-init
	TIM2->EGR = TIM_EGR_UG;
	//Timer on
	TIM2->CR1 |= TIM_CR1_CEN;
	return;
}

int keypad_scan(){
    int Yi=0,Xi=0;
    int flag_keypad, flag_debounce, k;
    //char key_val=-1;
	GPIOC->ODR=0xF;
	flag_keypad=GPIOB->IDR&0xF;

	if(flag_keypad!=0){
		k=40000;
		while(k!=0){
			flag_debounce=GPIOB->IDR&0xF;
			k--;
		}
		if(flag_debounce!=0){
          for(Yi=0;Yi<ROW_MAX;Yi++){ //output data from 1st row
             GPIOC->ODR&=0;
             GPIOC->ODR|=(1<<Yi);
        	     for(Xi=0;Xi<COL_MAX;Xi++){ //read input data from 1st col
                int value_get=GPIOB->IDR&0xf;
                int is_pressed=(value_get>>Xi)&1;
                if(is_pressed){
                	    return keypad_value[Yi][Xi];
                }
             }
          }
          GPIOC->ODR=GPIOC->ODR|0xF;
        }
	}
	else{
		return -1;
	}
}

int main(){
	int duty = 10, r;
	GPIO_init();
	keypad_init();
	GPIO_init_AF();
	Timer_init();
	display (duty, 2);
	TIM2->CR1 |= TIM_CR1_CEN;//TIM2 start
	while(1){
		r = keypad_scan();
		if(r == 14){
			if(duty > 10) duty -= 5;
			TIM2->CR1 |= TIM_CR1_CEN;
			TIM2->CCR1 = duty;
			display (duty, 2);
		}
		else if(r == 15){
			if(duty < 90)duty += 5;
			TIM2->CR1 |= TIM_CR1_CEN;
			TIM2->CCR1 = duty;
			display (duty, 2);
		}
		else{
			//TIM2->CR1 &= ~TIM_CR1_CEN;
			display (duty, 2);
		}
	}
}


void display(int data, int num_bits){
	int i=0;
	//clear
	for (; i<=8; i++)
		max7219_send(i, 0xF);
	//send
	for (i=1; i<=num_bits; i++){
		if(i==num_bits){ //7th
			max7219_send(i, data % 10);
		}
		else{ //1~6th bits
			max7219_send(i, data % 10);
			data /= 10;
		}
	}
}
*/
