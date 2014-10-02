#include "launchpad_sensor.h"
#include "uart_if.h"
#include "i2c_if.h"
#include "tmp006drv.h"
#include "bma222drv.h"

void launchpad_sensor_init(){
	long lRetVal = -1;

	Report ("Open BMA222 Accellerometer...");
  lRetVal = BMA222Open();
	if (lRetVal < 0){
		Report("Failed\n\r");
		while(1);
	}
	Report("Success!\n\r");

	Report ("Open TMP006 Temperature sensor...");
	if (lRetVal < 0){
		Report("Failed\n\r");
		while(1);
	}
	Report("Success!\n\r");
}
