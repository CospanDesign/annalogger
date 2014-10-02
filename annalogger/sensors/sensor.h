#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdint.h>
#include <malloc.h>


typedef struct _sensor_t sensor_t;

typedef void (*update_all_sensor_data_fp_t)(sensor_t *);
typedef void (*update_sensor_data_fp_t)(sensor_t *, uint8_t);
typedef void (*get_sensor_data_fp_t)(sensor_t *, uint8_t, void *);
typedef void (*get_all_sensor_data_fp_t)(sensor_t *, void *);
typedef void (*set_sensor_data_fp_t)(sensor_t *, uint8_t, void *);
typedef void (*set_all_sensor_data_fp_t)(sensor_t *, void *);


struct _sensor_t{
	uint8_t type;
	uint8_t address;
	uint8_t channel_count;
		
	void * data;
	
	update_all_sensor_data_fp_t update_all_sensor_data_fp;
	update_sensor_data_fp_t update_sensor_data_fp;
	get_sensor_data_fp_t get_sensor_data_fp;
	get_all_sensor_data_fp_t get_all_sensor_data_fp;
	set_sensor_data_fp_t set_sensor_data_fp;
	set_all_sensor_data_fp_t set_all_sensor_data_fp;

};





//void (*update_sensor_data) (sensor_t *, int);

//These functions must be used to initialize a new sensor
sensor_t * new_sensor(uint8_t type, uint8_t address);
void delete_sensor(sensor_t *sensor);


//These functions should be replaced by your sensor
void update_all_sensor_data(sensor_t *sensor);
void update_sensor_data(sensor_t *sensor, uint8_t channel);
void get_sensor_data(sensor_t * sensor, uint8_t channel, void * data);
void get_all_sensor_data(sensor_t * sensor, void * data);
void set_sensor_data(sensor_t * sensor, uint8_t channel, void * data);
void set_all_sensor_data(sensor_t *sensor, void *data);

//Concrete functions
uint8_t get_max_sensor_channel_count(sensor_t *sensor);
uint8_t get_sensor_type(sensor_t *sensor);
uint8_t get_sensor_address(sensor_t *sensor);

#endif