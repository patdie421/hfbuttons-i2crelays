SHELL = /bin/bash

-include env.mk

all:
	cd $(XPLLIBDIR) ; make TECHNO=$(TECHNO)
	cd $(XPLLIBDIR)/examples ; make TECHNO=$(TECHNO)
	cd $(COMMON) ; make EXECUTABLE=$(EXECUTABLE) TECHNO=$(TECHNO) CC=$(CC)
ifneq "$(BUILDMCU)" "no"
	cd $(ARDUINOSKETCHDIR) ; make MCUFAMILY=avr BOARD=yun HARDWARE=arduino TARGET=$(ARDUINOSKETCHNAME)
	cd $(RELAYSCONTROLERSKETCHDIR) ; make MCUFAMILY=avr BOARD=attiny84at1 HARDWARE=tiny TARGET=$(RELAYSCONTROLERSKETCHNAME)
	cd $(HFBUTTONSSKETCHDIR) ; make MCUFAMILY=avr BOARD=attiny85at8 HARDWARE=tiny TARGET=$(HFBUTTONSSKETCHNAME)
endif

ifeq "$(TECHNO)" "openwrt"
# voir :
# http://wiki.openwrt.org/doku.php?id=oldwiki:dropbearpublickeyauthenticationhowto
# pour la mise en place des cles ssh, 
installserver: all
	scp $(TECHNO)/$(EXECUTABLE) $(YUNUSERNAME)@$(YUNHOSTNAME):$(YUNINSTALLDIR)

execserver:
	ssh $(YUNUSERNAME)@$(YUNHOSTNAME) $(YUNINSTALLDIR)/$(EXECUTABLE)
endif

clean: cleancommon cleanxpllib cleanmcu

cleanxpllib:
	cd $(XPLLIBDIR) ; make TECHNO=$(TECHNO) clean
	cd $(XPLLIBDIR)/examples ; make TECHNO=$(TECHNO) clean
	 
cleancommon:
	rm -f $(YUNINSTALLFLAG)
	cd $(COMMON) ; make TECHNO=$(TECHNO) EXECUTABLE=$(EXECUTABLE) clean

ifneq "$(BUILDMCU)" "no"
cleanmcu: cleanarduino cleanrelayscontroler cleanhfbuttons

cleanarduino:
	cd $(ARDUINOSKETCHDIR) ; make clean

cleanrelayscontroler:
	cd $(RELAYSCONTROLERSKETCHDIR) ; make clean

cleanhfbuttons:
	cd $(HFBUTTONSSKETCHDIR) ; make clean

installsketch: all
	scp $(ARDUINOSKETCH).hex $(YUNUSERNAME)@$(YUNHOSTNAME):$(YUNINSTALLDIR) ; ssh $(YUNUSERNAME)@$(YUNHOSTNAME) run-avrdude $(YUNINSTALLDIR)/$(ARDUINOSKETCHNAME).hex
else
cleanmcu:
	@echo ""
endif	

openwrtremotebuild:
	ssh $(REMOTEUSERNAME)@$(REMOTEHOSTNAME) ". .bash_aliases ; cd $(REMOTEDEVDIR) ; make TECHNO=openwrt buildfromgit"

getfromgit:
	git pull

buildfromgit: getfromgit all
