#include <stdint.h>
#include <stdbool.h>

#include "simplelink.h"
#include "launchpad.h"
#include "device.h"
#include "utils.h"

#include "pinmux.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "pin.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"

#include "gpio.h"
#include "i2c.h"

#include "i2c_if.h"
#include "uart_if.h"
#include "gpio_if.h"

void launchpad_init(void){
  long lRetVal = -1;
	PinConfigSet(PIN_58, PIN_STRENGTH_2MA | PIN_STRENGTH_4MA, PIN_TYPE_STD_PD);
  GPIO_IF_LedConfigure(LED1|LED2|LED3);
  GPIO_IF_LedOff(MCU_ALL_LED_IND);
  MAP_UtilsDelay(8000000);
  GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);


	Report ("Initialize I2C...");
  //
  // I2C Init
  //
  lRetVal = I2C_IF_Open(I2C_MASTER_MODE_FST);
  if(lRetVal < 0)
  {
		Report ("Fail!\r\n");
    while(1);
  }    
	Report ("Success\r\n");


}
