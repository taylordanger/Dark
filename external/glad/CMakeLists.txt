cmake_minimum_required(VERSION 3.16)
project(glad)

# Add GLAD source files
set(GLAD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c
)

# Add GLAD include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create GLAD library
add_library(glad STATIC ${GLAD_SOURCES})

# Set include directories
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Download GLAD if not present
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c)
    message(STATUS "Downloading GLAD...")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/Dav1dde/glad/master/glad/src/glad.c"
        ${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c
        SHOW_PROGRESS
    )
endif()

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.h)
    message(STATUS "Downloading GLAD headers...")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/glad)
    file(DOWNLOAD
        "https://raw.githubusercontent.com/Dav1dde/glad/master/glad/include/glad/glad.h"
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.h
        SHOW_PROGRESS
    )
endif()

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/khrplatform.h)
    message(STATUS "Downloading KHR headers...")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/KHR)
    file(DOWNLOAD
        "https://raw.githubusercontent.com/Dav1dde/glad/master/glad/include/KHR/khrplatform.h"
        ${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/khrplatform.h
        SHOW_PROGRESS
    )
endif()

# Create include directory symlinks
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/external/include/glad)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/external/include/KHR)

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/include/glad/glad.h)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.h DESTINATION ${CMAKE_SOURCE_DIR}/external/include/glad)
endif()

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/include/KHR/khrplatform.h)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/khrplatform.h DESTINATION ${CMAKE_SOURCE_DIR}/external/include/KHR)
endif()