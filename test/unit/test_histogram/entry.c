/**
 * @file test/unit/test_histogram/entry.c
 * @brief Unit tests for the p99 histogram.
 *
 * Home: https://github.com/synesissoftware/p99
 *
 * Created: 4th July 2026
 * Updated: 4th August 2026
 *
 * @copyright Copyright (c) 2026, Matthew Wilson and Synesis Information
 *   Systems
 * @license BSD-3-Clause
 */

#include <p99/p99.h>

#include <bdut/bdut.h>

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Test harness (BDUT shims) ---------------------------------------- */

#define TEST(fn)                                                            \
                                                                            \
    static void fn(void);                                                   \
    static void run_##fn(void)                                              \
    {                                                                       \
        printf("  %s ... ", #fn);                                           \
        fflush(stdout);                                                     \
        fn();                                                               \
        printf("ok\n");                                                     \
    }                                                                       \
    static void fn(void)

#define ASSERT_TRUE(cond)                                   BDUT_ASSERT_TRUE(cond)
#define ASSERT_FALSE(cond)                                  BDUT_ASSERT_FALSE(cond)
#define ASSERT_EQ_U64(expected, actual)                     BDUT_ASSERT_EQ((expected), (actual))
#define ASSERT_EQ_SIZE(expected, actual)                    BDUT_ASSERT_EQ((expected), (actual))
#define ASSERT_LE_SIZE(expected, actual)                    BDUT_ASSERT_LE((expected), (actual))

/* Approximate equality is not provided by BDUT. */

static void
assert_eq_approx_u64(
    uint64_t expected
,   uint64_t actual
,   double tolerance_fraction
,   char const* file
,   int line
,   char const* func
,   char const* expected_comparand_string
,   char const* actual_comparand_string
)
{
    double const diff = fabs((double)actual - (double)expected);
    double const tolerance = fabs((double)expected * tolerance_fraction);

    if (diff > tolerance && diff > 1.0)
    {
        fprintf(
            stderr
        ,   "\n  %s:%d:%s: ASSERTION FAILED: (%s) ~= (%s)\n"
                "    expected: %llu\n"
                "    actual:   %llu\n"
                "    tolerance: %g\n"
        ,   file
        ,   line
        ,   func
        ,   expected_comparand_string
        ,   actual_comparand_string
        ,   (unsigned long long)expected
        ,   (unsigned long long)actual
        ,   tolerance
        );

        exit(EXIT_FAILURE);
    }
}

#define ASSERT_EQ_APPROX_U64(expected, actual, tolerance)                   \
                                                                            \
    assert_eq_approx_u64(                                                   \
        (uint64_t)(expected)                                                \
    ,   (uint64_t)(actual)                                                  \
    ,   (double)(tolerance)                                                 \
    ,   __FILE__                                                            \
    ,   __LINE__                                                            \
    ,   __func__                                                            \
    ,   #expected                                                           \
    ,   #actual                                                             \
    )


/* --- Tests ------------------------------------------------------------ */

TEST(test_version)
{
    ASSERT_EQ_SIZE(0, P99_VER_MAJOR);
    ASSERT_EQ_SIZE(2, P99_VER_MINOR);
    ASSERT_EQ_SIZE(1, P99_VER_PATCH);
    ASSERT_EQ_SIZE(1, P99_VER_REVISION);
    ASSERT_EQ_SIZE(0xFF, P99_VER_ALPHABETA);
    ASSERT_EQ_SIZE(0x000201FF, P99_VER);
}

TEST(test_histogram_struct_size)
{
    size_t const header_size  = offsetof(p99_histogram_t, buckets);
    size_t const buckets_size = P99_BUCKET_COUNT * sizeof(p99_bucket_count_t);

    ASSERT_EQ_SIZE(sizeof(p99_histogram_t), header_size + buckets_size);

#ifdef P99_COMPACT_HISTOGRAM
    ASSERT_LE_SIZE(512, sizeof(p99_histogram_t));
    ASSERT_EQ_SIZE(296, sizeof(p99_histogram_t));
#else
    ASSERT_LE_SIZE(576, sizeof(p99_histogram_t));
    ASSERT_EQ_SIZE(552, sizeof(p99_histogram_t));
#endif
}

TEST(test_histogram_default)
{
    p99_histogram_t h;
    uint64_t        total;
    uint64_t        bucket;

    p99_histogram_init(&h);

    ASSERT_EQ_SIZE(0, p99_histogram_event_count(&h));
    ASSERT_TRUE(p99_histogram_event_time_total(&h, &total));
    ASSERT_EQ_U64(0, total);
    ASSERT_EQ_U64(0, p99_histogram_event_time_total_raw(&h));
    ASSERT_FALSE(p99_histogram_has_overflowed(&h));
    ASSERT_FALSE(p99_histogram_min_event_time(&h, &total));
    ASSERT_FALSE(p99_histogram_max_event_time(&h, &total));

    for (size_t i = 0; i < P99_BUCKET_COUNT; ++i)
    {
        ASSERT_EQ_U64(0, p99_histogram_buckets(&h)[i]);
        ASSERT_TRUE(p99_histogram_bucket_value(&h, i, &bucket));
        ASSERT_EQ_U64(0, bucket);
    }

    ASSERT_FALSE(p99_histogram_bucket_value(&h, 64, &bucket));
}

TEST(test_histogram_bucket_placement)
{
    p99_histogram_t h;
    uint64_t        value;

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 0));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 0, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 1));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 0, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 2));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 1, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 3));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 1, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 4));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 2, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 7));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 2, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 8));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 3, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 15));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 3, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 1024));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 10, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 2047));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 10, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 1ULL << 63));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 63, &value));
    ASSERT_EQ_U64(1, value);

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, UINT64_MAX));
    ASSERT_TRUE(p99_histogram_bucket_value(&h, 63, &value));
    ASSERT_EQ_U64(1, value);

    ASSERT_FALSE(p99_histogram_bucket_value(&h, 64, &value));
}

