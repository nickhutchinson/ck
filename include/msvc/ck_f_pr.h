/* MSVC-provided features. ck_pr.h may supply generic fallbacks for omitted ones. */
#define CK_F_PR_ADD_16
#define CK_F_PR_ADD_32
#if defined(_WIN64)
#define CK_F_PR_ADD_64
#else
/* #undef CK_F_PR_ADD_64 */
#endif
#define CK_F_PR_ADD_8
#define CK_F_PR_ADD_CHAR
/* #undef CK_F_PR_ADD_DOUBLE */
#define CK_F_PR_ADD_INT
#define CK_F_PR_ADD_PTR
#define CK_F_PR_ADD_SHORT
#define CK_F_PR_ADD_UINT
#define CK_F_PR_AND_16
#define CK_F_PR_AND_32
#if defined(_WIN64)
#define CK_F_PR_AND_64
#else
/* #undef CK_F_PR_AND_64 */
#endif
#define CK_F_PR_AND_8
#define CK_F_PR_AND_CHAR
#define CK_F_PR_AND_INT
#define CK_F_PR_AND_PTR
#define CK_F_PR_AND_SHORT
#define CK_F_PR_AND_UINT
#define CK_F_PR_BARRIER
#define CK_F_PR_BTC_16
#define CK_F_PR_BTC_32
#if defined(_WIN64)
#define CK_F_PR_BTC_64
#endif
#define CK_F_PR_BTC_INT
#define CK_F_PR_BTC_PTR
#define CK_F_PR_BTC_UINT
#define CK_F_PR_BTR_16
#define CK_F_PR_BTR_32
#if defined(_WIN64)
#define CK_F_PR_BTR_64
#endif
#define CK_F_PR_BTR_INT
#define CK_F_PR_BTR_PTR
#define CK_F_PR_BTR_UINT
#define CK_F_PR_BTS_16
#define CK_F_PR_BTS_32
#if defined(_WIN64)
#define CK_F_PR_BTS_64
#endif
#define CK_F_PR_BTS_INT
#define CK_F_PR_BTS_PTR
#define CK_F_PR_BTS_UINT
#define CK_F_PR_CAS_16
#if defined(_WIN64)
#define CK_F_PR_CAS_16_8
#define CK_F_PR_CAS_16_8_VALUE
#endif
#define CK_F_PR_CAS_16_VALUE
#define CK_F_PR_CAS_32
#if defined(_WIN64)
#define CK_F_PR_CAS_32_4
#define CK_F_PR_CAS_32_4_VALUE
#endif
#define CK_F_PR_CAS_32_VALUE
#define CK_F_PR_CAS_64
#if defined(_WIN64)
#define CK_F_PR_CAS_64_2
#define CK_F_PR_CAS_64_2_VALUE
#else
/* #undef CK_F_PR_CAS_64_2 */
/* #undef CK_F_PR_CAS_64_2_VALUE */
#endif
#define CK_F_PR_CAS_64_VALUE
#define CK_F_PR_CAS_8
#if defined(_WIN64)
#define CK_F_PR_CAS_8_16
#define CK_F_PR_CAS_8_16_VALUE
#endif
#define CK_F_PR_CAS_8_VALUE
#define CK_F_PR_CAS_CHAR
#if defined(_WIN64)
#define CK_F_PR_CAS_CHAR_16
#define CK_F_PR_CAS_CHAR_16_VALUE
#endif
#define CK_F_PR_CAS_CHAR_VALUE
#ifndef CK_PR_DISABLE_DOUBLE
#define CK_F_PR_CAS_DOUBLE
#define CK_F_PR_CAS_DOUBLE_VALUE
#if defined(_WIN64)
#define CK_F_PR_CAS_DOUBLE_2
#endif
#endif
#define CK_F_PR_CAS_INT
#if defined(_WIN64)
#define CK_F_PR_CAS_INT_4
#define CK_F_PR_CAS_INT_4_VALUE
#endif
#define CK_F_PR_CAS_INT_VALUE
#define CK_F_PR_CAS_PTR
#define CK_F_PR_CAS_PTR_2
#define CK_F_PR_CAS_PTR_2_VALUE
#define CK_F_PR_CAS_PTR_VALUE
#define CK_F_PR_CAS_SHORT
#define CK_F_PR_CAS_SHORT_VALUE
#define CK_F_PR_CAS_UINT
#if defined(_WIN64)
#define CK_F_PR_CAS_UINT_4
#define CK_F_PR_CAS_UINT_4_VALUE
#endif
#define CK_F_PR_CAS_UINT_VALUE
#define CK_F_PR_DEC_16
#define CK_F_PR_DEC_16_ZERO
#define CK_F_PR_DEC_32
#define CK_F_PR_DEC_32_ZERO
#if defined(_WIN64)
#define CK_F_PR_DEC_64
#else
/* #undef CK_F_PR_DEC_64 */
#endif
#if defined(_WIN64)
#define CK_F_PR_DEC_64_ZERO
#endif
#define CK_F_PR_DEC_8
#define CK_F_PR_DEC_8_ZERO
#define CK_F_PR_DEC_CHAR
#define CK_F_PR_DEC_CHAR_ZERO
/* #undef CK_F_PR_DEC_DOUBLE */
#define CK_F_PR_DEC_INT
#define CK_F_PR_DEC_INT_ZERO
#define CK_F_PR_DEC_PTR
#define CK_F_PR_DEC_PTR_ZERO
#define CK_F_PR_DEC_SHORT
#define CK_F_PR_DEC_UINT
#define CK_F_PR_DEC_UINT_ZERO
#define CK_F_PR_FAA_16
#define CK_F_PR_FAA_32
#if defined(_WIN64)
#define CK_F_PR_FAA_64
#else
/* #undef CK_F_PR_FAA_64 */
#endif
#define CK_F_PR_FAA_8
#define CK_F_PR_FAA_CHAR
/* #undef CK_F_PR_FAA_DOUBLE */
#define CK_F_PR_FAA_INT
#define CK_F_PR_FAA_PTR
#define CK_F_PR_FAA_SHORT
#define CK_F_PR_FAA_UINT
#define CK_F_PR_FAS_16
#define CK_F_PR_FAS_32
#if defined(_WIN64)
#define CK_F_PR_FAS_64
#else
/* #undef CK_F_PR_FAS_64 */
#endif
#define CK_F_PR_FAS_8
#define CK_F_PR_FAS_CHAR
#if defined(_WIN64) && !defined(CK_PR_DISABLE_DOUBLE)
#define CK_F_PR_FAS_DOUBLE
#else
/* #undef CK_F_PR_FAS_DOUBLE */
#endif
#define CK_F_PR_FAS_INT
#define CK_F_PR_FAS_PTR
#define CK_F_PR_FAS_SHORT
#define CK_F_PR_FAS_UINT
/* #undef CK_F_PR_FENCE_ATOMIC */
/* #undef CK_F_PR_FENCE_ATOMIC_LOAD */
/* #undef CK_F_PR_FENCE_ATOMIC_STORE */
#define CK_F_PR_FENCE_LOAD
/* #undef CK_F_PR_FENCE_LOAD_ATOMIC */
#define CK_F_PR_FENCE_LOAD_DEPENDS
/* #undef CK_F_PR_FENCE_LOAD_STORE */
#define CK_F_PR_FENCE_MEMORY
#define CK_F_PR_FENCE_STORE
/* #undef CK_F_PR_FENCE_STORE_ATOMIC */
/* #undef CK_F_PR_FENCE_STORE_LOAD */
#define CK_F_PR_FENCE_STRICT_ACQREL
#define CK_F_PR_FENCE_STRICT_ACQUIRE
#define CK_F_PR_FENCE_STRICT_ATOMIC
#define CK_F_PR_FENCE_STRICT_ATOMIC_LOAD
#define CK_F_PR_FENCE_STRICT_ATOMIC_STORE
#define CK_F_PR_FENCE_STRICT_LOAD
#define CK_F_PR_FENCE_STRICT_LOAD_ATOMIC
/* #undef CK_F_PR_FENCE_STRICT_LOAD_DEPENDS */
#define CK_F_PR_FENCE_STRICT_LOAD_STORE
#define CK_F_PR_FENCE_STRICT_LOCK
#define CK_F_PR_FENCE_STRICT_MEMORY
#define CK_F_PR_FENCE_STRICT_RELEASE
#define CK_F_PR_FENCE_STRICT_STORE
#define CK_F_PR_FENCE_STRICT_STORE_ATOMIC
#define CK_F_PR_FENCE_STRICT_STORE_LOAD
#define CK_F_PR_FENCE_STRICT_UNLOCK
#define CK_F_PR_INC_16
#define CK_F_PR_INC_16_ZERO
#define CK_F_PR_INC_32
#define CK_F_PR_INC_32_ZERO
#if defined(_WIN64)
#define CK_F_PR_INC_64
#else
/* #undef CK_F_PR_INC_64 */
#endif
#if defined(_WIN64)
#define CK_F_PR_INC_64_ZERO
#endif
#define CK_F_PR_INC_8
#define CK_F_PR_INC_8_ZERO
#define CK_F_PR_INC_CHAR
#define CK_F_PR_INC_CHAR_ZERO
/* #undef CK_F_PR_INC_DOUBLE */
#define CK_F_PR_INC_INT
#define CK_F_PR_INC_INT_ZERO
#define CK_F_PR_INC_PTR
#define CK_F_PR_INC_PTR_ZERO
#define CK_F_PR_INC_SHORT
#define CK_F_PR_INC_UINT
#define CK_F_PR_INC_UINT_ZERO
#define CK_F_PR_LOAD_16
#if defined(_WIN64)
#define CK_F_PR_LOAD_16_8
#endif
#define CK_F_PR_LOAD_32
#if defined(_WIN64)
#define CK_F_PR_LOAD_32_4
#endif
#define CK_F_PR_LOAD_64
#if defined(_WIN64)
#define CK_F_PR_LOAD_64_2
#endif
#define CK_F_PR_LOAD_8
#if defined(_WIN64)
#define CK_F_PR_LOAD_8_16
#endif
#define CK_F_PR_LOAD_CHAR
#if defined(_WIN64)
#define CK_F_PR_LOAD_CHAR_16
#endif
#ifndef CK_PR_DISABLE_DOUBLE
#define CK_F_PR_LOAD_DOUBLE
#endif
#define CK_F_PR_LOAD_INT
#if defined(_WIN64)
#define CK_F_PR_LOAD_INT_4
#endif
#define CK_F_PR_LOAD_PTR
#if defined(_WIN64)
#define CK_F_PR_LOAD_PTR_2
#endif
#define CK_F_PR_LOAD_SHORT
#define CK_F_PR_LOAD_UINT
#if defined(_WIN64)
#define CK_F_PR_LOAD_UINT_4
#endif
/* #undef CK_F_PR_NEG_16 */
/* #undef CK_F_PR_NEG_16_ZERO */
/* #undef CK_F_PR_NEG_32 */
/* #undef CK_F_PR_NEG_32_ZERO */
/* #undef CK_F_PR_NEG_64 */
/* #undef CK_F_PR_NEG_64_ZERO */
/* #undef CK_F_PR_NEG_8 */
/* #undef CK_F_PR_NEG_8_ZERO */
/* #undef CK_F_PR_NEG_CHAR */
/* #undef CK_F_PR_NEG_CHAR_ZERO */
/* #undef CK_F_PR_NEG_DOUBLE */
/* #undef CK_F_PR_NEG_INT */
/* #undef CK_F_PR_NEG_INT_ZERO */
/* #undef CK_F_PR_NEG_PTR */
/* #undef CK_F_PR_NEG_PTR_ZERO */
#define CK_F_PR_NEG_SHORT
/* #undef CK_F_PR_NEG_UINT */
/* #undef CK_F_PR_NEG_UINT_ZERO */
#define CK_F_PR_NOT_16
#define CK_F_PR_NOT_32
#if defined(_WIN64)
#define CK_F_PR_NOT_64
#endif
#define CK_F_PR_NOT_8
#define CK_F_PR_NOT_CHAR
#define CK_F_PR_NOT_INT
#define CK_F_PR_NOT_PTR
#define CK_F_PR_NOT_SHORT
#define CK_F_PR_NOT_UINT
#define CK_F_PR_OR_16
#define CK_F_PR_OR_32
#if defined(_WIN64)
#define CK_F_PR_OR_64
#else
/* #undef CK_F_PR_OR_64 */
#endif
#define CK_F_PR_OR_8
#define CK_F_PR_OR_CHAR
#define CK_F_PR_OR_INT
#define CK_F_PR_OR_PTR
#define CK_F_PR_OR_SHORT
#define CK_F_PR_OR_UINT
/* #undef CK_F_PR_RFO */
#define CK_F_PR_STALL
#define CK_F_PR_STORE_16
#define CK_F_PR_STORE_32
#define CK_F_PR_STORE_64
#define CK_F_PR_STORE_8
#define CK_F_PR_STORE_CHAR
#ifndef CK_PR_DISABLE_DOUBLE
#define CK_F_PR_STORE_DOUBLE
#endif
#define CK_F_PR_STORE_INT
#define CK_F_PR_STORE_PTR
#define CK_F_PR_STORE_SHORT
#define CK_F_PR_STORE_UINT
#define CK_F_PR_SUB_16
#define CK_F_PR_SUB_32
#if defined(_WIN64)
#define CK_F_PR_SUB_64
#else
/* #undef CK_F_PR_SUB_64 */
#endif
#define CK_F_PR_SUB_8
#define CK_F_PR_SUB_CHAR
/* #undef CK_F_PR_SUB_DOUBLE */
#define CK_F_PR_SUB_INT
#define CK_F_PR_SUB_PTR
#define CK_F_PR_SUB_SHORT
#define CK_F_PR_SUB_UINT
#define CK_F_PR_XOR_16
#define CK_F_PR_XOR_32
#if defined(_WIN64)
#define CK_F_PR_XOR_64
#else
/* #undef CK_F_PR_XOR_64 */
#endif
#define CK_F_PR_XOR_8
#define CK_F_PR_XOR_CHAR
#define CK_F_PR_XOR_INT
#define CK_F_PR_XOR_PTR
#define CK_F_PR_XOR_SHORT
#define CK_F_PR_XOR_UINT
