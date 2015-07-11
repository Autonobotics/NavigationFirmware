/**
  ******************************************************************************
  * @file    Src/app_hc_sr04.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application HC_SR04 Sensor Implementation
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "app_ultrasonic_adapter.h"

/* Public variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;

/* Private variables --------------------------------------------------------*/
static sAPP_HC_SR04_CBLK AppHcsr04Cblk = {
    HC_SR04_INIT,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    FALSE,
    FRONT_LEFT_RIGHT,
    PULSE_FRONT,
    {0}
};

/* Private function prototypes -----------------------------------------------*/
static void hc_sr04_TIM2_init(void);
static void hc_sr04_TIM3_init(void);
static void hc_sr04_TIM4_init(void);
static void hc_sr04_TIM5_init(void);
static void hc_sr04_TIM6_init(void);
static void hc_sr04_start_pulse(void);


/* Private functions ---------------------------------------------------------*/
extern float front_distance;
extern float down_distance;
extern float back_distance;
extern float left_distance;
extern float right_distance;

/******************************************************************************/
/*                 Internal Support Functions                                 */
/******************************************************************************/
/* TIM2 init function */
void hc_sr04_TIM2_init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_IC_InitTypeDef sConfigIC;
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 8400;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 180;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
    
    HAL_TIM_IC_Init(&htim2);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
    
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1);
    
    AppHcsr04Cblk.tim2Handle = & htim2;
}

/* TIM3 init function */
void hc_sr04_TIM3_init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_IC_InitTypeDef sConfigIC;
    
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 8400;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 180;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim3);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);
    
    HAL_TIM_IC_Init(&htim3);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
    
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);
    
    HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2);
    
    AppHcsr04Cblk.tim3Handle = & htim3;
}

/* TIM4 init function */
void hc_sr04_TIM4_init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_IC_InitTypeDef sConfigIC;
    
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 8400;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 180;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim4);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig);
    
    HAL_TIM_IC_Init(&htim4);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig);
    
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1);
    
    HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_2);
    
    AppHcsr04Cblk.tim4Handle = & htim4;
}

/* TIM5 init function */
static void hc_sr04_TIM5_init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    
    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 84;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 10;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim5);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig);
    
    AppHcsr04Cblk.tim5Handle = & htim5;
}

/* TIM6 init function */
static void hc_sr04_TIM6_init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig;
    
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 8400;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 200;
    HAL_TIM_Base_Init(&htim6);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
    
    AppHcsr04Cblk.tim6Handle = & htim6;
}


static void hc_sr04_start_pulse(void)
{
    BOOL status;
    
    switch(AppHcsr04Cblk.pulse_indv)
    {
        case PULSE_FRONT:
            status = us_front();
            if (TRUE == status )
            {
                APP_Log("Ultrasonic: Front %f\r\n", front_distance);
            }
            break;
        case PULSE_REAR:
            status = us_back();
            if (TRUE == status )
            {
                APP_Log("Ultrasonic: Rear %f\r\n", back_distance);
            }
            break;
        case PULSE_LEFT:
            status = us_left();
            if (TRUE == status )
            {
                APP_Log("Ultrasonic: Left %f\r\n", left_distance);
            }
            break;
        case PULSE_RIGHT:
            status = us_right();
            if (TRUE == status )
            {
                APP_Log("Ultrasonic: Right %f\r\n", right_distance);
            }
            break;
        case PULSE_BOTTOM:
            status = us_down();
            if (TRUE == status )
            {
                APP_Log("Ultrasonic: Bottom %f\r\n", down_distance);
            }
            break;
        default:
            status = TRUE;
    }
    if ( TRUE == status )
    {
        AppHcsr04Cblk.pulse_indv++;
        AppHcsr04Cblk.pulse_indv = AppHcsr04Cblk.pulse_indv % PULSE_LIMIT;
    }
}


/* Public functions ---------------------------------------------------------*/
void APP_HC_SR04_Init(void)
{
    // Setup the Ultrasonic Component Hardware
    APP_Log("Configuring Ultrasonics Components.\r\n");
    
    // Initialize the Timers
    hc_sr04_TIM2_init();
    hc_sr04_TIM3_init();
    hc_sr04_TIM4_init();
    hc_sr04_TIM5_init();
    hc_sr04_TIM6_init();
    
    // Initialize the EXTI Pins
    HAL_HC_SR04_MspInit();
    
    // Change out of Initialization State
    AppHcsr04Cblk.state = HC_SR04_ENABLED;
}

eAPP_STATUS APP_HC_SR04_Pulse_Sensors(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    if ( HC_SR04_ENABLED == AppHcsr04Cblk.state )
    {
        hc_sr04_start_pulse();
    }
    
    return status;
}

/* -------------------------- End of File ------------------------------------*/
