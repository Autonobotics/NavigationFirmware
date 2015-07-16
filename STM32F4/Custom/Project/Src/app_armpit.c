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
#include "app_armpit.h"
#include "app_uart_generic.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define INPUT_ARMPIT_BUFFER_SIZE (sizeof(AppArmpitCblk.inputBuffer))
#define OUTPUT_ARMPIT_BUFFER_SIZE (sizeof(AppArmpitCblk.outputBuffer))
#define ARMPIT_TRANSACTION_RETRY_LIMIT 5

/* Private macro -------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
UART_HandleTypeDef ArmpitHandle;
TIM_HandleTypeDef htim11;

/* Private variables ---------------------------------------------------------*/
static sAPP_ARMPIT_CBLK AppArmpitCblk = {
    NULL,
    NULL,
    ARMPIT_INIT,
    ARMPIT_INIT,
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
     0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
     0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    UART_INITIAL
};


/* Private function prototypes -----------------------------------------------*/
static eAPP_STATUS armpit_receive(void);
static eAPP_STATUS armpit_transmit(void);
static void armpit_set_navigation_data(sAPP_NAVIGATION_CBLK* navigation_cblk,
                                       uint16_t x_distance,
                                       uint16_t y_distance,
                                       uint16_t z_distance,
                                       int16_t rotation);
static eAPP_STATUS armpit_send_response(sAPP_NAVIGATION_CBLK* navigation_cblk);
static eAPP_STATUS armpit_start_handshake(void);
static eAPP_STATUS armpit_finish_handshake(void);
static eAPP_STATUS armpit_handle_data_receive(sAPP_NAVIGATION_CBLK* navigation_cblk,
                                              uAPP_ARMPIT_MESSAGES message);
static eAPP_STATUS armpit_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk);


