.PHONY: check clean distribution

OBJECTS=validate

all: $(OBJECTS)

validate: validate.c ../../../include/ck_rwlock.h ../../../include/ck_elide.h
	$(CC) $(CFLAGS) -o validate validate.c

check: all
	rc=0; \
	$(call CK_CHECK,./validate $(CORES) 1) \
	exit $$rc

clean:
	rm -rf *.dSYM *.exe *~ *.o $(OBJECTS)

include ../../../build/regressions.build
CFLAGS+=$(PTHREAD_CFLAGS) -D_GNU_SOURCE
