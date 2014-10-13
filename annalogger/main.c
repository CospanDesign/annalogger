//*****************************************************************************
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


//*****************************************************************************
//
// Application Name     - Getting started with Annalogger
// Application Overview - 
//                        
//                        
// Application Details  -
//
//*****************************************************************************


//****************************************************************************
//
//! \addtogroup getting_started_ap
//! @{
//
//****************************************************************************

#include <stdlib.h>
#include <string.h>

// Simplelink includes
#include "annalogger.h"
#include "simplelink.h"
#include "device.h"
#include "gpio_if.h"

#ifdef USE_LAUNCHPAD
#include "launchpad.h"
#endif

// driverlib includes 
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"

// free_rtos/ti-rtos includes 
#include "osi.h"

// common interface includes
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "pinmux.h"


#include "tasks/uart/uart_task.h"
#include "tasks/sd/sd_task.h"
#include "tasks/network/network_task.h"
#include "tasks/sensor/sensor_task.h"
#include "tasks/master/master_task.h"


#define APP_NAME                "Annalogger"
#define APPLICATION_VERSION     "0.0.1"
#define OSI_STACK_SIZE          2048


extern void (* const g_pfnVectors[])(void);

al_queues_t al_queues;

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************


#ifdef USE_FREERTOS
//*****************************************************************************
// FreeRTOS User Hook Functions enabled in FreeRTOSConfig.h
//*****************************************************************************
void vAssertCalled( const char *pcFile, unsigned long ulLine )
{
    //Handle Assert here
    Report("%s: Entered\r\n", __func__);
    while(1)
    {
    }
}

void vApplicationIdleHook( void)
{
    //Handle Idle Hook for Profiling, Power Management etc
}

void vApplicationMallocFailedHook()
{
    Report("%s: Entered\r\n", __func__);
    //Handle Memory Allocation Errors
    while(1)
    {
    }
}

void vApplicationStackOverflowHook(OsiTaskHandle *pxTask, 
                                   signed char *pcTaskName)
{
    //Handle FreeRTOS Stack Overflow
    Report ("STACK OVERFLOW!!!\r\n");
    while(1)
    {
    }
}
#endif //USE_FREERTOS
OsiReturnVal_e generic_enqueue(  OsiMsgQ_t *queue,  
                                uint8_t    event_type,
                                int32_t    message_param,
                                void      *msg,
                                OsiTime_t timeout){
  al_msg_t m;
  
  m.event_type = event_type;
  m.msg_param = message_param;
  m.msg = msg;

  return osi_MsgQWrite(queue, (void *) &m, timeout);    
}


OsiReturnVal_e master_event(uint8_t    event_type,
                            int32_t    message_param,
                            void      *msg,
                            OsiTime_t timeout){
  return generic_enqueue(&al_queues.master_event_queue,
                         event_type,
                         message_param,
                         msg,
                         timeout);
}

OsiReturnVal_e network_event(uint8_t  event_type,
                            int32_t    message_param,
                            void      *msg,
                            OsiTime_t timeout){
  return generic_enqueue(&al_queues.network_queue,
                         event_type,
                         message_param,
                         msg,
                         timeout);
}

OsiReturnVal_e uart_event(uint8_t  event_type,
                            int32_t    message_param,
                            void      *msg,
                            OsiTime_t timeout){
  return generic_enqueue(&al_queues.uart_queue,
                         event_type,
                         message_param,
                         msg,
                         timeout);
}
OsiReturnVal_e sd_event(uint8_t  event_type,
                            int32_t    message_param,
                            void      *msg,
                            OsiTime_t timeout){
  return generic_enqueue(&al_queues.sd_queue,
                         event_type,
                         message_param,
                         msg,
                         timeout);
}





static void
DisplayBanner(char * AppName)
{
    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t       Cospan Design %s       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}

static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

int setup_queues(){
  long lRetVal = -1;

  lRetVal = osi_MsgQCreate(  &al_queues.master_event_queue,
                            "Master Event Queue",
                            sizeof(al_msg_t),
                            MASTER_QUEUE_SIZE);
  if(lRetVal < 0){
      ERR_PRINT(lRetVal);
      LOOP_FOREVER();
  }
  lRetVal = osi_MsgQCreate( &al_queues.sensor_queue,
                            "Sensor Queue",
                            sizeof(al_msg_t), 
                            SENSOR_QUEUE_SIZE);
  if(lRetVal < 0){
      ERR_PRINT(lRetVal);
      LOOP_FOREVER();
  }
  lRetVal = osi_MsgQCreate(  &al_queues.network_queue,
                            "Network Queue",
                            sizeof(al_msg_t), 
                            NETWORK_QUEUE_SIZE);
  if(lRetVal < 0){
      ERR_PRINT(lRetVal);
      LOOP_FOREVER();
  }
  lRetVal = osi_MsgQCreate( &al_queues.sd_queue,
                            "SD FatFS Queue",
                            sizeof(al_msg_t),
                            SD_QUEUE_SIZE);
  if(lRetVal < 0){
      ERR_PRINT(lRetVal);
      LOOP_FOREVER();
  }
  lRetVal = osi_MsgQCreate(  &al_queues.uart_queue,
                            "UART Queue",
                            sizeof(al_msg_t),
                            UART_QUEUE_SIZE);
  if(lRetVal < 0){
      ERR_PRINT(lRetVal);
      LOOP_FOREVER();
  }
}

//*****************************************************************************
//                            MAIN FUNCTION
//*****************************************************************************
void main()
{

    long lRetVal = -1;
  
  
    //
    // Board Initialization
    //
    BoardInit();
    
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();

    
#ifndef NOTERM
    //
    // Configuring UART
    //
    InitTerm();
#endif

#ifdef USE_LAUNCHPAD
  launchpad_init();
#else
  Report ("Error: Board is not initialized, Initialize Board!1!\r\n");
  LOOP_FOREVER();
#endif

    //
    // Display banner
    //
    DisplayBanner(APP_NAME);

    //
    // Start the SimpleLink Host
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    
    setup_queues();  

    //
    // Setup the main task  
    //
    lRetVal = osi_TaskCreate( master_task_entry,                    \
                              (const signed char *) "Master task",  \
                              OSI_STACK_SIZE,                        \
                              (void *) &al_queues,
                              8,
                              NULL);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    // Setup the UART task
    lRetVal = osi_TaskCreate( uart_task_entry,                    \
                              (const signed char *) "UART task",  \
                              OSI_STACK_SIZE,                      \
                              (void *) &al_queues,
                              2,
                              NULL);
 
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    // Setup the network task
    lRetVal = osi_TaskCreate( network_task_entry,                    \
                              (const signed char *) "Network task", \
                              OSI_STACK_SIZE,                        \
                              (void *) &al_queues,
                              7,
                              NULL);
 
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    // Setup the SD Card FatFS task
    lRetVal = osi_TaskCreate( sd_task_entry,                              \
                              (const signed char *) "SD Card FatFS task", \
                              OSI_STACK_SIZE,                              \
                              (void *) &al_queues,
                              6,
                              NULL);
 
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    // Setup the sensor task
    lRetVal = osi_TaskCreate( sensor_task_entry,                    \
                              (const signed char *) "Sensor task",  \
                              OSI_STACK_SIZE,                        \
                              (void *) &al_queues,
                              5,
                              NULL);
 
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

  


    osi_start();
}

