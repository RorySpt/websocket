
if (NOT TARGET libhv)
    option(BUILD_EXAMPLES "build examples" OFF)
    add_subdirectory(libhv)

    add_library(libhv_md INTERFACE)
    target_include_directories(libhv_md INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/libhv
            ${CMAKE_CURRENT_SOURCE_DIR}/libhv/include
    )
    target_link_libraries(libhv_md INTERFACE hv_static)
endif ()
