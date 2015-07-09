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
TIM_HandleTypeDef delayHandle;

/* Private variables --------------------------------------------------------*/
static sAPP_HC_SR04_CBLK AppHcsr04Cblk = {
    HC_SR04_INIT,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    FALSE,
    FRONT_LEFT_RIGHT,
    {0}
};

/* Private function prototypes -----------------------------------------------*/
static void hc_sr04_TIM2_init(void);
static void hc_sr04_TIM3_init(void);
static void hc_sr04_TIM4_init(void);
static void hc_sr04_Delay_init(void);
static void hc_sr04_Stop_Timer(TIM_HandleTypeDef *htim);
static void hc_sr04_start_pulse(eAPP_HC_SR04_PULSE_SET pulse_set);


/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*                 HAL Callback Functions                                     */
/******************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if( TIM5 == htim->Instance)
    {
        // Ensure the Timer is Stoped
        HAL_TIM_Base_Stop_IT(htim);
        // Depending on the Set, read and stop channel
        if ( FRONT_LEFT_RIGHT == AppHcsr04Cblk.pulse_set )
        {
            // Set FRL Trigger Pin to 0
            HAL_GPIO_WritePin(HC_SR04_FLR_TRIGGER_PORT, 
                              HC_SR04_FLR_TRIGGER_PIN, 
                              GPIO_PIN_RESET);
        }
        else
        {
            // Set FBD Trigger Pin to 0
            HAL_GPIO_WritePin(HC_SR04_FBR_TRIGGER_PORT, 
                              HC_SR04_FBR_TRIGGER_PIN, 
                              GPIO_PIN_RESET);
        }
        // Reset the Timer value
        __HAL_TIM_SET_COUNTER(htim, 0);
        
        HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    }
}

static void hc_sr04_Stop_Timer(TIM_HandleTypeDef *htim)
{
    if( TIM2 == htim->Instance )
    {
        // Update the count variable
        AppHcsr04Cblk.received_count++;
        // Stop the Timer
        HAL_TIM_Base_Stop_IT(htim);
        // Save the Comparison Value
        AppHcsr04Cblk.data.fbr_data.set[HC_SR04_FRONT] = __HAL_TIM_GetCounter(htim);
        AppHcsr04Cblk.data.flr_data.set[HC_SR04_FRONT] = __HAL_TIM_GetCounter(htim);
        // Reset counter after input capture interrupt occurs
        __HAL_TIM_SET_COUNTER(htim, 0);
        
        HAL_NVIC_DisableIRQ(EXTI0_IRQn);
    }
    
    if( TIM3 == htim->Instance )
    {
        // Update the count variable
        AppHcsr04Cblk.received_count++;
        // Stop the Timer
        HAL_TIM_Base_Stop_IT(htim);
        // Depending on the Set, read and stop channel
        if ( FRONT_LEFT_RIGHT == AppHcsr04Cblk.pulse_set )
        {
            // Save the Comparison Value
            AppHcsr04Cblk.data.flr_data.set[HC_SR04_LEFT] = __HAL_TIM_GetCounter(htim);
        }
        else
        {
            // Save the Comparison Value
            AppHcsr04Cblk.data.fbr_data.set[HC_SR04_BOTTOM] = __HAL_TIM_GetCounter(htim);
        }
        // Reset counter after input capture interrupt occurs
        __HAL_TIM_SET_COUNTER(htim, 0); 
    }
    
    if( TIM4 == htim->Instance )
    {
        // Update the count variable
        AppHcsr04Cblk.received_count++;
        // Stop the Timer
        HAL_TIM_Base_Stop_IT(htim);
        // Depending on the Set, read and stop channel
        if ( FRONT_LEFT_RIGHT == AppHcsr04Cblk.pulse_set )
        {
            // Save the Comparison Value
            AppHcsr04Cblk.data.flr_data.set[HC_SR04_RIGHT] = __HAL_TIM_GetCounter(htim);
        }
        else
        {
            // Save the Comparison Value
            AppHcsr04Cblk.data.fbr_data.set[HC_SR04_REAR] = __HAL_TIM_GetCounter(htim);
        }
        // Reset counter after input capture interrupt occurs
        __HAL_TIM_SET_COUNTER(htim, 0);
    }
}

void APP_HC_SR04_Handle_EXTI(TIM_HandleTypeDef *htim, GPIO_TypeDef* port, uint16_t pin)
{
    if ( HC_SR04_ENABLED == AppHcsr04Cblk.state )
    {
        if ( HAL_GPIO_ReadPin(port, pin) )
        {
            HAL_TIM_Base_Start_IT(htim);
        }
        else
        {
            hc_sr04_Stop_Timer(htim);
        }
    }
    
    HAL_GPIO_EXTI_IRQHandler(pin);
}

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
    htim2.Init.Period = 65000;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
    
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
    htim3.Init.Period = 65000;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim3);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
    
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
    htim4.Init.Period = 65000;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim4);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig);
    
    AppHcsr04Cblk.tim4Handle = & htim4;
}

static void hc_sr04_Delay_init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    
    delayHandle.Instance = TIM5;
    delayHandle.Init.Prescaler = 84;
    delayHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    delayHandle.Init.Period = 10;
    delayHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&delayHandle);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&delayHandle, &sClockSourceConfig);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&delayHandle, &sMasterConfig);
    
    AppHcsr04Cblk.delayHandle = & delayHandle;
}

static void hc_sr04_start_pulse(eAPP_HC_SR04_PULSE_SET pulse_set)
{
    // Depending on the Set, read and stop channel
    if ( FRONT_LEFT_RIGHT == AppHcsr04Cblk.pulse_set )
    {
        // Set FRL Trigger Pin to 1
        HAL_GPIO_WritePin(HC_SR04_FLR_TRIGGER_PORT, 
                          HC_SR04_FLR_TRIGGER_PIN, 
                          GPIO_PIN_SET);
    }
    else
    {
        // Set FBD Trigger Pin to 1
        HAL_GPIO_WritePin(HC_SR04_FBR_TRIGGER_PORT,
                          HC_SR04_FBR_TRIGGER_PIN,
                          GPIO_PIN_SET);
    }
    
    // Start timer
    HAL_TIM_Base_Start_IT(AppHcsr04Cblk.delayHandle);
}


/* Public functions ---------------------------------------------------------*/
void APP_HC_SR04_Init(void)
{
    // Setup the Ultrasonic Component Hardware
    APP_Log("Configuring Ultrasonics Components.\r\n");
    
    // Initialize the Timers
    hc_sr04_TIM2_init();
    //hc_sr04_TIM3_init();
    //hc_sr04_TIM4_init();
    hc_sr04_Delay_init();
    
    // Initialize the EXTI Pins
    HAL_HC_SR04_MspInit();
    
    // Change out of Initialization State
    AppHcsr04Cblk.state = HC_SR04_WAITING;
}

