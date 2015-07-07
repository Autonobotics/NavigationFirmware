/**
  ******************************************************************************
  * @file    readme.txt 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Description of the Navigation Application.
  ******************************************************************************

@par Description

< Insert Description >


@note Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
      based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
      a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.
      
@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.


@par Directory contents 

  - Src/main.c                 Main program
  - Src/system_stm32f4xx.c     STM32F4xx system clock configuration file
  - Src/stm32f4xx_it.c         Interrupt handlers 
  - Src/stm32f4xx_hal_msp.c    HAL MSP module
  - Inc/main.h                 Main program header file  
  - Inc/stm32f4xx_hal_conf.h   HAL Configuration file
  - Inc/stm32f4xx_it.h         Interrupt handlers header file

        
@par Hardware and Software environment  

  - This program runs on STM32F407xx devices.


@par How to use it ? 

In order to make the program work, you must do the following :
 - Open your preferred toolchain 
 - Rebuild all files and load your image into target memory
 - Run the example
 
 */
