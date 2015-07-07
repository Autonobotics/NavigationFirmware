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

/* Define Connection Timeout and Attempts */
#define UART_POLL_TIMEOUT 1  // Ie. Non-blocking Poll (In Milliseconds)


/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_USART_STATE
{
    UART_INIT = 0,
    UART_HANDSHAKE,
    UART_DATA_RECEIVE,
    UART_TERMINATE,
    
    UART_ERROR
    
} eAPP_USART_STATE;

typedef enum _eAPP_UART_REQUEST_STATE
{
    UART_INITIAL = 0,
    UART_NO_REQUEST,
    UART_TRANSMITING,
    UART_REQUEST_WAITING,
    UART_REQUEST_PROCESSING,
    UART_WAITING
    
} eAPP_UART_REQUEST_STATE;

#define ARMPIT_CMD_INVD 0x00
#define ARMPIT_CMD_SYNC 0x01
#define ARMPIT_CMD_DYNC 0xFE
#define ARMPIT_CMD_ACK  0x02
#define ARMPIT_CMD_RACK 0x03
#define ARMPIT_CMD_NO_BEACON 0x30
#define ARMPIT_CMD_BEACON_DETECTED 0x31
#define ARMPIT_CMD_EDGE_DETECTED 0x32
#define ARMPIT_CMD_BEACON_ROTATION 0x33
#define ARMPIT_CMD_QUERY_ROTATION 0x34

#define ARMPIT_FLAG_END 0xFF

typedef struct _sAPP_ARMPIT_COMMON
{
    uint8_t cmd;
    
    uint8_t padding[15];
    
} sAPP_ARMPIT_COMMON;

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

typedef struct _sAPP_ARMPIT_RACK
{
    uint8_t cmd;
    uint8_t sub_cmd;
    uint8_t axis;
    uint8_t padding_a;
    
    uint16_t payload_a;
    uint16_t payload_b;
    uint8_t flag;
    
    uint8_t padding[7];
    
} sAPP_ARMPIT_RACK;

typedef struct _sAPP_ARMPIT_NO_BEACON
{
    uint8_t cmd;
    uint8_t flag;
    
    uint8_t padding[14];
    
} sAPP_ARMPIT_BEACON;

typedef struct _sAPP_ARMPIT_BEACON_DETECTED
{
    uint8_t cmd;
    uint8_t padding_a;
    
    int16_t x_distance;
    int16_t y_distance;
    int16_t z_distance;
    
    uint8_t flag;
    
    uint8_t padding[7];
    
} sAPP_ARMPIT_BEACON_DETECTED;

typedef struct _sAPP_ARMPIT_EDGE_DETECTED
{
    uint8_t cmd;
    uint8_t padding_a;
    
    int16_t x_distance;
    
    uint8_t flag;
    
    uint8_t padding[11];
    
} sAPP_ARMPIT_EDGE_DETECTED;

typedef struct _sAPP_ARMPIT_BEACON_ROTATION
{
    uint8_t cmd;
    uint8_t padding_a;
    
    int16_t x_rotation;
    
    uint8_t flag;
    
    uint8_t padding[11];
    
} sAPP_ARMPIT_BEACON_ROTATION;

typedef struct _sAPP_ARMPIT_QUERY_ROTATION
{
    uint8_t cmd;
    uint8_t flag;
    
    uint8_t padding[14];
    
} sAPP_ARMPIT_QUERY_ROTATION;

typedef union _uAPP_USART_MESSAGES
{
    sAPP_ARMPIT_SYNC sync;
    sAPP_ARMPIT_ACK ack;
    sAPP_ARMPIT_RACK rack;
    
    sAPP_ARMPIT_BEACON beacon;
    sAPP_ARMPIT_BEACON_DETECTED beacon_detected;
    sAPP_ARMPIT_BEACON_ROTATION beacon_rotation;
    sAPP_ARMPIT_QUERY_ROTATION query_rotation;
    sAPP_ARMPIT_EDGE_DETECTED edge_detected;
    
    sAPP_ARMPIT_COMMON common;
    uint8_t buffer[16];
    
} uAPP_USART_MESSAGES;

typedef struct _sAPP_USART_CBLK
{
    UART_HandleTypeDef *handle;
    eAPP_USART_STATE state;
    uAPP_USART_MESSAGES inputBuffer;
    uAPP_USART_MESSAGES outputBuffer;
    
    volatile eAPP_UART_REQUEST_STATE requestState;
    
} sAPP_USART_CBLK;


/* Exported functions ------------------------------------------------------- */
void APP_USART_Init(void);
eAPP_STATUS APP_UART_Initiate(void);
eAPP_STATUS APP_UART_Process_Message(void);


#endif /* #ifndef __APP_USART_H */
