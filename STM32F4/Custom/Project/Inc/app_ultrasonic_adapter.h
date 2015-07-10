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
#include "app_ultrasonic.h"

/* Exported Constants --------------------------------------------------------*/
#define HC_SR04_FLR_TRIGGER_PIN             GPIO_PIN_8
#define HC_SR04_FLR_TRIGGER_PORT            GPIOA

#define HC_SR04_FBR_TRIGGER_PIN             GPIO_PIN_12
#define HC_SR04_FBR_TRIGGER_PORT            GPIOE

// FRONT_PIN must be Connected to EXTI0
#define HC_SR04_FRONT_PIN                   GPIO_PIN_0
#define HC_SR04_FRONT_PORT                  GPIOC

// BOTTOM_PIN must be Connected to EXTI2
#define HC_SR04_BOTTOM_PIN                  GPIO_PIN_2
#define HC_SR04_BOTTOM_PORT                 GPIOC

// LEFT_PIN must be Connected to EXTI1
#define HC_SR04_LEFT_PIN                    GPIO_PIN_1
#define HC_SR04_LEFT_PORT                   GPIOC

// RIGHT_PIN must be Connected to EXTI3
#define HC_SR04_RIGHT_PIN                   GPIO_PIN_3
#define HC_SR04_RIGHT_PORT                  GPIOC

// REAR_PIN must be Connected to EXTI4
#define HC_SR04_REAR_PIN                    GPIO_PIN_4
#define HC_SR04_REAR_PORT                   GPIOC


#define HC_SR04_SENSORS_PER_SET 3
#define HC_SR04_FRONT 0
#define HC_SR04_LEFT 1
#define HC_SR04_RIGHT 2
#define HC_SR04_BOTTOM 1
#define HC_SR04_REAR 2

/* Exported typedefs ---------------------------------------------------------*/
typedef enum _eAPP_HC_SR04_STATE
{
    HC_SR04_INIT = 0,
    HC_SR04_WAITING,
    HC_SR04_ENABLED,
    
} eAPP_HC_SR04_STATE;

typedef enum _eAPP_HC_SR04_PULSE_INDV
{
    PULSE_FRONT = 0,
    PULSE_RIGHT,
    PULSE_LEFT,
    PULSE_REAR,
    PULSE_BOTTOM,
    
    PULSE_LIMIT
    
} eAPP_HC_SR04_PULSE_INDV;

typedef enum _eAPP_HC_SR04_PULSE_SET
{
    FRONT_LEFT_RIGHT = 0,
    FRONT_BOTTOM_REAR,
    
} eAPP_HC_SR04_PULSE_SET;

typedef struct _sAPP_HC_SR04_SET_DATA
{
    uint16_t set[HC_SR04_SENSORS_PER_SET];
    
} sAPP_HC_SR04_SET_DATA;

typedef struct _sAPP_HC_SR04_DATA
{
    sAPP_HC_SR04_SET_DATA flr_data;
    sAPP_HC_SR04_SET_DATA fbr_data;
    
} sAPP_HC_SR04_DATA;

typedef struct _sAPP_HC_SR04_CBLK
{
    eAPP_HC_SR04_STATE state;
    TIM_HandleTypeDef* tim2Handle;
    TIM_HandleTypeDef* tim3Handle;
    TIM_HandleTypeDef* tim5Handle;
    
    volatile uint8_t received_count;
    BOOL pulsed_ongoing;
    eAPP_HC_SR04_PULSE_SET pulse_set;
    eAPP_HC_SR04_PULSE_INDV pulse_indv;
    
    sAPP_HC_SR04_DATA data;
    
} sAPP_HC_SR04_CBLK;

/* Exported constants --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void APP_HC_SR04_Init(void);
void HAL_HC_SR04_MspInit(void);
eAPP_STATUS APP_HC_SR04_Pulse_Sensors(sAPP_NAVIGATION_CBLK* navigation_cblk);

#endif // #ifndef __APP_HC_SR04_H
