#include "network.h"
#include "osi.h"


network_controller_t nc;

// Application specific status/error codes
typedef enum _network_state_t {
	NW_ERROR,
	RESET,
	STANDBY,
	AP_INIT_MODE,
	AP_DEVICE_CONNECTED,
	WS_INIT_MODE,
	WS_CONNECTED,
	CLEAR_PROFILES
} network_state_t;

void network_task_entry(void *pvParameters){
	network_state_t state;
	OsiMsgQ_t neq;
	nc.al_queues = (al_queues_t *) pvParameters;
	long retval;
	al_msg_t msg;

	neq = nc.al_queues->network_queue;
	
	state = RESET;
	retval = 0;

	//let all other threads settle before starting network initialization

	while (1){
		

		switch (state) {
			case (NW_ERROR):
				NETWORK_PRINT("network task in NW_ERROR state...");
				retval = osi_MsgQRead(nc.al_queues->network_queue, &msg, 1000);
				if (retval == OSI_OPERATION_FAILED) {
					NETWORK_PRINT("Error second timeout...");
				}
				else {
					if (msg.event_type == NETWORK_EVENT_RESTART){
						state = RESET;
					}
				}
				break;
			case (RESET):
				//Initialize the network
				NETWORK_PRINT ("network task RESET\n\r");
				nc.status = 0;
				nc.ip = 0;
				nc.ping_packet_receive = 0;
				nc.gateway_ip = 0;

				//Reset the network controller
				initialize_network_controller();

				state	=	STANDBY;
				break;
			case (CLEAR_PROFILES):
				configure_simplelink_to_default_state();
				state = RESET;
				break;
			case (STANDBY):
				NETWORK_PRINT ("network task in STANDBY\n\r");
				if (profiles_available()){
					//if there is a profile setup workstation mode
					NETWORK_PRINT("Profiles found\n\r");
					state = WS_INIT_MODE;
				}
				else {
					//if there is no profile then we need to setup an access point
					NETWORK_PRINT("No pre-existing profiles found\n\r");
					state = AP_INIT_MODE;
				}
				break;
			case (AP_INIT_MODE):
				NETWORK_PRINT ("Initiate an AP Mode\n\r");
				retval = setup_wlan_ap_mode(NULL);
				if (retval == -1) {
					master_event(MASTER_EVENT_NETWORK_ERROR_AP_CONNECT,
											 0,
											 NULL,
											 10);
					state = NW_ERROR;
				}
				NETWORK_PRINT ("AP Mode setup, wait for device to connect...\r\n");
				state = AP_DEVICE_CONNECTED;
				break;
			case (AP_DEVICE_CONNECTED):
				/* When Device connects to the AP Present a webpage */
				NETWORK_PRINT ("Waiting for a device to connect...\n\r");

				//if the previous event requires us to wait for an event, start it
				retval = osi_MsgQRead(&neq, &msg, OSI_WAIT_FOREVER);
				if (retval == OSI_OPERATION_FAILED) {
					NETWORK_PRINT("Error during network queue read...\n\r");
					state = NW_ERROR;
				}
				else {
					NETWORK_PRINT("Event occured with AP Connected: Event %s\n\r", msg.event_type);
/* XXX: Present a webpage to the user */


				}

				break;
			case (WS_INIT_MODE):
				//NETWORK_PRINT("Initialize Workstation!\n\r");
				osi_Sleep(20000);
				break;
			case (WS_CONNECTED):
				NETWORK_PRINT("Workstation Connected!\n\r");
				break;
			default:
				break;
		}
	}
}
