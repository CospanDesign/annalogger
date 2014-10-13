#ifndef __BMA222_SENSOR_H__
#define __BMA222_SENSOR_H__

#include <stdint.h>
#include "sensor.h"

void new_bma222_sensor(sensor_t *sensor, uint8_t address);
void delete_bma222_sensor(sensor_t *sensor);

void bma222_write_sensor_data(sensor_t *sensor, const char * data);
void bma222_read_sensor_data(sensor_t *sensor, char * data);
void bma222_read_sensor_config(sensor_t *sensor);

#endif
