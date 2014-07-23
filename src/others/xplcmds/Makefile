#
# Makefile for xPLLib
#

ifndef BASEDIR
endif
ifndef TECHNO
endif

CC=$(_CC)

#
# For LINUX, use the following

INCLUDES=-I $(BASEDIR)/src/libraries/unix/xPLLib

ifeq "$(TECHNO)" "linux"
CCOPTS = -O2 -DLINUX -pedantic -Wall $(INCLUDES)
LDOPTS	= -O2
endif
ifeq "$(TECHNO)" "openwrt"
CCOPTS = -O2 -DLINUX -pedantic -Wall $(INCLUDES)
LDOPTS	= -O2
endif

LIST = xPL_Hub xPL_Logger xPL_Clock xPL_ConfigClock xPLSend xPL_Hub_static

BINDIR=$(BASEDIR)/$(TECHNO)/bin

SOURCES = $(addsuffix .c, $(LIST))
OBJECTS = $(addprefix $(TECHNO).objects/, $(SOURCES:.c=.o))
CMDS = $(addprefix $(BINDIR)/, $(LIST))


all:	$(BINDIR) $(TECHNO).objects ${CMDS}

$(TECHNO).objects:
	@mkdir -p $@
	
$(BINDIR):
	@mkdir -p $@


$(TECHNO).objects/%.o: %.c
	$(CC) $(CCOPTS) -c $< -o $@


$(BINDIR)/xPL_Hub: $(TECHNO).objects/xPL_Hub.o
	echo $(CCOPTS)
	$(CC) $(CCOPTS) $< $(BASEDIR)/$(TECHNO)/lib/xPLLib.a -o $@
	
$(BINDIR)/xPL_Logger: $(TECHNO).objects/xPL_Logger.o
	$(CC) $(CCOPTS) $< $(BASEDIR)/$(TECHNO)/lib/xPLLib.a -o $@

$(BINDIR)/xPL_Clock: $(TECHNO).objects/xPL_Clock.o
	$(CC) $(CCOPTS) $< $(BASEDIR)/$(TECHNO)/lib/xPLLib.a -o $@
	
$(BINDIR).bin/xPL_ConfigClock: $(TECHNO).objects/xPL_ConfigClock.o
	$(CC) $(CCOPTS) $< $(BASEDIR)/$(TECHNO)/lib/xPLLib.a -o $@
	
$(BINDIR).bin/xPLSend: $(TECHNO).objects/xPLSend.o
	$(CC) $(CCOPTS) $< $(BASEDIR)/$(TECHNO)/lib/xPLLib.a -o $@

$(BINDIR).bin/xPL_Hub_static: $(TECHNO).objects/xPL_Hub.o
	$(CC) $(CCOPTS) $< $(BASEDIR)/$(TECHNO)/lib/xPLLib.a -o $@

clean:
	rm -f $(TECHNO).objects/*.o core ${CMDS}

rebuild: clean all
