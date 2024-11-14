function(configure_examples)
    if(NOT KEYVALUESTORE_BUILD_EXAMPLES)
        return()
    endif()

    # Memory example
    if(KEYVALUESTORE_BUILD_MEMORY)
        add_executable(memory_example examples/memory_example/main.cpp)
        target_link_libraries(memory_example PRIVATE keyvaluestore::memory)
    endif()

    # SQLite example
    if(KEYVALUESTORE_BUILD_SQLITE)
        add_executable(sqlite_example examples/sqlite_example/main.cpp)
        target_link_libraries(sqlite_example PRIVATE keyvaluestore::sqlite)
    endif()
endfunction()
