#
# Makefile for frame buffer experiment
#

SRCS = fbx.v

all:		show

fbx:		$(SRCS)
		iverilog -Wall -o fbx $(SRCS)

show:		fbx
		./fbx
		gtkwave dump.vcd fbx.cfg

clean:
		rm -f *~ fbx dump.vcd
