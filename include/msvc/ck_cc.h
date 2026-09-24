/*
 * Copyright 2026 The Concurrency Kit authors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef CK_MSVC_CC_H
#define CK_MSVC_CC_H

#include <intrin.h>
#include <stddef.h>

#include <ck_md.h>

#if defined(__clang__)
#define CK_CC_UNUSED __attribute__((unused))
#define CK_CC_USED __attribute__((used))
#else
/* #undef CK_CC_UNUSED */
/* #undef CK_CC_USED */
#endif

/*
 * Immediate-operand constraints for inline assembly. When targeting the MSVC
 * ABI, ConcurrencyKit uses intrinsics, not inline assembly.
 */
/* #undef CK_CC_IMM */
/* #undef CK_CC_IMM_U32 */
/* #undef CK_CC_IMM_S32 */

#if defined(__clang__)
#if !defined(_DEBUG)
#define CK_CC_INLINE CK_CC_UNUSED inline
#else
#define CK_CC_INLINE CK_CC_UNUSED
#endif
#else /* !__clang__ */
#if !defined(_DEBUG)
#define CK_CC_INLINE inline
#else
#define CK_CC_INLINE
#endif
#endif

#define CK_CC_FORCE_INLINE __forceinline
#define CK_CC_RESTRICT __restrict

#define CK_CC_DEPRECATED(m) __declspec(deprecated(m))

/*
 * Packed attribute. Clang _does_ support this, but MSVC does not, at least not
 * in a __declspec()-like form, and we don't want to be changing ABI between the
 * clang-cl and MSVC variants of the library.
 *
 * And, as it happens, the CK_CC_PACKED attributes in ConcurrencyKit seem to be
 * vestigial -- they're not aligning anything under its natural alignment.
 */
/* #undef CK_CC_PACKED */

/*
 * Weak reference.
 */
/* #undef CK_CC_WEAKREF */

/*
 * Alignment attribute.
 */
#define CK_CC_ALIGN(B) __declspec(align(B))

/*
 * Cache align.
 */
#define CK_CC_CACHELINE CK_CC_ALIGN(CK_MD_CACHELINE)

/*
 * Branch execution hints.
 */
#if defined(__clang__)
#define CK_CC_LIKELY(x) (__builtin_expect(!!(x), 1))
#define CK_CC_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
/* #undef CK_CC_LIKELY */
/* #undef CK_CC_UNLIKELY */
#endif

/*
 * MSVC does not implement type-based alias analysis. clang does, but the
 * clang-cl driver disables it by default. This can be overridden by specifying
 * -fstrict-aliasing, but that's generally a bad idea -- Windows SDK headers
 * type pun liberally.
 */
#if defined(__clang__)
#define CK_CC_ALIASED __attribute__((__may_alias__))
#else
/* #undef CK_CC_ALIASED */
#endif

#define CK_CC_CONTAINER(F, T, M, N)					\
	CK_CC_INLINE static T *						\
	N(F *p)								\
	{								\
									\
		return (T *)(void *)((char *)p - offsetof(T, M)); \
	}

/*
 * Compile-time typeof.
 */
#define CK_CC_TYPEOF(X, DEFAULT) __typeof__(X)

/*
 * Portability wrappers for bitwise operations.
 */
#ifndef CK_MD_CC_BUILTIN_DISABLE
#define CK_F_CC_FFS
CK_CC_INLINE static int
ck_cc_ffs(unsigned int x)
{
	unsigned long index;

	if (x == 0)
		return 0;
	_BitScanForward(&index, x);
	return (int)index + 1;
}

#define CK_F_CC_FFSL
CK_CC_INLINE static int
ck_cc_ffsl(unsigned long x)
{
	unsigned long index;

	if (x == 0)
		return 0;
	_BitScanForward(&index, x);
	return (int)index + 1;
}

#define CK_F_CC_FFSLL
CK_CC_INLINE static int
ck_cc_ffsll(unsigned long long x)
{
	unsigned long index;

	if (x == 0)
		return 0;
#if defined(_WIN64)
	_BitScanForward64(&index, x);
	return (int)index + 1;
#else
	if ((unsigned long)x != 0) {
		_BitScanForward(&index, (unsigned long)x);
		return (int)index + 1;
	}
	_BitScanForward(&index, (unsigned long)(x >> 32));
	return (int)index + 33;
#endif
}

#define CK_F_CC_CTZ
CK_CC_INLINE static int
ck_cc_ctz(unsigned int x)
{
	unsigned long index;

	if (x == 0)
		return 0;
	_BitScanForward(&index, x);
	return (int)index;
}

#define CK_F_CC_POPCOUNT
CK_CC_INLINE static int
ck_cc_popcount(unsigned int x)
{

	return (int)__popcnt(x);
}
#endif /* CK_MD_CC_BUILTIN_DISABLE */
#endif /* CK_MSVC_CC_H */
