-include config.mk

ifndef BASEDIR
$(error - BASEDIR is unset)
endif

ifndef TECHNO
$(error - TECHNO is unset)
endif

ifndef NAME
$(error - NAME is unset)
endif

LIBNAME=$(NAME).a

SHELL = /bin/bash

DEBUGFLAGS  = -D__DEBUG_ON__
CFLAGS      = -std=c99 \
              -O2 \
              -DTECHNO_$(TECHNO) \
              $(DEBUGFLAGS)

LIBDIR=$(BASEDIR)/$(TECHNO)/lib

SOURCES=$(shell echo *.c)
OBJECTS=$(addprefix $(TECHNO).objects/, $(SOURCES:.c=.o))

$(TECHNO).objects/%.o: %.c
	@$(CC) $(INCLUDES) -c $(CFLAGS) -MM -MT $(TECHNO).objects/$*.o $*.c > .deps/$*.dep
	$(CC) $(INCLUDES) -c $(CFLAGS) $*.c -o $(TECHNO).objects/$*.o

all: .depsdir $(TECHNO).objects $(LIBDIR)/$(LIBNAME)

.depsdir:
	@mkdir -p .deps

$(TECHNO).objects:
	@mkdir -p $(TECHNO).objects

$(LIBDIR)/$(LIBNAME): $(OBJECTS)
	@mkdir -p $(LIBDIR)
	rm -f $(LIBDIR)/$(LIBNAME)
	ar q $(LIBDIR)/$(LIBNAME) $(OBJECTS)

clean:
	rm -f $(TECHNO).objects/*.o $(LIBDIR)/$(LIBNAME) .deps/*.dep
 
-include .deps/*.dep
