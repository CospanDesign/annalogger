#ifndef __UART_TASK_H__
#define __UART_TASK_H__

#include <stdlib.h>
#include "annalogger.h"
#include "osi.h"
#include "uart_if.h"

void uart_task_entry(void *pvParameters);

#endif
