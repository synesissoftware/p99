/**
 * @file p99.hpp
 * @brief C++ wrapper for the p99 performance percentile histogram.
 *
 * Header-only facade over @ref p99.h. Requires a C++ compiler.
 *
 * Home: https://github.com/synesissoftware/p99
 *
 * Created: 6th July 2026
 * Updated: 6th July 2026
 *
 * @copyright Copyright (c) 2026, Matthew Wilson and Synesis Information
 *   Systems. Licensed under the 3-clause BSD License.
 */

#ifndef P99_HPP
#define P99_HPP

#ifndef __cplusplus
# error p99.hpp requires a C++ compiler
#endif

#include <p99/p99.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#if __cplusplus >= 202002L
# define P99_CPP20_OR_LATER_                                1
#endif

#if __cplusplus >= 201703L
# define P99_CPP17_OR_LATER_                                1
#endif

#if __cplusplus >= 201103L
# define P99_CPP11_OR_LATER_                                1
#endif

#if 0
#elif defined(P99_CPP17_OR_LATER_)
# if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(nodiscard)
#   define P99_NODISCARD_                                   [[nodiscard]]
#  endif
# endif
#endif

#ifndef P99_NODISCARD_
# define P99_NODISCARD_
#endif

#if defined(P99_CPP11_OR_LATER_)
# include <chrono>
# include <type_traits>
#endif

#if defined(P99_CPP17_OR_LATER_)
# include <optional>
#endif

#if defined(P99_CPP20_OR_LATER_)
# include <span>
#endif

namespace p99
{

/** C++ wrapper around @ref p99_histogram_t (composition; not inheritance).
 *
 * @note The class is not thread-safe.
 */
class histogram
{
public: // types
    /** This type. */
    typedef histogram                                       this_type;
    /** The size type. */
    typedef std::size_t                                     size_type;
private:
    p99_histogram_t                                         impl_;

public: // construction
    /** Construct a new histogram. */
    histogram()
    {
        p99_histogram_init(&impl_);
    }
#ifdef P99_CPP11_OR_LATER_

    histogram(this_type const&)            = default;
    histogram(this_type&&)                 = default;
    histogram& operator=(this_type const&) = default;
    histogram& operator=(this_type&&)      = default;
    ~histogram()                           = default;
#endif /* C++ 11+ */

public: // modifiers
    /** Clear the histogram. */
    void clear()
    {
        p99_histogram_clear(&impl_);
    }

    /** Return whether the histogram is empty. */
    P99_NODISCARD_
    bool empty() const
    {
        return 0 == p99_histogram_event_count(&impl_);
    }

    /** Push a new event time in nanoseconds. */
    bool push_ns(uint64_t time_in_ns)
    {
        return P99_FALSE != p99_histogram_push_event_time_ns(
            &impl_
        ,   time_in_ns
        );
    }

    /** Push a new event time in microseconds. */
    bool push_us(uint64_t time_in_us)
    {
        return P99_FALSE != p99_histogram_push_event_time_us(
            &impl_
        ,   time_in_us
        );
    }

    /** Push a new event time in milliseconds. */
    bool push_ms(uint64_t time_in_ms)
    {
        return P99_FALSE != p99_histogram_push_event_time_ms(
            &impl_
        ,   time_in_ms
        );
    }

    /** Push a new event time in seconds. */
    bool push_s(uint64_t time_in_s)
    {
        return P99_FALSE != p99_histogram_push_event_time_s(
            &impl_
        ,   time_in_s
        );
    }

#ifdef P99_CPP11_OR_LATER_

    /** Push a new event time from a duration. */
    template<
        typename T_rep
    ,   typename T_period
    >
    bool push_duration(
        std::chrono::duration<T_rep, T_period> const& duration
    )
    {
        typedef std::chrono::duration<T_rep, T_period>          duration_type_;

        if (duration < duration_type_::zero())
        {
            return false;
        }

        std::chrono::nanoseconds const ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

        if (ns.count() < 0)
        {
            return false;
        }

        return push_ns(static_cast<uint64_t>(ns.count()));
    }
#endif /* C++ 11+ */

public: // accessors
    /** Return the number of events recorded. */
    P99_NODISCARD_
    size_type event_count() const
    {
        return p99_histogram_event_count(&impl_);
    }

