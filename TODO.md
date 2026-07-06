# p99 - TODO <!-- omit in toc -->


## Table of Contents <!-- omit in toc -->

- [Build systems](#build-systems)
  - [Synesis CMake helper scripts](#synesis-cmake-helper-scripts)
  - [Other build systems](#other-build-systems)
  - [Continuous integration](#continuous-integration)
- [Functional improvements](#functional-improvements)
  - [API](#api)
  - [ABI and correctness](#abi-and-correctness)
  - [Platform](#platform)
  - [Post-1.0](#post-10)
- [Performance improvements](#performance-improvements)
- [Packaging improvements](#packaging-improvements)
  - [Consumer integration](#consumer-integration)
  - [Documentation and conventions](#documentation-and-conventions)


## Build systems

### Synesis CMake helper scripts

* [x] `prepare_cmake.sh`;
* [x] `build_cmake.sh`;
* [x] `clean_cmake.sh`;
* [x] `remove_cmake_artefacts.sh`;
* [x] `ctest_cmake.sh`;
* [x] `run_all_examples.sh`;
* [x] `run_all_benchmarks.sh`;
* [x] `-DP99_COMPACT_HISTOGRAM` selection (`--compact` / `-P` on prepare);
* [x] API documentation build (`--docs` / `-D` enabling `P99_BUILD_DOCS`,
  and `p99_docs` as a `build_cmake.sh` target);
* [x] `SIS_CMAKE_BUILD_DIR`, Debug/Release,
  disable-tests/examples/benchmarks, MinGW, MSVC-MT, `--run-make`, and
  `--help` consistent with **b64** / **BDUT** scripts;

### Other build systems

* [ ] ⏸️ Meson build (`meson.build`, matching CMake
  targets/tests/examples); deferred past v0.1.0;

### Continuous integration

* [x] Release benchmark smoke-run (no timing assertions);
* [x] Doxygen build on Linux Release;
* [ ] AddressSanitizer / UndefinedBehaviourSanitizer leg on Linux;
* [ ] Optional Debug benchmark smoke-run (currently Release only, for CI
  time);
* [x] Explicit Windows MinGW and MSVC matrix targets;


## Functional improvements

### API

* [x] C++ API — header-only `p99.hpp` (`p99::histogram`, composition over
  `p99_histogram_t`; C++11/17/20 feature gates; `p99_test_cxx`,
  `build_histogram_cxx`);

### ABI and correctness

* [x] Simplify struct flags — `has_min_event_time` / `has_max_event_time`
  removed; min/max validity follows `event_count` (layout still 552 / 296
  B on 64-bit);
* [ ] Document ABI/layout stability policy for 1.x (public
  `p99_histogram_t` is an ABI contract);
* [ ] Define and implement `event_count` overflow policy (`size_t`
  increment is currently unchecked; compact layout checks per-bucket
  `UINT32_MAX` only);
* [ ] Keep `P99_VER_*` header macros in sync with CMake `PROJECT_VERSION`
  (or document the manual update process);

### Platform

* [ ] `MSVC_NO_CRT` — freestanding MSVC build flavour with no CRT linkage
  (specialised; not a general default);

### Post-1.0

* [ ] ⏸️ Histogram merge; deferred past v0.1.0;
* [ ] ⏸️ Histogram serialisation; deferred past v1.0;


## Performance improvements

* [ ] ⏸️ binary scaling; deferred past v0.1.0;
* [ ] multi-percentile retrieval;


## Packaging improvements

### Consumer integration

* [ ] vcpkg overlay port and/or `FetchContent` consumer sample;
* [ ] `CONTRIBUTING.md` and release/versioning policy;

### Documentation and conventions

* [ ] DOC_76 comment-width checker (76 columns for comments only);
* [ ] Align file banner style with Synesis sibling libraries (if desired);
* [ ] Complete README API overview table (`bucket_value`, `buckets`, etc.);
* [ ] Fix minor `p99.h` doc typo (`reduing` → `reducing`);
* [ ] Complete Doxygen `@param` coverage for warning-free doc builds;


<!-- ########################### end of file ########################### -->
