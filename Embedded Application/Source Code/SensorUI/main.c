/*

 File        		: main.c

 Primary Author : Joshua Crafton

 Description 		: The main c file that configures the timers, the pins, and the GPIO
									outputs. Then uses all the functions to display a screen showing
									sensor information for the motorcyle mount system.
*/

// Header file for external files
#include "main.h"

#define wait_delay HAL_Delay

//--------MPU Registers--------------------
//
#define MPU6050_ADDR (0x68 << 1) // 0xD0


#define SMPLRT_DIV_REG 0x19
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41
#define GYRO_XOUT_H_REG 0x43
#define PWR_MGMT_1_REG 0x6B
#define WHO_AM_I_REG 0x75

//-----------------------------------------

#ifdef __RTX
extern uint32_t os_time;
uint32_t HAL_GetTick(void) {
	return os_time;
}
#endif

/**
* Global Variables
*/
TIM_HandleTypeDef htim2;
I2C_HandleTypeDef hi2c1;

uint16_t colourScheme, temperature, tempUnit, distUnit; // Variables to change UI related units/colours
int currentDistLeft = 0; // For remembering how many chevrons are currently appearing
int currentDistRight = 0;
uint16_t distLeft = 0; //Actual distance mesurement
uint16_t distRight = 0;


// Pi to 21 significant figures
const float M_PI = 3.14159265358979323846;

// Accelleromerter Raw Values
int16_t Accel_X_RAW = 0;
int16_t Accel_Y_RAW = 0;
int16_t Accel_Z_RAW = 0;
//Gyroscope Raw Values
int16_t Gyro_X_RAW = 0;
int16_t Gyro_Y_RAW = 0;
int16_t Gyro_Z_RAW = 0;
//Values after raw to real converstion
float Ax, Ay, Az, Gx, Gy, Gz;
float pitch = 0;
float roll = 0;
float yaw = 0;
// Init position of lean pointer head
int circX = 240;
int circY = 142;

uint32_t colour1;//Background usually
uint32_t colour2;//Foreground usually
uint32_t colour3;//Spare


/**
* Prototype Functions
*/

void SystemClock_Config(void);
static void TIM2_Init(void);
static void GPIO_Init(void);
void Error_Handler(void);
static void I2C1_Init(void);

// Buzzer/LED control
void turnOnBuzzer(){
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET);
}

void turnOffBuzzer(){
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET);
}


//------------------------START MPU CODE-------------------------------------
// Reference: https://controllerstech.com/how-to-interface-mpu6050-gy-521-with-stm32/
void MPU6050_Init (void)
{
	uint8_t check;
	uint8_t Data;
	
	// check device ID WHO_AM_I

	HAL_I2C_Mem_Read (&hi2c1, MPU6050_ADDR,WHO_AM_I_REG,1, &check, 1, 1000);

	if (check == 104)  // 0x68 will be returned by the sensor if everything goes well
	{
		// power management register 0X6B we should write all 0's to wake the sensor up
		Data = 0;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1,&Data, 1, 1000);

		// Set DATA RATE of 1KHz by writing SMPLRT_DIV register
		Data = 0x07;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, SMPLRT_DIV_REG, 1, &Data, 1, 1000);

		// Set accelerometer configuration in ACCEL_CONFIG Register
		// XA_ST=0,YA_ST=0,ZA_ST=0, FS_SEL=0 -> ± 2g
		Data = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &Data, 1, 1000);

		// Set Gyroscopic configuration in GYRO_CONFIG Register
		// XG_ST=0,YG_ST=0,ZG_ST=0, FS_SEL=0 -> ± 250 °/s
		Data = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &Data, 1, 1000);
	}
}

void MPU6050_Read_Accel (void)
{
	uint8_t Rec_Data[6];

	// Read 6 BYTES of data starting from ACCEL_XOUT_H register

	HAL_I2C_Mem_Read (&hi2c1, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, Rec_Data, 6, 1000);

	Accel_X_RAW = (int16_t)(Rec_Data[0] << 8 | Rec_Data [1]);
	Accel_Y_RAW = (int16_t)(Rec_Data[2] << 8 | Rec_Data [3]);
	Accel_Z_RAW = (int16_t)(Rec_Data[4] << 8 | Rec_Data [5]);

	/*** convert the RAW values into acceleration in 'g'
	     we have to divide according to the Full scale value set in FS_SEL
	     I have configured FS_SEL = 0. So I am dividing by 16384.0
	     for more details check ACCEL_CONFIG Register              ****/

	Ax = Accel_X_RAW/16384.0;
	Ay = Accel_Y_RAW/16384.0;
	Az = Accel_Z_RAW/16384.0;
}


