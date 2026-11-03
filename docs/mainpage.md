# p99 {#mainpage}

**p99** records event durations in a fixed-size, zero-allocation histogram
and answers percentile queries (p50, p90, p99, and so on) from that
histogram.


## Quick start

```c
#include <p99/p99.h>
#include <stdio.h>

int main(void)
{
    p99_histogram_t histogram;
    uint64_t value;

    p99_histogram_init(&histogram);

    p99_histogram_push_event_time_ns(&histogram, 150);
    p99_histogram_push_event_time_us(&histogram, 5);
    p99_histogram_push_event_time_ms(&histogram, 10);

    printf("events: %llu\n", (unsigned long long)p99_histogram_event_count(&histogram));

    if (p99_histogram_value_at_p99(&histogram, &value)) {
        printf("p99: %llu ns\n", (unsigned long long)value);
    }

    return 0;
}
```

The C++ wrapper is `p99::histogram` in `<p99/p99.hpp>`.


## See also

* [README](https://github.com/synesissoftware/p99/blob/master/README.md)
* [p99.Rust](https://github.com/synesissoftware/p99.Rust)


<!-- ########################### end of file ########################### -->
