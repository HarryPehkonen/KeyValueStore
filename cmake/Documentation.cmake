# Documentation configuration
function(configure_documentation)
    if(NOT KEYVALUESTORE_BUILD_DOCS)
        return()
    endif()

    find_package(Doxygen
                REQUIRED dot
                OPTIONAL_COMPONENTS mscgen dia)
    
    # Configure Doxygen
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_EXTRACT_PACKAGE YES)
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
    set(DOXYGEN_PROJECT_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/docs/logo.png")
    set(DOXYGEN_HTML_TIMESTAMP YES)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_COLLABORATION_GRAPH YES)
    set(DOXYGEN_CLASS_GRAPH YES)
    set(DOXYGEN_CALL_GRAPH YES)
    set(DOXYGEN_CALLER_GRAPH YES)
    set(DOXYGEN_HIDE_UNDOC_RELATIONS NO)
    set(DOXYGEN_HAVE_DOT YES)
    set(DOXYGEN_DOT_NUM_THREADS 0)
    set(DOXYGEN_UML_LOOK YES)
    set(DOXYGEN_DOT_UML_DETAILS YES)
    set(DOXYGEN_TEMPLATE_RELATIONS YES)
    set(DOXYGEN_DOT_GRAPH_MAX_NODES 100)
    set(DOXYGEN_MAX_DOT_GRAPH_DEPTH 0)
    set(DOXYGEN_GENERATE_LEGEND YES)
    
    # Create Doxygen target
    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
    
    # Add custom target to clean documentation
    add_custom_target(docs_clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOXYGEN_OUTPUT_DIRECTORY}
        COMMENT "Cleaning documentation..."
    )
    
    # Install documentation
    if(EXISTS "${DOXYGEN_OUTPUT_DIRECTORY}/html")
        install(DIRECTORY "${DOXYGEN_OUTPUT_DIRECTORY}/html"
                DESTINATION ${CMAKE_INSTALL_DOCDIR}
                COMPONENT documentation
                OPTIONAL)
    endif()
endfunction()
