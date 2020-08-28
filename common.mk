CC = gcc
CXX = g++
CUCC = nvcc
AR = ar cr
ECHO = @echo
SHELL = /bin/sh

ifdef VERBOSE
AT =
else
AT = @
endif

TOOLS_DIR ?= tools

CFLAGS = -Wall -Wno-format-truncation -std=gnu99 -D_GNU_SOURCE
CXXFLAGS = -std=c++11 -Wall
CUFLAGS = -m64 -arch=sm_30 -use_fast_math -ccbin $(CXX)
LDFLAGS = $(CFLAGS)

ifeq ($(DEBUG), yes)
CFLAGS += -g -O0 -DLN_DEBUG
CXXFLAGS += -g -O0 -DLN_DEBUG
# CFLAGS += -g -O3
# CXXFLAGS += -g -O3
CUFLAGS += -lineinfo
LDFLAGS += -g -O0
else
CFLAGS += -O2
CXXFLAGS += -O2
CUFLAGS += -O2
LDFLAGS += -O2
endif

INCPATHS = -I/usr/local/include -I. `pkg-config --cflags tensorlight`
LDFLAGS += -L/usr/local/lib -lm `pkg-config --libs tensorlight`

ifeq ($(WITH_CUDA), yes)
CFLAGS += -DLN_CUDA -DTL_CUDA
CXXFLAGS += -DLN_CUDA -DTL_CUDA
CUFLAGS += -DLN_CUDA -DTL_CUDA
CUDA_INSTALL_DIR ?= /usr/local/cuda
INCPATHS += -I$(CUDA_INSTALL_DIR)/include
LDFLAGS += -L$(CUDA_INSTALL_DIR)/lib64 -lcuda -lcudart -lcublas -lcurand -lstdc++
ifeq ($(WITH_CUDNN), yes)
CFLAGS += -DLN_CUDNN
CXXFLAGS += -DLN_CUDNN
CUFLAGS += -DLN_CUDNN
CUDNN_INSTALL_DIR ?= /usr/local/cuda
INCPATHS += -I$(CUDNN_INSTALL_DIR)/include
LDFLAGS += -L$(CUDNN_INSTALL_DIR)/lib -lcudnn
endif
ifeq ($(WITH_TENSORRT), yes)
CFLAGS += -DLN_TENSORRT
CXXFLAGS += -DLN_TENSORRT
CUFLAGS += -DLN_TENSORRT
TENSORRT_INSTALL_DIR ?= /usr
INCPATHS += -I$(TENSORRT_INSTALL_DIR)/include
LDFLAGS += -L$(TENSORRT_INSTALL_DIR)/lib -lnvinfer -lnvinfer_plugin
endif
ifeq ($(WITH_DPU), yes)
CFLAGS += -DLN_DPU
CXXFLAGS += -DLN_DPU
CUFLAGS += -DLN_DPU
endif
endif

CFLAGS += $(INCPATHS)
CXXFLAGS += $(INCPATHS)
CUFLAGS += $(INCPATHS)

define concat
$1$2$3$4$5$6$7$8
endef

# $(call make-depend,source-file,object-file,depend-file)
define make-depend-c
$(AT)$(CC) -MM -MF $(subst .o,.d,$@) -MP -MT $@ $(CFLAGS) $<
endef

define make-depend-cxx
$(AT)$(CXX) -MM -MF $3 -MP -MT $2 $(CXXFLAGS) $1
endef

define make-depend-cu
$(AT)$(CUCC) -M $(CUFLAGS) $1 > $3.$$$$; \
sed 's,.*\.o[ :]*,$2 : ,g' < $3.$$$$ > $3; \
rm -f $3.$$$$
endef

CMD_FILE ?= $(BUILD_DIR)/compile_commands.json

ifeq ($(GEN_CMD_FILE), no)
define compile-c
$(ECHO) CC $@
$(call make-depend-c)
$(AT)$(CC) $(CFLAGS) -c -o $@ $<
endef
else
define compile-c
$(ECHO) GEN_CMD $@
$(AT)$(TOOLS_DIR)/gen_compile_commands.pl -f $(CMD_FILE) `pwd` $< "$(CC) $(CFLAGS) -c -o $@ $<"
endef
endif

ifeq ($(GEN_CMD_FILE), no)
define compile-cxx
$(ECHO) CXX $@
$(call make-depend-cxx)
$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<
endef
else
define compile-cxx
$(ECHO) GEN_CMD $@
$(AT)$(TOOLS_DIR)/gen_compile_commands.pl -f $(CMD_FILE) `pwd` $< "$(CXX) $(CXXFLAGS) -c -o $@ $<"
endef
endif

ifeq ($(GEN_CMD_FILE), no)
define compile-cu
$(ECHO) CUCC $@
$(call make-depend-cu)
$(AT)$(CUCC) $(CUFLAGS) -c -o $@ $<
endef
else
define compile-cu
$(ECHO) GEN_CMD $@
$(AT)$(TOOLS_DIR)/gen_compile_commands.pl -f $(CMD_FILE) `pwd` $< "$(CUCC) $(CUFLAGS) -c -o $@ $<"
endef
endif
