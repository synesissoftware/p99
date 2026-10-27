add_executable(test.scratch.versions
    main.cpp
)

p99_configure_cxx_consumer(test.scratch.versions)

if(TARGET BDUT::BDUT)

	target_link_libraries(test.scratch.versions
		PRIVATE
			BDUT::BDUT
	)
endif()
