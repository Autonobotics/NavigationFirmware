/**
  ******************************************************************************
  * @file    Src/app_uart_generic.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application UART Utility and Callback Implementations
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "app_uart_generic.h"
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
char* APP_UART_Generic_Translate_Error(uint32_t error_code)
{
    switch (error_code)
    {
        case HAL_UART_ERROR_NONE:
            return "UART No Error";
        
        case HAL_UART_ERROR_PE:
            return "UART Parity Error";
        
        case HAL_UART_ERROR_NE:
            return "UART Noise Error";
        
        case HAL_UART_ERROR_FE:
            return "UART Frame Error";
        
        case HAL_UART_ERROR_ORE:
            return "UART Overrun Error";
        
        case HAL_UART_ERROR_DMA:
            return "UART DMA Transfer Error";
        
        default:
            return "Unknown Error Code";
    }
}

void APP_UART_Generic_Flush_Buffer(UART_HandleTypeDef *huart)
{
    __HAL_UART_FLUSH_DRREGISTER(huart);
}


eAPP_STATUS APP_UART_Generic_Recover_From_Error(UART_HandleTypeDef *huart)
{
    // Flush and Clear any Errors
    __HAL_UART_CLEAR_PEFLAG(huart);
    return STATUS_SUCCESS;
}

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
