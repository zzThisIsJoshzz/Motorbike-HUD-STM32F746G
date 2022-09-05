/*

 File        		: sensor_ui.h

 Primary Author : Manuel Dogbatse

 Description 		: The header file that defines all the GLCD display and
									touchscreen functionalities.

*/

#ifndef __SENSOR_UI_H
#define __SENSOR_UI_H

#include "main.h"

void wait(int delay)
{
	unsigned int i;
	
	// wait for specified time
	for(i = delay; i > 0; i--);
}

// Gets a 4-digit number and returns an array of each digit
int* getDigits(int num)
{
	static int digits[4];
	int i;
	for (i = 0; i < 4; i++)
	{
		// Mod operator insures the exact digit is recorded
		digits[i] = num % 10;
		num = num / 10;
	}
	return digits;
}

// Function to remove the need to change colours in separate command
void drawRectangle(int x, int y, int dx, int dy, uint32_t colour)
{
	GLCD_SetForegroundColor(colour);
	GLCD_DrawRectangle(x, y, dx, dy);
	GLCD_DrawPixel(x+dx, y+dy);
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
}

// Function to remove the need to change foreground and background colours in separate command
void drawString(int x, int y, char buffer[128], uint32_t foreColour, uint32_t backColour)
{
	GLCD_SetForegroundColor(foreColour);
	GLCD_SetBackgroundColor(backColour);
	GLCD_DrawString(x, y, buffer);
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
}

// Draws the foundation of a circle using the center value and radius values
void drawCircleFoundation(int centerX, int centerY, int distX, int distY)
{
	GLCD_DrawPixel(centerX - distX, centerY - distY);
	GLCD_DrawPixel(centerX + distX, centerY - distY);
	GLCD_DrawPixel(centerX - distX, centerY + distY);
	GLCD_DrawPixel(centerX + distX, centerY + distY);
	GLCD_DrawPixel(centerX - distY, centerY - distX);
	GLCD_DrawPixel(centerX + distY, centerY - distX);
	GLCD_DrawPixel(centerX - distY, centerY + distX);
	GLCD_DrawPixel(centerX + distY, centerY + distX);
}

// Drawing circles for the temperature and gyrometer displays
void drawCircle(int centerX, int centerY, int radius, uint32_t colour)
{
	// Drawing circle using Bresenham's circle algorithm
// Reference = https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/
	int x = 0, y = radius, dp = 3 - (2 * radius);
	GLCD_SetForegroundColor(colour);
	drawCircleFoundation(centerX, centerY, x, y);
	while (y >= x)
	{
		// For every pixel drawn in 'drawCircleFoundation', 8 pixels will be drawn
		x++;
		
		// Checks decision parameter and correspondingly updates d, x and y
		if (dp > 0)
		{
			y--;
			dp = dp + (4 * (x - y)) + 10;
		}
		else
		{
			dp = dp + (4 * x) + 6;
		}
			drawCircleFoundation(centerX, centerY, x, y);
	}
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
}

// Drawing a diagonal line for the chevrons and gyrometer arrow
void drawDiagonalLineLow(int x0, int y0, int x1, int y1)
{
	// Drawing a line using the Bresenham's line algorithm
	// Reference: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	int dx, dy, yi, D, x, y;
	dx = x1 - x0;
	dy = y1 - y0;
	yi = 1;
	if (dy < 0)
	{
		yi = -1;
		dy = -dy;
	}
	
	D = (2 * dy) - dx;
	y = y0;

	for (x = x0; x < x1; x++)
	{
		GLCD_DrawPixel(x, y);
		if (D > 0)
		{
			y = y + yi;
			D = D + (2 * (dy - dx));
		}
		else
		{
			D = D + 2*dy;
		}
	}
}

