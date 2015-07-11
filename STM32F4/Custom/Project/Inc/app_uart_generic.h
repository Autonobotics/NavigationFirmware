/**
  ******************************************************************************
  * @file    Inc/app_uart_generic.h
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Header for app_uart_generic.c module
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_UART_GENERIC_H
#define __APP_UART_GENERIC_H


/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
#if defined(ARMPIT_WATCHDOG_ENABLE) || defined(PIXARM_WATCHDOG_ENABLE)
typedef enum _eAPP_UART_PERIPHERAL_STATE
{
    UART_PERIPHERAL_IDLE = 0,
    UART_PERIPHERAL_TRANSMITTING,
    UART_PERIPHERAL_RECEIVING
    
} eAPP_UART_PERIPHERAL_STATE;
#endif // if defined(ARMPIT_WATCHDOG_ENABLE) || defined(PIXARM_WATCHDOG_ENABLE)

/* Exported functions ------------------------------------------------------- */
char* APP_UART_Generic_Translate_Error(uint32_t error_code);
void APP_UART_Generic_Flush_Buffer(UART_HandleTypeDef *huart);
eAPP_STATUS APP_UART_Generic_Recover_From_Error(UART_HandleTypeDef *huart);

#endif // #ifndef __APP_UART_GENERIC_H
