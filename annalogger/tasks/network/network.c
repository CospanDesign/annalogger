#include "network.h"

extern network_controller_t nc;

/* Events */

void SimpleLinkWlanEventHandler(SlWlanEvent_t *pSlWlanEvent)
{
  slWlanConnectAsyncResponse_t*  pEventData = NULL;
	NETWORK_PRINT("%s Entered\r\n", __func__);

  switch(pSlWlanEvent->Event)
  {
    case SL_WLAN_CONNECT_EVENT:
      SET_STATUS_BIT(nc.status, STATUS_BIT_CONNECTION);

      //
      // Information about the connected AP (like name, MAC etc) will be
      // available in 'slWlanConnectAsyncResponse_t'-Applications
      // can use it if required
      //
      //  slWlanConnectAsyncResponse_t *pEventData = NULL;
      // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
      //
      //
    	break;

    case SL_WLAN_DISCONNECT_EVENT:

      CLR_STATUS_BIT(nc.status, STATUS_BIT_CONNECTION);
      CLR_STATUS_BIT(nc.status, STATUS_BIT_IP_AQUIRED);

      pEventData = &pSlWlanEvent->EventData.STAandP2PModeDisconnected;

      // If the user has initiated 'Disconnect' request,
      //'reason_code' is SL_USER_INITIATED_DISCONNECTION
      if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code){
     		NETWORK_PRINT("Device disconnected from the AP on application's "
                      "request \n\r");
      }
      else{
          NETWORK_PRINT("Device disconnected from the AP on an ERROR..!! \n\r");
      }
    	break;

    case SL_WLAN_STA_CONNECTED_EVENT:
      // when device is in AP mode and any client connects to device cc3xxx
      SET_STATUS_BIT(nc.status, STATUS_BIT_CONNECTION);

      //
      // Information about the connected client (like SSID, MAC etc) will be
      // available in 'slPeerInfoAsyncResponse_t' - Applications
      // can use it if required
      //
      // slPeerInfoAsyncResponse_t *pEventData = NULL;
      // pEventData = &pSlWlanEvent->EventData.APModeStaConnected;
      //
    	break;

    case SL_WLAN_STA_DISCONNECTED_EVENT:
      // when client disconnects from device (AP)
      CLR_STATUS_BIT(nc.status, STATUS_BIT_CONNECTION);
      CLR_STATUS_BIT(nc.status, STATUS_BIT_IP_LEASED);

      //
      // Information about the connected client (like SSID, MAC etc) will
      // be available in 'slPeerInfoAsyncResponse_t' - Applications
      // can use it if required
      //
      // slPeerInfoAsyncResponse_t *pEventData = NULL;
      // pEventData = &pSlWlanEvent->EventData.APModestaDisconnected;
      //            
    	break;

    default:
      NETWORK_PRINT("[WLAN EVENT] Unexpected event \n\r");
    	break;
  }
}

void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
	long retval;
	NETWORK_PRINT("%s Entered\r\n", __func__);
	network_event(SIMPLE_LINK_NETAPP_EVENT, pNetAppEvent->Event, NULL, 0);
	switch(pNetAppEvent->Event){
		case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
		case SL_NETAPP_IPV6_IPACQUIRED_EVENT:
			SET_STATUS_BIT(nc.status, STATUS_BIT_IP_AQUIRED);
		break;
		case SL_NETAPP_IP_LEASED_EVENT:
			SET_STATUS_BIT(nc.status, STATUS_BIT_IP_LEASED);
		
			nc.ip = (pNetAppEvent)->EventData.ipLeased.ip_address;
			
			NETWORK_PRINT("[NETAPP EVENT] IP Leased to Client: IP=%d.%d.%d.%d , ",
									SL_IPV4_BYTE(nc.ip,3), SL_IPV4_BYTE(nc.ip,2),
									SL_IPV4_BYTE(nc.ip,1), SL_IPV4_BYTE(nc.ip,0));
			break;
		case SL_NETAPP_IP_RELEASED_EVENT:
			CLR_STATUS_BIT(nc.status, STATUS_BIT_IP_LEASED);

			NETWORK_PRINT("[NETAPP EVENT] IP Leased to Client: IP=%d.%d.%d.%d , ",
									SL_IPV4_BYTE(nc.ip,3), SL_IPV4_BYTE(nc.ip,2),
									SL_IPV4_BYTE(nc.ip,1), SL_IPV4_BYTE(nc.ip,0));

			break;

		default:
			NETWORK_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
									 pNetAppEvent->Event);
			break;
	}
}

void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent, SlHttpServerResponse_t *pHttpResponse)
{
	NETWORK_PRINT("%s Entered\r\n", __func__);
}

void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
  //
  // Most of the general errors are not FATAL are are to be handled
  // appropriately by the application
  //
  NETWORK_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
             pDevEvent->EventData.deviceEvent.status, 
             pDevEvent->EventData.deviceEvent.sender);
}

void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
  //
  // This application doesn't work w/ socket - Events are not expected
  //
	NETWORK_PRINT("%s Entered\r\n", __func__);
	switch( pSock->Event ){
    case SL_SOCKET_TX_FAILED_EVENT:
      switch( pSock->EventData.status )
      {
        case SL_ECLOSE: 
          NETWORK_PRINT("[SOCK ERROR] - close socket (%d) operation "
                      "failed to transmit all queued packets\n\n", 
                      pSock->EventData.sd);
          break;
        default: 
          NETWORK_PRINT("[SOCK ERROR] - TX FAILED : socket %d , reason"
                        "(%d) \n\n",
                        pSock->EventData.sd, pSock->EventData.status);
      }
      break;

    default:
      NETWORK_PRINT("[SOCK EVENT] - Unexpected Event [%x0x]\n\n",pSock->Event);
  }
}

