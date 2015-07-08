/**
  ******************************************************************************
  * @file    Src/stm32f4xx_it.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "app_common.h"
#include "app_pixarm.h"
#include "app_armpit.h"
#include "app_hc_sr04.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* UART handler declared in "app_pixarm.c" file */
extern UART_HandleTypeDef PixarmHandle;

/* I2C handler declared in "app_armpit.c" file */
extern UART_HandleTypeDef ArmpitHandle;

/* Timer handler declared in "app_hrs04.c" file */
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    HAL_IncTick();
    //HAL_SYSTICK_IRQHandler();
}


/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/
/**
  * @brief  This function handles UART interrupt request.  
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream 
  *         used for USART data transmission     
  */
void ARMPIT_USART_IRQHandler(void)
{
    HAL_UART_IRQHandler(& ArmpitHandle);
}


/**
  * @brief  This function handles UART interrupt request.  
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream 
  *         used for USART data transmission     
  */
void PIXARM_USART_IRQHandler(void)
{
    HAL_UART_IRQHandler(& PixarmHandle);
}


/**
* @brief This function handles EXTI Line0 interrupt.
*/
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET); //set trig = 1
    HAL_TIM_Base_Start_IT(&htim2); //start 10us counter
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/**
* @brief This function handles EXTI Line1 interrupt.
*/
void EXTI1_IRQHandler(void)
{
    // If rising edge detected on PA1 connected to Echo pin
    // Start timer 3 in input capture mode with interrupt
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

/**
* @brief This function handles EXTI Line3 interrupt.
*/
void EXTI3_IRQHandler(void)
{
    // If rising edge detected on PA7 connected to Echo pin
    // Start timer 4 in input capture mode with interrupt
    HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

/**
* @brief This function handles TIM2 global interrupt.
*/
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/**
* @brief This function handles TIM3 global interrupt.
*/
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

/**
* @brief This function handles TIM4 global interrupt.
*/
void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}


/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
