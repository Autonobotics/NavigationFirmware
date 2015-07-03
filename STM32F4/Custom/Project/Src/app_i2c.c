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
#define INPUT_I2C_BUFFER_SIZE (sizeof(AppI2CCblk.inputBuffer.buffer))
#define OUTPUT_I2C_BUFFER_SIZE (sizeof(AppI2CCblk.outputBuffer.buffer))
    
/* Private macro -------------------------------------------------------------*/

/* Pulbic variables ----------------------------------------------------------*/
I2C_HandleTypeDef I2CxHandle;

/* Private variables ---------------------------------------------------------*/
static sAPP_I2C_CBLK AppI2CCblk = {
    NULL,
    I2C_INIT,
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
     0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
     0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
     0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
     0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}
    };


/* Private function prototypes -----------------------------------------------*/
//static eAPP_STATUS i2c_transmit(void);
//static eAPP_STATUS i2c_receive(void);
static eAPP_STATUS i2c_handle_request(void);
static void i2c_async_state_machine(void);

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
    BSP_LED_On(LED6);
    
    /*##-2- Put I2C peripheral in reception process ############################*/  
    while( HAL_OK != HAL_I2C_Slave_Receive_IT(AppI2CCblk.handle, AppI2CCblk.inputBuffer.buffer, INPUT_I2C_BUFFER_SIZE))
    {
        if (HAL_I2C_ERROR_AF != HAL_I2C_GetError(AppI2CCblk.handle))
        {
            // Log Error and Return Failure
            APP_Log("Error on interrupt reception for I2C Interrupt process initiation.\r\n");
            Error_Handler();
        }
    }
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
    BSP_LED_On(LED4);
    
    // Call the I2C Statemachine
    i2c_async_state_machine();
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
    /* Turn On LED3 */
    BSP_LED_On(LED3);
    
    // Log Error and Return Failure
    APP_Log("Generic I2C Error Occured.\r\n");
    Error_Handler();
}

static eAPP_STATUS i2c_transmit(void)
{
    
}

static eAPP_STATUS i2c_receive(void)
{
    
}

/******************************************************************************/
/*                 State Machine Management Functions                         */
/******************************************************************************/
static eAPP_STATUS i2c_handle_request()
{
    uAPP_PIXARM_MESSAGES request = AppI2CCblk.inputBuffer;
    
    switch (request.common.cmd)
    {
        case PIXARM_CMD_READ_REQ:
            // Process the Read Req and send back a Read Data
            AppI2CCblk.outputBuffer.readData.cmd = PIXARM_CMD_READ_DATA;
            AppI2CCblk.outputBuffer.readData.x = 32;
            AppI2CCblk.outputBuffer.readData.y = -2;
            AppI2CCblk.outputBuffer.readData.z = 40;
            AppI2CCblk.outputBuffer.readData.distance = 300;
            AppI2CCblk.outputBuffer.readData.flag = PIXARM_FLAG_END;
        
            while (HAL_I2C_GetState(&I2CxHandle) != HAL_I2C_STATE_READY)
            {
            }
        
            while ( HAL_OK != HAL_I2C_Slave_Transmit_IT(AppI2CCblk.handle, AppI2CCblk.outputBuffer.buffer, OUTPUT_I2C_BUFFER_SIZE))
            {
                if (HAL_I2C_ERROR_AF != HAL_I2C_GetError(AppI2CCblk.handle))
                {
                    // Log Error and return failed
                    APP_Log("Error in transmission of READ DATA.\r\n");
                    return STATUS_FAILURE;
                }
            }
            
            return STATUS_SUCCESS;
        
        case PIXARM_CMD_SYNC:
        case PIXARM_CMD_DYNC:
        case PIXARM_CMD_ACK:
        case PIXARM_CMD_READ_DATA:
        default:
            APP_Log("Recieved Invalid Command %x.\r\n", request.common.cmd);
            AppI2CCblk.state = I2C_ERROR;
            return STATUS_FAILURE;
    }
}

