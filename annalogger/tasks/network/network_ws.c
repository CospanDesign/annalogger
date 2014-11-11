#include "network.h"

extern network_controller_t nc;

long setup_wlan_ws_mode(void){
  long retval = 0;
  retval = sl_Start(NULL,NULL,NULL);
  ASSERT_ON_ERROR(retval);

  retval = sl_WlanSetMode(ROLE_STA);
  ASSERT_ON_ERROR(retval);

  //Automatically connect to an AP
  retval = sl_WlanPolicySet(SL_POLICY_CONNECTION, 
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
 
  retval = sl_Stop(0xFF);
  ASSERT_ON_ERROR(retval);
  retval = sl_Start(NULL, NULL, NULL);
  ASSERT_ON_ERROR(retval);

  /* Stop Internal Server */
//XXX: NOTE THIS MUST BE DONE FOR AP SERVER AS WELL (THIS SHOULD PROBABLY BE DONE IN network.c)
  retval = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
  ASSERT_ON_ERROR(retval);

  http_event( HTTP_EVENT_START_SERVER,
              0,
              NULL,
              10);

  NETWORK_PRINT("Workstation: Start Server!\r\n");
  nc.mode = ROLE_STA;
  return retval;
}
