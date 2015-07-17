/**
  ******************************************************************************
  * @file    Inc/app_ultrasonic.h
  * @version V1.0
  * @date    02-July-2015
  * @brief   Header for app_ultrasonic.c module
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_ULTRASONIC_H
#define __APP_ULTRASONIC_H



/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

/* Function Declaration*/
//front left right
//front down back
//void scan_FLR(void);
//void scan_FDB(void);

BOOL us_front(void);
BOOL us_left(void);
BOOL us_right(void);
BOOL us_down(void);
BOOL us_back(void);

void Ultrasonic_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void Ultrasonic_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);


#endif /* #ifndef __APP_ULTRASONIC_H */
