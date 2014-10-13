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


#define SENSOR_TASK_ENABLE_EVENT           1
#define SENSOR_TASK_DISABLE_EVENT          2
#define SENSOR_TASK_IS_ENABLED_EVENT       3
#define SENSOR_TASK_SET_PARAMETERS         4
#define SENSOR_TASK_UPDATE_RATE            5
#define SENSOR_TASK_GET_SENSOR_CONFIG      6
#define SENSOR_TASK_GET_SENSOR_CONFIG_LEN  7

typedef enum _sensor_state_t {
  ERROR,
  RESET,
  STANDBY,
  ACTIVE
}sensor_state_t;

typedef struct _sc_t {
  int num_devices;
  //Only 3 subscribers: (SD Card, UART and Network);
  bool uart_subscriber;
  bool network_subscriber;
  bool sd_subscriber;
	int max_data_size;

#ifdef USE_LAUNCHPAD
  sensor_t sensors[LAUNCHPAD_SENSOR_COUNT];
#else
  sensor_t sensors[ANNALOGGER_SENSOR_COUNT];
#endif

  
  sensor_state_t STATE;
  OsiMsgQ_t sq;
	OsiLockObj_t lock;
	
  
} sc_t;
typedef struct _sensor_param_t {
  int dev_index;
  char * message;
} sensor_param_t;

sc_t sc;

void sensor_init(void);
void sensor_probe_all(void);
int  sensor_getconfig_length(int device_index);
const char * sensor_get_config(int device_index);
void read_all_sensors(void);

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
	retval = osi_LockObjCreate(&sc.lock);
	if (retval != 0){
  	ERR_PRINT(retval);
	}

  //Setup the sensor controller
  sc.STATE = RESET;
  sc.num_devices;
  sc.sq = al_queues->sensor_queue;
  sc.uart_subscriber = false;
  sc.network_subscriber = false;
  sc.sd_subscriber = false;
	sc.max_data_size = 1;

  setup_sensor_list(&sc.sensors[0], SENSOR_COUNT);

  SENSOR_PRINT ("%s: Entereting main loop\n\r", __func__);

  while (1) {
    switch (sc.STATE) {
      case (RESET):
        poll = false;
        sensor_init();
        sensor_probe_all();
        SENSOR_PRINT ("sensor task (RESET) initialized sensor going into standby\n\r");
        sc.STATE = STANDBY;
        timeout_value = SENSOR_TASK_TIMEOUT;
        
        //send a message to the master saying we are ready
        retval = master_event(MASTER_EVENT_SENSOR_IS_READY, 0, NULL, 10);
        break;
      case (STANDBY):
        SENSOR_PRINT ("sensor task (STANDBY) still alive\n\r");
        retval = osi_MsgQRead(&sq, &msg, OSI_WAIT_FOREVER);
        SENSOR_PRINT ("sensor task (STANDBY) Received event!\n\r");
        if (retval != 0) {
          ERR_PRINT(retval);
        }
        break;
      case (ACTIVE):
        SENSOR_PRINT ("sensor task (ACTIVE) still alive\n\r");
        retval = osi_MsgQRead(&sq, &msg, SENSOR_TASK_TIMEOUT);
        SENSOR_PRINT ("sensor task (ACTIVE) Received event! (retval: %d)\n\r", retval);
        
        if (retval == OSI_OPERATION_FAILED) {
          SENSOR_PRINT("%s: Timeout condition occured\r\n", __func__);
          poll = true;
        }
        else if (retval != 0) {
          ERR_PRINT(retval);
        }
        break;
      default:
        //Shouldn't have gotten here, go back to standby
        sc.STATE = STANDBY;
        break;
    }







    //Process an event
    if (poll){
      //In active state need to read from the sensors
      //Reset the poll
      poll = false;
      SENSOR_PRINT("%s: Processing 'Poll Event'\r\n", __func__);
			//Update all sensor data
			read_all_sensors();

			if (!sc.uart_subscriber &&
					!sc.network_subscriber &&
					!sc.sd_subscriber){
					SENSOR_PRINT("%s: No sensor subscribers setup\r\n", __func__);
			}

			if (sc.network_subscriber){
				retval = network_event(	SENSOR_EVENT_NEW_DATA,
																0,
																NULL,
																OSI_WAIT_FOREVER);
			}
			if (sc.sd_subscriber){
				retval = sd_event(			SENSOR_EVENT_NEW_DATA, 
																0, 
																NULL,
																OSI_WAIT_FOREVER);
      }
			if (sc.uart_subscriber){
				retval = uart_event(		SENSOR_EVENT_NEW_DATA,
																0,
																NULL,
																OSI_WAIT_FOREVER);
			}
      continue;

		}




    switch(msg.event_type){
      case(SENSOR_TASK_ENABLE_EVENT):           
				SENSOR_PRINT("sensor enable\r\n");
        sc.STATE = ACTIVE;
        break;
      case(SENSOR_TASK_DISABLE_EVENT):
				SENSOR_PRINT("sensor disable\r\n");
        sc.STATE = STANDBY;
        break;
      case(SENSOR_TASK_IS_ENABLED_EVENT):
				SENSOR_PRINT("sensor is enabled?\r\n");
        master_msg.event_type = MASTER_EVENT_SENSOR_IS_ENABLED;
        master_msg.msg_param = 1;
        master_msg.msg = (void *) ((sc.STATE == ACTIVE) ? 1 : 0);
        retval = osi_MsgQWrite(meq, (void *) &master_msg, 10);    
        if (retval != 0){
          SENSOR_PRINT("%s: ERROR TIMEOUT WHILE WAITING FOR MASTER EVENT QUEUE (retval = %d)",
                        __func__,
                        retval);
        }
        break;
      case(SENSOR_TASK_SET_PARAMETERS):
				SENSOR_PRINT("sensor set parameters\r\n");
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
				SENSOR_PRINT("sensor update rate\r\n");
        timeout_value = (int) msg.msg;
        if (timeout_value < SENSOR_MIN_TASK_TIMEOUT){
          timeout_value = SENSOR_MIN_TASK_TIMEOUT;
        }
        break;
      case(SENSOR_TASK_GET_SENSOR_CONFIG):
				SENSOR_PRINT("sensor get sensor config\r\n");
        break;
      case(SENSOR_TASK_GET_SENSOR_CONFIG_LEN):
				SENSOR_PRINT("sensor get config length\r\n");
        break;
      default:
        break;
    }
  }
}


