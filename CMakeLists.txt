cmake_minimum_required(VERSION 3.14)
project(keyvaluestore 
        VERSION 1.0.0 
        DESCRIPTION "A flexible key-value store with multiple backend implementations"
        LANGUAGES CXX)

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include our custom CMake modules
include(CompilerOptions)
include(Options)
include(Documentation)
include(Coverage)
include(Targets)
include(Testing)
include(Installation)
include(Examples)

# Configure everything
configure_compiler_options()
configure_build_options()
configure_documentation()
configure_coverage()
configure_targets()
configure_testing()
configure_examples()
configure_installation()
