#include "uart_task.h"
#include <sensor_task.h>
#include "common.h"
#include <string.h>

#ifdef DEBUG_UART
#define UART_PRINT Report
#else
#define UART_PRINT(x,...)
#endif

void uart_task_entry(void *pvParameters){

  OsiMsgQ_t uq;
  al_msg_t  msg;
  int i;
  char * data = NULL;
  long data_size = 0;
  int num_sensors;

  al_queues_t * al_queues;
  al_queues = (al_queues_t*) pvParameters;
  uq = al_queues->uart_queue;
  long retval;

  UART_PRINT ("%s: Entered\n\r", __func__);
  while (1) {
    retval = osi_MsgQRead(&uq, &msg, OSI_WAIT_FOREVER);
    //UART_PRINT("uart task: detected event!\r\n");
    switch (msg.event_type){
      case (SENSOR_EVENT_NEW_DATA):
        data_size = get_max_sensor_data_size();
        //UART_PRINT("uart task: data size: %d\r\n", data_size);
        if (strlen(data) < data_size){
          //UART_PRINT("%s: Free previous data\r\n", __func__);
          free(data);
          data = (char *) calloc(data_size, sizeof(char));
        }
        //UART_PRINT("%s: Reading number of sensors\r\n", __func__);
        sensor_get_num_sensors(&num_sensors);
        for (i = 0; i < num_sensors; i++){
          sensor_get_sensor_data(i, data);
          //UART_PRINT("%s: Sensor data from sensor: %d\r\n%s\r\n", __func__, i, data);
          UART_PRINT(".");
        }
        break;
      default:
        //UART_PRINT("%s: Unknown Event: %d\r\n", __func__, msg.event_type);
        break;
    }
  }
}

