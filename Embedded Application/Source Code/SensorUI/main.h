/*

 File        		: main.h

 Primary Author : Joshua Crafton

 Description 		: The header file that links all the separate source and header
									files in this project.

*/

#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "GLCD_Config.h"
#include "Board_GLCD.h"
#include "Board_Touch.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "rotary_encoder.h"
#include "sensor_ui.h"

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

#endif /* __MAIN_H */