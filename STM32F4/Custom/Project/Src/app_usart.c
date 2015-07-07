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
#include "app_usart.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define UART_ACCEPT_SYNC
#define INPUT_UART_BUFFER_SIZE (sizeof(AppUsartCblk.inputBuffer))
#define OUTPUT_UART_BUFFER_SIZE (sizeof(AppUsartCblk.outputBuffer))
#define UART_TRANSACTION_RETRY_LIMIT 5

/* Private macro -------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
UART_HandleTypeDef UartHandle;

/* Private variables ---------------------------------------------------------*/
static sAPP_USART_CBLK AppUsartCblk = {
    NULL,
    UART_INIT,
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
     0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
     0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    UART_INITIAL
};


/* Private function prototypes -----------------------------------------------*/
static eAPP_STATUS uart_receive(void);
static eAPP_STATUS uart_transmit(void);
static eAPP_STATUS uart_send_response(sAPP_NAVIGATION_CBLK* navigation_cblk);
static eAPP_STATUS uart_handle_handshake(uAPP_USART_MESSAGES message);
static eAPP_STATUS uart_handle_data_receive(sAPP_NAVIGATION_CBLK* navigation_cblk,
                                            uAPP_USART_MESSAGES message);
static eAPP_STATUS uart_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk);


/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*                 HAL Callback Functions                                     */
/******************************************************************************/
/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // Transmission was successful, so ready to listen for a new request
    AppUsartCblk.requestState = UART_NO_REQUEST;
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
    // Reception was successful, so ready for processing
    AppUsartCblk.requestState = UART_REQUEST_WAITING;
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
    /* Turn BSP_UART_ERROR_LED */
    BSP_LED_On(BSP_UART_ERROR_LED);
    
    // Log Error and Return Failure
    APP_Log("Generic UART Error Occured.\r\n");
    Error_Handler();
}

