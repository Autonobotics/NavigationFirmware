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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;

/* Public variables ----------------------------------------------------------*/
//holds the counter value
volatile int front_count = 0;
volatile int down_count = 0;
volatile int back_count = 0;
volatile int left_count = 0;
volatile int right_count = 0;
//logic for single sensor firing
volatile int trig = 0; //keeps track if the trig has been fired or not
volatile int front = 0; //keeps track of which sensor to listen from
volatile int back = 0;
volatile int left = 0;
volatile int right = 0;
volatile int down = 0;
//debug
volatile int tim_2 = 0;
volatile int tim_3 = 0;
volatile int tim_4 = 0;
volatile int tim_5 = 0;
volatile int run = 0;
volatile int to_f = 0;
volatile int to_l = 0;
volatile int to_d = 0;
volatile int to_r = 0;
volatile int to_b = 0;

/* Private variables ---------------------------------------------------------*/
//holds the calculated distance in cm
float front_distance = 0;
float down_distance = 0;
float back_distance = 0;
float left_distance = 0;
float right_distance = 0;
//array to return the distances

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
void Ultrasonic_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM5)
    {
        if (trig == 1)
        {
            //set trig = 0 and enable the interrupt for the correct sensor
            if(front == 1)
            {
                HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_RESET);
                HAL_NVIC_EnableIRQ(TIM2_IRQn);
            }
            else if (left == 1)
            {
                HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
                HAL_NVIC_EnableIRQ(TIM3_IRQn);
            }
            else if (down == 1)
            {
                HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET);
                HAL_NVIC_EnableIRQ(TIM3_IRQn);
            }
            else if (right == 1)
            {
                HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_RESET);
                HAL_NVIC_EnableIRQ(TIM4_IRQn);
            }
            else if (back == 1)
            {
                HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_RESET);
                HAL_NVIC_EnableIRQ(TIM4_IRQn);
            }
            HAL_TIM_Base_Stop_IT(&htim5);//stop the 10us timer
            HAL_NVIC_DisableIRQ(TIM5_IRQn); //disable trig timer interrupt
            HAL_TIM_Base_Start_IT(&htim6); //start watchdog timer
            //proceed to recieve echo
            trig++;
        }
        tim_5++;
    }
    
    //watchdog timer (tim6)
    //if the timeout occurs before falling edge of echo occurs, timeout set for 180 counts (0.018s ~ 3m)
    if (htim->Instance == TIM6)
    {
        if (front == 1)
        {
            HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);//stop the counter
            __HAL_TIM_SET_COUNTER(&htim2,0);//reset counter
            HAL_NVIC_DisableIRQ(TIM2_IRQn);
            front_count = DISTANCE_UNKNOWN; // time out, no-object flag
            //proceed to calculate with previous counter value
            front++;
            to_f++;
        }
        else if (left == 1)
        {
            HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
            __HAL_TIM_SET_COUNTER(&htim3,0);
            HAL_NVIC_DisableIRQ(TIM3_IRQn);
            left_count = DISTANCE_UNKNOWN; // time out, no-object flag
            left++;
            to_l++;

        }
        else if (down == 1)
        {
            HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
            __HAL_TIM_SET_COUNTER(&htim3,0);
            HAL_NVIC_DisableIRQ(TIM3_IRQn);
            down_count = DISTANCE_UNKNOWN; // time out, no-object flag
            down++;
            to_d++;
        }
        else if (right == 1)
        {
            HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_1);
            __HAL_TIM_SET_COUNTER(&htim4,0);
            HAL_NVIC_DisableIRQ(TIM4_IRQn);
            right_count = DISTANCE_UNKNOWN; // time out, no-object flag
            right++;
            to_r++;
        }
        else if (back == 1)
        {
            HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_2);
            __HAL_TIM_SET_COUNTER(&htim4,0);
            HAL_NVIC_DisableIRQ(TIM4_IRQn);
            back_count = DISTANCE_UNKNOWN; // time out, no-object flag
            back++;
            to_b++;	
        }
        HAL_TIM_Base_Stop_IT(&htim6);
    }
}