TEST(test_histogram_push_events)
{
    p99_histogram_t h;
    uint64_t        min;
    uint64_t        max;
    uint64_t        total;

    p99_histogram_init(&h);

    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 1));
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 3));
    ASSERT_TRUE(p99_histogram_push_event_time_us(&h, 10));
    ASSERT_TRUE(p99_histogram_push_event_time_ms(&h, 5));
    ASSERT_TRUE(p99_histogram_push_event_time_s(&h, 2));
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 100));

    ASSERT_EQ_SIZE(6, p99_histogram_event_count(&h));
    ASSERT_FALSE(p99_histogram_has_overflowed(&h));
    ASSERT_TRUE(p99_histogram_min_event_time(&h, &min));
    ASSERT_EQ_U64(1, min);
    ASSERT_TRUE(p99_histogram_max_event_time(&h, &max));
    ASSERT_EQ_U64(2000000000ULL, max);
    ASSERT_TRUE(p99_histogram_event_time_total(&h, &total));
    ASSERT_EQ_U64(2005010104ULL, total);

    ASSERT_EQ_U64(1, p99_histogram_buckets(&h)[0]);
    ASSERT_EQ_U64(1, p99_histogram_buckets(&h)[1]);
    ASSERT_EQ_U64(1, p99_histogram_buckets(&h)[6]);
    ASSERT_EQ_U64(1, p99_histogram_buckets(&h)[13]);
    ASSERT_EQ_U64(1, p99_histogram_buckets(&h)[22]);
    ASSERT_EQ_U64(1, p99_histogram_buckets(&h)[30]);

    p99_histogram_clear(&h);

    ASSERT_EQ_SIZE(0, p99_histogram_event_count(&h));
    ASSERT_TRUE(p99_histogram_event_time_total(&h, &total));
    ASSERT_EQ_U64(0, total);
}

