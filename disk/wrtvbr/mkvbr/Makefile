#
# Makefile to build the volume boot record
#

BUILD = ../../../build

SRC = vbr.s
OBJ = vbr.o
EXE = vbr
MAP = vbr.map
BIN = vbr.bin

all:		$(BIN)

$(BIN):		$(EXE)
		$(BUILD)/bin/load -p $(EXE) $(BIN)

$(EXE):		$(OBJ)
		$(BUILD)/bin/link -s standalone.lnk \
		  -m $(MAP) -o $(EXE) $(OBJ)

$(OBJ):		$(SRC)
		$(BUILD)/bin/asm -o $(OBJ) $(SRC)

clean:
		rm -f *~ $(OBJ) $(EXE) $(MAP) $(BIN)