void drawDiagonalLineHigh(int x0, int y0, int x1, int y1)
{
	// Drawing a line using the Bresenham's line algorithm
	// Reference: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	int dx, dy, xi, D, x, y;
	dx = x1 - x0;
	dy = y1 - y0;
	xi = 1;
	if (dx < 0)
	{
		xi = -1;
		dx = -dx;
	}
	D = (2 * dx) - dy;
	x = x0;

	for (y = y0; y < y1; y++)
	{
		GLCD_DrawPixel(x, y);
		if (D > 0)
		{
			x = x + xi;
			D = D + (2 * (dx - dy));
		}
		else
		{
			D = D + 2*dx;
		}
	}
}
				
void drawDiagonalLine(int x0, int y0, int x1, int y1, uint32_t colour)
{
	GLCD_SetForegroundColor(colour);
	// These statements insure that the correct variation of the Bresenham's
	// line algorithm is used for given starting and ending points
	if (abs(y1 - y0) < abs(x1 - x0))
	{
		if (x0 > x1)
		{
			drawDiagonalLineLow(x1, y1, x0, y0);
		}
		else
		{
			drawDiagonalLineLow(x0, y0, x1, y1);
		}
	}
	else
	{
		if (y0 > y1)
		{
			drawDiagonalLineHigh(x1, y1, x0, y0);
		}
		else
		{
			drawDiagonalLineHigh(x0, y0, x1, y1);
		}
	}
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
}

// Draws a chevron relative to the starting x value.
// This is used for the ultrasound sensors
void drawChevron(int x, bool isReverse, uint32_t colour)
{
	GLCD_SetForegroundColor(colour);
	// Ensures that the chevrons are drawn in the correct direction
	if (isReverse)
	{
		drawDiagonalLine(478-x, 136, 471-x, 0, colour);
		drawDiagonalLine(478-x, 136, 471-x, 272, colour);
		GLCD_DrawHLine(461-x, 0, 10);
		GLCD_DrawHLine(461-x, 270, 10);
		drawDiagonalLine(468-x, 136, 461-x, 0, colour);
		drawDiagonalLine(468-x, 136, 461-x, 272, colour);
	}
	else
	{
		drawDiagonalLine(x, 136, x+7, 0, colour);
		drawDiagonalLine(x, 136, x+7, 272, colour);
		GLCD_DrawHLine(x+7, 1, 10);
		GLCD_DrawHLine(x+7, 271, 10);
		drawDiagonalLine(x+10, 136, x+17, 0, colour);
		drawDiagonalLine(x+10, 136, x+17, 272, colour);
	}
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
}

// Fill the background a given colour
void fillBackground(uint32_t colour)
{
	int i, j;
	GLCD_SetForegroundColor(colour);
	for (i = 0; i < 479; i++)
	{
		for (j = 0; j < 271; j++)
		{
			GLCD_DrawPixel(i, j);
		}
	}
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
}

// Fill a rectangle a given colour
void fillRectangle(int x, int y, int dx, int dy, uint32_t colour)
{
	int i, j;
	GLCD_SetForegroundColor(colour);
	for (i = x + 1; i < x + dx; i++)
	{
		for (j = y + 1; j < y + dy; j++)
		{
			GLCD_DrawPixel(i, j);
		}
	}
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
}

// Fill a chevron with a given colour
void fillChevron(int x, bool isReverse, uint32_t colour)
{
	int i;
	GLCD_SetForegroundColor(colour);
	if (isReverse)
	{
		for(i = 1; i < 6; i++)
		{
			drawDiagonalLine(478-x-i, 136, 471-x-i, 0, colour);
			drawDiagonalLine(478-x-i, 136, 471-x-i, 272, colour);
			drawDiagonalLine(468-x+i, 136, 461-x+i, 0, colour);
			drawDiagonalLine(468-x+i, 136, 461-x+i, 272, colour);
		}
	}
	else
	{
		for(i = 1; i < 6; i++)
		{
			drawDiagonalLine(x+i, 136, x+7+i, 0, colour);
			drawDiagonalLine(x+i, 136, x+7+i, 272, colour);
			drawDiagonalLine(x+10-i, 136, x+17-i, 0, colour);
			drawDiagonalLine(x+10-i, 136, x+17-i, 272, colour);
		}
	}
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
}