TEST(test_histogram_overflow)
{
    p99_histogram_t h;
    uint64_t        total;

    p99_histogram_init(&h);

    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, UINT64_MAX));
    ASSERT_TRUE(p99_histogram_event_time_total(&h, &total));
    ASSERT_EQ_U64(UINT64_MAX, total);
    ASSERT_FALSE(p99_histogram_has_overflowed(&h));

    ASSERT_FALSE(p99_histogram_push_event_time_ns(&h, 1));
    ASSERT_TRUE(p99_histogram_has_overflowed(&h));
    ASSERT_FALSE(p99_histogram_event_time_total(&h, &total));
    ASSERT_EQ_U64(UINT64_MAX, p99_histogram_event_time_total_raw(&h));
}

TEST(test_histogram_percentiles_empty)
{
    p99_histogram_t h;
    uint64_t        value;

    p99_histogram_init(&h);

    ASSERT_FALSE(p99_histogram_value_at_percentile(&h, 50.0, &value));
    ASSERT_FALSE(p99_histogram_value_at_p50(&h, &value));
    ASSERT_FALSE(p99_histogram_value_at_p99(&h, &value));
}

TEST(test_histogram_percentiles_single_event)
{
    p99_histogram_t h;
    uint64_t        value;

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 100));

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 0.0, &value));
    ASSERT_EQ_U64(100, value);
    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 50.0, &value));
    ASSERT_EQ_U64(100, value);
    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 99.0, &value));
    ASSERT_EQ_U64(100, value);
    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 100.0, &value));
    ASSERT_EQ_U64(100, value);

    ASSERT_TRUE(p99_histogram_value_at_p50(&h, &value));
    ASSERT_EQ_U64(100, value);
    ASSERT_TRUE(p99_histogram_value_at_p90(&h, &value));
    ASSERT_EQ_U64(100, value);
    ASSERT_TRUE(p99_histogram_value_at_p99(&h, &value));
    ASSERT_EQ_U64(100, value);
    ASSERT_TRUE(p99_histogram_value_at_p99_999_9(&h, &value));
    ASSERT_EQ_U64(100, value);
}

TEST(test_histogram_percentiles_interpolation)
{
    p99_histogram_t h;
    uint64_t        p50;
    uint64_t        p99;
    uint64_t        value;

    p99_histogram_init(&h);
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 100));
    ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, 200));

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 50.0, &p50));
    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 99.0, &p99));

    ASSERT_TRUE(p50 >= 100 && p50 <= 200);
    ASSERT_TRUE(p99 >= 100 && p99 <= 200);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 0.0, &value));
    ASSERT_EQ_U64(100, value);
    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 100.0, &value));
    ASSERT_EQ_U64(200, value);

    ASSERT_TRUE(p99_histogram_value_at_p50(&h, &value));
    ASSERT_TRUE(value >= 100);
    ASSERT_TRUE(p99_histogram_value_at_p99(&h, &value));
    ASSERT_TRUE(value <= 200);
}

