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

static eAPP_NAVIGATION_STATE nav_state;
static int16_t reference_rotation;
static int16_t desired_rotation;
static int16_t rotation_direction;
static bool first_call = TRUE; //used for first rotation call

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
    if( (navigation_cblk->proximity_data.distance[5] > ALTITUDE_IDLE - ALTITUDE_MARGIN)&&(navigation_cblk->proximity_data.distance[5] < ALTITUDE_IDLE + ALTITUDE_MARGIN) )
    {
        nav_state.ALT_HIGH = FALSE;
        nav_state.ALT_LOW = FALSE;
        nav_state.ALT_MID = TRUE;
    } 
    else if( navigation_cblk->proximity_data.distance[5] > (ALTITUDE_IDLE + ALTITUDE_MARGIN) )
    {
        nav_state.ALT_HIGH = TRUE;
        nav_state.ALT_LOW = FALSE;
        nav_state.ALT_MID = FALSE;
    }
    else if( navigation_cblk->proximity_data.distance[5] < (ALTITUDE_IDLE - ALTITUDE_MARGIN) )
    {
        nav_state.ALT_HIGH = FALSE;
        nav_state.ALT_LOW = TRUE; 
        nav_state.ALT_MID = FALSE;
    }
    return;
} //as is, will oscillate instead of settle @ altitude

static void CHECK_PROXIMITY(sAPP_NAVIGATION_CBLK* navigation_cblk)
{ 
   
        nav_state.AVOID_FRONT = (navigation_cblk->proximity_data.distance[0] < AVOID_THRESHOLD_FRONT);
        nav_state.AVOID_REAR = (navigation_cblk->proximity_data.distance[1] < AVOID_THRESHOLD_REAR);
        nav_state.AVOID_LEFT = (navigation_cblk->proximity_data.distance[2] < AVOID_THRESHOLD_LEFT);
        nav_state.AVOID_RIGHT = (navigation_cblk->proximity_data.distance[3] < AVOID_THRESHOLD_RIGHT);

        // TOP sensor not implemented
}

static void CHECK_CAMERA(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    // To Compensate for Error, the Image board also suggests x movement...

    if (navigation_cblk->image_board_data.z_distance == DISTANCE_UNKNOWN)
    {
        nav_state.MOVE = FALSE;
    }
    else
    {
        nav_state.MOVE = TRUE;
    }
    
    if (navigation_cblk->image_board_data.rotation == ROTATION_UNKNOWN)
    {
        nav_state.ROTATE = FALSE;
    }
    else
    {
        nav_state.ROTATE = TRUE;
    }
    return;
}

static void NAV_DECISION(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    // Altitude control
    if( nav_state.ALT_MID ) 
    {
        //settle altitude
        navigation_cblk->navigation_data.z_axis = IDLE_INTENSITY;
    } 
    else if( nav_state.ALT_HIGH )
    {
        //decrease altitude by ramp coefficent
        navigation_cblk->navigation_data.z_axis = NEGATIVE_FAST;
    } 
    else if( nav_state.ALT_LOW )
    {
        //increase altitude by ramp coefficient
        navigation_cblk->navigation_data.z_axis = POSITIVE_FAST;
    }
    
    //Collision Avoidance
    // Avoid objects ahead and behind by dodging left, right, or freezing.
    if( nav_state.AVOID_FRONT && nav_state.AVOID_REAR )
    {
        if( !nav_state.AVOID_RIGHT )
        {
            navigation_cblk->navigation_data.x_axis = POSITIVE_FAST; //move right
            navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
        } 
        else if ( !nav_state.AVOID_LEFT )
        {
            navigation_cblk->navigation_data.x_axis = NEGATIVE_FAST; //move left
            navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
        }
        else 
        {
            navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY; //sit idle
            navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
        }
        return;
    }
    
    // Avoid objects left and right by dodging forward, back, or freezing
    if( nav_state.AVOID_LEFT && nav_state.AVOID_RIGHT )
    {
        if( !nav_state.AVOID_FRONT )
        {
            navigation_cblk->navigation_data.y_axis = POSITIVE_FAST; //move forward
            navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY;
        }
        else if ( !nav_state.AVOID_REAR )
        {
            navigation_cblk->navigation_data.y_axis = NEGATIVE_FAST; //move backward
            navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY;
        }
        else
        {
            navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY; //sit idle
            navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
        }
        return;
    }
    
    // Exclusing the cases above, avoid any/all remaining cases
    if( nav_state.AVOID_FRONT || nav_state.AVOID_REAR || nav_state.AVOID_LEFT || nav_state.AVOID_RIGHT)
    {
        if( nav_state.AVOID_FRONT)
        {
            navigation_cblk->navigation_data.y_axis = NEGATIVE_FAST; //move backward
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
            navigation_cblk->navigation_data.x_axis = NEGATIVE_FAST; //move left
        }
        return;
    }
    
    // Check Rotation flag
    if( nav_state.ROTATE )
    {
        navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY; // don't move
        navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY; // don't move
        
        reference_rotation = navigation_cblk->navigation_data.returned_rotation; // current, from 0 to 36000 centidegrees
        
        if( first_call ) // set reference values
        {
            rotation_direction = image_board_data.rotation; //positive or negative angle
            desired_rotation = rotation_direction*100 + reference_rotation; // reference plus desired change
            if( desired_rotation > ROTATION_MAX ) desired_rotation -= ROTATION_MAX; //correct for crossing max value
            if( desired_rotation < 0 ) desired_rotation += ROTATION_MAX; //correct for crossing min value
            first_call = FALSE;
        }
        
        //check if rotation is within desired bounds
        if( APP_Navigation_Check_Rotation(desired_rotation,reference_rotation) ) 
        {
            navigation_cblk->navigation_flags.rotation_status = ROTATION_COMPLETE;
            nav_state.ROTATE = FALSE;
            first_call = TRUE; //reset call flag
            return;
        }
        if( rotation_direction < 0 ) // negative is left
        {
            navigation_cblk->navigation_data.rotation_absolute = ROTATE_RIGHT; //rotate left at set rate
        }
        if( rotation_direction > 0 ) // positive is right
        {
            navigation_cblk->navigation_data.rotation_absolute = ROTATE_RIGHT; //rotate left at set rate
        }
        return;
    }
    
    // Movement phase, given no avoidance or rotation

    if( nav_state.MOVE )
    {
        navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY; // don't move sideways
        navigation_cblk->navigation_data.y_axis =  POSITIVE_FAST; // move forward
    
        if( navigation_cblk->image_board_data.x_distance > 10) //10cm tolerance
        {
            navigation_cblk->navigation_data.x_axis = POSITIVE_SLOW; // move right
        }
        else if ( navigation_cblk->image_board_data.x_distance < -10 ) //10cm tolerance
        {
            navigation_cblk->navigation_data.x_axis = NEGATIVE_SLOW; // move left
        }
        else
        {
            navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY; // don't move right or left
        }
        // does not compensate for altitude
    }
    return;
}

eAPP_STATUS APP_Navigation_Compute(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    CHECK_ALTITUDE(navigation_cblk);
    CHECK_PROXIMITY(navigation_cblk);
    
    if ( navigation_cblk->ir_data.guide_within_sight )
    {
        CHECK_CAMERA(navigation_cblk);
    }
    
    NAV_DECISION(navigation_cblk);
    
    return status;
}

/* -------------------------- End of File ------------------------------------*/
