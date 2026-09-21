#ifndef FUZZ_HARNESS_H
#define FUZZ_HARNESS_H
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <ck_stddef.h>
#include <ck_stdint.h>
#include <ck_string.h>

typedef struct fuzz_u128 {
	uint64_t hi;
	uint64_t lo;
} fuzz_u128_t;

typedef struct fuzz_s128 {
	int64_t hi;
	uint64_t lo;
} fuzz_s128_t;

static inline fuzz_u128_t
fuzz_u128_make_u64(uint64_t value)
{
	return (fuzz_u128_t) { 0, value };
}

static inline fuzz_s128_t
fuzz_s128_make_s64(int64_t value)
{
	return (fuzz_s128_t) { value < 0 ? -1 : 0, value };
}

static inline fuzz_u128_t
fuzz_u128_mul(fuzz_u128_t lhs, fuzz_u128_t rhs)
{
	/*
	 * A u128_t represents hi * 2^64 + lo. Multiplying two uint128_t values
	 * gives four terms:
	 *
	 *     lhs.lo * rhs.lo
	 *     lhs.hi * rhs.lo * 2^64
	 *     lhs.lo * rhs.hi * 2^64
	 *     lhs.hi * rhs.hi * 2^128
	 *
	 * We need only the low 128 bits, so the last term can be ignored. The
	 * middle two terms contribute only to result.hi.
	 *
	 * Computing lhs.lo * rhs.lo still requires a 128-bit result. We split
	 * both 64-bit values into 32-bit halves:
	 *
	 *     lhs.lo = a * 2^32 + b
	 *     rhs.lo = c * 2^32 + d
	 *
	 *     lhs.lo * rhs.lo =
	 *         a*c * 2^64 + (a*d + b*c) * 2^32 + b*d
	 *
	 * Inspired by Abseil's int128 implementation:
	 * https://github.com/abseil/abseil-cpp/blob/20260526.0/absl/numeric/int128.h#L1035
	 */
	uint64_t a = lhs.lo >> 32;
	uint64_t b = lhs.lo & 0xffffffffU;
	uint64_t c = rhs.lo >> 32;
	uint64_t d = rhs.lo & 0xffffffffU;
	uint64_t ad = a * d;
	uint64_t bc = b * c;

	/*
	 * The upper halves of a*d and b*c go directly into result.hi. We add
	 * their shifted lower halves to result.lo separately so each overflow
	 * into result.hi is preserved.
	 */
	uint64_t hi = (a * c) + (ad >> 32) + (bc >> 32);
	uint64_t lo = b * d;

	hi += (lo + (ad << 32)) < lo;
	lo += ad << 32;

	hi += (lo + (bc << 32)) < lo;
	lo += bc << 32;

	/*
	 * Add the middle two terms to result.hi. Only their low 64 bits are
	 * part of the result; uint64_t multiplication discards the rest.
	 */
	hi += lhs.hi * rhs.lo;
	hi += lhs.lo * rhs.hi;

	return (fuzz_u128_t) { hi, lo };
}

static inline fuzz_s128_t
fuzz_s128_mul(fuzz_s128_t lhs, fuzz_s128_t rhs)
{
	fuzz_u128_t product = fuzz_u128_mul(
	    (fuzz_u128_t) { lhs.hi, lhs.lo }, (fuzz_u128_t) { rhs.hi, rhs.lo });

	return (fuzz_s128_t) { (int64_t)product.hi, product.lo };
}

static inline fuzz_u128_t
fuzz_u128_add(fuzz_u128_t x, fuzz_u128_t y)
{
	return (fuzz_u128_t) {
		x.hi + y.hi + ((x.lo + y.lo) < x.lo),
		x.lo + y.lo,
	};
}

static inline fuzz_s128_t
fuzz_s128_add(fuzz_s128_t x, fuzz_s128_t y)
{
	fuzz_u128_t sum = fuzz_u128_add(
	    (fuzz_u128_t) { x.hi, x.lo }, (fuzz_u128_t) { y.hi, y.lo });

	return (fuzz_s128_t) { (int64_t)sum.hi, sum.lo };
}

static inline int
fuzz_u128_cmp(fuzz_u128_t x, fuzz_u128_t y)
{
	if (x.hi < y.hi) {
		return -1;
	}

	if (x.hi > y.hi) {
		return 1;
	}

	if (x.lo < y.lo) {
		return -1;
	}

	if (x.lo > y.lo) {
		return 1;
	}

	return 0;
}

static inline int
fuzz_s128_cmp(fuzz_s128_t x, fuzz_s128_t y)
{
	if (x.hi < y.hi) {
		return -1;
	}

	if (x.hi > y.hi) {
		return 1;
	}

	if (x.lo < y.lo) {
		return -1;
	}

	if (x.lo > y.lo) {
		return 1;
	}

	return 0;
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