TEST(test_histogram_percentiles_wide_range)
{
    p99_histogram_t h;
    uint64_t        min;
    uint64_t        max;
    uint64_t        p50;
    uint64_t        p75;
    uint64_t        p90;
    uint64_t        p95;
    uint64_t        p99;
    uint64_t        p99_5;
    uint64_t        p99_9;
    uint64_t        p99_99;
    uint64_t        p99_999;
    uint64_t        p99_999_9;

    static const uint64_t values[] = {
        1,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000,
        10000000000ULL,
    };

    p99_histogram_init(&h);

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, values[i]));
    }

    ASSERT_EQ_SIZE(sizeof(values) / sizeof(values[0]),
                 p99_histogram_event_count(&h));
    ASSERT_TRUE(p99_histogram_min_event_time(&h, &min));
    ASSERT_EQ_U64(1, min);
    ASSERT_TRUE(p99_histogram_max_event_time(&h, &max));
    ASSERT_EQ_U64(10000000000ULL, max);

    ASSERT_TRUE(p99_histogram_value_at_p50(&h, &p50));
    ASSERT_TRUE(p99_histogram_value_at_p75(&h, &p75));
    ASSERT_TRUE(p99_histogram_value_at_p90(&h, &p90));
    ASSERT_TRUE(p99_histogram_value_at_p95(&h, &p95));
    ASSERT_TRUE(p99_histogram_value_at_p99(&h, &p99));
    ASSERT_TRUE(p99_histogram_value_at_p99_5(&h, &p99_5));
    ASSERT_TRUE(p99_histogram_value_at_p99_9(&h, &p99_9));
    ASSERT_TRUE(p99_histogram_value_at_p99_99(&h, &p99_99));
    ASSERT_TRUE(p99_histogram_value_at_p99_999(&h, &p99_999));
    ASSERT_TRUE(p99_histogram_value_at_p99_999_9(&h, &p99_999_9));

    ASSERT_TRUE(p50 <= p75);
    ASSERT_TRUE(p75 <= p90);
    ASSERT_TRUE(p90 <= p95);
    ASSERT_TRUE(p95 <= p99);
    ASSERT_TRUE(p99 <= p99_5);
    ASSERT_TRUE(p99_5 <= p99_9);
    ASSERT_TRUE(p99_9 <= p99_99);
    ASSERT_TRUE(p99_99 <= p99_999);
    ASSERT_TRUE(p99_999 <= p99_999_9);

    ASSERT_TRUE(p50 >= 1);
    ASSERT_TRUE(p99_999_9 <= 10000000000ULL);
}

TEST(test_histogram_percentiles_many_events)
{
    p99_histogram_t h;
    const size_t    count = 100000;
    uint64_t        min;
    uint64_t        max;
    uint64_t        p50;
    uint64_t        p75;
    uint64_t        p90;
    uint64_t        p95;
    uint64_t        p99;
    uint64_t        p99_5;
    uint64_t        p99_9;
    uint64_t        p99_99;
    uint64_t        p99_999;
    uint64_t        p99_999_9;

    p99_histogram_init(&h);

    for (size_t i = 1; i <= count; ++i)
    {
        ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, (uint64_t)i));
    }

    ASSERT_EQ_SIZE(count, p99_histogram_event_count(&h));
    ASSERT_TRUE(p99_histogram_min_event_time(&h, &min));
    ASSERT_EQ_U64(1, min);
    ASSERT_TRUE(p99_histogram_max_event_time(&h, &max));
    ASSERT_EQ_U64(count, max);

    ASSERT_TRUE(p99_histogram_value_at_p50(&h, &p50));
    ASSERT_TRUE(p99_histogram_value_at_p90(&h, &p90));
    ASSERT_TRUE(p99_histogram_value_at_p99(&h, &p99));
    ASSERT_TRUE(p99_histogram_value_at_p99_9(&h, &p99_9));

    ASSERT_EQ_U64(50000, p50);
    ASSERT_EQ_U64(100000, p90);
    ASSERT_EQ_U64(100000, p99);
    ASSERT_EQ_U64(100000, p99_9);

    ASSERT_TRUE(p99_histogram_value_at_p75(&h, &p75));
    ASSERT_TRUE(p99_histogram_value_at_p95(&h, &p95));
    ASSERT_TRUE(p99_histogram_value_at_p99_5(&h, &p99_5));
    ASSERT_TRUE(p99_histogram_value_at_p99_99(&h, &p99_99));
    ASSERT_TRUE(p99_histogram_value_at_p99_999(&h, &p99_999));
    ASSERT_TRUE(p99_histogram_value_at_p99_999_9(&h, &p99_999_9));

    ASSERT_TRUE(p50 <= p75);
    ASSERT_TRUE(p75 <= p90);
    ASSERT_TRUE(p90 <= p95);
    ASSERT_TRUE(p95 <= p99);
    ASSERT_TRUE(p99 <= p99_5);
    ASSERT_TRUE(p99_5 <= p99_9);
    ASSERT_TRUE(p99_9 <= p99_99);
    ASSERT_TRUE(p99_99 <= p99_999);
    ASSERT_TRUE(p99_999 <= p99_999_9);
}