/*End Events */

/* General Network Functions */
int ping_test(unsigned long ulIpAddr)
{  
    signed long           lRetVal = -1;
    SlPingStartCommand_t PingParams;
    SlPingReport_t PingReport;
    PingParams.PingIntervalTime = PING_INTERVAL;
    PingParams.PingSize = PING_PKT_SIZE;
    PingParams.PingRequestTimeout = PING_TIMEOUT;
    PingParams.TotalNumberOfAttempts = NO_OF_ATTEMPTS;
    PingParams.Flags = PING_FLAG;
    PingParams.Ip = ulIpAddr; /* Cleint's ip address */
    
    NETWORK_PRINT("Running Ping Test...\n\r");
    /* Check for LAN connection */
    lRetVal = sl_NetAppPingStart((SlPingStartCommand_t*)&PingParams, SL_AF_INET,
                            (SlPingReport_t*)&PingReport, NULL);
    ASSERT_ON_ERROR(lRetVal);

    nc.ping_packet_receive = PingReport.PacketsReceived;

    if (nc.ping_packet_receive > 0 && nc.ping_packet_receive <= NO_OF_ATTEMPTS)
    {
      // LAN connection is successful
      NETWORK_PRINT("Ping Test successful\n\r");
    }
    else
    {
        // Problem with LAN connection
        ASSERT_ON_ERROR(LAN_CONNECTION_FAILED);
    }

    return SUCCESS;
}

void ping_report(SlPingReport_t *ping_report)
{
	SET_STATUS_BIT(nc.status, STATUS_BIT_PING_DONE);
  nc.ping_packet_receive = ping_report->PacketsReceived;
}

void initialize_network_controller()
{
	nc.status = 0;
	nc.ip = 0;
	nc.ping_packet_receive = 0;
	nc.gateway_ip = 0;
}

/* Configure Simplelink to a default state */
/*****************************************************************************
/! \brief This function puts the device in its default state. It:
/!           - Set the mode to STATION
/!           - Configures connection policy to Auto and AutoSmartConfig
/!           - Deletes all the stored profiles
/!           - Enables DHCP
/!           - Disables Scan policy
/!           - Sets Tx power to maximum
/!           - Sets power policy to normal
/!           - Unregister mDNS services
/!           - Remove all filters
/!
/! \param   none
/! \return  On success, zero is returned. On error, negative is returned
*****************************************************************************/
long configure_simplelink_to_default_state()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long retval = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // If the device is not in station-mode, try configuring it in station-mode 
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this event 
            // before doing anything 
            while(!IS_IP_ACQUIRED(nc.status))
            {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask(); 
#endif
            }
        }

        // Switch to STA role and restart 
        retval = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(retval);

        retval = sl_Stop(0xFF);
        ASSERT_ON_ERROR(retval);

        retval = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(retval);

        // Check if the device is in station again 
        if (ROLE_STA != retval)
        {
            // We don't want to proceed if the device is not coming up in STA-mode 
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }
    
    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    retval = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, 
                                &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(retval);
    
    NETWORK_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    NETWORK_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig 
    //      (Device's default connection policy)
    retval = sl_WlanPolicySet(SL_POLICY_CONNECTION, 
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(retval);

    // Remove all profiles
    retval = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(retval);

    

    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore 
    // other return-codes
    //
    retval = sl_WlanDisconnect();
    if(0 == retval)
    {
        // Wait
        while(IS_CONNECTED(nc.status))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask(); 
#endif
        }
    }

    // Enable DHCP client
    retval = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    ASSERT_ON_ERROR(retval);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    retval = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(retval);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    retval = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, 
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(retval);

    // Set PM policy to normal
    retval = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(retval);

    // Unregister mDNS services
    retval = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(retval);

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    retval = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(retval);

    retval = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(retval);

    initialize_network_controller();
    
    return retval; // Success
}

int get_ssid_name(char *ssid_name, unsigned int max_len)
{
  char ucRecvdAPDetails = 0;
  int  iRetVal = 0;
  char acCmdStore[128];
  do
  {
      ucRecvdAPDetails = 0;

      //
      // Get the AP name to connect over the UART
      //
      iRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
      if(iRetVal > 0)
      {
          // remove start/end spaces if any
          iRetVal = TrimSpace(acCmdStore);

          //
          // Parse the AP name
          //
          strncpy(ssid_name, acCmdStore, iRetVal);
          if(ssid_name != NULL)
          {
              ucRecvdAPDetails = 1;
              ssid_name[iRetVal] = '\0';
          }
      }
  }while(ucRecvdAPDetails == 0);

  return(iRetVal);
}

bool profiles_available(){
	long								retval;
	int 								i;
	char 								name[33];
	uint16_t 						name_len;
	uint8_t 						mac_addr[6];
	SlSecParams_t 			sec_params;
	SlGetSecParamsExt_t sec_ext_params;
	uint32_t						priority;

	sl_Start(NULL, NULL, NULL);

	for (i = 0; i < ANNALOGGER_PROFILE_COUNT; i++){
		NETWORK_PRINT("Getting network profile %d\n\r", i);
		retval = sl_WlanProfileGet(i, name, &name_len, mac_addr, &sec_params, &sec_ext_params, &priority);
		if (retval >= 0){
			//found one
			NETWORK_PRINT("Profile Security Type: %d", sec_params.Type);
			NETWORK_PRINT("Profile %d found\n\r", i);
			NETWORK_PRINT("Profile name: %s\r\n", name);
			NETWORK_PRINT("Profile priority: %d\r\n", priority);
			sl_Stop(0);
			return true;
		}
	}
	sl_Stop(0);
	return false;
}

