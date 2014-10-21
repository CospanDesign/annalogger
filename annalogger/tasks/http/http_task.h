#ifndef __HTTP_TASK_H__
#define __HTTP_TASK_H__

#include <stdlib.h>
#include "HttpCore.h"
#include "HttpRequest.h"
#include "annalogger.h"
#include "simplelink.h"
#include "osi.h"
#include "uart_if.h"
#include "common.h"

#ifdef DEBUG_HTTP
#define HTTP_PRINT Report
#else
#define HTTP_PRINT(x,...)
#endif

typedef struct _http_controller_t {
	al_queues_t * 	al_queues;
} http_controller_t;

void http_task_entry(void *pvParameters);

#endif
