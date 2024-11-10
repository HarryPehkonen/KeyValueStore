cmake_minimum_required(VERSION 3.14)
project(keyvaluestore 
        VERSION 1.0.0 
        DESCRIPTION "A flexible key-value store with multiple backend implementations"
        LANGUAGES CXX)

# Global settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(BUILD_MEMORY_STORE "Build the in-memory key-value store implementation" ON)
option(BUILD_SQLITE_STORE "Build the SQLite key-value store implementation" ON)
option(BUILD_TESTING "Build the tests" ON)
option(BUILD_DOCS "Build documentation" ON)
option(BUILD_COVERAGE "Build with coverage information" OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

# Add compile options
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-O3>
)

# Find SQLite3 if SQLite store is enabled
if(BUILD_SQLITE_STORE)
    find_package(SQLite3 REQUIRED)
endif()

# Coverage settings
if(BUILD_COVERAGE)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(WARNING "Code coverage results with an optimized (non-Debug) build may be misleading")
    endif()
    
    add_compile_options(-fprofile-arcs -ftest-coverage)
    link_libraries(gcov)
    
    find_program(LCOV lcov REQUIRED)
    find_program(GENHTML genhtml REQUIRED)
    
    add_custom_target(coverage
        COMMAND ${LCOV} --directory . --zerocounters
        COMMAND ctest
        COMMAND ${LCOV} --directory . --capture --output-file coverage.info
        COMMAND ${LCOV} 
            --remove coverage.info 
            '/usr/*' 
            '${CMAKE_SOURCE_DIR}/tests/*' 
            --output-file coverage.info
        COMMAND ${GENHTML} coverage.info --output-directory ${CMAKE_SOURCE_DIR}/coverage_report
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report"
    )
endif()

# Documentation
if(BUILD_DOCS)
    find_package(Doxygen
                REQUIRED dot
                OPTIONAL_COMPONENTS mscgen dia)
   
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
    
    doxygen_add_docs(docs
        include
        README.md
        COMMENT "Generating API documentation with Doxygen"
    )
endif()

# SQLite implementation library
if(BUILD_SQLITE_STORE)
    add_library(sqlite_store STATIC)
    target_sources(sqlite_store
        PRIVATE
            src/sqlite/SQLiteKeyValueStore.cpp
    )
    target_include_directories(sqlite_store
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(sqlite_store
        PUBLIC
            SQLite::SQLite3
    )
endif()

# Interface library for header-only components
add_library(keyvaluestore INTERFACE)
target_include_directories(keyvaluestore
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Compile definitions based on enabled implementations
if(BUILD_MEMORY_STORE)
    target_compile_definitions(keyvaluestore
        INTERFACE HAVE_MEMORY_STORE
    )
endif()

if(BUILD_SQLITE_STORE)
    target_compile_definitions(keyvaluestore
        INTERFACE HAVE_SQLITE_STORE
    )
endif()

# Testing setup
if(BUILD_TESTING)
    enable_testing()
    find_package(GTest REQUIRED)
    
    add_executable(keyvaluestore_tests)
    target_sources(keyvaluestore_tests
        PRIVATE
            tests/interface_test.cpp
            tests/memory_test.cpp
            tests/sqlite_test.cpp
    )
    
    target_link_libraries(keyvaluestore_tests
        PRIVATE
            keyvaluestore
            $<$<BOOL:${BUILD_SQLITE_STORE}>:sqlite_store>
            GTest::GTest
            GTest::Main
            pthread
    )
    
    include(GoogleTest)
    gtest_discover_tests(keyvaluestore_tests)
endif()

# Installation setup
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/keyvaluestore)

install(TARGETS keyvaluestore
    EXPORT keyvaluestore-targets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(BUILD_SQLITE_STORE)
    install(TARGETS sqlite_store
        EXPORT keyvaluestore-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT keyvaluestore-targets
    FILE keyvaluestore-targets.cmake
    NAMESPACE keyvaluestore::
    DESTINATION ${INSTALL_CONFIGDIR}
)

# Generate and install config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    cmake/keyvaluestore-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-config.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-config-version.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

# Export from build tree
export(EXPORT keyvaluestore-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-targets.cmake
    NAMESPACE keyvaluestore::
)

export(PACKAGE keyvaluestore)
