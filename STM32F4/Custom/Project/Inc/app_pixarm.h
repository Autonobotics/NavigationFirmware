/**
  ******************************************************************************
  * @file    Inc/app_pixarm.h
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Header for app_pixarm.c module
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_PIXARM_H
#define __APP_PIXARM_H


/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "app_uart_generic.h"
#include "app_navigation.h"

/* Exported constants --------------------------------------------------------*/

/* Definition for PIXARM clock resources */
#define PIXARM_USART                            UART4
#define PIXARM_USART_CLK_ENABLE()              __HAL_RCC_UART4_CLK_ENABLE();
#define PIXARM_USART_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define PIXARM_USART_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE() 
                                               
#define PIXARM_USART_FORCE_RESET()             __HAL_RCC_UART4_FORCE_RESET()
#define PIXARM_USART_RELEASE_RESET()           __HAL_RCC_UART4_RELEASE_RESET()
                                               
/* Definition for PIXARM Pins */               
#define PIXARM_USART_TX_PIN                    GPIO_PIN_10
#define PIXARM_USART_TX_GPIO_PORT              GPIOC  
#define PIXARM_USART_TX_AF                     GPIO_AF8_UART4
#define PIXARM_USART_RX_PIN                    GPIO_PIN_1
#define PIXARM_USART_RX_GPIO_PORT              GPIOA 
#define PIXARM_USART_RX_AF                     GPIO_AF8_UART4
                                               
/* Definition for PIXARM's NVIC */             
#define PIXARM_USART_IRQn                      UART4_IRQn
#define PIXARM_USART_IRQHandler                UART4_IRQHandler

/* Define Connection Timeout and Attempts */
#define PIXARM_POLL_TIMEOUT 1000  // Ie. Blocking Poll (In Milliseconds)
#define PIXARM_CONNECTION_ATTEMPTS 60000

/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_PIXARM_STATE
{
    PIXARM_INIT = 0,
    PIXARM_HANDSHAKE,
    PIXARM_PROCESS,
    PIXARM_SHUTDOWN,
    
    PIXARM_TRANSITION_TO_ERROR,
    PIXARM_ERROR
    
} eAPP_PIXARM_STATE;

#define PIXARM_CMD_SYNC 0x01
#define PIXARM_CMD_DYNC 0xFE
#define PIXARM_CMD_ACK  0x02

#define PIXARM_CMD_READ_REQ 0x03
#define PIXARM_CMD_READ_DATA 0x04

#define PIXARM_FLAG_END 0xFF

typedef struct _sAPP_PIXARM_COMMON
{
    uint8_t cmd;
    
    uint8_t padding[7];
    
} sAPP_PIXARM_COMMON;

typedef struct _sAPP_PIXARM_SYNC
{
    uint8_t cmd;
    uint8_t payload[6];
    uint8_t flag;
    
} sAPP_PIXARM_SYNC;

typedef struct _sAPP_PIXARM_ACK
{
    uint8_t cmd;
    
    uint8_t padding[6];
    
    uint8_t flag;
    
} sAPP_PIXARM_ACK;

typedef struct _sAPP_PIXARM_READ_REQ
{
    uint8_t cmd;
    uint8_t padding_a;
    
    uint16_t rotation_absolute;
    int16_t z_velocity;
    
    uint8_t padding;
    
    uint8_t flag;
    
} sAPP_PIXARM_READ_REQ;

typedef struct _sAPP_PIXARM_READ_DATA
{
    uint8_t cmd;
    
    uint8_t x_intensity;
    uint8_t y_intensity;
    uint8_t z_distance;
    int16_t rotation_speed;

    uint8_t padding_b;
    uint8_t flag;
    
} sAPP_PIXARM_READ_DATA;

typedef union _uAPP_PIXARM_MESSAGES
{
    sAPP_PIXARM_SYNC sync;
    sAPP_PIXARM_ACK ack;
    sAPP_PIXARM_READ_REQ readReq;
    sAPP_PIXARM_READ_DATA readData;

    sAPP_PIXARM_COMMON common;
    uint8_t buffer[8];
    
} uAPP_PIXARM_MESSAGES;

typedef struct _sAPP_PIXARM_CBLK
{
    UART_HandleTypeDef *handle;
    eAPP_PIXARM_STATE state;
    eAPP_PIXARM_STATE prev_state;
    uAPP_PIXARM_MESSAGES inputBuffer;
    uAPP_PIXARM_MESSAGES outputBuffer;
    
    volatile eAPP_UART_REQUEST_STATE requestState;
    
} sAPP_PIXARM_CBLK;


/* Callback functions ------------------------------------------------------- */
void PIXARM_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void PIXARM_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void PIXARM_UART_ErrorCallback(UART_HandleTypeDef *huart);

/* Exported functions ------------------------------------------------------- */
void APP_PIXARM_Init(void);
eAPP_STATUS APP_PIXARM_Initiate(void);
eAPP_STATUS APP_PIXARM_Process_Message(sAPP_NAVIGATION_CBLK* navigation_cblk);


#endif /* #ifndef __APP_I2C_H */
