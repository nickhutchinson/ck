.PHONY: check clean

OBJECTS=serial

all: $(OBJECTS)

serial: serial.c ../../../include/ck_bitmap.h
	$(CC) $(CFLAGS) -o serial serial.c

clean:
	rm -rf *~ *.o $(OBJECTS) *.dSYM *.exe

check: all
	rc=0; \
	$(call CK_CHECK,./serial) \
	exit $$rc

include ../../../build/regressions.build
CFLAGS+=-D_GNU_SOURCE
