# p99 vcpkg overlay port

This directory contains an **overlay port** for
[**vcpkg**](https://github.com/microsoft/vcpkg). It is intended for local
testing and as a source for a future contribution to the main vcpkg registry.

## Install with overlay

From a clone of **p99** (this repository):

```bash
/path/to/vcpkg install p99 --overlay-ports=/path/to/p99/vcpkg/ports
```

To build the latest **master** instead of the pinned release in `vcpkg.json`:

```bash
/path/to/vcpkg install p99 --overlay-ports=/path/to/p99/vcpkg/ports --head
```

## Use from CMake

Pass the vcpkg toolchain file when configuring your project, then:

```cmake
find_package(p99 CONFIG REQUIRED)
target_link_libraries(myapp PRIVATE p99::p99)
```

See [README.md](../README.md#via-vcpkg) for a full consumer example.

## Maintainers

When cutting a new **p99** release:

1. Update `version` in `ports/p99/vcpkg.json` to match `P99_VER_*` in
   **include/p99/p99.h**.
2. Refresh `SHA512` in `portfile.cmake` (set to `0`, run `vcpkg install`,
   copy the computed hash).

```bash
vcpkg install p99 --overlay-ports=./vcpkg/ports
```
