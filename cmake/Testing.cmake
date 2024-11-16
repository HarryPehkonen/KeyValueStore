function(configure_testing)
    if(NOT KEYVALUESTORE_BUILD_TESTS)
        return()
    endif()

    enable_testing()
    find_package(GTest REQUIRED)
    
    # Create test executable
    add_executable(keyvaluestore_tests)
    
    # Always include base tests
    target_sources(keyvaluestore_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/interface_test.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/memory_test.cpp
    )
    
    # Conditionally include SQLite tests
    if(KEYVALUESTORE_USE_SQLITE)
        target_sources(keyvaluestore_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/sqlite_test.cpp
        )
    endif()
    
    target_link_libraries(keyvaluestore_tests
        PRIVATE
            keyvaluestore::keyvaluestore
            GTest::GTest
            GTest::Main
    )
    
    # Add coverage flags if enabled
    if(KEYVALUESTORE_BUILD_COVERAGE)
        target_compile_options(keyvaluestore_tests PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_options(keyvaluestore_tests PRIVATE -fprofile-arcs -ftest-coverage)
    endif()
    
    # Register tests with CTest
    add_test(
        NAME AllTests
        COMMAND keyvaluestore_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Make this test discoverable by CTest
    set_tests_properties(AllTests PROPERTIES
        LABELS "UnitTests"
        ENVIRONMENT "GTEST_COLOR=1"
    )
endfunction()
