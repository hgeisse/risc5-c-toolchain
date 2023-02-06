#
# Makefile to build the master boot record
#

BUILD = ../../../build

SRC = mbr.s
OBJ = mbr.o
EXE = mbr
MAP = mbr.map
BIN = mbr.bin

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
