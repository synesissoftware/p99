/**
 * @file test_histogram.cpp
 * @brief Unit tests for the p99 C++ histogram wrapper.
 *
 * @copyright Copyright (c) 2026, Matthew Wilson and Synesis Information
 *   Systems
 * @license BSD-3-Clause
 */

#include <p99/p99.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <exception>

/* --- Test harness ----------------------------------------------------- */

static int g_tests_run    = 0;
static int g_tests_failed = 0;

#define TEST(fn)                                                            \
                                                                            \
    static void fn();                                                       \
    static void run_##fn()                                                  \
    {                                                                       \
        ++g_tests_run;                                                      \
        printf("  %s ... ", #fn);                                           \
        fflush(stdout);                                                     \
        fn();                                                               \
        printf("ok\n");                                                     \
    }                                                                       \
    static void fn()

#define ASSERT(cond)                                                        \
                                                                            \
    do                                                                      \
    {                                                                       \
        if (!(cond))                                                        \
        {                                                                   \
            fprintf(                                                        \
                stderr                                                      \
            ,   "\n  %s:%d: ASSERTION FAILED: %s\n"                         \
            ,   __FILE__                                                    \
            ,   __LINE__                                                    \
            ,   #cond                                                       \
            );                                                              \
            ++g_tests_failed;                                               \
                                                                            \
            return;                                                         \
        }                                                                   \
    } while (0)

/* --- Tests ------------------------------------------------------------ */

TEST(test_cpp_histogram_empty)
{
    p99::histogram histogram;

    ASSERT(histogram.empty());
    ASSERT(0 == histogram.event_count());
}

TEST(test_cpp_histogram_push_and_min_max)
{
    p99::histogram histogram;
    uint64_t       min;
    uint64_t       max;

    ASSERT(histogram.push_ns(100));
    ASSERT(histogram.push_us(2));
    ASSERT(!histogram.empty());
    ASSERT(2 == histogram.event_count());
    ASSERT(histogram.try_get_min_event_time(&min));
    ASSERT(100 == min);
    ASSERT(histogram.try_get_max_event_time(&max));
    ASSERT(2000 == max);
}

TEST(test_cpp_histogram_push_duration)
{
    p99::histogram histogram;
    uint64_t       value;

#if __cplusplus >= 201103L
    ASSERT(histogram.push_duration(std::chrono::milliseconds(3)));
    ASSERT(histogram.try_get_value_at_p50(&value));
    ASSERT(3000000 == value);
#else
    (void)value;
#endif
}

TEST(test_cpp_histogram_bucket_access)
{
    p99::histogram histogram;

    ASSERT(histogram.push_ns(1));
    ASSERT(1 == histogram[0]);
    ASSERT(1 == histogram.at(0));

    bool threw = false;

    try
    {
        (void)histogram.at(P99_BUCKET_COUNT);
    }
    catch (std::out_of_range const&)
    {
        threw = true;
    }

    ASSERT(threw);
}

#if __cplusplus >= 201703L

TEST(test_cpp_histogram_optional_min_max)
{
    p99::histogram histogram;

    ASSERT(!histogram.get_min_event_time().has_value());
    ASSERT(!histogram.get_max_event_time().has_value());

    ASSERT(histogram.push_ns(42));

    ASSERT(histogram.get_min_event_time().has_value());
    ASSERT(42 == *histogram.get_min_event_time());
    ASSERT(histogram.get_max_event_time().has_value());
    ASSERT(42 == *histogram.get_max_event_time());
}

#endif

#if __cplusplus >= 202002L

TEST(test_cpp_histogram_buckets_span)
{
    p99::histogram histogram;

    ASSERT(histogram.push_ns(4));

    std::span<p99_bucket_count_t const> const buckets = histogram.buckets();

    ASSERT(P99_BUCKET_COUNT == buckets.size());
    ASSERT(1 == buckets[2]);
}

#endif

TEST(test_cpp_histogram_clear)
{
    p99::histogram histogram;

    ASSERT(histogram.push_ns(10));
    histogram.clear();
    ASSERT(histogram.empty());
}

TEST(test_cpp_histogram_struct_size)
{
    ASSERT(sizeof(p99::histogram) == sizeof(p99_histogram_t));
}

/* --- Main ------------------------------------------------------------- */

int main()
{
    printf("p99 C++ histogram tests\n");

    run_test_cpp_histogram_empty();
    run_test_cpp_histogram_push_and_min_max();
    run_test_cpp_histogram_push_duration();
    run_test_cpp_histogram_bucket_access();
#if __cplusplus >= 201703L
    run_test_cpp_histogram_optional_min_max();
#endif
#if __cplusplus >= 202002L
    run_test_cpp_histogram_buckets_span();
#endif
    run_test_cpp_histogram_clear();
    run_test_cpp_histogram_struct_size();

    printf("\n%d tests run", g_tests_run);

    if (g_tests_failed > 0)
    {
        printf(", %d failed\n", g_tests_failed);

        return EXIT_FAILURE;
    }

    printf(", all passed\n");

    return EXIT_SUCCESS;
}
