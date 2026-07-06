vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO synesissoftware/p99
    REF "${VERSION}"
    SHA512 0
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DP99_BUILD_TESTS=OFF
        -DP99_BUILD_EXAMPLES=OFF
        -DP99_BUILD_BENCHMARKS=OFF
        -DP99_BUILD_DOCS=OFF
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME p99
    CONFIG_PATH "lib/cmake/p99"
)

vcpkg_fixup_pkgconfig()

vcpkg_copy_pdbs()

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
