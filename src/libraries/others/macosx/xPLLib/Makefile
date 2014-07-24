#
# Makefile for xPL4Linux's xPLLib
#
CCOPTS = -O2 -DLINUX -pedantic -Wall -g

LDOPTS	= -O2
LIBCC=$(CC) -fPIC

SOURCES = xPL-io.c xPL-utils.c xPL-service.c xPL-message.c xPL-listeners.c xPL-store.c xPL-config.c xPL-hub.c
OBJECTS=$(addprefix $(TECHNO).objects/, $(SOURCES:.c=.o))
XPLLIBDIR=$(BASEDIR)/macosx
XPLLIB=xPLLib.a

all:	.depdir $(TECHNO).objects xPLLib

$(TECHNO).objects:
	@mkdir -p $(TECHNO).objects

.depdir:
	@mkdir -p .deps

xPLLib: $(OBJECTS) $(XPLLIBDIR)/$(XPLLIB)

$(TECHNO).objects/%.o: %.c
	@$(CC) $(CCOPTS) -c -MM -MT $(TECHNO).objects/$*.o $*.c > .deps/$*.dep
	$(LIBCC) -c $*.c -o $(TECHNO).objects/$*.o 

$(XPLLIBDIR)/$(XPLLIB): $(OBJECTS)
  @mkdir -p $(XPLLIBDIR)
	@rm -f $(XPLLIBDIR)/$(XPLLIB)
	@ar q $(XPLLIB) $(OBJECTS)

clean:
	-rm -f $(TECHNO).objects/*.o $(XPLLIB) core

-include .deps/*.dep
