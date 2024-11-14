function(configure_testing)
    if(NOT KEYVALUESTORE_BUILD_TESTS)
        return()
    endif()

    enable_testing()
    find_package(GTest REQUIRED)
    
    add_executable(keyvaluestore_tests)
    target_sources(keyvaluestore_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/interface_test.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/memory_test.cpp
            $<$<BOOL:${KEYVALUESTORE_BUILD_SQLITE}>:${CMAKE_CURRENT_SOURCE_DIR}/tests/sqlite_test.cpp>
    )
    
    target_link_libraries(keyvaluestore_tests
        PRIVATE
            keyvaluestore::keyvaluestore
            GTest::GTest
            GTest::Main
            pthread
    )
    
    include(GoogleTest)
    gtest_discover_tests(keyvaluestore_tests)
endfunction()
