#include "network.h"
#include "device.h"
#include <stdio.h>

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

long setup_wlan_ap_mode(void *params)
{
  //char ssid_name[33] = AP_SSID_NAME;
  al_msg_t msg;
  char ssid_name[33] = "Annalogger\0";
  unsigned short len = 32;
  unsigned short config_opt = WLAN_AP_OPT_SSID;
  long retval = -1;

  uint8_t mac_address_value[SL_MAC_ADDR_LEN];
  uint8_t mac_address_len = SL_MAC_ADDR_LEN;



  //
  // Asumption is that the device is configured in station mode already
  // and it is in its default state
  //

  retval = sl_Start(NULL,NULL,NULL);

  if (retval < 0)
  {
    NETWORK_PRINT("Failed to start the device \n\r");
    return retval;
    LOOP_FOREVER();
  }

  NETWORK_PRINT("Device started as STATION need to reconfigure as an AP\n\r");
  NETWORK_PRINT("Network Status: 0x%08X\r\n", nc.status);
  
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
  nc.mode = ROLE_AP;

  sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &mac_address_len, (_u8 *)mac_address_value);
  snprintf(ssid_name, 32, "%s: %02X.%02X.%02X.%02X.%02X.%02X\0", AP_SSID_NAME,
                mac_address_value[0],
                mac_address_value[1],
                mac_address_value[2],
                mac_address_value[3],
                mac_address_value[4],
                mac_address_value[5]);


  NETWORK_PRINT("SSID: %s\r\n", ssid_name);

  //Set the SSID
  retval = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID, strlen(ssid_name),
                        (unsigned char*)ssid_name);


  ASSERT_ON_ERROR(retval);
  config_opt = WLAN_AP_OPT_SSID;

  retval = sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt , &len, (unsigned char*) ssid_name);
  NETWORK_PRINT("Current AP Name: %s\r\n", ssid_name);
  ASSERT_ON_ERROR(retval);

  //Restart CC3200
  retval = sl_Stop(0xFF);
  ASSERT_ON_ERROR(retval);

  retval = sl_Start(0, 0, 0);
  ASSERT_ON_ERROR(retval);

  if(retval != ROLE_AP){
    retval = sl_WlanSetMode(ROLE_AP);
    nc.mode = ROLE_AP;

    ASSERT_ON_ERROR(retval);
    retval = sl_Stop(0xFF);
    ASSERT_ON_ERROR(retval);

    retval = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(retval);
  }


  if (retval == ROLE_AP){
    //Wait for IP
    NETWORK_PRINT("Waiting for AP to give me an IP...");
    NETWORK_PRINT("Network Status: 0x%08X\r\n", nc.status);
    while(!IS_IP_ACQUIRED(nc.status)){
      NETWORK_PRINT(".");
      osi_Sleep(10);
    }
    NETWORK_PRINT("Network Status: 0x%08X\r\n", nc.status);
    NETWORK_PRINT("IP Acquired\r\n");
  }


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

  //test_result = ping_test(nc.ip);
  if(test_result < 0)
  {
      NETWORK_PRINT("Ping to client failed \n\r");
  }

  UNUSED(ucDHCP);
  UNUSED(test_result);

  // revert to STA mode
  retval = sl_WlanSetMode(ROLE_STA);
  nc.mode = ROLE_STA;
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
