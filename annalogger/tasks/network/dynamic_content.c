#include "network.h"
#include "HttpDynamic.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <stdio.h>
#include <string.h>



#define DYNAMIC_BUFFER_SIZE 4096

//Function Prototype
int process_dynamic_html(char * out_buffer, char * in_buffer);
long Network_IF_IpConfigGet(unsigned long *pulIP, unsigned long *pulSubnetMask,
                unsigned long *pulDefaultGateway, unsigned long *pulDNSServer);


char filename[40]; 				//String storing filename
char RESOURCE_PARAM_CONFIG[]  = "param_config.html";

static char * ERROR_STR = "Error processing request";

extern network_controller_t nc;
extern unsigned long tick_seconds;

void setup_dynamic_content(void){
  SetResources(GET, &RESOURCE_PARAM_CONFIG[0], &exe_param_config);
}

int exe_param_config(struct HttpRequest* request){

	struct HttpBlob content;
  struct HttpBlob content_type;
  struct HttpBlob location;
  char *in_buffer;
  char *buffer_out;
  long file_handle;
  uint32_t total_length;
  long retval;
	SlFsFileInfo_t file_info;
  bool response_header;
  content_type.pData = NULL;
  content_type.uLength = 0;

  location.pData = NULL;
  location.uLength = 0;
  memset(filename, '\0', 40);
  strcpy(filename, "www/");
  strncat(filename, RESOURCE_PARAM_CONFIG, strlen(RESOURCE_PARAM_CONFIG));

  //Allocate some space for the outgoing page
  in_buffer = (char *) calloc(DYNAMIC_BUFFER_SIZE, sizeof(char));
  buffer_out = (char *) calloc(6000, sizeof(char));

  NETWORK_PRINT("Dynamic Content: Buffer pointer: %p\r\n", in_buffer);
  if (in_buffer == NULL){
    NETWORK_PRINT("Dynamic Content: Allocation for input buffer failed!\r\n");
    return 0;
  }
  if (buffer_out == NULL){
    NETWORK_PRINT("Dynamic Content: Allocation for output buffer failed!\r\n");
    free(in_buffer);
    return 0;
  }

  //Get the length of the file
  retval = sl_FsGetInfo((unsigned char *) filename, 0, &file_info);
  if (retval < 0){
    NETWORK_PRINT("Dynamic Content: Failed to get file info for: %s\r\n", filename);
    retval = 0; 
    goto cleanup;
  }
  total_length = file_info.FileLen;

  //Open the param_config.html file
  retval = sl_FsOpen((unsigned char *) filename, FS_MODE_OPEN_READ, NULL, &file_handle);
  if (retval < 0){
    NETWORK_PRINT("Dynamic Content: Failed to open file: %s\r\n", filename);
    retval = 0; 
    goto cleanup;
  }

  //Read the file
  retval = sl_FsRead(file_handle, 0, in_buffer, total_length);
  if (retval < 0){
    NETWORK_PRINT("Dynamic Content: Failed to read file: %s\r\n", filename);
    retval = 0; 
    goto fileclose;
  }
  retval = process_dynamic_html(buffer_out, in_buffer);
  NETWORK_PRINT("Buffer Out:\r\n%s\r\n", buffer_out);


  //Send the header
  retval = HttpResponse_Headers(request->uConnection, HTTP_STATUS_OK, 0, total_length, content_type, location);
  if (retval < 0){
    NETWORK_PRINT("Dynamic Content: Failed to send Response Header\r\n");
    retval = 0; 
    goto fileclose;
  }


  //Create a response
  content.pData = buffer_out;
  content.uLength = total_length;

  //Send a response to the user
  retval = HttpResponse_Content(request->uConnection, content);
  if (retval < 0){
    NETWORK_PRINT("Dynamic Content: Failed to send Response\r\n");
    retval = 0;
    goto fileclose;
  }



  //passed the gauntlet
  retval = 1;
               
fileclose:
  sl_FsClose(file_handle, 0, 0, 0);
cleanup:
  free(in_buffer);
  free(buffer_out);
  

  //if not connected
  return retval;
}

