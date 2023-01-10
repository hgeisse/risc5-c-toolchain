#
# Makefile for C standard library
#

BUILD = ../../build

LIBC_DIRS = stdio string math
LIBM_DIRS = math
DIRS = $(LIBC_DIRS) $(LIBM_DIRS)

LIBC_OBJS = $(wildcard $(patsubst %,%/*.o,$(LIBC_DIRS)))
LIBM_OBJS = $(wildcard $(patsubst %,%/*.o,$(LIBM_DIRS)))

LIBC = libc.a
LIBM = libm.a

.PHONY:		all install objects clean

all:		$(LIBC) $(LIBM)

install:	$(LIBC) $(LIBM)
		mkdir -p $(BUILD)/lib
		cp $(LIBC) $(BUILD)/lib
		cp $(LIBM) $(BUILD)/lib

$(LIBC):		objects
		$(BUILD)/bin/mklib -cv $(LIBC) $(LIBC_OBJS)

$(LIBM):		objects
		$(BUILD)/bin/mklib -cv $(LIBM) $(LIBM_OBJS)

objects:
		for i in $(DIRS) ; do \
		  $(MAKE) -C $$i all ; \
		done

clean:
		for i in $(DIRS) ; do \
		  $(MAKE) -C $$i clean ; \
		done
		rm -f *~ $(LIBC)
		rm -f *~ $(LIBM)
