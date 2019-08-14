# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

pulpChip = GAP
PULP_APP = faceDetect
TILER_PATH = $(GAP_SDK_HOME)/tools/autotiler
TILER_LIB = $(TILER_PATH)/lib/libtile.a
TILER_INC = $(TILER_PATH)/include
MODEL_SRC = FaceDetGenerator.c FaceDetModel.c

MODEL_GEN = FaceDetKernels FaceDetKernelsInit
MODEL_GEN_C = $(addsuffix .c, $(MODEL_GEN))
MODEL_GEN_CLEAN = $(MODEL_GEN_C) $(addsuffix .h, $(MODEL_GEN))
pulpFc=1
RM=rm -f

PULP_APP_SRCS = main.c $(MODEL_GEN_C) FaceDetBasicKernels.c ImageDraw.c ili9341.c pad.c ImgIO.c

PULP_CFLAGS += -O3
PULP_CFLAGS += -mno-memcpy -fno-tree-loop-distribute-patterns -fdata-sections -ffunction-sections -Wimplicit-fallthrough=0
PULP_CFLAGS += -Wno-implicit-function-declaration -Wno-unused-but-set-variable -Wno-maybe-uninitialized
PULP_CFLAGS += -I$(TILER_INC)

# The generated code outputs a maybe-uninitialized error which is rather difficult to suppress
# in a clean way.
PULP_CFLAGS += -Wall -Werror -Wno-maybe-uninitialized -Wno-unused-but-set-variable

#PLPBRIDGE_FLAGS+= -f -hyper


PLPBRIDGE_FLAGS+= -fileIO 5
# The double colon allows us to force this to occur before the imported all target
# Link model generation to all step
all:: model

# Build the code generator
GenTile:
	gcc -o GenTile -I$(TILER_INC) $(MODEL_SRC) $(TILER_LIB)

# Run the code generator
$(MODEL_GEN_C): GenTile
	./GenTile

model: $(MODEL_GEN_C)

clean::
	$(RM) GenTile $(MODEL_GEN_CLEAN)

.PHONY: model clean

include $(GAP_SDK_HOME)/tools/rules/pulp_rules.mk