int process_dynamic_html(char * out_buffer, char * in_buffer){
  char *buffer;
  char *p_char;
  char *p_out;
  uint8_t i;
  uint16_t length;
  uint8_t small_buf[33];
  p_char = NULL;
  p_out = out_buffer;
  long retval;
  unsigned short config_opt;

  unsigned long ip[4];

  //Device Info
  SlVersionFull   ver = {0};
  unsigned char ucConfigOpt = 0;
  unsigned char ucConfigLen = 0;
  long ticks = 0;

	char 								name[33];
	uint16_t 						name_len;
	uint8_t 						mac_addr[6];
	SlSecParams_t 			sec_params;
	SlGetSecParamsExt_t sec_ext_params;
	uint32_t						priority;

  SlNetAppDhcpServerBasicOpt_t dhcpParams;
  Network_IF_IpConfigGet(&ip[0], &ip[1], &ip[2], &ip[3]);

  i = sizeof(SlNetCfgIpV4Args_t);
  uint8_t dhcpIsOn = 0;
  SlNetCfgIpV4Args_t ipV4 = {0};
  sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,
               &dhcpIsOn,
               &i,
               (uint8_t *) &ipV4);



  retval = sl_NetAppGet(SL_NET_APP_DHCP_SERVER_ID,
                        NETAPP_SET_DHCP_SRV_BASIC_OPT,
                        &i,
                        (unsigned char *)&dhcpParams);
 

  //Initialize values
  length = 0;


  //Get the Status
  ucConfigOpt = SL_DEVICE_STATUS;

  // Get the device's version-information
  ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
  ucConfigLen = sizeof(ver);
  retval = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, 
                              &ucConfigLen, (unsigned char *)(&ver));
  ASSERT_ON_ERROR(retval);


  //Replace the String Token
  p_char = strstr(in_buffer, "__SL_G_");
  length = p_char - in_buffer;
  strncpy(p_out, in_buffer, length);

  p_out += length;
  p_char += strlen("__SL_G_");
  while (p_char != NULL){
    //NETWORK_PRINT("P_CHAR: %c\r\n", p_char[0]);
    length = 0;
    switch(p_char[0]){
      case ('S'):
        //System Information
        //NETWORK_PRINT("System: %c\r\n", p_char[2]);
        //strcpy(p_out, "System");
        //p_out += strlen("System");
        switch (p_char[2]){
          case ('0'):
            //Device_Status
            if (IS_CONNECTED(nc.status)){
              length = sprintf(p_out, "Connected");
              p_out += length;
            }
            else {
              length = sprintf(p_out, "Not Connected");
              p_out += length;
            }
            break;
          case ('A'):
            //System_Up_Time
            ticks = tick_seconds;

            
            length = sprintf(p_out, "%03d days %02d.%02d.%02d", 
                              (ticks / (3600 * 24)),
                              (ticks / 3600),
                              (ticks /60 ) % 60,
                              ticks % 60);
            p_out += length;
            break;
          case ('B'):
            //Device_Name_URN
            i = 35;
            sl_NetAppGet( SL_NET_APP_DEVICE_CONFIG_ID,
                          NETAPP_SET_GET_DEV_CONF_OPT_DEVICE_URN,
                          &i,
                          small_buf);
            length = sprintf(p_out, "%s", small_buf);
            p_out += length;
            break;
          case ('C'):
            //Domain_Name              
            i = 35;
            sl_NetAppGet( SL_NET_APP_DEVICE_CONFIG_ID,
                          NETAPP_SET_GET_DEV_CONF_OPT_DOMAIN_NAME,
                          &i,
                          small_buf);
            length = sprintf(p_out, "%s", small_buf);
            p_out += length;
            break;
          case ('D'):
            //Device_mode_role         
            length = sprintf(p_out, "%d", nc.mode);
            p_out = p_out + length;
            break;
          case ('E'):
            //Device_Role_Station      
            if (nc.mode == ROLE_STA){
              length = sprintf(p_out, "true");
            }
            else {
              length = sprintf(p_out, "false");
            }
            p_out += length;
            break;
          case ('F'):
            //Device_Role_AP           
            if (nc.mode == ROLE_AP){
              length = sprintf(p_out, "true");
            }
            else {
              length = sprintf(p_out, "false");
            }
            p_out = p_out + length;
            break;
          case ('G'):
            //Device_Role_P2P          
            if (nc.mode == ROLE_P2P){
              length = sprintf(p_out, "true");
            }
            else {
              length = sprintf(p_out, "false");
            }
            p_out += length;
            break;
          case ('H'):
            //Device_Name_URN          
            i = 16;
            sl_NetAppGet( SL_NET_APP_DEVICE_CONFIG_ID,
                          NETAPP_SET_GET_DEV_CONF_OPT_DEVICE_URN,
                          &i,
                          small_buf);
            length = sprintf(p_out, "%s", small_buf);
            p_out += length;
            break;
          case ('I'):
            //System_requires_reset    
            length = sprintf(p_out, (nc.requires_reset) ? "true":"false");
            p_out += length;
            break;
          case ('J'):
            //Get_system_time_and_date 
            break;
          case ('K'):
            //Safe_mode_status         
            break;
          default:

            break;
        }
        break;
      case ('V'):
        //Versions
        switch(p_char[2]){
          case('A'):
            length = sprintf(p_out, "%d.%d.%d.%d",
                                    ver.NwpVersion[0],
                                    ver.NwpVersion[1],
                                    ver.NwpVersion[2],
                                    ver.NwpVersion[3]);
            break;
          case('B'):
            length = sprintf(p_out, "%d.%d.%d.%d",
                                    ver.ChipFwAndPhyVersion.FwVersion[0],
                                    ver.ChipFwAndPhyVersion.FwVersion[1],
                                    ver.ChipFwAndPhyVersion.FwVersion[2],
                                    ver.ChipFwAndPhyVersion.FwVersion[3]);


            break;
          case('C'):
            length = sprintf(p_out, "%d.%d.%d.%d",
                                    ver.ChipFwAndPhyVersion.PhyVersion[0],
                                    ver.ChipFwAndPhyVersion.PhyVersion[1],
                                    ver.ChipFwAndPhyVersion.PhyVersion[2],
                                    ver.ChipFwAndPhyVersion.PhyVersion[3]);

            break;
          case('D'):
            length = sprintf(p_out, "%d",
                                    ver.RomVersion);

            break;
          default:
            break;
        }
        //NETWORK_PRINT("Version: %c\r\n", p_char[2]);
        //strcpy(p_out, "Version");
        //p_out += strlen("Version");
        p_out += length;
        break;
      case ('N'):
        //Station/P2P Client/Network
        //NETWORK_PRINT("Network: %c\r\n", p_char[2]);
        //strcpy(p_out, "Network");
        //p_out += strlen("Network");
        switch (p_char[2]){
          case ('A'):
            //STA_IP_Addressg        
            
            /*
            if (nc.ipv6){
              length = sprintf(p_out, "%d.%d.%d.%d.%d.%d", 
                                      nc.ip[0], 
                                      nc.ip[1], 
                                      nc.ip[2], 
                                      nc.ip[3], 
                                      nc.ip[4], 
                                      nc.ip[5]);
            }
            else {
            */
              length = sprintf(p_out, "%d.%d.%d.%d",
                             SL_IPV4_BYTE(ip[0], 3),
                             SL_IPV4_BYTE(ip[0], 2),
                             SL_IPV4_BYTE(ip[0], 1),
                             SL_IPV4_BYTE(ip[0], 0));

            //}
            p_out += length;
            break;
          case ('B'):
            //STA_Subnet_Maskg       
            length = sprintf(p_out, "%d.%d.%d.%d",
                             SL_IPV4_BYTE(ip[1], 3),
                             SL_IPV4_BYTE(ip[1], 2),
                             SL_IPV4_BYTE(ip[1], 1),
                             SL_IPV4_BYTE(ip[1], 0));
            p_out += length;
            break;
          case ('C'):
            //STA_Default_Gateway    
            /*
            if (nc.ipv6){
              length = sprintf(p_out, "%d.%d.%d.%d.%d.%d", 
                                      nc.gip[0], 
                                      nc.gip[1], 
                                      nc.gip[2], 
                                      nc.gip[3], 
                                      nc.gip[4], 
                                      nc.gip[5]);
            }
            else {
            */
              length = sprintf(p_out, "%d.%d.%d.%d",
                                SL_IPV4_BYTE(ip[2], 3),
                                SL_IPV4_BYTE(ip[2], 2),
                                SL_IPV4_BYTE(ip[2], 1),
                                SL_IPV4_BYTE(ip[2], 0));

            //}
            p_out += length;
            break;
          case ('D'):
            //MAC_Addressg           
            i = SL_MAC_ADDR_LEN;
            sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &i, small_buf);
            length = sprintf(p_out, "%X:%X:%X:%X:%X:%X",
                    small_buf[0],
                    small_buf[1],
                    small_buf[2],
                    small_buf[3],
                    small_buf[4],
                    small_buf[5]);
            p_out += length;
            break;
          case ('E'):
            //STA_DHCP_State
            length = sprintf(p_out, "%d", dhcpIsOn);
            p_out += length;
            break;
          case ('F'):
            //STA_DHCP_Disable_State
            length = sprintf(p_out, (dhcpIsOn > 0) ? "false":"true");
            p_out += length;

            break;
          case ('G'):
            //STA_DHCP_Enable_State
            length = sprintf(p_out, (dhcpIsOn > 0) ? "true":"false");
            p_out += length;

            break;
          case ('H'):
            //STA_DNS_server
            if (nc.ipv6){
              length = sprintf(p_out, "%d.%d.%d.%d.%d.%d",
                                nc.dns[0],
                                nc.dns[1],
                                nc.dns[2],
                                nc.dns[3],
                                nc.dns[4],
                                nc.dns[5]);
            }
            else {
              length = sprintf(p_out, "%d.%d.%d.%d",
                                SL_IPV4_BYTE(ip[3], 3),
                                SL_IPV4_BYTE(ip[3], 2),
                                SL_IPV4_BYTE(ip[3], 1),
                                SL_IPV4_BYTE(ip[3], 0));
            }
            p_out += length;
            break;
          case ('I'):
            length = sprintf(p_out, "%d.%d.%d.%d",
              SL_IPV4_BYTE(dhcpParams.ipv4_addr_start,3),
              SL_IPV4_BYTE(dhcpParams.ipv4_addr_start,2),
              SL_IPV4_BYTE(dhcpParams.ipv4_addr_start,1),
              SL_IPV4_BYTE(dhcpParams.ipv4_addr_start,0));
            p_out += length;

            break;
          case ('J'):
            length = sprintf(p_out, "%d.%d.%d.%d",
              SL_IPV4_BYTE(dhcpParams.ipv4_addr_last,3),
              SL_IPV4_BYTE(dhcpParams.ipv4_addr_last,2),
              SL_IPV4_BYTE(dhcpParams.ipv4_addr_last,1),
              SL_IPV4_BYTE(dhcpParams.ipv4_addr_last,0));
            p_out += length;


            break;
          case ('K'):
            length = sprintf(p_out, "%d", dhcpParams.lease_time);
            p_out += length;
            break;

          default:
            break;
        }
        break;
      case ('W'):
        //WIFI
        //WIFI Status
        switch (p_char[2]){
          case ('A'):
            //Channel Num In AP Mode
            config_opt = WLAN_AP_OPT_CHANNEL;
            length = 1;
            sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &length, &i);
            length = sprintf(p_out, "%d", i);
            p_out += length;
            break;
          case ('B'):
            //SSID
            config_opt = WLAN_AP_OPT_SSID;
            length = 32;
            sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &length, small_buf);
            length = sprintf(p_out, "%s", small_buf);
            p_out += length;
            NETWORK_PRINT("Length: %d\r\n", length);
            break;
          case ('C'):
            //Security Type
            config_opt = WLAN_AP_OPT_SECURITY_TYPE;
            length = 1;
            sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &length, &i);
            length = sprintf(p_out, "%d", i);
            p_out += length;
            break;
          case ('D'):
            //Security Type Open
            config_opt = WLAN_AP_OPT_SECURITY_TYPE;
            length = 1;
            //sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &length, p_out);
            sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &length, &i);
            length = sprintf(p_out, (i == SL_SEC_TYPE_OPEN) ? "true":"false");
            p_out += length;
            break;
          case ('E'):
            //Security Type WEP
            config_opt = WLAN_AP_OPT_SECURITY_TYPE;
            length = 1;
            //sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &length, p_out);
            sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &length, &i);
            length = sprintf(p_out, (i == SL_SEC_TYPE_WEP) ? "true":"false");
            p_out += length;
            break;
          case ('F'):
            //Security Type WPA
            config_opt = WLAN_AP_OPT_SECURITY_TYPE;
            length = 1;
            //sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &length, p_out);
            sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &length, &i);
            length = sprintf(p_out, (i == SL_SEC_TYPE_WPA) ? "true":"false");
            p_out += length;
            break;
          default:
            break;
        }
        

        //NETWORK_PRINT("WIFI: %c\r\n", p_char[2]);
        //strcpy(p_out, "WIFI");
        //p_out += strlen("WIFI");
        break;
      case ('T'):
        //Ping Test Result
        //NETWORK_PRINT("Ping Test Result: %c\r\n", p_char[2]);
        strcpy(p_out, "Ping Test");
        p_out += strlen("Ping Test");
        break;
      case ('P'):
        //Profile
        //NETWORK_PRINT("Profile: %c%c\r\n", p_char[1], p_char[2]);
        //strcpy(p_out, "Profile");
        //p_out += strlen("Profile");
       
        length = p_char[2] - '1';
        //NETWORK_PRINT("Profile Index: %d\r\n", length);
        memset(name, 0, 1);
        retval  = sl_WlanProfileGet(length,
                                    name, 
                                    &name_len, 
                                    small_buf, 
                                    &sec_params, 
                                    &sec_ext_params,
                                    &priority);
        

        //NETWORK_PRINT("WLAN Profile: %d\r\n", retval);
        //NETWORK_PRINT("\tNAME: %s\r\n", name);
        switch (p_char[1]){
          case ('N'):
            length = sprintf(p_out, "%s", name);
            p_out += length;
            break;
          case ('S'):
            switch(sec_params.Type){
              case (0):
                length = sprintf(p_out, "Open");
                break;
              case (1):
                length = sprintf(p_out, "WEP");
                break;
              case (2):
                length = sprintf(p_out, "WPA");
                break;
              case (255):
              default:
                length = sprintf(p_out, "Not Setup");
                //length = sprintf(p_out, "%d", sec_params.Type);
                break;
            }
            p_out += length;
            break;
          case ('P'):
            length = sprintf(p_out, "%d", priority);
            p_out += length;
            break;
          default:
            break;
        }
        break;
      default:
        p_char = NULL;
        break;
    }
    if (p_char != NULL){
      //Add a new line
      buffer = p_char;
      p_char = strstr(buffer, "<p");
      if (p_char == NULL) {
        p_char = strstr(buffer, "</body>");
      }

      buffer += p_char - buffer;
      strcpy(p_out, "</p>\r\n");
      p_out += strlen("</p>\r\n");
      p_char = strstr(buffer, "__SL_G_");
      if (p_char != NULL){
      
        //There are still more values to substitute
        length = p_char - buffer;
        strncpy(p_out, buffer, length);
        p_out += length;
        p_char += strlen("__SL_G_");
      }
      else {
        //There are no more values to substitute
        strcpy(p_out, buffer);
        p_char = NULL;
      }
    }
  }
  return retval; 
}


long Network_IF_IpConfigGet(unsigned long *pulIP, unsigned long *pulSubnetMask,
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


