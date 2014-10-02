#ifndef __NETWORK_TASK__
#define __NETWORK_TASK__

#include <stdlib.h>
#include "annalogger.h"
#include "simplelink.h"
#include "osi.h"
#include "uart_if.h"
#include "common.h"


static int PingTest(unsigned long ulIpAddr);
static long ConfigureSimpleLinkToDefaultState();
static void InitializeAppVariables();
static int GetSsidName(char *pcSsidName, unsigned int uiMaxLen);


void network_task_entry(void *pvParameters);

#endif
