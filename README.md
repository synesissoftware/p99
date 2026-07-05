# p99 <!-- omit in toc -->

![C](https://img.shields.io/badge/C-00599C?style=flat&logo=c&logoColor=white)
[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![GitHub release](https://img.shields.io/github/v/release/synesissoftware/p99.svg)](https://github.com/synesissoftware/p99/releases/latest)
[![Last Commit](https://img.shields.io/github/last-commit/synesissoftware/p99)](https://github.com/synesissoftware/p99/commits/master)
[![CI](https://github.com/synesissoftware/p99/actions/workflows/ci.yml/badge.svg)](https://github.com/synesissoftware/p99/actions/workflows/ci.yml)

Low-cost generation of performance percentiles (p50, p90, p99, p99.9, etc.).


## Table of Contents <!-- omit in toc -->

- [Introduction](#introduction)
- [How It Works](#how-it-works)
- [Performance \& Trade-offs](#performance--trade-offs)
  - [Performance Claims](#performance-claims)
  - [Trade-offs \& Sacrifices](#trade-offs--sacrifices)
- [Building](#building)
- [Installation](#installation)
- [Using the Library](#using-the-library)
  - [CMake](#cmake)
  - [pkg-config](#pkg-config)
  - [Manual linking](#manual-linking)
- [API Overview](#api-overview)
  - [`p99_histogram_t`](#p99_histogram_t)
  - [Minimal Example](#minimal-example)
- [Examples](#examples)
- [Benchmarks](#benchmarks)
- [Project Information](#project-information)
  - [Where to get help](#where-to-get-help)
  - [Contribution guidelines](#contribution-guidelines)
  - [Dependencies](#dependencies)
  - [License](#license)


## Introduction

**p99** is a lightweight, low-overhead library designed for generating
real-time performance percentiles in high-frequency or latency-sensitive
environments.

This is the **C** implementation. A companion **Rust** implementation is
available as [p99.Rust](https://github.com/synesissoftware/p99.Rust).


## How It Works

`p99_histogram_t` is a low-overhead, zero-allocation, fixed-size structure
designed to track event durations (typically in nanoseconds) using 64
logarithmic buckets.

* **Logarithmic Bucketing**: The bucket boundaries are spaced as powers of
  two:
	* Bucket `0` represents `[0, 1]` nanoseconds;
	* Bucket `1` represents `[2, 3]` nanoseconds;
	* Bucket `2` represents `[4, 7]` nanoseconds;
	* Bucket `i` represents `[2^i, 2^(i+1) - 1]` nanoseconds.
* **Branchless Indexing**: Finding the correct bucket index for an incoming
  duration is extremely fast. It is computed in a few CPU instructions
  using the CPU's leading-zeros count intrinsic (`__builtin_clzll`).
* **Linear Interpolation**: Percentile queries iterate through the buckets
  to find the target rank and perform linear interpolation within the
  matching bucket to approximate the exact percentile duration.


## Performance & Trade-offs

### Performance Claims

* **Zero Allocation**: `p99_histogram_t` does not allocate memory on the
  heap during creation, event insertion, or percentile queries. It is a
  compact structure (**552 bytes** on 64-bit platforms by default; pass
  `-DP99_COMPACT_HISTOGRAM` when building for **296 bytes** with 32-bit
  bucket counts only) that can reside entirely on the stack or be embedded
  in other structures.
* **Ultra-Low Latency Insertion**: In the bundled benchmark, each
  `p99_histogram_push_event_time_ns` iteration re-initialises the
  histogram first (`init` + `push` per loop). That reports approximately
  **5 to 6 nanoseconds** by default, or with `P99_COMPACT_HISTOGRAM`
  (Release build, isolated benchmark, macOS, mains power).
* **Fast Reset**: The warm-reset benchmarks (`p99_histogram_init` or
  `p99_histogram_clear` on a histogram that already holds events) report
  approximately **9 to 10 nanoseconds** in either layout (`memset`-based
  initialisation). A cold stack `p99_histogram_init` is slightly faster
  (**~5 ns**) because the slot is often already zero from the prior
  iteration.
* **Blazing-Fast Queries**: Querying percentiles on a 100k-event
  histogram (such as `p99_histogram_value_at_p99`) takes approximately
  **9 to 16 nanoseconds** in either layout, depending on event
  distribution across buckets and whether the `double` percentile API or
  a named `p99` wrapper is used.
* **Instruction-Cache Friendly**: The query methods are designed with a
  "thin caller / heavy worker" pattern to prevent instruction-cache bloat
  and maintain high CPU cache locality under real-world workloads.

### Trade-offs & Sacrifices

* **Logarithmic Precision**: To achieve zero allocation and constant-time
  operations, the histogram sacrifices exact precision. It does not store
  individual event times. Instead, values are grouped into logarithmic
  buckets.
* **Approximation**: Percentile values are approximated using linear
  interpolation within the bucket boundaries. For very large values, the
  bucket width is wider, which leads to a wider approximation range.
  However, for low-latency performance measurements where precision is
  needed most (the lower nanosecond ranges), the buckets are extremely
  narrow (e.g., 1 ns, 2 ns, 4 ns wide), providing exceptional resolution.


## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
ctest --test-dir build --output-on-failure
```

Optional compact bucket counts (`uint32_t` per bucket; **296 bytes** on
64-bit; each bucket limited to `UINT32_MAX`). Library and all embedding code
must define `P99_COMPACT_HISTOGRAM`:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DP99_COMPACT_HISTOGRAM=ON
cmake --build build
ctest --test-dir build --output-on-failure
```


## Installation

```bash
cmake --install build
```

* `libp99.so` / `libp99.a` (or `.dylib` on macOS) into `lib/`;
* `p99.h` into `include/p99/`;
* `p99.pc` into `lib/pkgconfig/`;
* `p99Config.cmake` into `lib/cmake/p99/`.


## Using the Library

### CMake

```cmake
find_package(p99 REQUIRED)
target_link_libraries(myapp PRIVATE p99::p99)
```

When building from a local checkout without installing:

```cmake
add_subdirectory(path/to/p99)
target_link_libraries(myapp PRIVATE p99::p99)
```

### pkg-config

```bash
gcc myapp.c $(pkg-config --cflags --libs p99) -o myapp
```

If installed to a non-standard prefix:

```bash
export PKG_CONFIG_PATH=/opt/p99/lib/pkgconfig:$PKG_CONFIG_PATH
pkg-config --modversion p99
```

### Manual linking

```bash
gcc myapp.c -I/usr/local/include -L/usr/local/lib -lp99 -lm -o myapp
```

(Requires `#include <p99/p99.h>` in source.)


## API Overview

### `p99_histogram_t`

A low-cost, zero-allocation, 64-bucket logarithmic histogram designed for
recording event durations in nanoseconds and querying high-resolution
percentiles.

Predicate and status returns use `p99_truthy_t` (`int`): `P99_FALSE` (0) or
`P99_TRUE` (1). The public header does not require C99 `<stdbool.h>`.

Key functions:

| Category | Functions |
|----------|-----------|
| Lifecycle | `p99_histogram_init`, `p99_histogram_clear` |
| Recording | `p99_histogram_push_event_time_ns`, `_us`, `_ms`, `_s` |
| Statistics | `p99_histogram_event_count`, `_min_event_time`, `_max_event_time`, `_event_time_total` |
| Percentiles | `p99_histogram_value_at_percentile`, `_value_at_p50`, `_p75`, `_p90`, `_p95`, `_p99`, ... |

See [`include/p99/p99.h`](include/p99/p99.h) for full documentation.


### Minimal Example

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

    printf("events: %zu\n", p99_histogram_event_count(&histogram));

    if (p99_histogram_value_at_p99(&histogram, &value)) {
        printf("p99: %llu ns\n", (unsigned long long)value);
    }

    return 0;
}
```


## Examples

An example program showing histogram usage is provided in
[**examples/build_histogram.c**](examples/build_histogram.c).

It simulates a histogram of event times generated by microsecond delays
under a custom PRNG.

The number of iterations can be configured via the `P99_TRIES` environment
variable:

```bash
# Run with the default of 100 tries
./build/build_histogram

# Run with 1000 tries
P99_TRIES=1000 ./build/build_histogram
```


## Benchmarks

A hand-rolled benchmark suite mirroring [**p99.Rust**](../p99.Rust/benches/histogram.rs)
is provided in [**benchmarks/benchmark_histogram.c**](benchmarks/benchmark_histogram.c).

Build and run (use **Release** for meaningful timings):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DP99_BUILD_BENCHMARKS=ON
cmake --build build --target p99_benchmark
./build/p99_benchmark
```

Timings vary by CPU, power source, and operating system; figures in
[Performance Claims](#performance-claims) are indicative (macOS, Release,
isolated benchmark harness, mains power). Reconfigure with
`-DP99_COMPACT_HISTOGRAM=ON` to benchmark the compact layout (296-byte
struct; per-bucket counts limited to `UINT32_MAX`).

Benchmarks are not installed; they are for local development only.


## Project Information

### Where to get help

[GitHub Page](https://github.com/synesissoftware/p99 "GitHub Page")


### Contribution guidelines

Defect reports, feature requests, and pull requests are welcome on
https://github.com/synesissoftware/p99.


### Dependencies

**p99** has no runtime dependencies beyond the C standard library. The
library is built as C11; consumers need only a C89-or-later compiler for the
public header (`stddef.h`, `stdint.h`).


### License

**p99** is released under the 3-clause BSD license. See [LICENSE](./LICENSE)
for details.


<!-- ########################### end of file ########################### -->
