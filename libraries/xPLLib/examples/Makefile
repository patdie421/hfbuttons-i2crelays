#
# Makefile for xPLLib
#

TECHNO=openwrt

#
# For LINUX, use the following
CCOPTS = -O2 -DLINUX -pedantic -Wall
LDOPTS	= -O2

ifeq "$(TECHNO)" "openwrt"
OPENWRTSDKTOOLCHAIN_DIR=$(STAGING_DIR)/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2
CC=$(OPENWRTSDKTOOLCHAIN_DIR)/bin/mips-openwrt-linux-gcc $(CCOPTS)
LD=$(OPENWRTSDKTOOLCHAIN_DIR)/bin/mips-openwrt-linux-uclibc-gcc $(LDOPTS)
else
CC = gcc $(CCOPTS)
LD = gcc $(LDOPTS)
endif

LIBCC=$(CC) -fPIC

LIST = xPL_Hub xPL_Logger xPL_Clock xPL_ConfigClock xPLSend xPL_Hub_static

SOURCES = $(addsuffix .c, $(LIST))
OBJECTS = $(addprefix $(TECHNO).objects/, $(SOURCES:.c=.o))
CMDS = $(addprefix $(TECHNO).bin/, $(LIST))

all:	$(TECHNO).bin $(TECHNO).objects ${CMDS}

$(TECHNO).objects:
	@mkdir -p $(TECHNO).objects
	
$(TECHNO).bin:
	@mkdir -p $(TECHNO).bin


$(TECHNO).objects/%.o: %.c
	$(CC) -c $< -o $@


$(TECHNO).bin/xPL_Hub: $(TECHNO).objects/xPL_Hub.o
	$(CC) $< ../$(TECHNO)_xPL.a -o $@
	
$(TECHNO).bin/xPL_Logger: $(TECHNO).objects/xPL_Logger.o
	$(CC) $< ../$(TECHNO)_xPL.a -o $@

$(TECHNO).bin/xPL_Clock: $(TECHNO).objects/xPL_Clock.o
	$(CC) $< ../$(TECHNO)_xPL.a -o $@
	
$(TECHNO).bin/xPL_ConfigClock: $(TECHNO).objects/xPL_ConfigClock.o
	$(CC) $< ../$(TECHNO)_xPL.a -o $@
	
$(TECHNO).bin/xPLSend: $(TECHNO).objects/xPLSend.o
	$(CC) $< ../$(TECHNO)_xPL.a -o $@

$(TECHNO).bin/xPL_Hub_static: $(TECHNO).objects/xPL_Hub.o
	$(CC) $< ../$(TECHNO)_xPL.a -o $@

clean:
	rm -f $(TECHNO).objects/*.o core ${CMDS}

rebuild: clean all
