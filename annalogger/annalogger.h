#ifndef __ANNALOGGER_H__
#define __ANNALOGGER_H__

#include <stdint.h>
#include <stdbool.h>
#include "osi.h"

//uncomment to see lots of debug messages come out of the UART
#define ANNA_VERBOSE
#define ANNALOGGER_SENSOR_COUNT 8

#define AP_SSID_NAME "Annalogger"

/* Queue Information */
#define MASTER_QUEUE_SIZE 	32
#define SENSOR_QUEUE_SIZE 	32
#define UART_QUEUE_SIZE			32
#define SD_QUEUE_SIZE				32
#define NETWORK_QUEUE_SIZE	32


typedef struct _al_msg_t {
	uint8_t event_type;
	int32_t msg_param;
	void * msg;
} al_msg_t;

typedef struct _queue_struct_t {
	OsiMsgQ_t master_event_queue;
	OsiMsgQ_t sensor_queue;
	OsiMsgQ_t network_queue;	
	OsiMsgQ_t sd_queue;
	OsiMsgQ_t uart_queue;
} al_queues_t;

/*End Queue Information */



//----- DEBUG FLAG -----

#define DEBUG_SENSOR_READ

#define DEBUG_MASTER
#define DEBUG_SENSOR
#define DEBUG_UART
#define DEBUG_SD
#define DEBUG_NETWORK

//----- DEBUG FLAG END -----


#define MASTER_EVENT_SENSOR_IS_ENABLED					2
#define MASTER_EVENT_SENSOR_GET_CONFIG					3
#define MASTER_EVENT_SENSOR_GET_CONFIG_LEN			4
#define MASTER_EVENT_SENSOR_IS_READY						5
#define MASTER_EVENT_NETWORK_IS_READY						6
#define MASTER_EVENT_NETWORK_EVENT_TIMEOUT			7
#define MASTER_EVENT_NETWORK_ERROR_AP_CONNECT		8
#define MASTER_EVENT_SD_IS_READY								9

#define NETWORK_EVENT_RESTART										32

/* Simple Link Pass Through */
//Event that came from simplelink is passed to msg_param
	/* As an example, if the simple link sends the event (SL_NETAPP_IPV4_IPACQUIRED_EVENT) indicating
	 * that Annalogger is connected then:
	 *	al_msg_t.event_type = SIMPLE_LINK_GENERAL_EVENT
	 *	al_msg_t.msg_param = SL_NETAPP_IPV4_IPACQUIRED_EVENT
	 */


#define SIMPLE_LINK_GENERAL_EVENT								33
#define SIMPLE_LINK_WLAN_EVENT									34
#define SIMPLE_LINK_NETAPP_EVENT								35
#define SIMPLE_LINK_HTTP_SERVER_EVENT						36
#define SIMPLE_LINK_SOCKET_EVENT								37

#define NETWORK_EVENT_SENSOR_DATA								38


/* UART Events */
#define UART_EVENT_SENSOR_DATA									64

/* SD Events */
#define SD_EVENT_SENSOR_DATA										96

/* Sensor Events */
#define SENSOR_EVENT_NEW_DATA										128


//Function Prototypes
OsiReturnVal_e master_event(uint8_t,
														int32_t,
														void *,
														OsiTime_t);

OsiReturnVal_e network_event(uint8_t,
														int32_t,
														void *,
														OsiTime_t);

OsiReturnVal_e uart_event(uint8_t,
													int32_t,
													void *,
													OsiTime_t);

OsiReturnVal_e sd_event(uint8_t,
													int32_t,
													void *,
													OsiTime_t);
	
#endif
