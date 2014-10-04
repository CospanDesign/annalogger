#include "launchpad_sensor.h"
#include "uart_if.h"
#include "i2c_if.h"
#include "bma222_sensor.h"
#include "sensor.h"

void launchpad_sensor_init(void){
	long lRetVal = -1;

}

void launchpad_probe_sensor(sensor_t * sensor, int device_index){
	long retval = -1;
	retval = 0;
	switch (device_index){
		case (0):
			Report ("Open BMA222 Accellerometer...");
			new_bma222_sensor(sensor, device_index);
			if (retval < 0){
				Report("Failed\n\r");
				while(1);
			}
			Report("Success!\n\r");
			break;
		case (1):
Report("Setup temperature sensor!\r\n");
			Report ("Open TMP006 Temperature sensor...");
			if (retval < 0){
				Report("Failed\n\r");
				while(1);
			}
			Report("Success!\n\r");
			break;
		case (2):
			break;
		default:
			break;
	}

}
