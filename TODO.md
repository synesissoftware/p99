# p99 - TODO <!-- omit in toc -->


## Table of Contents <!-- omit in toc -->

- [Build systems](#build-systems)
- [Functional improvements](#functional-improvements)
- [Performance improvements](#performance-improvements)
- [Packaging improvements](#packaging-improvements)


## Build systems

* [x] Synesis standard helper scripts:
  * [x] `prepare_cmake.sh`;
  * [x] `build_cmake.sh`;
  * [x] `clean_cmake.sh`;
  * [x] `remove_cmake_artefacts.sh`;
  * [x] `ctest_cmake.sh`;
  * [x] `run_all_examples.sh`;
  * [x] `run_all_benchmarks.sh`;
  * [x] `-DP99_COMPACT_HISTOGRAM` selection (`--compact` / `-P` on prepare);
  * [x] API documentation build (`--docs` / `-D` enabling `P99_BUILD_DOCS`, and
    `p99_docs` as a `build_cmake.sh` target);
  * [x] `SIS_CMAKE_BUILD_DIR`, Debug/Release, disable-tests/examples/benchmarks,
    MinGW, MSVC-MT, `--run-make`, and `--help` consistent with **b64** / **BDUT**
    scripts;
* [ ] Meson build (`meson.build`, matching CMake targets/tests/examples);


## Functional improvements

* [ ] C++ API;
* [ ] `MSVC_NO_CRT` - no CRT dependencies at all, useful for very tight DLL;


## Performance improvements

* [ ] binary scaling;
* [ ] multi-percentile retrieval;


## Packaging improvements

* \<none>


<!-- ########################### end of file ########################### -->