long sensor_enable(){
  long retval;
  SENSOR_PRINT("%s Enable sensors\r\n", __func__);
  al_msg_t msg = {
    .event_type = SENSOR_TASK_ENABLE_EVENT,
    .msg_param    =  0,
    .msg        =  NULL
  };

  retval = osi_MsgQWrite(&sc.sq, (void *) &msg, 10);
  SENSOR_PRINT("%s Sent Message, retval: %d\r\n", __func__, retval);
  return retval;
  
}
long sensor_disable(){
  al_msg_t msg = {
    .event_type = SENSOR_TASK_DISABLE_EVENT,
    .msg_param    =  0,
    .msg        =  NULL
  };
  return osi_MsgQWrite(&sc.sq, (void *) &msg, 10);
}
long sensor_is_enabled(){
  al_msg_t msg = {
    .event_type = SENSOR_TASK_IS_ENABLED_EVENT,
    .msg_param    =  0,
    .msg        =  NULL
  };
  return osi_MsgQWrite(&sc.sq, (void *) &msg, 10);
}
void sensor_enable_uart_subscriber(bool enable){
  sc.uart_subscriber = enable;
}
void sensor_enable_sd_subscriber(bool enable){
  sc.sd_subscriber = enable;
}
void sensor_enable_network_subscriber(bool enable){
  sc.network_subscriber = enable;
}
long sensor_set_update_rate(int device, int update_rate_ms){
  al_msg_t msg = {
    .event_type = SENSOR_TASK_UPDATE_RATE,
    .msg_param    = 1,
    .msg        =  NULL
  };
  msg.msg_param = device;
  msg.msg = ((void *) update_rate_ms);
  return osi_MsgQWrite(&sc.sq, (void *) &msg, 10);

}
const char * sensor_get_sensor_config(int device){
  sensor_t s = sc.sensors[device];
  if (!s.exists){
    return NULL;
  }
  return get_sensor_config(&s);
}
long sensor_get_sensor_config_len(int device){
  sensor_t s = sc.sensors[device];
  if (!s.exists){
    return 0;
  }
  return strlen(get_sensor_config(&s));
}
long sensor_set_sensor_parameters(int device, const char * json_config){
  char * ljson_config;
  sensor_param_t * sp;

  ljson_config = NULL;
  sp = NULL;

  al_msg_t msg = {
    .event_type = SENSOR_TASK_GET_SENSOR_CONFIG_LEN,
    .msg_param  = 1,
    .msg        =  NULL
  };
  //XXX: NOTE USER MUST DELETE THE JSON CONFIG STRING!!!
  

  msg.msg_param = strlen(json_config);
  ljson_config = (char *) calloc(msg.msg_param, sizeof(char));
  strcpy(ljson_config, json_config);

  sp = (sensor_param_t *) calloc(1, sizeof(sensor_param_t));
  sp->dev_index = device;
  sp->message = ljson_config;
  
  msg.msg = (void *) sp;
  return osi_MsgQWrite(&sc.sq, (void *) &msg, 10);
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

  sc.num_devices = 0;
  for (i = 0; i < SENSOR_COUNT; i++){
#ifdef USE_LAUNCHPAD
    launchpad_probe_sensor(&sc.sensors[i], i);
#else
  //probe analogger board sensors
  SENSOR_PRINT ("Error %s: Need to probe sensors on annaloggerr\n", __func__);
#endif

    if (sc.sensors[i].exists){
      sc.num_devices++;
    }
  }
}
int  sensor_get_config_length(int device_index){
  return -1;
}
const char * sensor_get_config(int device_index){
  return NULL;
}


long sensor_get_num_sensors(int *num_sensors){
  *num_sensors = sc.num_devices;
  return 0;
}


/* read_all_sensors
 *
 * Read all the sensors data
 */
void read_all_sensors(void){
	int i = 0;
	long len = 0;
  sensor_t s;
	osi_LockObjLock(&sc.lock, OSI_WAIT_FOREVER);

	sc.max_data_size = 1;
	
	for (i = 0; i < sc.num_devices; i++){
		s = sc.sensors[i];
		//SENSOR_PRINT("Reading device: %d\r\n", i);
		read_sensor_data(&s, s.data);
		//SENSOR_PRINT("device data:\r\n%s\r\n", s.data);
		len = strlen(s.data);
		if (sc.max_data_size < len){
			sc.max_data_size = len;
		}
	}
	osi_LockObjUnlock(&sc.lock);
}

long get_max_sensor_data_size(void){
	return sc.max_data_size;
}

/* sensor_get_sensor_data
 *
 * Copies the sensor data string to the 'data' pointer
 * passed in
 */

long sensor_get_sensor_data(int device, char * data){
  sensor_t s = sc.sensors[device];
  if (!s.exists){
    return -1;
  }
 
	osi_LockObjLock(&sc.lock, OSI_WAIT_FOREVER);
  memcpy(data, s.data, strlen(s.data));
	osi_LockObjUnlock(&sc.lock);
	return 0;
}