void MPU6050_Read_Gyro (void)
{
	uint8_t Rec_Data[6];

	// Read 6 BYTES of data starting from GYRO_XOUT_H register

	HAL_I2C_Mem_Read (&hi2c1, MPU6050_ADDR, GYRO_XOUT_H_REG, 1, Rec_Data, 6, 1000);

	Gyro_X_RAW = (int16_t)(Rec_Data[0] << 8 | Rec_Data [1]);
	Gyro_Y_RAW = (int16_t)(Rec_Data[2] << 8 | Rec_Data [3]);
	Gyro_Z_RAW = (int16_t)(Rec_Data[4] << 8 | Rec_Data [5]);

	/*** convert the RAW values into dps (°/s)
	     we have to divide according to the Full scale value set in FS_SEL
	     I have configured FS_SEL = 0. So I am dividing by 131.0
	     for more details check GYRO_CONFIG Register              ****/

	Gx = Gyro_X_RAW/131.0;
	Gy = Gyro_Y_RAW/131.0;
	Gz = Gyro_Z_RAW/131.0;
}

// Calculating the pitch, roll, and yaw values using the accelerometer input data
// Reference: https://engineering.stackexchange.com/questions/3348/calculating-pitch-yaw-and-roll-from-mag-acc-and-gyro-data
void convertAcc (void){  
	
	pitch = 180 * atan (Ax/sqrt(Ay*Ay + Az*Az))/M_PI;
	roll = 180 * atan (Ay/sqrt(Ax*Ax + Az*Az))/M_PI;
	yaw = 180 * atan (Az/sqrt(Ax*Ax + Az*Az))/M_PI;
	roll *= -1; // The MPU is facing the opposite way to the screen and so the value is required to be flipped
}

//Returns the radians value of a degree angle
float toRadians(float angle){
	return angle * ( M_PI / 180.0 );  
}

// saves the Circumference X and Y, When the MPU is held up in the same orientation as the screen, the angle will be the roll value.
void getCircumferenceXY (int x0, int y0, int r, float angle){
	float xPos = 0;
	float yPos = 0;
	// The values lie between -90 and 90 with 0 at the top/bottom middle. Radians start at 0 on the right. 
	//Therefore can be translated by 3PI/2 to rotate 0 to be directly downwards.
	float radAngle = toRadians(angle) + (3*M_PI)/2; 
	// Find the x and y positions with trigonemetry functions for the circumference of a circle
	xPos = r * (float)cos(radAngle) + x0; 
  yPos = r * (float)sin(radAngle) + y0;
	
  circX = (int)xPos;
	circY = (int)yPos;
	
	if(angle >= 60 || angle <= -60){
		turnOnBuzzer();
	}else{
		turnOffBuzzer();
	}
}
//------------------------END MPU CODE---------------------------------------

