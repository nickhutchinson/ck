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

#ifndef CK_PR_MSVC_H
#define CK_PR_MSVC_H

#ifndef CK_PR_H
#error Do not include this file directly, use ck_pr.h
#endif

#include <intrin.h>

#include <ck_cc.h>
#include <ck_stdbool.h>
#include <ck_stdint.h>

#include "ck_f_pr.h"

#if (defined(_M_X64) && !defined(_M_ARM64EC)) || defined(_M_IX86)
#define CK_PR_RMW(I) I
#elif defined(_M_ARM64) || defined(_M_ARM64EC)
#define CK_PR_RMW(I) I##_nf
#else
#error "Unsupported arch"
#endif

#if defined(__clang__)
#pragma clang diagnostic push
/* Suppress warnings about use of __intXX spellings that MSVC intrinsics use. */
#pragma clang diagnostic ignored "-Wlanguage-extension-token"

#if defined(_M_X64) && !defined(_M_ARM64EC)
/* Allow 128-bit interlocked operations. */
#pragma clang attribute push(__attribute__((target("cx16"))), apply_to=function)
#else
#pragma clang attribute push
#endif

#endif /* __clang__ */

CK_CC_INLINE static void
ck_pr_barrier(void)
{
	_ReadWriteBarrier();
}

#define CK_PR_LOAD(S, M, T, W)								\
	CK_CC_INLINE static T								\
	ck_pr_md_load_##S(const M *target)						\
	{										\
		T value;								\
											\
		ck_pr_barrier();							\
		value = (T)__iso_volatile_load##W((const volatile __int##W *)target);	\
		ck_pr_barrier();							\
		return value;								\
	}										\
	 										\
	CK_CC_INLINE static void							\
	ck_pr_md_store_##S(M *target, T value)						\
	{										\
		ck_pr_barrier();							\
		__iso_volatile_store##W((volatile __int##W *)target, (__int##W)value);	\
		ck_pr_barrier();							\
	}

#define CK_PR_LOAD_S(S, T, W) CK_PR_LOAD(S, T, T, W)

CK_PR_LOAD_S(char, char, 8)
CK_PR_LOAD_S(uint, unsigned int, 32)
CK_PR_LOAD_S(int, int, 32)
CK_PR_LOAD_S(short, short, 16)
CK_PR_LOAD_S(64, uint64_t, 64)
CK_PR_LOAD_S(32, uint32_t, 32)
CK_PR_LOAD_S(16, uint16_t, 16)
CK_PR_LOAD_S(8, uint8_t, 8)

#undef CK_PR_LOAD_S
#undef CK_PR_LOAD

CK_CC_INLINE static void *
ck_pr_md_load_ptr(const void *target)
{
	void *value;

	ck_pr_barrier();
#if defined(_WIN64)
	value = (void *)(intptr_t)__iso_volatile_load64(
	    (const volatile __int64 *)target);
#else
	value = (void *)(intptr_t)__iso_volatile_load32(
	    (const volatile __int32 *)target);
#endif
	ck_pr_barrier();

	return value;
}

CK_CC_INLINE static void
ck_pr_md_store_ptr(void *target, const void *value)
{
	ck_pr_barrier();
#if defined(_WIN64)
	__iso_volatile_store64(
	    (volatile __int64 *)target, (__int64)(intptr_t)value);
#else
	__iso_volatile_store32(
	    (volatile __int32 *)target, (__int32)(intptr_t)value);
#endif
	ck_pr_barrier();
}

#ifndef CK_PR_DISABLE_DOUBLE
CK_CC_INLINE static double
ck_pr_md_load_double(const double *target)
{
	__int64 value;

	ck_pr_barrier();
	value = __iso_volatile_load64((const volatile __int64 *)target);
	ck_pr_barrier();

	return *(double *)&value;
}

CK_CC_INLINE static void
ck_pr_md_store_double(double *target, double value)
{
	ck_pr_barrier();
	__iso_volatile_store64((volatile __int64 *)target, *(__int64 *)&value);
	ck_pr_barrier();
}
#endif /* CK_PR_DISABLE_DOUBLE */

