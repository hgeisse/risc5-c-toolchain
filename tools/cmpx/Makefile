#
# Makefile for file comparison tool
#

BUILD = ../../build

SRC = cmpx.c
EXE = cmpx

all:		$(EXE)

install:	$(EXE)
		mkdir -p $(BUILD)/bin
		cp $(EXE) $(BUILD)/bin

$(EXE):		$(SRC)
		gcc -g -Wall -o $(EXE) $(SRC)

clean:
		rm -f *~ $(EXE)
