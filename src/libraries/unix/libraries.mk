SHELL = /bin/bash

ifndef TECHNO
$(error - TECHNO is unset)
endif

ifndef BASEDIR
$(error - BASEDIR is unset)
endif

LIBS=$(shell ls -d */)

.DEFAULT_GOAL = all

printenv:
	@$(foreach V, $(sort $(.VARIABLES)), $(if $(filter-out environment% default automatic, $(origin $V)), $(warning $V=$($V) ($(value $V)))))

all:
	@for i in $(LIBS) ; \
        do \
           $(MAKE) -C $$i -f library.mk BASEDIR=$(BASEDIR) TECHNO=$(TECHNO) CC=$(CC); \
        done

clean:
	@for i in $(LIBS) ; \
        do \
           $(MAKE) -C $$i -f library.mk BASEDIR=$(BASEDIR) TECHNO=$(TECHNO) clean ; \
        done
