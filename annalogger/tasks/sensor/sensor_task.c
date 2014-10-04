#include <string.h>
#include "sensor_task.h"
#include "sensor.h"
#include "common.h"

#ifdef DEBUG_SENSOR
#define SENSOR_PRINT Report
#else
#define SENSOR_PRINT(x,...)
#endif

#ifdef USE_LAUNCHPAD
	#include "launchpad_sensor.h"
#endif

#ifdef USE_LAUNCHPAD
	#define SENSOR_COUNT LAUNCHPAD_SENSOR_COUNT
#else
	#define SENSOR_COUNT ANNALOGGER_SENSOR_COUNT
#endif


#define SENSOR_TASK_ENABLE_EVENT 					1
#define SENSOR_TASK_DISABLE_EVENT 				2
#define SENSOR_TASK_IS_ENABLED_EVENT			3
#define SENSOR_TASK_SET_PARAMETERS				4
#define SENSOR_TASK_UPDATE_RATE						5
#define SENSOR_TASK_GET_SENSOR_CONFIG	   	6
#define SENSOR_TASK_GET_SENSOR_CONFIG_LEN	7

typedef enum _sensor_state_t {
	RESET,
	STANDBY,
	ACTIVE
}sensor_state_t;

typedef struct _sensor_controller_t {
	int num_devices;
	//Only 3 subscribers: (SD Card, UART and Network);
	bool uart_subscriber;
	bool network_subscriber;
	bool sd_subscriber;

#ifdef USE_LAUNCHPAD
	sensor_t sensors[LAUNCHPAD_SENSOR_COUNT];
#else
	sensor_t sensors[ANNALOGGER_SENSOR_COUNT];
#endif

	
	sensor_state_t STATE;
	OsiMsgQ_t sq;
	
} sensor_controller_t;
typedef struct _sensor_param_t {
	int dev_index;
	char * message;
} sensor_param_t;

sensor_controller_t sensor_controller;

void sensor_init(void);
void sensor_probe_all(void);
int	sensor_getconfig_length(int device_index);
const char * sensor_get_config(int device_index);

void sensor_task_entry(void *pvParameters){
	//Local prototypes
	long retval;
	int timeout_value = SENSOR_TASK_TIMEOUT;
	bool poll;

	OsiMsgQ_t meq;
	OsiMsgQ_t sq;
	al_queues_t * al_queues;
	al_msg_t msg;
	al_msg_t master_msg;
	sensor_param_t *sp;


	poll = false;
	al_queues = (al_queues_t*) pvParameters;
	sp = NULL;
	meq = al_queues->master_event_queue;
	sq = al_queues->sensor_queue;

	//Setup the sensor controller
	sensor_controller.STATE = RESET;
	sensor_controller.num_devices;
	sensor_controller.sq = al_queues->sensor_queue;
	sensor_controller.uart_subscriber = false;
	sensor_controller.network_subscriber = false;
	sensor_controller.sd_subscriber = false;

	setup_sensor_list(&sensor_controller.sensors[0], SENSOR_COUNT);

	SENSOR_PRINT ("%s: Entereting main loop\n\r", __func__);

	
	while (1) {
		switch (sensor_controller.STATE) {
			case (RESET):
				poll = false;
				sensor_init();
				sensor_probe_all();
				SENSOR_PRINT ("sensor task (RESET) initialized sensor going into standby\n\r");
				sensor_controller.STATE = STANDBY;
				timeout_value = SENSOR_TASK_TIMEOUT;
				
				//send a message to the master saying we are ready
				retval = master_event(MASTER_EVENT_SENSOR_IS_READY, 0, NULL, 10);
				break;
			case (STANDBY):
				SENSOR_PRINT ("sensor task (STANDBY) still alive\n\r");
				retval = osi_MsgQRead(&sq, &msg, OSI_WAIT_FOREVER);
				if (retval != 0) {
					ERR_PRINT(retval);
				}
				break;
			case (ACTIVE):
				SENSOR_PRINT ("sensor task (ACTIVE) still alive\n\r");
				retval = osi_MsgQRead(&sq, &msg, SENSOR_TASK_TIMEOUT);
				break;
				if (retval == OSI_OPERATION_FAILED) {
					SENSOR_PRINT("%s: Timeout condition occured\r\n", __func__);
					poll = true;
				}
				else if (retval != 0) {
					ERR_PRINT(retval);
				}
			default:
				//Shouldn't have gotten here, go back to reset
				sensor_controller.STATE = STANDBY;
				break;
		}

		//Process an event
		if (poll){
			//In active state need to read from the sensors
			SENSOR_PRINT("%s: Processing 'Poll Event'\r\n", __func__);
			//Reset the poll
			poll = false;
			continue;
		}

		switch(msg.event_type){
			case(SENSOR_TASK_ENABLE_EVENT): 				 
				sensor_controller.STATE = ACTIVE;
				break;
			case(SENSOR_TASK_DISABLE_EVENT):
				sensor_controller.STATE = STANDBY;
				break;
			case(SENSOR_TASK_IS_ENABLED_EVENT):
				master_msg.event_type = MASTER_EVENT_SENSOR_IS_ENABLED;
				master_msg.msg_param = 1;
				master_msg.msg = (void *) ((sensor_controller.STATE == ACTIVE) ? 1 : 0);
				retval = osi_MsgQWrite(meq, (void *) &master_msg, 10);		
				if (retval != 0){
					SENSOR_PRINT("%s: ERROR TIMEOUT WHILE WAITING FOR MASTER EVENT QUEUE (retval = %d)",
												__func__,
												retval);
				}
				break;
			case(SENSOR_TASK_SET_PARAMETERS):
				sp = (sensor_param_t *) msg.msg;
				//send the message out to the sensor specified by the dev_index in sp
				(void) sp->dev_index;
				//message to send
				(void) sp->message;
//XXX: Send a message to the device
				free((char *) sp->message);	
				free(sp);
				break;
			case(SENSOR_TASK_UPDATE_RATE):
				timeout_value = (int) msg.msg;
				if (timeout_value < SENSOR_MIN_TASK_TIMEOUT){
					timeout_value = SENSOR_MIN_TASK_TIMEOUT;
				}
				break;
			case(SENSOR_TASK_GET_SENSOR_CONFIG):
				break;
			case(SENSOR_TASK_GET_SENSOR_CONFIG_LEN):
				break;
			default:
				break;
		}
	}
}

