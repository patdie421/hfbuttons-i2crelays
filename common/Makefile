ifndef TECHNO
$(error - TECHNO is unset, set it : to openwrt, linux or macosx)
endif

-include ../env.mk

ifndef EXECUTABLE
$(error - EXECUTABLE is unset)
endif

SHELL = /bin/bash

DEBUGFLAGS  = -D__DEBUG_ON__
CFLAGS      = -std=c99 \
              -O2 \
              -I ../$(XPLLIBDIR) \
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

../$(TECHNO)/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) ../$(XPLLIBDIR)/$(XPLLIB) $(LDFLAGS) -o $@

clean:
	rm -f $(TECHNO).objects/*.o ../$(TECHNO)/$(EXECUTABLE) .deps/*.dep
 
-include .deps/*.dep
