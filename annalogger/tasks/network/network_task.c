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
  WS_CONNECT,
	WS_CONNECTED,
	CLEAR_PROFILES
} network_state_t;

void network_task_entry(void *pvParameters){
	network_state_t state;
	OsiMsgQ_t neq;
	nc.al_queues = (al_queues_t *) pvParameters;
	long retval;
	al_msg_t msg;
  int i;
  

	neq = nc.al_queues->network_queue;
  nc.ws_list = wsl_new("Web Socket Connections");
  msg.event_type = 0;
	
	state = RESET;
	retval = 0;

  setup_dynamic_content();

	//let all other threads settle before starting network initialization

	while (1){
		

    msg.event_type = 0;
    retval = 0;

		switch (state) {
			case (NW_ERROR):
				NETWORK_PRINT("network task in NW_ERROR state...");
				retval = osi_MsgQRead(nc.al_queues->network_queue, &msg, 1000);
				break;
			case (RESET):
				//Initialize the network
				NETWORK_PRINT ("network task RESET\n\r");
				nc.status = 0;
				nc.ping_packet_receive = 0;
        for (i = 0; i < 6; i++) {
				  nc.ip[i] = 0;
				  nc.gip[i] = 0;
        }
        memset(nc.ssid, 0, sizeof(nc.ssid));
        memset(nc.bssid, 0, sizeof(nc.bssid));

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
          CLR_STATUS_BIT(nc.status, STATUS_BIT_CONNECTION);
          CLR_STATUS_BIT(nc.status, STATUS_BIT_IP_ACQUIRED);
					state = WS_INIT_MODE;
				}
				else {
					//if there is no profile then we need to setup an access point
					NETWORK_PRINT("No pre-existing profiles found\n\r");
					state = AP_INIT_MODE;
				}
				break;
			case (AP_INIT_MODE):
				NETWORK_PRINT ("AP Mode: Initiatize\n\r");
				retval = setup_wlan_ap_mode(NULL);
				if (retval == -1) {
					master_event(MASTER_EVENT_NETWORK_ERROR_AP_CONNECT,
											 0,
											 NULL,
											 10);
					state = NW_ERROR;
				}
				state = AP_DEVICE_CONNECTED;
				break;
			case (AP_DEVICE_CONNECTED):
				NETWORK_PRINT ("AP Mode: Waiting for a device to connect...\n\r");
				retval = osi_MsgQRead(&neq, &msg, OSI_WAIT_FOREVER);
				break;
			case (WS_INIT_MODE):
				NETWORK_PRINT("Initialize Workstation!\n\r");
        retval = setup_wlan_ws_mode();
        if (retval != 0){
					master_event(MASTER_EVENT_NETWORK_ERROR_WS_CONNECT,
											 0,
											 NULL,
											 10);
					state = NW_ERROR;

        }
        state = WS_CONNECTED;
				break;
			case (WS_CONNECT):
				NETWORK_PRINT("Workstation waiting for IP\n\r");
				retval = osi_MsgQRead(&neq, &msg, OSI_WAIT_FOREVER);
				break;
			case (WS_CONNECTED):
				retval = osi_MsgQRead(&neq, &msg, 10000);
	      if (retval == OSI_OPERATION_FAILED) {
          NETWORK_PRINT("-");
          continue;
        }
        
			default:
				break;
		}

	  if (retval == OSI_OPERATION_FAILED) {
		  NETWORK_PRINT("Error during network queue read...\n\r");
		  state = NW_ERROR;
      continue;
	  }

    //Process an incomming event
    switch (msg.event_type){
      case (NETWORK_EVENT_RESTART):
        NETWORK_PRINT("[NETWORK EVENT] Restart\r\n");
        state = RESET;
        break;
      case (SIMPLE_LINK_GENERAL_EVENT):
        NETWORK_PRINT("[SIMPLELINK GENERAL EVENT]: %d\r\n", msg.msg_param);
        break;
      case (SIMPLE_LINK_WLAN_EVENT):
        NETWORK_PRINT("[SIMPLELINK WLAN EVENT]: %d\r\n", msg.msg_param);
        break;
      case (SIMPLE_LINK_NETAPP_EVENT):
        NETWORK_PRINT("[SIMPLELINK NETAPP EVENT]: %d\r\n", msg.msg_param);
        switch(msg.msg_param){
		      case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
		      case SL_NETAPP_IPV6_IPACQUIRED_EVENT:
            if (state == WS_CONNECT){
              state = WS_CONNECTED;
				      NETWORK_PRINT("Workstation Connected!\n\r");
            }
            break;
          default:
            break;
        }
        break;
      case (SIMPLE_LINK_HTTP_SERVER_EVENT):
        NETWORK_PRINT("[SIMPLELINK HTTP SERVER EVENT]: %d\r\n", msg.msg_param);
        break;
      case (SIMPLE_LINK_SOCKET_EVENT):
        NETWORK_PRINT("[SIMPLELINK SOCKET EVENT]: %d\r\n", msg.msg_param);
      case (0):
        //pass through
        break;
      default:
        break;
    }

	}
}
