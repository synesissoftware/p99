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
    - [FetchContent](#fetchcontent)
    - [Via vcpkg](#via-vcpkg)
  - [pkg-config](#pkg-config)
  - [Manual linking](#manual-linking)
- [API Overview](#api-overview)
  - [`p99_histogram_t`](#p99_histogram_t)
  - [Types and constants](#types-and-constants)
  - [Functions](#functions)
    - [Lifecycle](#lifecycle)
    - [Recording](#recording)
    - [Statistics](#statistics)
    - [Percentiles](#percentiles)
  - [Minimal Example](#minimal-example)
- [Examples](#examples)
- [Benchmarks](#benchmarks)
- [API documentation](#api-documentation)
- [Project Information](#project-information)
  - [Where to get help](#where-to-get-help)
  - [Contribution guidelines](#contribution-guidelines)
  - [ABI stability](#abi-stability)
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

Disable **p99**'s own tests, examples, and benchmarks when embedding:

```cmake
set(P99_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(P99_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(P99_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
add_subdirectory(path/to/p99)
target_link_libraries(myapp PRIVATE p99::p99)
```

#### FetchContent

Download **p99** at configure time (no system install and no submodule):

```cmake
cmake_minimum_required(VERSION 3.16)
project(myapp C)

include(FetchContent)

set(P99_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(P99_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(P99_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    p99
    GIT_REPOSITORY https://github.com/synesissoftware/p99.git
    GIT_TAG        0.1.0
)
FetchContent_MakeAvailable(p99)

add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE p99::p99)
```

Pin `GIT_TAG` to a [release tag](https://github.com/synesissoftware/p99/releases)
or a commit SHA for reproducible builds. A local smoke consumer lives under
[**test/scratch/consumer_fetchcontent**](test/scratch/consumer_fetchcontent).

#### Via vcpkg

An overlay port ships in [**vcpkg/ports/p99**](vcpkg/ports/p99/) (see
[**vcpkg/README.md**](vcpkg/README.md)). Install into your vcpkg instance:

```bash
/path/to/vcpkg install p99 --overlay-ports=/path/to/p99/vcpkg/ports
```

For the latest **master** (instead of the pinned port version):

```bash
/path/to/vcpkg install p99 --overlay-ports=/path/to/p99/vcpkg/ports --head
```

Configure your project with the vcpkg toolchain file, then:

```cmake
cmake_minimum_required(VERSION 3.16)
project(myapp C)

find_package(p99 CONFIG REQUIRED)

add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE p99::p99)
```

Example:

```bash
cmake -B _build -S . \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build _build
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
percentiles. **552 bytes** on 64-bit platforms by default (**296 bytes**
with `P99_COMPACT_HISTOGRAM`). See [ABI.md](./ABI.md) for layout and
stability guarantees.

Predicate and status returns use `p99_truthy_t` (`int`): `P99_FALSE` (0) or
`P99_TRUE` (1). The public header does not require C99 `<stdbool.h>`.

### Types and constants

| Symbol | Description |
|--------|-------------|
| `p99_histogram_t` | Fixed-size histogram (stack or embed); see [ABI.md](./ABI.md) |
| `p99_pr_fp_result_t` | One floating-point percentile level and result value |
| `p99_pr_fixed_results_t` | Ten fixed percentile results (p50 through p99.9999); 80 bytes |
| `p99_bucket_count_t` | `uint64_t` per bucket (default); `uint32_t` when `P99_COMPACT_HISTOGRAM` is defined |
| `p99_truthy_t` | `int` used for predicate / status returns |
| `P99_FALSE`, `P99_TRUE` | `0` and `1` |
| `P99_BUCKET_COUNT` | Number of logarithmic buckets (`64`) |
| `P99_COMPACT_HISTOGRAM` | Optional compile flag for the 296-byte layout |
| `P99_VER_MAJOR`, `P99_VER_MINOR`, `P99_VER_PATCH` | Version components (canonical; CMake reads these from the header) |
| `P99_VER`, `P99_VER_STRING` | Composite and string forms of the version |

### Functions

#### Lifecycle

| Function | Description |
|----------|-------------|
| `p99_histogram_init` | Zero-initialise a histogram |
| `p99_histogram_clear` | Reset a histogram (same as `init`) |

#### Recording

| Function | Description |
|----------|-------------|
| `p99_histogram_push_event_time_ns` | Record a duration in nanoseconds |
| `p99_histogram_push_event_time_us` | Record a duration in microseconds |
| `p99_histogram_push_event_time_ms` | Record a duration in milliseconds |
| `p99_histogram_push_event_time_s` | Record a duration in seconds |

Returns `P99_TRUE` on success; `P99_FALSE` if overflow has already occurred,
the running total would overflow, or unit conversion would overflow.

#### Statistics

| Function | Description |
|----------|-------------|
| `p99_histogram_event_count` | Number of events recorded (`uint64_t`) |
| `p99_histogram_event_time_total` | Total duration (ns) if no overflow; `P99_FALSE` otherwise |
| `p99_histogram_event_time_total_raw` | Total duration (ns) regardless of overflow |
| `p99_histogram_has_overflowed` | Whether an arithmetic overflow occurred |
| `p99_histogram_min_event_time` | Minimum observed duration (ns); `P99_FALSE` if empty |
| `p99_histogram_max_event_time` | Maximum observed duration (ns); `P99_FALSE` if empty |
| `p99_histogram_bucket_value` | Count for bucket `index`; `P99_FALSE` if out of range |
| `p99_histogram_buckets` | Read-only pointer to `P99_BUCKET_COUNT` bucket counts |

Min/max are valid when `event_count > 0` (see [ABI.md](./ABI.md)).
Incrementing `event_count` past `UINT64_MAX` is undefined behaviour.

#### Percentiles

All percentile queries return an approximated duration in nanoseconds and
`P99_FALSE` when the histogram is empty. `p99_histogram_value_at_percentile`
clamps `percentile` to `[0.0, 100.0]`.

| Function | Description |
|----------|-------------|
| `p99_histogram_value_at_percentile` | Arbitrary percentile (`double`) |
| `p99_histogram_value_at_p50` | p50 |
| `p99_histogram_value_at_p75` | p75 |
| `p99_histogram_value_at_p90` | p90 |
| `p99_histogram_value_at_p95` | p95 |
| `p99_histogram_value_at_p99` | p99 |
| `p99_histogram_value_at_p99_5` | p99.5 |
| `p99_histogram_value_at_p99_9` | p99.9 |
| `p99_histogram_value_at_p99_99` | p99.99 |
| `p99_histogram_value_at_p99_999` | p99.999 |
| `p99_histogram_value_at_p99_999_9` | p99.9999 |
| `p99_histogram_values_at_percentiles` | Multiple arbitrary percentiles (sorted levels) |
| `p99_histogram_values_at_fixed_percentiles` | All ten fixed percentiles in one call |

Parameter and return details are in [`include/p99/p99.h`](include/p99/p99.h) and
the [API documentation](#api-documentation) (Doxygen).


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

    printf("events: %llu\n", (unsigned long long)p99_histogram_event_count(&histogram));

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


## API documentation

HTML reference for the public C API (`include/p99/p99.h`) is generated with
[Doxygen](https://www.doxygen.org/):

```bash
cmake -B build -DP99_BUILD_DOCS=ON
cmake --build build --target p99_docs
open build/docs/html/index.html
```

Requires Doxygen on `PATH` (`find_package(Doxygen)`). Documentation is not
installed; it is for local development and release publishing.


## Project Information

### Where to get help

[GitHub Page](https://github.com/synesissoftware/p99 "GitHub Page")


### Contribution guidelines

Defect reports, feature requests, and pull requests are welcome on
https://github.com/synesissoftware/p99.

See [CONTRIBUTING.md](./CONTRIBUTING.md) for development setup, coding
standards, and release policy.


### ABI stability

The C struct layout and linking rules are documented in [ABI.md](./ABI.md).
Before **1.0**, the layout may still change between **0.x** releases; each
release updates **ABI.md** and **CHANGES.md**.


### Dependencies

**p99** has no runtime dependencies beyond the C standard library. The
library is built as C11; consumers need only a C89-or-later compiler for the
public header (`stddef.h`, `stdint.h`).


### License

**p99** is released under the 3-clause BSD license. See [LICENSE](./LICENSE)
for details.


<!-- ########################### end of file ########################### -->