long sensor_enable(){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_ENABLE_EVENT,
		.msg_param		=	0,
		.msg				=	NULL
	};
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}
long sensor_disable(){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_DISABLE_EVENT,
		.msg_param		=	0,
		.msg				=	NULL
	};
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}
long sensor_is_enabled(){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_IS_ENABLED_EVENT,
		.msg_param		=	0,
		.msg				=	NULL
	};
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}
void sensor_enable_uart_subscriber(bool enable){
	sensor_controller.uart_subscriber = enable;
}
void sensor_enable_sd_subscriber(bool enable){
	sensor_controller.sd_subscriber = enable;
}
void sensor_enable_network_subscriber(bool enable){
	sensor_controller.network_subscriber = enable;
}
long sensor_set_update_rate(int device, int update_rate_ms){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_UPDATE_RATE,
		.msg_param		= 1,
		.msg				=	NULL
	};
	msg.msg_param = device;
	msg.msg = ((void *) update_rate_ms);
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);

}
long sensor_get_sensor_config(int device){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_GET_SENSOR_CONFIG,
		.msg_param		= 1,
		.msg				=	NULL
	};
	msg.msg_param = device;
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}
long sensor_get_sensor_config_len(int device){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_GET_SENSOR_CONFIG_LEN,
		.msg_param		= 1,
		.msg				=	NULL
	};
	msg.msg_param = device;
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}
long sensor_set_sensor_parameters(int device, const char * json_config){
	char * ljson_config;
	sensor_param_t * sp;

	ljson_config = NULL;
	sp = NULL;

	al_msg_t msg = {
		.event_type = SENSOR_TASK_GET_SENSOR_CONFIG_LEN,
		.msg_param	= 1,
		.msg				=	NULL
	};
	//XXX: NOTE USER MUST DELETE THE JSON CONFIG STRING!!!
	

	msg.msg_param = strlen(json_config);
	ljson_config = (char *) calloc(msg.msg_param, sizeof(char));
	strcpy(ljson_config, json_config);

	sp = (sensor_param_t *) calloc(1, sizeof(sensor_param_t));
	sp->dev_index = device;
	sp->message = ljson_config;
	
	msg.msg = (void *) sp;
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}

//hardware interface
void sensor_init(void){

#ifdef USE_LAUNCHPAD
	launchpad_sensor_init();
#else
	//Initialize analogger board
	SENSOR_PRINT ("Error %s: Need to initialize sensor task\r\n", __func__);
#endif
}
/* sensor probe
 *
 * can be called multiple times and will initialize the sensor
 * controller sensor list
 */
void sensor_probe_all(void){
	int i;

	sensor_controller.num_devices = 0;
	for (i = 0; i < SENSOR_COUNT; i++){
#ifdef USE_LAUNCHPAD
		launchpad_probe_sensor(&sensor_controller.sensors[i], i);
#else
	//probe analogger board sensors
	SENSOR_PRINT ("Error %s: Need to probe sensors on annaloggerr\n", __func__);
#endif

		if (sensor_controller.sensors[i].exists){
			sensor_controller.num_devices++;
		}
	}
}
int	sensor_get_config_length(int device_index){
	return -1;
}
const char * sensor_get_config(int device_index){
	return NULL;
}