    /** Return whether the histogram has overflowed. */
    P99_NODISCARD_
    bool has_overflowed() const
    {
        return P99_FALSE != p99_histogram_has_overflowed(&impl_);
    }

    /** Return the total event time in nanoseconds. */
    P99_NODISCARD_
    bool try_get_event_time_total(uint64_t* total) const
    {
        return P99_FALSE != p99_histogram_event_time_total(&impl_, total);
    }

    /** Return the total event time in nanoseconds. */
    P99_NODISCARD_
    uint64_t event_time_total_raw() const
    {
        return p99_histogram_event_time_total_raw(&impl_);
    }

    /** Return the minimum event time in nanoseconds. */
    P99_NODISCARD_
    bool try_get_min_event_time(uint64_t* min) const
    {
        return P99_FALSE != p99_histogram_min_event_time(&impl_, min);
    }

    /** Return the maximum event time in nanoseconds. */
    P99_NODISCARD_
    bool try_get_max_event_time(uint64_t* max) const
    {
        return P99_FALSE != p99_histogram_max_event_time(&impl_, max);
    }

#ifdef P99_CPP17_OR_LATER_

    /** Return the minimum event time in nanoseconds. */
    P99_NODISCARD_
    std::optional<uint64_t> get_min_event_time() const
    {
        uint64_t min;

        if (!try_get_min_event_time(&min))
        {
            return std::nullopt;
        }

        return min;
    }

    /** Return the maximum event time in nanoseconds. */
    P99_NODISCARD_
    std::optional<uint64_t> get_max_event_time() const
    {
        uint64_t max;

        if (!try_get_max_event_time(&max))
        {
            return std::nullopt;
        }

        return max;
    }
#endif /* C++ 17+ */

    /** Return the value at the given percentile. */
    P99_NODISCARD_
    bool try_get_value_at_percentile(
        double percentile
    ,   uint64_t* value
    ) const
    {
        return P99_FALSE != p99_histogram_value_at_percentile(
            &impl_
        ,   percentile
        ,   value
        );
    }

    /** Return the value at the 50th percentile. */
    P99_NODISCARD_
    bool try_get_value_at_p50(uint64_t* value) const
    {
        return P99_FALSE != p99_histogram_value_at_p50(&impl_, value);
    }

    /** Return the value at the 99th percentile. */
    P99_NODISCARD_
    bool try_get_value_at_p99(uint64_t* value) const
    {
        return P99_FALSE != p99_histogram_value_at_p99(&impl_, value);
    }

    /** Return the value at the given bucket index.
     *
     * @param[in] index The index of the bucket to return the value of;
     *
     * @return The value at the given bucket index.
     *
     * @note The function does not perform bounds checking.
     *
     * @pre `index < P99_BUCKET_COUNT`;
     */
    P99_NODISCARD_
    uint64_t operator[](size_type index) const
    {
        assert(index < P99_BUCKET_COUNT);

        return static_cast<uint64_t>(p99_histogram_buckets(&impl_)[index]);
    }

    /** Return the value at the given bucket index.
     *
     * @param[in] index The index of the bucket to return the value of;
     *
     * @return The value at the given bucket index.
     *
     * @throw std::out_of_range If @p index is out of range.
     */
    P99_NODISCARD_
    uint64_t at(size_type index) const
    {
        if (index >= P99_BUCKET_COUNT)
        {
            throw std::out_of_range("p99::histogram::at: bucket index out of range");
        }

        return (*this)[index];
    }

#ifdef P99_CPP20_OR_LATER_

    /** Return a span of the bucket counts. */
    P99_NODISCARD_
    std::span<p99_bucket_count_t const> buckets() const
    {
        return std::span<p99_bucket_count_t const>(
            p99_histogram_buckets(&impl_)
        ,   P99_BUCKET_COUNT
        );
    }
#endif /* C++ 20+ */
};

} /* namespace p99 */

#endif /* P99_HPP */