/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*                 Internal Support Functions                                 */
/******************************************************************************/
static eAPP_STATUS armpit_transmit(void)
{
    HAL_StatusTypeDef status;
    uint32_t retryCount = 0;
    
    /* Start the UART peripheral transmission process */
    AppArmpitCblk.requestState = UART_REQUEST_PROCESSING;
    while ( HAL_OK != (status = HAL_UART_Transmit_IT(AppArmpitCblk.handle,
                                                     AppArmpitCblk.outputBuffer.buffer, 
                                                     OUTPUT_ARMPIT_BUFFER_SIZE)))
    {
        retryCount++;
        if ( retryCount > ARMPIT_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("ARMPIT: Error during UART transmission. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}

static eAPP_STATUS armpit_receive(void)
{
    HAL_StatusTypeDef status;
    uint32_t retryCount = 0;
    
    /* Put UART peripheral in reception mode */ 
    Flush_Buffer(AppArmpitCblk.inputBuffer.buffer, INPUT_ARMPIT_BUFFER_SIZE);
    Flush_Buffer(AppArmpitCblk.outputBuffer.buffer, OUTPUT_ARMPIT_BUFFER_SIZE);
    AppArmpitCblk.requestState = UART_WAITING;
    while( HAL_OK != (status = HAL_UART_Receive_IT(AppArmpitCblk.handle, 
                                                   AppArmpitCblk.inputBuffer.buffer, 
                                                   INPUT_ARMPIT_BUFFER_SIZE)))
    {
        retryCount++;
        if ( retryCount > ARMPIT_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("ARMPIT: Error during UART reception. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}


static void armpit_set_navigation_data(sAPP_NAVIGATION_CBLK* navigation_cblk,
                                       uint16_t x_distance,
                                       uint16_t y_distance,
                                       uint16_t z_distance,
                                       int16_t rotation)
{
    // Set the modified Flag
    navigation_cblk->image_board_data.modified = TRUE;
    navigation_cblk->image_board_data.x_distance = x_distance;
    navigation_cblk->image_board_data.y_distance = y_distance;
    navigation_cblk->image_board_data.z_distance = z_distance;
    navigation_cblk->image_board_data.rotation = rotation;
}


/******************************************************************************/
/*                 State Machine Management Functions                         */
/******************************************************************************/
static eAPP_STATUS armpit_send_response(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status;
    
    // Format the RACK Packet by default
    AppArmpitCblk.outputBuffer.rack.cmd = ARMPIT_CMD_RACK;
    AppArmpitCblk.outputBuffer.rack.flag = ARMPIT_FLAG_END;
    
    // If we have something to tell the Image Board, send RACK, else just ACK
    if ( ROTATION_COMPLETE ==  navigation_cblk->navigation_flags.rotation_status )
    {
        AppArmpitCblk.outputBuffer.rack.sub_cmd = ARMPIT_SUBCMD_ROTATION_COMPLETE;
        // Reset the Rotation Flag
        navigation_cblk->navigation_flags.rotation_status = ROTATION_INCOMPLETE;
        
        APP_Log("ARMPIT: Replying with RACK. Subcmd: ROTATION_COMPLETE"ENDLINE);
    }
    else if ( FRONTAL_AVOIDANCE_MODE_ON == navigation_cblk->navigation_flags.trigger_edge_on_image )
    {
        // Read distance on frontal Ultrasonic
        AppArmpitCblk.outputBuffer.rack.sub_cmd = ARMPIT_SUBCMD_COLLISION_DETECTED;
        AppArmpitCblk.outputBuffer.rack.axis = AXIS_FRONT; // No other possible value currently
        AppArmpitCblk.outputBuffer.rack.payload_a = navigation_cblk->proximity_data.distance[AXIS_FRONT];
        
        APP_Log("ARMPIT: Replying with RACK. Subcmd: COLLISION_DETECTED"ENDLINE);
    }
    else
    {
        // Format the ACK Packet
        AppArmpitCblk.outputBuffer.ack.cmd = ARMPIT_CMD_ACK;
        AppArmpitCblk.outputBuffer.ack.flag = ARMPIT_FLAG_END;
        
        APP_Log("ARMPIT: Replying with ACK."ENDLINE);
    }
    
    // Transmit and setup for receive
    status = armpit_transmit();
    return status;
}

static eAPP_STATUS armpit_start_handshake(void)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    HAL_StatusTypeDef halStatus;
    
    // Format the Handshake SYNC Packet
    AppArmpitCblk.outputBuffer.sync.cmd = ARMPIT_CMD_SYNC;
    AppArmpitCblk.outputBuffer.sync.flag = ARMPIT_FLAG_END;
    
    if ( HAL_OK == (halStatus = HAL_UART_Transmit(AppArmpitCblk.handle, 
                                                  AppArmpitCblk.outputBuffer.buffer, 
                                                  OUTPUT_ARMPIT_BUFFER_SIZE, 
                                                  ARMPIT_POLL_TIMEOUT)))
    {
        Flush_Buffer(AppArmpitCblk.outputBuffer.buffer, OUTPUT_ARMPIT_BUFFER_SIZE);
        
        halStatus = HAL_UART_Receive(AppArmpitCblk.handle, 
                                     AppArmpitCblk.inputBuffer.buffer, 
                                     INPUT_ARMPIT_BUFFER_SIZE, 
                                     ARMPIT_POLL_TIMEOUT);
        if ( HAL_OK == halStatus)
        {
            AppArmpitCblk.state = ARMPIT_HANDSHAKE_FINISH;
            AppArmpitCblk.requestState = UART_REQUEST_WAITING;
        }
        else
        {
            APP_Log("ARMPIT: Unable to receive expected SYNC. HAL_STATUS: %s"ENDLINE,
                    Translate_HAL_Status(halStatus));
        }
    }
    
    // Ensure that there was no major error with the UART on our end
    if ( HAL_OK != halStatus
      && HAL_TIMEOUT != halStatus )
    {
        APP_Log("ARMPIT: An error occured with the UART on handshake. Entering Error State.\r\n");
        status = STATUS_FAILURE;
    }
    return status;
    
}

static eAPP_STATUS armpit_finish_handshake(void)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch (AppArmpitCblk.inputBuffer.common.cmd)
    {
        case ARMPIT_CMD_RSYNC:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != AppArmpitCblk.inputBuffer.sync.flag )
            {
                // Log Failure and return
                APP_Log("ARMPIT: FLAG Bits wrong on SYNC Command.\r\n");
                AppArmpitCblk.state = ARMPIT_HANDSHAKE_START;
                AppArmpitCblk.requestState = UART_TRANSMITING;
                return STATUS_SUCCESS;
            }
        
            // Format the Handshake ACK Packet
            AppArmpitCblk.outputBuffer.ack.cmd = ARMPIT_CMD_ACK;
            AppArmpitCblk.outputBuffer.ack.flag = ARMPIT_FLAG_END;
        
            // Transmit and setup for receive
            status = armpit_transmit();
            if ( STATUS_SUCCESS == status)
            {
                // Change states
                APP_Log("Finished ARMPIT Handshake, starting Interrupt Process.\r\n");
                AppArmpitCblk.state = ARMPIT_DATA_RECEIVE;
            }
            else
            {
                APP_Log("ARMPIT: Unable to transmit ACK."ENDLINE);
            }
            return status;
        
        case ARMPIT_CMD_SYNC:
        case ARMPIT_CMD_DYNC:
        case ARMPIT_CMD_ACK:
        default:
            APP_Log("ARMPIT: Received unknown command %u on Handshake response"ENDLINE,
                    AppArmpitCblk.inputBuffer.common.cmd);
            AppArmpitCblk.state = ARMPIT_HANDSHAKE_START;
            AppArmpitCblk.requestState = UART_TRANSMITING;
            return STATUS_SUCCESS;
        
    }
}

static eAPP_STATUS armpit_handle_data_receive(sAPP_NAVIGATION_CBLK* navigation_cblk, 
                                              uAPP_ARMPIT_MESSAGES message)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch ( message.common.cmd )
    {
        case ARMPIT_CMD_NO_BEACON:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.no_beacon.flag )
            {
                // Log Failure and return
                APP_Log("ARMPIT: FLAG Bits wrong on NO_BEACON Command.\r\n");
                return STATUS_FAILURE;
            }
            APP_Log("ARMPIT: Received CMD_NO_BEACON.\r\n");
            
            // Set Data Variables here
            armpit_set_navigation_data(navigation_cblk,
                                       DISTANCE_UNKNOWN,
                                       DISTANCE_UNKNOWN,
                                       DISTANCE_UNKNOWN,
                                       ROTATION_UNKNOWN);

            // Transmit Response
            status = armpit_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_BEACON_DETECTED:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.beacon_detected.flag )
            {
                // Log Failure and return
                APP_Log("ARMPIT: FLAG Bits wrong on BEACON_DETECTED Command.\r\n");
                return STATUS_FAILURE;
            }
            APP_Log("ARMPIT: Received CMD_BEACON_DETECTED."ENDLINE);
            
            // Set Data Variables here
            armpit_set_navigation_data(navigation_cblk,
                                       message.beacon_detected.x_distance,
                                       message.beacon_detected.y_distance,
                                       message.beacon_detected.z_distance,
                                       ROTATION_UNKNOWN);
            
            APP_Log("ARMPIT: x_distance: %d y_distance: %d z_distance: %d."ENDLINE, 
                    message.beacon_detected.x_distance,
                    message.beacon_detected.y_distance,
                    message.beacon_detected.z_distance);

            // Transmit Response
            status = armpit_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_EDGE_DETECTED:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.edge_detected.flag )
            {
                // Log Failure and return
                APP_Log("ARMPIT: FLAG Bits wrong on EDGE_DETECTED Command.\r\n");
                return STATUS_FAILURE;
            }
            APP_Log("ARMPIT: Received CMD_EDGE_DETECTED."ENDLINE);
            
            // Set Data Variables here
            armpit_set_navigation_data(navigation_cblk,
                                       message.edge_detected.x_distance,
                                       DISTANCE_UNKNOWN,
                                       DISTANCE_UNKNOWN,
                                       ROTATION_UNKNOWN);
            
            // Transmit Response
            status = armpit_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_BEACON_ROTATION:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.beacon_rotation.flag )
            {
                // Log Failure and return
                APP_Log("ARMPIT: FLAG Bits wrong on BEACON_ROTATION Command.\r\n");
                return STATUS_FAILURE;
            }
            APP_Log("ARMPIT: Received CMD_BEACON_ROTATION."ENDLINE);
            
            // Set Data Variables here
            armpit_set_navigation_data(navigation_cblk,
                                       DISTANCE_UNKNOWN,
                                       DISTANCE_UNKNOWN,
                                       DISTANCE_UNKNOWN,
                                       message.beacon_rotation.x_rotation);
            
            // Transmit Response
            status = armpit_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_QUERY_ROTATION:
            // Validate the Flag as end
            if ( ARMPIT_FLAG_END != message.query_rotation.flag )
            {
                // Log Failure and return
                APP_Log("ARMPIT: FLAG Bits wrong on QUERY_ROTATION Command.\r\n");
                return STATUS_FAILURE;
            }
            APP_Log("ARMPIT: Received CMD_QUERY_ROTATION."ENDLINE);
            
            // NOTE: No data set in a Query Rotation as we want our old
            //       rotation value to continue to be transmitted.
            
            // Transmit Response
            status = armpit_send_response(navigation_cblk);
            return status;
        
        case ARMPIT_CMD_SYNC:
        case ARMPIT_CMD_INVD:
        case ARMPIT_CMD_DYNC:
        case ARMPIT_CMD_ACK:
        case ARMPIT_CMD_RACK:
        default:
            APP_Log("ARMPIT: Driver state received bad command. Command: %x\r\n", message.common.cmd);
            AppArmpitCblk.state = ARMPIT_ERROR;
            return STATUS_FAILURE;
    }
}

