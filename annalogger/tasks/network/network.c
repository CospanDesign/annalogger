#include "network.h"

extern network_controller_t nc;

/* Events */

void SimpleLinkWlanEventHandler(SlWlanEvent_t *pSlWlanEvent)
{
  slWlanConnectAsyncResponse_t*  pEventData = NULL; 
  NETWORK_PRINT("[WLAN EVENT]\r\n");

  switch(pSlWlanEvent->Event)
  {
    case SL_WLAN_CONNECT_EVENT:
      SET_STATUS_BIT(nc.status, STATUS_BIT_CONNECTION);
      //Copy new connection SSID and BSSID to structure
      memcpy(nc.ssid,
             pSlWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_name,
             pSlWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
      memcpy(nc.bssid,
             pSlWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
             SL_BSSID_LENGTH);

      NETWORK_PRINT("\tSTA Connected to the AP: %s ,"
                    "BSSID: %X:%X:%X:%X:%X:%X\r\n",
                    nc.ssid,
                    nc.bssid[0], nc.bssid[1], nc.bssid[2],
                    nc.bssid[3], nc.bssid[4], nc.bssid[5]);

    	break;

    case SL_WLAN_DISCONNECT_EVENT:

      NETWORK_PRINT("\tDisconnect event");
      CLR_STATUS_BIT(nc.status, STATUS_BIT_CONNECTION);
      CLR_STATUS_BIT(nc.status, STATUS_BIT_IP_ACQUIRED);

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
      memset(nc.ssid, 0, sizeof(nc.ssid));
      memset(nc.bssid, 0, sizeof(nc.bssid));

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

	network_event(SIMPLE_LINK_WLAN_EVENT, pSlWlanEvent->Event, NULL, 0);
}

void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
	long retval;
  int i;
	NETWORK_PRINT("[NETAPP EVENT]\r\n", __func__);
	switch(pNetAppEvent->Event){
		case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
      nc.ipv6 = false;
      for (i = 0; i < 4; i++){
        nc.ip[i] = SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip, i);
        nc.gip[i] = SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,i);


      }
      NETWORK_PRINT("[NETAPP EVENT] IPV4 Acquired: IP=%d.%d.%d.%d , "
            "Gateway=%d.%d.%d.%d\n\r", 
            nc.ip[3], nc.ip[2], nc.ip[1], nc.ip[0],
            nc.gip[3], nc.gip[2], nc.gip[1], nc.gip[0]);
			SET_STATUS_BIT(nc.status, STATUS_BIT_IP_ACQUIRED);
      break;

		case SL_NETAPP_IPV6_IPACQUIRED_EVENT:
      nc.ipv6 = true;
      /*
	    for (i = 0; i < 6; i++){
//XXX: Don't know if the SL_IPV4_BYTE will work on IPV6!
        //nc.ip[i] = SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV6.ip, i);
        //nc.gip[i] = SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV6.gateway,i);


      }
      */
      //NETWORK_PRINT("[NETAPP EVENT] IPV6 Acquired: IP=%d.%d.%d.%d.%d.%d , "
      //      "Gateway=%d.%d.%d.%d.%d.%d\n\r", 
      //      nc.ip[5], nc.ip[4], nc.ip[3], nc.ip[2], nc.ip[1], nc.ip[0],
      //      nc.gip[5], nc.gip[4], nc.gip[3], nc.gip[2], nc.gip[1], nc.gip[0]);
      NETWORK_PRINT("[NETAPP EVENT] IPV6 Acquired, TODO SAVE IP!\r\n");


			SET_STATUS_BIT(nc.status, STATUS_BIT_IP_ACQUIRED);

		  break;
		case SL_NETAPP_IP_LEASED_EVENT:
			SET_STATUS_BIT(nc.status, STATUS_BIT_IP_LEASED);
	    NETWORK_PRINT("\tIP %d.%d.%d.%d Leased\r\n",
  									nc.ip[3], nc.ip[2], nc.ip[1], nc.ip[0]);
		
      for (i = 0; i < 4; i++){
        nc.ip[i] = SL_IPV4_BYTE(pNetAppEvent->EventData.ipLeased.ip_address, i);
      }

			
      /*
			NETWORK_PRINT("[NETAPP EVENT] IP Leased to Client: IP=%d.%d.%d.%d , ",
									SL_IPV4_BYTE(nc.ip,3), SL_IPV4_BYTE(nc.ip,2),
									SL_IPV4_BYTE(nc.ip,1), SL_IPV4_BYTE(nc.ip,0));
      */
			break;
		case SL_NETAPP_IP_RELEASED_EVENT:
			CLR_STATUS_BIT(nc.status, STATUS_BIT_IP_LEASED);
	    NETWORK_PRINT("\tIP %d.%d.%d.%d Released\r\n",
  									nc.ip[3], nc.ip[2], nc.ip[1], nc.ip[0]);
			break;

		default:
      NETWORK_PRINT("\tUnexpected event: [0x%X]\r\n", pNetAppEvent->Event);
      /*
			NETWORK_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
									 pNetAppEvent->Event);
      */
			break;
	}
	network_event(SIMPLE_LINK_NETAPP_EVENT, pNetAppEvent->Event, NULL, 0);
}

void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent, SlHttpServerResponse_t *pHttpResponse)
{
	NETWORK_PRINT("%s Entered\r\n", __func__);
	network_event(SIMPLE_LINK_HTTP_SERVER_EVENT, pHttpEvent->Event, NULL, 0);
}

void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
  //
  // Most of the general errors are not FATAL are are to be handled
  // appropriately by the application
  //
  NETWORK_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\r\n",
             pDevEvent->EventData.deviceEvent.status, 
             pDevEvent->EventData.deviceEvent.sender);
}