void Ultrasonic_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        //as soon as you get here disable watchdog to prevent unecessary timeouts
        //since the CaptureCallback() gets called on the falling edge of echo - capture was sucessful
        HAL_TIM_Base_Stop_IT(&htim6); 
        if (front == 1 && trig == 2)
        {
            //falling edge of echo triggers the capture counter value, reset+stop counter
            HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);//stop counter
            front_count = __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_1); //capture counter value
            front++; //indicate new counter value, perform calculation on next function call
        }
        __HAL_TIM_SET_COUNTER(&htim2, 0);//reset counter
        HAL_NVIC_DisableIRQ(TIM2_IRQn); //disable interrupt
        tim_2++;
    }
    
    if (htim->Instance == TIM3)
    {
        HAL_TIM_Base_Stop_IT(&htim6);
        if (left == 1 && trig == 2)
        {
            HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
            left_count = __HAL_TIM_GetCompare(&htim3, TIM_CHANNEL_1);
            left++;
        }
        else if (down == 1 && trig == 2)
        {
            HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
            down_count = __HAL_TIM_GetCompare(&htim3, TIM_CHANNEL_2);
            down++;
        }
        __HAL_TIM_SET_COUNTER(&htim3,0);
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
        tim_3++;
    }
    
    if (htim->Instance == TIM4)
    {
        HAL_TIM_Base_Stop_IT(&htim6);
        if (right == 1 && trig == 2)
        {
            HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_1);
            right_count = __HAL_TIM_GetCompare(&htim4, TIM_CHANNEL_1);
            right++;
        }
        else if (back == 1 && trig == 2)
        {
            HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_2);
            back_count = __HAL_TIM_GetCompare(&htim4, TIM_CHANNEL_2);
            back++;
        }
        __HAL_TIM_SET_COUNTER(&htim4,0);
        HAL_NVIC_DisableIRQ(TIM4_IRQn);
        tim_4++;
    }
}

BOOL us_front(void)
{
    if (trig == 0)
    {
        HAL_NVIC_EnableIRQ(TIM5_IRQn); //enable trig counter to interrupt
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET); //set trig = 1
        HAL_TIM_Base_Start_IT(&htim5); //start 10us counter
        front++; //indicate front sensor is operating
        trig++; //logic to prevent multi trig fire
        run++; //keep track of #times this got called
    }
    else if (front == 2)
    {
        front_distance = ((front_count*0.0001)*340)/2; //calculate distance in m
        front = 0;
        trig = 0;
        return TRUE;
    }
    return FALSE;
}


BOOL us_left(void)
{
    if (trig == 0)
    {
        HAL_NVIC_EnableIRQ(TIM5_IRQn); 
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_SET); 
        HAL_TIM_Base_Start_IT(&htim5); 
        left++; 
        trig++; 
        run++; 
    }
    else if (left == 2)
    {
        left_distance = ((left_count*0.0001)*340)/2; //calculate distance in m
        left = 0;
        trig = 0;
        return TRUE;
    }
    return FALSE;
}


BOOL us_down(void)
{
    if (trig == 0)
    {
        HAL_NVIC_EnableIRQ(TIM5_IRQn); 
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET); 
        HAL_TIM_Base_Start_IT(&htim5); 
        down++; 
        trig++; 
        run++; 
    }
    else if (down == 2)
    {
        down_distance = ((down_count*0.0001)*340)/2; //calculate distance in m
        down = 0;
        trig = 0;
        return TRUE;
    }
    return FALSE;
}


BOOL us_right(void)
{
    if (trig == 0)
    {
        HAL_NVIC_EnableIRQ(TIM5_IRQn); 
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET); 
        HAL_TIM_Base_Start_IT(&htim5); 
        right++; 
        trig++; 
        run++; 
    }
    else if (right == 2)
    {
        right_distance = ((right_count*0.0001)*340)/2; //calculate distance in m
        right = 0;
        trig = 0;
        return TRUE;
    }
    return FALSE;
}


BOOL us_back(void)
{
    if (trig == 0)
    {
        HAL_NVIC_EnableIRQ(TIM5_IRQn); 
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_SET); 
        HAL_TIM_Base_Start_IT(&htim5); 
        back++; 
        trig++; 
        run++; 
    }
    else if (back == 2)
    {
        back_distance = ((back_count*0.0001)*340)/2;
        back = 0;
        trig = 0;
        return TRUE;
    }
    return FALSE;
}

/* -------------------------- End of File ------------------------------------*/
