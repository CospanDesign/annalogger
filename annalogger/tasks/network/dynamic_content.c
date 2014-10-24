#include "network.h"
#include "HttpDynamic.h"
#include "HttpRequest.h"
#include "HttpResponse.h"



#define DYNAMIC_BUFFER_SIZE 4096

char filename[40]; 				//String storing filename
char RESOURCE_PARAM_CONFIG[]  = "param_config.html";

static char * ERROR_STR = "Error processing request";

extern network_controller_t nc;

void setup_dynamic_content(void){
  SetResources(GET, &RESOURCE_PARAM_CONFIG[0], &exe_param_config);
}


int exe_param_config(struct HttpRequest* request){

	struct HttpBlob content;
  struct HttpBlob content_type;
  struct HttpBlob location;
  char *buffer;
  long file_handle;
  uint32_t total_length;
  long retval;
	SlFsFileInfo_t file_info;
  bool response_header;
  uint16_t length;
  uint16_t offset;

  //Initialize values
  length = 0;
  offset = 0;

  content_type.pData = NULL;
  content_type.uLength = 0;

  location.pData = NULL;
  location.uLength = 0;
  memset(filename, '\0', 40);
  strcpy(filename, "www/");
  strncat(filename, RESOURCE_PARAM_CONFIG, strlen(RESOURCE_PARAM_CONFIG));


  //Allocate some space for the outgoing page
  buffer = (char *) calloc(DYNAMIC_BUFFER_SIZE, sizeof(char));

  NETWORK_PRINT("Dynamic Content: Buffer pointer: %p\r\n", buffer);
  if (buffer == NULL){
    NETWORK_PRINT("Dynamic Content: Allocation failed!\r\n");
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
  retval = sl_FsRead(file_handle, 0, buffer, total_length);
  if (retval < 0){
    NETWORK_PRINT("Dynamic Content: Failed to read file: %s\r\n", filename);
    retval = 0; 
    goto fileclose;
  }

  //Send the header
  retval = HttpResponse_Headers(request->uConnection, HTTP_STATUS_OK, 0, total_length, content_type, location);
  if (retval < 0){
    NETWORK_PRINT("Dynamic Content: Failed to send Response Header\r\n");
    retval = 0; 
    goto fileclose;
  }


  //Create a response
  content.pData = buffer;
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
  free(buffer);
  

  //if not connected
  return retval;
}
