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
typedef enum _eAPP_UART_DIR
{
    UART_TRANSMIT = 0,
    UART_RECEIVE
    
} eAPP_UART_DIR;

/* Private define ------------------------------------------------------------*/
#define INPUT_UART_BUFFER_SIZE (sizeof(AppUsartCblk.inputBuffer))
#define OUTPUT_UART_BUFFER_SIZE (sizeof(AppUsartCblk.outputBuffer))

/* Private macro -------------------------------------------------------------*/
/* Pulbic variables ----------------------------------------------------------*/
UART_HandleTypeDef UartHandle;

/* Private variables ---------------------------------------------------------*/
static sAPP_USART_CBLK AppUsartCblk = {
    NULL,
    UART_INIT,
    FALSE,
    RESET,
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}
    };


/* Private function prototypes -----------------------------------------------*/
static eAPP_STATUS uart_handle_handshake(eAPP_UART_DIR direction);
static eAPP_STATUS uart_handle_data_receive(eAPP_UART_DIR direction);
static eAPP_STATUS uart_handle_data_send(eAPP_UART_DIR direction);
static eAPP_STATUS uart_handle_terminate(eAPP_UART_DIR direction);
static void uart_state_machine(eAPP_UART_DIR direction);
 

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
    /* Set transmission flag: transfer complete */
    APP_UART_SetStatus(SET);
    
    /* Turn LED6 on: Transfer in transmission process is correct */
    BSP_LED_On(LED6);
    
    // Enter the Receive state
    if ( HAL_OK != HAL_UART_Receive_IT(AppUsartCblk.handle, AppUsartCblk.inputBuffer.buffer, INPUT_UART_BUFFER_SIZE))
    {
        APP_Log("Error entering Receive state from Transmission Complete Callback.\r\n");
        Error_Handler();
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
    /* Set transmission flag: transfer complete */
    APP_UART_SetStatus(SET);
    
    /* Turn LED4 on: Transfer in reception process is correct */
    BSP_LED_On(LED4);
    
    // Determine what to transmit
    uart_state_machine(UART_RECEIVE);
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
    /* Turn LED3 on: Transfer error in reception/transmission process */
    BSP_LED_On(LED3); 
}


/******************************************************************************/
/*                 State Machine Management Functions                         */
/******************************************************************************/
static eAPP_STATUS uart_handle_handshake(eAPP_UART_DIR direction)
{
    uAPP_USART_MESSAGES message;
    
    if ( UART_TRANSMIT == direction )
    {
        // Format the Handshake SYNC Packet
        AppUsartCblk.outputBuffer.sync.cmd = ARMPIT_CMD_SYNC;
        AppUsartCblk.outputBuffer.sync.flag = ARMPIT_FLAG_END;
        
        // Send the Packet
        APP_UART_SetStatus(RESET);
        if ( HAL_OK != HAL_UART_Transmit_IT(AppUsartCblk.handle, AppUsartCblk.outputBuffer.buffer, OUTPUT_UART_BUFFER_SIZE))
        {
            // Log Error and return failed
            APP_Log("Error in transmission of SYNC.\r\n");
            return STATUS_FAILURE;
        }
        return STATUS_SUCCESS;
    }
    else
    {
        // Process the Handshake SYNC Packet
        message.sync = AppUsartCblk.inputBuffer.sync;
        Flush_Buffer(AppUsartCblk.inputBuffer.buffer, sizeof(AppUsartCblk.inputBuffer.buffer));
        
        // Validate the CMD
        if ( ARMPIT_CMD_SYNC != message.sync.cmd )
        {
            // Log Failure and return
            APP_Log("CMD Bits wrong on SYNC Command.\r\n");
            return STATUS_FAILURE;
        }
        
        // Validate the Flag as end
        if ( ARMPIT_FLAG_END != message.sync.flag )
        {
            // Log Failure and return
            APP_Log("FLAG Bits wrong on SYNC Command.\r\n");
            return STATUS_FAILURE;
        }
        
        // Format an ACK Packet for response
        Flush_Buffer(AppUsartCblk.outputBuffer.buffer, sizeof(AppUsartCblk.outputBuffer.buffer));
        AppUsartCblk.outputBuffer.ack.cmd = ARMPIT_CMD_ACK;
        AppUsartCblk.outputBuffer.ack.flag = ARMPIT_FLAG_END;
        
        // Send the ACK Packet
        APP_UART_SetStatus(RESET);
        if ( HAL_OK != HAL_UART_Transmit_IT(AppUsartCblk.handle, AppUsartCblk.outputBuffer.buffer, OUTPUT_UART_BUFFER_SIZE))
        {
            // Log Error and return failed
            return STATUS_FAILURE;
        }
        
        // Change states
        AppUsartCblk.state = UART_DATA_RECEIVE;
        return STATUS_SUCCESS;
    }
}

static eAPP_STATUS uart_handle_data_receive(eAPP_UART_DIR direction)
{
    if ( UART_TRANSMIT == direction )
    {
    }
    else
    {
    }
}

static eAPP_STATUS uart_handle_data_send(eAPP_UART_DIR direction)
{
    if ( UART_RECEIVE == direction )
    {
    }
    else
    {
    }
}

static eAPP_STATUS uart_handle_terminate(eAPP_UART_DIR direction)
{
    if ( UART_TRANSMIT == direction )
    {
    }
    else
    {
    }
}

static void uart_state_machine(eAPP_UART_DIR direction)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch( AppUsartCblk.state )
    {
        case UART_HANDSHAKE:
            status = uart_handle_handshake(direction);
            break;
        
        case UART_DATA_RECEIVE:
            status = uart_handle_data_receive(direction);
            break;
        
        case UART_DATA_SEND:
            status = uart_handle_data_send(direction);
            break;
        
        case UART_TERMINATE:
            status = uart_handle_terminate(direction);
            break;
        
        case UART_INIT:
        case UART_ERROR:
        default:
            APP_Log("UART APP Driver in ERROR State.\r\n");
            AppUsartCblk.state = UART_ERROR;
            break;
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
}

void APP_UART_Initiate(void)
{
    // Start the internal state machine
    APP_Log("Starting UART Handshake.\r\n");
    uart_state_machine(UART_TRANSMIT);
}

ITStatus APP_UART_GetStatus(void)
{
    return AppUsartCblk.uartReady;
}

void APP_UART_SetStatus(__IO ITStatus newStatus)
{
    AppUsartCblk.uartReady = newStatus;
}


/* -------------------------- End of File ------------------------------------*/
