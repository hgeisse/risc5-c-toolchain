#
# Makefile for a standalone program
#

BUILD = ../../build

SRC = start.s main.c iolib.c promlib.c
EXE = vid16test
MAP = vid16test.map
BIN = vid16test.bin
MEM = vid16test.mem

DSK = disk.img

all:		$(BIN) $(MEM)

install:	$(BIN) $(MEM)
		mkdir -p $(BUILD)/stdalone
		cp $(BIN) $(BUILD)/stdalone
		cp $(MAP) $(BUILD)/stdalone

boot:		$(DSK)
		$(BUILD)/bin/sim -p $(BUILD)/prom/boot.mem -d $(DSK) -s 001

boot-connect:	$(BIN)
		$(BUILD)/bin/sercomm $(BIN)

run:
		@echo "This program cannot be run without a PROM, sorry!"

run-connect:
		@echo "This program cannot be run without a PROM, sorry!"

$(BIN):		$(EXE)
		$(BUILD)/bin/load -p $(EXE) $(BIN)

$(MEM):		$(BIN)
		$(BUILD)/bin/bin2mem $(BIN) $(MEM)

$(EXE):		$(SRC)
		$(BUILD)/bin/lcc -A \
		  -Wo-nostdinc -Wo-nostdlib \
		  -Wo-ldscript=standalone.lnk \
		  -Wo-ldmap=$(MAP) -o $(EXE) $(SRC)

$(DSK):
		$(BUILD)/bin/mkdisk $(DSK) 2048

clean:
		rm -f *~ $(EXE) $(MAP) $(BIN) $(MEM) $(DSK)
		rm -f serial.dev
