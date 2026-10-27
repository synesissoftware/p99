add_executable(test.scratch.versions
    main.cpp
)

p99_configure_cxx_consumer(test.scratch.versions)

# Same attachment as the unit tests: installed BDUT::BDUT, or the bundled
# include/ when the package is absent. HAS_BDUT is defined in either case.
p99_link_bdut(test.scratch.versions)
