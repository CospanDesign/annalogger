#include <stdint.h>
#include <stdbool.h>

#include "master_task.h"

#include <common.h>
#include <uart_if.h>
#include "rom.h"
#include "rom_map.h"


#ifdef DEBUG_MASTER
#define MASTER_PRINT Report
#else
#define MASTER_PRINT(x,...)
#endif

typedef enum _master_state_t {
	RESET,
	READY
}master_state_t;
typedef struct _master_t {
	master_state_t MASTER_STATE;
	bool	sensor_task_ready;
} master_t;

master_t master;

void master_task_entry(void *pvParameters){
	long retval;
	bool poll;

	al_queues_t * al_queues;
	OsiMsgQ_t meq;
	al_msg_t msg;


	al_queues = (al_queues_t*) pvParameters;
	meq = al_queues->master_event_queue;

	master.MASTER_STATE = RESET;
	master.sensor_task_ready = false;

	MASTER_PRINT ("In master task\n\r");
	MASTER_PRINT ("I'm alive\n\r");
	while (1){
		retval = osi_MsgQRead(&meq, &msg, MASTER_TASK_TIMEOUT);
		if (retval == OSI_OPERATION_FAILED) {
#ifdef ANNA_VERBOSE
			MASTER_PRINT ("master task alive\n\r");
			MASTER_PRINT ("exited MsgQRead from timeout\n\r");
#endif
		}
		else {
			MASTER_PRINT ("Master Task: Received event: %d\n\r", msg.event_type);
		}
	};
}


