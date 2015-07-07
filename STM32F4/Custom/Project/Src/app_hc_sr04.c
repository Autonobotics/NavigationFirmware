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
#include "app_hc_sr04.h"

/* Public variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* Private function prototypes -----------------------------------------------*/
static void hc_sr04_TIM2_init(void);
static void hc_sr04_TIM3_init(void);
static void hc_sr04_TIM4_init(void);


/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*                 HAL Callback Functions                                     */
/******************************************************************************/
//once one period for tim2 (10us) is done this function is called
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM2)
        {
            HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);//set trig = 0
            HAL_TIM_Base_Stop_IT(&htim2);//stop the 10us timer
        }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM3)
    {
        //once the falling edge triggers the capture value, reset+stop counter
        HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
        __HAL_TIM_SET_COUNTER(&htim3,0);//reset counter after input capture interrupt occurs
        HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,GPIO_PIN_RESET);//turn off blue led
    }
    
    if(htim->Instance==TIM4)
    {
        //once the falling edge triggers the capture value, reset+stop counter
        HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_1);
        __HAL_TIM_SET_COUNTER(&htim4,0);//reset counter after input capture interrupt occurs
    }
}

/******************************************************************************/
/*                 Internal Support Functions                                 */
/******************************************************************************/
/* TIM2 init function */
void hc_sr04_TIM2_init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 84;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 10;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
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
    htim3.Init.Period = 65000;
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
    sConfigIC.ICFilter = 15;
    HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);
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
    htim4.Init.Period = 65000;
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
    sConfigIC.ICFilter = 15;
    HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1);
}

void SOMETHING_CODE(void)
{
    long echo_captureR = 0;
    long echo_captureL = 0;
    float distanceR = 0;
    float distanceL = 0;
    float timeR = 0;
    float timeL = 0;
    float threshold = 0.3;//30cm
    
    echo_captureR= __HAL_TIM_GetCompare(&htim3, TIM_CHANNEL_1);//read TIM3 channel 1 capture value
    echo_captureL= __HAL_TIM_GetCompare(&htim4, TIM_CHANNEL_1);
    timeR= echo_captureR*0.0001; //100us per counter value
    timeL = echo_captureL*0.0001;
    distanceR = timeR*340/2;
    distanceL = timeL*340/2;
    
    
    if(distanceR < threshold)//less then 30cm
        {
            HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_SET);//turn on orange led
            //turn off other leds
            HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET);//turn off green led
            HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);//turn off red led
        }
    else if(distanceR > threshold )
        {
            HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET);//turn off green led
            HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_RESET);//turn off orange led
        }
}


/* Public functions ---------------------------------------------------------*/
void APP_HC_SR04_Init(void)
{
    // Setup the Ultrasonic Component Hardware
    APP_Log("Configuring Ultrasonics Components.\r\n");
    hc_sr04_TIM2_init();
    hc_sr04_TIM3_init();
    hc_sr04_TIM4_init();
    HAL_HC_SR04_MspInit();
}

eAPP_STATUS APP_HC_SR04_Pulse_Sensors(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    
    
    return status;
}

/* -------------------------- End of File ------------------------------------*/
