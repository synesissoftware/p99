/**
 * @file p99_portable.h
 * @brief Portable 64/128-bit arithmetic and bit-scan helpers.
 *
 * Not installed. Used from @c src/ only.
 *
 * Capability macros (minimum toolchain support):
 *
 * - @c P99_HAS_BUILTIN_CLZLL_ — GCC >= 3.4, Clang >= 4.0, or
 *   @c __has_builtin(__builtin_clzll);
 * - @c P99_HAS_BITSCANREVERSE64_ — MSVC >= 2005 (VC8, @c _MSC_VER >= 1400)
 *   on x64 / ARM64;
 * - @c P99_HAS_UMUL128_ — MSVC >= 2005 (VC8, @c _MSC_VER >= 1400) on x64;
 * - @c P99_HAS_UDIV128_ — MSVC >= 2019 RTM (VC16, @c _MSC_VER >= 1920) on
 *   x64;
 * - @c P99_HAS_INT128_ — GCC/Clang @c __int128 (@c __SIZEOF_INT128__);
 *
 * @copyright Copyright (c) 2026, Matthew Wilson and Synesis Information
 *   Systems
 * @license BSD-3-Clause
 */

#ifndef P99_PORTABLE_H
#define P99_PORTABLE_H

#include <stdint.h>

/* --- Capability detection --------------------------------------------- */

/* P99_HAS_BUILTIN_CLZLL_ */

#if 0
#elif defined(__has_builtin)
# if __has_builtin(__builtin_clzll)
#  define P99_HAS_BUILTIN_CLZLL_                            1
# endif
#endif

#if 0
#elif !defined(P99_HAS_BUILTIN_CLZLL_)
# if defined(__GNUC__) && (__GNUC__ > 3 || \
        (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
#  define P99_HAS_BUILTIN_CLZLL_                            1
# elif defined(__clang__) && __clang_major__ >= 4
#  define P99_HAS_BUILTIN_CLZLL_                            1
# endif
#endif

/* P99_HAS_BITSCANREVERSE64_ */

#if 0
#elif defined(_MSC_VER) && (_MSC_VER >= 1400) && \
    (defined(_M_X64) || defined(_M_ARM64) || defined(_WIN64))
# define P99_HAS_BITSCANREVERSE64_                          1
#endif

/* P99_HAS_UMUL128_ */

#if 0
#elif defined(_MSC_VER) && (_MSC_VER >= 1400) && \
    (defined(_M_X64) || defined(_WIN64))
# define P99_HAS_UMUL128_                                   1
#endif

/* P99_HAS_UDIV128_ */

#if 0
#elif defined(_MSC_VER) && (_MSC_VER >= 1920) && \
    (defined(_M_X64) || defined(_WIN64))
# define P99_HAS_UDIV128_                                   1
#endif

/* P99_HAS_INT128_ */

#if 0
#elif defined(__SIZEOF_INT128__)
# define P99_HAS_INT128_                                    1
#endif

/* includes */

#if 0
#elif defined(P99_HAS_BITSCANREVERSE64_) || defined(P99_HAS_UMUL128_)
# include <intrin.h>
#endif

#if 0
#elif defined(P99_HAS_UDIV128_)
# include <immintrin.h>
#endif

/* --- Floor log2 / bucket index ---------------------------------------- */

/**
 * @brief Return @c floor(log2(@p value)) for @p value >= 2.
 *
 * Matches @c 63 - clz(value) on 64-bit targets.
 */
static size_t
p99_floor_log2_u64_(uint64_t value)
{
#if 0
#elif defined(P99_HAS_BUILTIN_CLZLL_)

    return (size_t)(63 - __builtin_clzll(value));
#elif defined(P99_HAS_BITSCANREVERSE64_)

    unsigned long index;

    (void)_BitScanReverse64(&index, value);

    return (size_t)index;
#else

    size_t   index = 0;
    uint64_t v     = value;

    while (v > 1)
    {
        v >>= 1;
        ++index;
    }

    return index;
#endif
}

/* --- 64 x 64 -> 128 multiply ------------------------------------------ */

static void
p99_mul_u64_u64_(
    uint64_t  multiplicand
,   uint64_t  multiplier
,   uint64_t* high
,   uint64_t* low
)
{
#if 0
#elif defined(P99_HAS_INT128_)

    __uint128_t const product =
        (__uint128_t)multiplicand * (__uint128_t)multiplier;

    *high = (uint64_t)(product >> 64);
    *low  = (uint64_t)product;
#elif defined(P99_HAS_UMUL128_)

    *low = _umul128(multiplicand, multiplier, high);
#else

    uint64_t const a_lo = multiplicand & UINT32_MAX;
    uint64_t const a_hi = multiplicand >> 32;
    uint64_t const b_lo = multiplier & UINT32_MAX;
    uint64_t const b_hi = multiplier >> 32;
    uint64_t const ll   = a_lo * b_lo;
    uint64_t const lh   = a_lo * b_hi;
    uint64_t const hl   = a_hi * b_lo;
    uint64_t const hh   = a_hi * b_hi;
    uint64_t const mid  = lh + hl + (ll >> 32);

    *high = hh + (mid >> 32);
    *low  = (mid << 32) | (ll & UINT32_MAX);
#endif
}

/* --- 128 / 64 division ------------------------------------------------ */

static uint64_t
p99_div_u128_u64_(
    uint64_t high
,   uint64_t low
,   uint64_t divisor
)
{
    if (0 == high)
    {
        return low / divisor;
    }

#if 0
#elif defined(P99_HAS_INT128_)

    return (uint64_t)((((__uint128_t)high << 64) | low) / divisor);
#elif defined(P99_HAS_UDIV128_)

    uint64_t remainder;

    return _udiv128(high, low, divisor, &remainder);
#else

    uint64_t quotient  = 0;
    uint64_t remainder = high;
    int      bit;

    for (bit = 63; bit >= 0; --bit)
    {
        remainder = (remainder << 1) | ((low >> (size_t)bit) & 1u);

        quotient <<= 1;

        if (remainder >= divisor)
        {
            remainder -= divisor;
            quotient  |= 1;
        }
    }

    return quotient;
#endif
}

/* --- Combined helpers ------------------------------------------------- */

static uint64_t
p99_u64_mul_div_u64_(
    uint64_t multiplicand
,   uint64_t multiplier
,   uint64_t divisor
)
{
    uint64_t high;
    uint64_t low;

    p99_mul_u64_u64_(multiplicand, multiplier, &high, &low);

    return p99_div_u128_u64_(high, low, divisor);
}

static uint64_t
p99_u64_add_mul_div_u64_(
    uint64_t addend
,   uint64_t multiplicand
,   uint64_t multiplier
,   uint64_t divisor
)
{
    return addend + p99_u64_mul_div_u64_(multiplicand, multiplier, divisor);
}

#endif /* P99_PORTABLE_H */
