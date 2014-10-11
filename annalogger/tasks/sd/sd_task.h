#ifndef __SD_TASK__
#define __SD_TASK__

#include <stdlib.h>
#include "annalogger.h"
#include "osi.h"
#include "uart_if.h"

#ifdef DEBUG_SD
#define SD_PRINT Report
#else
#define SD_PRINT(x,...)
#endif


void sd_task_entry(void *pvParameters);

#endif