void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
  //
  // This application doesn't work w/ socket - Events are not expected
  //
	NETWORK_PRINT("[SOCK EVENT]\r\n", __func__);
	switch( pSock->Event ){
    case SL_SOCKET_TX_FAILED_EVENT:
      switch( pSock->EventData.status )
      {
        case SL_ECLOSE: 
          NETWORK_PRINT("[SOCK ERROR] - close socket (%d) operation failed to transmit all queued packets\r\n", 
                      pSock->EventData.sd);
          break;
        default: 
          NETWORK_PRINT("[SOCK ERROR] - TX FAILED : socket %d , reason (%d) \r\n",
                        pSock->EventData.sd, pSock->EventData.status);
          break;
      }
      break;

    default:
      NETWORK_PRINT("[SOCK EVENT] - Unexpected Event [%x0x]\r\n",pSock->Event);
      break;
  }
}


/* Web Socket Events */

/*!
 * 	\brief 					This websocket Event is called when WebSocket Server receives data
 * 							from client.
 *
 *
 * 	\param[in] puConnection	Websocket Client Id
 * 	\param[in] *ReadBuffer		Pointer to the buffer that holds the payload.
 *
 * 	\return					none.
 *     					
 */

void WebSocketRecvEventHandler(UINT16 uConnection, char *ReadBuffer)
{
  NETWORK_PRINT("[WEB SOCKET EVENT]: Received from client!\r\n");
}

/*!
 * 	\brief 							Callback function that indicates that handshake was a success
 * 									Once this is called the server can start sending data packets over websocket using
 * 									the sl_WebSocketSend API.
 *
 *
 * 	\param[in] uConnection				Websocket Client Id
 *
 * 	\return							void
 */
void WebSocketHandshakeEventHandler(UINT16 uConnection)
{
  NETWORK_PRINT("[WEB SOCKET EVENT]: Handshake!\r\n");
  /*
  ws_conn_t * ws_conn = calloc(1, sizeof(ws_conn_t));

  if (ws_conn == NULL){
    //NETWORK_PRINT(
    //  "Failed to allocate space for a web socket connection\r\n");
    //master_event( MASTER_EVENT_MALLOC_FAILED,
    //              0,
    //              NULL,
    //              0);
    return;
  }
  if (!wsl_add(nc.ws_list, uConnection, ws_conn)){
    //NETWORK_PRINT("Failed to add space to the socket list\r\n");
    //master_event( MASTER_EVENT_NETWORK_GENERAL_ERROR,
    //              0,
    //              NULL,
    //              0);
  }
  */
  
}

/*!
 * 	\brief 							Callback function that indicates that Websocket is closed
 * 									Once this is called the server acts as HTTP Server
 *
 *
 * 	\return							None
 */
void WebSocketCloseSessionHandler(void)
{

  NETWORK_PRINT("[WEB SOCKET EVENT]: Close\r\n");
}


/* End Events */

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
  int i = 0;
	nc.status = 0;
  for (i = 0; i < 6; i++){
	  nc.ip[i] = 0;
	  nc.gip[i] = 0;
  }
	nc.ping_packet_receive = 0;
}

/*
 * \brief This function puts the device in its default state. It:
 *           - Set the mode to STATION
 *           - Configures connection policy to Auto and AutoSmartConfig
 *           - Deletes all the stored profiles
 *           - Enables DHCP
 *          - Disables Scan policy
 *          - Sets Tx power to maximum
 *          - Sets power policy to normal
 *          - Unregister mDNS services
 *          - Remove all filters
 *
 *\param   none
 *\return  On success, zero is returned. On error, negative is returned
 */
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
    nc.mode = lMode;
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
    nc.mode = ROLE_STA;
    
    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    retval = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, 
                                &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(retval);
    
    NETWORK_PRINT("Host Driver Version: %s\r\n",SL_DRIVER_VERSION);
    NETWORK_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\r\n",
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

bool profiles_available()
{
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
		NETWORK_PRINT("Getting network profile %d\r\n", i);
		retval = sl_WlanProfileGet(i, name, &name_len, mac_addr, &sec_params, &sec_ext_params, &priority);
		if (retval >= 0){
			//found one
			NETWORK_PRINT("Profile Security Type: %d\r\n", sec_params.Type);
			NETWORK_PRINT("Profile %d found\r\n", i);
			NETWORK_PRINT("Profile name: %s\r\n", name);
			NETWORK_PRINT("Profile priority: %d\r\n", priority);
			sl_Stop(0);
			return true;
		}
	}
	sl_Stop(0);
	return false;
}

//*****************************************************************************
//
//! Network_IF_IpConfigGet  Get the IP Address of the device.
//!
//! \param  ip IP Address of Device
//! \param  subnetmask Subnetmask of Device
//! \param  default_gateway Default Gateway value
//! \param  dns_server DNS Server
//!
//! \return On success, zero is returned. On error, -1 is returned
//
//*****************************************************************************
long get_my_ip(unsigned long *ip,
               unsigned long *subnetmask,
               unsigned long *default_gateway,
               unsigned long *dns_server)
{
    unsigned char isDhcp;
    unsigned char len = sizeof(SlNetCfgIpV4Args_t);
    long lRetVal = -1;
    SlNetCfgIpV4Args_t ipV4 = {0};

    lRetVal = sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,&isDhcp,&len,
                                  (unsigned char *)&ipV4);
    ASSERT_ON_ERROR(lRetVal);

    *ip=ipV4.ipV4;
    *subnetmask=ipV4.ipV4Mask;
    *default_gateway=ipV4.ipV4Gateway;
    *default_gateway=ipV4.ipV4DnsServer;

    return lRetVal;
}





