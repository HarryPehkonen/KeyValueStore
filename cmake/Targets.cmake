function(configure_targets)
    # Create main library
    add_library(keyvaluestore
        src/KeyValueStore.cpp
        src/memory/MemoryKeyValueStore.cpp
    )
    add_library(keyvaluestore::keyvaluestore ALIAS keyvaluestore)

    target_include_directories(keyvaluestore
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    # Add coverage flags if enabled
    if(KEYVALUESTORE_BUILD_COVERAGE)
        target_compile_options(keyvaluestore PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_options(keyvaluestore PRIVATE -fprofile-arcs -ftest-coverage)
    endif()

    # SQLite implementation
    if(KEYVALUESTORE_USE_SQLITE)
        find_package(SQLite3 REQUIRED)
        
        target_sources(keyvaluestore
            PRIVATE
                src/sqlite/SQLiteKeyValueStore.cpp
        )
        
        target_link_libraries(keyvaluestore
            PUBLIC
                SQLite::SQLite3
        )
    endif()
endfunction()
