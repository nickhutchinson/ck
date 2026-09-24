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

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#endif

CK_CC_INLINE static void
ck_pr_barrier(void)
{
	_ReadWriteBarrier();
	return;
}

/*
 * Relaxed loads and stores use the __iso_volatile_load/store intrinsics,
 * which emit plain load/store instructions without the implicit hardware
 * barriers MSVC attaches to volatile accesses under /volatile:ms. The
 * surrounding compiler barriers mirror the GCC port's cheap fallback.
 */
#define CK_PR_LOAD(S, M, T, W)						\
	CK_CC_INLINE static T						\
	ck_pr_md_load_##S(const M *target)				\
	{								\
		T value;						\
									\
		ck_pr_barrier();					\
		value = (T)__iso_volatile_load##W((const volatile __int##W *)target); \
		ck_pr_barrier();					\
		return (value);						\
	}								\
	CK_CC_INLINE static void					\
	ck_pr_md_store_##S(M *target, T value)				\
	{								\
		ck_pr_barrier();					\
		__iso_volatile_store##W((volatile __int##W *)target, (__int##W)value); \
		ck_pr_barrier();					\
		return;							\
	}

#define CK_PR_LOAD_S(S, T, W) CK_PR_LOAD(S, T, T, W)

CK_PR_LOAD_S(char, char, 8)
CK_PR_LOAD_S(uint, unsigned int, 32)
CK_PR_LOAD_S(int, int, 32)
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
	return;
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
	return;
}
#endif /* CK_PR_DISABLE_DOUBLE */

CK_CC_INLINE static void
ck_pr_stall(void)
{
	/* YieldProcessor() from winnt.h */
#if defined(_M_ARM64) || defined(_M_ARM64EC)
	__dmb(_ARM64_BARRIER_ISHST);
	__yield();
#elif defined( _M_AMD64) || defined(_M_IX86)
	_mm_pause();
#else
#error "Unsupported arch"
#endif
}

CK_CC_INLINE static void
ck_pr_fence_full(void)
{
	/* MemoryBarrier() from winnt.h */
#if defined(_M_ARM64) || defined(_M_ARM64EC)
	__dmb(_ARM64_BARRIER_SY);
#elif defined( _M_AMD64)
	__faststorefence();
#elif defined(_M_IX86)
	long barrier = 0;
	(void)_InterlockedOr(&barrier, 0);
#else
#error "Unsupported arch"
#endif
}

/*
 * All fences collapse to a full barrier in the initial port.
 */
#define CK_PR_FENCE(T)							\
	CK_CC_INLINE static void					\
	ck_pr_fence_strict_##T(void)					\
	{								\
		ck_pr_fence_full();					\
	}

CK_PR_FENCE(atomic)
CK_PR_FENCE(atomic_atomic)
CK_PR_FENCE(atomic_load)
CK_PR_FENCE(atomic_store)
CK_PR_FENCE(store_atomic)
CK_PR_FENCE(load_atomic)
CK_PR_FENCE(load)
CK_PR_FENCE(load_load)
CK_PR_FENCE(load_store)
CK_PR_FENCE(store)
CK_PR_FENCE(store_store)
CK_PR_FENCE(store_load)
CK_PR_FENCE(memory)
CK_PR_FENCE(acquire)
CK_PR_FENCE(release)
CK_PR_FENCE(acqrel)
CK_PR_FENCE(lock)
CK_PR_FENCE(unlock)

#undef CK_PR_FENCE

/*
 * Atomic compare and swap.
 */
#define CK_PR_CAS(S, M, T, I, IT)					\
	CK_CC_INLINE static bool					\
	ck_pr_cas_##S(M *target, T compare, T set)			\
	{								\
		return (T)I((volatile IT *)target, (IT)set, (IT)compare) == compare; \
	}								\
	CK_CC_INLINE static bool					\
	ck_pr_cas_##S##_value(M *target, T compare, T set, T *value)	\
	{								\
		T previous = (T)I((volatile IT *)target, (IT)set, (IT)compare); \
		*value = previous;					\
		return previous == compare;				\
	}

CK_PR_CAS(char, char, char, _InterlockedCompareExchange8, char)
CK_PR_CAS(8, uint8_t, uint8_t, _InterlockedCompareExchange8, char)
CK_PR_CAS(16, uint16_t, uint16_t, _InterlockedCompareExchange16, short)
CK_PR_CAS(32, uint32_t, uint32_t, _InterlockedCompareExchange, long)
CK_PR_CAS(int, int, int, _InterlockedCompareExchange, long)
CK_PR_CAS(uint, unsigned int, unsigned int, _InterlockedCompareExchange, long)
CK_PR_CAS(64, uint64_t, uint64_t, _InterlockedCompareExchange64, __int64)

