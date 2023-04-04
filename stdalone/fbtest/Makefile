#
# Makefile for a standalone program
#

BUILD = ../../build

SRC = start.s main.c iolib.c biolib.c
EXE = fbtest
MAP = fbtest.map
BIN = fbtest.bin
MEM = fbtest.mem

all:		$(BIN) $(MEM)

install:	$(BIN) $(MEM)
		mkdir -p $(BUILD)/stdalone
		cp $(BIN) $(BUILD)/stdalone
		cp $(MAP) $(BUILD)/stdalone

boot:
		$(BUILD)/bin/sim -p $(BUILD)/prom/boot.mem -s 001

boot-connect:	$(BIN)
		$(BUILD)/bin/sercomm $(BIN)

run:		$(MEM)
		$(BUILD)/bin/sim -i -r $(MEM) -s 001

run-connect:
		$(BUILD)/bin/sercomm

$(BIN):		$(EXE)
		$(BUILD)/bin/load -p $(EXE) $(BIN)

$(MEM):		$(BIN)
		$(BUILD)/bin/bin2mem $(BIN) $(MEM)

$(EXE):		$(SRC)
		$(BUILD)/bin/lcc -A \
		  -Wo-nostdinc -Wo-nostdlib \
		  -Wo-ldscript=standalone.lnk \
		  -Wo-ldmap=$(MAP) -o $(EXE) $(SRC)

clean:
		rm -f *~ $(EXE) $(MAP) $(BIN) $(MEM)
		rm -f serial.dev
