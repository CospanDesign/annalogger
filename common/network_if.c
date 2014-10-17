//*****************************************************************************
// network_if.c
//
// Networking interface functions for CC3200 device
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

#include <string.h>
#include <stdlib.h>

// Simplelink includes 
#include "simplelink.h"

// driverlib includes 
#include "hw_types.h"
#include "timer.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"

// free-rtos/TI-rtos include
#ifdef SL_PLATFORM_MULTI_THREADED
#include "osi.h"
#endif

// common interface includes 
#include "network_if.h"
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "timer_if.h"
#include "common.h"


// Network App specific status/error codes which are used only in this file
typedef enum{
     // Choosing this number to avoid overlap w/ host-driver's error codes
    DEVICE_NOT_IN_STATION_MODE = -0x7F0,
    DEVICE_NOT_IN_AP_MODE = DEVICE_NOT_IN_STATION_MODE - 1,
    DEVICE_NOT_IN_P2P_MODE = DEVICE_NOT_IN_AP_MODE - 1,

    STATUS_CODE_MAX = -0xBB8
}e_NetAppStatusCodes;



//*****************************************************************************
//
//! Network_IF_InitDriver
//! The function initializes a CC3200 device and triggers it to start operation
//!  
//! \param  uiMode (device mode in which device will be configured)
//!  
//! \return 0 : sucess, -ve : failure
//
//*****************************************************************************
long
Network_IF_InitDriver(unsigned int uiMode)
{
    long lRetVal = -1;
    // Reset CC3200 Network State Machine
    InitializeAppVariables();

    //
    // Following function configure the device to default state by cleaning
    // the persistent settings stored in NVMEM (viz. connection profiles &
    // policies, power policy etc)
    //
    // Applications may choose to skip this step if the developer is sure
    // that the device is in its default state at start of application
    //
    // Note that all profiles and persistent settings that were done on the
    // device will be lost
    //
    lRetVal = ConfigureSimpleLinkToDefaultState();
    if(lRetVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
           UART_PRINT("Failed to configure the device in its default state \n\r");

        LOOP_FOREVER();
    }

    UART_PRINT("Device is configured in default state \n\r");

    //
    // Assumption is that the device is configured in station mode already
    // and it is in its default state
    //

    lRetVal = sl_Start(NULL,NULL,NULL);

    if (lRetVal < 0 || lRetVal != ROLE_STA)
    {
        UART_PRINT("Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Started SimpleLink Device: STA Mode\n\r");

    if(uiMode == ROLE_AP)
    {
        UART_PRINT("Switching to AP mode on application request\n\r");
        // Switch to AP role and restart
        lRetVal = sl_WlanSetMode(uiMode);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is up in AP Mode
        if (ROLE_AP == lRetVal)
        {
            // If the device is in AP mode, we need to wait for this event
            // before doing anything
            while(!IS_IP_ACQUIRED(g_ulStatus))
            {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
            }
        }
        else
        {
            // We don't want to proceed if the device is not coming up in AP-mode
            ASSERT_ON_ERROR(DEVICE_NOT_IN_AP_MODE);
        }

        UART_PRINT("Re-started SimpleLink Device: AP Mode\n\r");
    }
    else if(uiMode == ROLE_P2P)
    {
        UART_PRINT("Switching to P2P mode on application request\n\r");
        // Switch to AP role and restart
        lRetVal = sl_WlanSetMode(uiMode);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again
        if (ROLE_P2P != lRetVal)
        {
            // We don't want to proceed if the device is not coming up in P2P-mode
            ASSERT_ON_ERROR(DEVICE_NOT_IN_P2P_MODE);
        }

        UART_PRINT("Re-started SimpleLink Device: P2P Mode\n\r");
    }
    else
    {
        // Device already started in STA-Mode
    }
    return 0;
}

//*****************************************************************************
//
//! Network_IF_DeInitDriver
//! The function de-initializes a CC3200 device
//!  
//! \param  None
//!  
//! \return On success, zero is returned. On error, other
//
//*****************************************************************************
long
Network_IF_DeInitDriver(void)
{
    long lRetVal = -1;
    UART_PRINT("SL Disconnect...\n\r");

    //
    // Disconnect from the AP
    //
    lRetVal = Network_IF_DisconnectFromAP();

    //
    // Stop the simplelink host
    //
    sl_Stop(SL_STOP_TIMEOUT);

    //
    // Reset the state to uninitialized
    //
    Network_IF_ResetMCUStateMachine();
    return lRetVal;
}


//*****************************************************************************
//
//! Network_IF_ConnectAP  Connect to an Access Point using the specified SSID
//!
//! \param[in]  pcSsid is a string of the AP's SSID
//! \param[in]  SecurityParams is Security parameter for AP
//!
//! \return On success, zero is returned. On error, -ve value is returned
//
//*****************************************************************************
long
Network_IF_ConnectAP(char *pcSsid, SlSecParams_t SecurityParams)
{
#ifndef NOTERM  
    char acCmdStore[128];
    unsigned short usConnTimeout;
    unsigned char ucRecvdAPDetails;
#endif
    long lRetVal;
    unsigned long ulIP = 0;
    unsigned long ulSubMask = 0;
    unsigned long ulDefGateway = 0;
    unsigned long ulDns = 0;

    //
    // Disconnect from the AP
    //
    Network_IF_DisconnectFromAP();
    
    //
    // This triggers the CC3200 to connect to specific AP
    //
    lRetVal = sl_WlanConnect((signed char *)pcSsid, strlen((const char *)pcSsid),
                        NULL, &SecurityParams, NULL);
    ASSERT_ON_ERROR(lRetVal);

    //
    // Wait for ~10 sec to check if connection to desire AP succeeds
    //
    while(g_usConnectIndex < 15)
    {
#ifndef SL_PLATFORM_MULTI_THREADED
        _SlNonOsMainLoopTask();
#endif
        MAP_UtilsDelay(8000000);
        if(IS_CONNECTED(g_ulStatus) && IS_IP_ACQUIRED(g_ulStatus))
        {
            break;
        }
        g_usConnectIndex++;
    }

#ifndef NOTERM
    //
    // Check and loop until AP connection successful, else ask new AP SSID name
    //
    while(!(IS_CONNECTED(g_ulStatus)) || !(IS_IP_ACQUIRED(g_ulStatus)))
    {
        //
        // Disconnect the previous attempt
        //
        Network_IF_DisconnectFromAP();

        CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
        CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);
        UART_PRINT("Device could not connect to %s\n\r",pcSsid);

        do
        {
            ucRecvdAPDetails = 0;

            UART_PRINT("\n\r\n\rPlease enter the AP(open) SSID name # ");

            //
            // Get the AP name to connect over the UART
            //
            lRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
            if(lRetVal > 0)
            {
                // remove start/end spaces if any
                lRetVal = TrimSpace(acCmdStore);

                //
                // Parse the AP name
                //
                strncpy(pcSsid, acCmdStore, lRetVal);
                if(pcSsid != NULL)
                {
                    ucRecvdAPDetails = 1;
                    pcSsid[lRetVal] = '\0';

                }
            }
        }while(ucRecvdAPDetails == 0);

        //
        // Reset Security Parameters to OPEN security type
        //
        SecurityParams.Key = (signed char *)"";
        SecurityParams.KeyLen = 0;
        SecurityParams.Type = SL_SEC_TYPE_OPEN;

        UART_PRINT("\n\rTrying to connect to AP: %s ...\n\r",pcSsid);

        //
        // Get the current timer tick and setup the timeout accordingly
        //
        usConnTimeout = g_usConnectIndex + 15;

        //
        // This triggers the CC3200 to connect to specific AP
        //
        lRetVal = sl_WlanConnect((signed char *)pcSsid,
                                  strlen((const char *)pcSsid), NULL,
                                  &SecurityParams, NULL);
        ASSERT_ON_ERROR(lRetVal);

        //
        // Wait ~10 sec to check if connection to specifed AP succeeds
        //
        while(!(IS_CONNECTED(g_ulStatus)) || !(IS_IP_ACQUIRED(g_ulStatus)))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
            _SlNonOsMainLoopTask();
#endif
            MAP_UtilsDelay(8000000);
            if(g_usConnectIndex >= usConnTimeout)
            {
                break;
            }
            g_usConnectIndex++;
        }

    }