TEST(test_histogram_compare_float_and_int_percentiles)
{
    p99_histogram_t h;
    uint64_t        float_p50;
    uint64_t        int_p50;
    uint64_t        float_p75;
    uint64_t        int_p75;
    uint64_t        float_p90;
    uint64_t        int_p90;
    uint64_t        float_p95;
    uint64_t        int_p95;
    uint64_t        float_p99;
    uint64_t        int_p99;
    uint64_t        float_p99_5;
    uint64_t        int_p99_5;
    uint64_t        float_p99_9;
    uint64_t        int_p99_9;
    uint64_t        float_p99_99;
    uint64_t        int_p99_99;
    uint64_t        float_p99_999;
    uint64_t        int_p99_999;
    uint64_t        float_p99_999_9;
    uint64_t        int_p99_999_9;

    p99_histogram_init(&h);

    for (size_t i = 1; i <= 10000; ++i)
    {
        uint64_t val = (uint64_t)((i * i) % 1000000);
        ASSERT_TRUE(p99_histogram_push_event_time_ns(&h, val));
    }

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 50.0, &float_p50));
    ASSERT_TRUE(p99_histogram_value_at_p50(&h, &int_p50));
    ASSERT_EQ_APPROX_U64(float_p50, int_p50, 0.01);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 75.0, &float_p75));
    ASSERT_TRUE(p99_histogram_value_at_p75(&h, &int_p75));
    ASSERT_EQ_APPROX_U64(float_p75, int_p75, 0.01);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 90.0, &float_p90));
    ASSERT_TRUE(p99_histogram_value_at_p90(&h, &int_p90));
    ASSERT_EQ_APPROX_U64(float_p90, int_p90, 0.01);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 95.0, &float_p95));
    ASSERT_TRUE(p99_histogram_value_at_p95(&h, &int_p95));
    ASSERT_EQ_APPROX_U64(float_p95, int_p95, 0.01);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 99.0, &float_p99));
    ASSERT_TRUE(p99_histogram_value_at_p99(&h, &int_p99));
    ASSERT_EQ_APPROX_U64(float_p99, int_p99, 0.01);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 99.5, &float_p99_5));
    ASSERT_TRUE(p99_histogram_value_at_p99_5(&h, &int_p99_5));
    ASSERT_EQ_APPROX_U64(float_p99_5, int_p99_5, 0.01);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 99.9, &float_p99_9));
    ASSERT_TRUE(p99_histogram_value_at_p99_9(&h, &int_p99_9));
    ASSERT_EQ_APPROX_U64(float_p99_9, int_p99_9, 0.01);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 99.99, &float_p99_99));
    ASSERT_TRUE(p99_histogram_value_at_p99_99(&h, &int_p99_99));
    ASSERT_EQ_APPROX_U64(float_p99_99, int_p99_99, 0.01);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 99.999, &float_p99_999));
    ASSERT_TRUE(p99_histogram_value_at_p99_999(&h, &int_p99_999));
    ASSERT_EQ_APPROX_U64(float_p99_999, int_p99_999, 0.01);

    ASSERT_TRUE(p99_histogram_value_at_percentile(&h, 99.9999, &float_p99_999_9));
    ASSERT_TRUE(p99_histogram_value_at_p99_999_9(&h, &int_p99_999_9));
    ASSERT_EQ_APPROX_U64(float_p99_999_9, int_p99_999_9, 0.01);
}

/* --- Main ------------------------------------------------------------- */

int
main(int argc, char** argv)
{
    printf("p99 histogram tests\n");

    run_test_version();
    run_test_histogram_struct_size();
    run_test_histogram_default();
    run_test_histogram_bucket_placement();
    run_test_histogram_push_events();
    run_test_histogram_overflow();
    run_test_histogram_percentiles_empty();
    run_test_histogram_percentiles_single_event();
    run_test_histogram_percentiles_interpolation();
    run_test_histogram_percentiles_wide_range();
    run_test_histogram_percentiles_many_events();
    run_test_histogram_compare_float_and_int_percentiles();

    return BDUT_TESTS_PASSED(argc, argv);
}
