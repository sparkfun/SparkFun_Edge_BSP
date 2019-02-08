#******************************************************************************
#
# Makefile - Rules for building the libraries, examples and docs.
#
# Copyright (c) 2019, Ambiq Micro
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from this
# software without specific prior written permission.
# 
# Third party software included in this distribution is subject to the
# additional license terms as defined in the /docs/licenses directory.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# This is part of revision v2.0.0 of the AmbiqSuite Development Package.
#
# Modifications by Owen Lyke, SparkFun Electronics, February 2019
#
#******************************************************************************
TARGET := SparkFun_Edge_Project_Template# this name is the name of the resulting binary file
COMPILERNAME := gcc
PROJECT := SparkFun_Edge_Project_Template
CONFIG := bin

SHELL:=/bin/bash

# Here are some user modifiable settings. Note that leading spaces are used to separate strings when appending to a variable
COM_PORT			 = COM4
SDKPATH				 = # Note that if you copy/paste a windows file path here you need to change backslashes to forward slashes
BOARDPATH			 = ${SDKPATH}/boards/SparkFun_Edge_AmbiqSDK_BSP

USER_INCLUDE_DIRS	 = -I../src/inc1
USER_INCLUDE_DIRS	+= -I../src/inc2

USER_SOURCEDIRS		 = ../src
USER_SOURCEDIRS		+= ../src/inc1
USER_SOURCEDIRS		+= ../src/inc2

USER_MAIN_SRC		 = main.c
USER_SOURCE_FILES	 = inc1.c
USER_SOURCE_FILES	+= inc2.c


#### Bootloader Configuration
SERIAL_PORT ?= ${COM_PORT} # This can be overridden in another file

#### Setup ####

TOOLCHAIN ?= arm-none-eabi
PART = apollo3
CPU = cortex-m4
FPU = fpv4-sp-d16
# Default to FPU hardware calling convention.  However, some customers and/or
# applications may need the software calling convention.
#FABI = softfp
FABI = hard

LINKER_FILE := ./template_link.ld
STARTUP_FILE := ./startup_$(COMPILERNAME).c

#### Required Executables ####
CC = $(TOOLCHAIN)-gcc
GCC = $(TOOLCHAIN)-gcc
CPP = $(TOOLCHAIN)-cpp
LD = $(TOOLCHAIN)-ld
CP = $(TOOLCHAIN)-objcopy
OD = $(TOOLCHAIN)-objdump
RD = $(TOOLCHAIN)-readelf
AR = $(TOOLCHAIN)-ar
SIZE = $(TOOLCHAIN)-size
RM = $(shell which rm 2>/dev/null)

EXECUTABLES = CC LD CP OD AR RD SIZE GCC
K := $(foreach exec,$(EXECUTABLES),\
        $(if $(shell which $($(exec)) 2>/dev/null),,\
        $(info $(exec) not found on PATH ($($(exec))).)$(exec)))
$(if $(strip $(value K)),$(info Required Program(s) $(strip $(value K)) not found))

ifneq ($(strip $(value K)),)
all clean:
	$(info Tools $(TOOLCHAIN)-$(COMPILERNAME) not installed.)
	$(RM) -rf bin
else

DEFINES = -DPART_$(PART)
DEFINES+= -DAM_PACKAGE_BGA
DEFINES+= -DAM_PART_APOLLO3

# Includes from the SDK
# INCLUDES+= -I../../../../.. # This one doesn't make much sense to me
INCLUDES = -I${SDKPATH}/CMSIS/AmbiqMicro/Include
INCLUDES+= -I${SDKPATH}/mcu/apollo3
INCLUDES+= -I${SDKPATH}/CMSIS/ARM/Include
INCLUDES+= -I${SDKPATH}/utils
INCLUDES+= -I${SDKPATH}/devices

# Includes for the board you've chosen
INCLUDES+= -I../src # This is where the source for the particular project is
INCLUDES+= -I${BOARDPATH}/bsp

INCLUDES+= ${USER_INCLUDE_DIRS}

VPATH = ${SDKPATH}/devices
VPATH+= ${SDKPATH}/utils
VPATH+= ${USER_SOURCEDIRS}

