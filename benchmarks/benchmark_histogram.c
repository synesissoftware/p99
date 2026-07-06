/**
 * @file benchmark_histogram.c
 * @brief Performance benchmarks for the p99 histogram.
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

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
# define _POSIX_C_SOURCE 200809L
#endif

#include <p99/p99.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

/* --- Anti-optimisation sink ------------------------------------------- */

static volatile uint64_t g_benchmark_sink;

static void
benchmark_sink_u64(uint64_t value)
{
    g_benchmark_sink = value;
}

static void
benchmark_sink_truthy(p99_truthy_t value)
{
    g_benchmark_sink = value ? 1ULL : 0ULL;
}

/* --- Timer ------------------------------------------------------------ */

static uint64_t
benchmark_time_ns(void)
{
#ifdef _WIN32
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER counter;

    if (0 == frequency.QuadPart)
    {
        QueryPerformanceFrequency(&frequency);
    }

    QueryPerformanceCounter(&counter);

    return (uint64_t)((counter.QuadPart * 1000000000ULL) /
                        (uint64_t)frequency.QuadPart);
#else
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
}

/* --- Harness ---------------------------------------------------------- */

typedef struct {
    char const* name;
    size_t      warmup_iterations;
    size_t      measure_iterations;
    void (*setup)(void);
    void (*run)(void);
} benchmark_def_t;

static void
benchmark_run(
    benchmark_def_t const* def
,   int name_width
,   int ns_width
,   int iters_width
)
{
    uint64_t start_ns;
    uint64_t end_ns;
    uint64_t elapsed_ns;
    double   ns_per_op;
    size_t   i;

    for (i = 0; i < def->warmup_iterations; ++i)
    {
        if (def->setup != NULL)
        {
            def->setup();
        }

        def->run();
    }

    start_ns = benchmark_time_ns();

    for (i = 0; i < def->measure_iterations; ++i)
    {
        if (def->setup != NULL)
        {
            def->setup();
        }

        def->run();
    }

    end_ns     = benchmark_time_ns();
    elapsed_ns = end_ns - start_ns;
    ns_per_op  = (double)elapsed_ns / (double)def->measure_iterations;

    printf("  %-*s %*.*f  %*zu\n",
           name_width,
           def->name,
           ns_width,
           2,
           ns_per_op,
           iters_width,
           def->measure_iterations);
}

/* --- Histogram builders ----------------------------------------------- */

static void
build_sequential_histogram(p99_histogram_t* histogram)
{
    p99_histogram_init(histogram);

    for (size_t i = 1; i <= 100000; ++i)
    {
        p99_histogram_push_event_time_ns(histogram, (uint64_t)i * 10ULL);
    }
}

static void
build_wide_range_histogram(p99_histogram_t* histogram)
{
    uint64_t state = 12345;

    p99_histogram_init(histogram);

    for (size_t i = 0; i < 100000; ++i)
    {
        uint64_t val;

        state = state * 6364136223846793005ULL + 1ULL;
        val   = (state % 10000000000ULL) + 1ULL;
        p99_histogram_push_event_time_ns(histogram, val);
    }
}

/* --- Benchmark workloads ---------------------------------------------- */

static void
BENCHMARK_histogram_init_cold(void)
{
    p99_histogram_t histogram;

    p99_histogram_init(&histogram);
}

static p99_histogram_t g_push_histogram;
static p99_histogram_t g_reset_histogram;

static void
BENCHMARK_push_event_time_ns_setup(void)
{
    p99_histogram_init(&g_push_histogram);
}

static void
BENCHMARK_push_event_time_ns(void)
{
    benchmark_sink_truthy(
        p99_histogram_push_event_time_ns(&g_push_histogram, 12345)
    );
}

static void
BENCHMARK_warm_histogram_setup(void)
{
    p99_histogram_init(&g_reset_histogram);
    p99_histogram_push_event_time_ns(&g_reset_histogram, 100);
    p99_histogram_push_event_time_ns(&g_reset_histogram, 200);
}

static void
BENCHMARK_histogram_init_warm(void)
{
    p99_histogram_init(&g_reset_histogram);
}

static void
BENCHMARK_clear(void)
{
    p99_histogram_clear(&g_reset_histogram);
}

