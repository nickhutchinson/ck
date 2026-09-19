#ifndef FUZZ_HARNESS_H
#define FUZZ_HARNESS_H
#include <assert.h>
#include <stdio.h>

#include <ck_stddef.h>
#include <ck_stdlib.h>
#include <ck_string.h>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

static inline void
fuzz_harness_init(void)
{
#if defined(_WIN32)
	/* Disable blocking, interactive dialog on assert() failures when we're
	 * linked against the debug CRT. */
	_set_error_mode(_OUT_TO_STDERR);

	/* Disable blocking, interactive dialog from calls to abort() when we're
	 * linked against debug CRT */
	_set_abort_behavior(0, _WRITE_ABORT_MSG);

	/* Make abort() terminate the process with
	 * __fastfail(FAST_FAIL_FATAL_APP_EXIT) rather than exit(3) when we're
	 * linked agianst the debug CRT. This allows for Windows to generate
	 * minidumps via Windows Error Reporting. */
	_set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
#endif
}

#if defined(USE_LIBFUZZER)
#define TEST(function, examples)					\
	void LLVMFuzzerInitialize(int *argcp, char ***argvp);		\
	int LLVMFuzzerTestOneInput(const void *data, size_t n);		\
									\
	void LLVMFuzzerInitialize(int *argcp, char ***argvp)		\
	{								\
		static char size[128];					\
		static char *argv[1024];				\
		int argc = *argcp;					\
									\
		fuzz_harness_init();					\
									\
		assert(argc < 1023);					\
									\
		int r = snprintf(size, sizeof(size),			\
				 "-max_len=%zu", sizeof(examples[0]));	\
		assert((size_t)r < sizeof(size));			\
									\
		memcpy(argv, *argvp, argc * sizeof(argv[0]));		\
		argv[argc++] = size;					\
									\
		*argcp = argc;						\
		*argvp = argv;						\
									\
		for (size_t i = 0;					\
		     i < sizeof(examples) / sizeof(examples[0]);	\
		     i++) {						\
			assert(function(&examples[i]) == 0);		\
		}							\
									\
		return;							\
	}								\
									\
	int LLVMFuzzerTestOneInput(const void *data, size_t n)		\
	{								\
		char buf[sizeof(examples[0])];				\
									\
		memset(buf, 0, sizeof(buf));				\
		if (n < sizeof(buf)) {					\
			memcpy(buf, data, n);				\
		} else {						\
			memcpy(buf, data, sizeof(buf));			\
		}							\
									\
		assert(function((const void *)buf) == 0);		\
		return 0;						\
	}
#elif defined(USE_AFL)
#define TEST(function, examples)					\
	int main(int argc, char **argv)					\
	{								\
		char buf[sizeof(examples[0])];				\
									\
		(void)argc;						\
		(void)argv;						\
									\
		fuzz_harness_init();					\
									\
		for (size_t i = 0;					\
		     i < sizeof(examples) / sizeof(examples[0]);	\
		     i++) {						\
			assert(function(&examples[i]) == 0);		\
		}							\
									\
									\
		while (__AFL_LOOP(10000)) {				\
			memset(buf, 0, sizeof(buf));			\
			read(0, buf, sizeof(buf));			\
									\
			assert(function((const void *)buf) == 0);	\
		}							\
									\
		return 0;						\
	}
#else
#define TEST(function, examples)					\
	int main(int argc, char **argv)					\
	{								\
		(void)argc;						\
		(void)argv;						\
									\
		fuzz_harness_init();					\
									\
		for (size_t i = 0;					\
		     i < sizeof(examples) / sizeof(examples[0]);	\
		     i++) {						\
			assert(function(&examples[i]) == 0);		\
		}							\
									\
		return 0;						\
	}
#endif
#endif /* !FUZZ_HARNESS_H */
