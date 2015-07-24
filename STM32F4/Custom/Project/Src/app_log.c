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
    ITM->TER |= (1UL << 1);
    // This is a Work around. The tracer seems to drop part of
    // the first message sent. Thus, we will allow a space to
    // be dropped in this dummy init function.
    APP_Log(" ");
}

#ifdef DEBUG
__STATIC_INLINE uint32_t ITM_SendCharX (uint32_t Channel, uint32_t Data)
{
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) &&      /* ITM enabled */
        (ITM->TER & (1UL << Channel) ) )        /* ITM Port X enabled */
    {
        while (ITM->PORT[Channel].u32 == 0);
        ITM->PORT[Channel].u8 = (uint8_t) Data;
    }
    return (Data);
}

int fputc(int c, FILE *stream)
{
    return ITM_SendCharX(1, c);
}
#endif
