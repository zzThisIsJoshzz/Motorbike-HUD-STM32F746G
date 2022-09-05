/*

 File        		: rotary_encoder.h

 Primary Author : Joshua Crafton

 Description 		: The header file with functions that read the rotary encoders.

*/

#ifndef __ROTARY_ENCODER_H
#define __ROTARY_ENCODER_H

#include "main.h"



#define GPIO_PORT_RIGHT GPIOC
#define GPIO_PORT_LEFT GPIOG
#define OUTA_PIN_RIGHT GPIO_PIN_6
#define OUTB_PIN_RIGHT GPIO_PIN_7
#define OUTA_PIN_LEFT GPIO_PIN_7
#define OUTB_PIN_LEFT GPIO_PIN_6

uint16_t checkEncoderLeft(uint16_t);
uint16_t checkEncoderRight(uint16_t);




uint16_t checkEncoderLeft(uint16_t counter){
	
  	if (HAL_GPIO_ReadPin(GPIO_PORT_LEFT, OUTA_PIN_LEFT) == GPIO_PIN_RESET)  // If the OUTA is RESET
		{
			if (HAL_GPIO_ReadPin(GPIO_PORT_LEFT, OUTB_PIN_LEFT) == GPIO_PIN_RESET)  // If OUTB is also reset... CCK
			{
				while (HAL_GPIO_ReadPin(GPIO_PORT_LEFT, OUTB_PIN_LEFT) == GPIO_PIN_RESET);  // wait for the OUTB to go high
				counter--;
				while (HAL_GPIO_ReadPin(GPIO_PORT_LEFT, OUTA_PIN_LEFT) == GPIO_PIN_RESET);  // wait for the OUTA to go high
				HAL_Delay (10);  // wait for some more time
			}

			else if (HAL_GPIO_ReadPin(GPIO_PORT_LEFT, OUTB_PIN_LEFT) == GPIO_PIN_SET)  // If OUTB is also set
			{
				while (HAL_GPIO_ReadPin(GPIO_PORT_LEFT, OUTB_PIN_LEFT) == GPIO_PIN_SET);  // wait for the OUTB to go LOW.. CK
				counter++;
				while (HAL_GPIO_ReadPin(GPIO_PORT_LEFT, OUTA_PIN_LEFT) == GPIO_PIN_RESET);  // wait for the OUTA to go high
				while (HAL_GPIO_ReadPin(GPIO_PORT_LEFT, OUTB_PIN_LEFT) == GPIO_PIN_RESET);  // wait for the OUTB to go high
				HAL_Delay (10);  // wait for some more time
			}
			
			if (counter<0) counter = 0;
			if (counter>30) counter = 30;
			
			

	}
		
 	return counter;
}

uint16_t checkEncoderRight(uint16_t counter){
	if (HAL_GPIO_ReadPin(GPIO_PORT_RIGHT, OUTA_PIN_RIGHT) == GPIO_PIN_RESET)  // If the OUTA is RESET
		{
			if (HAL_GPIO_ReadPin(GPIO_PORT_RIGHT, OUTB_PIN_RIGHT) == GPIO_PIN_RESET)  // If OUTB is also reset... CCK
			{
				while (HAL_GPIO_ReadPin(GPIO_PORT_RIGHT, OUTB_PIN_RIGHT) == GPIO_PIN_RESET);  // wait for the OUTB to go high
				counter--;
				while (HAL_GPIO_ReadPin(GPIO_PORT_RIGHT, OUTA_PIN_RIGHT) == GPIO_PIN_RESET);  // wait for the OUTA to go high
				HAL_Delay (10);  // wait for some more time
			}

			else if (HAL_GPIO_ReadPin(GPIO_PORT_RIGHT, OUTB_PIN_RIGHT) == GPIO_PIN_SET)  // If OUTB is also set
			{
				while (HAL_GPIO_ReadPin(GPIO_PORT_RIGHT, OUTB_PIN_RIGHT) == GPIO_PIN_SET);  // wait for the OUTB to go LOW.. CK
				counter++;
				while (HAL_GPIO_ReadPin(GPIO_PORT_RIGHT, OUTA_PIN_RIGHT) == GPIO_PIN_RESET);  // wait for the OUTA to go high
				while (HAL_GPIO_ReadPin(GPIO_PORT_RIGHT, OUTB_PIN_RIGHT) == GPIO_PIN_RESET);  // wait for the OUTB to go high
				HAL_Delay (10);  // wait for some more time
			}
			
			

			if (counter<0) counter = 0;
			if (counter>30) counter = 30;
			
			
		}
	
		return counter;
}



#endif
