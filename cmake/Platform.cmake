# Platform-specific configuration for RPG Engine

# Detect platform
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    set(PLATFORM_NAME "Windows")
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    set(PLATFORM_NAME "macOS")
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
    set(PLATFORM_NAME "Linux")
else()
    set(PLATFORM_UNKNOWN TRUE)
    set(PLATFORM_NAME "Unknown")
endif()

message(STATUS "Building for platform: ${PLATFORM_NAME}")

# Platform-specific compiler flags
if(PLATFORM_WINDOWS)
    # Windows-specific flags
    if(MSVC)
        add_compile_options(/W4 /WX- /permissive-)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
        add_compile_definitions(NOMINMAX)
        add_compile_definitions(WIN32_LEAN_AND_MEAN)
    endif()
    
    # Windows libraries
    set(PLATFORM_LIBRARIES 
        opengl32
        gdi32
        user32
        kernel32
        shell32
        ole32
        oleaut32
        uuid
        comdlg32
        advapi32
    )
    
elseif(PLATFORM_MACOS)
    # macOS-specific flags
    add_compile_options(-Wall -Wextra -Wpedantic)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    
    # macOS frameworks
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    
    set(PLATFORM_LIBRARIES 
        ${COCOA_LIBRARY}
        ${OPENGL_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
    
elseif(PLATFORM_LINUX)
    # Linux-specific flags
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    # Linux libraries
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED x11)
    pkg_check_modules(XRANDR REQUIRED xrandr)
    pkg_check_modules(XI REQUIRED xi)
    pkg_check_modules(XCURSOR REQUIRED xcursor)
    pkg_check_modules(XINERAMA REQUIRED xinerama)
    
    set(PLATFORM_LIBRARIES 
        GL
        ${X11_LIBRARIES}
        ${XRANDR_LIBRARIES}
        ${XI_LIBRARIES}
        ${XCURSOR_LIBRARIES}
        ${XINERAMA_LIBRARIES}
        pthread
        dl
    )
    
endif()

# Platform-specific definitions
if(PLATFORM_WINDOWS)
    add_compile_definitions(PLATFORM_WINDOWS=1)
elseif(PLATFORM_MACOS)
    add_compile_definitions(PLATFORM_MACOS=1)
elseif(PLATFORM_LINUX)
    add_compile_definitions(PLATFORM_LINUX=1)
endif()

# Function to configure platform-specific settings for a target
function(configure_platform_target target_name)
    target_link_libraries(${target_name} ${PLATFORM_LIBRARIES})
    
    if(PLATFORM_WINDOWS)
        # Windows-specific target configuration
        set_target_properties(${target_name} PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    elseif(PLATFORM_MACOS)
        # macOS-specific target configuration
        # Only create bundles for main applications, not test executables
        if(NOT ${target_name} MATCHES ".*Test.*")
            set_target_properties(${target_name} PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in"
            )
        endif()
    elseif(PLATFORM_LINUX)
        # Linux-specific target configuration
        set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
    endif()
endfunction()

# Web platform support (Emscripten)
if(EMSCRIPTEN)
    set(PLATFORM_WEB TRUE)
    set(PLATFORM_NAME "Web")
    
    message(STATUS "Building for Web platform with Emscripten")
    
    # Emscripten-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_WEBGL2=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s FULL_ES3=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
    
    # Web-specific function
    function(configure_web_target target_name)
        set_target_properties(${target_name} PROPERTIES
            SUFFIX ".html"
            LINK_FLAGS "--shell-file ${CMAKE_SOURCE_DIR}/web/shell.html --preload-file assets"
        )
    endfunction()
    
    add_compile_definitions(PLATFORM_WEB=1)
endif()

message(STATUS "Platform configuration complete for ${PLATFORM_NAME}")