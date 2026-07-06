cmake_minimum_required(VERSION 3.16)

project(p99_consumer_fetchcontent C)

include(FetchContent)

set(P99_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(P99_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(P99_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(P99_BUILD_DOCS OFF CACHE BOOL "" FORCE)

# Local smoke test: consume this repository checkout without a network fetch.
# For downstream projects, use GIT_REPOSITORY / GIT_TAG (see README.md).
FetchContent_Declare(
    p99
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.."
)
FetchContent_MakeAvailable(p99)

add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE p99::p99)
