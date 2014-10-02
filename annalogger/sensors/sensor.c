/*
 * LTC2448.c
 *
 * Created: 8/28/2014 12:25:55 PM
 *  Author: cospan
 */ 

#include "sensor.h"
#include <malloc.h>

sensor_t * new_sensor(uint8_t type, uint8_t address){
	//Local Function Declaration
	sensor_t * sensor = NULL;

	//Create some space for a sensor
	sensor = (sensor_t*) malloc(sizeof(sensor_t));
	if (sensor == NULL){
		return NULL;
	}
	
	//Set up the sensor data structure with all the constants
	sensor->type = type;
	sensor->address = address;
	
	sensor->update_all_sensor_data_fp = NULL;
	sensor->update_sensor_data_fp = NULL;
	sensor->get_sensor_data_fp = NULL;
	sensor->get_all_sensor_data_fp = NULL;
	sensor->set_sensor_data_fp = NULL;
	sensor->set_all_sensor_data_fp = NULL;
	
	return sensor;
}

void delete_sensor(sensor_t *sensor){
	free(sensor);	
};

//Concrete Fuunctions
uint8_t get_max_sensor_channel_count(sensor_t *sensor){
	return sensor->channel_count;
}
uint8_t get_sensor_type(sensor_t *sensor){
	return sensor->type;
}
uint8_t get_sensor_address(sensor_t *sensor){
	return sensor->address;
}

void update_all_sensor_data(sensor_t *sensor){
	if (sensor->update_all_sensor_data_fp != NULL){
		sensor->update_all_sensor_data_fp(sensor);
	}
}
void update_sensor_data(sensor_t *sensor, uint8_t channel){
	if (sensor->update_sensor_data_fp != NULL){
		sensor->update_sensor_data_fp(sensor, channel);	
	}
}

void get_sensor_data(sensor_t * sensor, uint8_t channel, void * data){
	if (sensor->get_sensor_data_fp != NULL){
		sensor->get_sensor_data_fp(sensor, channel, data);
	}
}
void get_all_sensor_data(sensor_t * sensor, void * data){
	if (sensor->get_all_sensor_data_fp != NULL){
		sensor->get_all_sensor_data_fp(sensor, data);
	}
}
void set_sensor_data(sensor_t * sensor, uint8_t channel, void * data){
	if (sensor->set_sensor_data_fp != NULL){
		sensor->set_sensor_data_fp(sensor, channel, data);
	}
}
void set_all_sensor_data(sensor_t *sensor, void *data){
	if (sensor->set_all_sensor_data_fp != NULL){
		sensor->set_all_sensor_data_fp(sensor, data);
	}
}







