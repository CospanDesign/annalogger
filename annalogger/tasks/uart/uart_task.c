

#include "uart_task.h"

void uart_task_entry(void *pvParameters){

	while (1) {
#ifdef ANNA_VERBOSE
		Report ("uart task stil alive\n\r");
#endif
		osi_Sleep(15000);
	}
}