CK_CC_INLINE static void
ck_pr_stall(void)
{
#if (defined(_M_X64) && !defined(_M_ARM64EC)) || defined(_M_IX86)
	_mm_pause();
#elif defined(_M_ARM64) || defined(_M_ARM64EC)
	/* FIXME: for parity with the GCC implementation this is just a compiler
	 * barrier at present, but should we not use something stronger here?
	 *
	 * FWIW, winnt.h's YieldProcessor() uses `dmb ishst; yield`. Rust, Go
	 * and various other projects use `isb`.
	 *
	 * See:
	 * - https://www.postgresql.org/message-id/1c2a29b8-5b1e-44f7-a871-71ec5fefc120%40app.fastmail.com
	 * - https://developer.arm.com/community/arm-community-blogs/b/architectures-and-processors-blog/posts/multi-threaded-applications-arm
	 * - https://github.com/golang/go/issues/69232
	 */
	ck_pr_barrier();
#else
#error "Unsupported arch"
#endif
}

CK_CC_INLINE static void
ck_pr_fence_full(void)
{
#if (defined(_M_X64) && !defined(_M_ARM64EC)) ||	\
    (defined(_M_IX86) && !defined(CK_MD_SSE_DISABLE))
	_mm_mfence();
#elif defined(_M_IX86)
	volatile long guard;
	(void)_InterlockedIncrement(&guard);
#elif defined(_M_ARM64) || defined(_M_ARM64EC)
	__dmb(_ARM64_BARRIER_ISH);
#else
#error "Unsupported arch"
#endif
}

#define CK_PR_FENCE(name, action)					\
	CK_CC_INLINE static void					\
	ck_pr_fence_strict_##name(void)					\
	{								\
		action;							\
	}

#if defined(_M_X64) && !defined(_M_ARM64EC)
CK_PR_FENCE(atomic, ck_pr_barrier())
CK_PR_FENCE(atomic_atomic, ck_pr_barrier())
CK_PR_FENCE(atomic_load, ck_pr_barrier())
CK_PR_FENCE(atomic_store, ck_pr_barrier())
CK_PR_FENCE(store_atomic, ck_pr_barrier())
CK_PR_FENCE(load_atomic, ck_pr_barrier())
CK_PR_FENCE(load, _mm_lfence())
CK_PR_FENCE(load_load, _mm_lfence())
CK_PR_FENCE(load_store, ck_pr_fence_full())
CK_PR_FENCE(store, _mm_sfence())
CK_PR_FENCE(store_store, _mm_sfence())
CK_PR_FENCE(store_load, ck_pr_fence_full())
CK_PR_FENCE(memory, ck_pr_fence_full())
CK_PR_FENCE(acquire, ck_pr_fence_full())
CK_PR_FENCE(release, ck_pr_fence_full())
CK_PR_FENCE(acqrel, ck_pr_fence_full())
CK_PR_FENCE(lock, ck_pr_fence_full())
CK_PR_FENCE(unlock, ck_pr_fence_full())

#elif defined(_M_IX86)
CK_PR_FENCE(atomic, ck_pr_barrier())
CK_PR_FENCE(atomic_atomic, ck_pr_barrier())
CK_PR_FENCE(atomic_load, ck_pr_barrier())
CK_PR_FENCE(atomic_store, ck_pr_barrier())
CK_PR_FENCE(store_atomic, ck_pr_barrier())
CK_PR_FENCE(load_atomic, ck_pr_barrier())
#if defined(CK_MD_SSE_DISABLE)
CK_PR_FENCE(load, ck_pr_fence_full())
CK_PR_FENCE(load_load, ck_pr_fence_full())
#else
CK_PR_FENCE(load, _mm_lfence())
CK_PR_FENCE(load_load, _mm_lfence())
#endif
CK_PR_FENCE(load_store, ck_pr_fence_full())
#if defined(CK_MD_SSE_DISABLE)
CK_PR_FENCE(store, ck_pr_fence_full())
CK_PR_FENCE(store_store, ck_pr_fence_full())
#else
CK_PR_FENCE(store, _mm_sfence())
CK_PR_FENCE(store_store, _mm_sfence())
#endif
CK_PR_FENCE(store_load, ck_pr_fence_full())
CK_PR_FENCE(memory, ck_pr_fence_full())
CK_PR_FENCE(acquire, ck_pr_fence_full())
CK_PR_FENCE(release, ck_pr_fence_full())
CK_PR_FENCE(acqrel, ck_pr_fence_full())
CK_PR_FENCE(lock, ck_pr_fence_full())
CK_PR_FENCE(unlock, ck_pr_fence_full())

