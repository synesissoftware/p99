#include <p99/p99.h>
#include <stdio.h>

int
main(void)
{
    p99_histogram_t histogram;
    uint64_t        value;

    p99_histogram_init(&histogram);
    p99_histogram_push_event_time_ns(&histogram, 100);
    p99_histogram_push_event_time_ns(&histogram, 200);

    if (p99_histogram_value_at_p50(&histogram, &value))
    {
        printf(
            "p50: %llu ns (%llu events)\n"
        ,   value
        ,   p99_histogram_event_count(&histogram)
        );

        return 0;
    }

    fputs("p50 query failed\n", stderr);
    return 1;
}
