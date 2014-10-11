
#include "sd_task.h"
#include "sd.h"
#include "hw_types.h"
#include "prcm.h"
#include "sdhost.h"
#include "diskio.h"
#include "ff.h"
#include "rom_map.h"
#include "rom.h"
#include "hw_memmap.h"



#define USERFILE        "userfile.txt"
#define SYSFILE         "sysfile.txt"
#define SYSTEXT         "The quick brown fox jumps over the lazy dog"


typedef enum _sd_state_t {
	ERROR,
	RESET,
	INIT,
	READY
} sd_state_t;

//****************************************************************************
//
//! List the files in the given directory
//!
//! \param pointer to directory
//!
//! \return None.
//
//****************************************************************************
static void
ListDirectory(DIR *dir)
{
  FILINFO fno;
  FRESULT res;
  unsigned long ulSize;
  bool bIsInKB;

  for(;;)
  {
    res = f_readdir(dir, &fno);           // Read a directory item
    if (res != FR_OK || fno.fname[0] == 0)
    {
    break;                                // Break on error or end of dir
    }
    ulSize = fno.fsize;
    bIsInKB = false;
    if(ulSize > 1024)
    {
      ulSize = ulSize/1024;
      bIsInKB = true;
    }
    Report("->%-15s%5d %-2s    %-5s\n\r",fno.fname,ulSize,\
            (bIsInKB == true)?"KB":"B",(fno.fattrib&AM_DIR)?"Dir":"File");
  }

}


void sd_task_entry(void *pvParameters){ 
  FIL fp;
  FATFS fs;
  FRESULT res;
  DIR dir;
  WORD Size;
	OsiMsgQ_t seq;
	char buffer[100];
	sd_controller_t sc;
	al_msg_t msg;
	sd_state_t state;

	long retval;


	sc.al_queues = (al_queues_t *) pvParameters;
	seq = sc.al_queues->network_queue;
	
	state = RESET;

	while (1){
		switch(state){
			case (ERROR):
				osi_Sleep(1000);
				SD_PRINT("%s: ERROR\r\n", __func__);
				break;
			case (RESET):
				SD_PRINT("%s: RESET\r\n", __func__);
				state = INIT;
				break;
			case (INIT):
			  //Reset MMCHS
  			MAP_PRCMPeripheralReset(PRCM_SDHOST);

  			// Configure MMCHS
  			MAP_SDHostInit(SDHOST_BASE);

  			// Configure card clock
  			MAP_SDHostSetExpClk(SDHOST_BASE,
  			                    MAP_PRCMPeripheralClockGet(PRCM_SDHOST),15000000);
  			//MAP_SDHostSetExpClk(SDHOST_BASE,
        //              MAP_PRCMPeripheralClockGet(PRCM_SDHOST),1000000);

  			SD_PRINT("%s: Mounting SD Card\r\n", __func__);
        f_mount(0,&fs);
   
        res = f_opendir(&dir,"/");
        if( res == FR_OK)
        {
            SD_PRINT("Opening root directory.................... [ok]\n\n\r");
            SD_PRINT("/\n\r");
            ListDirectory(&dir);
        }
        else
        {
            SD_PRINT("Opening root directory.................... [Failed]\n\n\r");
        }
    
				/*
        SD_PRINT("\n\rReading user file...\n\r");
        res = f_open(&fp,USERFILE,FA_READ);
        if(res == FR_OK)
        {
            f_read(&fp,buffer,100,&Size);
            SD_PRINT("Read : %d Bytes\n\n\r",Size);
            SD_PRINT("%s",buffer);
            f_close(&fp);
        }
        else
        {
            SD_PRINT("Failed to open %s\n\r",USERFILE);
        }
    
        SD_PRINT("\n\n\rWriting system file...\n\r");
        res = f_open(&fp,SYSFILE,FA_CREATE_ALWAYS|FA_WRITE);
        if(res == FR_OK)
        {
            f_write(&fp,SYSTEXT,sizeof(SYSTEXT),&Size);
            SD_PRINT("Wrote : %d Bytes",Size);
            res = f_close(&fp);
        }
        else
        {
            SD_PRINT("Failed to create a new file\n\r");
        }
				*/
				state = READY;

				break;
			case (READY):
  			SD_PRINT("%s: READY\r\n", __func__);
				retval = master_event(MASTER_EVENT_SD_IS_READY, 0, NULL, 10);
				retval = osi_MsgQRead(&seq, &msg, OSI_WAIT_FOREVER);
				if (retval == OSI_OPERATION_FAILED){
					SD_PRINT("%s: Error durring queue read\r\n");
					state = ERROR;
					continue;
				}
				SD_PRINT("%s: Received a message on SD Queue\r\n");
				//Write data to SD Card
				//Read data from SD Card and send it to ???
				
				
				break;
			default:
				state = ERROR;
				break;
		}
	}
}
