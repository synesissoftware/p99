/**
 * @file histogram.c
 * @brief Implementation of the p99 performance percentile histogram.
 *
 * Home: https://github.com/synesissoftware/p99
 *
 * Created: 4th July 2026
 * Updated: 6th July 2026
 *
 * @copyright Copyright (c) 2026, Matthew Wilson and Synesis Information
 *   Systems
 * @license BSD-3-Clause
 */

#include <p99/p99.h>

#include "p99_portable.h"

#include <math.h>
#include <string.h>

/* --- Internal helpers ------------------------------------------------- */

static size_t
p99_histogram_bucket_index_(uint64_t time_in_ns);

static p99_truthy_t
p99_histogram_bucket_range_(
    size_t index
,   uint64_t* lower
,   uint64_t* upper
);

static uint64_t
p99_hist_bucket_at_(
    p99_histogram_t const* histogram
,   size_t index
)
{
    return (uint64_t)histogram->buckets[index];
}

static p99_truthy_t
try_add_ns_to_total_and_update_minmax_(
    p99_histogram_t* histogram
,   uint64_t time_in_ns
)
{
    if (histogram->has_overflowed)
    {
        return P99_FALSE;
    }

    if (time_in_ns > UINT64_MAX - histogram->event_time_total)
    {
        histogram->has_overflowed = (uint8_t)P99_TRUE;

        return P99_FALSE;
    }

    histogram->event_time_total += time_in_ns;

    if (0 == histogram->event_count)
    {
        histogram->min_event_time = time_in_ns;
        histogram->max_event_time = time_in_ns;
    }
    else
    {
        if (time_in_ns < histogram->min_event_time)
        {
            histogram->min_event_time = time_in_ns;
        }

        if (time_in_ns > histogram->max_event_time)
        {
            histogram->max_event_time = time_in_ns;
        }
    }

    return P99_TRUE;
}

static p99_truthy_t
value_at_target_rank(
    p99_histogram_t const* histogram
,   uint64_t target_rank
,   uint64_t* value
)
{
    uint64_t accumulated = 0;
    size_t   i;

    if (0 == histogram->event_count)
    {
        return P99_FALSE;
    }

    for (i = 0; i < P99_BUCKET_COUNT; ++i)
    {
        uint64_t count = p99_hist_bucket_at_(histogram, i);

        if (count > 0)
        {
            uint64_t prev_accumulated = accumulated;
            uint64_t lower;
            uint64_t upper;
            uint64_t interpolated;

            accumulated += count;

            if (accumulated >= target_rank)
            {
                if (!p99_histogram_bucket_range_(
                        i
                    ,   &lower
                    ,   &upper
                    ))
                {
                    lower = 0;
                    upper = UINT64_MAX;
                }

                if (target_rank <= prev_accumulated)
                {
                    interpolated = lower;
                }
                else
                {
                    uint64_t target_offset = target_rank - prev_accumulated;
                    uint64_t range_width;

                    if ((P99_BUCKET_COUNT - 1) == i)
                    {
                        range_width = UINT64_MAX - lower;
                    }
                    else
                    {
                        range_width = upper - lower;
                    }

                    if (range_width <= UINT64_MAX / target_offset)
                    {
                        interpolated = lower + (range_width * target_offset) / count;
                    }
                    else
                    {
                        interpolated = p99_u64_add_mul_div_u64_(
                            lower
                        ,   range_width
                        ,   target_offset
                        ,   count
                        );
                    }
                }

                if (interpolated < histogram->min_event_time)
                {
                    interpolated = histogram->min_event_time;
                }

                if (interpolated > histogram->max_event_time)
                {
                    interpolated = histogram->max_event_time;
                }

                *value = interpolated;

                return P99_TRUE;
            }
        }
    }

    *value = histogram->max_event_time;

    return P99_TRUE;
}

static double
clamp_percentile(double percentile)
{
    if (percentile < 0.0)
    {
        return 0.0;
    }
    if (percentile > 100.0)
    {
        return 100.0;
    }

    return percentile;
}

