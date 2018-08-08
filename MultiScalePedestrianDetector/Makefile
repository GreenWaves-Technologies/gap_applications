# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

PULP_APP = HoG

TILER_PATH = $(GAP_SDK_HOME)/tools/autotiler
MODEL_SRC = HoGEstimGenerator.c ResizeGenerator.c HoGEstimModel.c

MODEL_GEN = HoGEstimKernelsInit HoGEstimKernels
MODEL_GEN_C = $(addsuffix .c, $(MODEL_GEN))
MODEL_GEN_CLEAN = $(MODEL_GEN_C) $(addsuffix .h, $(MODEL_GEN))

RM=rm -f

PULP_APP_SRCS  = HoGEstim.c HoGEstimManage.c ImgIO.c HoGBasicKernels.c HoGEstimBasicKernels.c pad.c ili9341.c ImageDraw.c
PULP_APP_SRCS += ResizeBasicKernels.c $(MODEL_GEN_C)

PULP_CFLAGS += -O3
PULP_CFLAGS += -mno-memcpy -fno-tree-loop-distribute-patterns -fdata-sections -ffunction-sections
PULP_CFLAGS += -I$(TILER_PATH)/StdTypes/
PULP_LDFLAGS +=  -flto -Wl,--gc-sections

imgOut:
	mkdir $@

all:: model imgOut

# Build the code generator
GenTile:
	gcc -o GenTile -I$(TILER_PATH)/GenTiling -I$(TILER_PATH)/StdTypes/ $(MODEL_SRC) $(TILER_PATH)/GenTiling/LibTile.a

# Run the code generator
$(MODEL_GEN_C): GenTile
	./GenTile

model: $(MODEL_GEN_C)

clean::
	$(RM) GenTile $(MODEL_GEN_CLEAN)
	rm -rf imgOut

.PHONY: model clean

include $(GAP_SDK_HOME)/tools/rules/pulp_rules.mk
