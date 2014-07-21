-include config.mk

# repertoire de base contenant l'arboressance de compilation
ifndef
BASEDIR=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
endif

# techno cible du module master
ifndef TECHNO
TECHNO=$(shell uname | tr "[:upper:]" "[:lower:]")
ifeq "$(TECHNO)" "darwin"
TECHNO=macosx
endif
endif

# choix du compilateur pour le module master
ifeq "$(TECHNO)" "openwrt"
ifndef STAGING_DIR
$(error "STAGING_DIR not set, can't make openwrt binaries")
endif
CC=$(shell find $(STAGING_DIR) -name "*-linux-gcc" -print)
else
CC=gcc
endif

ifndef BUILDMCU
BUILDMCU=yes
endif

# noms des repertoires
ifndef SRCDIR
SRCDIR=src
endif

ifndef MASTERDIR
MASTERDIR=master
endif

ifndef SLAVEDIR
SLAVEDIR=slave
endif

ifndef LIBSDIR
LIBSDIR=libraries/unix
endif

# nom des executables
ifndef BINNAME
BINNAME=$(APPSNAME)
endif

ifndef ARDUINOSKETCHNAME
ARDUINOSKETCHNAME=$(APPSNAME)
endif
ifndef ARDUINOSKETCHDIR
ARDUINOSKETCHDIR=$(SRCDIR)/$(SLAVEDIR)
endif
ifndef BOARD
BOARD=yun
endif

# déploiement
ifndef YUNDEPLOY
YUNDEPLOY=no
ifdef YUNHOSTNAME
ifdef YUNUSERNAME
ifdef YUNINSTALLDIR
YUNDEPLOY=yes
endif
endif
endif
endif

# remote build
ifndef REMOTEBUILD
REMOTEBUILD=no
ifdef REMOTEHOSTNAME
ifdef REMOTEUSERNAME
ifdef REMOTEDEVDIR
REMOTEBUILD=yes
endif
endif
endif
endif

SHELL = /bin/bash


.DEFAULT_GOAL = all

printenv:
	@$(foreach V, $(sort $(.VARIABLES)), $(if $(filter-out environment% default automatic, $(origin $V)), $(warning $V=$($V) ($(value $V)))))

all: libs libsothers master slave others

libs:
	$(MAKE) -C $(SRCDIR)/$(LIBSDIR) -f libraries.mk BASEDIR=$(BASEDIR) TECHNO=$(TECHNO) CC=$(CC)

libsothers:
	@if [ -e $(SRCDIR)/$(LIBSDIR)/others/Makefile ]; then \
	   $(MAKE) -C $(SRCDIR)/$(LIBSDIR)/others BASEDIR=$(BASEDIR) TECHNO=$(TECHNO) CC=$(CC); \
	fi

master:
	$(MAKE) -C $(SRCDIR)/$(MASTERDIR) -f master.mk BASEDIR=$(BASEDIR) BINNAME=$(BINNAME) TECHNO=$(TECHNO) CC=$(CC)

slave:
ifneq "$(BUILDMCU)" "no"
	$(MAKE) -C $(ARDUINOSKETCHDIR) -f slave.mk MCUFAMILY=avr BOARD=$(BOARD) HARDWARE=arduino TARGET=$(ARDUINOSKETCHNAME)
else
	@echo "MCU building disabled\n"
endif

others:
	@if [ -e $(SRCDIR)/others/Makefile ]; then \
	   $(MAKE) -C $(SRCDIR)/others BASEDIR=$(BASEDIR) TECHNO=$(TECHNO) CC=$(CC); \
	fi

ifeq "$(YUNDEPLOY)" "yes"
ifeq "$(TECHNO)" "openwrt"
# voir :
# http://wiki.openwrt.org/doku.php?id=oldwiki:dropbearpublickeyauthenticationhowto
# pour la mise en place des cles ssh, 
installmaster: all
	scp $(TECHNO)/$(BINNAME) $(YUNUSERNAME)@$(YUNHOSTNAME):$(YUNINSTALLDIR)

execmaster:
	ssh $(YUNUSERNAME)@$(YUNHOSTNAME) $(YUNINSTALLDIR)/$(BINNAME)
endif
endif

clean: cleanmaster cleanslave cleanlibs

cleanmaster:
	rm -f $(YUNINSTALLFLAG)
	$(MAKE) -f master.mk -C $(SRCDIR)/$(MASTERDIR) BASEDIR=$(BASEDIR) TECHNO=$(TECHNO) BINNAME=$(BINNAME) clean

ifneq "$(BUILDMCU)" "no"
cleanslave: cleanarduino

cleanarduino:
	$(MAKE) -C $(ARDUINOSKETCHDIR) -f slave.mk clean
else
cleanslave:
	@echo "BUILDMCU is set to no: Arduino skech will not be clean"
endif

cleanlibs:
	$(MAKE) -C $(SRCDIR)/$(LIBSDIR) -f libraries.mk BASEDIR=$(BASEDIR) TECHNO=$(TECHNO) clean

ifneq "$(BUILDMCU)" "no"
installslave: slave
	scp $(ARDUINOSKETCH).hex $(YUNUSERNAME)@$(YUNHOSTNAME):$(YUNINSTALLDIR) ; ssh $(YUNUSERNAME)@$(YUNHOSTNAME) run-avrdude $(YUNINSTALLDIR)/$(ARDUINOSKETCHNAME).hex
endif

ifeq "$(REMOTEBUILD)" "yes"
remotebuild:
	ssh $(REMOTEUSERNAME)@$(REMOTEHOSTNAME) ". .bash_aliases ; $(MAKE) -C $(REMOTEDEVDIR) TECHNO=openwrt clean all"
endif
