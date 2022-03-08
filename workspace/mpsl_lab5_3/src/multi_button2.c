/*
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "stm32l476xx.h"
#define keypad_row_max 4
#define keypad_col_max 4
unsigned int keypad_value[4][4] ={{1,2,3,10},
                            	  {4,5,6,11},
								  {7,8,9,12},
								  {15,0,14,13}};
extern void GPIO_init();
extern void max7219_send(unsigned char address, unsigned char data);
extern void max7219_init();
void keypad_init()
{
    GPIO_init(); //have initialized in arm
    RCC->AHB2ENR   |= 0b00000000000000000000000000000111; //safely initialize again

    GPIOC->MODER   &= 0b11111111111111111111111100000000; //use PC0123 for Yi
    GPIOC->MODER   |= 0b00000000000000000000000001010101;
    GPIOC->PUPDR   &= 0b11111111111111111111111100000000; //clear and set output use pup since we want 1 to be sent high level voltage
    GPIOC->PUPDR   |= 0b00000000000000000000000001010101;
    GPIOC->OSPEEDR &= 0b11111111111111111111111100000000;
    GPIOC->OSPEEDR |= 0b00000000000000000000000001010101;

    GPIOB->MODER   &= 0b11111111111111111111111100000000; //use PB 0123 for Xi
    GPIOB->PUPDR   &= 0b11111111111111111111111100000000; //clear and set input as pdown mode
    GPIOB->PUPDR   |= 0b00000000000000000000000001010101; //clear and set input as pdown mode
}


int display_clr(int num_bits){
	for(int i=1;i<=num_bits;i++){
		max7219_send(i,0xF);
	}
	return 0;
}
int display(int data, int num_bits){
    int i=0;
    for(i=1;i<=num_bits;i++){
        max7219_send(i,data%10);
        data/=10;
    }
    return (data>99999999 || data<-9999999) ? -1 : 0;
}


unsigned char keypad_scan(){

    //set pressed_table
    bool pressed_table[17];
    memset(pressed_table,0,sizeof(bool)*17);

    // var used in while()
    int Yi=0,Xi=0;
    int out_sum=0;
    int nothing_is_pressed=1;
    unsigned char key_val=-1;

    //delay 3s
    int tmp=3000;
    while(1){
        nothing_is_pressed=1;
        //detect 4x4
        int a=-1;
    	for(Yi=0;Yi<4;Yi++){ //output data from 1st row
			GPIOC->ODR|=1;
	    	switch(Yi){
	    	case 0:
	    		GPIOC->ODR&=(0b1110);
	    		break;
	    	case 1:
	    		GPIOC->ODR&=(0b1101);
	    	    break;
	    	case 2:
	    		GPIOC->ODR&=(0b1011);
	    	 	break;
	    	case 3:
	    		GPIOC->ODR&=(0b0111);
	    	 	break;
	    	}
			for(Xi=0;Xi<4;Xi++){
                int value_get=GPIOB->IDR&0xf;
                int is_pressed=(value_get>>Xi)&1;
                key_val=keypad_value[Yi][Xi];
                if(pressed_table[key_val]==1 && is_pressed==1) pressed_table[key_val]=0;
				if(is_pressed==0){ //key is pressed
					nothing_is_pressed=0;
					pressed_table[key_val]=1;
				}
			}
    	}



    	//sum_up

		out_sum=0;
		for(int i=0;i<16;i++){
			if(pressed_table[i]){
				out_sum+=i;
			}
		}

		//display
    	switch(out_sum){
    	case 0:
    		if(nothing_is_pressed)
    			display_clr(8);
    		else
    			display(out_sum,1);
    		break;
    	case 1 ... 9:
			display_clr(2);
    		display(out_sum,1);
    	    break;
    	case 10 ... 99:
    		display(out_sum,2);
    	 	break;
    	}


    	 //if no press, clear
		if(nothing_is_pressed){
				out_sum=0;
				for(int i=15;i>=0;i--)
					pressed_table[i]=0;
		}

    }
    return key_val;
}

int main(){
    GPIO_init();
    max7219_init();
    keypad_init();
    display_clr(8);
    keypad_scan();
    return 0;
}
*/


