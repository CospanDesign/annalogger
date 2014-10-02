
#include "sd_task.h"

void sd_task_entry(void *pvParameters){

	while (1){
#ifdef ANNA_VERBOSE
		Report ("sd task alive\n\r");
#endif
		osi_Sleep(20000);
	}
}
