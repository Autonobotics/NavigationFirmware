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


TIM_HandleTypeDef htim11;
static eAPP_NAVIGATION_STATE nav_state;
static uint16_t reference_rotation;
static uint16_t desired_rotation;
static int16_t rotation_direction;
static BOOL first_call = TRUE; //used for first rotation call
static volatile BOOL internal_guide_check;

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
    uint8_t temp = 0;
    
    // Altitude Management
    if ( navigation_cblk->proximity_data.distance[AXIS_BOTTOM] == HC_SR04_OUT_OF_RANGE )
    {
        navigation_cblk->navigation_data.z_distance = 100;
    }
    else
    {
        // Ensure distance is not greater than 8bit uint range
        if ( navigation_cblk->proximity_data.distance[AXIS_BOTTOM] > (uint16_t)255 )
        {
            temp = 255;
        }
        else
        {
            temp = (uint8_t) navigation_cblk->proximity_data.distance[AXIS_BOTTOM];
        }
        navigation_cblk->navigation_data.z_distance = temp;
    }
    
    //Collision Avoidance
    // Avoid objects ahead and behind by dodging left, right, or freezing.
    //if( nav_state.AVOID_FRONT && nav_state.AVOID_REAR )
    //{
    //    if( !nav_state.AVOID_RIGHT )
    //    {
    //        navigation_cblk->navigation_data.x_axis = POSITIVE_FAST; //move right
    //        navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
    //    } 
    //    else if ( !nav_state.AVOID_LEFT )
    //    {
    //        navigation_cblk->navigation_data.x_axis = NEGATIVE_FAST; //move left
    //        navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
    //    }
    //    else 
    //    {
    //        navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY; //sit idle
    //        navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
    //    }
    //    return;
    //}
    //
    //// Avoid objects left and right by dodging forward, back, or freezing
    //if( nav_state.AVOID_LEFT && nav_state.AVOID_RIGHT )
    //{
    //    if( !nav_state.AVOID_FRONT )
    //    {
    //        navigation_cblk->navigation_data.y_axis = POSITIVE_FAST; //move forward
    //        navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY;
    //    }
    //    else if ( !nav_state.AVOID_REAR )
    //    {
    //        navigation_cblk->navigation_data.y_axis = NEGATIVE_FAST; //move backward
    //        navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY;
    //    }
    //    else
    //    {
    //        navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY; //sit idle
    //        navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
    //    }
    //    return;
    //}
    //
    //// Exclusing the cases above, avoid any/all remaining cases
    //if( nav_state.AVOID_FRONT || nav_state.AVOID_REAR || nav_state.AVOID_LEFT || nav_state.AVOID_RIGHT)
    //{
    //    if( nav_state.AVOID_FRONT)
    //    {
    //        navigation_cblk->navigation_data.y_axis = NEGATIVE_FAST; //move backward
    //    }
    //    if( nav_state.AVOID_REAR)
    //    {
    //        navigation_cblk->navigation_data.y_axis = POSITIVE_FAST; //move forward
    //    }
    //    if( nav_state.AVOID_LEFT)
    //    {
    //        navigation_cblk->navigation_data.x_axis = POSITIVE_FAST; //move right
    //    }
    //    if( nav_state.AVOID_RIGHT)
    //    {
    //        navigation_cblk->navigation_data.x_axis = NEGATIVE_FAST; //move left
    //    }
    //    return;
    //}
    //
    //// Check Rotation flag
    //if( nav_state.ROTATE )
    //{
    //    navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY; // don't move
    //    navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY; // don't move
    //    
    //    reference_rotation = navigation_cblk->navigation_data.returned_rotation; // current, from 0 to 36000 centidegrees
    //    
    //    if( first_call ) // set reference values
    //    {
    //        rotation_direction = (navigation_cblk->image_board_data.rotation)*100; //positive or negative angle
    //        
    //        //correct for below-zero case
    //        if( (rotation_direction < 0) && (reference_rotation < (rotation_direction*-1)) )
    //        {
    //            desired_rotation = ROTATION_MAX + rotation_direction + reference_rotation; //rotation_direction is negative
    //        }
    //        //correct for above-max case
    //        else if( (rotation_direction > 0) && ((reference_rotation + (uint16_t)rotation_direction) > ROTATION_MAX) )
    //        {
    //            desired_rotation = rotation_direction + reference_rotation - ROTATION_MAX; //rotation_direction is negative
    //        }
    //        else
    //        {
    //            desired_rotation = rotation_direction + reference_rotation;
    //        }
    //        first_call = FALSE;
    //    }
    //    
    //    //check if rotation is within desired bounds
    //    if( APP_Navigation_Check_Rotation(desired_rotation,reference_rotation) ) 
    //    {
    //        navigation_cblk->navigation_flags.rotation_status = ROTATION_COMPLETE;
    //        nav_state.ROTATE = FALSE;
    //        first_call = TRUE; //reset call flag
    //        return;
    //    }
    //    if( rotation_direction < 0 ) // negative is left
    //    {
    //        navigation_cblk->navigation_data.rotation_speed = ROTATE_RIGHT; //rotate left at set rate
    //    }
    //    if( rotation_direction > 0 ) // positive is right
    //    {
    //        navigation_cblk->navigation_data.rotation_speed = ROTATE_RIGHT; //rotate left at set rate
    //    }
    //    return;
    //}
    
    // Movement phase, given no avoidance or rotation
    if ( !navigation_cblk->ir_data.guide_within_sight )
    {
        HAL_TIM_Base_Start_IT(&htim11);
    }
    else
    {
        HAL_TIM_Base_Stop_IT(&htim11);
        internal_guide_check = TRUE;
    }
    
    //if ( internal_guide_check )
    //{
    //    if( nav_state.MOVE )
    //    {
    //        navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY; // don't move sideways
    //        navigation_cblk->navigation_data.y_axis =  POSITIVE_FAST; // move forward
    //    
    //        if( navigation_cblk->image_board_data.x_distance > 10) //10cm tolerance
    //        {
    //            navigation_cblk->navigation_data.x_axis = POSITIVE_SLOW; // move right
    //        }
    //        else if ( navigation_cblk->image_board_data.x_distance < -10 ) //10cm tolerance
    //        {
    //            navigation_cblk->navigation_data.x_axis = NEGATIVE_SLOW; // move left
    //        }
    //        // does not compensate for altitude
    //    }
    //}
    return;
}

void APP_Guide_Timeout_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    HAL_TIM_Base_Stop_IT(&htim11);
    internal_guide_check = FALSE;
    APP_Log("Guide Timeout Occured.\r\n");
}

void APP_Guide_Timeout_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig;
    
    htim11.Instance = TIM11;
    htim11.Init.Prescaler = 42000;
    htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
    htim11.Init.Period = 20000;
    HAL_TIM_Base_Init(&htim11);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim11, &sMasterConfig);
    
    internal_guide_check = TRUE;
}

eAPP_STATUS APP_Navigation_Compute(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    //CHECK_PROXIMITY(navigation_cblk);
    //CHECK_CAMERA(navigation_cblk);
    
    NAV_DECISION(navigation_cblk);
    
    return status;
}

/* -------------------------- End of File ------------------------------------*/
