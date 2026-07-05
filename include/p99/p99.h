/**
 * @file p99.h
 * @brief Low-cost performance percentile histogram.
 *
 * p99 provides a zero-allocation, fixed-size histogram for recording event
 * durations (typically in nanoseconds) and querying high-resolution
 * percentiles (p50, p90, p99, etc.).
 *
 * @copyright Copyright (c) 2026, Matthew Wilson and Synesis Information
 *   Systems. Licensed under the 3-clause BSD License.
 */

#ifndef P99_H
#define P99_H

/* --- Version ---------------------------------------------------------- */

/** @def P99_VER_MAJOR
 * The major version number of **p99**.
 */

/** @def P99_VER_MINOR
 * The minor version number of **p99**.
 */

/** @def P99_VER_PATCH
 * The patch version number of **p99**.
 */

/** @def P99_VER
 * The current composite version number of **p99**.
 */

#define P99_VER_MAJOR                                       0
#define P99_VER_MINOR                                       0
#define P99_VER_PATCH                                       1
#define P99_VER_ALPHABETA                                   0x41

#define P99_VER \
    (0 \
        |   (   P99_VER_MAJOR      << 24   ) \
        |   (   P99_VER_MINOR      << 16   ) \
        |   (   P99_VER_PATCH      <<  8   ) \
        |   (   P99_VER_ALPHABETA  <<  0   ) \
    )

#define P99_VER_REVISION                                    P99_VER_PATCH

#define P99_VER_STRINGIZE_(M, m, p)                         #M "." #m "." #p
#define P99_VER_STRINGIZE(M, m, p)                          P99_VER_STRINGIZE_(M, m, p)
#define P99_VER_STRING                                      P99_VER_STRINGIZE(P99_VER_MAJOR, P99_VER_MINOR, P99_VER_PATCH)

#ifndef RC_INVOKED

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif


/* --- Symbol visibility ------------------------------------------------ */

#if 0
#elif defined(P99_STATIC)
# define P99_API_VISIBILITY
#elif defined(_WIN32)
# if defined(P99_SHARED_BUILD)
#  define P99_API_VISIBILITY                                __declspec(dllexport)
# else
#  define P99_API_VISIBILITY                                __declspec(dllimport)
# endif
#elif defined(__GNUC__) && (__GNUC__ >= 4)
# define P99_API_VISIBILITY                                 __attribute__((visibility("default")))
#else
# define P99_API_VISIBILITY
#endif

#define P99_CALL(type)                                      P99_API_VISIBILITY type

/* --- API -------------------------------------------------------------- */


/**
 * @brief Truthy type for p99 predicates and status returns (`int`).
 *
 * Treat @c 0 as false/failure; any non-zero value as true/success.
 * Library functions return @ref P99_FALSE or @ref P99_TRUE only.
 */
typedef int p99_truthy_t;

/** @brief False / failure (0). */
#define P99_FALSE                                           ((p99_truthy_t)0)

/** @brief True / success (1). */
#define P99_TRUE                                            ((p99_truthy_t)1)

/**
 * @def P99_COMPACT_HISTOGRAM
 * @brief Optional compile-time feature.
 *
 * Define before including this header (or pass `-DP99_COMPACT_HISTOGRAM`
 * when compiling the library and all translation units that embed
 * @ref p99_histogram_t) to store bucket counts as 32-bit integers instead
 * of 64-bit (296 bytes vs 552 bytes on 64-bit platforms). Each bucket count
 * is limited to `UINT32_MAX`. Doing so reduces the footprint of the
 * histogram to 296 bytes on 64-bit platforms, and in doing so reduces the
 * performance costs by reduing the number of cache line misses; the cost is
 * in resolution, but for many use cases this is acceptable.
 */
#ifdef DOXYGEN
# define P99_COMPACT_HISTOGRAM
# undef P99_COMPACT_HISTOGRAM
#endif
#ifdef P99_COMPACT_HISTOGRAM
typedef uint32_t                                            p99_bucket_count_t;
#else
typedef uint64_t                                            p99_bucket_count_t;
#endif

