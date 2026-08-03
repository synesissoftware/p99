/**
 * @file build_histogram_cxx.cpp
 * @brief C++ example program for the p99 histogram wrapper.
 *
 * @copyright Copyright (c) 2026, Matthew Wilson and Synesis Information
 *   Systems
 * @license BSD-3-Clause
 */

#include <p99/p99.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>

int main()
{
    p99::histogram histogram;
    uint64_t        value;

    printf("p99 C++ histogram example\n\n");

    printf("empty: %s\n", histogram.empty() ? "yes" : "no");

    histogram.push_ns(150);
    histogram.push_us(5);
    histogram.push_ms(10);
    histogram.push_s(1);
    histogram.push_duration(std::chrono::nanoseconds(250));
    histogram.push_duration(std::chrono::microseconds(42));

    printf("empty: %s\n", histogram.empty() ? "yes" : "no");
    printf("event_count: %zu\n", histogram.event_count());

    if (histogram.try_get_min_event_time(&value))
    {
        printf("min (try_get): %llu ns\n", (unsigned long long)value);
    }

#if __cplusplus >= 201703L
    if (auto const min = histogram.get_min_event_time())
    {
        printf("min (optional): %llu ns\n", (unsigned long long)*min);
    }

    if (auto const max = histogram.get_max_event_time())
    {
        printf("max (optional): %llu ns\n", (unsigned long long)*max);
    }
#endif

    printf("bucket[0]: %llu\n", (unsigned long long)histogram[0]);
    printf("bucket at(1): %llu\n", (unsigned long long)histogram.at(1));

#if __cplusplus >= 202002L
    std::span<p99_bucket_count_t const> const buckets = histogram.buckets();

    printf("buckets span size: %zu\n", buckets.size());
#endif

    if (histogram.try_get_value_at_p99(&value))
    {
        printf("p99: %llu ns\n", (unsigned long long)value);
    }

    return EXIT_SUCCESS;
}
