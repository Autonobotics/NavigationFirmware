/**
  ******************************************************************************
  * @file    I2C/I2C_TwoBoards_ComPolling/Src/main.c 
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    13-March-2015
  * @brief   This sample code shows how to use STM32F4xx I2C HAL API to transmit 
  *          and receive a data buffer with a communication process based on
  *          Polling transfer. 
  *          The communication is done using 2 Boards.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup I2C_TwoBoards_ComPolling
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define I2C_ADDRESS        0x30

/* Private variables ---------------------------------------------------------*/
/* I2C handler declaration */
I2C_HandleTypeDef I2cHandle;

/* Buffer used for transmission */
static uint8_t aTxBuffer[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};

static uint8_t bTxBuffer[] = {0x03, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* Buffer used for reception */
uint8_t aRxBuffer[RXBUFFERSIZE];

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static uint16_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint16_t BufferLength);
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    int32_t temp = 0;
    uint32_t dis = 0;
    sAPP_PIXARM_READ_DATA* data;
    uint8_t iter = 1;
    HAL_StatusTypeDef stat;
    
    /* STM32F4xx HAL library initialization:
        - Configure the Flash prefetch, instruction and Data caches
        - Configure the Systick to generate an interrupt each 1 msec
        - Set NVIC Group Priority to 4
        - Global MSP (MCU Support Package) initialization
        */
    HAL_Init();
    
    /* Configure LED4, LED5 and LED6 */
    BSP_LED_Init(LED4);
    BSP_LED_Init(LED5);
    BSP_LED_Init(LED6);
    BSP_LED_Init(LED3);
    
    /* Configure the system clock to 168 MHz */
    SystemClock_Config();
    
    /*##-1- Configure the I2C peripheral ######################################*/
    I2cHandle.Instance             = I2Cx;
    
    I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    I2cHandle.Init.ClockSpeed      = 400000;
    I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
    I2cHandle.Init.OwnAddress2     = 0x0;
    
    if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();    
    }
    
    Test_Log_Init();
    
    /* Configure USER Button */
    BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
    
    /* Wait for USER Button press before starting the Communication */
    while (BSP_PB_GetState(BUTTON_KEY) != 1)
    {
    }
    
    /* Wait for USER Button release before starting the Communication */
    while (BSP_PB_GetState(BUTTON_KEY) != 0)
    {
    }
    
    BSP_LED_On(LED3);
    BSP_LED_Off(LED6);
    BSP_LED_Off(LED4);
    
    /* The board sends the message and expects to receive it back */
    Test_Log("Starting I2C Handshake.\r\n");
    
    /*##-2- Start the transmission process #####################################*/  
    /* Timeout is set to 10S */
    while(HAL_I2C_Master_Transmit(&I2cHandle, (uint16_t)I2C_ADDRESS, aTxBuffer, TXBUFFERSIZE, 10000)!= HAL_OK)
    {
        /* Error_Handler() function is called when Timeout error occurs.
        When Acknowledge failure occurs (Slave don't acknowledge it's address)
        Master restarts communication */
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
        {
            Error_Handler();
        }
    }
    
    /*##-3- Put I2C peripheral in reception process ############################*/ 
    /* Timeout is set to 10S */ 
    while((stat = HAL_I2C_Master_Receive(&I2cHandle, (uint16_t)I2C_ADDRESS, aRxBuffer, RXBUFFERSIZE, 10000)) != HAL_OK)
    {
        /* Error_Handler() function is called when Timeout error occurs.
        When Acknowledge failure occurs (Slave don't acknowledge it's address)
        Master restarts communication */
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
        {
        Error_Handler();
        }   
    }
    
    /* Turn LED6 on: Transfer in reception process is correct */
    BSP_LED_On(LED6);
    
    /*##-4- Compare the sent and received buffers ##############################*/
    //if(Buffercmp((uint8_t*)aTxBuffer,(uint8_t*)aRxBuffer,RXBUFFERSIZE))
    //{
    //    /* Processing Error */
    //    Error_Handler();
    //}
    
    Test_Log("Finished I2C Handshaking.\r\n");
    //Flush_Buffer(aRxBuffer, RXBUFFERSIZE);
    
    /* Infinite loop */  
    while (1)
    {
        /*##-2- Start the transmission process #####################################*/  
        /* Timeout is set to 10S */
        Test_Log("Iteration %d\r\n", iter);
        while((stat = HAL_I2C_Master_Transmit(&I2cHandle, (uint16_t)I2C_ADDRESS, bTxBuffer, TXBUFFERSIZE, 10000)) !=  HAL_OK)
        {
            /* Error_Handler() function is called when Timeout error occurs.
            When Acknowledge failure occurs (Slave don't acknowledge it's address)
            Master restarts communication */
            if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
            {
                Test_Log("Error during Transmission. Transmission Error no. %d\r\n", stat);
                Test_Log("Error During Transmission.\r\n");
                Error_Handler();
            }
            //Test_Log("Transmit Failed. Error no. %d. Trying again.\r\n", stat);
        }
        
        while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
        {
        }
        
        /*##-3- Put I2C peripheral in reception process ############################*/ 
        /* Timeout is set to 10S */ 
        while((stat = HAL_I2C_Master_Receive(&I2cHandle, (uint16_t)I2C_ADDRESS, aRxBuffer, RXBUFFERSIZE, 10000)) != HAL_OK)
        {
            /* Error_Handler() function is called when Timeout error occurs.
            When Acknowledge failure occurs (Slave don't acknowledge it's address)
            Master restarts communication */
            if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
            {
                Test_Log("Error during Reception. Receive Error no. %d\r\n", stat);
                Test_Log("Error during Reception. I2C Error no. %d\r\n", HAL_I2C_GetError(&I2cHandle));
                Error_Handler();
            }
            //Test_Log("Receive Failed. Error no. %d. Trying again.\r\n", stat);
        }
        
        while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
        {
        }
        
        // Validate the data
        data = (sAPP_PIXARM_READ_DATA *) aRxBuffer;
        if (data->cmd != 0x04)
        {
            Test_Log("CMD incorrect on packet. Received %x.\r\n", data->cmd);
        }
        if (data->x_intensity != 0)
        {
            Test_Log("X incorrect on packet. Received %d.\r\n", data->x_intensity);
        }
        if (data->y_intensity != 0)
        {
            Test_Log("Y incorrect on packet. Received %d.\r\n", data->y_intensity);
        }
        if (data->z_intensity != 0)
        {
            Test_Log("Z incorrect on packet. Received %d.\r\n", data->z_intensity);
        }
        if (data->rotation_absolute != 0)
        {
            Test_Log("Distance incorrect on packet. Received %d.\r\n", data->rotation_absolute);
        }
        if (data->flag != 0xFF)
        {
            Test_Log("Flag incorrect on packet. Received %x.\r\n", data->flag);
        }
        
        HAL_Delay(10);
        iter++;
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
    /* Turn LED5 on */
    BSP_LED_On(LED5);
    while(1)
    {
    }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    
    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();
    
    /* The voltage scaling allows optimizing the power consumption when the device is 
        clocked below the maximum system frequency, to update the voltage scaling value 
        regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
        clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
    
    /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
    if (HAL_GetREVID() == 0x1001)
    {
        /* Enable the Flash prefetch */
        __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
    }
}

/**
  * @brief  I2C error callbacks.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
    /* Turn LED5 on: Transfer error in reception/transmission process */
    BSP_LED_On(LED5); 
}

/**
  * @brief  Flushes the buffer
  * @param  pBuffer: buffers to be flushed.
  * @param  BufferLength: buffer's length
  * @retval None
  */
void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength)
{
    uint8_t* pBufferTemp = pBuffer;
    while (BufferLength--)
    {
        *pBufferTemp = 0;
    
        pBufferTemp++;
    }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 0  : pBuffer1 identical to pBuffer2
  *         >0 : pBuffer1 differs from pBuffer2
  */
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
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
    /* User can add his own implementation to report the file name and line number,
        ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    
    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
