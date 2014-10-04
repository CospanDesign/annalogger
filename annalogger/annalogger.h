#ifndef __ANNALOGGER_H__
#define __ANNALOGGER_H__

#include <stdint.h>
#include "osi.h"

//uncomment to see lots of debug messages come out of the UART
#define ANNA_VERBOSE

#define ANNALOGGER_SENSOR_COUNT 8

#define MASTER_QUEUE_SIZE 	32
#define SENSOR_QUEUE_SIZE 	32
#define UART_QUEUE_SIZE			32
#define SD_QUEUE_SIZE				32
#define NETWORK_QUEUE_SIZE	32

typedef struct _al_msg_t {
	uint8_t event_type;
	uint8_t msg_param;
	void * msg;
} al_msg_t;

typedef struct _queue_struct_t {
	OsiMsgQ_t master_event_queue;
	OsiMsgQ_t sensor_queue;
	OsiMsgQ_t network_queue;	
	OsiMsgQ_t sd_queue;
	OsiMsgQ_t uart_queue;
} al_queues_t;

#define DEBUG_MASTER
#define DEBUG_SENSOR
#define DEBUG_UART
#define DEBUG_SD
#define DEBUG_NETWORK

#define MASTER_EVENT_SENSOR_IS_ENABLED 			2
#define MASTER_EVENT_SENSOR_GET_CONFIG 			3
#define MASTER_EVENT_SENSOR_GET_CONFIG_LEN 	4
#define MASTER_EVENT_SENSOR_IS_READY				5

//Function Prototypes
OsiReturnVal_e master_event(uint8_t,
														uint8_t,
														void *,
														OsiTime_t);
	

#endif