#elif defined(_M_ARM64) || defined(_M_ARM64EC)
CK_PR_FENCE(atomic, __dmb(_ARM64_BARRIER_ISHST))
CK_PR_FENCE(atomic_atomic, __dmb(_ARM64_BARRIER_ISHST))
CK_PR_FENCE(atomic_load, ck_pr_fence_full())
CK_PR_FENCE(atomic_store, __dmb(_ARM64_BARRIER_ISHST))
CK_PR_FENCE(store_atomic, __dmb(_ARM64_BARRIER_ISHST))
CK_PR_FENCE(load_atomic, ck_pr_fence_full())
CK_PR_FENCE(load, __dmb(_ARM64_BARRIER_ISHLD))
CK_PR_FENCE(load_load, __dmb(_ARM64_BARRIER_ISHLD))
CK_PR_FENCE(load_store, ck_pr_fence_full())
CK_PR_FENCE(store, __dmb(_ARM64_BARRIER_ISHST))
CK_PR_FENCE(store_store, __dmb(_ARM64_BARRIER_ISHST))
CK_PR_FENCE(store_load, ck_pr_fence_full())
CK_PR_FENCE(memory, ck_pr_fence_full())
CK_PR_FENCE(acquire, ck_pr_fence_full())
CK_PR_FENCE(release, ck_pr_fence_full())
CK_PR_FENCE(acqrel, ck_pr_fence_full())
CK_PR_FENCE(lock, ck_pr_fence_full())
CK_PR_FENCE(unlock, ck_pr_fence_full())

#else
#error "Unsupported arch"
#endif

#undef CK_PR_FENCE

/*
 * Atomic compare and swap.
 */
#define CK_PR_CAS(S, M, T, I, IT)								\
	CK_CC_INLINE static bool								\
	ck_pr_cas_##S(M *target, T compare, T set)						\
	{											\
		return (T)CK_PR_RMW(I)((volatile IT *)target, (IT)set, (IT)compare) == compare;	\
	}											\
	CK_CC_INLINE static bool								\
	ck_pr_cas_##S##_value(M *target, T compare, T set, T *value)				\
	{											\
		T previous = (T)CK_PR_RMW(I)((volatile IT *)target, (IT)set, (IT)compare);	\
		*value = previous;								\
		return previous == compare;							\
	}

CK_PR_CAS(char, char, char, _InterlockedCompareExchange8, char)
CK_PR_CAS(8, uint8_t, uint8_t, _InterlockedCompareExchange8, char)
CK_PR_CAS(16, uint16_t, uint16_t, _InterlockedCompareExchange16, short)
CK_PR_CAS(short, short, short, _InterlockedCompareExchange16, short)
CK_PR_CAS(32, uint32_t, uint32_t, _InterlockedCompareExchange, long)
CK_PR_CAS(int, int, int, _InterlockedCompareExchange, long)
CK_PR_CAS(uint, unsigned int, unsigned int, _InterlockedCompareExchange, long)
CK_PR_CAS(64, uint64_t, uint64_t, _InterlockedCompareExchange64, __int64)

#undef CK_PR_CAS

#ifndef CK_PR_DISABLE_DOUBLE
CK_CC_INLINE static bool
ck_pr_cas_double(double *target, double compare, double set)
{
	__int64 expected = *(__int64 *)&compare;

	return CK_PR_RMW(_InterlockedCompareExchange64)(
	    (volatile __int64 *)target, *(__int64 *)&set, expected) == expected;
}

