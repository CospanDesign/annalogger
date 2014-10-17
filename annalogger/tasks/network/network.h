#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "network_task.h"
#include "HttpCore.h"
#include "HttpRequest.h"
#include "WebSockHandler.h"
#include "ws_list.h"

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

#define WS_CONN_COUNT 8

//Controller
typedef enum _connection_status_t {
	LAN_CONNECTION_FAILED 			= -0x7D0,
	CLIENT_CONNECTION_FAILED 		= LAN_CONNECTION_FAILED - 1,
	DEVICE_NOT_IN_STATION_MODE 	= CLIENT_CONNECTION_FAILED - 1,

	STATUS_CODE_MAX 						= -0xBB8
} connection_status_t;

typedef struct _network_controller_t {

  unsigned char   ssid[SSID_LEN_MAX + 1];
  unsigned char   bssid[BSSID_LEN_MAX];

	unsigned char  	status;

  bool            ipv6;
	unsigned char  	ip[6];
	unsigned char 	gip[6];
  unsigned char   dns[6];

	unsigned long  	ping_packet_receive;
	al_queues_t * 	al_queues;
  wsl_head_t  *   ws_list;
  
} network_controller_t;

typedef struct _ws_conn_t {
  uint32_t        keep_alive;
}ws_conn_t;

//Network

long configure_simplelink_to_default_state();
void initialize_network_controller();
void ping_report(SlPingReport_t *ping_report);
int ping_test(unsigned long ip);
int get_ssid_name(char *ssid_name, unsigned int max_len);
bool profiles_available();
long get_my_ip(unsigned long *ip,
               unsigned long *subnetmask,
               unsigned long *default_gateway,
               unsigned long *dns_server);

//Network AP
//int configure_ap_mode(int mode);
long setup_wlan_ap_mode(void *params);
int device_connected_ap(void);

//Network Workstation
long setup_wlan_ws_mode(void);

#endif
