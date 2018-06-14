# Copyright (c) 2017 GreenWaves Technologies SAS
# All rights reserved.
# # Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of GreenWaves Technologies SAS nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.


TILER_PATH = $(GAP_SDK_HOME)/tools/autotiler
GEN_PATH=$(TILER_PATH)/autotiler_generator
MODEL_SRC = ./CNN_kernels/KwsGenerator.c
PULP_APP = main

PULP_APP_SRCS =   main.c  ./libs/MFCC.c ./libs/MFCC_Dump.c ./CnnKernels_hwce.c CNN_kernels/CnnKernels.c  ./CNN_kernels/CnnKernels_tileddense.c  ./CnnKernelsInit_hwce.c ./libs/CNN_BasicKernels.c ./libs/CNN_HwCE.c ./libs/Cluster_FLL_optimal_settings.c

#To activate Hardware CE SET -DRT_HAS_HWCE=1
#USE_HARDWARE_CE = -DRT_HAS_HWCE=1

PULP_CFLAGS += -O3 $(USE_HARDWARE_CE)

PULP_CFLAGS  +=  -O3 -Wno-unused-but-set-variable  -Wno-overflow -Wno-sign-compare -DW_HALFCHAR -DTILED -DHWCE_ACTIVE  -DDOMFCC # -DDUMP_DENSE  -DDOMFCC -DPRINTINT -DPRINTFEAT #-DDUMP_LAYER #-DREF -DPRINTDEB_TILE
PULP_CFLAGS  += -I./include  -I./CNN_kernels -I./include_libs -I./weights_bias -lm  #-I$(GEN_PATH)/CnnStdModel

#override CONFIG_OPT += camera=himax system/camera/image-stream=$(CURDIR)/IMG.pgm

all:: model

# Build the code generator
GenTile:
	gcc $(USE_HARDWARE_CE)  -o GenTile -I$(TILER_PATH)/GenTiling -I$(TILER_PATH)/StdTypes/ -I./include  $(MODEL_SRC) ./CNN_kernels/CNN_Generator.c $(TILER_PATH)/GenTiling/LibTile.a

# Run the code generator
CnnKernels_hwce.c CnnKernelsInit_hwce.c: GenTile
	./GenTile

cleanmodel: CnnKernels_hwce.c CnnKernelsInit_hwce.c

cleanmodel:
	$(RM) GenTile CnnKernels_hwce.* CnnKernelsInit_hwce.*


.PHONY: model

#include $(PULP_SDK_HOME)/install/rules/pulp_rt.mk
include $(GAP_SDK_HOME)/tools/rules/pulp_rules.mk
