#******************************************************************************
#
# Makefile - Rules for building the annalogger application.
#
#
#  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
#
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#*****************************************************************************

#
# The base directory.
#
ROOT=../../..
TARGET=${BINDIR}/annalogger.axf
HTTP_SERVER_PATH=netapps/http/server

#
# Include the common make definitions.
#
include ${ROOT}/tools/gcc_scripts/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=..
VPATH+=../../drivers
VPATH+=../../common
VPATH+=${ROOT}/projects/${HTTP_SERVER_PATH}


#Uncomment the line below to use the launchpad platform instead of annalogger board
LAUNCHPAD=1

#
# Additional Compiler Flags
#


ifdef LAUNCHPAD
CFLAGS+=-DUSE_LAUNCHPAD
endif

CFLAGS+=-DSL_PLATFORM_MULTI_THREADED
CFLAGS+=-DUSE_FREERTOS

#
# Where to find header files that do not live in the source directory.
#
IPATH=..
IPATH+=../..
IPATH+=../tasks/sensor
IPATH+=../tasks/sd
IPATH+=../tasks/network
IPATH+=../tasks/http
IPATH+=../../common
IPATH+=../../../inc
IPATH+=../../../oslib
IPATH+=../../../driverlib
IPATH+=../../../simplelink
IPATH+=../../../simplelink/source
IPATH+=../../../simplelink/include
IPATH+=../../../third_party/fatfs
IPATH+=../../../third_party/FreeRTOS
IPATH+=../../../third_party/FreeRTOS/source
IPATH+=../../../third_party/FreeRTOS/source/portable/GCC/ARM_CM4
IPATH+=../../../third_party/FreeRTOS/source/include
IPATH+=../../${HTTP_SERVER_PATH}

#
# The default rule, which causes the driver library to be built.
#
all: ${OBJDIR} ${BINDIR}
all: ${TARGET}

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${OBJDIR} ${wildcard *~}
	@rm -rf ${BINDIR} ${wildcard *~}


#
# The rule to create the target directories.
#
${OBJDIR}:
	@mkdir -p ${OBJDIR}
	@mkdir -p ${OBJDIR}/${HTTP_SERVER_PATH}
	@mkdir -p ${OBJDIR}/tasks/uart
	@mkdir -p ${OBJDIR}/tasks/network
	@mkdir -p ${OBJDIR}/tasks/sd
	@mkdir -p ${OBJDIR}/tasks/sensor
	@mkdir -p ${OBJDIR}/tasks/master
	@mkdir -p ${OBJDIR}/tasks/http


${BINDIR}:
	@mkdir -p ${BINDIR}

debug:
	@cgdb -d arm-none-eabi-gdb.exe -x gdbinit ${BINDIR}/annalogger.axf

#
# Rules for building the annalogger example.
#

ifdef LAUNCHPAD
${TARGET}: ${OBJDIR}/launchpad.o
${TARGET}: ${OBJDIR}/launchpad_sensor.o
${TARGET}: ${OBJDIR}/bma222_sensor.o
${TARGET}: ${OBJDIR}/bma222drv.o
${TARGET}: ${OBJDIR}/tmp006drv.o
endif

${TARGET}: ${OBJDIR}/strlib.o
${TARGET}: ${OBJDIR}/base64.o
${TARGET}: ${OBJDIR}/HttpAuth.o
${TARGET}: ${OBJDIR}/HttpDebug.o
${TARGET}: ${OBJDIR}/HttpDynamic.o
${TARGET}: ${OBJDIR}/HttpStatic.o
${TARGET}: ${OBJDIR}/HttpString.o
${TARGET}: ${OBJDIR}/sha1.o
${TARGET}: ${OBJDIR}/WebSockHandler.o
${TARGET}: ${OBJDIR}/HttpSocket.o
${TARGET}: ${OBJDIR}/HttpCore.o

${TARGET}: ${OBJDIR}/sensor.o

${TARGET}: ${OBJDIR}/tasks/uart/uart_task.o
${TARGET}: ${OBJDIR}/tasks/sd/sd_task.o
${TARGET}: ${OBJDIR}/tasks/sd/ff.o
${TARGET}: ${OBJDIR}/tasks/sd/diskio.o

#${TARGET}: ${OBJDIR}/network_if.o
${TARGET}: ${OBJDIR}/tasks/network/ws_list.o
${TARGET}: ${OBJDIR}/tasks/network/network.o
${TARGET}: ${OBJDIR}/tasks/network/network_task.o
${TARGET}: ${OBJDIR}/tasks/network/network_ap.o
${TARGET}: ${OBJDIR}/tasks/network/network_ws.o
${TARGET}: ${OBJDIR}/tasks/network/dynamic_content.o
${TARGET}: ${OBJDIR}/tasks/sensor/sensor_task.o
${TARGET}: ${OBJDIR}/tasks/master/master_task.o
${TARGET}: ${OBJDIR}/tasks/http/http_task.o

${TARGET}: ${OBJDIR}/pinmux.o
${TARGET}: ${OBJDIR}/i2c_if.o
${TARGET}: ${OBJDIR}/uart_if.o
${TARGET}: ${OBJDIR}/gpio_if.o
${TARGET}: ${OBJDIR}/main.o

${TARGET}: ${OBJDIR}/startup_${COMPILER}.o
${TARGET}: ${ROOT}/simplelink/${COMPILER}/${BINDIR}/libsimplelink.a
${TARGET}: ${ROOT}/driverlib/${COMPILER}/${BINDIR}/libdriver.a
${TARGET}: ${ROOT}/oslib/${COMPILER}/${BINDIR}/FreeRTOS.a
#${TARGET}: ${ROOT}/netapps/http/server/gcc/exe/libwebserver.a
SCATTERgcc_annalogger=annalogger.ld
ENTRY_annalogger=ResetISR


#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif


