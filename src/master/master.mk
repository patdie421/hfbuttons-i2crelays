-include config.mk

ifndef TECHNO
$(error - TECHNO is unset)
endif

ifndef BASEDIR
$(error - BASEDIR is unset)
endif

ifndef BINNAME
$(error - BINNAME is unset)
endif
BINDIR=$(BASEDIR)/$(TECHNO)/bin
LIBDIR=$(BASEDIR)/$(TECHNO)/lib

SHELL = /bin/bash

DEBUGFLAGS  = -D__DEBUG_ON__
CFLAGS      = -std=c99 \
              -O2 \
              -DTECHNO_$(TECHNO) \
              -I../includes \
              $(DEBUGFLAGS)
LDFLAGS     = -lpthread 

LIBSDIR=$(BASEDIR)/src/libraries/unix
LIBSLIST=$(patsubst %/, %, $(shell ls -d $(LIBSDIR)/*/))
LIBS=$(addprefix $(LIBDIR)/, $(addsuffix .a, $(notdir $(LIBSLIST))))
LIBSINCLUDES=$(addprefix -I, $(LIBSLIST))

SOURCES=$(shell echo *.c)
OBJECTSDIR=$(TECHNO).objects
OBJECTS=$(addprefix $(OBJECTSDIR)/, $(SOURCES:.c=.o))

$(OBJECTSDIR)/%.o: %.c
	@$(CC) -c $(CFLAGS) $(LIBSINCLUDES) -MM -MT $(TECHNO).objects/$*.o $*.c > .deps/$*.dep
	$(CC) -c $(CFLAGS) $(LIBSINCLUDES) $*.c -o $(TECHNO).objects/$*.o

all: depsdir $(TECHNO).objects $(BINDIR)/$(BINNAME)

depsdir:
	@mkdir -p .deps

$(OBJECTSDIR):
	@mkdir -p $(OBJECTSDIR)

$(BINDIR)/$(BINNAME): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) $(LIBS) $(LDFLAGS) -o $@

clean:
	rm -f $(TECHNO).objects/*.o $(BINDIR)/$(BINNAME) .deps/*.dep
 
-include .deps/*.dep