#endif
    //
    // Put message on UART
    //
    UART_PRINT("\n\rDevice has connected to %s\n\r",pcSsid);

    //
    // Get IP address
    //
    lRetVal = Network_IF_IpConfigGet(&ulIP,&ulSubMask,&ulDefGateway,&ulDns);
    ASSERT_ON_ERROR(lRetVal);

    //
    // Send the information
    //
    UART_PRINT("Device IP Address is %d.%d.%d.%d \n\r\n\r",
            SL_IPV4_BYTE(ulIP, 3),SL_IPV4_BYTE(ulIP, 2),
            SL_IPV4_BYTE(ulIP, 1),SL_IPV4_BYTE(ulIP, 0));
    return 0;
}

//*****************************************************************************
//
//! Disconnect  Disconnects from an Access Point
//!
//! \param  none
//!
//! \return 0 disconnected done, other already disconnected
//
//*****************************************************************************
long
Network_IF_DisconnectFromAP()
{
    long lRetVal = 0;

    if (IS_CONNECTED(g_ulStatus))
    {
        lRetVal = sl_WlanDisconnect();
        if(0 == lRetVal)
        {
            // Wait
            while(IS_CONNECTED(g_ulStatus))
            {
    #ifndef SL_PLATFORM_MULTI_THREADED
                  _SlNonOsMainLoopTask();
    #endif
            }
            return lRetVal;
        }
        else
        {
            return lRetVal;
        }
    }
    else
    {
        return lRetVal;
    }

}

