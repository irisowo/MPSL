#include "stm32l476xx.h"
extern void GPIO_init();
extern void max7219_init();
extern void max7219_send(unsigned char address, unsigned char data);
int duty = 10;
int keypad_value[4][4] ={{1,2,3,10},
                         {4,5,6,11},
						 {7,8,9,12},
						 {15,0,14,13}};

void GPIO_init_AF(){
 RCC->AHB2ENR |= 0x2;
 GPIOB->MODER |= GPIO_MODER_MODE3_1;
 GPIOB->AFR[0] |= GPIO_AFRL_AFSEL3_0; //PORT1 PIN4
}

void Timer_init(TIM_TypeDef *timer){
 RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
 timer->CR1 |= TIM_CR1_ARPE;
 timer->PSC = (uint32_t)399;
 timer->ARR = (uint32_t) 99;
 timer->CCER |= TIM_CCER_CC2E; // enable
 timer->CCMR1 |= (TIM_CCMR1_OC2M); // as pwm MODE
 timer->CCR2 = duty;

 timer->EGR = TIM_EGR_UG; // reinit counter
 timer->CCMR1 |= TIM_CCMR1_OC2PE;

}
void keypad_init(){
 // SET keypad gpio OUTPUT //
 RCC->AHB2ENR = RCC->AHB2ENR|0x5;
 GPIOA->MODER= GPIOA->MODER&0xFF55FFFF; //FF55FFFF
 GPIOA->PUPDR=GPIOA->PUPDR|0x550000; //550000
 GPIOA->OSPEEDR=GPIOA->OSPEEDR|0x550000;
 GPIOA->ODR=GPIOA->ODR|0xF<<8;

 GPIOC->MODER=GPIOC->MODER&0xFFFC03FF;
 GPIOC->PUPDR=GPIOC->PUPDR|0x2A800;
 GPIOC->OSPEEDR=GPIOC->OSPEEDR|0x15400;
}

int keypad_scan(){
 int flag_keypad, flag_debounce, Pc, Pr, value_get, k;

 GPIOA->ODR=0xF<<8;
 flag_keypad=GPIOC->IDR&0xF<<5;

 if(flag_keypad!=0){
  k=45000;
  while(k!=0){
   flag_debounce=GPIOC->IDR&0xF<<5;
   k--;
  }
  if(flag_debounce!=0){
     for(int i=0;i<4;i++){ //scan keypad from first column
       Pc=i+8;
       //set PA8,9,10,12(column) low and set pin high from PA8
       GPIOA->ODR=(GPIOA->ODR&0xFFFFF0FF)|1<<Pc;
       for(int j=0;j<4;j++){ //read input from first row
         Pr=j+5;
         value_get=GPIOC->IDR&1<<Pr;
         if(value_get!=0){
           return keypad_value[j][i];
         }
       }
     }
   GPIOA->ODR=GPIOA->ODR|0xF<<8;
  }
 }
 else{
   return -1;
 }
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
 while(1){
   r = keypad_scan();
   if(r == 1){
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
