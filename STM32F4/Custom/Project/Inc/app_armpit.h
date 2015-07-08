/**
  ******************************************************************************
  * @file    Inc/app_usart.h
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Header for app_usart.c module
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_USART_H
#define __APP_USART_H
 
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "app_navigation.h"

/* Exported constants --------------------------------------------------------*/

/* Definition for ARMPIT clock resources */
#define ARMPIT_USART                            USART2
#define ARMPIT_USART_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE();
#define ARMPIT_USART_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define ARMPIT_USART_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE() 

#define ARMPIT_USART_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define ARMPIT_USART_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* Definition for ARMPIT Pins */
#define ARMPIT_USART_TX_PIN                    GPIO_PIN_2
#define ARMPIT_USART_TX_GPIO_PORT              GPIOA  
#define ARMPIT_USART_TX_AF                     GPIO_AF7_USART2
#define ARMPIT_USART_RX_PIN                    GPIO_PIN_3
#define ARMPIT_USART_RX_GPIO_PORT              GPIOA 
#define ARMPIT_USART_RX_AF                     GPIO_AF7_USART2

/* Definition for ARMPIT's NVIC */
#define ARMPIT_USART_IRQn                      USART2_IRQn
#define ARMPIT_USART_IRQHandler                USART2_IRQHandler

/* Define Connection Timeout and Attempts */
#define ARMPIT_POLL_TIMEOUT 1  // Ie. Blocking Poll (In Milliseconds) Note: Would like Non-blocking Poll

/* ARMPIT Definitions */
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

#define ARMPIT_SUBCMD_COLLISION_DETECTED 0x01
#define ARMPIT_SUBCMD_ROTATION_COMPLETE 0x02

#define ARMPIT_FLAG_END 0xFF


/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_ARMPIT_STATE
{
    ARMPIT_INIT = 0,
    ARMPIT_HANDSHAKE,
    ARMPIT_DATA_RECEIVE,
    ARMPIT_TERMINATE,
    
    ARMPIT_ERROR
    
} eAPP_ARMPIT_STATE;

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
    
} sAPP_ARMPIT_NO_BEACON;

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
    
    sAPP_ARMPIT_NO_BEACON no_beacon;
    sAPP_ARMPIT_BEACON_DETECTED beacon_detected;
    sAPP_ARMPIT_BEACON_ROTATION beacon_rotation;
    sAPP_ARMPIT_QUERY_ROTATION query_rotation;
    sAPP_ARMPIT_EDGE_DETECTED edge_detected;
    
    sAPP_ARMPIT_COMMON common;
    uint8_t buffer[16];
    
} uAPP_ARMPIT_MESSAGES;

typedef struct _sAPP_USART_CBLK
{
    UART_HandleTypeDef *handle;
    eAPP_ARMPIT_STATE state;
    uAPP_ARMPIT_MESSAGES inputBuffer;
    uAPP_ARMPIT_MESSAGES outputBuffer;
    
    volatile eAPP_UART_REQUEST_STATE requestState;
    
} sAPP_ARMPIT_CBLK;

/* Callback functions ------------------------------------------------------- */
void ARMPIT_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void ARMPIT_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void ARMPIT_UART_ErrorCallback(UART_HandleTypeDef *huart);

/* Exported functions ------------------------------------------------------- */
void APP_ARMPIT_Init(void);
eAPP_STATUS APP_ARMPIT_Initiate(void);
eAPP_STATUS APP_ARMPIT_Process_Message(sAPP_NAVIGATION_CBLK* navigation_cblk);


#endif /* #ifndef __APP_USART_H */
