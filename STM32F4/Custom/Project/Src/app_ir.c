/**
  ******************************************************************************
  * @file    Src/app_ir.c 
  * @version V1.0
  * @date    3-July-2015
  * @brief   Application Infrared Implementation
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "app_ir.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void APP_IR_Init(void)
{
    HAL_IR_MspInit();
}


/* Public functions ----------------------------------------------------------*/
GPIO_PinState APP_Scan_IR(void)
{
    GPIO_PinState ir_pin;
    
    // Since the IR Sensor is Active Low, the signal is inverted
    ir_pin = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0);
    return ir_pin;
}


/* -------------------------- End of File ------------------------------------*/
