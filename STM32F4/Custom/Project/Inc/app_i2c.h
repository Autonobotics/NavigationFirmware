/**
  ******************************************************************************
  * @file    Inc/app_i2c.h
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Header for app_i2c.c module
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_I2C_H
#define __APP_I2C_H


/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "app_navigation.h"

/* Exported constants --------------------------------------------------------*/

/* Definition for I2C commands and Address */
#define I2C_ADDRESS        0x30

/* Definition for I2Cx clock resources */
#define I2Cx                             I2C1
#define I2Cx_CLK_ENABLE()                __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()               __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()             __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SCL_AF                     GPIO_AF4_I2C1
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_AF                     GPIO_AF4_I2C1

/* Definition for I2Cx's NVIC */
#define I2Cx_EV_IRQn                    I2C1_EV_IRQn
#define I2Cx_EV_IRQHandler              I2C1_EV_IRQHandler
#define I2Cx_ER_IRQn                    I2C1_ER_IRQn
#define I2Cx_ER_IRQHandler              I2C1_ER_IRQHandler

/* Define Connection Timeout and Attempts */
#define I2C_POLL_TIMEOUT 1000  // In Milliseconds
#define I2C_CONNECTION_ATTEMPTS 60


/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_I2C_STATE
{
    I2C_INIT = 0,
    I2C_HANDSHAKE,
    I2C_PROCESS,
    I2C_SHUTDOWN,
    
    I2C_ERROR
    
} eAPP_I2C_STATE;

typedef enum _eAPP_I2C_REQUEST_STATE
{
    I2C_INITIAL = 0,
    I2C_NO_REQUEST,
    I2C_REQUEST_WAITING,
    I2C_REQUEST_PROCESSING,
    I2C_WAITING
    
} eAPP_I2C_REQUEST_STATE;

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
    uint8_t flag;
    
    uint8_t padding[6];
    
} sAPP_PIXARM_ACK;

typedef struct _sAPP_PIXARM_READ_REQ
{
    uint8_t cmd;
    uint8_t padding_a;
    
    uint16_t rotation_absolute;
    uint8_t flag;
    
    uint8_t padding[3];
    
} sAPP_PIXARM_READ_REQ;

typedef struct _sAPP_PIXARM_READ_DATA
{
    uint8_t cmd;
    
    uint8_t x_intensity;
    uint8_t y_intensity;
    uint8_t z_intensity;
    int16_t rotation_absolute;

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

typedef struct
{
    I2C_HandleTypeDef *handle;
    eAPP_I2C_STATE state;
    uAPP_PIXARM_MESSAGES inputBuffer;
    uAPP_PIXARM_MESSAGES outputBuffer;
    
    volatile eAPP_I2C_REQUEST_STATE requestState;
    
} sAPP_I2C_CBLK;


/* Exported functions ------------------------------------------------------- */
void APP_I2C_Init(void);
eAPP_STATUS APP_I2C_Initiate(void);
eAPP_STATUS APP_I2C_Process_Message(sAPP_NAVIGATION_CBLK* navigation_cblk);


#endif /* #ifndef __APP_I2C_H */
