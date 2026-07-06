# p99 ABI and layout stability <!-- omit in toc -->

This document describes the **C** application binary interface (ABI) for
**p99**: what is stable, what may change before 1.0, and how to build
consumers so library and application agree on layout.


## Table of Contents <!-- omit in toc -->

- [Summary](#summary)
- [Versioning](#versioning)
- [Build configurations](#build-configurations)
- [`p99_histogram_t` layout](#p99_histogram_t-layout)
  - [LP64 offsets and sizes (documented for 0.1.0)](#lp64-offsets-and-sizes-documented-for-010)
- [Field semantics](#field-semantics)
- [Public C API surface](#public-c-api-surface)
- [Linking and visibility](#linking-and-visibility)
- [What is not part of the ABI](#what-is-not-part-of-the-abi)
- [Stability timeline](#stability-timeline)


## Summary

- The only public data type with a layout contract is `p99_histogram_t`
  (plus its dependent typedef `p99_bucket_count_t` and constant
  `P99_BUCKET_COUNT`);
- Two mutually exclusive layouts exist: **default** (64-bit bucket counts)
  and **compact** (`P99_COMPACT_HISTOGRAM`, 32-bit bucket counts). The
  library and every translation unit that embeds or touches a histogram must
  use the same choice;
- **0.x** releases may still adjust API or layout; each release documents
  the current layout here and in **CHANGES.md**;
- **1.0** freezes the C function signatures, exported symbol set, and both
  histogram layouts documented below. Incompatible changes after 1.0 require
  a major version bump;


## Versioning

**p99** follows [semantic versioning](https://semver.org/). See
[CONTRIBUTING.md](./CONTRIBUTING.md#release-policy) for branch, tag, and
changelog policy.

| Release line | API | `p99_histogram_t` layout |
|--------------|-----|--------------------------|
| **0.x** | May evolve | May evolve; documented per release |
| **1.0+** | Stable within major | Frozen for default and compact builds |

CMake package config uses `SameMajorVersion`: consumers built against `p99`
0.y should remain compatible within major version 0 until the 1.0 policy
applies.


## Build configurations

| Configuration | Macro | `p99_bucket_count_t` | `sizeof(p99_histogram_t)` on LP64 |
|---------------|-------|----------------------|-----------------------------------|
| **Default** | (none) | `uint64_t` | **552** bytes |
| **Compact** | `P99_COMPACT_HISTOGRAM` | `uint32_t` | **296** bytes |

Rules:

1. Define `P99_COMPACT_HISTOGRAM` consistently when compiling the **p99**
   library and all code that includes **p99/p99.h** and embeds or passes
   `p99_histogram_t`;
3. With CMake, set `-DP99_COMPACT_HISTOGRAM=ON` on the **p99** target and
   consume via `find_package(p99)` so the macro is propagated (`PUBLIC`
   compile definition);
4. With pkg-config, use the `Cflags` from `p99.pc` (compact builds add
   `-DP99_COMPACT_HISTOGRAM`);
5. Mixing layouts (e.g. library built default, application built compact) is
   undefined behaviour;

On **ILP32** (32-bit `size_t`), total struct size differs; field order and
alignment rules below still apply. CI and documented sizes assume **LP64**
(64-bit `size_t`, 64-bit pointers).


## `p99_histogram_t` layout

Source definition (**include/p99/p99.h**):

```c
typedef struct p99_histogram {
    uint8_t             has_overflowed;
    uint64_t            event_count;
    uint64_t            event_time_total;
    uint64_t            min_event_time;
    uint64_t            max_event_time;
    p99_bucket_count_t  buckets[P99_BUCKET_COUNT];
} p99_histogram_t;
```

`P99_BUCKET_COUNT` is **64** (fixed).

### LP64 offsets and sizes (documented for 0.1.0)

| Offset (bytes) | Size (bytes) | Field | Type |
|----------------|--------------|-------|------|
| 0 | 1 | `has_overflowed` | `uint8_t` |
| 1–7 | 7 | *(padding)* | — |
| 8 | 8 | `event_count` | `uint64_t` |
| 16 | 8 | `event_time_total` | `uint64_t` |
| 24 | 8 | `min_event_time` | `uint64_t` |
| 32 | 8 | `max_event_time` | `uint64_t` |
| 40 | 256 or 512 | `buckets[64]` | `p99_bucket_count_t` |

Header size (through end of `max_event_time`, start of `buckets`):
**40** bytes on LP64 (`offsetof(p99_histogram_t, buckets)`).

Total size:

- Default: 40 + 64 × 8 = **552**
- Compact: 40 + 64 × 4 = **296**

Unit tests in **tests/test_histogram.c** (`test_histogram_struct_size`)
assert these sizes on the CI platforms.


## Field semantics

Prefer the public functions over reading or writing struct fields directly.
Layout is documented so embedders (stack allocation, struct members, shared
memory) can size storage correctly.

| Field | Semantics |
|-------|-----------|
| `has_overflowed` | Non-zero after an arithmetic overflow (running total, or per-bucket limit in compact mode). Further `push` calls return `P99_FALSE`. |
| `event_count` | Number of successfully recorded events (`uint64_t`). Min/max queries succeed when `event_count > 0`. Incrementing past `UINT64_MAX` is **undefined behaviour** (not detected; does not set `has_overflowed`). |
| `event_time_total` | Sum of recorded durations (nanoseconds) while not overflowed. |
| `min_event_time` | Minimum observed duration (ns); valid when `event_count > 0`. |
| `max_event_time` | Maximum observed duration (ns); valid when `event_count > 0`. |
| `buckets[]` | Logarithmic power-of-two bucket counts; index from internal mapping of duration. |

Initialisation: `p99_histogram_init` / `p99_histogram_clear` zero the
entire struct (`memset`).


## Public C API surface

All callable entry points are declared in **include/p99/p99.h** and exported
from the shared library (see **p99.def** on Windows).

| Category | Symbols |
|----------|---------|
| Lifecycle | `p99_histogram_init`, `p99_histogram_clear` |
| Recording | `p99_histogram_push_event_time_ns`, `_us`, `_ms`, `_s` |
| Statistics | `p99_histogram_event_count`, `p99_histogram_event_time_total`, `p99_histogram_event_time_total_raw`, `p99_histogram_has_overflowed`, `p99_histogram_min_event_time`, `p99_histogram_max_event_time`, `p99_histogram_bucket_value`, `p99_histogram_buckets` |
| Percentiles | `p99_histogram_value_at_percentile`, `p99_histogram_value_at_p50`, `_p75`, `_p90`, `_p95`, `_p99`, `_p99_5`, `_p99_9`, `_p99_99`, `_p99_999`, `_p99_999_9` |

Adding functions is a **minor** release. Removing or changing signatures is
**major** (after 1.0). `P99_VER_*` macros in the header identify the
release.


## Linking and visibility

| Mode | Macro | Notes |
|------|-------|-------|
| Shared library | (default on Unix; `p99` DLL on Windows) | `P99_CALL` expands to import/export as appropriate |
| Static library | `P99_STATIC` | No import/export decorations |

Consumers must use the same **compact vs default** and **static vs
shared** choices as the **p99** build they link against.


## What is not part of the ABI

- Internal helpers in **src/** (e.g. `p99_histogram_bucket_index_`);
- Undocumented or private macros beyond those in **p99.h**;
- Histogram merge, serialisation, or on-wire formats (not yet shipped);
- Percentile approximation algorithm details (may be tuned in patch
  releases if observable results remain within documented tolerance; tests
  guard behaviour).


## Stability timeline

| Milestone | Guarantee |
|-----------|-----------|
| **0.1.0** (current) | Layout and API as documented in this file |
| **0.x** | Layout/API may change; **CHANGES.md** and this file updated each release |
| **1.0** | Freeze documented layouts (default + compact), public function set, and `P99_BUCKET_COUNT` |
| **1.x** | Patch: compatible fixes. Minor: additive API only. Major: breaking API/ABI |

There are no prior public releases; no migration guide is required for
earlier layouts.


<!-- ########################### end of file ########################### -->
