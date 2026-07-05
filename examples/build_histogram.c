/**
 * @file build_histogram.c
 * @brief Example program showing p99 histogram usage.
 *
 * @copyright Copyright (c) 2026, Matthew Wilson and Synesis Information
 *   Systems
 * @license BSD-3-Clause
 */

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

/* --- Simple PRNG ------------------------------------------------------ */

typedef struct {
    uint64_t state;
} simple_rng_t;

static simple_rng_t
simple_rng_new(uint64_t seed)
{
    simple_rng_t rng;
    rng.state = seed;

    return rng;
}

static uint64_t
simple_rng_next(simple_rng_t* rng)
{
    rng->state = rng->state * 6364136223846793005ULL + 1ULL;

    return rng->state;
}

/* --- Sleep helper ----------------------------------------------------- */

static void
sleep_microseconds(uint64_t us)
{
#ifdef _WIN32
    Sleep((DWORD)((us + 999) / 1000));
#else
    struct timespec ts;
    ts.tv_sec  = (time_t)(us / 1000000ULL);
    ts.tv_nsec = (long)((us % 1000000ULL) * 1000ULL);
    nanosleep(&ts, NULL);
#endif
}

/* --- Main ------------------------------------------------------------- */

int
main(void)
{
    p99_histogram_t histogram;
    simple_rng_t    rng;
    size_t          tries = 100;
    char const*     env;

    env = getenv("P99_TRIES");
    if (env != NULL)
    {
        char* end = NULL;
        long  parsed;

        parsed = strtol(env, &end, 10);
        if (end == env || *end != '\0' || parsed <= 0)
        {
            fprintf(stderr,
                    "Warning: failed to parse P99_TRIES value '%s', "
                    "defaulting to 100\n",
                    env);
        }
        else
        {
            tries = (size_t)parsed;
        }
    }

    printf("Running Histogram example with %zu tries...\n", tries);

    p99_histogram_init(&histogram);
    rng = simple_rng_new(12345);

    for (size_t i = 0; i < tries; ++i)
    {
        uint64_t delay_us;
        uint64_t elapsed_ns;

#ifdef _WIN32
        LARGE_INTEGER freq;
        LARGE_INTEGER start;
        LARGE_INTEGER end;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
#else
        struct timespec start;
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &start);
#endif

        delay_us = (simple_rng_next(&rng) % 1000) + 1;
        sleep_microseconds(delay_us);

#ifdef _WIN32
        QueryPerformanceCounter(&end);
        elapsed_ns =
            (uint64_t)(((end.QuadPart - start.QuadPart) * 1000000000ULL) /
                       (uint64_t)freq.QuadPart);
#else
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_ns = (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000ULL +
                     (uint64_t)(end.tv_nsec - start.tv_nsec);
#endif

        p99_histogram_push_event_time_ns(&histogram, elapsed_ns);
    }

    {
        int const label_width = 18;
        int const value_width = 10;

        printf("\nHistogram summary:\n");
        printf("  %-*s %*zu\n",
               label_width,
               "event_count:",
               value_width,
               p99_histogram_event_count(&histogram));

        if (p99_histogram_has_overflowed(&histogram))
        {
            printf("  %-*s %*s\n",
                   label_width,
                   "event_time_total:",
                   value_width + 3,
                   "<overflow>");
        }
        else
        {
            uint64_t total;

            p99_histogram_event_time_total(&histogram, &total);
            printf("  %-*s %*llu ns\n",
                   label_width,
                   "event_time_total:",
                   value_width,
                   (unsigned long long)total);
        }

        {
            uint64_t min;
            uint64_t max;

            if (p99_histogram_min_event_time(&histogram, &min))
            {
                printf("  %-*s %*llu ns\n",
                       label_width,
                       "min_event_time:",
                       value_width,
                       (unsigned long long)min);
            }
            if (p99_histogram_max_event_time(&histogram, &max))
            {
                printf("  %-*s %*llu ns\n",
                       label_width,
                       "max_event_time:",
                       value_width,
                       (unsigned long long)max);
            }
        }

        printf("\nPercentiles (approximated):\n");

        {
            uint64_t value;

#define PRINT_PERCENTILE(label, fn)                                         \
                                                                            \
            if (fn(&histogram, &value)) \
            { \
                printf("  %-18s %*llu ns\n", label, value_width,            \
                       (unsigned long long)value);                          \
            } \
            else \
            { \
                printf("  %-18s %*s\n", label, value_width + 3, "(none)");  \
            }

            if (p99_histogram_value_at_percentile(&histogram, 50.0, &value))
            {
                printf("  %-18s %*llu ns\n", "p50 (f64):", value_width,
                       (unsigned long long)value);
            }

            PRINT_PERCENTILE("p50 (integer):", p99_histogram_value_at_p50);
            PRINT_PERCENTILE("p75 (integer):", p99_histogram_value_at_p75);
            PRINT_PERCENTILE("p90 (integer):", p99_histogram_value_at_p90);
            PRINT_PERCENTILE("p95 (integer):", p99_histogram_value_at_p95);
            PRINT_PERCENTILE("p99 (integer):", p99_histogram_value_at_p99);
            PRINT_PERCENTILE("p99.5 (integer):", p99_histogram_value_at_p99_5);
            PRINT_PERCENTILE("p99.9 (integer):", p99_histogram_value_at_p99_9);
            PRINT_PERCENTILE("p99.99 (integer):", p99_histogram_value_at_p99_99);

#undef PRINT_PERCENTILE
        }
    }

    return EXIT_SUCCESS;
}
