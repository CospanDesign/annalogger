#ifndef __SENSOR_TASK__
#define __SENSOR_TASK__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "annalogger.h"
#include "osi.h"
#include "uart_if.h"
#include "common.h"

//10 second timeout for debug purposes
#define SENSOR_TASK_TIMEOUT 10000

/*
 * sensor task API:
 *
 * 	Note that most functions do not return parameters because the API is a
 *	wrapper to a inter process queue, the master thread will receive an update
 *	on it's event queue when a funciton has finished executing along with the
 *	return code
 *
 */ 

void sensor_task_entry(void *pvParameters);

/* sensor_enable
 * 
 * Enable the sensor updates
 *
 * Return
 *	Nothing
 *
 */
long sensor_enable();

/* sensor_disable
 * 
 * Disable sensor updates
 *
 * Return
 *	OsiReturnVal_e
 *
 */
long sensor_disable();

/* sensor_is_enabled
 * 
 * returns true if sensor thread is enabled
 *
 * Return
 *	OsiReturnVal_e
 */
long sensor_is_enabled();

/* sensor_add_subscriber
 * 
 * Add a sensor subscriber in the form of a Queue to update
 *
 * Return
 *	OsiReturnVal_e
 *
 */
long sensor_add_subscriber(OsiMsgQ_t* queue);

/* sensor_remove_subscriber
 * 
 * Params
 *	queue (OsiMsgQ_t): a pointer to a queue to send to
 *
 * Remove a sensor subscriber
 *
 * Return
 *	OsiReturnVal_e
 *
 */
long sensor_remove_subscriber(OsiMsgQ_t* queue);

/* sensor_set_update_rate
 * 
 * Params
 *	queue (OsiMsgQ_t): a pointer to a queue to send to
 * 
 * Set the update rate of the particular device
 *
 * Return
 *	OsiReturnVal_e
 *
 */
long sensor_set_update_rate(uint8_t device, uint16_t update_rate_ms);

/* sensor_get_sensor_config
 * 
 * Params
 *	device (uint8_t): index to the sensor to get the config data
 *	json_config (const char *): index to the sensor to get the config data
 * 
 * Reads the sensor configuration data
 *
 * Return
 *	OsiReturnVal_e
 *
 */
long sensor_get_sensor_config(uint8_t device, const char * json_config);

/* sensor_get_sensor_config_len
 * 
 * Params
 *	device (uint8_t): index to the sensor to get the config data
 * 
 * Reads the sensor configuration data length so users can malloc a string
 *
 * Return
 *	OsiReturnVal_e
 *
 */
long sensor_get_sensor_config_len(uint8_t device);

/* sensor_set_sensor_parameters
 * 
 * Params
 *	device (uint8_t): index to the sensor to get the config data
 * 
 * sends a JSON strong to be used to set the parameters for a specific sensor
 *	used for regulators, update rates etc...
 *
 * Return
 *	OsiReturnVal_e
 *
 */
long sensor_set_sensor_parameters(uint8_t device, const char * json_config);


#endif