// Function for drawing the display for the colour palettes
void drawPalette(int x, int y, int d)
{
	int f = (d+1)/2;
	
	drawRectangle(x, y, d, d, GLCD_COLOR_BLACK);
	GLCD_DrawHLine(x, y+f, d);
	GLCD_DrawVLine(x+f, y, d);
}

// Filling the colour palettes with their respective colours
void fillPalette(int x, int y, int d, uint32_t colourPalette)
{
	int f = (d+1)/2;
	
	fillRectangle(x, y, f, f, colourPalette);
	fillRectangle(x+f, y, f-1, f, GLCD_COLOR_BLACK);
	fillRectangle(x, y+f, f, f-1, GLCD_COLOR_BLACK);
	fillRectangle(x+f, y+f, f-1, f-1, colourPalette);
}

// Highlights the chosen buttons in the settings menu
void highlightButton(int x, int y, int dx, int dy, uint32_t colour)
{
	int i;
	
	for (i = 0; i < 4; i++)
	{
		drawRectangle(x-5-i, y-5-i, dx+10+(2*i), dy+10+(2*i), colour);
	}
}

// Function for highlighting temperature and distance units in the settings screen
// without having to repeat the if statement
void highlightTempUnit(int tempUnit)
{
	if (tempUnit)
	{
		highlightButton(25, 135, 70, 30, 0x033F);
		highlightButton(131, 135, 70, 30, GLCD_COLOR_WHITE);
	}
	else
	{
		highlightButton(131, 135, 70, 30, 0x033F);
		highlightButton(25, 135, 70, 30, GLCD_COLOR_WHITE);
	}
}

void highlightDistUnit(int distUnit)
{
	if (distUnit)
	{
		highlightButton(25, 215, 70, 30, 0x033F);
		highlightButton(131, 215, 70, 30, GLCD_COLOR_WHITE);
	}
	else
	{
		highlightButton(131, 215, 70, 30, 0x033F);
		highlightButton(25, 215, 70, 30, GLCD_COLOR_WHITE);
	}
}

void highlightColour(int colourScheme)
{	
	//Dependent on colour1 the values will be removed then replace with the required box
	if (colourScheme == 0)
	{
		highlightButton(295, 120, 44, 44, 0x033F); //top left
		highlightButton(295, 208, 44, 44, GLCD_COLOR_WHITE); //bot left
		highlightButton(396, 120, 44, 44, GLCD_COLOR_WHITE); //top right
		highlightButton(396, 208, 44, 44, GLCD_COLOR_WHITE); //bot right
		
	}
	else if(colourScheme == 1)
	{
		highlightButton(295, 120, 44, 44, GLCD_COLOR_WHITE); //top left
		highlightButton(295, 208, 44, 44, GLCD_COLOR_WHITE); //bot left
		highlightButton(396, 120, 44, 44, 0x033F); //top right
		highlightButton(396, 208, 44, 44, GLCD_COLOR_WHITE); //bot right
	}
	else if(colourScheme == 2)
	{
		highlightButton(295, 120, 44, 44, GLCD_COLOR_WHITE); //top left
		highlightButton(295, 208, 44, 44, 0x033F); //bot left
		highlightButton(396, 120, 44, 44, GLCD_COLOR_WHITE); //top right
		highlightButton(396, 208, 44, 44, GLCD_COLOR_WHITE); //bot right
	}
	else if(colourScheme == 3)
	{
		highlightButton(295, 120, 44, 44, GLCD_COLOR_WHITE); //top left
		highlightButton(295, 208, 44, 44, GLCD_COLOR_WHITE); //bot left
		highlightButton(396, 120, 44, 44, GLCD_COLOR_WHITE); //top right
		highlightButton(396, 208, 44, 44, 0x033F); //bot right
	}
}

int checkCoordsMain(int x, int y){
		// Location of the setting button
		if (x > 320 && x < 380 && y > 5 && y < 35)
		{
			// Returns one to change menu screens
			return 1;
		}
		return 0;
}

