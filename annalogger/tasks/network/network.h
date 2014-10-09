#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "network_task.h"

//
// Values for below macros shall be modified for setting the 'Ping' properties
//

#define PING_INTERVAL            1000    /* In msecs */
#define PING_TIMEOUT             3000    /* In msecs */
#define PING_PKT_SIZE            20      /* In bytes */
#define NO_OF_ATTEMPTS           3
#define PING_FLAG                0

#define ANNALOGGER_PROFILE_COUNT	7

#ifdef DEBUG_NETWORK
#define NETWORK_PRINT Report
#else
#define NETWORK_PRINT(x,...)
#endif

//Controller
typedef enum _connection_status_t {
	LAN_CONNECTION_FAILED 			= -0x7D0,
	CLIENT_CONNECTION_FAILED 		= LAN_CONNECTION_FAILED - 1,
	DEVICE_NOT_IN_STATION_MODE 	= CLIENT_CONNECTION_FAILED - 1,

	STATUS_CODE_MAX 						= -0xBB8
} connection_status_t;

typedef struct _network_controller_t {
	unsigned char  	status;
	unsigned long  	ip;
	unsigned long  	ping_packet_receive;
	unsigned long 	gateway_ip;
	al_queues_t * 	al_queues;
} network_controller_t;

//Network

long configure_simplelink_to_default_state();
void initialize_network_controller();
void ping_report(SlPingReport_t *ping_report);
int ping_test(unsigned long ip);
int get_ssid_name(char *ssid_name, unsigned int max_len);
bool profiles_available();

//Network AP
//int configure_ap_mode(int mode);
int setup_wlan_ap_mode(void *params);
int device_connected_ap(void);

//Network Workstation


#endif
