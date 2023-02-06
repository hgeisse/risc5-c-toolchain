#
# Makefile to build the master boot block (with an empty partition table)
#

BUILD = ../../build

all:		mbootblk

install:	mbootblk
		mkdir -p $(BUILD)/disk
		cp mbootblk $(BUILD)/disk

mbootblk:	stage1/mbr.bin stage2/mboot.bin
		cat stage1/mbr.bin stage2/mboot.bin >mbootblk

stage1/mbr.bin:
		$(MAKE) -C stage1

stage2/mboot.bin:
		$(MAKE) -C stage2

clean:
		$(MAKE) -C stage1 clean
		$(MAKE) -C stage2 clean
		rm -f *~ mbootblk
