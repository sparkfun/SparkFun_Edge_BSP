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
#******************************************************************************
TARGET := tensorflow_demo
COMPILERNAME := gcc
PROJECT := ${TARGET}_gcc
CONFIG := bin

SHELL:=/bin/bash


#### Bootloader Configuration
SERIAL_PORT ?= COM4# Or perhaps a Unix-like machine might put '/dev/cu.usbserial-1410' 

#### Binary Location
BINARY = ../binaries/micro_speech.bin

#### Setup ####
TOOLCHAIN ?= arm-none-eabi

#### Required Executables ####
RM = $(shell which rm 2>/dev/null)

#### Rules ####
all: directories $(CONFIG)/$(TARGET).bin

directories: $(CONFIG)


$(CONFIG):
	@echo " Making directory $@"
	@mkdir -p $@

$(CONFIG)/$(TARGET).bin:
	@echo " copying binary to $@"
	@cp $(BINARY) $@ ;\

clean:
	@echo "Cleaning..." ;\
	$(RM) -f $(CONFIG)/$(TARGET).bin


bootload: all
	../../../../../tools/apollo3_scripts/create_cust_image_blob.py --bin $(CONFIG)/$(TARGET).bin --load-address 0xC000 --magic-num 0xCB -o $(CONFIG)/main_nonsecure_ota --version 0x0
	../../../../../tools/apollo3_scripts/create_cust_wireupdate_blob.py --load-address 0x20000 --bin $(CONFIG)/main_nonsecure_ota.bin -i 6 -o $(CONFIG)/main_nonsecure_wire --options 0x1
	../../../bsp/tools/uart_wired_update_sparkfun.py -b 921600 $(SERIAL_PORT) -r 1 -f $(CONFIG)/main_nonsecure_wire.bin -i 6

# Automatically include any generated dependencies
-include $(DEPS)

.PHONY: all clean directories
