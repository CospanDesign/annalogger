#include "network.h"

extern network_controller_t nc;

/*
int configure_ap_mode(int mode)
{
	char ssid_name[33] = AP_SSID_NAME;
	long retval = -1;

  retval = sl_WlanSetMode(ROLE_AP);
  ASSERT_ON_ERROR(retval);

  retval = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID, strlen(ssid_name),
                          (unsigned char*)ssid_name);
  //ASSERT_ON_ERROR(retval);

  NETWORK_PRINT("Device is configured in AP mode\n\r");

  // Restart Network processor
  //retval = sl_Stop(SL_STOP_TIMEOUT);

  // reset status bits
  //CLR_STATUS_BIT_ALL(nc.status);

  //return sl_Start(NULL,NULL,NULL);
	return retval;

}
*/

int setup_wlan_ap_mode(void *params)
{
	//char ssid_name[33] = AP_SSID_NAME;
	al_msg_t msg;
	char ssid_name[33] = "Annalogger\0";
	unsigned short len = 32;
	unsigned short config_opt = WLAN_AP_OPT_SSID;
  long retval = -1;
  //
  // Asumption is that the device is configured in station mode already
  // and it is in its default state
  //
  retval = sl_Start(NULL,NULL,NULL);

  if (retval < 0)
  {
    NETWORK_PRINT("Failed to start the device \n\r");
    LOOP_FOREVER();
  }

  NETWORK_PRINT("Device started as STATION need to reconfigure as an AP\n\r");
  
  //
  // Configure the networking mode and ssid name(for AP mode)
  //
  if(retval != ROLE_AP){
  	retval = sl_WlanSetMode(ROLE_AP);
  	ASSERT_ON_ERROR(retval);
		retval = sl_Stop(0xFF);
  	ASSERT_ON_ERROR(retval);
		retval = sl_Start(0, 0, 0);
  	ASSERT_ON_ERROR(retval);
			
	}
  if (retval == ROLE_AP){
		//Wait for IP
		NETWORK_PRINT("Waiting for AP to give me an IP...");
    while(!IS_IP_ACQUIRED(nc.status)){
			NETWORK_PRINT(".");
			osi_Sleep(10);
		}
		NETWORK_PRINT("IP Acquired\r\n");
	}

	//get_ssid_name(ssid_name, 32);
 	retval = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID, strlen(AP_SSID_NAME),
                          (unsigned char*)AP_SSID_NAME);

  ASSERT_ON_ERROR(retval);
	config_opt = WLAN_AP_OPT_SSID;
  retval = sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt , &len, (unsigned char*) ssid_name);
	NETWORK_PRINT("Current AP Name: %s\r\n", ssid_name);



 

	return 0;
}


int device_connected_ap(void){
  int test_result = 0;
  unsigned char ucDHCP;
	long retval;
  unsigned char len = sizeof(SlNetCfgIpV4Args_t);

  SlNetCfgIpV4Args_t ipV4 = {0};

  // get network configuration
  retval = sl_NetCfgGet(SL_IPV4_AP_P2P_GO_GET_INFO, &ucDHCP, &len,
                          (unsigned char *)&ipV4);
  if (retval < 0)
  {
      NETWORK_PRINT("Failed to get network configuration \n\r");
      LOOP_FOREVER();
  }
  
  NETWORK_PRINT("Connect a client to Device\n\r");
  while(!IS_IP_LEASED(nc.status))
  {
    //wating for the client to connect
  }
  NETWORK_PRINT("Client is connected to Device\n\r");

  test_result = ping_test(nc.ip);
  if(test_result < 0)
  {
      NETWORK_PRINT("Ping to client failed \n\r");
  }

  UNUSED(ucDHCP);
  UNUSED(test_result);

  // revert to STA mode
  retval = sl_WlanSetMode(ROLE_STA);
  if(retval < 0)
  {
    ERR_PRINT(retval);
    LOOP_FOREVER();
  }

  // Switch off Network processor
  retval = sl_Stop(SL_STOP_TIMEOUT);
  
  NETWORK_PRINT("Application exits\n\r");
  while(1);

}