/* --- Lifecycle -------------------------------------------------------- */

P99_CALL(void)
p99_histogram_init(p99_histogram_t* histogram)
{
    memset(histogram, 0, sizeof(p99_histogram_t));
}

P99_CALL(void)
p99_histogram_clear(p99_histogram_t* histogram)
{
    p99_histogram_init(histogram);
}

/* --- Recording -------------------------------------------------------- */

P99_CALL(p99_truthy_t)
p99_histogram_push_event_time_ns(
    p99_histogram_t* histogram
,   uint64_t time_in_ns
)
{
    size_t bucket_index;

    bucket_index = p99_histogram_bucket_index_(time_in_ns);

#if 0
#elif defined(P99_COMPACT_HISTOGRAM)
    if (histogram->buckets[bucket_index] >= UINT32_MAX)
    {
        histogram->has_overflowed = (uint8_t)P99_TRUE;

        return P99_FALSE;
    }
#endif

    if (!try_add_ns_to_total_and_update_minmax_(
            histogram
        ,   time_in_ns
        ))
    {
        return P99_FALSE;
    }

    histogram->event_count += 1;
    histogram->buckets[bucket_index] += 1;

    return P99_TRUE;
}

P99_CALL(p99_truthy_t)
p99_histogram_push_event_time_us(
    p99_histogram_t* histogram
,   uint64_t time_in_us
)
{
    if (time_in_us > UINT64_MAX / 1000ULL)
    {
        histogram->has_overflowed = (uint8_t)P99_TRUE;

        return P99_FALSE;
    }

    return p99_histogram_push_event_time_ns(
        histogram
    ,   time_in_us * 1000ULL
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_push_event_time_ms(
    p99_histogram_t* histogram
,   uint64_t time_in_ms
)
{
    if (time_in_ms > UINT64_MAX / 1000000ULL)
    {
        histogram->has_overflowed = (uint8_t)P99_TRUE;

        return P99_FALSE;
    }

    return p99_histogram_push_event_time_ns(
        histogram
    ,   time_in_ms * 1000000ULL
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_push_event_time_s(
    p99_histogram_t* histogram
,   uint64_t time_in_s
)
{
    if (time_in_s > UINT64_MAX / 1000000000ULL)
    {
        histogram->has_overflowed = (uint8_t)P99_TRUE;

        return P99_FALSE;
    }

    return p99_histogram_push_event_time_ns(
        histogram
    ,   time_in_s * 1000000000ULL
    );
}

/* --- Statistics ------------------------------------------------------- */

P99_CALL(uint64_t)
p99_histogram_event_count(p99_histogram_t const* histogram)
{
    return histogram->event_count;
}

P99_CALL(p99_truthy_t)
p99_histogram_event_time_total(
    p99_histogram_t const* histogram
,   uint64_t* total
)
{
    if (histogram->has_overflowed)
    {
        return P99_FALSE;
    }

    *total = histogram->event_time_total;

    return P99_TRUE;
}

P99_CALL(uint64_t)
p99_histogram_event_time_total_raw(p99_histogram_t const* histogram)
{
    return histogram->event_time_total;
}

P99_CALL(p99_truthy_t)
p99_histogram_has_overflowed(p99_histogram_t const* histogram)
{
    return histogram->has_overflowed;
}

P99_CALL(p99_truthy_t)
p99_histogram_min_event_time(
    p99_histogram_t const* histogram
,   uint64_t* min
)
{
    if (0 == histogram->event_count)
    {
        return P99_FALSE;
    }

    *min = histogram->min_event_time;

    return P99_TRUE;
}

P99_CALL(p99_truthy_t)
p99_histogram_max_event_time(
    p99_histogram_t const* histogram
,   uint64_t* max
)
{
    if (0 == histogram->event_count)
    {
        return P99_FALSE;
    }

    *max = histogram->max_event_time;

    return P99_TRUE;
}

P99_CALL(p99_truthy_t)
p99_histogram_bucket_value(
    p99_histogram_t const* histogram
,   size_t index
,   uint64_t* value
)
{
    if (index >= P99_BUCKET_COUNT)
    {
        return P99_FALSE;
    }

    *value = p99_hist_bucket_at_(histogram, index);

    return P99_TRUE;
}

P99_CALL(p99_bucket_count_t const*)
p99_histogram_buckets(p99_histogram_t const* histogram)
{
    return histogram->buckets;
}

/* --- Percentiles ------------------------------------------------------ */

P99_CALL(p99_truthy_t)
p99_histogram_value_at_percentile(
    p99_histogram_t const* histogram
,   double percentile
,   uint64_t* value
)
{
    double      p;
    double      target_rank;
    uint64_t    accumulated = 0;

    if (0 == histogram->event_count)
    {
        return P99_FALSE;
    }

    p = clamp_percentile(percentile);

    if (p <= 0.0)
    {
        return p99_histogram_min_event_time(
            histogram
        ,   value
        );
    }

    if (p >= 100.0)
    {
        return p99_histogram_max_event_time(
            histogram
        ,   value
        );
    }

    target_rank = (double)histogram->event_count * (p / 100.0);

    { size_t i; for (i = 0; i < P99_BUCKET_COUNT; ++i)
    {
        uint64_t count = p99_hist_bucket_at_(histogram, i);

        if (count > 0)
        {
            uint64_t prev_accumulated = accumulated;
            uint64_t lower;
            uint64_t upper;
            double   range_width;
            double   target_offset;
            double   fraction;
            double   interpolated;
            uint64_t result;

            accumulated += count;

            if ((double)accumulated >= target_rank)
            {
                if (!p99_histogram_bucket_range_(
                        i
                    ,   &lower
                    ,   &upper
                    ))
                {
                    lower = 0;
                    upper = UINT64_MAX;
                }

                target_offset = target_rank - (double)prev_accumulated;

                if ((P99_BUCKET_COUNT - 1) == i)
                {
                    range_width = (double)(UINT64_MAX - lower);
                }
                else
                {
                    range_width = (double)(upper - lower);
                }

                fraction     = target_offset / (double)count;
                interpolated = (double)lower + (range_width * fraction);
                result       = (uint64_t)llround(interpolated);

                if (result < histogram->min_event_time)
                {
                    result = histogram->min_event_time;
                }

                if (result > histogram->max_event_time)
                {
                    result = histogram->max_event_time;
                }

                *value = result;

                return P99_TRUE;
            }
        }
    }}

    return p99_histogram_max_event_time(
        histogram
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p50(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   1
    ,   2
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p75(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   3
    ,   4
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p90(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   90
    ,   100
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p95(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   95
    ,   100
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   99
    ,   100
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_5(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   995
    ,   1000
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_9(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   999
    ,   1000
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_99(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   9999
    ,   10000
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_999(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   99999
    ,   100000
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

P99_CALL(p99_truthy_t)
p99_histogram_value_at_p99_999_9(
    p99_histogram_t const* histogram
,   uint64_t* value
)
{
    uint64_t target_rank = p99_u64_mul_div_u64_(
        histogram->event_count
    ,   999999
    ,   1000000
    );

    return value_at_target_rank(
        histogram
    ,   target_rank
    ,   value
    );
}

/* --- Utilities -------------------------------------------------------- */

static size_t
p99_histogram_bucket_index_(uint64_t time_in_ns)
{
    if (time_in_ns <= 1)
    {
        return 0;
    }
    else
    {
        return p99_floor_log2_u64_(time_in_ns);
    }
}

static p99_truthy_t
p99_histogram_bucket_range_(
    size_t index
,   uint64_t* lower
,   uint64_t* upper
)
{
    if (index >= P99_BUCKET_COUNT)
    {
        return P99_FALSE;
    }

    if (0 == index)
    {
        *lower = 0;
        *upper = 1;

        return P99_TRUE;
    }

    *lower = 1ULL << index;

    if ((P99_BUCKET_COUNT - 1) == index)
    {
        *upper = UINT64_MAX;
    }
    else
    {
        *upper = (1ULL << (index + 1)) - 1;
    }

    return P99_TRUE;
}
