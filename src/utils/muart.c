/* Standard includes. */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "uartstdio.h"

SemaphoreHandle_t uartMutex;

#include <stdarg.h>

void MUARTprintf(const char *s, ...)
{
    va_list args;

    xSemaphoreTake(uartMutex, portMAX_DELAY);

    va_start(args, s);
    UARTvprintf(s, args);   // NOTE: use UARTvprintf, not UARTprintf
    va_end(args);

    xSemaphoreGive(uartMutex);
}