#
# Makefile for xPL4Linux's xPLLib
#

#
# For LINUX, use the following
CCOPTS = -O2 -D__DARWIN_UNIX03 -pedantic -Wall -g
LIBS = -lm
LIBDIR = /usr/local/lib
INCDIR = /usr/local/include

# For UnixWare systems, use the following
# CCOPTS = -O2 -DUNIXWARE
# LIBS = -lnsl -lsocket -lm

#
# For Alpha/DEC OSF/Tru64
#
# CCOPTS = -O2 -DTRU64
# LIBS = -lm

CCOPTS = -std=c99

# *******************************************************
# ******* No more customizations from here down *********
# *******************************************************

LDOPTS	= -O
CC	=	gcc $(CCOPTS)
LIBCC	=	$(CC) -fPIC -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/include/ -L/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/lib/
LD	= 	gcc $(LDOPTS)

INCLUDES = xPL.h xPL_priv.h
LIB_OBJS = xPL-io.o xPL-utils.o xPL-service.o xPL-message.o xPL-listeners.o xPL-store.o xPL-config.o xPL-hub.o

all:	xPLLib examples

install: libxPL.dylib xPL.a xPL.h
	cp -f libxPL.dylib $(LIBDIR)
	cp -f xPL.a $(LIBDIR)
	cp -f xPL.h $(INCDIR)

xPLLib: $(LIB_OBJS) libxPL.dylib xPL.a

libxPL.dylib: $(LIB_OBJS)
	@echo "Creating libxPL.dylib..."
	libtool -dynamic -flat_namespace -lSystem -install_name /usr/local/lib/libxPL.dylib -o libxPL.dylib $(LIB_OBJS) -L/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/usr/lib/

xPL.a: $(LIB_OBJS)
	@echo "Creating xPL.a..."
	@rm -f xPL.a
	@ar q xPL.a $(LIB_OBJS)

xPL-utils.o: $(INCLUDES) xPL-utils.c
	$(LIBCC) -c xPL-utils.c

xPL-config.o: $(INCLUDES) xPL-config.c
	$(LIBCC) -c xPL-config.c

xPL-io.o:	$(INCLUDES) xPL-io.c
	$(LIBCC) -c xPL-io.c

xPL-service.o:	$(INCLUDES) xPL-service.c
	$(LIBCC) -c xPL-service.c

xPL-message.o:	$(INCLUDES) xPL-message.c
	$(LIBCC) -c xPL-message.c

xPL-listeners.o:	$(INCLUDES) xPL-listeners.c
	$(LIBCC) -c xPL-listeners.c

xPL-store.o:	$(INCLUDES) xPL-store.c
	$(LIBCC) -c xPL-store.c

xPL-hub.o:	$(INCLUDES) xPL-hub.c
	$(LIBCC) -c xPL-hub.c


clean:
	-rm -f *.o *.a *.dylib core
dist:
	rm -rf /tmp/xPLLib
	mkdir /tmp/xPLLib
	cp *.[ch] Makefile *.txt  /tmp/xPLLib
	mkdir /tmp/xPLLib/examples
	cp examples/*.[ch] examples/*.txt examples/Makefile /tmp/xPLLib/examples

	CURDIR=`pwd`
	rm -f ${CURDIR}/web/xPLLib.tgz
	cd /tmp; tar czf ${CURDIR}/../web/xPLLib.tgz xPLLib
	cd ${CURDIR}
	rm -rf /tmp/xPLLib