static void i2c_async_state_machine()
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch( AppI2CCblk.state )
    {
        case I2C_PROCESS:
            status = i2c_handle_request();
            break;
        
        case I2C_HANDSHAKE:
        case I2C_SHUTDOWN:
        case I2C_INIT:
        case I2C_ERROR:
        default:
            APP_Log("I2C APP Driver in Bad State %d.\r\n", AppI2CCblk.state);
            AppI2CCblk.state = I2C_ERROR;
            break;
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
    I2CxHandle.Init.OwnAddress2     = 0x0;
    
    if(HAL_I2C_Init(& I2CxHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
    
    AppI2CCblk.handle = & I2CxHandle;
    AppI2CCblk.state = I2C_HANDSHAKE;
}


eAPP_STATUS APP_I2C_Initiate(void)
{
    uAPP_PIXARM_MESSAGES message;
    uint8_t errorCount = 0;
    
    // Validate we are in correct state
    if ( I2C_HANDSHAKE != AppI2CCblk.state )
    {
        APP_Log("I2C in incorrect state for Handshake Procedure.\r\n");
        return STATUS_FAILURE;
    }
    
    // Start the Handshake Procedure (Syncronous Process)
    APP_Log("Starting I2C Handshake.\r\n");
    
    // Send the Packet
    do {
        if (HAL_OK != HAL_I2C_Slave_Receive(AppI2CCblk.handle, AppI2CCblk.inputBuffer.buffer, INPUT_I2C_BUFFER_SIZE, I2C_POLL_TIMEOUT))
        {
            APP_Log("Error in reception of SYNC.\r\n");
            errorCount++;
        }
        else
        {
            break;
        }
        if ( I2C_CONNECTION_ATTEMPTS == errorCount)
        {
            // Log Error and Return Failed
            APP_Log("Error in reception of SYNC, abandoning Process.\r\n");
            return STATUS_FAILURE;
        }
        
    } while( errorCount < I2C_CONNECTION_ATTEMPTS );
    
    // Process the Handshake SYNC Packet
    message.sync = AppI2CCblk.inputBuffer.sync;
    
    // Validate the CMD
    if ( PIXARM_CMD_SYNC != message.sync.cmd )
    {
        // Log Failure and return
        APP_Log("CMD Bits wrong on SYNC Command.\r\n");
        return STATUS_FAILURE;
    }
    
    // Validate the Flag as end
    if ( PIXARM_FLAG_END != message.sync.flag )
    {
        // Log Failure and return
        APP_Log("FLAG Bits wrong on SYNC Command.\r\n");
        return STATUS_FAILURE;
    }
    
    // Format the Handshake SYNC Packet
    AppI2CCblk.outputBuffer.sync.cmd = PIXARM_CMD_SYNC;
    AppI2CCblk.outputBuffer.sync.flag = PIXARM_FLAG_END;
    
    if ( HAL_OK != HAL_I2C_Slave_Transmit(AppI2CCblk.handle, AppI2CCblk.outputBuffer.buffer, OUTPUT_I2C_BUFFER_SIZE, I2C_POLL_TIMEOUT))
    {
        // Log Error and return failed
        APP_Log("Error in transmission of SYNC.\r\n");
        return STATUS_FAILURE;
    }
    
    // Change states
    APP_Log("Finished I2C Handshake, starting Interrupt Process.\r\n");
    AppI2CCblk.state = I2C_PROCESS;
    
    /*##-2- Put I2C peripheral in reception process ############################*/ 
    Flush_Buffer(AppI2CCblk.inputBuffer.buffer, sizeof(AppI2CCblk.inputBuffer.buffer));
    if( HAL_OK != HAL_I2C_Slave_Receive_IT(AppI2CCblk.handle, AppI2CCblk.inputBuffer.buffer, INPUT_I2C_BUFFER_SIZE))
    {
        // Log Error and Return Failure
        APP_Log("Error on interrupt reception for I2C Interrupt process initiation.\r\n");
        return STATUS_FAILURE;
    }
    
    return STATUS_SUCCESS;
}

HAL_I2C_StateTypeDef APP_I2C_GetState(void)
{
    return HAL_I2C_GetState(AppI2CCblk.handle);
}

/* -------------------------- End of File ------------------------------------*/