static eAPP_STATUS armpit_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch( AppArmpitCblk.state )
    {
        case ARMPIT_HANDSHAKE_START:
            // If the ARMPIT Enable Pin is set
            if ( GPIO_PIN_SET == HAL_GPIO_ReadPin(ARMPIT_ENABLE_PORT, ARMPIT_ENABLE_PIN) )
            {
                status = armpit_start_handshake();
                return status;
            }
            else
            {
                return STATUS_SUCCESS;
            }
            break;
        
        case ARMPIT_HANDSHAKE_FINISH:
            status = armpit_finish_handshake();
            return status;
        
        case ARMPIT_DATA_RECEIVE:
            status = armpit_handle_data_receive(navigation_cblk, AppArmpitCblk.inputBuffer);
            return status;
        
        case ARMPIT_TERMINATE:
        case ARMPIT_INIT:
        case ARMPIT_ERROR:
        default:
            APP_Log("ARMPIT: Driver in Bad State.\r\n");
            AppArmpitCblk.state = ARMPIT_ERROR;
            return status;
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
void ARMPIT_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // Transmission was successful, so ready to listen for a new request
    AppArmpitCblk.requestState = UART_NO_REQUEST;
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void ARMPIT_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // Reception was successful, so ready for processing
    AppArmpitCblk.requestState = UART_REQUEST_WAITING;
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void ARMPIT_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    /* Turn BSP_UART_ERROR_LED */
    BSP_LED_On(BSP_ARMPIT_ERROR_LED);
    AppArmpitCblk.state = ARMPIT_TRANSITION_TO_ERROR;
}


