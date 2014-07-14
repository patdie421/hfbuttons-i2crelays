# pour quelle cible
-include techno.mk

BUILDMCU=yes

ifndef TECHNO
TECHNO=$(shell uname | tr "[:upper:]" "[:lower:]")
ifeq "$(OS)" "darwin"
TECHNO=macosx
endif
endif

# choix du compilateur
ifeq "$(TECHNO)" "openwrt"
ifndef STAGING_DIR
$(error "STAGING_DIR not set, can't make openwrt binaries")
endif
CC=$(shell find $(STAGING_DIR) -name "mips-openwrt-linux-gcc" -print)
else
CC=gcc
endif

# xpllib en fonction de la cible
XPLLIB=$(TECHNO)_xPL.a
ifeq "$(TECHNO)" "macosx"
XPLLIBDIR="$(TECHNO)/xPLLib"
else
XPLLIBDIR="libraries/xPLLib"
endif

COMMON=common

# noms des repertoires et des binaires
EXECUTABLE=mea-relaysbox-$(TECHNO)
ARDUINOSRCDIR=Arduino
ARDUINOSKETCHNAME=HFavecComio2
ARDUINOSKETCHDIR=$(ARDUINOSRCDIR)/HFavecComio2
RELAYSCONTROLERSKETCHNAME=relayscontroler
RELAYSCONTROLERSKETCHDIR=$(ARDUINOSRCDIR)/relayscontroler
HFBUTTONSSKETCHNAME=hfbuttons
HFBUTTONSSKETCHDIR=$(ARDUINOSRCDIR)/hfbuttons

# déploiement
ifeq "$(TECHNO)" "openwrt"
YUNHOSTNAME=192.168.0.253
YUNUSERNAME=root
YUNINSTALLDIR=.
endif

# remote build
REMOTEHOSTNAME=192.168.0.42
REMOTEUSERNAME=patrice
REMOTEDEVDIR=/home/patrice/Developpement/mea-relaysbox

.DEFAULT_GOAL = all

printenv:
	@echo TECHNO = $(TECHNO)
	@echo CC = $(CC)
	@echo EXECUTABLE = $(EXECUTABLE)
	@echo XPLLIBDIR = $(XPLLIBDIR)
	@echo XPLLIB = $(XPLLIB)
	@echo COMMON = $(COMMON)
	@echo YUNHOSTNAME = $(YUNHOSTNAME)
	@echo YUNUSERNAME = $(YUNUSERNAME)
	@echo YUNINSTALLDIR = $(YUNINSTALLDIR)
	@echo ARDUINOSRCDIR = $(ARDUINOSRCDIR)
	@echo ARDUINOSKETCHNAME = $(ARDUINOSKETCHNAME)
	@echo ARDUINOSKETCHDIR = $(ARDUINOSKETCHDIR)
	@echo RELAYSCONTROLERSKETCHNAME = $(RELAYSCONTROLERSKETCHNAME)
	@echo RELAYSCONTROLERSKETCHDIR = $(RELAYSCONTROLERSKETCHDIR)
	@echo HFBUTTONSSKETCHNAME = $(HFBUTTONSSKETCHNAME)
	@echo HFBUTTONSSKETCHDIR = $(HFBUTTONSSKETCHDIR)
	@echo REMOTEUSERNAME = $(REMOTEUSERNAME)
	@echo REMOTEHOSTNAME = $(REMOTEHOSTNAME)
	@echo REMOTEDEVDIR = $(REMOTEDEVDIR)
