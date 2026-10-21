.PHONY: check clean distribution

OBJECTS=ck_cc

all: $(OBJECTS)

ck_cc: ck_cc.c ../../../include/ck_cc.h
	$(CC) $(CFLAGS) -g2 -o ck_cc ck_cc.c

check: all
	rc=0; \
	$(call CK_CHECK,./ck_cc) \
	exit $$rc

clean:
	rm -rf *~ *.o $(OBJECTS) *.dSYM *.exe

include ../../../build/regressions.build
CFLAGS+=-D_GNU_SOURCE
