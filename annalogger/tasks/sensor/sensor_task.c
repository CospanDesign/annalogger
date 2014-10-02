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


#define SENSOR_TASK_ENABLE_EVENT 					1
#define SENSOR_TASK_DISABLE_EVENT 				2
#define SENSOR_TASK_IS_ENABLED_EVENT			3
#define SENSOR_TASK_SET_PARAMETERS				4
#define SENSOR_TASK_UPDATE_RATE						5
#define SENSOR_TASK_ADD_SUBSCRIBER				6
#define SENSOR_TASK_REMOVE_SUBSCRIBER	  	7
#define SENSOR_TASK_GET_SENSOR_CONFIG	   	8
#define SENSOR_TASK_GET_SENSOR_CONFIG_LEN	9


void sensor_init(void);

typedef enum _sensor_state_t {
	STANDBY,
	ACTIVE
}sensor_state_t;

typedef struct _sensor_controller_t {
	sensor_state_t STATE;
	uint16_t num_devices;
	OsiMsgQ_t sq;
} sensor_controller_t;

sensor_controller_t sensor_controller;


void sensor_task_entry(void *pvParameters){
	//Local prototypes
	long retval;
	OsiMsgQ_t meq;
	OsiMsgQ_t sq;
	al_queues_t * al_queues;
	al_msg_t msg;

	al_queues = (al_queues_t*) pvParameters;
	meq = al_queues->master_event_queue;
	sq = al_queues->sensor_queue;
	sensor_controller.sq = al_queues->sensor_queue;

	sensor_init();
	sensor_controller.STATE = STANDBY;
	SENSOR_PRINT ("%s: Entereting main loop\n\r", __func__);

	
	while (1) {
		switch (sensor_controller.STATE) {
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
				}
				else if (retval != 0) {
					ERR_PRINT(retval);
				}


		}
	}
}


long sensor_enable(){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_ENABLE_EVENT,
		.msg_len		=	0,
		.msg				=	NULL
	};
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}
long sensor_disable(){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_DISABLE_EVENT,
		.msg_len		=	0,
		.msg				=	NULL
	};
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}
long sensor_is_enabled(){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_IS_ENABLED_EVENT,
		.msg_len		=	0,
		.msg				=	NULL
	};
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}
long sensor_add_subscriber(OsiMsgQ_t* queue){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_ADD_SUBSCRIBER,
		.msg_len		= 1,
		.msg				=	NULL
	};
	msg.msg = (void *)queue;
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);
}
long sensor_remove_subscriber(OsiMsgQ_t* queue){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_REMOVE_SUBSCRIBER,
		.msg_len		= 1,
		.msg				=	NULL
	};
	msg.msg = (void *)queue;
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);

}
long sensor_set_update_rate(uint8_t device, uint16_t update_rate_ms){
	al_msg_t msg = {
		.event_type = SENSOR_TASK_UPDATE_RATE,
		.msg_len		= 1,
		.msg				=	NULL
	};
	msg.msg = (void *)update_rate_ms;
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);

}
long sensor_get_sensor_config(uint8_t device, const char * json_config){
SENSOR_TASK_GET_SENSOR_CONFIG	   
	al_msg_t msg = {
		.event_type = SENSOR_TASK_GET_SENSOR_CONFIG,
		.msg_len		= 1,
		.msg				=	NULL
	};
	msg.msg = (void *)update_rate_ms;
	return osi_MsgQWrite(sensor_controller.sq, (void *) &msg, 10);

}
long sensor_get_sensor_config_len(uint8_t device){
SENSOR_TASK_GET_SENSOR_CONFIG_LEN
}
long sensor_set_sensor_parameters(uint8_t device, const char * json_config){
SENSOR_TASK_SET_PARAMETERS			 
}


void sensor_init(void){

#ifdef USE_LAUNCHPAD
	launchpad_sensor_init();
#else
	//Initialize analogger board
	SENSOR_PRINT ("Error %s: Need to initialize sensor task\r\n", __func__);
#endif
}