static p99_histogram_t g_seq_histogram;
static p99_histogram_t g_wide_histogram;

static void
BENCHMARK_value_at_percentile_99_0_seq(void)
{
    uint64_t value;

    p99_histogram_value_at_percentile(&g_seq_histogram, 99.0, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_p99_seq(void)
{
    uint64_t value;

    p99_histogram_value_at_p99(&g_seq_histogram, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_percentile_50_0_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_percentile(&g_wide_histogram, 50.0, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_p50_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_p50(&g_wide_histogram, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_percentile_75_0_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_percentile(&g_wide_histogram, 75.0, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_p75_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_p75(&g_wide_histogram, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_percentile_90_0_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_percentile(&g_wide_histogram, 90.0, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_p90_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_p90(&g_wide_histogram, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_percentile_99_0_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_percentile(&g_wide_histogram, 99.0, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_p99_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_p99(&g_wide_histogram, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_percentile_99_99_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_percentile(&g_wide_histogram, 99.99, &value);
    benchmark_sink_u64(value);
}

static void
BENCHMARK_value_at_p99_99_wide(void)
{
    uint64_t value;

    p99_histogram_value_at_p99_99(&g_wide_histogram, &value);
    benchmark_sink_u64(value);
}

/* --- Main ------------------------------------------------------------- */

int
main(void)
{
    static const benchmark_def_t benchmarks[] = {
        {
            "`p99_histogram_init()` [stack, cold]",
            100000,
            10000000,
            NULL,
            BENCHMARK_histogram_init_cold,
        },
        {
            "`p99_histogram_push_event_time_ns()`",
            10000,
            1000000,
            BENCHMARK_push_event_time_ns_setup,
            BENCHMARK_push_event_time_ns,
        },
        {
            "`p99_histogram_init()` [warm histogram]",
            10000,
            1000000,
            BENCHMARK_warm_histogram_setup,
            BENCHMARK_histogram_init_warm,
        },
        {
            "`p99_histogram_clear()` [warm histogram]",
            10000,
            1000000,
            BENCHMARK_warm_histogram_setup,
            BENCHMARK_clear,
        },
        {
            "`p99_histogram_value_at_percentile(99.0)` [100k events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_percentile_99_0_seq,
        },
        {
            "`p99_histogram_value_at_p99()` [100k events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_p99_seq,
        },
        {
            "`p99_histogram_value_at_percentile(50.0)` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_percentile_50_0_wide,
        },
        {
            "`p99_histogram_value_at_p50()` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_p50_wide,
        },
        {
            "`p99_histogram_value_at_percentile(75.0)` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_percentile_75_0_wide,
        },
        {
            "`p99_histogram_value_at_p75()` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_p75_wide,
        },
        {
            "`p99_histogram_value_at_percentile(90.0)` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_percentile_90_0_wide,
        },
        {
            "`p99_histogram_value_at_p90()` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_p90_wide,
        },
        {
            "`p99_histogram_value_at_percentile(99.0)` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_percentile_99_0_wide,
        },
        {
            "`p99_histogram_value_at_p99()` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_p99_wide,
        },
        {
            "`p99_histogram_value_at_percentile(99.99)` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_percentile_99_99_wide,
        },
        {
            "`p99_histogram_value_at_p99_99()` [100k wide-range events]",
            1000,
            100000,
            NULL,
            BENCHMARK_value_at_p99_99_wide,
        },
    };

    size_t i;
    int const name_width  = 68;
    int const ns_width    = 10;
    int const iters_width = 10;

    printf("p99 histogram benchmarks\n");
    printf("(build with -DCMAKE_BUILD_TYPE=Release for meaningful results)\n\n");

    build_sequential_histogram(&g_seq_histogram);
    build_wide_range_histogram(&g_wide_histogram);

    printf("  %-*s %*s  %*s\n",
           name_width,
           "benchmark",
           ns_width,
           "ns/op",
           iters_width,
           "iters");

    for (i = 0; i < sizeof(benchmarks) / sizeof(benchmarks[0]); ++i)
    {
        benchmark_run(
            &benchmarks[i]
        ,   name_width
        ,   ns_width
        ,   iters_width
        );
    }

    return EXIT_SUCCESS;
}
