/**
  ******************************************************************************
  * @file    Src/app_log.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application Logging Implementation
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "app_common.h"


void APP_Log_Init()
{
    // This is a Work around. The tracer seems to drop part of
    // the first message sent. Thus, we will allow a space to
    // be dropped in this dummy init function.
    APP_Log(" ");
}

int fputc(int c, FILE *stream)
{
    return ITM_SendChar(c);
}
