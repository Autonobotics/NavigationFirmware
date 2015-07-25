/**
  ******************************************************************************
  * @file    Src/app_usart.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application USART Implementation
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim10;
static volatile BOOL heartbeat_ready;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*                 Utility Functions and Helpers                              */
/******************************************************************************/
char* Translate_HAL_Status(HAL_StatusTypeDef status)
{
    switch (status)
    {
        case HAL_OK:
            return "HAL_OK";
        case HAL_ERROR:
            return "HAL_ERROR";
        case HAL_BUSY:
            return "HAL_BUSY";
        case HAL_TIMEOUT:
            return "HAL_TIMEOUT";
        default:
            return "UNKNOWN HAL_STATUS";
    }
}


/**
  * @brief  Flushes the buffer
  * @param  pBuffer: buffers to be flushed.
  * @param  BufferLength: buffer's length
  * @retval None
  */
void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength)
{
    while (BufferLength--)
    {
        *pBuffer = 0;
    
        pBuffer++;
    }
}


/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 0  : pBuffer1 identical to pBuffer2
  *         >0 : pBuffer1 differs from pBuffer2
  */
uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
    while (BufferLength--)
    {
        if ((*pBuffer1) != *pBuffer2)
        {
        return BufferLength;
        }
        pBuffer1++;
        pBuffer2++;
    }
    
    return 0;
}


/******************************************************************************/
/*                 Error Code and Handlers                                    */
/******************************************************************************/
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    while(1)
    {
        if ( !HAL_GPIO_ReadPin(LED5_GPIO_PORT, LED5_PIN) )
        {
            /* Turn BSP_HARD_ERROR_LED on */
            BSP_LED_On(BSP_HARD_ERROR_LED);
        }
    }
}

void Heartbeat_Start(void)
{
    TIM_MasterConfigTypeDef sMasterConfig;
    
    htim10.Instance = TIM10;
    htim10.Init.Prescaler = 42000;
    htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
    htim10.Init.Period = 1000;
    HAL_TIM_Base_Init(&htim10);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim10, &sMasterConfig);
    
    heartbeat_ready = FALSE;
    HAL_TIM_Base_Start_IT(&htim10);
}


void Heartbeat_Update(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    static int i = 0;
    
    if ( heartbeat_ready )
    {
        heartbeat_ready = FALSE;
        BSP_LED_Toggle(BSP_HARD_ERROR_LED);
        HAL_TIM_Base_Start_IT(&htim10);
        
        // Log Current State
        if ( i == 0 )
        {
            // Log Ultrasonics
            APP_Log("Ultrasonic: Front  %u\r\n", navigation_cblk->proximity_data.distance[AXIS_FRONT]);
            APP_Log("Ultrasonic: Rear   %u\r\n", navigation_cblk->proximity_data.distance[AXIS_REAR]);
            APP_Log("Ultrasonic: Left   %u\r\n", navigation_cblk->proximity_data.distance[AXIS_LEFT]);
            APP_Log("Ultrasonic: Right  %u\r\n", navigation_cblk->proximity_data.distance[AXIS_RIGHT]);
            APP_Log("Ultrasonic: Bottom %u\r\n", navigation_cblk->proximity_data.distance[AXIS_BOTTOM]);
        }
        else if ( i == 1 )
        {
            // Log Image Data
            APP_Log("ImageBoard: X         %hd\r\n", navigation_cblk->image_board_data.x_distance);
            APP_Log("ImageBoard: Y         %hd\r\n", navigation_cblk->image_board_data.y_distance);
            APP_Log("ImageBoard: Z         %hd\r\n", navigation_cblk->image_board_data.z_distance);
            APP_Log("ImageBoard: Rotation  %hd\r\n", navigation_cblk->image_board_data.rotation);
            
            // Log Guide Data
            APP_Log("GuideIR:    Guide  %u\r\n", navigation_cblk->ir_data.guide_within_sight);
        }
        else if ( i == 2 )
        {
            // Log PIXARM Return Data
            APP_Log("Pixhawk: X Intensity %hd\r\n", (uint16_t) navigation_cblk->navigation_data.x_axis);
            APP_Log("Pixhawk: Y Intensity %hd\r\n", (uint16_t) navigation_cblk->navigation_data.y_axis);
            APP_Log("Pixhawk: Z Distance  %hd\r\n", (uint16_t) navigation_cblk->navigation_data.z_distance);
            APP_Log("Pixhawk: Rotation    %hd\r\n", navigation_cblk->navigation_data.returned_rotation);
            APP_Log("Pixhawk: Z Velocity  %hd\r\n", navigation_cblk->navigation_data.returned_velocity);
        }
        
        // Roll over i
        if (++i >= 3)
        {
            i = i % 3;
        }
    }
}


void Heartbeat_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    HAL_TIM_Base_Stop_IT(&htim10);
    heartbeat_ready = TRUE;
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    // Alert the user of the file and line number
    printf("Error occured in file %s on line %d\r\n", file, line);
    
    /* Infinite loop */
    while (1)
    {
    }
}
#endif
