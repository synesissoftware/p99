# p99 - TODO <!-- omit in toc -->


## Table of Contents <!-- omit in toc -->

- [Build systems](#build-systems)
- [Functional improvements](#functional-improvements)
- [Performance improvements](#performance-improvements)
- [Packaging improvements](#packaging-improvements)


## Build systems

* [ ] Meson build (`meson.build`, matching CMake targets/tests/examples);
* [ ] Synesis standard build helper scripts, including:
  * [ ]  (`prepare_cmake.sh`, `build_cmake.sh`, and `remove_cmake_artefacts.sh` per sibling libraries);
  * [ ] `-DP99_COMPACT_HISTOGRAM` selection (e.g. `--compact` / `-P` on prepare);
  * [ ]API documentation build (e.g. `--docs` / `-D` enabling `P99_BUILD_DOCS`, and `p99_docs` as a `build_cmake.sh` target);
  * [ ] `SIS_CMAKE_BUILD_DIR`, Debug/Release, disable-tests/examples, MinGW, MSVC-MT, `--run-make`, and `--help` consistent with **b64** / **BDUT** scripts;


## Functional improvements

* \<none>


## Performance improvements

* [ ] binary scaling;
* [ ] multi-percentile retrieval;


## Packaging improvements

* \<none>


<!-- ########################### end of file ########################### -->