CK_CC_INLINE static bool
ck_pr_cas_double_value(double *target, double compare, double set,
    double *value)
{
	__int64 expected = *(__int64 *)&compare;
	__int64 previous;

	previous = CK_PR_RMW(_InterlockedCompareExchange64)(
	    (volatile __int64 *)target, *(__int64 *)&set, expected);
	*value = *(double *)&previous;
	return previous == expected;
}
#endif


CK_CC_INLINE static bool
ck_pr_cas_ptr(void *target, void *compare, void *set)
{
	return CK_PR_RMW(_InterlockedCompareExchangePointer)(
		   (void *volatile *)target, set, compare) == compare;
}

CK_CC_INLINE static bool
ck_pr_cas_ptr_value(void *target, void *compare, void *set, void *value)
{
	void *previous;

	previous = CK_PR_RMW(_InterlockedCompareExchangePointer)(
	    (void *volatile *)target, set, compare);
	*(void **)value = previous;
	return previous == compare;
}

/*
 * Compare and exchange two adjacent words. The target must be aligned to
 * twice the pointer width; the comparand/result is kept separate so that
 * callers may use the same buffer for compare and value.
 * The typed wrappers cast through void pointers without relaxing the
 * target alignment requirement.
 */
#if defined(_WIN64)
CK_CC_INLINE static bool
ck_pr_cas_64_2(uint64_t target[2], uint64_t compare[2], uint64_t set[2])
{
	__int64 previous[2] = { (__int64)compare[0], (__int64)compare[1] };

	return CK_PR_RMW(_InterlockedCompareExchange128)(
	    (volatile __int64 *)target, (__int64)set[1], (__int64)set[0],
	    previous) != 0;
}

CK_CC_INLINE static bool
ck_pr_cas_64_2_value(uint64_t target[2], uint64_t compare[2],
    uint64_t set[2], uint64_t value[2])
{
	__int64 previous[2] = { (__int64)compare[0], (__int64)compare[1] };
	bool success;

	success = CK_PR_RMW(_InterlockedCompareExchange128)(
	    (volatile __int64 *)target, (__int64)set[1], (__int64)set[0],
	    previous) != 0;
	value[0] = (uint64_t)previous[0];
	value[1] = (uint64_t)previous[1];
	return success;
}

CK_CC_INLINE static void
ck_pr_load_64_2(const uint64_t target[2], uint64_t value[2])
{
	__int64 previous[2] = { 0, 0 };

	/* The intrinsic needs a writable pointer for this value-preserving RMW. */
	(void)CK_PR_RMW(_InterlockedCompareExchange128)(
	    (volatile __int64 *)CK_CC_DECONST_PTR(target), 0, 0, previous);
	value[0] = (uint64_t)previous[0];
	value[1] = (uint64_t)previous[1];
}

CK_CC_INLINE static void
ck_pr_load_ptr_2(const void *target, void *value)
{
	ck_pr_load_64_2((const uint64_t *)target, (uint64_t *)value);
}

#define CK_PR_CAS_PAIR(S, W, T)							\
	CK_CC_INLINE static bool						\
	ck_pr_cas_##S##_##W(T target[W], T compare[W], T set[W])		\
	{									\
		return ck_pr_cas_64_2((uint64_t *)(void *)target,		\
		    (uint64_t *)(void *)compare, (uint64_t *)(void *)set);	\
	}									\
	CK_CC_INLINE static bool						\
	ck_pr_cas_##S##_##W##_value(T target[W], T compare[W],			\
	    T set[W], T value[W])						\
	{									\
		return ck_pr_cas_64_2_value((uint64_t *)(void *)target,		\
		    (uint64_t *)(void *)compare, (uint64_t *)(void *)set,	\
		    (uint64_t *)(void *)value);					\
	}

CK_PR_CAS_PAIR(char, 16, char)
CK_PR_CAS_PAIR(int, 4, int)
CK_PR_CAS_PAIR(uint, 4, unsigned int)
CK_PR_CAS_PAIR(32, 4, uint32_t)
CK_PR_CAS_PAIR(16, 8, uint16_t)
CK_PR_CAS_PAIR(8, 16, uint8_t)
#ifndef CK_PR_DISABLE_DOUBLE
CK_PR_CAS_PAIR(double, 2, double)
#endif

#undef CK_PR_CAS_PAIR