SRC = ${USER_MAIN_SRC}
SRC += am_devices_led.c
SRC += am_util_delay.c
SRC += am_util_faultisr.c
SRC += am_util_stdio.c
SRC += startup_gcc.c
SRC += ${USER_SOURCE_FILES}

CSRC = $(filter %.c,$(SRC))
ASRC = $(filter %.s,$(SRC))

OBJS = $(CSRC:%.c=$(CONFIG)/%.o)
OBJS+= $(ASRC:%.s=$(CONFIG)/%.o)

DEPS = $(CSRC:%.c=$(CONFIG)/%.d)
DEPS+= $(ASRC:%.s=$(CONFIG)/%.d)

LIBS = ${BOARDPATH}/bsp/gcc/bin/libam_bsp.a
LIBS += ${SDKPATH}/mcu/apollo3/hal/gcc/bin/libam_hal.a

CFLAGS = -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
CFLAGS+= -ffunction-sections -fdata-sections
CFLAGS+= -MMD -MP -std=c99 -Wall -g
CFLAGS+= -O3
CFLAGS+= $(DEFINES)
CFLAGS+= $(INCLUDES)
CFLAGS+= 

LFLAGS = -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
LFLAGS+= -nostartfiles -static
LFLAGS+= -Wl,--gc-sections,--entry,Reset_Handler,-Map,$(CONFIG)/$(TARGET).map
LFLAGS+= -Wl,--start-group -lm -lc -lgcc $(LIBS) -Wl,--end-group
LFLAGS+= 

# Additional user specified CFLAGS
CFLAGS+=$(EXTRA_CFLAGS)

CPFLAGS = -Obinary

ODFLAGS = -S

#### Rules ####
all: directories $(CONFIG)/$(TARGET).bin

directories: $(CONFIG)

$(CONFIG):
	@mkdir -p $@

$(CONFIG)/%.o: %.c $(CONFIG)/%.d
	@echo " Compiling $(COMPILERNAME) $<" ;\
	$(CC) -c $(CFLAGS) $< -o $@

$(CONFIG)/%.o: %.s $(CONFIG)/%.d
	@echo " Assembling $(COMPILERNAME) $<" ;\
	$(CC) -c $(CFLAGS) $< -o $@

$(CONFIG)/$(TARGET).axf: $(OBJS) $(LIBS)
	@echo " Linking $(COMPILERNAME) $@" ;\
	$(CC) -Wl,-T,$(LINKER_FILE) -o $@ $(OBJS) $(LFLAGS)

$(CONFIG)/$(TARGET).bin: $(CONFIG)/$(TARGET).axf
	@echo " Copying $(COMPILERNAME) $@..." ;\
	$(CP) $(CPFLAGS) $< $@ ;\
	$(OD) $(ODFLAGS) $< > $(CONFIG)/$(TARGET).lst

clean:
	@echo "Cleaning..." ;\
	$(RM) -f $(OBJS) $(DEPS) \
	    $(CONFIG)/$(TARGET).bin $(CONFIG)/$(TARGET).axf \
	    $(CONFIG)/$(TARGET).lst $(CONFIG)/$(TARGET).map

$(CONFIG)/%.d: ;

${BOARDPATH}/bsp/gcc/bin/libam_bsp.a:
	$(MAKE) -C ${BOARDPATH}/bsp

${SDKPATH}/mcu/apollo3/hal/gcc/bin/libam_hal.a:
	$(MAKE) -C ${SDKPATH}/mcu/apollo3/hal

bootload: all
	${SDKPATH}/tools/apollo3_scripts/create_cust_image_blob.py --bin $(CONFIG)/$(TARGET).bin --load-address 0xC000 --magic-num 0xCB -o $(CONFIG)/main_nonsecure_ota --version 0x0
	${SDKPATH}/tools/apollo3_scripts/create_cust_wireupdate_blob.py --load-address 0x20000 --bin $(CONFIG)/main_nonsecure_ota.bin -i 6 -o $(CONFIG)/main_nonsecure_wire --options 0x1
	${BOARDPATH}/bsp/tools/uart_wired_update_sparkfun.py -b 115200 $(SERIAL_PORT) -r 1 -f $(CONFIG)/main_nonsecure_wire.bin -i 6

# Automatically include any generated dependencies
-include $(DEPS)
endif
.PHONY: all clean directories