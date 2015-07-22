/**
  ******************************************************************************
  * @file    Src/app_usart.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application USART Implementation
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*                 Utility Functions and Helpers                              */
/******************************************************************************/
char* Translate_HAL_Status(HAL_StatusTypeDef status)
{
    switch (status)
    {
        case HAL_OK:
            return "HAL_OK";
        case HAL_ERROR:
            return "HAL_ERROR";
        case HAL_BUSY:
            return "HAL_BUSY";
        case HAL_TIMEOUT:
            return "HAL_TIMEOUT";
        default:
            return "UNKNOWN HAL_STATUS";
    }
}


/**
  * @brief  Flushes the buffer
  * @param  pBuffer: buffers to be flushed.
  * @param  BufferLength: buffer's length
  * @retval None
  */
void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength)
{
    while (BufferLength--)
    {
        *pBuffer = 0;
    
        pBuffer++;
    }
}


/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 0  : pBuffer1 identical to pBuffer2
  *         >0 : pBuffer1 differs from pBuffer2
  */
uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
    while (BufferLength--)
    {
        if ((*pBuffer1) != *pBuffer2)
        {
        return BufferLength;
        }
        pBuffer1++;
        pBuffer2++;
    }
    
    return 0;
}


/******************************************************************************/
/*                 Error Code and Handlers                                    */
/******************************************************************************/
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    /* Turn BSP_HARD_ERROR_LED on */
    BSP_LED_On(BSP_HARD_ERROR_LED);
        
    while(1)
    {
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    // Alert the user of the file and line number
    printf("Error occured in file %s on line %d\r\n", file, line);
    
    /* Infinite loop */
    while (1)
    {
    }
}
#endif
