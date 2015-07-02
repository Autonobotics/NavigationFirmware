/**
  ******************************************************************************
  * @file    Inc/app_usart.h
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    13-March-2015
  * @brief   Header for app_usart.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_USART_H
#define __APP_USART_H
 
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

/* Exported constants --------------------------------------------------------*/

/* Definition for USARTx clock resources */
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE() 

#define USARTx_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA  
#define USARTx_TX_AF                     GPIO_AF7_USART2
#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA 
#define USARTx_RX_AF                     GPIO_AF7_USART2

/* Definition for USARTx's NVIC */
#define USARTx_IRQn                      USART2_IRQn
#define USARTx_IRQHandler                USART2_IRQHandler

/* Size of Transmission buffer */
#define USART2_TXBUFFERSIZE              (COUNTOF(usartTxBuffer) - 1)
/* Size of Reception buffer */
#define USART2_RXBUFFERSIZE              USART2_TXBUFFERSIZE


/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_USART_STATE
{
    UART_INIT = 0,
    UART_HANDSHAKE,
    UART_DATA_RECEIVE,
    UART_DATA_SEND,
    UART_TERMINATE,
    
    UART_ERROR
    
} eAPP_USART_STATE;

#define ARMPIT_CMD_SYNC 0x01
#define ARMPIT_CMD_DYNC 0xFE
#define ARMPIT_CMD_ACK  0x02

#define ARMPIT_FLAG_END 0xFF

typedef struct _sAPP_ARMPIT_SYNC
{
    uint8_t cmd;
    uint8_t payload[6];
    uint8_t flag;
    
    uint8_t padding[8];
    
} sAPP_ARMPIT_SYNC;

typedef struct _sAPP_ARMPIT_ACK
{
    uint8_t cmd;
    uint8_t flag;
    
    uint8_t padding[14];
    
} sAPP_ARMPIT_ACK;

typedef union _uAPP_USART_MESSAGES
{
    sAPP_ARMPIT_SYNC sync;
    sAPP_ARMPIT_ACK ack;
    uint8_t buffer[16];
    
} uAPP_USART_MESSAGES;

typedef struct _sAPP_USART_CBLK
{
    UART_HandleTypeDef *handle;
    eAPP_USART_STATE state;
    BOOL started;
    __IO ITStatus uartReady;
    uAPP_USART_MESSAGES inputBuffer;
    uAPP_USART_MESSAGES outputBuffer;
    
} sAPP_USART_CBLK;


/* Exported functions ------------------------------------------------------- */
void APP_USART_Init(void);
void APP_UART_SetStatus(__IO ITStatus newStatus);
ITStatus APP_UART_GetStatus(void);


#endif /* #ifndef __APP_USART_H */
