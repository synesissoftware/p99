# Contributing to p99 <!-- omit in toc -->

Thank you for your interest in **p99**. This document covers how to report
issues, propose changes, validate them locally, and how releases are made.


## Table of Contents <!-- omit in toc -->

- [Code of conduct](#code-of-conduct)
- [Reporting issues](#reporting-issues)
- [Pull requests](#pull-requests)
- [Development setup](#development-setup)
- [Coding standards](#coding-standards)
- [Documentation](#documentation)
- [Release policy](#release-policy)


## Code of conduct

Be respectful and constructive. Defect reports, feature requests, and pull
requests are welcome on [GitHub](https://github.com/synesissoftware/p99).


## Reporting issues

Use [GitHub Issues](https://github.com/synesissoftware/p99/issues). Include:

- **p99** version (from `P99_VER_*` in **include/p99/p99.h** or the release
  tag);
- Operating system and architecture;
- Compiler and version (e.g. Apple Clang 17, GCC 13, MSVC 2022);
- **CMake** version (if relevant);
- Whether `P99_COMPACT_HISTOGRAM` was defined when building;
- Minimal reproduction steps or a link to a branch;

For build failures, attach the configure/build log. For behavioural issues,
show expected vs actual results.


## Pull requests

1. Fork the repository and create a branch from **dev** (or the current
   integration branch);
2. Make focused changes; avoid unrelated formatting or drive-by edits;
3. Update **CHANGES.md** for user-visible changes;
4. Ensure the project builds and tests pass locally (see below);
5. Open a pull request with a clear description of the problem and solution;

The initial **C** release is developed on **dev**. A **C++** wrapper
(`p99.hpp`) is maintained on a separate branch and is not part of the first
release.

Licensing: contributions are accepted under the same
[3-clause BSD license](./LICENSE) as the project.


## Development setup

**p99** has no third-party runtime dependencies. To configure, build, and
test:

```bash
./prepare_cmake.sh -m
./ctest_cmake.sh -M
```

Optional:

```bash
./run_all_examples.sh -M
./run_all_benchmarks.sh -M
```

Compact histogram layout (296-byte struct on 64-bit):

```bash
./prepare_cmake.sh -P -m
./ctest_cmake.sh -M
```

API documentation (requires Doxygen):

```bash
./prepare_cmake.sh -D -m
./build_cmake.sh p99_docs
```

Environment variables used by the Synesis CMake helper scripts:

| Variable | Purpose |
|----------|---------|
| `SIS_CMAKE_BUILD_DIR` | Override default `_build` output directory |
| `SIS_CMAKE_MAKE_COMMAND` | Override `make` / `mingw32-make.exe` |

Continuous integration runs on push and pull request; all checks should pass
before merge.


## Coding standards

- **C** sources are built with strict warnings as errors (`-Werror`) on GCC
  and Clang;
- Match existing layout: **Allman** braces; pointers as `type* name`;
  east const (`type const*`);
- Public API lives in **include/p99/p99.h** with Doxygen comments;
- Internal helpers stay in **src/** and are not installed;
- Comments follow the **DOC_76** convention (76 columns maximum for comment
  text only; code lines are not limited);
- Predicate returns use `p99_truthy_t` (`int`); the public header does not
  require C99 `<stdbool.h>`;
- Version bumps: update `P99_VER_*` in **p99.h** and **CMakeLists.txt**
  `PROJECT_VERSION` together;


## Documentation

When adding or changing behaviour, update as appropriate:

- [README.md](./README.md) — overview, building, API summary;
- [CHANGES.md](./CHANGES.md) — release notes (not the backlog);
- [TODO.md](./TODO.md) — planned work and deferrals;
- [ABI.md](./ABI.md) — layout and ABI stability;

Regenerate HTML API docs locally after header changes if you use Doxygen
(`./build_cmake.sh p99_docs`).


## Release policy

**p99** uses [semantic versioning](https://semver.org/):

| Component | Meaning |
|-----------|---------|
| **Major** | Incompatible API or ABI change (after 1.0) |
| **Minor** | Backward-compatible features |
| **Patch** | Backward-compatible fixes |

**Branches and tags**

- **dev** — integration branch for the current release line;
- Release tags are created on **dev** (or **master** once promoted);
- Annotated git tags match **CHANGES.md** entries (e.g. `0.1.0`);

**Changelog discipline**

- **CHANGES.md** — what shipped in each release;
- **TODO.md** — what remains; use ⏸️ for items deferred past the current
  release;

**0.x vs 1.0**

- **0.x** — API and layout may still evolve; each release documents the
  current `p99_histogram_t` layout (see [ABI.md](./ABI.md));
- **1.0** — C API and struct layout frozen for the default and compact
  build configurations; incompatible changes require a major version bump;

**CMake package compatibility**

- `p99ConfigVersion.cmake` uses `SameMajorVersion` — consumers linking
  against `p99` 0.y should remain compatible within major version 0 until
  1.0 policy applies.


<!-- ########################### end of file ########################### -->
