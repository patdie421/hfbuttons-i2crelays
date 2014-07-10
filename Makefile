SHELL = /bin/bash

TECHNO=openwrt

-include env.mk

# voir :
# http://wiki.openwrt.org/doku.php?id=oldwiki:dropbearpublickeyauthenticationhowto
# pour la mise en place des cles ssh

all:
	cd $(XPLLIBDIR) ; make TECHNO=$(TECHNO)
	cd $(XPLLIBDIR)/examples ; make TECHNO=$(TECHNO)
	cd $(COMMON) ; make EXECUTABLE=$(EXECUTABLE) TECHNO=$(TECHNO) CC=$(CC)
ifneq "$(BUILDMCU)" "no"
	cd $(ARDUINOSKETCHDIR) ; make BOARD=yun HARDWARE=arduino TARGET=$(ARDUINOSKETCHNAME)
	cd $(RELAYSCONTROLERSKETCHDIR) ; make BOARD=attiny84at1 HARDWARE=tiny TARGET=$(RELAYSCONTROLERSKETCHNAME)
	cd $(HFBUTTONSSKETCHDIR) ; make BOARD=attiny85at8 HARDWARE=tiny TARGET=$(HFBUTTONSSKETCHNAME)
endif

ifeq "$(TECHNO)" "openwrt"
installserver: all
	scp $(TECHNO)/$(EXECUTABLE) $(YUNUSER)@$(YUNHOST):$(YUNDIR)

execserver:
	ssh $(YUNUSER)@$(YUNHOST) $(YUNDIR)/$(EXECUTABLE)
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
	scp $(ARDUINOSKETCH).hex $(YUNUSER)@$(YUNHOST):$(YUNDIR) ; ssh $(YUNUSER)@$(YUNHOST) run-avrdude $(ARDUINOSKETCH).hex
else
cleanmcu:
	@echo ""
endif	

getfromgit: clean
	git pull

buildfromgit: getfromgit all
