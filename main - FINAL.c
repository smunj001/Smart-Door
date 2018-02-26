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
#include <util/delay.h>

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "usart_ATmega1284.h"

/*START STATE MACHINES AND CODE FOR OUTPUT
	Stepper Motor
	LED Lights
	IR SENSOR BLINKER
*/

unsigned char getINPUT = 0;
unsigned char IRFlag = 0;
unsigned char getPIN = 0;
unsigned short phaseVal = (90/5.625)/ 2;
unsigned char cnt = 0;
unsigned char port_C_value = 0x00;


/////////////////////USART TEST////////////////////
enum USART_States{Start, receive} state_usart;
	
void USART_Tick(){
	switch (state_usart){
		
		case Start:
			state_usart = receive;
			break;
		
		case receive:
			state_usart = receive;
			break;
		
		default:
			break;
	}
	
	switch (state_usart){
		
		case Start:
			break;
			
		case receive:
			if(USART_HasReceived(0))
			{
				port_C_value = USART_Receive(0);
				USART_Flush(0);
				/*
				if (port_C_value == 0x0A)
				{
					PORTC = 0x01;
				}
				*/
			
				if( port_C_value == 0x0A ) 
				{
					PORTC = 0x01;	
				}
				else if( port_C_value == 0x0B )
				{
					PORTC = 0x02;
				}				
				else if( port_C_value == 0x0C )
				{
					PORTC = 0x04;
				}
				else
				{
					PORTC = 0x00;
				}
				
				_delay_ms(10000);
				PORTC = 0x00;
			}
			break;
		
		default:
			break;
	}
}

void USART_Task()
{
	for(;;)
	{
		USART_Tick();
		vTaskDelay(50);
	}
}
//////////////End USART TEST///////////////////////

//////////////////IR SENSOR////////////////////////
enum SM_IR {Init, blink} states_led;
	
void SM_IR_Tick()
{
	//Actions
	getINPUT = ~(PINB) & 0x01;
	
	switch(states_led)
	{
		case Init:
			break;
		
		case blink:
			break;
		
		default:
			break;
	}
	
	//Transitions
	switch(states_led)
	{
		case Init:
			states_led = blink;
			break;
		
		case blink:
			if (!getINPUT) //Read the value of getPin and if non-zero then set a flag for light
			{
				IRFlag = 0;
			}
			else
			{
				IRFlag = 1;
			}
			break;
		
		default:
			break;
	}
}

void SM_IR_Task()
{
	for(;;)
	{
		SM_IR_Task();
		vTaskDelay(100);
	}
}

//////////////////END IR SENSOR////////////////////////

/////////////////LED Start///////////////////////
enum SM_Lights {INIT, OFF, ON} state;

void SM_Lights_Tick(){
	
	//Actions
	
	switch(state)
	{
		case INIT:
			break;
			
		case OFF:
			PORTC = 0x00;
			break;
		
		case ON:
			PORTC = 0x10;
			break;
		
		default:
			break;
	}
	
	//Transitions
	switch(state)
	{
		case INIT:
			state = OFF;
			break;
			
		case OFF:
		
			if (IRFlag == 1) //check our lightFlag
			{
				state = ON;
			}
			else
			{
				state = OFF; //Stay off
			}
			
			break;
			
		case ON:
		
			if (IRFlag == 0)
			{
				state = OFF;
			}
			else
			{
				
				state = ON; //Stay ON
			}
			
			break;
		
		default:
			break;
	}
}

void SM_Lights_Task()
{
	for(;;)
	{
		SM_Lights_Tick();
		vTaskDelay(100);
	}
}

//////////////////LED End////////////////////////

//////////////////MOTOR START///////////////////
enum SM_Motor {Motor_init, A, AB, B, BC, C, CD, D, DA} state_motor;

void SM_Motor_Tick(){
	//Actions
	switch(state_motor)
	{
		case Motor_init:
			break;
			
		case A:
			PORTA = 0x01;
			break;
		
		case AB:
			PORTA = 0x03;
			break;
		
		case B:
			PORTA = 0x02;
			break;
		
		case BC:
			PORTA = 0x06;
			break;
		
		case C:
			PORTA = 0x04;
			break;
		
		case CD:
			PORTA = 0x0C;
			break;
		
		case D:
			PORTA = 0x08;
			break;
		
		case DA:
			PORTA = 0x09;
			break;
			
		default:
			break;
	}
	
	//Transitions
	switch(state_motor)
	{
		case A:
		if (IRFlag == 1)
		{
			if ( phaseVal != cnt )
			{
				cnt++;
				state_motor = AB;
			}
			cnt = 0; //reset
		}
		else
		{
			state_motor = A;
		}
		break;
		
		case AB:
		if (IRFlag == 1)
		{
			if ( phaseVal != cnt )
			{
				cnt++;
				state_motor = B;
			}
			cnt = 0; //reset
		}
		else
		{
			state_motor = AB;
		}
		
		break;
		
		case B:
		if (IRFlag == 1)
		{
			if ( phaseVal != cnt )
			{
				cnt++;
				state_motor = BC;
			}
			cnt = 0; //reset
		}
		else
		{
			state_motor = B;
		}
		break;
		
		case BC:
		if (IRFlag == 1)
		{
			if ( phaseVal != cnt )
			{
				cnt++;
				state_motor = C;
			}
			cnt = 0; //reset
		}
		else
		{
			state_motor = BC;
		}
		break;
		
		case C:
		if (IRFlag == 1)
		{
			if ( phaseVal != cnt )
			{
				cnt++;
				state_motor = CD;
			}
			cnt = 0; //reset
		}
		else
		{
			state_motor = C;
		}
		
		break;
		
		case CD:
		if (IRFlag == 1)
		{
			if ( phaseVal != cnt )
			{
				cnt++;
				state_motor = D;
			}
			cnt = 0; //reset
		}
		else
		{
			state_motor = CD;
		}
		
		break;
		
		case D:
		if (IRFlag == 1)
		{
			if ( phaseVal != cnt )
			{
				cnt++;
				state_motor = DA;
			}
			cnt = 0; //reset
		}
		else
		{
			state_motor = D;
		}
		
		break;
		
		case DA:
		if (IRFlag == 1)
		{
			if ( phaseVal != cnt )
			{
				cnt++;
				state_motor = A;
			}
			cnt = 0; //reset
		}
		else
		{
			state_motor = DA;
		}
		
		break;
		
		default:
			break;
	}
}

void SM_Motor_Task()
{
	for(;;)
	{
		SM_Motor_Tick();
		vTaskDelay(3);
	}
}

//////////////////MOTOR END///////////////////

//Schedule the Tasks

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(SM_IR_Task, (signed portCHAR *)"SM_LED_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(SM_Lights_Task, (signed portCHAR *)"SM_Lights_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(SM_Motor_Task, (signed portCHAR *)"SM_Motor_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(USART_Task, (signed portCHAR *)"USART_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL);
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	
	initUSART(0);
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}