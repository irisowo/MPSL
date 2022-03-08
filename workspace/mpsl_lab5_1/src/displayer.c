#include <stdio.h>
#include <stdlib.h>

#define ID_LEN 7
extern void GPIO_init();
extern void max7219_init();
extern void max7219_send(unsigned char address, unsigned char data);

int display(int data, int num_bits){

	int i=0;
	//clear
	for (; i<=8; i++)
		max7219_send(i, 0xF);
	//send
	for (i=1; i<=num_bits; i++){
		if(i==num_bits){ //7th
			max7219_send(i, 0);
		}
		else{ //1~6th bits
			max7219_send(i, data % 10);
			data /= 10;
		}
	}

	return (data > 99999999 || data < -9999999) ? -1 : 0;
}

int main(){
	int student_id = 616086;
	GPIO_init();
	max7219_init();
	display(student_id, 7);
}
