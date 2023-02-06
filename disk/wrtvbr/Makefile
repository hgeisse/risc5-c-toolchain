#
# Makefile for wrtvbr utility
#

BUILD = ../../build

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -g
LDLIBS = -lm

SRCS = wrtvbr.c
OBJS = $(patsubst %.c,%.o,$(SRCS))
BIN = wrtvbr

all:		$(BIN)

install:	$(BIN)
		mkdir -p $(BUILD)/bin
		cp $(BIN) $(BUILD)/bin

$(BIN):		vbr.dump $(OBJS)
		$(CC) $(LDFLAGS) -o $(BIN) $(OBJS) $(LDLIBS)

%.o:		%.c
		$(CC) $(CFLAGS) -o $@ -c $<

vbr.dump:	dump/dump mkvbr/vbr.bin
		dump/dump mkvbr/vbr.bin vbr.dump

dump/dump:
		$(MAKE) -C dump all

mkvbr/vbr.bin:
		$(MAKE) -C mkvbr all

depend.mak:
		$(CC) -MM -MG $(CFLAGS) $(SRCS) >depend.mak

-include depend.mak

clean:
		$(MAKE) -C dump clean
		$(MAKE) -C mkvbr clean
		rm -f *~ vbr.dump $(OBJS) $(BIN) depend.mak
