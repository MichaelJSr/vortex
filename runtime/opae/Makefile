ROOT_DIR := $(realpath ../..)
include ../common.mk

TARGET ?= opaesim

DESTDIR ?= $(CURDIR)/..

SYN_DIR := $(HW_DIR)/syn/altera/opae

SRC_DIR := $(VORTEX_HOME)/runtime/opae

CXXFLAGS += -std=c++17 -Wall -Wextra -pedantic -Wfatal-errors
CXXFLAGS += -I$(INC_DIR) -I$(COMMON_DIR) -I$(ROOT_DIR)/hw -I$(DESTDIR) -I$(SIM_DIR)/common
CXXFLAGS += -DXLEN_$(XLEN)

# Position independent code
CXXFLAGS += -fPIC

# Add external configuration
CXXFLAGS += $(CONFIGS)

LDFLAGS += -shared -luuid -ldl -pthread

SRCS = $(SRC_DIR)/vortex.cpp $(SRC_DIR)/driver.cpp

# set up target types
ifeq ($(TARGET), opaesim)
	BUILD_DEPS = $(DESTDIR)/libopae-c-sim.so
	CXXFLAGS += -DOPAESIM -I$(SIM_DIR)/opaesim
else
	BUILD_DEPS = $(ROOT_DIR)/hw/syn/altera/opae/vortex_afu.h
	CXXFLAGS += -I$(SYN_DIR) -I$(ROOT_DIR)/hw/syn/altera/opae
endif

# Debugging
ifdef DEBUG
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O2 -DNDEBUG
endif

# Enable scope logic analyzer
ifdef SCOPE
	CXXFLAGS += -DSCOPE
	SRCS += $(COMMON_DIR)/scope.cpp
endif

PROJECT := libvortex-opae.so

all: $(DESTDIR)/$(PROJECT)

$(ROOT_DIR)/hw/syn/altera/opae/vortex_afu.h:
	$(MAKE) -C $(ROOT_DIR)/hw/syn/altera/opae swconfig

driver: $(DESTDIR)/libopae-c-sim.so

$(DESTDIR)/libopae-c-sim.so:
	DESTDIR=$(DESTDIR) $(MAKE) -C $(ROOT_DIR)/sim/opaesim $(DESTDIR)/libopae-c-sim.so

$(DESTDIR)/$(PROJECT): $(SRCS) $(BUILD_DEPS)
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $@

clean-driver:
	DESTDIR=$(DESTDIR) $(MAKE) -C $(ROOT_DIR)/sim/opaesim clean

clean-runtime:
	rm -f $(DESTDIR)/$(PROJECT)

clean: clean-driver clean-runtime

.PHONY: all driver clean-driver clean-runtime clean