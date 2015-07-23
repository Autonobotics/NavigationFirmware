/**
  ******************************************************************************
  * @file    Src/stm32f4xx_hal_msp.c
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   HAL MSP module.
  ******************************************************************************
  */ 
/* Includes ------------------------------------------------------------------*/
// User Implementation of HAL_MSP functions in #include "stm32f4xx_hal.h"
#include "app_common.h"
#include "app_pixarm.h"
#include "app_armpit.h"
#include "app_ir.h"
#include "app_ultrasonic_adapter.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
    /* System interrupt init*/
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_1);
    
    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


void HAL_IR_MspInit(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    IR_CLOCK_ENABLE();
    /* Configure PD0 pin as input floating */
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;

    //nopull the Vout of the TSOP34840 will drive it 3.3-0V (no IR - sees IR)
    GPIO_InitStructure.Pull = IR_GPIO_PULLUPDOWN; 
    GPIO_InitStructure.Pin = IR_PIN;
    HAL_GPIO_Init(IR_GPIO_PORT, &GPIO_InitStructure);
}


void HAL_HC_SR04_MspInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __GPIOE_CLK_ENABLE();
    __GPIOH_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    
    /*Configure GPIO pin : PE3 */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    /*Configure GPIO pins : PC0 PC1 PC2 PC3 PC4 */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2
                         |GPIO_PIN_3|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*Configure GPIO pin : PB2 */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*Configure GPIO pins : PE10 PE11 PE12 PE13 PE14 */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12
                         |GPIO_PIN_13|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    /*Configure GPIO pins : PD12 PD13 PD14 PD15 PD4 */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                         |GPIO_PIN_15|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    /* EXTI interrupt init */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    
    HAL_NVIC_SetPriority(EXTI1_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    
    HAL_NVIC_SetPriority(EXTI2_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
    
    HAL_NVIC_SetPriority(EXTI3_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
    
    HAL_NVIC_SetPriority(EXTI4_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
}


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    GPIO_InitTypeDef GPIO_InitStruct;
        
    if ( TIM2 == htim_base->Instance )
    {
        /* Peripheral clock enable */
        __TIM2_CLK_ENABLE();
    
        /* TIM2 GPIO Configuration    
            PA15 --> TIM2_CH1 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
        /* Peripheral interrupt init */
        HAL_NVIC_SetPriority(TIM2_IRQn, 1, 3);
    }
    else if ( TIM3 == htim_base->Instance )
    {
        /* Peripheral clock enable */
        __TIM3_CLK_ENABLE();
    
        /* TIM3 GPIO Configuration    
            PA6 --> TIM3_CH1
            PA7 --> TIM3_CH2 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(TIM3_IRQn, 1, 3);
    }
    else if ( TIM4 == htim_base->Instance )
    {
        /* Peripheral clock enable */
        __TIM4_CLK_ENABLE();
    
        /* TIM4 GPIO Configuration    
            PB6 --> TIM4_CH1
            PB7 --> TIM4_CH2 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(TIM4_IRQn, 1, 3);
    }
    else if ( TIM5 == htim_base->Instance )
    {
        /* Peripheral clock enable */
        __TIM5_CLK_ENABLE();
        
        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(TIM5_IRQn, 1, 3);
    }
    else if ( TIM6 == htim_base->Instance )
    {
        /* Peripheral clock enable */
        __TIM6_CLK_ENABLE();
        
        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 3);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    }
    else if ( TIM10 == htim_base->Instance )
    {
        /* Peripheral clock enable */
        __TIM10_CLK_ENABLE();
        
        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1, 4);
        HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
    }
    else if ( TIM11 == htim_base->Instance )
    {
        /* Peripheral clock enable */
        __TIM11_CLK_ENABLE();
        
        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 1, 4);
        HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
    }
}


void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
    if ( TIM2 == htim_base->Instance )
    {
        __TIM2_CLK_DISABLE();
    
        /* TIM2 GPIO Configuration    
            PA15 --> TIM2_CH1 
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);
    
        /* Peripheral interrupt DeInit*/
        HAL_NVIC_DisableIRQ(TIM2_IRQn);
    }
    else if( TIM3 == htim_base->Instance )
    {
        /* Peripheral clock disable */
        __TIM3_CLK_DISABLE();
    
        /* TIM3 GPIO Configuration    
            PA6 --> TIM3_CH1
            PA7 --> TIM3_CH2 
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6|GPIO_PIN_7);
    
        /* Peripheral interrupt DeInit*/
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
    }
    else if ( TIM4 == htim_base->Instance )
    {
        /* Peripheral clock disable */
        __TIM4_CLK_DISABLE();
    
        /* TIM4 GPIO Configuration
            PB6 --> TIM4_CH1
            PB7 --> TIM4_CH2 
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);
    
        /* Peripheral interrupt DeInit*/
        HAL_NVIC_DisableIRQ(TIM4_IRQn);
    }
    else if ( TIM5 == htim_base->Instance )
    {
        /* Peripheral clock disable */
        __TIM5_CLK_DISABLE();
    
        /* Peripheral interrupt DeInit*/
        HAL_NVIC_DisableIRQ(TIM5_IRQn);
    }
    else if ( TIM6 == htim_base->Instance )
    {
        /* Peripheral clock disable */
        __TIM6_CLK_DISABLE();
    
        /* Peripheral interrupt DeInit*/
        HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
    }
    else if ( TIM10 == htim_base->Instance )
    {
        /* Peripheral clock disable */
        __TIM10_CLK_DISABLE();
    
        /* Peripheral interrupt DeInit*/
        HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
    }
    else if ( TIM11 == htim_base->Instance )
    {
        /* Peripheral clock disable */
        __TIM11_CLK_DISABLE();
    
        /* Peripheral interrupt DeInit*/
        HAL_NVIC_DisableIRQ(TIM1_TRG_COM_TIM11_IRQn);
    }
}


