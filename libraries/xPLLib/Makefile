#
# Makefile for xPL4Linux's xPLLib
#
TECHNO=openwrt

#
# For all LINUXs, use the following
CCOPTS = -O2 -DLINUX -pedantic -Wall -g
LIBS = -lm
#
# For Openwrt linux crosscompilation
ifeq "$(TECHNO)" "openwrt"
OPENWRTSDKTOOLCHAIN_DIR=$(STAGING_DIR)/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2
CC=$(OPENWRTSDKTOOLCHAIN_DIR)/bin/mips-openwrt-linux-gcc $(CCOPTS)
LD=$(OPENWRTSDKTOOLCHAIN_DIR)/bin/mips-openwrt-linux-uclibc-gcc $(LDOPTS)
else
CC = gcc $(CCOPTS)
LD = gcc $(LDOPTS)
endif

# *******************************************************
# ******* No more customizations from here down *********
# *******************************************************
LDOPTS	= -O2
LIBCC=$(CC) -fPIC

SOURCES = xPL-io.c xPL-utils.c xPL-service.c xPL-message.c xPL-listeners.c xPL-store.c xPL-config.c xPL-hub.c
OBJECTS=$(addprefix $(TECHNO).objects/, $(SOURCES:.c=.o))
XPLLIB=$(TECHNO)_xPL.a

all:	.depdir $(TECHNO).objects xPLLib

$(TECHNO).objects:
	@mkdir -p $(TECHNO).objects

.depdir:
	@mkdir -p .deps

xPLLib: $(OBJECTS) $(XPLLIB)

$(TECHNO).objects/%.o: %.c
	@$(CC) -c -MM -MT $(TECHNO).objects/$*.o $*.c > .deps/$*.dep
	$(LIBCC) -c $*.c -o $(TECHNO).objects/$*.o 

$(XPLLIB): $(OBJECTS)
	@rm -f $(XPLLIB)
	@ar q $(XPLLIB) $(OBJECTS)

clean:
	-rm -f $(TECHNO).objects/*.o $(XPLLIB) core

-include .deps/*.dep
