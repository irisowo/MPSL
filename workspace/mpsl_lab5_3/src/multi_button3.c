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

//Horizontal
void keypad_init(){
    GPIOC->MODER   &= 0b11111111111111111111111100000000; //use PC0123 for Yi
    GPIOC->MODER   |= 0b00000000000000000000000001010101;
    GPIOC->PUPDR   &= 0b11111111111111111111111100000000; //clear
    GPIOC->PUPDR   |= 0b00000000000000000000000001010101; //pup
    GPIOC->OSPEEDR &= 0b11111111111111111111111100000000;
    GPIOC->OSPEEDR |= 0b00000000000000000000000001010101;
    GPIOC->ODR     |= 0b00000000000000000000000000001111;

    GPIOB->MODER   &= 0b11111111111111111111111100000000; //use PB 0123 for Xi
    GPIOB->PUPDR   &= 0b11111111111111111111111100000000; //clear
    GPIOB->PUPDR   |= 0b00000000000000000000000010101010; //pdown
}

//Straight
void keypad_init2(){
    GPIOB->MODER   &= 0b11111111111111111111111100000000;
    GPIOB->MODER   |= 0b00000000000000000000000001010101;
    GPIOB->PUPDR   &= 0b11111111111111111111111100000000;
    GPIOB->PUPDR   |= 0b00000000000000000000000001010101;
    GPIOB->OSPEEDR &= 0b11111111111111111111111100000000;
    GPIOB->OSPEEDR |= 0b00000000000000000000000001010101;
    GPIOB->ODR     |= 0b00000000000000000000000000001111;


    GPIOC->MODER   &= 0b11111111111111111111111100000000;
    GPIOC->PUPDR   &= 0b11111111111111111111111100000000;
    GPIOC->PUPDR   |= 0b00000000000000000000000010101010;
}



int display_clr(int num_bits){
	for(int i=1;i<=num_bits;i++){
		max7219_send(i,0xF);
	}
	return 0;
}


unsigned char keypad_scan(){

    //set pressed_table for straight direction
    bool pressed_table[16];
    memset(pressed_table,0,sizeof(bool)*16);

    // pressed_table2 for horizontal direction
    bool pressed_table2[16];
    memset(pressed_table2,0,sizeof(bool)*16);

    // var used in while()
    int Yi=0,Xi=0;
    int nothing_is_pressed=1;
    unsigned char key_val=-1;

    int first=-1;
    int second=-1;
    //delay 3s
    while(1){
        nothing_is_pressed=1;
        //---------------------------------------------------------------------//
        //Straight
        keypad_init2();
    	for(Yi=0;Yi<4;Yi++){
			GPIOB->ODR&=0;
			GPIOB->ODR|=(1<<Yi);
			for(Xi=0;Xi<4;Xi++){
                int value_get=GPIOC->IDR&0xf;
                int is_pressed=(value_get>>Xi)&1;
                key_val=keypad_value[Xi][Yi];
                if(pressed_table[key_val]==1 && !is_pressed) pressed_table[key_val]=0;
				if(is_pressed){ //key is pressed
					nothing_is_pressed=0;
					pressed_table[key_val]=1;
				}
			}
    	}

    	//Horizontal
        keypad_init();
     	for(Yi=0;Yi<4;Yi++){
 			GPIOC->ODR&=0;
 			GPIOC->ODR|=(1<<Yi);
 			for(Xi=0;Xi<4;Xi++){
                 int value_get=GPIOB->IDR&0xf;
                 int is_pressed=(value_get>>Xi)&1;
                 key_val=keypad_value[Yi][Xi];
                 if(pressed_table2[key_val]==1 && !is_pressed) pressed_table2[key_val]=0;
 				if(is_pressed){ //key is pressed
 					nothing_is_pressed=0;
 					pressed_table2[key_val]=1;

 				}
 			}
     	}
     	//---------------------------------------------------------------------//
     	//Sum up and choose the larger one
		int cnt=0, cnt2=0;
		int out_sum=0, out_sum2=0;
		for(int i=0;i<16;i++){
			if(pressed_table[i]){
				out_sum+=i;
				cnt++;
			}
			if(pressed_table2[i]){
				out_sum2+=i;
				cnt2++;
			}
		}

		out_sum=(out_sum>=out_sum2)?out_sum:out_sum2;
		cnt=(cnt>cnt2)?cnt:cnt2;


		//---------------------------------------------------------------------//
		//Select (1st,2nd) and print out
		if(cnt==1){
			for(int i=0;i<16;i++){
				if(pressed_table[i]){
					first=i;
				}
			}
		}
		else if(cnt==2){
			//select (1st, 2nd)
			if(cnt>cnt2){
				for(int i=0;i<16;i++){
					if(pressed_table[i]&& i!=first)
						second=i;
				}
			}
			else{
				for(int i=0;i<16;i++){
					if(pressed_table2[i]&& i!=first)
						second=i;
				}
			}
			//end of select(1st, 2nd)
			//---------------------------------------------------------------------//
			display_clr(8);
			if(first<=9){
				if(second<=9){
					if(out_sum<=9){
						max7219_send(1,out_sum);//out_sum
						max7219_send(2,0xF);//blank
						max7219_send(3,second);
						max7219_send(4,0xF);//blank
						max7219_send(5,first);
					}//(f=1,s=1,o=1)
					else{
						max7219_send(1,out_sum%10);//out_sum
						max7219_send(2,out_sum/10);//out_sum
						max7219_send(3,0xF);//blank
						max7219_send(4,second);
						max7219_send(5,0xF);//blank
						max7219_send(6,first);
					}//(f=1,s=1,o=2)
				}
				else{
					max7219_send(1,out_sum%10);//out_sum
					max7219_send(2,out_sum/10);//out_sum
					max7219_send(3,0xF);//blank
					max7219_send(4,second%10);
					max7219_send(5,second/10);
					max7219_send(6,0xF);//blank
					max7219_send(7,first);
				}//(f=1,s=2,o=2)
			}
			//---------------------------------------------------------------------//
			else{
				max7219_send(1,out_sum%10);//out_sum
				max7219_send(2,out_sum/10);//out_sum
				max7219_send(3,0xF);//blank
				if(second<=9){
					max7219_send(4,second);
					max7219_send(5,0xF);//blank
					max7219_send(6,first%10);
					max7219_send(7,first/10);
				}//(f=2,s=1,o=2)
				else{
					max7219_send(4,second%10);
					max7219_send(5,second/10);
					max7219_send(6,0xF);//blank
					max7219_send(7,first%10);
					max7219_send(8,first/10);
				}//(f=2,s=2,o=2)
			}
		}// (cnt==2) end
		//---------------------------------------------------------------------//
    	 //if no press, clear
		if(nothing_is_pressed ){
			display_clr(8);
			for(int i=16;i>=0;i--){
				pressed_table[i]=0;
			    pressed_table2[i]=0;
			}
			first=-1;
			second=-1;
		}
		//Question :  int tmp=1000; while(tmp--);

    }//end of while()
    return key_val;
}

int main(){
	//initiate
    GPIO_init();
    max7219_init();
    RCC->AHB2ENR   |= 0b00000000000000000000000000000111;
    //clear & run
    display_clr(8);
    keypad_scan();
    return 0;
}
*/

