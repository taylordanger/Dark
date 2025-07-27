cmake_minimum_required(VERSION 3.16)
project(stb)

# Create STB library (header-only)
add_library(stb INTERFACE)

# Set include directories
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Download STB Image if not present
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.h)
    message(STATUS "Downloading STB Image...")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
    file(DOWNLOAD
        "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.h
        SHOW_PROGRESS
    )
endif()

# Create include directory symlinks
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/include/stb_image.h)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.h DESTINATION ${CMAKE_SOURCE_DIR}/external/include)
endif()