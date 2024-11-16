function(configure_coverage)
    if(NOT KEYVALUESTORE_BUILD_COVERAGE)
        return()
    endif()

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(WARNING "Code coverage results with an optimized (non-Debug) build may be misleading")
    endif()
    
    # Find required tools
    find_program(LCOV lcov REQUIRED)
    find_program(GENHTML genhtml REQUIRED)
    
    if(NOT KEYVALUESTORE_BUILD_TESTS)
        message(FATAL_ERROR "Tests must be enabled for coverage reporting (KEYVALUESTORE_BUILD_TESTS=ON)")
    endif()

    # Create coverage directory in source tree
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/coverage")
    
    # Custom target for coverage report
    add_custom_target(coverage
        # Clean coverage data
        COMMAND ${CMAKE_COMMAND} -E echo "Cleaning previous coverage data..."
        COMMAND ${LCOV} --directory . --zerocounters
        
        # Run tests directly (don't use CTest)
        COMMAND ${CMAKE_COMMAND} -E echo "Running tests..."
        COMMAND $<TARGET_FILE:keyvaluestore_tests>
        
        # Capture coverage data
        COMMAND ${CMAKE_COMMAND} -E echo "Capturing coverage data..."
        COMMAND ${LCOV} 
            --directory .
            --capture 
            --output-file "${CMAKE_SOURCE_DIR}/coverage/coverage.info"
            --rc lcov_branch_coverage=1
        
        # Remove unwanted data
        COMMAND ${CMAKE_COMMAND} -E echo "Filtering coverage data..."
        COMMAND ${LCOV} 
            --remove "${CMAKE_SOURCE_DIR}/coverage/coverage.info"
            '/usr/*' 
            '${CMAKE_SOURCE_DIR}/tests/*'
            '${CMAKE_SOURCE_DIR}/examples/*'
            '*/_deps/*'
            --output-file "${CMAKE_SOURCE_DIR}/coverage/coverage.info"
        
        # Generate HTML report
        COMMAND ${CMAKE_COMMAND} -E echo "Generating HTML report..."
        COMMAND ${GENHTML} 
            "${CMAKE_SOURCE_DIR}/coverage/coverage.info"
            --output-directory "${CMAKE_SOURCE_DIR}/coverage"
            --title "KeyValueStore Coverage Report"
            --legend 
            --show-details
            --branch-coverage
        
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report..."
        DEPENDS keyvaluestore_tests
    )
    
    # Add custom target to clean coverage data
    add_custom_target(coverage_clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/coverage"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Cleaning coverage data..."
    )

    message(STATUS "Coverage reports will be generated in ${CMAKE_SOURCE_DIR}/coverage")
endfunction()
