/**
  ******************************************************************************
  * @file    Src/app_i2c.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application I2C Implementation
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "app_i2c.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define INPUT_I2C_BUFFER_SIZE (sizeof(AppI2CCblk.inputBuffer.buffer))
#define OUTPUT_I2C_BUFFER_SIZE (sizeof(AppI2CCblk.outputBuffer.buffer))
#define I2C_TRANSACTION_RETRY_LIMIT 5
    
/* Private macro -------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
I2C_HandleTypeDef I2CxHandle;

/* Private variables ---------------------------------------------------------*/
static sAPP_I2C_CBLK AppI2CCblk = {
    NULL,
    I2C_INIT,
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    I2C_INITIAL
};


/* Private function prototypes -----------------------------------------------*/
static eAPP_STATUS i2c_transmit(void);
static eAPP_STATUS i2c_receive(void);
static eAPP_STATUS i2c_handle_request(sAPP_NAVIGATION_CBLK* navigation_cblk);
static eAPP_STATUS i2c_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk);

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
    // Transmission was successful, so ready to listen for a new request
    AppI2CCblk.requestState = I2C_NO_REQUEST;
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
    // Reception was successful, so ready for processing
    AppI2CCblk.requestState = I2C_REQUEST_WAITING;
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
    /* Turn On BSP_I2C_ERROR_LED */
    BSP_LED_On(BSP_I2C_ERROR_LED);
    
    // Log Error and Return Failure
    APP_Log("Generic I2C Error Occured.\r\n");
    Error_Handler();
}


/******************************************************************************/
/*                 Internal Support Functions                                 */
/******************************************************************************/
static eAPP_STATUS i2c_transmit(void)
{
    HAL_StatusTypeDef status;
    uint32_t i2cError;
    uint32_t retryCount = 0;
    
    /* Start the I2C peripheral transmission process */
    AppI2CCblk.requestState = I2C_REQUEST_PROCESSING;
    while ( HAL_OK != (status = HAL_I2C_Slave_Transmit_IT(AppI2CCblk.handle, AppI2CCblk.outputBuffer.buffer, OUTPUT_I2C_BUFFER_SIZE)))
    {
        if (HAL_I2C_ERROR_AF != (i2cError = HAL_I2C_GetError(AppI2CCblk.handle)))
        {
            // Log Error and return failed
            APP_Log("Error during I2C transmission. IT Status: %d.  I2C Error: %d.\r\n", status, i2cError);
            return STATUS_FAILURE;
        }
        
        retryCount++;
        if ( retryCount > I2C_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("Error during I2C transmission. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}

static eAPP_STATUS i2c_receive(void)
{
    HAL_StatusTypeDef status;
    uint32_t i2cError;
    uint32_t retryCount = 0;
    
    /* Put I2C peripheral in reception mode */ 
    Flush_Buffer(AppI2CCblk.inputBuffer.buffer, INPUT_I2C_BUFFER_SIZE);
    Flush_Buffer(AppI2CCblk.outputBuffer.buffer, OUTPUT_I2C_BUFFER_SIZE);
    AppI2CCblk.requestState = I2C_WAITING;
    while( HAL_OK != (status = HAL_I2C_Slave_Receive_IT(AppI2CCblk.handle, AppI2CCblk.inputBuffer.buffer, INPUT_I2C_BUFFER_SIZE)))
    {
        if (HAL_I2C_ERROR_AF != (i2cError = HAL_I2C_GetError(AppI2CCblk.handle)))
        {
            // Log Error and Return Failure
            APP_Log("Error during I2C reception. IT Status: %d.  I2C Error: %d.\r\n", status, i2cError);
            Error_Handler();
        }
        
        retryCount++;
        if ( retryCount > I2C_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("Error during I2C reception. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}

/******************************************************************************/
/*                 State Machine Management Functions                         */
/******************************************************************************/
static eAPP_STATUS i2c_handle_request(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status;
    uAPP_PIXARM_MESSAGES request = AppI2CCblk.inputBuffer;
    
    switch (request.common.cmd)
    {
        case PIXARM_CMD_READ_REQ:
            // Process the Read Req, pulling out Rotation Completion Data
            if ( APP_Navigation_Check_Rotation(request.readReq.rotation_absolute, 
                                               navigation_cblk->navigation_data.rotation_absolute) )
            {
                navigation_cblk->navigation_flags.rotation_status = TRUE;
            }
        
            // Send back a Read Data
            AppI2CCblk.outputBuffer.readData.cmd = PIXARM_CMD_READ_DATA;
            AppI2CCblk.outputBuffer.readData.x_intensity = navigation_cblk->navigation_data.x_axis;
            AppI2CCblk.outputBuffer.readData.y_intensity = navigation_cblk->navigation_data.y_axis;
            AppI2CCblk.outputBuffer.readData.z_intensity = navigation_cblk->navigation_data.z_axis;
            AppI2CCblk.outputBuffer.readData.rotation_absolute = navigation_cblk->navigation_data.rotation_absolute;
            AppI2CCblk.outputBuffer.readData.flag = PIXARM_FLAG_END;
        
            // Transmit Response
            status = i2c_transmit();
            return status;
        
        case PIXARM_CMD_SYNC:
        case PIXARM_CMD_DYNC:
        case PIXARM_CMD_ACK:
        case PIXARM_CMD_READ_DATA:
        default:
            APP_Log("Recieved Invalid Command %x in state %x.\r\n", request.common.cmd, AppI2CCblk.state);
            AppI2CCblk.state = I2C_ERROR;
            return STATUS_FAILURE;
    }
    
}


static eAPP_STATUS i2c_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch( AppI2CCblk.state )
    {   
        case I2C_PROCESS:
            status = i2c_handle_request(navigation_cblk);
            return status;

        case I2C_HANDSHAKE:
        case I2C_SHUTDOWN:
        case I2C_INIT:
        case I2C_ERROR:
        default:
            APP_Log("I2C APP Driver in Bad State %d.\r\n", AppI2CCblk.state);
            AppI2CCblk.state = I2C_ERROR;
            return STATUS_FAILURE;
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
    
    /* Put I2C peripheral in reception process */ 
    if ( STATUS_FAILURE != i2c_receive() )
    {
        return STATUS_SUCCESS;
    }
    else
    {
        APP_Log("Error in reception call after Handshake.\r\n");
        AppI2CCblk.state = I2C_ERROR;
        BSP_LED_On(BSP_I2C_ERROR_LED);
        return STATUS_FAILURE;
    }
}


eAPP_STATUS APP_I2C_Process_Message(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    // If we are not in an invalid state
    if ( (I2C_INIT != AppI2CCblk.state)
      && (I2C_ERROR != AppI2CCblk.state ) )
    {
        // If a message is waiting, process it
        if ( I2C_REQUEST_WAITING == AppI2CCblk.requestState )
        {
            // Process Message
            status = i2c_state_machine(navigation_cblk);
        }
        else if ( I2C_NO_REQUEST == AppI2CCblk.requestState )
        {
            // Enter the Reception state
            status = i2c_receive();
        }
    }
    // Signal any failures via LED
    if ( STATUS_FAILURE == status )
    {
        BSP_LED_On(BSP_I2C_ERROR_LED);
    }
    return status;
}

/* -------------------------- End of File ------------------------------------*/
