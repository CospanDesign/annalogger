

#include "http_task.h"


typedef enum _http_state_t {
	HTTP_ERROR,
	RESET,
	STANDBY,
  RUN,
} http_state_t;

http_controller_t hc;

void http_task_entry(void *pvParameters)
{

  http_state_t state;
	OsiMsgQ_t heq;
	hc.al_queues = (al_queues_t *) pvParameters;
  long retval;
	al_msg_t msg;



  retval = 0;
  state = RESET;
	heq = hc.al_queues->http_queue;

  while (1){

    switch(state){
      case (HTTP_ERROR):
        HTTP_PRINT("%s: ERROR!\r\n", __func__);
        break;
      case (RESET):
        HTTP_PRINT("%s: RESET!\r\n", __func__);
        state = STANDBY;
        break;
      case (STANDBY):
        HTTP_PRINT("%s: STANDBY\r\n", __func__);
				retval = osi_MsgQRead(&heq,
                              &msg,
                              OSI_WAIT_FOREVER);
        HTTP_PRINT("Received a message\r\n");
        if (retval == -1) {
					master_event(MASTER_EVENT_HTTP_ERROR,
											 0,
											 NULL,
											 10);
					state = HTTP_ERROR;
				}
        if (msg.event_type == HTTP_EVENT_START_SERVER) {
          state = RUN;
        }
        break;
      case (RUN):
        HTTP_PRINT("%s: Start Server\r\n", __func__);
        HttpServerInitAndRun(NULL);
        state = RESET;
        break;
      default:
        HTTP_PRINT("%s: ILLEGAL STATE, GO TO RESET\r\n", __func__);
        state = RESET;
        break;
    }
  }
}
