function(configure_build_options)
    option(KEYVALUESTORE_USE_SQLITE "Build with SQLite backend support" OFF)
    option(KEYVALUESTORE_BUILD_TESTS "Build the tests" ON)
    option(KEYVALUESTORE_BUILD_EXAMPLES "Build example applications" ON)
    option(KEYVALUESTORE_BUILD_DOCS "Build documentation" ON)
    option(KEYVALUESTORE_BUILD_COVERAGE "Build with coverage information" OFF)

    # Make the SQLite option available to the code
    if(KEYVALUESTORE_USE_SQLITE)
        add_compile_definitions(KEYVALUESTORE_USE_SQLITE)
    endif()
endfunction()
