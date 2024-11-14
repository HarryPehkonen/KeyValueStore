# Coverage configuration
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
    
    # Add coverage flags
    add_compile_options(-fprofile-arcs -ftest-coverage)
    add_link_options(-fprofile-arcs -ftest-coverage)
    
    # Custom target for coverage report
    add_custom_target(coverage
        # Clean coverage data
        COMMAND ${LCOV} --directory . --zerocounters
        
        # Run tests
        COMMAND ctest --output-on-failure
        
        # Capture coverage data
        COMMAND ${LCOV} 
            --directory . 
            --capture 
            --output-file coverage.info
        
        # Remove unwanted data
        COMMAND ${LCOV} 
            --remove coverage.info 
            '/usr/*' 
            '${CMAKE_SOURCE_DIR}/tests/*'
            '${CMAKE_SOURCE_DIR}/examples/*'
            '*/_deps/*'
            --output-file coverage.info
        
        # Generate HTML report
        COMMAND ${GENHTML} 
            coverage.info 
            --output-directory coverage_report
            --title "KeyValueStore Coverage Report"
            --legend 
            --show-details
        
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report..."
    )
    
    # Add custom target to clean coverage data
    add_custom_target(coverage_clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory coverage_report
        COMMAND ${CMAKE_COMMAND} -E remove coverage.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Cleaning coverage data..."
    )
endfunction()
