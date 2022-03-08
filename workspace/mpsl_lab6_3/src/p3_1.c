#include "stm32l476xx.h"
extern void GPIO_init();
extern void max7219_init();
extern void max7219_send(unsigned char address, unsigned char data);
int duty = 10;
int keypad_value[4][4] ={{1,2,3,10},
                         {4,5,6,11},
						 {7,8,9,12},
						 {15,0,14,13}};

#define MODER_AF 0b10
#define LED_PIN 0b1

void GPIO_init_AF(){
 RCC->AHB2ENR |= 0x3;
 //GPIOB->MODER |= GPIO_MODER_MODE1_1;
 //GPIOB->AFR[0] |= GPIO_AFRL_AFSEL1_0; //PORT1 PIN4

 GPIOA->MODER &= (~(0b11 << LED_PIN * 2));
 GPIOA->MODER |= (MODER_AF << LED_PIN * 2);

 GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL1; // PA1 mask
 GPIOA->AFR[0] |= GPIO_AFRL_AFSEL1_0; // PA1: TIM2_CH2: AF1
}

void Timer_init(TIM_TypeDef *timer){
 RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
 timer->CR1 |= TIM_CR1_ARPE;//enable auto-reload preload
 timer->PSC = (uint32_t)399;
 //HCLK / (prescaler + 1) / 100 =333 -->PSC~120-1
 //timer->PSC = (uint32_t)120;
 timer->ARR = (uint32_t) 99;
 // Capture/Compare 2 output enable
 timer->CCER |= TIM_CCER_CC2E;
 // [6:4] 0111: PWM2
 timer->CCMR1 |= ( (TIM_CCMR1_OC2M & 0xFFFFFF8F) | (0b01110000) );

 timer->CCR2 = duty;

 timer->EGR = TIM_EGR_UG; // reinit counter
 timer->CCMR1 |= TIM_CCMR1_OC2PE;//preload enable

}

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

int last=-1;
int keypad_scan(){
    //if pressed , keypad return the value of that key, otherwise, return 255 for no pressed (unsigned char)
    int Yi=0,Xi=0;
    char key_val=-1;
    // clear out per second
    int tmp=1000;
    while(tmp--){
    	//4x4 detection
        for(Yi=0;Yi<4;Yi++){
        	//signal in horizontal direction
           	GPIOC->ODR&=0;
            GPIOC->ODR|=(1<<Yi);
            for(Xi=0;Xi<4;Xi++){
                // Get value in straight direction
                int value_get=GPIOB->IDR&0xf;
                int is_pressed=(value_get>>Xi)&1;
                if(is_pressed){
                    key_val=keypad_value[Yi][Xi];
                    //clear if the current value is diff from the last one
                    //if(key_val!=last)  display_clr(2);
                    //display
                    display(keypad_value[Yi][Xi],(key_val>=10?2:1));
                    last=key_val;
                }

            }
        }
    }
    return key_val;
}


void PWM_channel_init(){
//TODO: Initialize timer PWM channel
}


int main(){
 int r;
 TIM_TypeDef *timer = TIM2;
 //GPIO_init();
 keypad_init();
 GPIO_init_AF();
 Timer_init(timer);
 PWM_channel_init();
 timer->CR1 |= TIM_CR1_CEN; ////////
 display (duty, 2);
 while(2){
   r = keypad_scan();
   if(r == 14){
     if(duty > 10)duty -= 5;
     Timer_init(timer);
     //timer->CCR2 = duty;
     display (duty, 2);
   }
   else if(r == 15){
     if(duty < 90)duty += 5;
     timer->CCR2 = duty;
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
