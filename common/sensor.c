/*
 * LTC2448.c
 *
 * Created: 8/28/2014 12:25:55 PM
 *  Author: cospan
 */ 

#include "sensor.h"
#include <malloc.h>
#include <string.h>

void new_sensor(sensor_t * sensor, uint8_t type, uint8_t address){
	//Local Function Declaration
	//Create some space for a sensor
	//Set up the sensor data structure with all the constants
	sensor->exists = true;
	sensor->type = type;
	sensor->address = address;

	sensor->write_sensor_data_fp = NULL;
	sensor->read_sensor_data_fp = NULL;
	sensor->data_len = 0;
	sensor->data = NULL;
}
void delete_sensor(sensor_t *sensor){
	sensor->exists = false;
};

//Concrete Fuunctions
uint8_t get_sensor_type(sensor_t *sensor){
	return sensor->type;
}
uint8_t get_sensor_address(sensor_t *sensor){
	return sensor->address;
}
const char * get_sensor_config(sensor_t *sensor){
	return sensor->config;
}
int get_sensor_config_len(sensor_t *sensor){
	if (sensor->config == NULL){
		return 0;
	}
	return strlen(sensor->config);
}

void write_sensor_data(sensor_t *sensor, const char * data){
	if (sensor->write_sensor_data_fp != NULL){
		sensor->write_sensor_data_fp(sensor, data);
	}
}
void read_sensor_data(sensor_t *sensor, const char * data){
	if (sensor->read_sensor_data_fp != NULL){
		sensor->read_sensor_data_fp(sensor, data);
	}
}
void read_sensor_config(sensor_t *sensor){
	if (sensor->read_sensor_config_fp != NULL){
		sensor->read_sensor_config_fp(sensor);	
	}
}


//Setup Functions
void setup_sensor_list(sensor_t *sensors, int count){
	int i;
	for (i = 0; i < count; i++){
		sensors[i].exists = false;
		sensors[i].type = 0;
		sensors[i].address = 0;
		sensors[i].channel_count = 0;
		sensors[i].config = NULL;
		sensors[i].data = NULL;

		sensors[i].write_sensor_data_fp = NULL;
		sensors[i].read_sensor_data_fp = NULL;
		sensors[i].read_sensor_config_fp = NULL;
	}
}