/**
  * @brief  Performs initialization of the Application-specific UART.
  * @param  None
  * @retval None
  */
void APP_ARMPIT_Init(void)
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
    ArmpitHandle.Instance          = ARMPIT_USART;
    ArmpitHandle.Init.BaudRate     = 115200;
    ArmpitHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    ArmpitHandle.Init.StopBits     = UART_STOPBITS_1;
    ArmpitHandle.Init.Parity       = UART_PARITY_NONE;
    ArmpitHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    ArmpitHandle.Init.Mode         = UART_MODE_TX_RX;
    ArmpitHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(& ArmpitHandle) != HAL_OK)
    {
        Error_Handler();
    }
    
    AppArmpitCblk.handle = & ArmpitHandle;
    AppArmpitCblk.state = ARMPIT_HANDSHAKE_START;
    AppArmpitCblk.requestState = UART_TRANSMITING;
}

eAPP_STATUS APP_ARMPIT_Initiate(void)
{
    // Validate we are in correct state
    if ( ARMPIT_HANDSHAKE_START != AppArmpitCblk.state )
    {
        APP_Log("ARMPIT in incorrect state for Handshake Procedure.\r\n");
        return STATUS_FAILURE;
    }
    
    // Start the Handshake Procedure (Asychronous Process)
    APP_Log("Starting ARMPIT Handshake.\r\n");
    return STATUS_SUCCESS;
    
}

eAPP_STATUS APP_ARMPIT_Process_Message(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    // Determine the state we are in and process accordingly
    switch ( AppArmpitCblk.state )
    {
        case ARMPIT_HANDSHAKE_START:
        case ARMPIT_HANDSHAKE_FINISH:
        case ARMPIT_DATA_RECEIVE:
        case ARMPIT_TERMINATE:
            // If a message is waiting, process it
            if ( UART_REQUEST_WAITING == AppArmpitCblk.requestState 
              || UART_TRANSMITING == AppArmpitCblk.requestState )
            {
                // Process Message
                status = armpit_state_machine(navigation_cblk);
            }
            else if ( UART_NO_REQUEST == AppArmpitCblk.requestState )
            {
                // Enter the Reception state
                status = armpit_receive();
            }
            break;
            
        case ARMPIT_TRANSITION_TO_ERROR:
            // Log Error and Return Failure
            APP_Log("ARMPIT: UART Error Occured: %s. Transitioning to Error Recovering.\r\n",
            APP_UART_Generic_Translate_Error(AppArmpitCblk.handle->ErrorCode));
            AppArmpitCblk.state = ARMPIT_ERROR;
            break;
        
        case ARMPIT_INIT:
            // Do nothing
            break;
        
        case ARMPIT_ERROR:
        default:
            status = STATUS_FAILURE;
            break;
    }
    
    // If any failures, signal via LED
    if ( STATUS_FAILURE == status )
    {
        BSP_LED_On(BSP_ARMPIT_ERROR_LED);
        AppArmpitCblk.state = ARMPIT_ERROR;
    }
    return status;
}

/* -------------------------- End of File ------------------------------------*/
