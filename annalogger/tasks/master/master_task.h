#ifndef __MASTER_TASK_H__
#define __MASTER_TASK_H__

#include <stdlib.h>
#include "annalogger.h"
#include "osi.h"
#include "uart_if.h"
#include "common.h"

#define MASTER_TASK_TIMEOUT 10000
#define DEBUG_MASTER


void master_task_entry(void *pvParameters);


#endif