//*****************************************************************************
//
//! Network_IF_IpConfigGet  Get the IP Address of the device.
//!
//! \param  pulIP IP Address of Device
//! \param  pulSubnetMask Subnetmask of Device
//! \param  pulDefaultGateway Default Gateway value
//! \param  pulDNSServer DNS Server
//!
//! \return On success, zero is returned. On error, -1 is returned
//
//*****************************************************************************
long
Network_IF_IpConfigGet(unsigned long *pulIP, unsigned long *pulSubnetMask,
                unsigned long *pulDefaultGateway, unsigned long *pulDNSServer)
{
    unsigned char isDhcp;
    unsigned char len = sizeof(SlNetCfgIpV4Args_t);
    long lRetVal = -1;
    SlNetCfgIpV4Args_t ipV4 = {0};

    lRetVal = sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,&isDhcp,&len,
                                  (unsigned char *)&ipV4);
    ASSERT_ON_ERROR(lRetVal);

    *pulIP=ipV4.ipV4;
    *pulSubnetMask=ipV4.ipV4Mask;
    *pulDefaultGateway=ipV4.ipV4Gateway;
    *pulDefaultGateway=ipV4.ipV4DnsServer;

    return lRetVal;
}

//*****************************************************************************
//
//! Network_IF_GetHostIP
//!
//! \brief  This function obtains the server IP address using a DNS lookup
//!
//! \param[in]  pcHostName        The server hostname
//! \param[out] pDestinationIP    This parameter is filled with host IP address.
//!
//! \return On success, +ve value is returned. On error, -ve value is returned
//!
//
//*****************************************************************************
long Network_IF_GetHostIP( char* pcHostName,unsigned long * pDestinationIP )
{
    long lStatus = 0;

    lStatus = sl_NetAppDnsGetHostByName((signed char *) pcHostName,
                                            strlen(pcHostName),
                                            pDestinationIP, SL_AF_INET);
    ASSERT_ON_ERROR(lStatus);

    UART_PRINT("Get Host IP succeeded.\n\rHost: %s IP: %d.%d.%d.%d \n\r\n\r",
                    pcHostName, SL_IPV4_BYTE(*pDestinationIP,3),
                    SL_IPV4_BYTE(*pDestinationIP,2),
                    SL_IPV4_BYTE(*pDestinationIP,1),
                    SL_IPV4_BYTE(*pDestinationIP,0));
    return lStatus;

}

//*****************************************************************************
//
//!  \brief  Reset state from the state machine
//!
//!  \param  None
//!
//!  \return none
//!
//*****************************************************************************
void 
Network_IF_ResetMCUStateMachine()
{
    g_ulStatus = 0;
}

//*****************************************************************************
//
//!  \brief  Return the current state bits
//!
//!  \param  None
//!
//!  \return none
//!
//
//*****************************************************************************
unsigned long
Network_IF_CurrentMCUState()
{
    return g_ulStatus;
}
//*****************************************************************************
//
//!  \brief  sets a state from the state machine
//!
//!  \param  cStat Status of State Machine defined in e_StatusBits
//!
//!  \return none
//!
//*****************************************************************************
void 
Network_IF_SetMCUMachineState(char cStat)
{
    SET_STATUS_BIT(g_ulStatus, cStat);
}

//*****************************************************************************
//
//!  \brief  Unsets a state from the state machine
//!
//!  \param  cStat Status of State Machine defined in e_StatusBits
//!
//!  \return none
//!
//*****************************************************************************
void 
Network_IF_UnsetMCUMachineState(char cStat)
{
    CLR_STATUS_BIT(g_ulStatus, cStat);
}


//*****************************************************************************
//
//! itoa
//!
//!    @brief  Convert integer to ASCII in decimal base
//!
//!     @param  cNum is input integer number to convert
//!     @param  cString is output string
//!
//!     @return number of ASCII parameters
//!
//!
//
//*****************************************************************************
unsigned short itoa(short cNum, char *cString)
{
    char* ptr;
    short uTemp = cNum;
    unsigned short length;

    // value 0 is a special case
    if (cNum == 0)
    {
        length = 1;
        *cString = '0';

        return length;
    }

    // Find out the length of the number, in decimal base
    length = 0;
    while (uTemp > 0)
    {
        uTemp /= 10;
        length++;
    }

    // Do the actual formatting, right to left
    uTemp = cNum;
    ptr = cString + length;
    while (uTemp > 0)
    {
        --ptr;
        *ptr = pcDigits[uTemp % 10];
        uTemp /= 10;
    }

    return length;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************


