# p99 API Reference

Low-cost generation of performance percentiles (p50, p90, p99, p99.9, etc.).

**p99** is a lightweight C library for recording event durations (typically in
nanoseconds) in a fixed-size, zero-allocation histogram and querying
high-resolution percentiles in real time.

## Design

- **64 logarithmic buckets** spaced as powers of two (`P99_BUCKET_COUNT`).
- **Zero heap allocation** — `p99_histogram_t` is embeddable on the stack
  (552 bytes by default on 64-bit platforms; 296 bytes with
  `P99_COMPACT_HISTOGRAM`).
- **Branchless bucket indexing** via leading-zero count on supported
  platforms.
- **Linear interpolation** within bucket boundaries for percentile queries.

## Predicate returns

Functions that report success or failure return `p99_truthy_t` (`int`):
`P99_FALSE` (0) or `P99_TRUE` (1). The public header does not require
`<stdbool.h>`.

## See also

- [README](https://github.com/synesissoftware/p99/blob/master/README.md)
- [p99.Rust](https://github.com/synesissoftware/p99.Rust) companion
  implementation
