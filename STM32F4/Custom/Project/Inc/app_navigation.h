/**
  ******************************************************************************
  * @file    Inc/app_navigation.h
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Supplies Interface for navigation data structures and methods.
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_NAVIGATION_H
#define __APP_NAVIGATION_H

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

/* Exported constants --------------------------------------------------------*/
#define HC_SR04_OUT_OF_RANGE 0x7FFF

#define FRONTAL_AVOIDANCE_MODE_ON   TRUE
#define FRONTAL_AVOIDANCE_MODE_OFF  FALSE

#define DISTANCE_UNKNOWN 0x7FFF

#define ROTATION_UNKNOWN 0x7FFF  // Centidegrees
#define ROTATION_SEARCH 0x7FFE

#define ROTATION_COMPLETE TRUE
#define ROTATION_INCOMPLETE FALSE

// Tolerance of a degree. Ie. 100 Centidegrees
#define ROTATION_EPSILON    100

#define ROTATION_MAX 36000
#define ROTATION_LOWER_BOUND(_desired_) (_desired_ - (ROTATION_EPSILON >> 1))
#define ROTATION_UPPER_BOUND(_desired_) (_desired_ + (ROTATION_EPSILON >> 1))

#define ROTATE_LEFT -3000
#define ROTATE_RIGHT 3000

//idle at 50cm
#define ALTITUDE_IDLE 80
//acceptable error of 20cm
#define ALTITUDE_MARGIN 20

#define AVOID_THRESHOLD_FRONT 30
#define AVOID_THRESHOLD_REAR 30
#define AVOID_THRESHOLD_LEFT 30
#define AVOID_THRESHOLD_RIGHT 30


/* Exported functions ------------------------------------------------------- */
BOOL APP_Navigation_Check_Rotation(uint16_t desired_rotation, uint16_t current_rotation);
eAPP_STATUS APP_Navigation_Compute(sAPP_NAVIGATION_CBLK* navigation_cblk);

void APP_Guide_Timeout_Init(void);
void APP_Guide_Timeout_PeriodElapsedCallback(TIM_HandleTypeDef *htim);


#endif // __APP_NAVIGATION_H
