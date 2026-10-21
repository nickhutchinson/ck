.PHONY: check clean distribution

OBJECTS=serial serial.delete

all: $(OBJECTS)

serial: serial.c ../../../include/ck_ht.h ../../../src/ck_ht.c
	$(CC) $(CFLAGS) -o serial serial.c ../../../src/ck_ht.c

serial.delete: serial.c ../../../include/ck_ht.h ../../../src/ck_ht.c
	$(CC) $(CFLAGS) -DHT_DELETE -o serial.delete serial.c ../../../src/ck_ht.c

check: all
	rc=0; \
	$(call CK_CHECK,./serial) \
	$(call CK_CHECK,./serial.delete) \
	exit $$rc

clean:
	rm -rf *~ *.o $(OBJECTS) *.dSYM *.exe

include ../../../build/regressions.build
CFLAGS+=-D_GNU_SOURCE
