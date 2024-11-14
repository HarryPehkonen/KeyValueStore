# Build options
function(configure_build_options)
    option(KEYVALUESTORE_BUILD_MEMORY "Build the in-memory key-value store implementation" ON)
    option(KEYVALUESTORE_BUILD_SQLITE "Build the SQLite key-value store implementation" ON)
    option(KEYVALUESTORE_BUILD_TESTS "Build the tests" ON)
    option(KEYVALUESTORE_BUILD_EXAMPLES "Build example applications" ON)
    option(KEYVALUESTORE_BUILD_DOCS "Build documentation" ON)
    option(KEYVALUESTORE_BUILD_COVERAGE "Build with coverage information" OFF)
endfunction()