#define CK_PR_LOAD_PAIR(S, W, T)						\
	CK_CC_INLINE static void						\
	ck_pr_md_load_##S##_##W(const T target[W], T value[W])			\
	{									\
		ck_pr_load_64_2((const uint64_t *)(const void *)target,		\
		    (uint64_t *)(void *)value);					\
	}

CK_PR_LOAD_PAIR(char, 16, char)
CK_PR_LOAD_PAIR(int, 4, int)
CK_PR_LOAD_PAIR(uint, 4, unsigned int)
CK_PR_LOAD_PAIR(32, 4, uint32_t)
CK_PR_LOAD_PAIR(16, 8, uint16_t)
CK_PR_LOAD_PAIR(8, 16, uint8_t)

#undef CK_PR_LOAD_PAIR
#endif

CK_CC_INLINE static bool
ck_pr_cas_ptr_2(void *target, void *compare, void *set)
{
#if defined(_WIN64)
	return ck_pr_cas_64_2((uint64_t *)target, (uint64_t *)compare,
	    (uint64_t *)set);
#else
	const uint32_t *comparand = (const uint32_t *)compare;
	const uint32_t *replacement = (const uint32_t *)set;
	uint64_t expected, desired, previous;

	expected = (uint64_t)comparand[0] | ((uint64_t)comparand[1] << 32);
	desired = (uint64_t)replacement[0] | ((uint64_t)replacement[1] << 32);
	previous = (uint64_t)_InterlockedCompareExchange64(
	    (volatile __int64 *)target, (__int64)desired, (__int64)expected);
	return previous == expected;
#endif
}

CK_CC_INLINE static bool
ck_pr_cas_ptr_2_value(void *target, void *compare, void *set, void *value)
{
#if defined(_WIN64)
	return ck_pr_cas_64_2_value((uint64_t *)target, (uint64_t *)compare,
	    (uint64_t *)set, (uint64_t *)value);
#else
	const uint32_t *comparand = (const uint32_t *)compare;
	const uint32_t *replacement = (const uint32_t *)set;
	uint32_t *observed = (uint32_t *)value;
	uint64_t expected, desired, previous;

	expected = (uint64_t)comparand[0] | ((uint64_t)comparand[1] << 32);
	desired = (uint64_t)replacement[0] | ((uint64_t)replacement[1] << 32);
	previous = (uint64_t)_InterlockedCompareExchange64(
	    (volatile __int64 *)target, (__int64)desired, (__int64)expected);
	observed[0] = (uint32_t)previous;
	observed[1] = (uint32_t)(previous >> 32);
	return previous == expected;
#endif
}

/*
 * Atomic exchange.
 */
#define CK_PR_FAS(S, M, T, I, IT)						\
	CK_CC_INLINE static T							\
	ck_pr_fas_##S(M *target, T update)					\
	{									\
		return (T)CK_PR_RMW(I)((volatile IT *)target, (IT)update);	\
	}

CK_PR_FAS(char, char, char, _InterlockedExchange8, char)
CK_PR_FAS(8, uint8_t, uint8_t, _InterlockedExchange8, char)
CK_PR_FAS(16, uint16_t, uint16_t, _InterlockedExchange16, short)
CK_PR_FAS(short, short, short, _InterlockedExchange16, short)
CK_PR_FAS(32, uint32_t, uint32_t, _InterlockedExchange, long)
CK_PR_FAS(int, int, int, _InterlockedExchange, long)
CK_PR_FAS(uint, unsigned int, unsigned int, _InterlockedExchange, long)
#if defined(_WIN64)
CK_PR_FAS(64, uint64_t, uint64_t, _InterlockedExchange64, __int64)
#endif

#undef CK_PR_FAS

CK_CC_INLINE static void *
ck_pr_fas_ptr(void *target, void *update)
{
	return CK_PR_RMW(_InterlockedExchangePointer)(
	    (void *volatile *)target, update);
}

#if defined(_WIN64) && !defined(CK_PR_DISABLE_DOUBLE)
CK_CC_INLINE static double
ck_pr_fas_double(double *target, double update)
{
	__int64 bits;

	bits = CK_PR_RMW(_InterlockedExchange64)(
	    (volatile __int64 *)target, *(__int64 *)&update);
	return *(double *)&bits;
}
#endif

