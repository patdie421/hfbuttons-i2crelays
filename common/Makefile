SHELL = /bin/bash
XPLLIB=xPL.a
XPLLIBDIR=../$(TECHNO)/xPLLib

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

all: .dep $(TECHNO).objects ../$(TECHNO)/$(EXECUTABLE)

.dep:
	@mkdir -p .deps

$(TECHNO).objects:
	@mkdir -p $(TECHNO).objects

../$(TECHNO)/$(EXECUTABLE): $(OBJECTS) $(XPLLIBDIR)/$(XPLLIB)
	$(CC) $(OBJECTS) $(XPLLIBDIR)/$(XPLLIB) $(LDFLAGS) -o $@

clean:
	-rm -f $(TECHNO).objects/*.o $(EXECUTALBE) .deps/*.dep
 
-include .deps/*.dep