eAPP_STATUS APP_HC_SR04_Pulse_Sensors(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    float distanceR = 0;
    float distanceL = 0;
    float distanceF = 0;
    float timeR = 0;
    float timeL = 0;
    float timeF = 0;
    
    if ( HC_SR04_ENABLED == AppHcsr04Cblk.state )
    {
        if ( FALSE == AppHcsr04Cblk.pulsed_ongoing )
        {
            // Log
            APP_Log("HC_SR04: Firing Sensors.\r\n");
            
            // Set Pulse Ongoing Flag
            AppHcsr04Cblk.pulsed_ongoing = TRUE;
            
            // Clear the Received Count variable
            AppHcsr04Cblk.received_count = 0;
            
            // Start a new pulse
            hc_sr04_start_pulse(AppHcsr04Cblk.pulse_set);
            
        }
        //HC_SR04_SENSORS_PER_SET
        if ( 1 <= AppHcsr04Cblk.received_count )
        {
            // Compute new Readings for the given Sensor set
            if ( FRONT_LEFT_RIGHT == AppHcsr04Cblk.pulse_set )
            {
                timeR = AppHcsr04Cblk.data.flr_data.set[HC_SR04_RIGHT]*0.0001;
                timeL = AppHcsr04Cblk.data.flr_data.set[HC_SR04_LEFT]*0.0001;
                timeF = AppHcsr04Cblk.data.flr_data.set[HC_SR04_FRONT]*0.0001;
                distanceR = timeR*340/2;
                distanceL = timeL*340/2;
                distanceF = timeF*340/2;
                APP_Log("HC_SR04 Measured: Front %f. Right %f. Left %f.\r\n",distanceF, distanceR, distanceL);
            }
            else if ( FRONT_BOTTOM_REAR == AppHcsr04Cblk.pulse_set )
            {
                
            }
            
            // Change sensor sets
            //AppHcsr04Cblk.pulse_set = !AppHcsr04Cblk.pulse_set;
            
            // Clear Pulse Ongoing Flag
            AppHcsr04Cblk.pulsed_ongoing = FALSE;
            AppHcsr04Cblk.state = HC_SR04_WAITING;
        }
    }
    else if ( HC_SR04_WAITING == AppHcsr04Cblk.state )
    {
        // Start delay timer for a short period to settle
        APP_Log("Waiting\r\n");
        HAL_Delay(1000);
        AppHcsr04Cblk.state = HC_SR04_ENABLED;
    }
    
    return status;
}

/* -------------------------- End of File ------------------------------------*/
