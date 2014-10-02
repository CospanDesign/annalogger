#include "master_task.h"

#include <common.h>
#include <uart_if.h>
#include "rom.h"
#include "rom_map.h"



void master_task_entry(void *pvParameters){

	Report ("In master task\n\r");
	Report ("I'm alive\n\r");
	while (1){
#ifdef ANN_VERBOSE
		Report ("still alive\n\r");
#endif
		osi_Sleep(10000);
	};
}


