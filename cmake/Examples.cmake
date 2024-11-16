function(configure_examples)
    if(NOT KEYVALUESTORE_BUILD_EXAMPLES)
        return()
    endif()

    # Memory example is always available
    add_executable(memory_example examples/memory_example/main.cpp)
    target_link_libraries(memory_example PRIVATE keyvaluestore::keyvaluestore)
    
    # Add coverage flags if enabled
    if(KEYVALUESTORE_BUILD_COVERAGE)
        target_compile_options(memory_example PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_options(memory_example PRIVATE -fprofile-arcs -ftest-coverage)
    endif()

    # SQLite example only when enabled
    if(KEYVALUESTORE_USE_SQLITE)
        add_executable(sqlite_example examples/sqlite_example/main.cpp)
        target_link_libraries(sqlite_example PRIVATE keyvaluestore::keyvaluestore)
        
        # Add coverage flags if enabled
        if(KEYVALUESTORE_BUILD_COVERAGE)
            target_compile_options(sqlite_example PRIVATE -fprofile-arcs -ftest-coverage)
            target_link_options(sqlite_example PRIVATE -fprofile-arcs -ftest-coverage)
        endif()
    endif()
endfunction()
