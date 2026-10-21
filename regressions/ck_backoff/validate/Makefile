.PHONY: check clean

all: validate

validate: validate.c ../../../include/ck_backoff.h
	$(CC) $(CFLAGS) -o validate validate.c

check: all
	rc=0; \
	$(call CK_CHECK,./validate) \
	exit $$rc

clean:
	rm -rf validate *.dSYM *.exe

include ../../../build/regressions.build
CFLAGS+=-D_GNU_SOURCE