/** Number of logarithmic buckets in a histogram. */
#define P99_BUCKET_COUNT                                    (64)

/**
 * @brief Low-cost performance percentile histogram.
 *
 * Tracks event durations with nanosecond precision across 64 logarithmic
 * power-of-two buckets. The structure is compact (552 bytes on 64-bit
 * platforms with default bucket counts; 296 bytes when
 * `P99_COMPACT_HISTOGRAM` is defined) and suitable for stack allocation or
 * embedding.
 */
typedef struct p99_histogram {
    uint8_t             has_overflowed;             /**< Indicates whether an arithmetic overflow has occurred. */
    uint8_t             has_min_event_time;         /**< Indicates whether a minimum event time has been recorded. */
    uint8_t             has_max_event_time;         /**< Indicates whether a maximum event time has been recorded. */
    size_t              event_count;                /**< The number of events recorded. */
    uint64_t            event_time_total;           /**< The total event time in nanoseconds. */
    uint64_t            min_event_time;             /**< The minimum event time in nanoseconds. */
    uint64_t            max_event_time;             /**< The maximum event time in nanoseconds. */
    p99_bucket_count_t  buckets[P99_BUCKET_COUNT];  /**< The bucket counts (power-of-two ranges). */
} p99_histogram_t;

/* --- Lifecycle -------------------------------------------------------- */

/**
 * Initialise @p histogram to the equivalent of a newly constructed
 * instance.
 */
P99_CALL(void)
p99_histogram_init(p99_histogram_t* histogram);

/**
 * @brief Clear @p histogram, resetting all values.
 *
 * Equivalent to @ref p99_histogram_init.
 */
P99_CALL(void)
p99_histogram_clear(p99_histogram_t* histogram);

/* --- Recording -------------------------------------------------------- */

/**
 * @brief Record an event duration in nanoseconds.
 *
 * @return @ref P99_TRUE on success; @ref P99_FALSE if overflow has already
 *   occurred or the running total would overflow.
 */
P99_CALL(p99_truthy_t)
p99_histogram_push_event_time_ns(
    p99_histogram_t* histogram
,   uint64_t time_in_ns
);

/**
 * @brief Record an event duration in microseconds.
 *
 * @return @ref P99_TRUE on success; @ref P99_FALSE on overflow or
 *   unit-conversion overflow.
 */
P99_CALL(p99_truthy_t)
p99_histogram_push_event_time_us(
    p99_histogram_t* histogram
,   uint64_t time_in_us
);

/**
 * @brief Record an event duration in milliseconds.
 *
 * @return @ref P99_TRUE on success; @ref P99_FALSE on overflow or
 *   unit-conversion overflow.
 */
P99_CALL(p99_truthy_t)
p99_histogram_push_event_time_ms(
    p99_histogram_t* histogram
,   uint64_t time_in_ms
);

/**
 * @brief Record an event duration in seconds.
 *
 * @return @ref P99_TRUE on success; @ref P99_FALSE on overflow or
 *   unit-conversion overflow.
 */
P99_CALL(p99_truthy_t)
p99_histogram_push_event_time_s(
    p99_histogram_t* histogram
,   uint64_t time_in_s
);

/* --- Statistics ------------------------------------------------------- */

/** Return the number of events recorded. */
P99_CALL(size_t)
p99_histogram_event_count(p99_histogram_t const* histogram);

/**
 * @brief Return the total event time in nanoseconds.
 *
 * @param[in] histogram The histogram to query;
 * @param[out] total  Receives the total when the function returns
 *   @ref P99_TRUE;
 *
 * @return @ref P99_TRUE if no overflow has occurred; @ref P99_FALSE
 *   otherwise.
 */
P99_CALL(p99_truthy_t)
p99_histogram_event_time_total(
    p99_histogram_t const* histogram
,   uint64_t* total
);

/** Return the total event time regardless of overflow status. */
P99_CALL(uint64_t)
p99_histogram_event_time_total_raw(
    p99_histogram_t const* histogram
);

