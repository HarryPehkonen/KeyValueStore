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

# Configure initial options
configure_compiler_options()
configure_build_options()
configure_documentation()

# Setup targets first
include(Targets)
configure_targets()

# Now configure coverage (after targets exist)
include(Coverage)
configure_coverage()

# Configure testing (which may need coverage flags)
include(Testing)
configure_testing()

# Examples and installation can come last
include(Examples)
include(Installation)
configure_examples()
configure_installation()
