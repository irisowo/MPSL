#include <stdio.h>
#include <stdlib.h>
#include "stm32l476xx.h"
#define ROW_MAX 4
#define COL_MAX 4
unsigned int keypad_value[4][4] ={{1,2,3,10},
                            {4,5,6,11},
                            {7,8,9,12},
                            {15,0,14,13}};
extern void GPIO_init();
extern void max7219_send(unsigned char address, unsigned char data);
extern void max7219_init();
void keypad_init()
{
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

int display_clr(int num_bits){
	for(int i=1;i<=num_bits;i++){
		max7219_send(i,0xF);
	}
	return 0;
}
//integer to display (-99999999~99999999)
int display(int data, int num_bits){
    int i=0;
    for(i=1;i<=num_bits;i++){
        max7219_send(i,data%10);
        data/=10; //get the next digit
    }
    return (data>99999999 || data<-9999999) ? -1 : 0;
}

/* TODO: scan keypad value
* return:
* >=0: key pressed value
* -1: no key press
*/
int last=-1;
char keypad_scan(){
    //if pressed , keypad return the value of that key, otherwise, return 255 for no pressed (unsigned char)
    int Yi=0,Xi=0;
    char key_val=-1;
    // clear out per second
    int tmp=1000;
    while(tmp--){
    	//4x4 detection
        for(Yi=0;Yi<ROW_MAX;Yi++){
        	//signal in horizontal direction
           	GPIOC->ODR&=0;
            GPIOC->ODR|=(1<<Yi);
            for(Xi=0;Xi<COL_MAX;Xi++){
                // Get value in straight direction
                int value_get=GPIOB->IDR&0xf;
                int is_pressed=(value_get>>Xi)&1;
                if(is_pressed){
                    key_val=keypad_value[Yi][Xi];
                    //clear if the current value is diff from the last one
                    if(key_val!=last)  display_clr(2);
                    //display
                    display(keypad_value[Yi][Xi],(key_val>=10?2:1));
                    last=key_val;
                }

            }
        }
    }
    return key_val;
}

int main(){
    GPIO_init();
    max7219_init();
    display_clr(8);
    keypad_init();
    while(1) {
    	keypad_scan();
    	display_clr(2);
    }
    return 0;
}
