/* Includes ------------------------------------------------------------------*/
#include "main.h"


void Test_Log_Init()
{
    // This is a Work around. The tracer seems to drop part of
    // the first message sent. Thus, we will allow a space to
    // be dropped in this dummy init function.
    Test_Log(" ");
}

int fputc(int c, FILE *stream)
{
    return ITM_SendChar(c);
}