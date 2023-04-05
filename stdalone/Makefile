#
# Makefile for building standalone programs
#

BUILD = ../build

DIRS = beat blinkintr blinkintr2 blinkpoll echo hello hello2 \
       lcdtest memtest promtest rs232intr sregtest fbtest fbtest-hw

.PHONY:		all install clean

all:
		for i in $(DIRS) ; do \
		  $(MAKE) -C $$i all ; \
		done

install:
		for i in $(DIRS) ; do \
		  $(MAKE) -C $$i install ; \
		done

clean:
		for i in $(DIRS) ; do \
		  $(MAKE) -C $$i clean ; \
		done
		rm -f *~
