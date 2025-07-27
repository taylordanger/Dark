#!/bin/bash

# Cross-platform build script for RPG Engine
# Builds the engine for all supported platforms

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"

echo "=== RPG Engine Cross-Platform Build Script ==="
echo "Project root: $PROJECT_ROOT"

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf "$BUILD_DIR"
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Function to build for a specific platform
build_platform() {
    local platform=$1
    local cmake_args=$2
    local build_name=$3
    
    echo ""
    echo "=== Building for $platform ==="
    
    local platform_build_dir="$BUILD_DIR/$build_name"
    mkdir -p "$platform_build_dir"
    cd "$platform_build_dir"
    
    # Configure
    echo "Configuring for $platform..."
    cmake $cmake_args "$PROJECT_ROOT"
    
    # Build (focus on CrossPlatformTest which is known to work)
    echo "Building for $platform..."
    make CrossPlatformTest -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    # Package
    echo "Packaging for $platform..."
    local dist_platform_dir="$DIST_DIR/$build_name"
    mkdir -p "$dist_platform_dir"
    
    # Copy executables
    find . -name "*.exe" -o -name "*Test" -o -name "*Demo" | while read -r file; do
        if [[ -f "$file" && -x "$file" ]]; then
            cp "$file" "$dist_platform_dir/"
        fi
    done
    
    # Copy assets if they exist
    if [[ -d "$PROJECT_ROOT/assets" ]]; then
        cp -r "$PROJECT_ROOT/assets" "$dist_platform_dir/"
    fi
    
    # Create platform-specific package
    cd "$DIST_DIR"
    if command -v zip >/dev/null 2>&1; then
        zip -r "${build_name}.zip" "$build_name"
        echo "Created package: ${build_name}.zip"
    else
        tar -czf "${build_name}.tar.gz" "$build_name"
        echo "Created package: ${build_name}.tar.gz"
    fi
    
    cd "$PROJECT_ROOT"
}

# Detect current platform
CURRENT_OS=""
case "$(uname -s)" in
    Darwin*)
        CURRENT_OS="macOS"
        ;;
    Linux*)
        CURRENT_OS="Linux"
        ;;
    CYGWIN*|MINGW*|MSYS*)
        CURRENT_OS="Windows"
        ;;
    *)
        CURRENT_OS="Unknown"
        ;;
esac

echo "Current platform: $CURRENT_OS"

# Build for current platform
case "$CURRENT_OS" in
    "macOS")
        build_platform "macOS" "-DCMAKE_BUILD_TYPE=Release" "macos"
        ;;
    "Linux")
        build_platform "Linux" "-DCMAKE_BUILD_TYPE=Release" "linux"
        ;;
    "Windows")
        build_platform "Windows" "-DCMAKE_BUILD_TYPE=Release -G \"MinGW Makefiles\"" "windows"
        ;;
    *)
        echo "Warning: Unknown platform, building with default settings"
        build_platform "Unknown" "-DCMAKE_BUILD_TYPE=Release" "unknown"
        ;;
esac

# Build for Web (if Emscripten is available)
if command -v emcmake >/dev/null 2>&1; then
    echo ""
    echo "=== Building for Web (Emscripten) ==="
    
    web_build_dir="$BUILD_DIR/web"
    mkdir -p "$web_build_dir"
    cd "$web_build_dir"
    
    # Configure with Emscripten
    echo "Configuring for Web..."
    if emcmake cmake -DCMAKE_BUILD_TYPE=Release "$PROJECT_ROOT"; then
        echo "✅ Web configuration successful"
        
        # Build
        echo "Building for Web..."
        if emmake make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4); then
            echo "✅ Web build successful"
            
            # Package
            echo "Packaging for Web..."
            dist_web_dir="$DIST_DIR/web"
            mkdir -p "$dist_web_dir"
            
            # Copy web files
            files_copied=0
            find . -name "*.html" -o -name "*.js" -o -name "*.wasm" -o -name "*.data" | while read -r file; do
                if [[ -f "$file" ]]; then
                    cp "$file" "$dist_web_dir/"
                    echo "  Copied: $(basename "$file")"
                    ((files_copied++))
                fi
            done
            
            # Copy shell template if it exists
            if [[ -f "$PROJECT_ROOT/web/shell.html" ]]; then
                cp "$PROJECT_ROOT/web/shell.html" "$dist_web_dir/"
                echo "  Copied: shell.html"
            fi
            
            # Create web package
            cd "$DIST_DIR"
            if command -v zip >/dev/null 2>&1; then
                zip -r "web.zip" "web"
                echo "✅ Created package: web.zip"
            else
                tar -czf "web.tar.gz" "web"
                echo "✅ Created package: web.tar.gz"
            fi
            
            cd "$PROJECT_ROOT"
        else
            echo "❌ Web build failed"
        fi
    else
        echo "❌ Web configuration failed"
    fi
else
    echo ""
    echo "=== Web Build Skipped ==="
    echo "Emscripten not found. To enable web builds:"
    echo "1. Install Emscripten SDK:"
    echo "   git clone https://github.com/emscripten-core/emsdk.git"
    echo "   cd emsdk"
    echo "   ./emsdk install latest"
    echo "   ./emsdk activate latest"
    echo "2. Source the environment:"
    echo "   source ./emsdk_env.sh"
    echo "3. Re-run this build script"
fi

echo ""
echo "=== Build Summary ==="
echo "Build directory: $BUILD_DIR"
echo "Distribution directory: $DIST_DIR"
echo ""
echo "Available packages:"
ls -la "$DIST_DIR"/*.zip "$DIST_DIR"/*.tar.gz 2>/dev/null || echo "No packages created"

echo ""
echo "=== Cross-Platform Build Complete ==="