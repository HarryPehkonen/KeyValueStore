function(configure_installation)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Install the header files
    install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Install the library target
    install(
        TARGETS keyvaluestore
        EXPORT keyvaluestore-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Generate and install package files
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/keyvaluestore-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/keyvaluestore
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/keyvaluestore
    )

    install(
        EXPORT keyvaluestore-targets
        FILE keyvaluestore-targets.cmake
        NAMESPACE keyvaluestore::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/keyvaluestore
    )

    # Export targets for use in build tree
    export(
        EXPORT keyvaluestore-targets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/keyvaluestore-targets.cmake
        NAMESPACE keyvaluestore::
    )
endfunction()
