
#include "bma222_sensor.h"
#include "bma222drv.h"
#include <string.h>
#include <stdio.h>

#define SENSOR_TYPE 5
#define SENSOR_NAME "BMA222\0"

const char bma222_config[] = ""			\
"{\r\n"															\
"	\"name\":\"bma222\",\r\n"					\
"	\"type\":\"accellerometer\",\r\n" \
"	\"x map\":\"1=.1\",\r\n"					\
"	\"x unit\":\"15mg\",\r\n"					\
"	\"x max\":\"127\",\r\n"						\
"	\"x min\":\"-128\",\r\n"					\
"	\"y map\":\"1=.1\",\r\n"					\
"	\"y unit\":\"15mg\",\r\n"					\
"	\"y max\":\"127\",\r\n"						\
"	\"y min\":\"-128\",\r\n"					\
"	\"z map\":\"1=.1\",\r\n"					\
"	\"z unit\":\"15mg\",\r\n"					\
"	\"z max\":\"127\",\r\n"						\
"	\"z min\":\"-128\"\r\n"						\
"}";


const char bma222_read_data[] = ""	\
"{\r\n"															\
"	\"x\":\"%d\",\r\n"								\
"	\"y\":\"%d\",\r\n"								\
"	\"z\":\"%d\"\r\n"									\
"}";

char bma222_data[sizeof(bma222_read_data) + 10];


void new_bma222_sensor(sensor_t *sensor, uint8_t address){
	new_sensor(sensor, SENSOR_TYPE, address);
	sensor->data_len = strlen(bma222_read_data) + 10;
	sensor->data = calloc(sensor->data_len, sizeof(char));
	bma222_read_sensor_config(sensor);
	BMA222Open();
}
void delete_bma222_sensor(sensor_t *sensor){
	BMA222Close();
}

void bma222_read_sensor_config(sensor_t *sensor){
	sensor->config = bma222_config;
}
void bma222_write_sensor_data(sensor_t *sensor, const char * data){
}

void bma222_read_sensor_data(sensor_t *sensor, const char * data){
	signed char x;
	signed char y;
	signed char z;

	BMA222ReadNew(&x, &y, &z);
	snprintf(sensor->data, sensor->data_len, bma222_read_data, x, y, z);
}



