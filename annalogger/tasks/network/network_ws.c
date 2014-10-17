#include "network.h"


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

  return retval;
}