/*
 * Atomic fetch-and-add.
 */
#define CK_PR_FAA(S, M, T, I, IT)						\
	CK_CC_INLINE static T							\
	ck_pr_faa_##S(M *target, T delta)					\
	{									\
		return (T)CK_PR_RMW(I)((volatile IT *)target, (IT)delta);	\
	}

CK_PR_FAA(char, char, char, _InterlockedExchangeAdd8, char)
CK_PR_FAA(8, uint8_t, uint8_t, _InterlockedExchangeAdd8, char)
CK_PR_FAA(16, uint16_t, uint16_t, _InterlockedExchangeAdd16, short)
CK_PR_FAA(short, short, short, _InterlockedExchangeAdd16, short)
CK_PR_FAA(32, uint32_t, uint32_t, _InterlockedExchangeAdd, long)
CK_PR_FAA(int, int, int, _InterlockedExchangeAdd, long)
CK_PR_FAA(uint, unsigned int, unsigned int, _InterlockedExchangeAdd, long)
#if defined(_WIN64)
CK_PR_FAA(64, uint64_t, uint64_t, _InterlockedExchangeAdd64, __int64)
#endif

#undef CK_PR_FAA

CK_CC_INLINE static void *
ck_pr_faa_ptr(void *target, uintptr_t delta)
{
#if defined(_WIN64)
	return (void *)(intptr_t)CK_PR_RMW(_InterlockedExchangeAdd64)(
	    (volatile __int64 *)target, (__int64)delta);
#else
	return (void *)(intptr_t)CK_PR_RMW(_InterlockedExchangeAdd)(
	    (volatile long *)target, (long)delta);
#endif
}

/*
 * Atomic store-only binary operations.
 */
#define CK_PR_BINARY(K, S, M, T, I, IT)					\
	CK_CC_INLINE static void					\
	ck_pr_##K##_##S(M *target, T value)				\
	{								\
		(void)CK_PR_RMW(I)((volatile IT *)target, (IT)value);	\
	}

#define CK_PR_BINARY_TYPES(K, I8, I16, I32)				\
	CK_PR_BINARY(K, char, char, char, I8, char)			\
	CK_PR_BINARY(K, 8, uint8_t, uint8_t, I8, char)			\
	CK_PR_BINARY(K, 16, uint16_t, uint16_t, I16, short)		\
	CK_PR_BINARY(K, short, short, short, I16, short)		\
	CK_PR_BINARY(K, 32, uint32_t, uint32_t, I32, long)		\
	CK_PR_BINARY(K, int, int, int, I32, long)			\
	CK_PR_BINARY(K, uint, unsigned int, unsigned int, I32, long)

CK_PR_BINARY_TYPES(add, _InterlockedExchangeAdd8, _InterlockedExchangeAdd16,
    _InterlockedExchangeAdd)

#if defined(_WIN64)
CK_PR_BINARY(add, 64, uint64_t, uint64_t, _InterlockedExchangeAdd64, __int64)
#endif

#define CK_PR_SUB(S, M, T, I, IT)					\
	CK_CC_INLINE static void					\
	ck_pr_sub_##S(M *target, T value)				\
	{								\
		(void)CK_PR_RMW(I)((volatile IT *)target, -(IT)value);	\
	}

CK_PR_SUB(char, char, char, _InterlockedExchangeAdd8, char)
CK_PR_SUB(8, uint8_t, uint8_t, _InterlockedExchangeAdd8, char)
CK_PR_SUB(16, uint16_t, uint16_t, _InterlockedExchangeAdd16, short)
CK_PR_SUB(short, short, short, _InterlockedExchangeAdd16, short)
CK_PR_SUB(32, uint32_t, uint32_t, _InterlockedExchangeAdd, long)
CK_PR_SUB(int, int, int, _InterlockedExchangeAdd, long)
CK_PR_SUB(uint, unsigned int, unsigned int, _InterlockedExchangeAdd, long)
#if defined(_WIN64)
CK_PR_SUB(64, uint64_t, uint64_t, _InterlockedExchangeAdd64, __int64)
#endif

