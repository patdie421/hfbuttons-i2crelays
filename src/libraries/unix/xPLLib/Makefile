#
# Makefile for xPL4Linux's xPLLib
#

#
# For all LINUXs, use the following
CCOPTS = -O2 -DLINUX -pedantic -Wall -g

LIBCC=$(CC) -fPIC

SOURCES = xPL-io.c xPL-utils.c xPL-service.c xPL-message.c xPL-listeners.c xPL-store.c xPL-config.c xPL-hub.c
OBJECTS=$(addprefix $(TECHNO).objects/, $(SOURCES:.c=.o))
LIBDIR=$(BASEDIR)/$(TECHNO)/lib
XPLLIB=$(LIBDIR)/xPLLib.a

ifneq "$(TECHNO)" "macosx"
all:	.depdir $(TECHNO).objects xPLLib
else
all:
	@echo "this xPLLib is not Mac OS X compatible"
endif

$(TECHNO).objects:
	@mkdir -p $(TECHNO).objects

.depdir:
	@mkdir -p .deps

xPLLib: $(OBJECTS) $(XPLLIB)

$(TECHNO).objects/%.o: %.c
	@$(CC) $(CCOPTS) -c -MM -MT $(TECHNO).objects/$*.o $*.c > .deps/$*.dep
	$(LIBCC) $(CCOPTS) -c $*.c -o $(TECHNO).objects/$*.o 

$(XPLLIB): $(OBJECTS)
	@mkdir -p $(LIBDIR)
	@rm -f $(XPLLIB)
	@ar q $(XPLLIB) $(OBJECTS)

clean:
	-rm -f $(TECHNO).objects/*.o $(XPLLIB) core

-include .deps/*.dep