#undef CK_PR_CAS

#if defined(_WIN64)
#define CK_PR_INTERLOCKED_EXCHANGE_ADD_64 _InterlockedExchangeAdd64
#define CK_PR_INTERLOCKED_AND_64 _InterlockedAnd64
#define CK_PR_INTERLOCKED_OR_64 _InterlockedOr64
#define CK_PR_INTERLOCKED_XOR_64 _InterlockedXor64
#else
#define CK_PR_CAS_64_OPERATION(N, O)					\
	CK_CC_INLINE static __int64					\
	N(volatile __int64 *target, __int64 value)			\
	{								\
		__int64 compare;					\
		__int64 previous;					\
									\
		compare = _InterlockedCompareExchange64(target, 0, 0);	\
		do {							\
			previous = compare;				\
			compare = _InterlockedCompareExchange64(	\
			    target, (__int64)((uint64_t)previous O	\
			    (uint64_t)value), previous);		\
		} while (compare != previous);				\
		return previous;					\
	}

CK_PR_CAS_64_OPERATION(ck_pr_interlocked_exchange_add_64, +)
CK_PR_CAS_64_OPERATION(ck_pr_interlocked_and_64, &)
CK_PR_CAS_64_OPERATION(ck_pr_interlocked_or_64, |)
CK_PR_CAS_64_OPERATION(ck_pr_interlocked_xor_64, ^)

#undef CK_PR_CAS_64_OPERATION

#define CK_PR_INTERLOCKED_EXCHANGE_ADD_64 ck_pr_interlocked_exchange_add_64
#define CK_PR_INTERLOCKED_AND_64 ck_pr_interlocked_and_64
#define CK_PR_INTERLOCKED_OR_64 ck_pr_interlocked_or_64
#define CK_PR_INTERLOCKED_XOR_64 ck_pr_interlocked_xor_64
#endif

CK_CC_INLINE static bool
ck_pr_cas_ptr(void *target, void *compare, void *set)
{
	return _InterlockedCompareExchangePointer(
		   (void *volatile *)target, set, compare) == compare;
}

CK_CC_INLINE static bool
ck_pr_cas_ptr_value(void *target, void *compare, void *set, void *value)
{
	void *previous;

	previous = _InterlockedCompareExchangePointer(
	    (void *volatile *)target, set, compare);
	*(void **)value = previous;
	return previous == compare;
}

/*
 * Atomic fetch-and-add.
 */
#define CK_PR_FAA(S, M, T, I, IT)					\
	CK_CC_INLINE static T						\
	ck_pr_faa_##S(M *target, T delta)				\
	{								\
		return (T)I((volatile IT *)target, (IT)delta);		\
	}

CK_PR_FAA(char, char, char, _InterlockedExchangeAdd8, char)
CK_PR_FAA(8, uint8_t, uint8_t, _InterlockedExchangeAdd8, char)
CK_PR_FAA(16, uint16_t, uint16_t, _InterlockedExchangeAdd16, short)
CK_PR_FAA(32, uint32_t, uint32_t, _InterlockedExchangeAdd, long)
CK_PR_FAA(int, int, int, _InterlockedExchangeAdd, long)
CK_PR_FAA(uint, unsigned int, unsigned int, _InterlockedExchangeAdd, long)
CK_PR_FAA(64, uint64_t, uint64_t, CK_PR_INTERLOCKED_EXCHANGE_ADD_64, __int64)

#undef CK_PR_FAA

CK_CC_INLINE static void *
ck_pr_faa_ptr(void *target, uintptr_t delta)
{
#if defined(_WIN64)
	return (void *)(intptr_t)_InterlockedExchangeAdd64(
	    (volatile __int64 *)target, (__int64)delta);
#else
	return (void *)(intptr_t)_InterlockedExchangeAdd(
	    (volatile long *)target, (long)delta);
#endif
}

/*
 * Atomic store-only binary operations.
 */
#define CK_PR_BINARY(K, S, M, T, I, IT)				\
	CK_CC_INLINE static void					\
	ck_pr_##K##_##S(M *target, T value)				\
	{								\
		(void)I((volatile IT *)target, (IT)value);		\
	}

