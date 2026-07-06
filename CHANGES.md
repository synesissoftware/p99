# p99 - Changes


## 0.2.0-alpha1 - 7th July 2026

* batch percentile queries — `p99_histogram_values_at_percentiles` and `p99_histogram_values_at_fixed_percentiles`, with result types `p99_pr_fp_result_t` and `p99_pr_fixed_results_t` (80-byte fixed set);
* single-pass bucket walks for both batch APIs (fixed integer ranks and floating-point levels resolved in one cumulative scan);
* sequential semantics for `p99_histogram_values_at_percentiles` — levels should be ascending; a later target rank not greater than an earlier one receives the earlier result;
* benchmarks for batch percentile retrieval; unit tests including non-monotonic level order;
* **ABI.md** and **README.md** updated for new types and symbols;
* public header reorganised (types before functions; nested section banners);


## 0.1.1-alpha1 - 6th July 2026

* tidying;


## 0.1.0 - 6th July 2026

### Initial release

* C implementation of the p99 performance percentile histogram;
* 64-bucket logarithmic histogram with nanosecond precision;
* percentile queries (p50, p75, p90, p95, p99, p99.5, p99.9, p99.99, p99.999, p99.9999, and arbitrary floating-point percentiles);
* optional compact layout (`P99_COMPACT_HISTOGRAM`: 296 bytes vs 552 bytes on 64-bit; `uint32_t` bucket counts);
* public API uses `p99_truthy_t` (`int`); `event_count` is `uint64_t` (overflow past `UINT64_MAX` is undefined behaviour); struct holds `has_overflowed` only (min/max validity follows `event_count`);
* portable 64/128-bit arithmetic and bit-scan helpers (`p99_portable.h`);
* CMake build with static and shared library targets; `MSVC_USE_MT` for `/MT`;
* Windows DLL (`p99.def`, `DllGetVersion`, version resource);
* pkg-config (`p99.pc`) and CMake package config (`find_package(p99)`);
* consumer integration — `FetchContent` and vcpkg overlay port (`vcpkg/ports/p99`, `test/scratch/consumer_fetchcontent`);
* C unit tests (CTest), C example (`build_histogram`), and hand-rolled benchmarks (`p99_benchmark`);
* Doxygen API documentation (`P99_BUILD_DOCS`, `p99_docs` target);
* Synesis-standard CMake helper scripts (`prepare_cmake.sh`, `build_cmake.sh`, `clean_cmake.sh`, `remove_cmake_artefacts.sh`, `ctest_cmake.sh`, `run_all_examples.sh`, `run_all_benchmarks.sh`);
* continuous integration — Ubuntu (default and compact), macOS, Windows MinGW and MSVC; unit tests; Release benchmark smoke-run; Doxygen on Linux Release; AddressSanitizer and UndefinedBehaviourSanitizer on Linux Debug;


<!-- ########################### end of file ########################### -->
