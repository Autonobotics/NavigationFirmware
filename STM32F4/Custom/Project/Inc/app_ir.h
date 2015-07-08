/**
  ******************************************************************************
  * @file    Inc/app_ir.h
  * @version V1.0
  * @date    02-July-2015
  * @brief   Header for app_ir.c module
  ******************************************************************************
  */
/*
This function when called returns a 1/0 indicating it sees the IR or not.
Note: If sensor Vout pin = 0 (sees the IR) -> function returns 1
*/
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_IR_H
#define __APP_IR_H

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

/* Exported constants --------------------------------------------------------*/
#define IR_CLOCK_ENABLE()       __HAL_RCC_GPIOD_CLK_ENABLE()

#define IR_PIN                  GPIO_PIN_0
#define IR_GPIO_PORT            GPIOD
#define IR_GPIO_PULLUPDOWN      GPIO_NOPULL


/* Function Declaration*/
void APP_IR_Init(void);
void HAL_IR_MspInit(void);
GPIO_PinState APP_Scan_IR(void);

#endif /* #ifndef __APP_IR_H */