/******************************************************************************/
/*                 Internal Support Functions                                 */
/******************************************************************************/
static eAPP_STATUS uart_transmit(void)
{
    HAL_StatusTypeDef status;
    uint32_t retryCount = 0;
    
    /* Start the UART peripheral transmission process */
    AppUsartCblk.requestState = UART_REQUEST_PROCESSING;
    while ( HAL_OK != (status = HAL_UART_Transmit_IT(AppUsartCblk.handle, AppUsartCblk.outputBuffer.buffer, OUTPUT_UART_BUFFER_SIZE)))
    {
        retryCount++;
        if ( retryCount > UART_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("Error during UART transmission. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}

static eAPP_STATUS uart_receive(void)
{
    HAL_StatusTypeDef status;
    uint32_t retryCount = 0;
    
    /* Put I2C peripheral in reception mode */ 
    Flush_Buffer(AppUsartCblk.inputBuffer.buffer, INPUT_UART_BUFFER_SIZE);
    Flush_Buffer(AppUsartCblk.outputBuffer.buffer, OUTPUT_UART_BUFFER_SIZE);
    AppUsartCblk.requestState = UART_WAITING;
    while( HAL_OK != (status = HAL_UART_Receive_IT(AppUsartCblk.handle, AppUsartCblk.inputBuffer.buffer, INPUT_UART_BUFFER_SIZE)))
    {
        retryCount++;
        if ( retryCount > UART_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("Error during UART reception. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}


/******************************************************************************/
/*                 State Machine Management Functions                         */
/******************************************************************************/
static eAPP_STATUS uart_send_response(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status;
    uint16_t distance;
    
    // Format the RACK Packet by default
    AppUsartCblk.outputBuffer.rack.cmd = ARMPIT_CMD_RACK;
    AppUsartCblk.outputBuffer.rack.flag = ARMPIT_FLAG_END;
    
    // Read distance on frontal Ultrasonic
    distance = navigation_cblk->hc_sr04_data.distance[AXIS_FRONT];
    
    // If we have something to tell the Image Board, send RACK, else just ACK
    if ( ROTATION_COMPLETE ==  navigation_cblk->navigation_flags.rotation_status )
    {
        AppUsartCblk.outputBuffer.rack.sub_cmd = ARMPIT_SUBCMD_ROTATION_COMPLETE;
        // Reset the Rotation Flag
        navigation_cblk->navigation_flags.rotation_status = ROTATION_INCOMPLETE;
    }
    else if ( HC_SR04_OUT_OF_RANGE != distance )
    {
        AppUsartCblk.outputBuffer.rack.sub_cmd = ARMPIT_SUBCMD_COLLISION_DETECTED;
        AppUsartCblk.outputBuffer.rack.axis = AXIS_FRONT; // No other possible value currently
        AppUsartCblk.outputBuffer.rack.payload_a = distance;
    }
    else
    {
         // Format the ACK Packet
         AppUsartCblk.outputBuffer.ack.cmd = ARMPIT_CMD_ACK;
         AppUsartCblk.outputBuffer.ack.flag = ARMPIT_FLAG_END;
    }
    
    // Transmit and setup for receive
    status = uart_transmit();
    return status;
}

static eAPP_STATUS uart_handle_handshake(uAPP_USART_MESSAGES message)
{
    eAPP_STATUS status = STATUS_FAILURE;
    HAL_StatusTypeDef halStatus;
    
    switch (message.common.cmd)
    {
        // Invalid 0x00 is used to start Handshake Procedure
        case ARMPIT_CMD_INVD:
            // Even if Receive fails, it was a successful attempt
            status = STATUS_SUCCESS;
        
            // Format the Handshake SYNC Packet
            AppUsartCblk.outputBuffer.sync.cmd = ARMPIT_CMD_SYNC;
            AppUsartCblk.outputBuffer.sync.flag = ARMPIT_FLAG_END;
        
            if ( HAL_OK == (halStatus = HAL_UART_Transmit(AppUsartCblk.handle, AppUsartCblk.outputBuffer.buffer, OUTPUT_UART_BUFFER_SIZE, UART_POLL_TIMEOUT)))
            {
                Flush_Buffer(AppUsartCblk.outputBuffer.buffer, OUTPUT_UART_BUFFER_SIZE);
                
                halStatus = HAL_UART_Receive(AppUsartCblk.handle, AppUsartCblk.inputBuffer.buffer, INPUT_UART_BUFFER_SIZE, UART_POLL_TIMEOUT);
                if ( HAL_OK == halStatus)
                {
                    AppUsartCblk.requestState = UART_REQUEST_WAITING;
                }
            }
            
            // Ensure that there was no major error with the UART on our end
            if ( HAL_OK != halStatus
              && HAL_TIMEOUT != halStatus )
            {
                APP_Log("An error occured with the UART on handshake. Entering Error State.\r\n");
                status = STATUS_FAILURE;
            }
            return status;
        
        case ARMPIT_CMD_SYNC:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.sync.flag )
            {
                // Log Failure and return
                APP_Log("FLAG Bits wrong on SYNC Command.\r\n");
                return STATUS_FAILURE;
            }
        
            // Format the Handshake ACK Packet
            AppUsartCblk.outputBuffer.ack.cmd = ARMPIT_CMD_ACK;
            AppUsartCblk.outputBuffer.ack.flag = ARMPIT_FLAG_END;
        
            // Transmit and setup for receive
            status = uart_transmit();
            if ( STATUS_SUCCESS == status)
            {
                // Change states
                APP_Log("Finished UART Handshake, starting Interrupt Process.\r\n");
                AppUsartCblk.state = UART_DATA_RECEIVE;
            }
            return status;
        
        case ARMPIT_CMD_DYNC:
        case ARMPIT_CMD_ACK:
        default:
            return status;
        
    }
}

static eAPP_STATUS uart_handle_data_receive(sAPP_NAVIGATION_CBLK* navigation_cblk, 
                                            uAPP_USART_MESSAGES message)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch ( message.common.cmd )
    {
        case ARMPIT_CMD_NO_BEACON:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.no_beacon.flag )
            {
                // Log Failure and return
                APP_Log("FLAG Bits wrong on NO_BEACON Command.\r\n");
                return STATUS_FAILURE;
            }
            
            // Set Data Variables here
            // TODO:
            

            // Transmit Response
            status = uart_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_BEACON_DETECTED:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.no_beacon.flag )
            {
                // Log Failure and return
                APP_Log("FLAG Bits wrong on BEACON_DETECTED Command.\r\n");
                return STATUS_FAILURE;
            }
            
            // Set Data Variables here
            // TODO:
            

            // Transmit Response
            status = uart_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_EDGE_DETECTED:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.no_beacon.flag )
            {
                // Log Failure and return
                APP_Log("FLAG Bits wrong on EDGE_DETECTED Command.\r\n");
                return STATUS_FAILURE;
            }
            
            // Set Data Variables here
            // TODO:
            
            
            // Transmit Response
            status = uart_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_BEACON_ROTATION:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.beacon_rotation.flag )
            {
                // Log Failure and return
                APP_Log("FLAG Bits wrong on BEACON_ROTATION Command.\r\n");
                return STATUS_FAILURE;
            }
            
            // Set Data Variables here
            // TODO:
            
            
            // Transmit Response
            status = uart_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_QUERY_ROTATION:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.query_rotation.flag )
            {
                // Log Failure and return
                APP_Log("FLAG Bits wrong on QUERY_ROTATION Command.\r\n");
                return STATUS_FAILURE;
            }
            
            // Set Data Variables here
            // TODO:
            
            
            // Transmit Response
            status = uart_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_SYNC:
#ifdef UART_ACCEPT_SYNC
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.sync.flag )
            {
                // Log Failure and return
                APP_Log("FLAG Bits wrong on SYNC Command.\r\n");
                return STATUS_FAILURE;
            }
        
            // Transmit Response
            status = uart_send_response(navigation_cblk);
            return status;
#endif // #ifdef UART_ACCEPT_SYNC
        case ARMPIT_CMD_INVD:
        case ARMPIT_CMD_DYNC:
        case ARMPIT_CMD_ACK:
        case ARMPIT_CMD_RACK:
        default:
            APP_Log("UART APP Driver state received bad command. Command: %x\r\n", message.common.cmd);
            AppUsartCblk.state = UART_ERROR;
            return STATUS_FAILURE;
    }
}

