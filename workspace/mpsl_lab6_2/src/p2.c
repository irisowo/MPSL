#include "stm32l476xx.h"
#include <stdio.h>
#include <stdlib.h>
#define TIME_SEC 12.70
#define TIME_SEC 30.00

extern void GPIO_init();
extern void max7219_init();
extern void max7219_send(unsigned char address, unsigned char data);

// global var
unsigned int millisecond;
unsigned int second;
unsigned int get_cur_cnt_value;

//4MHz  = 40000*100 =  (39999+1)*(99+1)
void Timer_init( TIM_TypeDef *timer){
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; //TIM2 ON
    TIM2->CR1 &= 0x0000; //Turned on the counter as the count up mode
    TIM2->PSC = 39999U;  //Prescaler, how many counter clock cycle to update counter
    TIM2->ARR = 99U;
    TIM2->EGR = 0x0001;  //re-initailzie timer to startup
}

void Timer_start(TIM_TypeDef *timer){
    TIM2->CR1 |= TIM_CR1_CEN; //Turn on the counter mode, change in the control register
    TIM2->SR &= ~(TIM_SR_UIF); //off the user interrupt mode, so the cpu can keep working on the clock increment
}

void timer_display(int data,int len){
	unsigned int tmp_s = second;
    unsigned int tmp_ms = millisecond;

    if( data == 0 && len == 3 ){
        for(int i = 1;i <= len;i++){
            if(i == 3)
            	    max7219_send(i,(data % 10) | (0x80));
            else
            	    max7219_send(i,data % 10);
            data /= 10;
        }
    }
    else if(data == 100 && len ==100){ //stop the timer
        tmp_s = TIME_SEC / 1;
        tmp_ms = TIME_SEC*100-tmp_s*100;
        for(int i = 1;i <= len;i++){
            if(i<3){
                max7219_send(i,tmp_ms % 10);
                tmp_ms /= 10;
            }
            else if(i == 3){
                max7219_send(i,(tmp_s % 10) | (0x80)); //the float digit bit has to be turned on
                tmp_s /= 10;
                if(tmp_s == 0) break;
            }
            else{
                max7219_send(i,tmp_s % 10);
                tmp_s /= 10;
                if(tmp_s == 0) break;
            }
        }
    }
    else{
        for(int i = 1;i <= len;i++){
            if(i<3){
                max7219_send(i,tmp_ms % 10);
                tmp_ms /= 10;
            }
            else if(i == 3){
                max7219_send(i,(tmp_s % 10) | (0x80));//add floating point
                tmp_s /= 10;
                if(tmp_s == 0) break;
            }
            else{
                max7219_send(i,tmp_s % 10);
                tmp_s /= 10;
                if(tmp_s == 0) break;
            }
        }
    }
}

int display_clr(){
    for(int i = 0;i <= 8;i++)
        max7219_send(i,0xF);
    return 0;
}

int main()
{
 	GPIO_init();
	max7219_init();
	Timer_init(TIM2);
	Timer_start(TIM2);
    display_clr();

    int TARGET_SEC = TIME_SEC / 1;
    int TARGET_MSEC = TIME_SEC * 100 - ( TARGET_SEC * 100 );
    second=0;
    millisecond = 0;

	while(1){
		millisecond = TIM2->CNT;
		//Check Target
        if(TIME_SEC < 0.01 || TIME_SEC > 10000.0){
            timer_display(0,3);
        }
        millisecond = TIM2->CNT;
        //Check Bound
        if( second == TARGET_SEC && TIM2->CNT == TARGET_MSEC){
            timer_display(100,100);
            while(1);
        }
        millisecond = TIM2->CNT;
        //Display
        timer_display(1,8);
        millisecond = TIM2->CNT;
        //UEV時 update flag(UIF in SR reg)
        if(TIM2->SR & 0x0001){ //one second is reached
            second+=1;
            TIM2->SR &= ~(TIM_SR_UIF); //reset again for next counter event
        }
        millisecond = TIM2->CNT;
        timer_display(1,8);
	}
    return 0;
}

