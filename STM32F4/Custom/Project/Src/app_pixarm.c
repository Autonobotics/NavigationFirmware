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
#include "app_pixarm.h"
#include "app_uart_generic.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define INPUT_PIXARM_BUFFER_SIZE (sizeof(AppPixarmCblk.inputBuffer.buffer))
#define OUTPUT_PIXARM_BUFFER_SIZE (sizeof(AppPixarmCblk.outputBuffer.buffer))
#define PIXARM_TRANSACTION_RETRY_LIMIT 5
    
/* Private macro -------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
UART_HandleTypeDef PixarmHandle;

/* Private variables ---------------------------------------------------------*/
static sAPP_PIXARM_CBLK AppPixarmCblk = {
    NULL,
    PIXARM_INIT,
    PIXARM_INIT,
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    UART_INITIAL
};


/* Private function prototypes -----------------------------------------------*/
static eAPP_STATUS pixarm_transmit(void);
static eAPP_STATUS pixarm_receive(void);
static eAPP_STATUS pixarm_handle_request(sAPP_NAVIGATION_CBLK* navigation_cblk);
static eAPP_STATUS pixarm_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk);

/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*                 Internal Support Functions                                 */
/******************************************************************************/
static eAPP_STATUS pixarm_transmit(void)
{
    HAL_StatusTypeDef status;
    uint32_t retryCount = 0;
    
    /* Start the UART peripheral transmission process */
    AppPixarmCblk.requestState = UART_REQUEST_PROCESSING;
    while ( HAL_OK != (status = HAL_UART_Transmit_IT(AppPixarmCblk.handle, 
                                                     AppPixarmCblk.outputBuffer.buffer, 
                                                     OUTPUT_PIXARM_BUFFER_SIZE)))
    {
        retryCount++;
        if ( retryCount > PIXARM_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("PIXARM: Error during UART transmission. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}

static eAPP_STATUS pixarm_receive(void)
{
    HAL_StatusTypeDef status;
    uint32_t retryCount = 0;
    
    /* Put UART peripheral in reception mode */ 
    Flush_Buffer(AppPixarmCblk.inputBuffer.buffer, INPUT_PIXARM_BUFFER_SIZE);
    Flush_Buffer(AppPixarmCblk.outputBuffer.buffer, OUTPUT_PIXARM_BUFFER_SIZE);
    APP_UART_Generic_Flush_Buffer(AppPixarmCblk.handle);
    AppPixarmCblk.requestState = UART_WAITING;
    while( HAL_OK != (status = HAL_UART_Receive_IT(AppPixarmCblk.handle, 
                                                   AppPixarmCblk.inputBuffer.buffer,
                                                   INPUT_PIXARM_BUFFER_SIZE)))
    {
        retryCount++;
        if ( retryCount > PIXARM_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("PIXARM: Error during UART reception. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}

/******************************************************************************/
/*                 State Machine Management Functions                         */
/******************************************************************************/
static eAPP_STATUS pixarm_handle_request(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status;
    uAPP_PIXARM_MESSAGES request = AppPixarmCblk.inputBuffer;
    
    switch (request.common.cmd)
    {
        case PIXARM_CMD_READ_REQ:
            // Validate the Flag as end
            if ( PIXARM_FLAG_END != request.readReq.flag )
            {
                // Log Failure and return
                APP_Log("PIXARM: FLAG Bits wrong on READ_REQ Command.\r\n");
                return STATUS_FAILURE;
            }
            
            APP_Log("PIXARM: Received CMD_READ_REQ."ENDLINE);
        
            // Process the Read Req, pulling out Rotation Completion Data
            if ( ROTATION_UNKNOWN != navigation_cblk->image_board_data.rotation )
            {
                if ( APP_Navigation_Check_Rotation(request.readReq.rotation_absolute, 
                                               navigation_cblk->navigation_data.rotation_absolute) )
                {
                    // If the previous rotation finished, override this cycles rotation calculation
                    navigation_cblk->navigation_flags.rotation_status = TRUE;
                    navigation_cblk->image_board_data.rotation = ROTATION_UNKNOWN;
                    navigation_cblk->navigation_data.rotation_absolute = ROTATION_UNKNOWN;
                }
            }
        
            // Send back a Read Data
            AppPixarmCblk.outputBuffer.readData.cmd = PIXARM_CMD_READ_DATA;
            AppPixarmCblk.outputBuffer.readData.x_intensity = navigation_cblk->navigation_data.x_axis;
            AppPixarmCblk.outputBuffer.readData.y_intensity = navigation_cblk->navigation_data.y_axis;
            AppPixarmCblk.outputBuffer.readData.z_intensity = navigation_cblk->navigation_data.z_axis;
            AppPixarmCblk.outputBuffer.readData.rotation_absolute = navigation_cblk->navigation_data.rotation_absolute;
            AppPixarmCblk.outputBuffer.readData.flag = PIXARM_FLAG_END;
            
            APP_Log("PIXARM: Sending CMD_READ_DATA."ENDLINE);
        
            // Transmit Response
            status = pixarm_transmit();
            return status;
        
        case PIXARM_CMD_SYNC:
        case PIXARM_CMD_DYNC:
        case PIXARM_CMD_ACK:
        case PIXARM_CMD_READ_DATA:
        default:
            APP_Log("PIXARM: Recieved Invalid Command %x in state %x.\r\n", request.common.cmd, AppPixarmCblk.state);
            AppPixarmCblk.state = PIXARM_ERROR;
            return STATUS_FAILURE;
    }
    
}


static eAPP_STATUS pixarm_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch( AppPixarmCblk.state )
    {   
        case PIXARM_PROCESS:
            status = pixarm_handle_request(navigation_cblk);
            return status;

        case PIXARM_HANDSHAKE:
        case PIXARM_SHUTDOWN:
        case PIXARM_INIT:
        case PIXARM_ERROR:
        default:
            APP_Log("PIXARM Driver in Bad State %d.\r\n", AppPixarmCblk.state);
            AppPixarmCblk.state = PIXARM_ERROR;
            return STATUS_FAILURE;
    }
}


/* Public functions ---------------------------------------------------------*/
/******************************************************************************/
/*                 Callback Functions                                     */
/******************************************************************************/
/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void PIXARM_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // Transmission was successful, so ready to listen for a new request
    AppPixarmCblk.requestState = UART_NO_REQUEST;
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void PIXARM_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // Reception was successful, so ready for processing
    AppPixarmCblk.requestState = UART_REQUEST_WAITING;
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void PIXARM_UART_ErrorCallback(UART_HandleTypeDef *huart)
{    /* Turn On BSP_PIXARM_ERROR_LED */
    BSP_LED_On(BSP_PIXARM_ERROR_LED);
    AppPixarmCblk.state = PIXARM_TRANSITION_TO_ERROR;
}


/**
  * @brief  Performs initialization of the Application-specific I2C.
  * @param  None
  * @retval None
  */
void APP_PIXARM_Init(void)
{
    /*##-1- Configure the UART peripheral #######################################*/
    PixarmHandle.Instance          = PIXARM_USART;
    PixarmHandle.Init.BaudRate     = 115200;
    PixarmHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    PixarmHandle.Init.StopBits     = UART_STOPBITS_1;
    PixarmHandle.Init.Parity       = UART_PARITY_NONE;
    PixarmHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    PixarmHandle.Init.Mode         = UART_MODE_TX_RX;
    PixarmHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if(HAL_UART_Init(& PixarmHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
    
    AppPixarmCblk.handle = & PixarmHandle;
    AppPixarmCblk.state = PIXARM_HANDSHAKE;
}


eAPP_STATUS APP_PIXARM_Initiate(void)
{
    uAPP_PIXARM_MESSAGES message;
    uint32_t errorCount = 0;
    
    // Validate we are in correct state
    if ( PIXARM_HANDSHAKE != AppPixarmCblk.state )
    {
        APP_Log("PIXARM in incorrect state for Handshake Procedure.\r\n");
        return STATUS_FAILURE;
    }
    
    // Start the Handshake Procedure (Syncronous Process)
    APP_Log("Starting PIXARM Handshake.\r\n");
    
    // Start Handshake Loop
    do {
        // Send the Packet
        do {
            if (HAL_OK != HAL_UART_Receive(AppPixarmCblk.handle, 
                                        AppPixarmCblk.inputBuffer.buffer, 
                                        INPUT_PIXARM_BUFFER_SIZE,
                                        PIXARM_POLL_TIMEOUT))
            {
                APP_Log("PIXARM: Error in reception of SYNC.\r\n");
                errorCount++;
            }
            else
            {
                break;
            }
            if ( (uint32_t) PIXARM_CONNECTION_ATTEMPTS <= errorCount)
            {
                // Log Error and Return Failed
                APP_Log("PIXARM: Error in reception of SYNC, abandoning Process.\r\n");
                return STATUS_FAILURE;
            }
            
        } while( errorCount < (uint32_t) PIXARM_CONNECTION_ATTEMPTS );
        
        // Process the Handshake SYNC Packet
        message.sync = AppPixarmCblk.inputBuffer.sync;
        
        // Validate the CMD
        if ( PIXARM_CMD_SYNC != message.sync.cmd )
        {
            // Log Failure and return
            APP_Log("PIXARM: CMD Bits wrong on SYNC Command.\r\n");
            Flush_Buffer(AppPixarmCblk.inputBuffer.buffer, INPUT_PIXARM_BUFFER_SIZE);
            continue;
        }
        
        // Validate the Flag as end
        if ( PIXARM_FLAG_END != message.sync.flag )
        {
            // Log Failure and return
            APP_Log("PIXARM: FLAG Bits wrong on SYNC Command.\r\n");
            Flush_Buffer(AppPixarmCblk.inputBuffer.buffer, INPUT_PIXARM_BUFFER_SIZE);
            continue;
        }
        
        // Format the Handshake SYNC Packet
        AppPixarmCblk.outputBuffer.sync.cmd = PIXARM_CMD_SYNC;
        AppPixarmCblk.outputBuffer.sync.flag = PIXARM_FLAG_END;
        
        if ( HAL_OK != HAL_UART_Transmit(AppPixarmCblk.handle, 
                                        AppPixarmCblk.outputBuffer.buffer, 
                                        OUTPUT_PIXARM_BUFFER_SIZE, 
                                        PIXARM_POLL_TIMEOUT))
        {
            // Log Error and return failed
            APP_Log("PIXARM: Error in transmission of SYNC.\r\n");
            Flush_Buffer(AppPixarmCblk.outputBuffer.buffer, OUTPUT_PIXARM_BUFFER_SIZE);
            continue;
        }
        else
        {
            break;
        }
        
    } while (TRUE);
    
    // Change states
    APP_Log("Finished PIXARM Handshake, starting Interrupt Process.\r\n");
    AppPixarmCblk.state = PIXARM_PROCESS;
    
    /* Put PIXARM peripheral in reception process */ 
    if ( STATUS_FAILURE != pixarm_receive() )
    {
        return STATUS_SUCCESS;
    }
    else
    {
        APP_Log("PIXARM: Error in reception call after Handshake.\r\n");
        AppPixarmCblk.state = PIXARM_ERROR;
        BSP_LED_On(BSP_PIXARM_ERROR_LED);
        return STATUS_FAILURE;
    }
}


eAPP_STATUS APP_PIXARM_Process_Message(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    // If we are not in an invalid state
    switch ( AppPixarmCblk.state )
    {
        case PIXARM_HANDSHAKE:
        case PIXARM_PROCESS:
        case PIXARM_SHUTDOWN:
            // If a message is waiting, process it
            if ( UART_REQUEST_WAITING == AppPixarmCblk.requestState )
            {
                // Process Message
                status = pixarm_state_machine(navigation_cblk);
            }
            else if ( UART_NO_REQUEST == AppPixarmCblk.requestState )
            {
                // Enter the Reception state
                status = pixarm_receive();
            }
            break;
            
        case PIXARM_TRANSITION_TO_ERROR:
            // Log Error and Return Failure
            APP_Log("PIXARM: UART Error Occured: %s. Transitioning to Error.\r\n",
            APP_UART_Generic_Translate_Error(AppPixarmCblk.handle->ErrorCode));
            AppPixarmCblk.state = PIXARM_ERROR;
            break;
        
        case PIXARM_INIT:
            // Do nothing
            break;
        
        case PIXARM_ERROR:
        default:
            status = STATUS_FAILURE;
            break;
    }
    
    // Signal any failures via LED
    if ( STATUS_FAILURE == status )
    {
        BSP_LED_On(BSP_PIXARM_ERROR_LED);
        AppPixarmCblk.state = PIXARM_ERROR;
    }
    return status;
}

/* -------------------------- End of File ------------------------------------*/
