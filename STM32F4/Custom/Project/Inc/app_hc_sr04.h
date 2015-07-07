/**
  ******************************************************************************
  * @file    Inc/app_hc_sr04.h 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Header for app_hc_sr04.c module
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_HC_SR04_H
#define __APP_HC_SR04_H


/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "app_navigation.h"

/* Exported constants --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void APP_HC_SR04_Init(void);
void HAL_HC_SR04_MspInit(void);
eAPP_STATUS APP_HC_SR04_Pulse_Sensors(sAPP_NAVIGATION_CBLK* navigation_cblk);

#endif // #ifndef __APP_HC_SR04_H
