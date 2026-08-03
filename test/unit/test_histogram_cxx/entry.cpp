/**
 * @file test/unit/test_histogram_cxx/entry.cpp
 * @brief Unit tests for the p99 C++ histogram wrapper.
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

#include <p99/p99.hpp>

#include <bdut/bdut.h>

#include <chrono>
#include <exception>


/* --- Tests ------------------------------------------------------------ */

int main(int argc, char** argv)
{
    /* empty */
    {
        p99::histogram histogram;

        BDUT_ASSERT_TRUE(histogram.empty());
        BDUT_ASSERT_EQ(0, histogram.event_count());
    }

    /* push and min/max */
    {
        p99::histogram histogram;
        uint64_t       min;
        uint64_t       max;

        BDUT_ASSERT_TRUE(histogram.push_ns(100));
        BDUT_ASSERT_TRUE(histogram.push_us(2));
        BDUT_ASSERT_FALSE(histogram.empty());
        BDUT_ASSERT_EQ(2, histogram.event_count());
        BDUT_ASSERT_TRUE(histogram.try_get_min_event_time(&min));
        BDUT_ASSERT_EQ(100, min);
        BDUT_ASSERT_TRUE(histogram.try_get_max_event_time(&max));
        BDUT_ASSERT_EQ(2000, max);
    }

    /* push duration */
    {
        p99::histogram histogram;
        uint64_t       value;

#if __cplusplus >= 201103L
        BDUT_ASSERT_TRUE(histogram.push_duration(std::chrono::milliseconds(3)));
        BDUT_ASSERT_TRUE(histogram.try_get_value_at_p50(&value));
        BDUT_ASSERT_EQ(3000000, value);
#else
        (void)value;
#endif
    }

    /* bucket access */
    {
        p99::histogram histogram;

        BDUT_ASSERT_TRUE(histogram.push_ns(1));
        BDUT_ASSERT_EQ(1, histogram[0]);
        BDUT_ASSERT_EQ(1, histogram.at(0));

        bool threw = false;

        try
        {
            (void)histogram.at(P99_BUCKET_COUNT);
        }
        catch (std::out_of_range const&)
        {
            threw = true;
        }

        BDUT_ASSERT_TRUE(threw);
    }

#if __cplusplus >= 201703L

    /* optional min/max */
    {
        p99::histogram histogram;

        BDUT_ASSERT_FALSE(histogram.get_min_event_time().has_value());
        BDUT_ASSERT_FALSE(histogram.get_max_event_time().has_value());

        BDUT_ASSERT_TRUE(histogram.push_ns(42));

        BDUT_ASSERT_TRUE(histogram.get_min_event_time().has_value());
        BDUT_ASSERT_EQ(42, *histogram.get_min_event_time());
        BDUT_ASSERT_TRUE(histogram.get_max_event_time().has_value());
        BDUT_ASSERT_EQ(42, *histogram.get_max_event_time());
    }

#endif

#if __cplusplus >= 202002L

    /* buckets span */
    {
        p99::histogram histogram;

        BDUT_ASSERT_TRUE(histogram.push_ns(4));

        std::span<p99_bucket_count_t const> const buckets = histogram.buckets();

        BDUT_ASSERT_EQ(P99_BUCKET_COUNT, buckets.size());
        BDUT_ASSERT_EQ(1, buckets[2]);
    }

#endif

    /* clear */
    {
        p99::histogram histogram;

        BDUT_ASSERT_TRUE(histogram.push_ns(10));
        histogram.clear();
        BDUT_ASSERT_TRUE(histogram.empty());
    }

    /* struct size */
    {
        BDUT_ASSERT_EQ(sizeof(p99::histogram), sizeof(p99_histogram_t));
    }

    return BDUT_TESTS_PASSED(argc, argv);
}
