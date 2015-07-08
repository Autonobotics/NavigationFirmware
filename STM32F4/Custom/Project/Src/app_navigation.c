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

eAPP_STATUS APP_Navigation_Compute(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    navigation_cblk->navigation_data.x_axis = IDLE_INTENSITY;
    navigation_cblk->navigation_data.y_axis = IDLE_INTENSITY;
    navigation_cblk->navigation_data.z_axis = IDLE_INTENSITY;
    navigation_cblk->navigation_data.rotation_absolute = navigation_cblk->image_board_data.rotation;
    
    return status;
}

/* -------------------------- End of File ------------------------------------*/
