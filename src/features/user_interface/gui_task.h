#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#define UI_TASK_STACK_WORDS   1024
#define UI_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define UI_QUEUE_DEPTH        32     // Deep enough to absorb bursts; UiMsg_t is small
void TouchCallBack(uint32_t ui32Message, int32_t i32X, int32_t i32Y);