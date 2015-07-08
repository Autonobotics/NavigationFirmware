/**
  ******************************************************************************
  * @file    Src/app_uart_callbacks.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application UART Callback Implementations
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "app_armpit.h"
#include "app_pixarm.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if ( huart->Instance == ARMPIT_USART )
    {
        ARMPIT_UART_TxCpltCallback(huart);
    }
    else if ( huart->Instance == PIXARM_USART )
    {
        PIXARM_UART_TxCpltCallback(huart);
    }
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if ( huart->Instance == ARMPIT_USART )
    {
        ARMPIT_UART_RxCpltCallback(huart);
    }
    else if ( huart->Instance == PIXARM_USART )
    {
        PIXARM_UART_RxCpltCallback(huart);
    }
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if ( huart->Instance == ARMPIT_USART )
    {
        ARMPIT_UART_ErrorCallback(huart);
    }
    else if ( huart->Instance == PIXARM_USART )
    {
        PIXARM_UART_ErrorCallback(huart);
    }
}
