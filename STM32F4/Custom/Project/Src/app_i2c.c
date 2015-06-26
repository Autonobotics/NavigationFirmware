/**
  ******************************************************************************
  * @file    Src/app_i2c.c 
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    13-March-2015
  * @brief   Application I2C Implementation
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
#include "app_i2c.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Pulbic variables ----------------------------------------------------------*/
I2C_HandleTypeDef I2CxHandle;

/* Private variables ---------------------------------------------------------*/
static sAPP_I2C_CBLK AppI2CCblk = {
    NULL,
    I2C_INIT
    };


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*                 HAL Callback Functions                                     */
/******************************************************************************/
/**
  * @brief  I2C Slave Tx Transfer completed callback.
  * @param  I2cHandle: I2C handle 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    /* Turn LED4 on: Transfer in transmission process is correct */
    BSP_LED_On(LED4);
}

/**
  * @brief  I2C Slave Rx Transfer completed callback.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    /* Turn LED6 on: Transfer in reception process is correct */
    BSP_LED_On(LED6);
}

/**
  * @brief  I2C error callbacks
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
    /* Turn On LED5 */
    BSP_LED_On(LED3);
    while(1)
    {
    }
}


/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Performs initialization of the Application-specific I2C.
  * @param  None
  * @retval None
  */
void APP_I2C_Init(void)
{
    /*##-1- Configure the I2C peripheral #######################################*/
    I2CxHandle.Instance             = I2Cx;
    I2CxHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    I2CxHandle.Init.ClockSpeed      = 400000;
    I2CxHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2CxHandle.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
    I2CxHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2CxHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    I2CxHandle.Init.OwnAddress1     = I2C_ADDRESS;
    I2CxHandle.Init.OwnAddress2     = 0;
    
    if(HAL_I2C_Init(& I2CxHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
    
    AppI2CCblk.handle = & I2CxHandle;
    AppI2CCblk.state = I2C_PROCESS;
}


/* -------------------------- End of File ------------------------------------*/
