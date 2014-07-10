ifndef TECHNO
$(error - TECHNO is unset, set it : to openwrt, linux or macosx)
endif

ifndef EXECUTABLE
EXECUTABLE=hfbuttons-i2crelays
endif

ifeq "$(TECHNO)" "openwrt"
CC = $(STAGING_DIR)/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-gcc
else
CC = gcc
endif

SHELL = /bin/bash
XPLLIB=$(TECHNO)_xPL.a
XPLLIBDIR=../libraries/xPLLib

DEBUGFLAGS  = -D__DEBUG_ON__

CFLAGS      = -std=c99 \
              -O2 \
              -I ../$(TECHNO)/xPLLib \
              $(DEBUGFLAGS)

LDFLAGS     = -lpthread 

SOURCES=$(shell echo *.c)
OBJECTS=$(addprefix $(TECHNO).objects/, $(SOURCES:.c=.o))

$(TECHNO).objects/%.o: %.c
	@$(CC) -c $(CFLAGS) -MM -MT $(TECHNO).objects/$*.o $*.c > .deps/$*.dep
	$(CC) -c $(CFLAGS) $*.c -o $(TECHNO).objects/$*.o

all: .depdir $(TECHNO).objects ../$(TECHNO)/$(EXECUTABLE)

.depdir:
	@mkdir -p .deps

$(TECHNO).objects:
	@mkdir -p $(TECHNO).objects

../$(TECHNO)/$(EXECUTABLE): $(OBJECTS) $(XPLLIBDIR)/$(XPLLIB)
	$(CC) $(OBJECTS) $(XPLLIBDIR)/$(XPLLIB) $(LDFLAGS) -o $@

clean:
	rm -f $(TECHNO).objects/*.o $(EXECUTALBE) .deps/*.dep
 
-include .deps/*.dep
