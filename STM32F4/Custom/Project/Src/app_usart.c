/**
  ******************************************************************************
  * @file    Src/app_usart.c 
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    13-March-2015
  * @brief   Application USART Implementation
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
#include "app_usart.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define INPUT_UART_BUFFER_SIZE (sizeof(AppUsartCblk.inputBuffer))
#define OUTPUT_UART_BUFFER_SIZE (sizeof(AppUsartCblk.outputBuffer))
#define UART_TRANSACTION_RETRY_LIMIT 5

/* Private macro -------------------------------------------------------------*/
/* Pulbic variables ----------------------------------------------------------*/
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
static eAPP_STATUS uart_handle_handshake(uAPP_USART_MESSAGES message);
static eAPP_STATUS uart_handle_data_receive(uAPP_USART_MESSAGES message);
static eAPP_STATUS uart_state_machine(void);


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

static eAPP_STATUS uart_handle_data_receive(uAPP_USART_MESSAGES message)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch ( message.common.cmd )
    {
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
            return status;
        
        case ARMPIT_CMD_INVD:
        case ARMPIT_CMD_DYNC:
        case ARMPIT_CMD_ACK:
        default:
            APP_Log("UART APP Driver state received bad command. Command: %x\r\n", message.common.cmd);
            AppUsartCblk.state = UART_ERROR;
            return STATUS_FAILURE;
    }
}

static eAPP_STATUS uart_state_machine()
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch( AppUsartCblk.state )
    {
        case UART_HANDSHAKE:
            status = uart_handle_handshake(AppUsartCblk.inputBuffer);
            return status;
        
        case UART_DATA_RECEIVE:
            status = uart_handle_data_receive(AppUsartCblk.inputBuffer);
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

eAPP_STATUS APP_UART_Process_Message(void)
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
            status = uart_state_machine();
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
