.PHONY: check clean distribution

OBJECTS=ck_neutral ck_rp ck_wp

all: $(OBJECTS)

ck_neutral: ck_neutral.c ../../../include/ck_rwcohort.h
	$(CC) $(CFLAGS) -o ck_neutral ck_neutral.c

ck_rp: ck_rp.c ../../../include/ck_rwcohort.h
	$(CC) $(CFLAGS) -o ck_rp ck_rp.c

ck_wp: ck_wp.c ../../../include/ck_rwcohort.h
	$(CC) $(CFLAGS) -o ck_wp ck_wp.c

check: all
	rc=0; \
	$(call CK_CHECK,./ck_neutral `expr $(CORES) / 2` 2 1) \
	$(call CK_CHECK,./ck_rp `expr $(CORES) / 2` 2 1) \
	$(call CK_CHECK,./ck_wp `expr $(CORES) / 2` 2 1) \
	exit $$rc

clean:
	rm -rf *.dSYM *~ *.o $(OBJECTS)

include ../../../build/regressions.build
CFLAGS+=$(PTHREAD_CFLAGS) -D_GNU_SOURCE
