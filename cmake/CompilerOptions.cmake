# Compiler and build options
function(configure_compiler_options)
    # Global compiler settings
    set(CMAKE_CXX_STANDARD 20 PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)

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
endfunction()
