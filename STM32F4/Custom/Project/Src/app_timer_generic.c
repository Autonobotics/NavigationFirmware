/**
  ******************************************************************************
  * @file    Src/app_timer_generic.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application Timer Utility and Callback Implementations
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "app_timer_generic.h"
#include "app_common.h"
#include "app_navigation.h"
#include "app_ultrasonic_adapter.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
//once one period for tim5 (10us) is done this function is called
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if ( (TIM2 == htim->Instance) ||
         (TIM3 == htim->Instance) ||
         (TIM4 == htim->Instance) ||
         (TIM5 == htim->Instance) ||
         (TIM6 == htim->Instance) )
    {
        Ultrasonic_TIM_PeriodElapsedCallback(htim);
    }
    else if ( TIM10 == htim->Instance )
    {
        Heartbeat_PeriodElapsedCallback(htim);
    }
    else if ( TIM11 == htim->Instance )
    {
        APP_Guide_Timeout_PeriodElapsedCallback(htim);
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if ( (TIM2 == htim->Instance) ||
         (TIM3 == htim->Instance) ||
         (TIM4 == htim->Instance) ||
         (TIM5 == htim->Instance) ||
         (TIM6 == htim->Instance) )
    {
        Ultrasonic_TIM_IC_CaptureCallback(htim);
    }
}
