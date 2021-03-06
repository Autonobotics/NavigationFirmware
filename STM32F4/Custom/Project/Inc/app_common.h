/**
  ******************************************************************************
  * @file    Inc/app_common.h
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Functions, Defines and Types common to all elements
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_COMMON_H
#define __APP_COMMON_H
 
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"
#include "app_config.h"
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_STATUS
{
    STATUS_SUCCESS = 0,
    
    STATUS_FAILURE = -500
    
} eAPP_STATUS;


/* Quadcopter Axis Definitions */
typedef enum _eAPP_AXIS
{
    AXIS_FRONT = 0,     // +z
    AXIS_REAR,      // -z
    AXIS_LEFT,      // -x
    AXIS_RIGHT,      // +x
    AXIS_TOP,       // +y
    AXIS_BOTTOM     // -y
    
} eAPP_AXIS;

typedef enum _eAPP_UART_REQUEST_STATE
{
    UART_INITIAL = 0,
    UART_NO_REQUEST,
    UART_TRANSMITING,
    UART_REQUEST_WAITING,
    UART_REQUEST_PROCESSING,
    UART_WAITING
    
} eAPP_UART_REQUEST_STATE;

/* Exported constants --------------------------------------------------------*/
#define TRUE    1
#define FALSE   0
typedef uint16_t BOOL;

// LED5 -> RED
#define BSP_HARD_ERROR_LED          LED5
// LED3 -> ORANGE
#define BSP_PIXARM_ERROR_LED        LED3
// LED4 -> GREEN
#define BSP_ARMPIT_ERROR_LED        LED4
// LED6 -> BLUE
#define BSP_ULTRASONIC_ERROR_LED    LED6


/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

// This could be fixed by making a proper logging function that
// uses vprintf with a given variadic and any extra data.
#ifdef DEBUG
    #define APP_Log printf
#else
    #define APP_Log(...)    
#endif // #ifdef DEBUG
#define ENDLINE "\r\n"


/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_NAVIGATION_AXIS_INTENSITY
{
    POSITIVE_FAST = 0,
    NEGATIVE_FAST,
    
    POSITIVE_SLOW,
    NEGATIVE_SLOW,
    
    IDLE_INTENSITY
    
} eAPP_NAVIGATION_AXIS_INTENSITY;

typedef struct _sAPP_NAVIGATION_STATE
{
    BOOL ROTATE;
    BOOL MOVE;

    BOOL AVOID_FRONT;
    BOOL AVOID_RIGHT;
    BOOL AVOID_LEFT;
    BOOL AVOID_REAR;
    
    BOOL ALT_LOW;
    BOOL ALT_HIGH;
    BOOL ALT_MID;
    
} eAPP_NAVIGATION_STATE;

typedef struct _sAPP_IMAGE_BOARD_DATA
{
    BOOL modified;
    int16_t x_distance;
    int16_t y_distance;
    int16_t z_distance;
    int16_t rotation;                      // Centidegrees
    
} sAPP_IMAGE_BOARD_DATA;

typedef struct _sAPP_PROXIMITY_DATA
{
    BOOL modified[6];       // Indexed by eAPP_AXIS value
    uint16_t distance[6];   // Indexed by eAPP_AXIS value
    
} sAPP_PROXIMITY_DATA;

typedef struct _sAPP_IR_DATA
{
    uint8_t guide_within_sight; 
    
} sAPP_IR_DATA;

typedef struct _sAPP_NAVIGATION_DATA
{
    eAPP_NAVIGATION_AXIS_INTENSITY x_axis;
    eAPP_NAVIGATION_AXIS_INTENSITY y_axis;
    uint8_t z_distance;
    int16_t rotation_speed;                 // Centidegrees
    
    // Returned Rotation
    uint16_t returned_rotation;
    // Returned Velocity
    int16_t returned_velocity;
    
} sAPP_NAVIGATION_DATA;

typedef struct _sAPP_NAVIGATION_FLAGS
{
    BOOL trigger_edge_on_image;  // Used to tell image board that frontal collision detected
    BOOL rotation_status;        // Used to determine if rotation has completed
    
} sAPP_NAVIGATION_FLAGS;

typedef struct _sAPP_NAVIGATION_CBLK
{
    sAPP_IMAGE_BOARD_DATA   image_board_data;
    sAPP_PROXIMITY_DATA     proximity_data;
    sAPP_IR_DATA            ir_data;
    sAPP_NAVIGATION_DATA    navigation_data;
    sAPP_NAVIGATION_FLAGS   navigation_flags;
    
} sAPP_NAVIGATION_CBLK;


/* Exported functions ------------------------------------------------------- */
void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength);
uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
char* Translate_HAL_Status(HAL_StatusTypeDef status);
void Error_Handler(void);

void Heartbeat_Start(void);
void Heartbeat_Update(sAPP_NAVIGATION_CBLK* navigation_cblk);
void Heartbeat_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void APP_Log_Init(void);

#endif /* #ifndef __APP_COMMON_H */
