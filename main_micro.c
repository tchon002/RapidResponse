#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "joystick.h"
#include "PWM.h"
#include "usart_ATmega1284.h"
#include "bit.h"
//#include "sonar.h"
#define button_go (~PINA & 0x0C)

//Assistance from http://stackoverflow.com/questions/28896680/interfacing-ultrasonic-range-sensor-hcsr-04-with-avr-atmega-32
//Took code from source and reworked it to fit my atmega and my project
#include<util/delay.h>

enum Car_State{input_check, move_car, wall_go, autodrive}car_state;
unsigned char temp;
unsigned char temp_check;
void Car_Init(){
	motors_init();
}
void Car_Tick(){
	switch(car_state){
		case input_check:
		if(USART_HasReceived(1)){
			USART_Flush(1);
			M_off();
			car_state = wall_go;
			PORTC = 0x04;
			delay_ms(100);
		}
		else if(USART_HasReceived(0)){
			temp = USART_Receive(0);
			car_state = move_car;
			PORTC = 0x08;
		}
		else{
			car_state = input_check;
			
		}
		break;
		case move_car:
		if(temp == 0x01){
			M_off();
		}
		//SLeft---Left
		else if(temp == 0x02){
			M1_forward(100);
			M2_reverse(100);
		}
		else if(temp == 0x03){
			M1_forward(20);
			M2_reverse(20);
		}
		//SRight---Right
		else if(temp == 0x04){
			M1_reverse(100);
			M2_forward(100);
		}
		else if(temp == 0x05){
			M1_reverse(20);
			M2_forward(20);
		}
		//SUp---Up
		else if(temp == 0x06){
			M1_forward(100);
			M2_forward(100);
		}
		else if(temp == 0x07){
			M1_forward(20);
			M2_forward(20);
		}
		//SDown---Down
		else if(temp == 0x08){
			M1_reverse(100);
			M2_reverse(100);
		}
		else if(temp == 0x09){
			M1_reverse(20);
			M2_reverse(20);
		}
		car_state = input_check;
		if(temp == 0x10){
			car_state = autodrive;
		}
		break;
		case wall_go:
		if(!USART_HasReceived(1)){
			car_state = input_check;
			USART_Flush(1);
			delay_ms(100);
			break;
		}
		else{
			if(USART_HasReceived(0)){
				temp = USART_Receive(0);
			}
			if(temp == 0x01){
				M_off();
			}
			//SLeft---Left
			else if(temp == 0x02){
				M1_forward(100);
				M2_reverse(100);
			}
			else if(temp == 0x03){
				M1_forward(20);
				M2_reverse(20);
			}
			//SRight---Right
			else if(temp == 0x04){
				M1_reverse(100);
				M2_forward(100);
			}
			else if(temp == 0x05){
				M1_reverse(20);
				M2_forward(20);
			}
			//SUp---Up
			else if(temp == 0x06){
				M_off();
			}
			else if(temp == 0x07){
				M_off();
			}
			//SDown---Down
			else if(temp == 0x08){
				M1_reverse(100);
				M2_reverse(100);
			}
			else if(temp == 0x09){
				M1_reverse(20);
				M2_reverse(20);
			}
			car_state = wall_go;
			USART_Flush(1);
		}
		break;
		case autodrive:
		if(USART_HasReceived(0)){
			temp = USART_Receive(0);
		}
		if(temp == 0x10){
			car_state = input_check;
			break;
		}
		else{
			if(USART_HasReceived(1)){
				USART_Flush(1);
				for(unsigned long i = 0; i < 20000; i++){
					M1_forward(100);
					M2_reverse(100);
				}
				delay_ms(20);
			}
			else{
				M1_forward(100);
				M2_forward(100);
				USART_Flush(1);
			}
			car_state = autodrive;
		}
		break;
	}
}
void Car_Task()
{
	Car_Init();
	for(;;)
	{
		Car_Tick();
		vTaskDelay(5);
	}
}

enum Joystick_State{joystick_start,joystick_input}joystick_state;
void Joystick_Init(){
}
void Joystick_Tick_Send(){
	switch(joystick_state){
		case joystick_start:
		joystick_state = joystick_input;
		break;
		case joystick_input:
		joystickSample();
		if((joyPos == None) || (button_go == 0x00)){
			USART_Send(0x01,0);
		}
		if((joyPos == SLeft) && (button_go == 0x04)){
			USART_Send(0x02,0);
		}
		if((joyPos == Left) && (button_go == 0x04)){
			USART_Send(0x03,0);
		}
		if((joyPos == SRight) && (button_go == 0x04)){
			USART_Send(0x04,0);
		}
		if((joyPos == Right) && (button_go == 0x04)){
			USART_Send(0x05,0);
		}
		if((joyPos == SUp) && (button_go == 0x04)){
			USART_Send(0x06,0);
		}
		if((joyPos == Up) && (button_go == 0x04)){
			USART_Send(0x07,0);
		}
		if((joyPos == SDown) && (button_go == 0x04)){
			USART_Send(0x08,0);
		}
		if((joyPos == Down) && (button_go == 0x04)){
			USART_Send(0x09,0);
		}
		if(button_go == 0x08){
			USART_Send(0x10,0);
		}
		joystick_state = joystick_input;
		break;
	}
	
}
void Joystick_Task_Send(){
	for(;;){
		Joystick_Tick_Send();
		vTaskDelay(5);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(Car_Task, (signed portCHAR *)"SM", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(Joystick_Task_Send, (signed portCHAR *)"SM", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	//xTaskCreate(Sonar_Task, (signed portCHAR *)"SM", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}
int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	//led
	DDRB = 0xFF; PORTB = 0x00;
	//last 2-bit led
	DDRC = 0xFD; PORTC = 0x02;
	DDRD = 0xFF; PORTD = 0x00;
	//PWM_on();
	motors_init();
	joystickInit();
	initUSART(0);
	initUSART(1);
	//Sonar_Init();

	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}