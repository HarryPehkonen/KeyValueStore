function(configure_targets)
    # Create main interface library
    add_library(keyvaluestore INTERFACE)
    add_library(keyvaluestore::keyvaluestore ALIAS keyvaluestore)

    target_include_directories(keyvaluestore
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    # Memory implementation
    if(KEYVALUESTORE_BUILD_MEMORY)
        add_library(keyvaluestore_memory INTERFACE)
        add_library(keyvaluestore::memory ALIAS keyvaluestore_memory)
        
        target_include_directories(keyvaluestore_memory
            INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
        )
        
        target_link_libraries(keyvaluestore
            INTERFACE
                keyvaluestore::memory
        )
        
        target_compile_definitions(keyvaluestore
            INTERFACE
                KEYVALUESTORE_HAS_MEMORY=1
        )
    endif()

    # SQLite implementation
    if(KEYVALUESTORE_BUILD_SQLITE)
        find_package(SQLite3 REQUIRED)
        
        add_library(keyvaluestore_sqlite STATIC
            src/sqlite/SQLiteKeyValueStore.cpp
        )
        add_library(keyvaluestore::sqlite ALIAS keyvaluestore_sqlite)
        
        target_include_directories(keyvaluestore_sqlite
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
        )
        
        target_link_libraries(keyvaluestore_sqlite
            PUBLIC
                SQLite::SQLite3
        )
        
        target_link_libraries(keyvaluestore
            INTERFACE
                keyvaluestore::sqlite
        )
        
        target_compile_definitions(keyvaluestore
            INTERFACE
                KEYVALUESTORE_HAS_SQLITE=1
        )
    endif()
endfunction()