#undef CK_PR_SUB

CK_PR_BINARY_TYPES(and, _InterlockedAnd8, _InterlockedAnd16,
    _InterlockedAnd)
CK_PR_BINARY_TYPES(or, _InterlockedOr8, _InterlockedOr16,
    _InterlockedOr)
CK_PR_BINARY_TYPES(xor, _InterlockedXor8, _InterlockedXor16,
    _InterlockedXor)

#if defined(_WIN64)
CK_PR_BINARY(and, 64, uint64_t, uint64_t, _InterlockedAnd64, __int64)
CK_PR_BINARY(or, 64, uint64_t, uint64_t, _InterlockedOr64, __int64)
CK_PR_BINARY(xor, 64, uint64_t, uint64_t, _InterlockedXor64, __int64)
#endif

#undef CK_PR_BINARY_TYPES
#undef CK_PR_BINARY


CK_CC_INLINE static void
ck_pr_add_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)CK_PR_RMW(_InterlockedExchangeAdd64)(
	    (volatile __int64 *)target, (__int64)value);
#else
	(void)CK_PR_RMW(_InterlockedExchangeAdd)((volatile long *)target, (long)value);
#endif
}

CK_CC_INLINE static void
ck_pr_and_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)CK_PR_RMW(_InterlockedAnd64)((volatile __int64 *)target, (__int64)value);
#else
	(void)CK_PR_RMW(_InterlockedAnd)((volatile long *)target, (long)value);
#endif
}

CK_CC_INLINE static void
ck_pr_or_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)CK_PR_RMW(_InterlockedOr64)((volatile __int64 *)target, (__int64)value);
#else
	(void)CK_PR_RMW(_InterlockedOr)((volatile long *)target, (long)value);
#endif
}

CK_CC_INLINE static void
ck_pr_xor_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)CK_PR_RMW(_InterlockedXor64)((volatile __int64 *)target, (__int64)value);
#else
	(void)CK_PR_RMW(_InterlockedXor)((volatile long *)target, (long)value);
#endif
}

CK_CC_INLINE static void
ck_pr_sub_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)CK_PR_RMW(_InterlockedExchangeAdd64)(
	    (volatile __int64 *)target, -(__int64)value);
#else
	(void)CK_PR_RMW(_InterlockedExchangeAdd)((volatile long *)target, -(long)value);
#endif
}

#define CK_PR_BIT(K, S, M, U, I, IT, OP)				\
	CK_CC_INLINE static bool					\
	ck_pr_##K##_##S(M *target, unsigned int offset)			\
	{								\
		U mask = (U)1 << offset;				\
		IT previous = CK_PR_RMW(I)((volatile IT *)target,	\
		    (IT)(OP mask));					\
		return ((U)previous & mask) != 0;			\
	}

#define CK_PR_BITS(S, M, U, IT, AND, OR, XOR)			\
	CK_PR_BIT(btc, S, M, U, XOR, IT, )			\
	CK_PR_BIT(btr, S, M, U, AND, IT, ~)			\
	CK_PR_BIT(bts, S, M, U, OR, IT, )

CK_PR_BITS(16, uint16_t, uint16_t, short, _InterlockedAnd16,
    _InterlockedOr16, _InterlockedXor16)
CK_PR_BITS(32, uint32_t, uint32_t, long, _InterlockedAnd,
    _InterlockedOr, _InterlockedXor)
CK_PR_BITS(int, int, unsigned int, long, _InterlockedAnd,
    _InterlockedOr, _InterlockedXor)
CK_PR_BITS(uint, unsigned int, unsigned int, long, _InterlockedAnd,
    _InterlockedOr, _InterlockedXor)
#if defined(_WIN64)
CK_PR_BITS(64, uint64_t, uint64_t, __int64, _InterlockedAnd64,
    _InterlockedOr64, _InterlockedXor64)
CK_PR_BITS(ptr, void, uintptr_t, __int64, _InterlockedAnd64,
    _InterlockedOr64, _InterlockedXor64)
