#include "network.h"
#include "HttpDynamic.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <stdio.h>
#include <string.h>



#define DYNAMIC_BUFFER_SIZE 6000

//Function Prototype
int process_dynamic_html(char * out_buffer, char * in_buffer);

int process_dynamic_html_request(struct HttpRequest* request, char * fname);
int process_setup_config(struct HttpRequest* request, char * fname);


long dhtml_process_web_request(struct HttpRequest* request, char * filename);
long dhtml_process_line(char * out_buffer, char * line);
long dhtml_process_file(char *out_buffer, char * filename);
long Network_IF_IpConfigGet(unsigned long *pulIP, unsigned long *pulSubnetMask,
                unsigned long *pulDefaultGateway, unsigned long *pulDNSServer);


char RESOURCE_PARAM_CONFIG[]  = "param_config.html";
char RESOURCE_SETUP_CONFIG[]  = "setup.html";

static char * ERROR_STR = "Error processing request";

extern network_controller_t nc;
extern unsigned long tick_seconds;

void setup_dynamic_content(void){
  SetResources(GET, &RESOURCE_PARAM_CONFIG[0], &process_dynamic_html_request);
  SetResources(GET, &RESOURCE_SETUP_CONFIG[0], &process_dynamic_html_request);

}

int process_dynamic_html_request(struct HttpRequest* request, char * fname){
  NETWORK_PRINT("\tFilename: %s\r\n", fname);
  char filename[40];
  NETWORK_PRINT("\tDHTML Process Param Config\r\n");

  memset(filename, '\0', 40);

  strcpy(filename, "www/");
  strncat(filename, fname, strlen(fname));
 
  dhtml_process_web_request(request, filename);
  NETWORK_PRINT("\tDHTML Finished\r\n");
}