static eAPP_STATUS uart_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch( AppUsartCblk.state )
    {
        case UART_HANDSHAKE:
            status = uart_handle_handshake(AppUsartCblk.inputBuffer);
            return status;
        
        case UART_DATA_RECEIVE:
            status = uart_handle_data_receive(navigation_cblk, AppUsartCblk.inputBuffer);
            return status;
        
        case UART_TERMINATE:
        case UART_INIT:
        case UART_ERROR:
        default:
            APP_Log("UART APP Driver in Bad State.\r\n");
            AppUsartCblk.state = UART_ERROR;
            return status;
    }
}


/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Performs initialization of the Application-specific UART.
  * @param  None
  * @retval None
  */
void APP_USART_Init(void)
{
    /*##-2- Configure the UART peripheral ######################################*/
    /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
    /* UART1 configured as follow:
        - Word Length = 8 Bits
        - Stop Bit = One Stop bit
        - Parity = None
        - BaudRate = 115200 baud
        - Hardware flow control disabled (RTS and CTS signals)
    */
    UartHandle.Instance          = USARTx;
    UartHandle.Init.BaudRate     = 115200;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(& UartHandle) != HAL_OK)
    {
        Error_Handler();
    }
    
    AppUsartCblk.handle = & UartHandle;
    AppUsartCblk.state = UART_HANDSHAKE;
    AppUsartCblk.requestState = UART_TRANSMITING;
}

eAPP_STATUS APP_UART_Initiate(void)
{
    // Validate we are in correct state
    if ( UART_HANDSHAKE != AppUsartCblk.state )
    {
        APP_Log("UART in incorrect state for Handshake Procedure.\r\n");
        return STATUS_FAILURE;
    }
    
    // Start the Handshake Procedure (Asychronous Process)
    APP_Log("Starting UART Handshake.\r\n");
    return STATUS_SUCCESS;
    
}

eAPP_STATUS APP_UART_Process_Message(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    // If we are not in an invalid state
    if ( (UART_INIT != AppUsartCblk.state)
      && (UART_ERROR != AppUsartCblk.state))
    {
        // If a message is waiting, process it
        if ( UART_REQUEST_WAITING == AppUsartCblk.requestState 
          || UART_TRANSMITING == AppUsartCblk.requestState )
        {
            // Process Message
            status = uart_state_machine(navigation_cblk);
        }
        else if ( UART_NO_REQUEST == AppUsartCblk.requestState )
        {
            // Enter the Reception state
            status = uart_receive();
        }
    }
    // If any failures, signal via LED
    if ( STATUS_FAILURE == status )
    {
        BSP_LED_On(BSP_UART_ERROR_LED);
    }
    return status;
}

/* -------------------------- End of File ------------------------------------*/
