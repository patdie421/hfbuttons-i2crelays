all:
	make -C relayscontroler -f arduino.mk HARDWARE=tiny BOARD=attiny84at1
	make -C hfbuttons -f arduino.mk HARDWARE=tiny BOARD=attiny85at8
  make -C xplcmds TECHNO=$(TECHNO) BASEDIR=$(BASEDIR) _CC=$(_CC)
clean:
	make -C relayscontroler -f arduino.mk HARDWARE=tiny BOARD=attiny84at1 clean
	make -C hfbuttons -f arduino.mk HARDWARE=tiny BOARD=attiny85at8 clean
  make -C xplcmds TECHNO=$(TECHNO) BASEDIR=$(BASEDIR) _CC=$(_CC) clean