#else
CK_PR_BITS(ptr, void, uintptr_t, long, _InterlockedAnd,
    _InterlockedOr, _InterlockedXor)
#endif

#undef CK_PR_BITS
#undef CK_PR_BIT

#define CK_PR_UNARY(S, M, T)						\
	CK_CC_INLINE static void					\
	ck_pr_inc_##S(M *target)					\
	{								\
		ck_pr_add_##S(target, (T)1);				\
	}								\
	CK_CC_INLINE static void					\
	ck_pr_dec_##S(M *target)					\
	{								\
		ck_pr_sub_##S(target, (T)1);				\
	}

CK_PR_UNARY(char, char, char)
CK_PR_UNARY(8, uint8_t, uint8_t)
CK_PR_UNARY(16, uint16_t, uint16_t)
CK_PR_UNARY(short, short, short)
CK_PR_UNARY(32, uint32_t, uint32_t)
CK_PR_UNARY(int, int, int)
CK_PR_UNARY(uint, unsigned int, unsigned int)
#if defined(_WIN64)
CK_PR_UNARY(64, uint64_t, uint64_t)
#endif
CK_PR_UNARY(ptr, void, uintptr_t)

#undef CK_PR_UNARY

#define CK_PR_UNARY_ZERO(S, M, T, I, IT)				\
	CK_CC_INLINE static bool					\
	ck_pr_inc_##S##_is_zero(M *target)				\
	{								\
		IT previous = CK_PR_RMW(I)((volatile IT *)target, 1);	\
		return (T)previous == (T)-1;				\
	}								\
	CK_CC_INLINE static bool					\
	ck_pr_dec_##S##_is_zero(M *target)				\
	{								\
		IT previous = CK_PR_RMW(I)((volatile IT *)target, -1);	\
		return (T)previous == (T)1;				\
	}

CK_PR_UNARY_ZERO(char, char, char, _InterlockedExchangeAdd8, char)
CK_PR_UNARY_ZERO(8, uint8_t, uint8_t, _InterlockedExchangeAdd8, char)
CK_PR_UNARY_ZERO(16, uint16_t, uint16_t, _InterlockedExchangeAdd16, short)
CK_PR_UNARY_ZERO(32, uint32_t, uint32_t, _InterlockedExchangeAdd, long)
CK_PR_UNARY_ZERO(int, int, int, _InterlockedExchangeAdd, long)
CK_PR_UNARY_ZERO(uint, unsigned int, unsigned int, _InterlockedExchangeAdd, long)
#if defined(_WIN64)
CK_PR_UNARY_ZERO(64, uint64_t, uint64_t, _InterlockedExchangeAdd64, __int64)
CK_PR_UNARY_ZERO(ptr, void, uintptr_t, _InterlockedExchangeAdd64, __int64)
#else
CK_PR_UNARY_ZERO(ptr, void, uintptr_t, _InterlockedExchangeAdd, long)
#endif

#undef CK_PR_UNARY_ZERO

#define CK_PR_NOT(S, M, T)					\
	CK_CC_INLINE static void				\
	ck_pr_not_##S(M *target)				\
	{							\
		ck_pr_xor_##S(target, (T)-1);			\
	}

CK_PR_NOT(char, char, char)
CK_PR_NOT(8, uint8_t, uint8_t)
CK_PR_NOT(16, uint16_t, uint16_t)
CK_PR_NOT(short, short, short)
CK_PR_NOT(32, uint32_t, uint32_t)
CK_PR_NOT(int, int, int)
CK_PR_NOT(uint, unsigned int, unsigned int)
#if defined(_WIN64)
CK_PR_NOT(64, uint64_t, uint64_t)
#endif
CK_PR_NOT(ptr, void, uintptr_t)

#undef CK_PR_NOT

CK_CC_INLINE static void
ck_pr_neg_short(short *target)
{
	short previous = ck_pr_md_load_short(target);

	while (!ck_pr_cas_short_value(target, previous, (short)-previous,
	    &previous))
		ck_pr_stall();
}

#undef CK_PR_RMW

#if defined(__clang__)
#pragma clang attribute pop
#pragma clang diagnostic pop
#endif

#endif /* CK_PR_MSVC_H */
