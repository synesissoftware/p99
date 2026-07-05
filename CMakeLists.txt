cmake_minimum_required(VERSION 3.16)

project(p99
    VERSION 0.0.1
    DESCRIPTION "Low-cost generation of performance percentiles (p50, p90, p99, etc.)"
    HOMEPAGE_URL "https://github.com/synesissoftware/p99"
    LANGUAGES C
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# --- Options ---------------------------------------------------------------

option(P99_BUILD_SHARED "Build shared library" ON)
option(P99_BUILD_STATIC "Build static library" ON)
option(P99_BUILD_TESTS  "Build unit tests" ON)
option(P99_BUILD_EXAMPLES "Build example programs" ON)
option(P99_BUILD_BENCHMARKS "Build benchmark programs" ON)
option(P99_COMPACT_HISTOGRAM "Use compact histogram layout (<=512 bytes)" OFF)

if(NOT P99_BUILD_SHARED AND NOT P99_BUILD_STATIC)
    message(FATAL_ERROR "At least one of P99_BUILD_SHARED or P99_BUILD_STATIC must be ON")
endif()

if(WIN32)
    enable_language(RC)
endif()

# --- Library ---------------------------------------------------------------

set(P99_SOURCES src/histogram.c)
set(P99_HEADERS
    include/p99/p99.h
)

set(P99_COMPILE_OPTIONS
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Werror>
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wpedantic>
)

set(P99_LIBRARY_COMPILE_OPTIONS
    ${P99_COMPILE_OPTIONS}
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-fvisibility=hidden>
)

function(p99_configure_library target)
    target_include_directories(${target}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_compile_options(${target} PRIVATE ${P99_LIBRARY_COMPILE_OPTIONS})
    if(UNIX)
        target_link_libraries(${target} PRIVATE m)
    endif()
    if(P99_COMPACT_HISTOGRAM)
        target_compile_definitions(${target} PUBLIC P99_COMPACT_HISTOGRAM)
    endif()
    set_target_properties(${target} PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endfunction()

# In-tree executables: on Windows prefer the static library so tests and
# tools do not depend on locating p99.dll at runtime. A missing DLL on
# headless CI shows a modal dialog and never reaches main().
function(p99_link_consumer target)
    if(WIN32 AND P99_BUILD_STATIC)
        target_link_libraries(${target} PRIVATE p99_static)
    else()
        target_link_libraries(${target} PRIVATE p99::p99)
        if(WIN32 AND P99_BUILD_SHARED)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:p99_shared>"
                    "$<TARGET_FILE_DIR:${target}>"
                COMMENT "Copy p99.dll next to ${target}"
            )
        endif()
    endif()
    if(UNIX)
        target_link_libraries(${target} PRIVATE m)
    endif()
endfunction()

if(P99_BUILD_SHARED)
    add_library(p99_shared SHARED ${P99_SOURCES})
    add_library(p99::p99 ALIAS p99_shared)
    p99_configure_library(p99_shared)
    set_target_properties(p99_shared PROPERTIES OUTPUT_NAME p99)
    target_compile_definitions(p99_shared PRIVATE P99_SHARED_BUILD)
    if(WIN32)
        target_sources(p99_shared PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/p99.def"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/dllmain.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/p99.rc"
        )
    endif()
endif()

if(P99_BUILD_STATIC)
    add_library(p99_static STATIC ${P99_SOURCES})
    p99_configure_library(p99_static)
    set_target_properties(p99_static PROPERTIES OUTPUT_NAME p99)
    target_compile_definitions(p99_static INTERFACE P99_STATIC)
    if(NOT P99_BUILD_SHARED)
        add_library(p99::p99 ALIAS p99_static)
    endif()
endif()

# --- Tests -----------------------------------------------------------------

if(P99_BUILD_TESTS)
    enable_testing()

    add_executable(p99_test tests/test_histogram.c)
    p99_link_consumer(p99_test)
    target_compile_options(p99_test PRIVATE ${P99_COMPILE_OPTIONS})
    set_target_properties(p99_test PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
    add_test(NAME histogram COMMAND p99_test)
    set_tests_properties(histogram PROPERTIES TIMEOUT 120)
endif()

# --- Examples --------------------------------------------------------------

if(P99_BUILD_EXAMPLES)
    add_executable(build_histogram examples/build_histogram.c)
    p99_link_consumer(build_histogram)
    target_compile_options(build_histogram PRIVATE ${P99_COMPILE_OPTIONS})
    set_target_properties(build_histogram PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
endif()

# --- Benchmarks ------------------------------------------------------------

if(P99_BUILD_BENCHMARKS)
    add_executable(p99_benchmark benchmarks/benchmark_histogram.c)
    p99_link_consumer(p99_benchmark)
    target_compile_options(p99_benchmark PRIVATE ${P99_COMPILE_OPTIONS})
    set_target_properties(p99_benchmark PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
endif()

# --- pkg-config ------------------------------------------------------------

if(P99_COMPACT_HISTOGRAM)
    set(P99_PC_CFLAGS "-I\${includedir} -DP99_COMPACT_HISTOGRAM")
else()
    set(P99_PC_CFLAGS "-I\${includedir}")
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/p99.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/p99.pc"
    @ONLY
)

# --- Install ---------------------------------------------------------------

set(P99_INSTALL_TARGETS)

if(P99_BUILD_SHARED)
    list(APPEND P99_INSTALL_TARGETS p99_shared)
endif()

if(P99_BUILD_STATIC)
    list(APPEND P99_INSTALL_TARGETS p99_static)
endif()

install(TARGETS ${P99_INSTALL_TARGETS}
    EXPORT p99Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${P99_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/p99
)

if(WIN32 AND P99_BUILD_SHARED)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/p99.def"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/p99.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# --- CMake package config --------------------------------------------------

install(EXPORT p99Targets
    FILE p99Targets.cmake
    NAMESPACE p99::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/p99
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/p99Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/p99Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/p99
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/p99ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/p99Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/p99ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/p99
)

# --- pkg-config variable for consumers -------------------------------------

set(P99_PC_LIBS "-L\${libdir} -lp99")
