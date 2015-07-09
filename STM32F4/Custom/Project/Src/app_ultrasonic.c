/**
  ******************************************************************************
  * @file    Src/app_ultrasonic.c 
  * @version V1.0
  * @date    7-July-2015
  * @brief   Application Ultrasonic Sensor HC-SR04, Triggers sensor, returns a value
						 volatile int ultrasonic_busy prevents user from calling function
						 when sensors are still active
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "app_ultrasonic.h"
#include "stm32f4xx_hal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;

/* Public variables ----------------------------------------------------------*/
volatile int sensor_count = 0; //once value 4 means all sensors done, can calculate new value
volatile int FLR_sel = 0; //if FLR_sel = 1 then we are doing front,left,right sensor else its FDB
//holds the counter value
volatile int front_count = 0;
volatile int down_count = 0;
volatile int back_count = 0;
volatile int left_count = 0;
volatile int right_count = 0;
volatile int tim_5 = 0;
volatile int tim_2 = 0;
volatile int tim_3 = 0;
volatile int tim_4 = 0;
volatile int calc_distance = 0;
volatile int run = 0;
//logic for single sensor firing
volatile int trig = 0; //keeps track if the trig has been fired or not
volatile int front = 0; //keeps track of which sensor to listen from
volatile int back = 0;
volatile int left = 0;
volatile int right = 0;
volatile int down = 0;

/* Private variables ---------------------------------------------------------*/

//holds the calculated distance in cm
static float front_distance = 0;
static float down_distance = 0;
static float back_distance = 0;
static float left_distance = 0;
static float right_distance = 0;
//array to return the distances

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//once one period for tim5 (10us) is done this function is called
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM5)
		{
			if(trig == 1){
				if(front ==1){HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_RESET);}//set trig = 0
				else if(left ==1){HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);}
				else if(down ==1){HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET);}
				else if(right ==1){HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_RESET);}
				else if(back ==1){HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_RESET);}
				HAL_TIM_Base_Stop_IT(&htim5);//stop the 10us timer
				trig = 1;
				HAL_NVIC_DisableIRQ(TIM5_IRQn); //disable trig timer interrupt
				//enable the interrupt for the correct sensor
				if(front ==1){HAL_NVIC_EnableIRQ(TIM2_IRQn);}
				else if(left ==1 || down ==1){HAL_NVIC_EnableIRQ(TIM3_IRQn);}
				else if(right ==1 || back ==1){HAL_NVIC_EnableIRQ(TIM2_IRQn);}
				trig++;
			}
			tim_5++;
		}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM2)
	{
		//sensor_count++;
		if(front == 1 && trig == 2){
			//falling edge of echo triggers the capture counter value, reset+stop counter
			HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);//stop counter
			front_count = __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_1);
			__HAL_TIM_SET_COUNTER(&htim2,0);//reset counter
			front++;
		}
		else if(back == 1 && trig == 2){
			HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);
			back_count = __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_2);
			__HAL_TIM_SET_COUNTER(&htim2,0);
			back++;
		}
		HAL_NVIC_DisableIRQ(TIM2_IRQn);
		tim_2++;
	}
	
	if(htim->Instance==TIM3)
	{
		if(left == 1 && trig == 2){
			HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
			left_count = __HAL_TIM_GetCompare(&htim3, TIM_CHANNEL_1);
			__HAL_TIM_SET_COUNTER(&htim3,0);
			left++;
		}
		else if(down == 1 && trig == 2){
			HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
			down_count = __HAL_TIM_GetCompare(&htim3, TIM_CHANNEL_2);
			__HAL_TIM_SET_COUNTER(&htim3,0);
			down++;
		}
		else if(right == 1 && trig == 2){
			HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_3);
			right_count = __HAL_TIM_GetCompare(&htim3, TIM_CHANNEL_3);
			__HAL_TIM_SET_COUNTER(&htim3,0);
			right++;
		}
		HAL_NVIC_DisableIRQ(TIM3_IRQn);
		tim_3++;
	}
	
	/*if(htim->Instance==TIM4)
	{
		if(right == 1 && trig == 2){
			HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_2);
			right_count = __HAL_TIM_GetCompare(&htim4, TIM_CHANNEL_2);
			__HAL_TIM_SET_COUNTER(&htim4,0);
			right++;
		}
		else if(back == 1 && trig == 2){
			HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_3);
			back_count = __HAL_TIM_GetCompare(&htim4, TIM_CHANNEL_3);
			__HAL_TIM_SET_COUNTER(&htim4,0);
			back++;
		}
		HAL_NVIC_DisableIRQ(TIM4_IRQn);
		tim_4++;
	}*/
}
/* Public functions ----------------------------------------------------------*/
void us_front(void){
	if (trig == 0){
		HAL_NVIC_EnableIRQ(TIM5_IRQn); //enable trig counter to interrupt
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET); //set trig = 1
		HAL_TIM_Base_Start_IT(&htim5); //start 10us counter
		front++; //indicate front sensor is operating
		trig++; //logic to prevent multi trig fire
		run++; //keep track of #times this got called
	}
	else if (front == 2){
		calc_distance++;
		front_distance = ((front_count*0.0001)*340)/2; //converted to cm/s
		front = 0;
		trig = 0;
	}
}

void us_left(void){
	if (trig == 0){
		HAL_NVIC_EnableIRQ(TIM5_IRQn); 
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_SET); 
		HAL_TIM_Base_Start_IT(&htim5); 
		left++; 
		trig++; 
		run++; 
	}
	else if (left == 2){
		left_distance = ((left_count*0.0001)*340)/2;
		left = 0;
		trig = 0;
	}
}
void us_down(void){
	if (trig == 0){
		HAL_NVIC_EnableIRQ(TIM5_IRQn); 
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET); 
		HAL_TIM_Base_Start_IT(&htim5); 
		down++; 
		trig++; 
		run++; 
	}
	else if (down == 2){
		down_distance = ((down_count*0.0001)*340)/2;
		down = 0;
		trig = 0;
	}
}

void us_right(void){
	if (trig == 0){
		HAL_NVIC_EnableIRQ(TIM5_IRQn); 
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET); 
		HAL_TIM_Base_Start_IT(&htim5); 
		right++; 
		trig++; 
		run++; 
	}
	else if (right == 2){
		right_distance = ((right_count*0.0001)*340)/2;
		right = 0;
		trig = 0;
	}
}



void us_back(void){
	if (trig == 0){
		HAL_NVIC_EnableIRQ(TIM5_IRQn); 
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_SET); 
		HAL_TIM_Base_Start_IT(&htim5); 
		back++; 
		trig++; 
		run++; 
	}
	else if (back == 2){
		back_distance = ((back_count*0.0001)*340)/2;
		back = 0;
		trig = 0;
	}
}

/* -------------------------- End of File ------------------------------------*/
