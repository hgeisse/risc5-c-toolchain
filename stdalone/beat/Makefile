#
# Makefile for a standalone program
#

BUILD = ../../build

SRC = beat.s
OBJ = beat.o
EXE = beat
MAP = beat.map
BIN = beat.bin
MEM = beat.mem

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

$(MEM):		$(BIN)
		$(BUILD)/bin/bin2mem $(BIN) $(MEM)

$(BIN):		$(EXE)
		$(BUILD)/bin/load -p $(EXE) $(BIN)

$(EXE):		$(OBJ)
		$(BUILD)/bin/link -s standalone.lnk \
		  -m $(MAP) -o $(EXE) $(OBJ)

$(OBJ):		$(SRC)
		$(BUILD)/bin/asm -o $(OBJ) $(SRC)

clean:
		rm -f *~ $(OBJ) $(EXE) $(MAP) $(BIN) $(MEM)
		rm -f serial.dev
