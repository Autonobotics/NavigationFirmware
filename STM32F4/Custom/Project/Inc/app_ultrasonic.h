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
//#include "app_common.h"

/* Function Declaration*/
//front left right
//front down back
//void scan_FLR(void);
//void scan_FDB(void);

void us_front(void);
void us_left(void);
void us_right(void);
void us_down(void);
void us_back(void);


#endif /* #ifndef __APP_ULTRASONIC_H */
