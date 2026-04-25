#define UI_TASK_STACK_WORDS   1024
#define UI_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define UI_QUEUE_DEPTH        32     // Deep enough to absorb bursts; UiMsg_t is small