#define CK_PR_BINARY_TYPES(K, I8, I16, I32, I64)			\
	CK_PR_BINARY(K, char, char, char, I8, char)			\
	CK_PR_BINARY(K, 8, uint8_t, uint8_t, I8, char)			\
	CK_PR_BINARY(K, 16, uint16_t, uint16_t, I16, short)		\
	CK_PR_BINARY(K, 32, uint32_t, uint32_t, I32, long)		\
	CK_PR_BINARY(K, int, int, int, I32, long)			\
	CK_PR_BINARY(K, uint, unsigned int, unsigned int, I32, long)	\
	CK_PR_BINARY(K, 64, uint64_t, uint64_t, I64, __int64)

CK_PR_BINARY_TYPES(add, _InterlockedExchangeAdd8, _InterlockedExchangeAdd16,
    _InterlockedExchangeAdd, CK_PR_INTERLOCKED_EXCHANGE_ADD_64)

#define CK_PR_SUB(S, M, T, I, IT)					\
	CK_CC_INLINE static void					\
	ck_pr_sub_##S(M *target, T value)				\
	{								\
		(void)I((volatile IT *)target, -(IT)value);		\
	}

CK_PR_SUB(char, char, char, _InterlockedExchangeAdd8, char)
CK_PR_SUB(8, uint8_t, uint8_t, _InterlockedExchangeAdd8, char)
CK_PR_SUB(16, uint16_t, uint16_t, _InterlockedExchangeAdd16, short)
CK_PR_SUB(32, uint32_t, uint32_t, _InterlockedExchangeAdd, long)
CK_PR_SUB(int, int, int, _InterlockedExchangeAdd, long)
CK_PR_SUB(uint, unsigned int, unsigned int, _InterlockedExchangeAdd, long)
CK_PR_SUB(64, uint64_t, uint64_t, CK_PR_INTERLOCKED_EXCHANGE_ADD_64, __int64)

#undef CK_PR_SUB

CK_PR_BINARY_TYPES(and, _InterlockedAnd8, _InterlockedAnd16,
    _InterlockedAnd, CK_PR_INTERLOCKED_AND_64)
CK_PR_BINARY_TYPES(or, _InterlockedOr8, _InterlockedOr16,
    _InterlockedOr, CK_PR_INTERLOCKED_OR_64)
CK_PR_BINARY_TYPES(xor, _InterlockedXor8, _InterlockedXor16,
    _InterlockedXor, CK_PR_INTERLOCKED_XOR_64)

#undef CK_PR_BINARY_TYPES
#undef CK_PR_BINARY

#undef CK_PR_INTERLOCKED_EXCHANGE_ADD_64
#undef CK_PR_INTERLOCKED_AND_64
#undef CK_PR_INTERLOCKED_OR_64
#undef CK_PR_INTERLOCKED_XOR_64

CK_CC_INLINE static void
ck_pr_add_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)_InterlockedExchangeAdd64(
	    (volatile __int64 *)target, (__int64)value);
#else
	(void)_InterlockedExchangeAdd((volatile long *)target, (long)value);
#endif
	return;
}

CK_CC_INLINE static void
ck_pr_and_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)_InterlockedAnd64((volatile __int64 *)target, (__int64)value);
#else
	(void)_InterlockedAnd((volatile long *)target, (long)value);
#endif
	return;
}

CK_CC_INLINE static void
ck_pr_or_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)_InterlockedOr64((volatile __int64 *)target, (__int64)value);
#else
	(void)_InterlockedOr((volatile long *)target, (long)value);
#endif
	return;
}

CK_CC_INLINE static void
ck_pr_xor_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)_InterlockedXor64((volatile __int64 *)target, (__int64)value);
#else
	(void)_InterlockedXor((volatile long *)target, (long)value);
#endif
	return;
}

CK_CC_INLINE static void
ck_pr_sub_ptr(void *target, uintptr_t value)
{
#if defined(_WIN64)
	(void)_InterlockedExchangeAdd64(
	    (volatile __int64 *)target, -(__int64)value);
#else
	(void)_InterlockedExchangeAdd((volatile long *)target, -(long)value);
#endif
	return;
}

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
CK_PR_UNARY(32, uint32_t, uint32_t)
CK_PR_UNARY(int, int, int)
CK_PR_UNARY(uint, unsigned int, unsigned int)
CK_PR_UNARY(64, uint64_t, uint64_t)
CK_PR_UNARY(ptr, void, uintptr_t)

#undef CK_PR_UNARY

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif /* CK_PR_MSVC_H */
