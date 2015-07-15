/**
  ******************************************************************************
  * @file    Inc/app_common.h
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Functions, Defines and Types common to all elements
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_COMMON_H
#define __APP_COMMON_H
 
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"
#include "app_config.h"
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_STATUS
{
    STATUS_SUCCESS = 0,
    
    STATUS_FAILURE = -500
    
} eAPP_STATUS;


/* Quadcopter Axis Definitions */
typedef enum _eAPP_AXIS
{
    AXIS_FRONT,     // +z
    AXIS_REAR,      // -z
    AXIS_LEFT,      // -x
    AXIS_RIGH,      // +x
    AXIS_TOP,       // +y
    AXIS_BOTTOM     // -y
    
} eAPP_AXIS;

typedef enum _eAPP_UART_REQUEST_STATE
{
    UART_INITIAL = 0,
    UART_NO_REQUEST,
    UART_TRANSMITING,
    UART_REQUEST_WAITING,
    UART_REQUEST_PROCESSING,
    UART_WAITING
    
} eAPP_UART_REQUEST_STATE;

/* Exported constants --------------------------------------------------------*/
#define TRUE    1
#define FALSE   0
typedef uint16_t BOOL;

// LED5 -> RED
#define BSP_HARD_ERROR_LED          LED5
// LED3 -> ORANGE
#define BSP_PIXARM_ERROR_LED           LED3
// LED4 -> GREEN
#define BSP_ARMPIT_ERROR_LED          LED4
// LED6 -> BLUE
#define BSP_ULTRASONIC_ERROR_LED    LED6


/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

// This could be fixed by making a proper logging function that
// uses vprintf with a given variadic and any extra data.
#ifdef DEBUG
    #define APP_Log printf
#else
    #define APP_Log(...)    
#endif // #ifdef DEBUG
#define ENDLINE "\r\n"
    
/* Exported functions ------------------------------------------------------- */
void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength);
uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
void Error_Handler(void);

void APP_Log_Init(void);

#endif /* #ifndef __APP_COMMON_H */
