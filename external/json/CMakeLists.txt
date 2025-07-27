cmake_minimum_required(VERSION 3.16)
project(json)

# Create JSON library (header-only)
add_library(json INTERFACE)

# Set include directories
target_include_directories(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Download nlohmann/json if not present
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann/json.hpp)
    message(STATUS "Downloading nlohmann/json...")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann)
    file(DOWNLOAD
        "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp"
        ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann/json.hpp
        SHOW_PROGRESS
    )
endif()

# Create include directory symlinks
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/include/nlohmann)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/external/include/nlohmann)
endif()

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/include/nlohmann/json.hpp)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann/json.hpp DESTINATION ${CMAKE_SOURCE_DIR}/external/include/nlohmann)
endif()