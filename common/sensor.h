#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>


typedef struct _sensor_t sensor_t;

typedef void (*write_sensor_data_fp_t)(sensor_t *, const char *);
typedef void (*read_sensor_data_fp_t)(sensor_t *, const char *);
typedef void (*read_sensor_config_fp_t)(sensor_t *);

struct _sensor_t{
	bool exists;
	uint8_t type;
	uint8_t address;
	uint8_t channel_count;
	const char * config;
		
	int	data_len;
	void * data;
	
	write_sensor_data_fp_t write_sensor_data_fp;
	read_sensor_data_fp_t read_sensor_data_fp;
	read_sensor_config_fp_t read_sensor_config_fp;
};

//These functions must be used to initialize a new sensor
void new_sensor(sensor_t * sensor, uint8_t type, uint8_t address);
void delete_sensor(sensor_t *sensor);

//These functions should be replaced by your sensor
void write_sensor_data(sensor_t *sensor, const char *);
void read_sensor_data(sensor_t *sensor, const char *);
void read_sensor_config(sensor_t *sensor);

//Concrete functions
uint8_t get_sensor_type(sensor_t *sensor);
uint8_t get_sensor_address(sensor_t *sensor);

const char * get_sensor_config(sensor_t *sensor);
int get_sensor_config_len(sensor_t *sensor);

//Setup functions
void setup_sensor_list(sensor_t *sensors, int count);

#endif