// All no moving/changing UI elements are called in the function.
void mainScreen(){
	
	GLCD_ClearScreen();
	//Used to ensure the correct colour scheme is setup.
	if (colourScheme == 0){
		colour1 = GLCD_COLOR_BLACK;
		colour2 = GLCD_COLOR_WHITE;
	}
	else if (colourScheme == 1){
		colour1 = GLCD_COLOR_BLACK;
		// Cyan
		colour2 = 0x07F9;
	}
	else if (colourScheme == 2){
		colour1 = GLCD_COLOR_BLACK;
		colour2 = GLCD_COLOR_MAGENTA;
	}
	else if (colourScheme == 3){
		colour1 = GLCD_COLOR_BLACK;
		// Red
		colour2 = 0xFA20;
	}
	//Fill the background colour
	fillBackground(colour1);
	// Settings Button
	drawRectangle(320, 5, 60, 30, colour2);
	// Settings Annotation
	drawString(327, 11, "SET", colour2, colour1);
	
	// Temperature Display
	drawCircle(240, 71, 71, colour2);
	
	// Temperature Reading		
	if(tempUnit == 0){
		// °F Annotation 
		drawCircle(233, 88, 4, colour2);
		drawString(240, 85, "F", colour2, colour1);
	}else{
		// °C Annotation 
		drawCircle(233, 88, 4, colour2);
		drawString(240, 85, "C", colour2, colour1);
	}
	
	// Gyrometer Display		
	drawCircle(240, 272, 130, colour2);
	
	// Left Ultrasonic Display
	displayDisChevronsLeft(0, colour1, colour2);
	
	if(distUnit == 0){
		// Left Ultrasonic Reading
		drawString(118, 125, ".", colour2, colour1);
		drawString(164, 125, "yd", colour2, colour1);
	}else{
		// Left Ultrasonic Reading
		drawString(118, 125, ".", colour2, colour1);
		drawString(164, 125, "m", colour2, colour1);
	}
	
	// Right Ultrasonic Display
	displayDisChevronsRight(0, colour1, colour2);
	
	if(distUnit == 0){
		// Right Ultrasonic Reading
	drawString(325, 125, ".", colour2, colour1);
	drawString(371, 125, "yd", colour2, colour1);
	}else{
		// Right Ultrasonic Reading
	drawString(325, 125, ".", colour2, colour1);
	drawString(371, 125, "m", colour2, colour1);
	}	
}

