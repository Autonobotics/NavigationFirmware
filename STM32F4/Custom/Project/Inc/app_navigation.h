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


/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

/* Exported constants --------------------------------------------------------*/
#define HC_SR04_OUT_OF_RANGE 0xFFFF

#define ROTATION_COMPLETE TRUE
#define ROTATION_INCOMPLETE FALSE

#define ROTATION_EPSILON    100 // TODO: FIND REAL VALUE in Centidegrees
#define ROTATION_LOWER_BOUND(_desired_) (_desired_ - (ROTATION_EPSILON >> 2))
#define ROTATION_UPPER_BOUND(_desired_) (_desired_ + (ROTATION_EPSILON >> 2))

/* Exported types ------------------------------------------------------------*/
typedef enum _eAPP_NAVIGATION_AXIS_INTENSITY
{
    IDLE_INTENSITY = 0,
    
    POSITIVE_SLOW,
    POSITIVE_FAST,
    
    NEGATIVE_SLOW,
    NEGATIVE_FAST
    
} eAPP_NAVIGATION_AXIS_INTENSITY;

typedef struct _sAPP_IMAGE_BOARD_DATA
{
    BOOL modified;
    uint16_t x_distance;
    uint16_t y_distance;
    uint16_t z_distance;
    uint16_t rotation;                      // Centidegrees
    
} sAPP_IMAGE_BOARD_DATA;

typedef struct _sAPP_HC_SR04_DATA
{
    BOOL modified[6];       // Indexed by eAPP_AXIS value
    uint16_t distance[6];   // Indexed by eAPP_AXIS value
    
} sAPP_HC_SR04_DATA;

typedef struct _sAPP_IR_DATA
{
    uint8_t guide_within_sight; 
    
} sAPP_IR_DATA;

typedef struct _sAPP_NAVIGATION_DATA
{
    eAPP_NAVIGATION_AXIS_INTENSITY x_axis;
    eAPP_NAVIGATION_AXIS_INTENSITY y_axis;
    eAPP_NAVIGATION_AXIS_INTENSITY z_axis;
    uint16_t rotation_absolute;             // Centidegrees
    
} sAPP_NAVIGATION_DATA;

typedef struct _sAPP_NAVIGATION_FLAGS
{
    BOOL trigger_edge_on_image;  // Used to tell image board that frontal collision detected
    BOOL rotation_status;        // Used to determine if rotation has completed
    
} sAPP_NAVIGATION_FLAGS;

typedef struct _sAPP_NAVIGATION_CBLK
{
    sAPP_IMAGE_BOARD_DATA image_board_data;
    sAPP_HC_SR04_DATA hc_sr04_data;
    sAPP_IR_DATA ir_data;
    sAPP_NAVIGATION_DATA navigation_data;
    sAPP_NAVIGATION_FLAGS navigation_flags;
    
} sAPP_NAVIGATION_CBLK;


/* Exported functions ------------------------------------------------------- */
BOOL APP_Navigation_Check_Rotation(uint16_t desired_rotation, uint16_t current_rotation);
eAPP_STATUS APP_Navigation_Compute(sAPP_NAVIGATION_CBLK* navigation_cblk);


#endif // __APP_NAVIGATION_H
