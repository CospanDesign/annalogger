#ifndef __LAUNCHPAD_SENSOR_H__
#define __LAUNCHPAD_SENSOR_H__

#include "sensor.h"

#define LAUNCHPAD_SENSOR_COUNT 3

void launchpad_sensor_init(void);
void launchpad_probe_sensor(sensor_t * sensor, int device_index);

#endif