void settingsScreen(){
	
	int touchValue;
	
	TOUCH_STATE tsc_state;

	GLCD_ClearScreen();

	// Setting up colour schemes
	if (colourScheme == 0){
		colour1 = GLCD_COLOR_BLACK;
		colour2 = GLCD_COLOR_WHITE;
	}
	else if (colourScheme == 1){
		colour1 = GLCD_COLOR_BLACK;
		colour2 = 0x07F9;
	}
	else if (colourScheme == 2){
		colour1 = GLCD_COLOR_MAGENTA;
		colour2 = GLCD_COLOR_WHITE;
	}
	else if (colourScheme == 3){
		colour1 = GLCD_COLOR_BLACK;
		colour2 = 0xFA20;
	}
	
	// Settings Title
	drawRectangle(160, 0, 160, 35, GLCD_COLOR_BLACK);
	drawString(177, 8, "SETTINGS", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	
	// Back Button
	drawRectangle(403, 5, 70, 30, GLCD_COLOR_BLACK);
	// Back Annotation
	drawString(406, 11, "BACK", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	
	// Unit Measurement Select Display
	drawRectangle(5, 61, 213, 204, GLCD_COLOR_BLACK);
	drawString(72, 65, "Units", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	
	// Temperature Measurement Select
	drawString(24, 101, "Temperature", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	// °C Button
	drawRectangle(25, 135, 70, 30, GLCD_COLOR_BLACK);
	drawCircle(50, 143, 4, GLCD_COLOR_BLACK);
	drawString(57, 140, "C", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	// °F Button
	drawRectangle(131, 135, 70, 30, GLCD_COLOR_BLACK);
	drawCircle(156, 143, 4, GLCD_COLOR_BLACK);
	drawString(163, 140, "F", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	
	// Distance Measurement Select
	drawString(52, 180, "Distance", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	// m button
	drawRectangle(25, 215, 70, 30, GLCD_COLOR_BLACK);
	drawString(53, 220, "m", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	// yd button
	drawRectangle(131, 215, 70, 30, GLCD_COLOR_BLACK);
	drawString(150, 218, "yd", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	
	// Colour Palette Select
	drawRectangle(260, 61, 213, 204, GLCD_COLOR_BLACK);
	drawString(322, 65, "Colour", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	
	// Palette Displays
	drawPalette(295, 120, 45);
	fillPalette(295, 120, 45, GLCD_COLOR_WHITE);
	drawString(293, 88, "Day", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	drawPalette(396, 120, 45);
	fillPalette(396, 120, 45, 0x07F9);
	drawString(381, 88, "Night", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	drawPalette(295, 208, 45);
	fillPalette(295, 208, 45, GLCD_COLOR_MAGENTA);
	drawString(280, 175, "Funky", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);
	drawPalette(396, 208, 45);
	fillPalette(396, 208, 45, 0xFA20);
	drawString(389, 175, "Evil", GLCD_COLOR_BLACK, GLCD_COLOR_WHITE);

	// Highlight the bounds of the current setting box
	highlightTempUnit(tempUnit);
	highlightDistUnit(distUnit);
	highlightColour(colourScheme);

	// Required to stay on this screen until the back button is pressed.
	for(;;)
	{ 
		Touch_GetState(&tsc_state);
		if (tsc_state.pressed)
		{		
			touchValue = checkCoordsSettings(tsc_state.x, tsc_state.y);
			if (touchValue == -1)
			{
				mainScreen();
				break;
			}
		}
		else if (touchValue == 0)
			tempUnit = 0;
		else if (touchValue == 1)
				tempUnit = 1;
		else if (touchValue == 10)
				distUnit = 0;
		else if (touchValue == 11)
				distUnit = 1;
		else if (touchValue == 20)
				colourScheme = 0;
		else if (touchValue == 21)
				colourScheme = 1;
		else if(touchValue == 22)
				colourScheme = 2;
		else if(touchValue == 23)
				colourScheme = 3;
	}
}

int main(void){
	int touchValue;
	char buf[3];	
	int* digits;
	char tempBuffer[3][128], lUltBuffer[4][128], rUltBuffer[4][128];
	int prev = 0;
	int loop = 0;
	
	TOUCH_STATE tsc_state;
	
	//-------------INIT START--------------------
	HAL_Init(); //Init Hardware Abstraction Layer
	SystemClock_Config(); //Config Clocks
	GPIO_Init();
	I2C1_Init();
	TIM2_Init();	
	__HAL_RCC_TIM2_CLK_ENABLE();
	
	Touch_Initialize();
	GLCD_Initialize(); //Init GLCD	
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	MPU6050_Init();
	//-------------INIT END----------------------
	
	temperature = 0;
	// °C = 1, °F = 0
	tempUnit = 1;
	// m = 1, yd = 0
	distUnit = 1;
			
	colourScheme = 0;

	mainScreen();
	HAL_Delay(1000);	
	for(;;)
	{
		//call MPU read functions
		MPU6050_Read_Accel();
		MPU6050_Read_Gyro();
		
		//Check if the user want to go to the settings menu
		Touch_GetState(&tsc_state);
		if (tsc_state.pressed)
		{
			touchValue = checkCoordsMain(tsc_state.x, tsc_state.y);
			if (touchValue)
			{
				settingsScreen();
			}
		}
		
		//------------Start MPU Calculations---------
		convertAcc();
	
		drawDiagonalLine(240, 272, circX, circY, colour1);
		getCircumferenceXY(240, 272, 128, roll);
		drawDiagonalLine(240, 272, circX, circY,colour2);
		
		//-----------------END MPU Calcs--------------
		
		//-------------Distance------------------
		// Determine how many chevrons to place on the left or right
		if(distLeft > 0 && distLeft <= 5 && currentDistLeft != 5){
			displayDisChevronsLeft(5, colour1, colour2);
			currentDistLeft = 5;
		}else if(distLeft > 5 && distLeft <= 10 && currentDistLeft != 4){
			displayDisChevronsLeft(4, colour1, colour2);
			currentDistLeft = 4;
		}else if(distLeft > 10 && distLeft <= 15 && currentDistLeft != 3){
			displayDisChevronsLeft(3, colour1, colour2);
			currentDistLeft = 3;
		}else if(distLeft > 15 && distLeft <= 20 && currentDistLeft != 2){
			displayDisChevronsLeft(2, colour1, colour2);
			currentDistLeft = 2;
		}else if(distLeft > 20 && distLeft <= 25 && currentDistLeft != 1){
			displayDisChevronsLeft(1, colour1, colour2);
			currentDistLeft = 1;
		}else if(distLeft > 30){
			displayDisChevronsLeft(0, colour1, colour2);
			distLeft = 0;
		}
		
		if(distRight > 0 && distRight <= 5 && currentDistRight != 5){
			displayDisChevronsRight(5, colour1, colour2);
			currentDistRight = 5;
		}else if(distRight > 5 && distRight <= 10 && currentDistRight != 4){
			displayDisChevronsRight(4, colour1, colour2);
			currentDistRight = 4;
		}else if(distRight > 10 && distRight <= 15 && currentDistRight != 3){
			displayDisChevronsRight(3, colour1, colour2);
			currentDistRight = 3;
		}else if(distRight > 15 && distRight <= 20 && currentDistRight != 2){
			displayDisChevronsRight(2, colour1, colour2);
			currentDistRight = 2;
		}else if(distRight > 20 && distRight <= 25 && currentDistRight != 1){
			displayDisChevronsRight(1, colour1, colour2);
			currentDistRight = 1;
		}else if(distRight > 30){
			displayDisChevronsRight(0, colour1, colour2);
			distRight = 0;
		}
		
		
		// If the rotary encoders button is pressed down then allow for the rotating function to be checked
		// otherwise pass straight through
		for(;;)	//right side
		{
			//Read the button pin
			if(HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_0) == GPIO_PIN_RESET)
			{
				//run check function and return 1 higher or lower dependent on direction spun	
				distLeft = checkEncoderLeft(distLeft);		
				sprintf(buf, "%2d", distLeft);
				if(distUnit == 0)
				{
					// Left Ultrasonic Reading
					drawString(118, 125, buf, colour2, colour1);
					drawString(164, 125, "yd", colour2, colour1);
				}
				else
				{
					// Left Ultrasonic Reading
					drawString(118, 125, buf, colour2, colour1);
					drawString(164, 125, "m", colour2, colour1);
				}
			}
			else
			{
				break;
			}
		}
		
		for(;;){//left side
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET){
			distRight = checkEncoderRight(distRight);		
			
			sprintf(buf, "%2d", distRight);
			if(distUnit == 0){
			// Right Ultrasonic Reading
			drawString(325, 125, buf, colour2, colour1);
			drawString(371, 125, "yd", colour2, colour1);
			}else{
				// Right Ultrasonic Reading
			drawString(325, 125, buf, colour2, colour1);
			drawString(371, 125, "m", colour2, colour1);
			}
			
			}else{break;}
		}
		//----------------end--------------------
		
		//-------------Temperature---------------
		
				digits = getDigits(temperature);
				sprintf(tempBuffer[0], "%d", digits[0]);
				sprintf(tempBuffer[1], "%d", digits[1]);
				sprintf(tempBuffer[2], "%d", digits[2]);
				
				drawString(220, 50, tempBuffer[2], colour2, colour1);
				drawString(235, 50, tempBuffer[1], colour2, colour1);
				drawString(250, 50, tempBuffer[0], colour2, colour1);
				
		//-----------------End-------------------
		
		temperature += 1;

		if (temperature == 1000)
			temperature = 0;
		
		HAL_Delay(200);
	}
}

void SystemClock_Config(void)
{
			RCC_OscInitTypeDef RCC_OscInitStruct;
			RCC_ClkInitTypeDef RCC_ClkInitStruct;
			/* Enable Power Control clock */
			__HAL_RCC_PWR_CLK_ENABLE();
			/* The voltage scaling allows optimizing the power
			consumption when the device is clocked below the
			maximum system frequency. */
			__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
			/* Enable HSE Oscillator and activate PLL
			with HSE as source */
			RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
			RCC_OscInitStruct.HSEState = RCC_HSE_ON;
			RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
			RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
			RCC_OscInitStruct.PLL.PLLM = 25;
			RCC_OscInitStruct.PLL.PLLN = 336;
			RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
			RCC_OscInitStruct.PLL.PLLQ = 7;
			HAL_RCC_OscConfig(&RCC_OscInitStruct);
			/* Select PLL as system clock source and configure
			the HCLK, PCLK1 and PCLK2 clocks dividers */
			RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | 
			RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
			RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
			RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
			HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

static void I2C1_Init(void)
{
  /* USER CODE BEGIN I2C1_Init 0 */
	__HAL_RCC_I2C1_CLK_ENABLE();
    
  __HAL_RCC_I2C1_FORCE_RESET();
  HAL_Delay(2);
  __HAL_RCC_I2C1_RELEASE_RESET();
  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00808CD2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

	
  /* USER CODE END I2C1_Init 2 */


}

static void TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 32000;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

static void GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
	
	
	// MPU GPIO
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; // alternate function - open drain
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
  // LED_Buzzer GPIO
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = NULL;
	
	HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	
	
  //Encoder GPIO
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = NULL;
	GPIO_InitStruct.Alternate = NULL;
	
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */



