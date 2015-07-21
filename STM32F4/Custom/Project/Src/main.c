/**
  ******************************************************************************
  * @file    Src/main.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "app_navigation.h"
#include "app_pixarm.h"
#include "app_armpit.h"
#include "app_ir.h"
#include "app_ultrasonic_adapter.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static sAPP_NAVIGATION_CBLK AppNavigationCblk = {
    {FALSE, 0x0000, 0x0000, 0x0000, ROTATION_UNKNOWN},
    {{FALSE}, {0x0000}},
    {0x00},
    {IDLE_INTENSITY, IDLE_INTENSITY, IDLE_INTENSITY, 0x0000},
    {FALSE, ROTATION_INCOMPLETE}
};


/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    eAPP_STATUS status;
    
    /* STM32F4xx HAL library initialization:
        - Configure the Flash prefetch, Flash preread and Buffer caches
        - Systick timer is configured by default as source of time base, but user 
                can eventually implement his proper time base source (a general purpose 
                timer for example or other time source), keeping in mind that Time base 
                duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
                handled in milliseconds basis.
        - Low Level Initialization
    */

    HAL_Init();

    /* Configure LED3, LED4, LED5 and LED6 */
    BSP_LED_Init(LED3);
    BSP_LED_Init(LED4);
    BSP_LED_Init(LED5);
    BSP_LED_Init(LED6);

    /* Configure the system clock to 168 MHz */
    SystemClock_Config();

    /* Init the Logger */
    APP_Log_Init();

    /* Initialize the Communication Components */
    APP_PIXARM_Init();
    //APP_ARMPIT_Init();

    /* Initialize the Sensory Components */
    //APP_IR_Init();
    //APP_HC_SR04_Init();

    /* Log Configuration finished */
    APP_Log("Finished Component Configuration.\r\n");
    
    // Start PIXARM Interrupt Process: Synchronous Initiation Process
    status = APP_PIXARM_Initiate();
    if ( STATUS_FAILURE == status )
    {
        APP_Log("PIXARM Handshake failed. Transitioning to Error.\r\n");
        BSP_LED_On(BSP_PIXARM_ERROR_LED);
        Error_Handler();
    }

    // Start ARMPIT Interrupt Process: Asynchronous Initiation Process
    //status = APP_ARMPIT_Initiate();
    //if ( STATUS_FAILURE == status )
    //{
    //    APP_Log("ARMPIT Handshake initiation failed. Transitioning to Error.\r\n");
    //    BSP_LED_On(BSP_ARMPIT_ERROR_LED);
    //    Error_Handler();
    //}

    /* Log Initation Finished */
    APP_Log("Finished Component Initiation.\r\n");
    
    /* Infinite loop */
    while (1)
    {
        // Process ARMPIT Requests
        //status = APP_ARMPIT_Process_Message(&AppNavigationCblk);
        
        // Save State of IR Sensor
        //AppNavigationCblk.ir_data.guide_within_sight = APP_Scan_IR();
        
        // Run Ultrasonics Driver
        //status = APP_HC_SR04_Pulse_Sensors(&AppNavigationCblk);
        
        // Make any calculations
        status = APP_Navigation_Compute(&AppNavigationCblk);
        
        // Process PIXARM Requests
        status = APP_PIXARM_Process_Message(&AppNavigationCblk);
    }
}

/******************************************************************************/
/*                 System Configuration and Handlers                          */
/******************************************************************************/
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    
    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();
    
    /* The voltage scaling allows optimizing the power consumption when the device is 
        clocked below the maximum system frequency, to update the voltage scaling value 
        regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
    
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
        clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
    
    /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
    if (HAL_GetREVID() == 0x1001)
    {
        /* Enable the Flash prefetch */
        __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
    }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
