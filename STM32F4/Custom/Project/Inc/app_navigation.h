/**
  ******************************************************************************
  * @file    Inc/app_navigation.h
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Supplies Interface for navigation data structures and methods.
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_NAVIGATION_H
#define __APP_NAVIGATION_H
#define DISTANCE_UNKNOWN 0xFFFF

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

/* Exported constants --------------------------------------------------------*/
#define HC_SR04_OUT_OF_RANGE 0xFFFF

#define FRONTAL_AVOIDANCE_MODE_ON   TRUE
#define FRONTAL_AVOIDANCE_MODE_OFF  FALSE

#define DISTANCE_UNKNOWN 0xFFFF

#define ROTATION_UNKNOWN 0x7FFF  // Centidegrees
#define ROTATION_SEARCH 0x7FFE

#define ROTATION_COMPLETE TRUE
#define ROTATION_INCOMPLETE FALSE

#define ROTATION_EPSILON    100 // Tolerance of a degree. Ie. 100 Centidegrees
#define ROTATION_LOWER_BOUND(_desired_) (_desired_ - (ROTATION_EPSILON >> 1))
#define ROTATION_UPPER_BOUND(_desired_) (_desired_ + (ROTATION_EPSILON >> 1))
#define ROTATE_LEFT -3000;
#define ROTATE_RIGHT 3000;

#define ALTITUDE_IDLE 80 //idle at 50cm
#define ALTITUDE_MARGIN 10 //acceptable error of 10cm
#define AVOID_THRESHOLD_FRONT 30
#define AVOID_THRESHOLD_REAR 30
#define AVOID_THRESHOLD_LEFT 30
#define AVOID_THRESHOLD_RIGHT 30

/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_NAVIGATION_AXIS_INTENSITY
{
    POSITIVE_FAST = 0,
    NEGATIVE_FAST,
    
 //   POSITIVE_SLOW,
 //   NEGATIVE_SLOW,
    
    IDLE_INTENSITY
    
} eAPP_NAVIGATION_AXIS_INTENSITY;

typedef struct _sAPP_NAVIGATION_STATE
{
    BOOL ROTATE;
    BOOL MOVE;
//    BOOL SET_ROTATE;

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
    uint16_t x_distance;
    uint16_t y_distance;
    uint16_t z_distance;
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
    eAPP_NAVIGATION_AXIS_INTENSITY z_axis;
    int16_t rotation_absolute;             // Centidegrees
    
    // Returned Rotation
    int16_t returned_rotation;
    
} sAPP_NAVIGATION_DATA;

typedef struct _sAPP_NAVIGATION_FLAGS
{
    BOOL trigger_edge_on_image;  // Used to tell image board that frontal collision detected
    BOOL rotation_status;        // Used to determine if rotation has completed
    
} sAPP_NAVIGATION_FLAGS;

typedef struct _sAPP_NAVIGATION_CBLK
{
    sAPP_IMAGE_BOARD_DATA image_board_data;
    sAPP_PROXIMITY_DATA proximity_data;
    sAPP_IR_DATA ir_data;
    sAPP_NAVIGATION_DATA navigation_data;
    sAPP_NAVIGATION_FLAGS navigation_flags;
    
} sAPP_NAVIGATION_CBLK;


/* Exported functions ------------------------------------------------------- */
BOOL APP_Navigation_Check_Rotation(uint16_t desired_rotation, uint16_t current_rotation);
eAPP_STATUS APP_Navigation_Compute(sAPP_NAVIGATION_CBLK* navigation_cblk);


#endif // __APP_NAVIGATION_H
