/**
  ******************************************************************************
  * @file    Src/app_navigation.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application Navigation Implementation
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "app_navigation.h"

static BOOL ALT_OVERRIDE;
static eAPP_NAVIGATION_STATE nav_state;

/* Public functions ---------------------------------------------------------*/
BOOL APP_Navigation_Check_Rotation(uint16_t desired_rotation, uint16_t current_rotation)
{
    if( ROTATION_UPPER_BOUND(desired_rotation) > current_rotation
     && ROTATION_LOWER_BOUND(desired_rotation) < current_rotation )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void CHECK_ALTITUDE(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    if( ALT_OVERRIDE )
    {
        nav_state.ALT_HIGH = FALSE;
        nav_state.ALT_LOW = FALSE;
        return;
    } else if( (navigation_cblk->proximity_data.distance[5] > ALTITUDE_IDLE + ALTITUDE_MARGIN) ) {
        nav_state.ALT_HIGH = TRUE;
        nav_state.ALT_LOW = FALSE;
        return;
    } else if( (navigation_cblk->proximity_data.distance[5] < ALTITUDE_IDLE - ALTITUDE_MARGIN) ) {
        nav_state.ALT_HIGH = FALSE;
        nav_state.ALT_LOW = TRUE; 
        return;
    }
} //as is, will oscillate instead of settle @ altitude

static void CHECK_PROXIMITY(sAPP_NAVIGATION_CBLK* navigation_cblk)
{ 
   /* if ( navigation_cblk->proximity_data.distance[0] < AVOID_THRESHOLD_FRONT ) 
        {
            nav_state.AVOID_FRONT = TRUE;
        } else {
            nav_state.AVOID_FRONT = FALSE;
        }
        if ( navigation_cblk->proximity_data.distance[1] < AVOID_THRESHOLD_REAR ) 
        {
            nav_state.AVOID_REAR = TRUE;
        } else {
            nav_state.AVOID_REAR = FALSE;
        } 
        if ( navigation_cblk->proximity_data.distance[2] < AVOID_THRESHOLD_LEFT ) //maybe scale if moving?
        {
            nav_state.AVOID_LEFT = TRUE;
        } else {
            nav_state.AVOID_LEFT = FALSE;
        }
        if ( navigation_cblk->proximity_data.distance[3] < AVOID_THRESHOLD_RIGHT ) //maybe scale if moving?
        {
            nav_state.AVOID_RIGHT = TRUE;
        } else {
            nav_state.AVOID_RIGHT = FALSE;
    */
        nav_state.AVOID_FRONT = (navigation_cblk->proximity_data.distance[0] < AVOID_THRESHOLD_FRONT);
        nav_state.AVOID_REAR = (navigation_cblk->proximity_data.distance[1] < AVOID_THRESHOLD_REAR);
        nav_state.AVOID_LEFT = (navigation_cblk->proximity_data.distance[2] < AVOID_THRESHOLD_LEFT);
        nav_state.AVOID_RIGHT = (navigation_cblk->proximity_data.distance[3] < AVOID_THRESHOLD_RIGHT);

        // TOP sensor not implemented
}

static void CHECK_CAMERA(sAPP_NAVIGATION_CBLK* navigation_cblk)
{

//if (!navigation_cblk->image_board_data.modified) return;

    if (navigation_cblk->image_board_data.z_distance == DISTANCE_UNKNOWN)
    {
        nav_state.ROTATE = TRUE; 
        nav_state.MOVE = FALSE;
        return;
    } else {
        nav_state.MOVE = TRUE;
        nav_state.ROTATE = FALSE;
    }
}

static void NAV_DECISION(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    if( nav_state.ALT_LOW ) 
    {
    //increase altitude
    navigation_cblk->navigation_data.z_axis = POSITIVE_SLOW;
    } else if( nav_state.ALT_HIGH ) {
    
    //decrease altitude
    navigation_cblk->navigation_data.z_axis = NEGATIVE_SLOW;
    }
    
    if( nav_state.AVOID_FRONT || nav_state.AVOID_REAR || nav_state.AVOID_LEFT || nav_state.AVOID_RIGHT)
    {
        if( nav_state.AVOID_FRONT)
        {
            navigation_cblk->navigation_data.y_axis = NEGATIVE_FAST; //move back
        }
        if( nav_state.AVOID_REAR)
        {
            navigation_cblk->navigation_data.y_axis = POSITIVE_FAST; //move forward
        }
        if( nav_state.AVOID_LEFT)
        {
            navigation_cblk->navigation_data.x_axis = POSITIVE_FAST; //move right
            
        }
        if( nav_state.AVOID_RIGHT)
        {
            navigation_cblk->navigation_data.x_axis = POSITIVE_FAST; //move left
            
        }
        return

    } else if( nav_state.ROTATE ) {
    //HOW DO???
    }
    return
}

eAPP_STATUS APP_Navigation_Compute(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    CHECK_ALTITUDE(navigation_cblk);
    CHECK_PROXIMITY(navigation_cblk);
    CHECK_CAMERA(navigation_cblk);
    NAV_DECISION(navigation_cblk);
    
    
    navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY;
    navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
    navigation_cblk->navigation_data.z_axis = IDLE_INTENSITY;
    navigation_cblk->navigation_data.rotation_absolute = navigation_cblk->image_board_data.rotation;
    
    return status;
}

/* -------------------------- End of File ------------------------------------*/