long dhtml_process_web_request(struct HttpRequest* request, char * filename){
	struct HttpBlob content;
  struct HttpBlob content_type;
  struct HttpBlob location;
  long  total_length;

  char *out_buffer;
  long retval;

  content_type.pData = NULL;
  content_type.uLength = 0;

  location.pData = NULL;
  location.uLength = 0;

  //Allocate some space for the outgoing page
  out_buffer = (char *) calloc(DYNAMIC_BUFFER_SIZE, sizeof(char));
  if (out_buffer == NULL) {
    return 0;
  }
  NETWORK_PRINT("out buffer address: %p\r\n", out_buffer);
  memset(out_buffer, 0, DYNAMIC_BUFFER_SIZE);
  total_length = dhtml_process_file(out_buffer, filename);
  NETWORK_PRINT("data buffer generated, data size is: %d\r\n", total_length);
  //NETWORK_PRINT("out buffer address: %p\r\n", out_buffer);

  //NETWORK_PRINT("Output Data\r\n%s", out_buffer);
  NETWORK_PRINT("Output Data Length: %d\r\n", total_length);

  //Create a response
  content.pData = out_buffer;
  content.uLength = total_length;

  //Send the header
  retval = HttpResponse_Headers(request->uConnection, HTTP_STATUS_OK, 0, total_length, content_type, location);
  if (retval < 0){
    return retval;
  }

  //Create a response
  content.pData = out_buffer;
  content.uLength = total_length;

  //Send a response to the user
  retval = HttpResponse_Content(request->uConnection, content);
  if (retval < 0){
    return retval;
  }
  free(out_buffer);
}
long dhtml_process_file(char *out_buffer, char * filename){

  long            file_handle;
  uint32_t        file_length;
	SlFsFileInfo_t  file_info;
  long            retval;
  char          * in_buffer;
  char          * ptr;
  char          * out_ptr;

  NETWORK_PRINT("%s: entered\r\n", __func__);
  //Initialize Variables
  file_length     = 0;
  retval          = 0;
  ptr             = NULL;

  //Allocate some space for the outgoing page
  in_buffer       = (char *) calloc(DYNAMIC_BUFFER_SIZE, sizeof(char));
  //out_buffer      = (char *) calloc(DYNAMIC_BUFFER_SIZE, sizeof(char));
  out_ptr         = out_buffer;

  //Get the length of the file
  retval = sl_FsGetInfo((unsigned char *) filename, 0, &file_info);
  ASSERT_ON_ERROR(retval);
  file_length = file_info.FileLen;

  //Opened File
  retval = sl_FsOpen((unsigned char *) filename, FS_MODE_OPEN_READ, NULL, &file_handle);
  ASSERT_ON_ERROR(retval);

  //While we haven't reached the end of the file
  retval = sl_FsRead(file_handle, 0, in_buffer, file_length);
  ASSERT_ON_ERROR(retval);

  //Close File
  sl_FsClose(file_handle, 0, 0, 0);

  ptr = strtok(in_buffer, "\n");
  while (ptr != NULL){
    NETWORK_PRINT("%s(): process line: %s\r\n", __func__, ptr);
    out_ptr += dhtml_process_line(out_ptr, ptr);
    ptr = strtok(NULL, "\n"); 
  }
  NETWORK_PRINT("%s: Buffer:\r\n %s", __func__, out_buffer);
  //NETWORK_PRINT("%s: Finished Line Process\r\n", __func__);
  //Output Buffer now contains the processed data
  retval = strlen(out_buffer);
cleaneup:
  free(in_buffer);
  return retval;
}
long dhtml_process_line(char * out_buffer, char * line){
  int                 total_length;
  int                 length = 0;
  long                final_line_length = 0;
  char    *           ptr = NULL;
  char    *           out_ptr = NULL;
  char    *           final_ptr = NULL;
  uint8_t             small_buf[33];
  uint8_t             i;
  uint16_t            j;
                      
  long                retval;
  unsigned short      config_opt;
  //Device Info       
  SlVersionFull       ver = {0};
  unsigned char       ucConfigOpt = 0;
  unsigned char       ucConfigLen = 0;
  long                ticks = 0;


	char 								name[33];
	uint16_t 						name_len;
	uint8_t 						mac_addr[6];
	SlSecParams_t 			sec_params;
	SlGetSecParamsExt_t sec_ext_params;
	uint32_t						priority;
  uint8_t             dhcpIsOn = 0;
  SlNetCfgIpV4Args_t  ipV4 = {0};
  unsigned long       ip[4];

  SlNetAppDhcpServerBasicOpt_t dhcpParams;

  //Initialize
  //Get the total length of the incomming line
  total_length  = strlen(line);

  //get the length of the line
  ptr = strstr(line, "__SL_G_");
  if ((total_length == 0) || 
      (strlen(line) < strlen("__SL_G_")) || 
      (ptr == NULL)){

    //Didn't find any dynamicalble string so copy the entire input string to the output
    //Don't copy over the null terminator

    //NETWORK_PRINT("pass the line directly\r\n");
    return sprintf(out_buffer, "%s\r\n", line);
  } 
  out_ptr = out_buffer;
  final_ptr = out_buffer;
  //We have a dynamicable string

  //Need to point to the offset of where the new data is
  length = total_length - strlen(ptr);
  //NETWORK_PRINT("process line with length: %d\r\n", length);
  //Copy over the first part of the string as is
  strncpy(out_ptr, line, length);

  final_line_length += length;
  out_ptr += length;

  ptr += strlen("__SL_G_");

  length = 0;
  //NETWORK_PRINT("Data to process: %s\r\n", ptr);
  switch(ptr[0]){
    case ('S'):
      //NETWORK_PRINT("Status\r\n");
      switch (ptr[2]){
        case ('0'):
          //NETWORK_PRINT("Connection Status\r\n");
          //Device_Status
          if (IS_CONNECTED(nc.status)){
            length += sprintf(out_ptr, "Connected");
          }
          else {
            length += sprintf(out_ptr, "Not Connected");
          }
          break;
        case ('A'):
          //System Up Time
          length += sprintf(out_ptr, "0000");
          break;
        case ('B'):
          //Device_Name_URN
          i = 35;
          sl_NetAppGet( SL_NET_APP_DEVICE_CONFIG_ID,
                        NETAPP_SET_GET_DEV_CONF_OPT_DEVICE_URN,
                        &i,
                        small_buf);
          length += sprintf(out_ptr, "%s", small_buf);
          break;
        case ('C'):
          //Domain_Name              
          i = 35;
          sl_NetAppGet( SL_NET_APP_DEVICE_CONFIG_ID,
                        NETAPP_SET_GET_DEV_CONF_OPT_DOMAIN_NAME,
                        &i,
                        small_buf);
          length += sprintf(out_ptr, "%s", small_buf);
          break;
        case ('D'):
          //Device_mode_role         
          length += sprintf(out_ptr, "%d", nc.mode);
          break;
        case ('E'):
          //Device_Role_Station      
          if (nc.mode == ROLE_STA){
            length += sprintf(out_ptr, "true");
          }
          else {
            length += sprintf(out_ptr, "false");
          }
          break;
        case ('F'):
          //Device_Role_AP           
          if (nc.mode == ROLE_AP){
            length += sprintf(out_ptr, "true");
          }
          else {
            length += sprintf(out_ptr, "false");
          }
          break;
        case ('G'):
          //Device_Role_P2P          
          if (nc.mode == ROLE_P2P){
            length += sprintf(out_ptr, "true");
          }
          else {
            length += sprintf(out_ptr, "false");
          }
          break;
        case ('H'):
          //Device_Name_URN          
          i = 16;
          sl_NetAppGet( SL_NET_APP_DEVICE_CONFIG_ID,
                        NETAPP_SET_GET_DEV_CONF_OPT_DEVICE_URN,
                        &i,
                        small_buf);
          length += sprintf(out_ptr, "%s", small_buf);
          break;
        case ('I'):
          //System_requires_reset    
          length += sprintf(out_ptr, (nc.requires_reset) ? "true":"false");
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
      // Get the device's version-information
      ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
      ucConfigLen = sizeof(ver);
      retval = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, 
                              &ucConfigLen, (unsigned char *)(&ver));
      ASSERT_ON_ERROR(retval);
 
      switch(ptr[2]){
        case ('A'):
          length += sprintf(out_ptr, "%d.%d.%d.%d",
                           ver.NwpVersion[0],
                           ver.NwpVersion[1],
                           ver.NwpVersion[2],
                           ver.NwpVersion[3]);
          break;
        case ('B'):
          length += sprintf(out_ptr, "%d.%d.%d.%d",
                           ver.ChipFwAndPhyVersion.FwVersion[0],
                           ver.ChipFwAndPhyVersion.FwVersion[1],
                           ver.ChipFwAndPhyVersion.FwVersion[2],
                           ver.ChipFwAndPhyVersion.FwVersion[3]);
          break;
        case ('C'):
          length += sprintf(out_ptr, "%d.%d.%d.%d",
                           ver.ChipFwAndPhyVersion.PhyVersion[0],
                           ver.ChipFwAndPhyVersion.PhyVersion[1],
                           ver.ChipFwAndPhyVersion.PhyVersion[2],
                           ver.ChipFwAndPhyVersion.PhyVersion[3]);
          break;
        case ('D'):
          length += sprintf(out_ptr, "%d",
                           ver.RomVersion);
          break;
        default:
          break;
      }
      break;
    case ('N'):
      Network_IF_IpConfigGet(&ip[0], &ip[1], &ip[2], &ip[3]);
      i = sizeof(SlNetCfgIpV4Args_t);
      sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,
             &dhcpIsOn,
             &i,
             (uint8_t *) &ipV4);
      retval = sl_NetAppGet(SL_NET_APP_DHCP_SERVER_ID,
             NETAPP_SET_DHCP_SRV_BASIC_OPT,
             &i,
             (unsigned char *)&dhcpParams);

      switch(ptr[2]){
        case ('A'):
          //STA_IP_Addressg        
          length += sprintf(out_ptr, "%d.%d.%d.%d",
                            SL_IPV4_BYTE(ip[0], 3),
                            SL_IPV4_BYTE(ip[0], 2),
                            SL_IPV4_BYTE(ip[0], 1),
                            SL_IPV4_BYTE(ip[0], 0));
          break;
        case ('B'):
          //STA_Subnet_Maskg       
          length += sprintf(out_ptr, "%d.%d.%d.%d",
                           SL_IPV4_BYTE(ip[1], 3),
                           SL_IPV4_BYTE(ip[1], 2),
                           SL_IPV4_BYTE(ip[1], 1),
                           SL_IPV4_BYTE(ip[1], 0));
          break;
        case ('C'):
          //STA_Default_Gateway    
          length += sprintf(out_ptr, "%d.%d.%d.%d",
                            SL_IPV4_BYTE(ip[2], 3),
                            SL_IPV4_BYTE(ip[2], 2),
                            SL_IPV4_BYTE(ip[2], 1),
                            SL_IPV4_BYTE(ip[2], 0));
          break;
        case ('D'):
          //MAC_Addressg           
          i = SL_MAC_ADDR_LEN;
          sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &i, small_buf);
          length += sprintf(out_ptr, "%X:%X:%X:%X:%X:%X",
                            small_buf[0],
                            small_buf[1],
                            small_buf[2],
                            small_buf[3],
                            small_buf[4],
                            small_buf[5]);
          break;
        case ('E'):
          //STA_DHCP_State
          length += sprintf(out_ptr, "%d", dhcpIsOn);
          break;
        case ('F'):
          //STA_DHCP_Disable_State
          length += sprintf(out_ptr, (dhcpIsOn > 0) ? "false":"true");
          break;
        case ('G'):
          //STA_DHCP_Enable_State
          length += sprintf(out_ptr, (dhcpIsOn > 0) ? "true":"false");
          break;
        case ('H'):
          //STA_DNS_server
          length += sprintf(out_ptr, "%d.%d.%d.%d",
                            SL_IPV4_BYTE(ip[3], 3),
                            SL_IPV4_BYTE(ip[3], 2),
                            SL_IPV4_BYTE(ip[3], 1),
                            SL_IPV4_BYTE(ip[3], 0));
          break;
        case ('I'):
          length += sprintf(out_ptr, "%d.%d.%d.%d",
                            SL_IPV4_BYTE(dhcpParams.ipv4_addr_start,3),
                            SL_IPV4_BYTE(dhcpParams.ipv4_addr_start,2),
                            SL_IPV4_BYTE(dhcpParams.ipv4_addr_start,1),
                            SL_IPV4_BYTE(dhcpParams.ipv4_addr_start,0));
          break;
        case ('J'):
          length += sprintf(out_ptr, "%d.%d.%d.%d",
                            SL_IPV4_BYTE(dhcpParams.ipv4_addr_last,3),
                            SL_IPV4_BYTE(dhcpParams.ipv4_addr_last,2),
                            SL_IPV4_BYTE(dhcpParams.ipv4_addr_last,1),
                            SL_IPV4_BYTE(dhcpParams.ipv4_addr_last,0));
          break;
        case ('K'):
          length += sprintf(out_ptr, "%d", dhcpParams.lease_time);
          break;
        default:
          break;
      }
      break;
    case ('W'):
      //WIFI
      switch(ptr[2]){
        case ('A'):
          //Channel Num In AP Mode
          config_opt = WLAN_AP_OPT_CHANNEL;
          j = 1;
          sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &j, &i);
          //NETWORK_PRINT("Channel Num AP Mode: %d\r\n", i);
          length += sprintf(out_ptr, "%d", i);
          break;
        case ('B'):
          //SSID
          config_opt = WLAN_AP_OPT_SSID;
          j = 32;
          sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &j, small_buf);
          length += sprintf(out_ptr, "%s", small_buf);
          break;
        case ('C'):
          //Security Type
          config_opt = WLAN_AP_OPT_SECURITY_TYPE;
          j = 1;
          sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &j, &i);
          length += sprintf(out_ptr, "%d", i);
          break;
        case ('D'):
          //Security Type Open
          config_opt = WLAN_AP_OPT_SECURITY_TYPE;
          j = 1;
          sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &j, &i);
          length += sprintf(out_ptr, (i == SL_SEC_TYPE_OPEN) ? "true":"false");
          break;
        case ('E'):
          //Security Type WEP
          config_opt = WLAN_AP_OPT_SECURITY_TYPE;
          j = 1;
          sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &j, &i);
          length = sprintf(out_ptr, (i == SL_SEC_TYPE_WEP) ? "true":"false");
          break;
        case ('F'):
          //Security WPA
          config_opt = WLAN_AP_OPT_SECURITY_TYPE;
          j = 1;
          sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt, &j, &i);
          length += sprintf(out_ptr, (i == SL_SEC_TYPE_WPA) ? "true":"false");
          break;
        default:
          break;
      }
      break;
    case ('T'):
      length += sprintf(out_ptr, "Ping Test");
      break;
    case ('P'):
      //Profile
      j = ptr[2] - '1';
      memset(name, 0, 1);
      retval  = sl_WlanProfileGet(j,
                                  name, 
                                  &name_len, 
                                  small_buf, 
                                  &sec_params, 
                                  &sec_ext_params,
                                  &priority);
      switch(ptr[1]){
        case ('N'):
          //Network Name
          length += sprintf(out_ptr, "%s", name);
          break;
        case ('S'):
          //Security
          switch(ptr[2]){
            case ('0'):
              length += sprintf(out_ptr, "Open");
              break;
            case ('1'):
              length += sprintf(out_ptr, "WEP");
              break;
            case ('2'):
              length += sprintf(out_ptr, "WPA");
              break;
            default:
              length += sprintf(out_ptr, "Not Setup");
              break;
          }
          break;
        case ('P'):
          //Priority
          length += sprintf(out_ptr, "%d", priority);
          break;
        default:
          break;
        }//Network Print
      break;
    default:
      //NETWORK_PRINT("Unknown Request: %s", ptr);
      break;
  }

  out_ptr += length;
  length += sprintf(out_ptr, "%s\r\n", &ptr[3]);
  final_line_length += length;

  //NETWORK_PRINT("Finalized Processed Line: %s", final_ptr);
  return final_line_length;
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