/** Return whether an arithmetic overflow has occurred. */
P99_CALL(p99_truthy_t)
p99_histogram_has_overflowed(p99_histogram_t const* histogram);

/**
 * @brief Return the minimum observed event time.
 *
 * @param[in] histogram The histogram to query;
 * @param[out] min Receives the minimum when the function returns
 *   @ref P99_TRUE;
 *
 * @return @ref P99_TRUE if at least one event has been recorded;
 *   @ref P99_FALSE otherwise.
 */
P99_CALL(p99_truthy_t)
p99_histogram_min_event_time(
    p99_histogram_t const* histogram
,   uint64_t* min
);

/**
 * @brief Return the maximum observed event time.
 *
 * @param[in] histogram The histogram to query;
 * @param[out] max Receives the maximum when the function returns
 *   @ref P99_TRUE;
 *
 * @return @ref P99_TRUE if at least one event has been recorded;
 *   @ref P99_FALSE otherwise.
 */
P99_CALL(p99_truthy_t)
p99_histogram_max_event_time(
    p99_histogram_t const* histogram
,   uint64_t* max
);

/**
 * @brief Return the count of events in bucket @p index.
 *
 * @param[in] histogram The histogram to query;
 * @param[in] index The index of the bucket to return the count of;
 * @param[out] value Receives the bucket count when the function returns
 *   @ref P99_TRUE;
 *
 * @return @ref P99_TRUE if @p index is in `[0, P99_BUCKET_COUNT)`;
 *   @ref P99_FALSE otherwise.
 */
P99_CALL(p99_truthy_t)
p99_histogram_bucket_value(
    p99_histogram_t const* histogram
,   size_t index
,   uint64_t* value
);

/**
 * @brief Return a pointer to the 64 bucket counts (read-only).
 *
 * The returned pointer is valid for the lifetime of @p histogram.
 */
P99_CALL(p99_bucket_count_t const*)
p99_histogram_buckets(p99_histogram_t const* histogram);

/* --- Percentiles ------------------------------------------------------ */

/**
 * @brief Return the approximated duration at the given percentile.
 *
 * @p percentile is clamped to `[0.0, 100.0]`.
 *
 * @param[in] histogram The histogram to query;
 * @param[in] percentile The percentile to return the approximated duration for;
 * @param[out] value Receives the approximated duration in nanoseconds;
 *
 * @return @ref P99_TRUE if the histogram contains one or more events;
 *   @ref P99_FALSE otherwise.
 */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_percentile(
    p99_histogram_t const* histogram
,   double percentile
,   uint64_t* value
);

/** @brief Return the approximated p50 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p50(
    p99_histogram_t const* histogram
,   uint64_t* value
);

/** @brief Return the approximated p75 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p75(
    p99_histogram_t const* histogram
,   uint64_t* value
);

/** @brief Return the approximated p90 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p90(
    p99_histogram_t const* histogram
,   uint64_t* value
);

/** @brief Return the approximated p95 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p95(
    p99_histogram_t const* histogram
,   uint64_t* value
);

/** @brief Return the approximated p99 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99(
    p99_histogram_t const* histogram
,   uint64_t* value
);

/** @brief Return the approximated p99.5 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_5(
    p99_histogram_t const* histogram
,   uint64_t* value
);

/** @brief Return the approximated p99.9 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_9(
    p99_histogram_t const* histogram
,   uint64_t* value
);

/** @brief Return the approximated p99.99 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_99(
    p99_histogram_t const* histogram
,   uint64_t* value
);

/** @brief Return the approximated p99.999 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_999(
    p99_histogram_t const* histogram
,   uint64_t* value
);

/** @brief Return the approximated p99.9999 duration in nanoseconds. */
P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_999_9(
    p99_histogram_t const* histogram
,   uint64_t* value
);

#ifdef __cplusplus
}
#endif

#endif /* !RC_INVOKED */

#endif /* P99_H */
