#
# Makefile to build the master bootstrap (boot manager)
#

BUILD = ../../../build

SRC = start.s main.c iolib.c promlib.c
EXE = mboot
MAP = mboot.map
BIN = mboot.bin

all:		$(BIN)

$(BIN):		$(EXE)
		$(BUILD)/bin/load -p $(EXE) $(BIN)

$(EXE):		$(SRC)
		$(BUILD)/bin/lcc -A \
		  -Wo-nostdinc -Wo-nostdlib \
		  -Wo-ldscript=stage2.lnk \
		  -Wo-ldmap=$(MAP) -o $(EXE) $(SRC)

clean:
		rm -f *~ $(EXE) $(MAP) $(BIN)