int checkCoordsSettings(int x, int y)
{
		// Location of the back button
		if (x > 388 && x < 458 && y > 20 && y < 50)
		{
			// Returns one to change menu screens
			return -1;
		}
		
		// temp and distance unit settings
		if (x >= 25 && x <= 135)
		{
			if (y >= 135 && y <= 165)
			{
				highlightTempUnit(1);
				return 1;
			}
			if (y >= 195 && y <= 225)
			{
				highlightDistUnit(1);
				return 11;
			}
		}
		else if(x >= 131 && x <= 200)
		{
			if (y >= 135 && y <= 165)
			{
				highlightTempUnit(0);
				return 0;
			}
			if (y >= 215 && y <= 245)
			{
				highlightDistUnit(0);
				return 10;
			}
		}
		
		//-----Colours setting boxes
		
		else if(x >= 295 && x <= 340)
		{
			if (y >= 120 && y <= 165)
			{
				highlightColour(0);
				return 20;
			}
			if (y >= 208 && y <= 253)
			{
				highlightColour(2);
				return 22;
			}
		}
		else if(x >= 396 && x <= 441)
		{
			if (y >= 120 && y <= 165)
			{
				highlightColour(1);
				return 21;
			}
			if (y >= 208 && y <= 253)
			{
				highlightColour(3);
				return 23;
			}
		}
	// '0' is used to represent 'null'
	return 0;
}

void displayDisChevronsLeft(int numChev, uint32_t colour1, uint32_t colour2)
{
	//Remove all current chevrons
	drawChevron(68, false, colour1);
	fillChevron(68, false, colour1);
	drawChevron(51, false, colour1);
	fillChevron(51, false, colour1);
	drawChevron(34, false, colour1);
	fillChevron(34, false, colour1);
	drawChevron(17, false, colour1);
	fillChevron(17, false, colour1);
	drawChevron(0, false, colour1);
	fillChevron(0, false, colour1);
	
	// Is used to place cheverons.
	// No breaks are needed because if 1 is need then it goes straight to the bottom
	// and if 5 are needed then it will droip through each switch case and add another chevron.
	switch(numChev)
	{
		
		case 5:
			drawChevron(68, false, colour2);
			fillChevron(68, false, colour2);
			
		case 4:
				drawChevron(51, false, colour2);
				fillChevron(51, false, colour2);
			
		case 3:
				drawChevron(34, false, colour2);
				fillChevron(34, false, colour2);
			
		case 2:
				drawChevron(17, false, colour2);
				fillChevron(17, false, colour2);
			
		case 1:
				drawChevron(0, false, colour2);
				fillChevron(0, false, colour2);
			
		default:
			break;
	}
}

void displayDisChevronsRight(int numChev, uint32_t colour1, uint32_t colour2)
{
	//Remove all current chevrons
	drawChevron(68, true, colour1);
	fillChevron(68, true, colour1);
	drawChevron(51, true, colour1);
	fillChevron(51, true, colour1);
	drawChevron(34, true, colour1);
	fillChevron(34, true, colour1);
	drawChevron(17, true, colour1);
	fillChevron(17, true, colour1);
	drawChevron(0, true, colour1);
	fillChevron(0, true, colour1);
	// Is used to place cheverons.
	// No breaks are needed because if 1 is need then it goes straight to the bottom
	// and if 5 are needed then it will droip through each switch case and add another chevron.
		switch(numChev)
	{
		case 5:
			drawChevron(68, true, colour2);
			fillChevron(68, true, colour2);
			
		case 4:
				drawChevron(51, true, colour2);
				fillChevron(51, true, colour2);
			
		case 3:
				drawChevron(34, true, colour2);
				fillChevron(34, true, colour2);
			
		case 2:
				drawChevron(17, true, colour2);
				fillChevron(17, true, colour2);
			
		case 1:
				drawChevron(0, true, colour2);
				fillChevron(0, true, colour2);
			
		default:
			break;
	}
}

#endif