/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
    GPIO_InitTypeDef  GPIO_InitStruct;
    
    if ( huart->Instance == ARMPIT_USART )
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        ARMPIT_USART_TX_GPIO_CLK_ENABLE();
        ARMPIT_USART_RX_GPIO_CLK_ENABLE();
        __GPIOD_CLK_ENABLE();
        /* Enable USART2 clock */
        ARMPIT_USART_CLK_ENABLE(); 
        
        /*##-2- Configure peripheral GPIO ##########################################*/  
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin       = ARMPIT_USART_TX_PIN;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
        GPIO_InitStruct.Alternate = ARMPIT_USART_TX_AF;
        
        HAL_GPIO_Init(ARMPIT_USART_TX_GPIO_PORT, &GPIO_InitStruct);
        
        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = ARMPIT_USART_RX_PIN;
        GPIO_InitStruct.Alternate = ARMPIT_USART_RX_AF;
            
        HAL_GPIO_Init(ARMPIT_USART_RX_GPIO_PORT, &GPIO_InitStruct);
        
        /*Configure GPIO pin : PB2 */
        GPIO_InitStruct.Pin = ARMPIT_ENABLE_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Alternate = 0x00;
        HAL_GPIO_Init(ARMPIT_ENABLE_PORT, &GPIO_InitStruct);
            
        /*##-3- Configure the NVIC for UART ########################################*/
        /* NVIC for USART1 */
        HAL_NVIC_SetPriority(ARMPIT_USART_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(ARMPIT_USART_IRQn);
    }
    else if ( huart->Instance == PIXARM_USART )
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        PIXARM_USART_TX_GPIO_CLK_ENABLE();
        PIXARM_USART_RX_GPIO_CLK_ENABLE();
        /* Enable PIXARM_USART clock */
        PIXARM_USART_CLK_ENABLE(); 
        
        /*##-2- Configure peripheral GPIO ##########################################*/  
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin       = PIXARM_USART_TX_PIN;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = PIXARM_USART_TX_AF;
        
        HAL_GPIO_Init(PIXARM_USART_TX_GPIO_PORT, &GPIO_InitStruct);
    
        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = PIXARM_USART_RX_PIN;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = PIXARM_USART_RX_AF;
            
        HAL_GPIO_Init(PIXARM_USART_RX_GPIO_PORT, &GPIO_InitStruct);
            
        /*##-3- Configure the NVIC for UART ########################################*/
        /* NVIC for USART1 */
        HAL_NVIC_SetPriority(PIXARM_USART_IRQn, 0, 2);
        HAL_NVIC_EnableIRQ(PIXARM_USART_IRQn);
    }
}


/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if ( huart->Instance == ARMPIT_USART )
    {
        /*##-1- Reset peripherals ##################################################*/
        ARMPIT_USART_FORCE_RESET();
        ARMPIT_USART_RELEASE_RESET();
        
        /*##-2- Disable peripherals and GPIO Clocks #################################*/
        /* Configure UART Tx as alternate function  */
        HAL_GPIO_DeInit(ARMPIT_USART_TX_GPIO_PORT, ARMPIT_USART_TX_PIN);
        /* Configure UART Rx as alternate function  */
        HAL_GPIO_DeInit(ARMPIT_USART_RX_GPIO_PORT, ARMPIT_USART_RX_PIN);
        HAL_GPIO_DeInit(ARMPIT_ENABLE_PORT, ARMPIT_ENABLE_PIN);
        
        /*##-3- Disable the NVIC for UART ##########################################*/
        HAL_NVIC_DisableIRQ(ARMPIT_USART_IRQn);
    }
    else if ( huart->Instance == PIXARM_USART )
    {
        /*##-1- Reset peripherals ##################################################*/
        PIXARM_USART_FORCE_RESET();
        PIXARM_USART_RELEASE_RESET();
        
        /*##-2- Disable peripherals and GPIO Clocks #################################*/
        /* Configure UART Tx as alternate function  */
        HAL_GPIO_DeInit(PIXARM_USART_TX_GPIO_PORT, PIXARM_USART_TX_PIN);
        /* Configure UART Rx as alternate function  */
        HAL_GPIO_DeInit(PIXARM_USART_RX_GPIO_PORT, PIXARM_USART_RX_PIN);
        
        /*##-3- Disable the NVIC for UART ##########################################*/
        HAL_NVIC_DisableIRQ(PIXARM_USART_IRQn);
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
