# RPG Engine - Cross-Platform Deployment Guide

This document describes how to build and deploy the RPG Engine Framework across different platforms.

## Supported Platforms

- **Windows** (x64) - Cross-compilation or native build
- **macOS** (Intel and Apple Silicon) - ✅ **TESTED AND WORKING**
- **Linux** (x64) - Cross-compilation or native build  
- **Web** (via Emscripten) - WebAssembly deployment

## Current Deployment Status

**Last Updated:** $(date)
**Test Platform:** macOS (Apple M3, 16GB RAM)
**Test Results:** ✅ ALL CROSS-PLATFORM TESTS PASSED

### Verified Functionality
- ✅ Basic data types and memory layout
- ✅ Multi-threading and concurrency
- ✅ High-resolution timing
- ✅ Memory allocation and management
- ✅ Platform-specific system information
- ✅ Build system integration
- ✅ Deployment package creation

## Prerequisites

### All Platforms
- CMake 3.16 or higher
- C++17 compatible compiler
- Lua development libraries

### Platform-Specific Requirements

#### Windows
- Visual Studio 2019 or higher (with C++ tools)
- OR MinGW-w64 compiler
- Windows SDK

#### macOS
- Xcode Command Line Tools
- Homebrew (recommended for dependencies)

#### Linux
- GCC 7+ or Clang 6+
- Development packages: `libx11-dev`, `libxrandr-dev`, `libxi-dev`, `libxcursor-dev`, `libxinerama-dev`
- OpenGL development libraries

#### Web (Emscripten)
- Emscripten SDK (emsdk)
- Python 3.6+
- Node.js (for testing)

## Building

### Quick Build (Current Platform)

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Cross-Platform Build Script

Use the provided build script to build for all supported platforms:

```bash
./scripts/build_all_platforms.sh
```

This script will:
1. Detect the current platform
2. Build for the current platform
3. Attempt to build for Web (if Emscripten is available)
4. Package the results in the `dist/` directory

### Manual Platform-Specific Builds

#### Windows (Visual Studio)
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

#### Windows (MinGW)
```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
make -j4
```

#### macOS
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

#### Linux
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### Web (Emscripten)
```bash
mkdir build-web
cd build-web
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make -j4
```

## Testing Cross-Platform Compatibility

Run the cross-platform test to verify the engine works correctly:

```bash
./build/CrossPlatformTest
```

This test verifies:
- Basic data types and sizes
- Threading functionality
- Timing and sleep functions
- Memory allocation
- Platform-specific features

## Deployment

### Desktop Platforms

The build process creates executable files that can be distributed:

- **Windows**: `.exe` files with required DLLs
- **macOS**: Executable files (can be bundled into `.app` packages)
- **Linux**: Executable files with shared library dependencies

### Web Deployment

For web deployment, the build process creates:
- `.html` file (main page)
- `.js` file (JavaScript runtime)
- `.wasm` file (WebAssembly binary)
- `.data` file (preloaded assets)

Deploy these files to a web server. Note that WebAssembly requires proper MIME types:
- `.wasm` → `application/wasm`
- `.data` → `application/octet-stream`

### Distribution Packages

The build script automatically creates distribution packages:
- **Windows**: `windows.zip`
- **macOS**: `macos.zip` or `macos.tar.gz`
- **Linux**: `linux.tar.gz`
- **Web**: `web.zip`

## Platform-Specific Notes

### Windows
- Ensure Visual C++ Redistributable is installed on target systems
- Consider static linking for standalone distribution
- Use Windows Defender exclusions during development

### macOS
- Code signing may be required for distribution
- Consider creating `.dmg` installers for user-friendly distribution
- Test on both Intel and Apple Silicon Macs

### Linux
- Different distributions may require different library versions
- Consider using AppImage for portable distribution
- Test on major distributions (Ubuntu, Fedora, Arch)

### Web
- Requires HTTPS for some features (SharedArrayBuffer, etc.)
- Test on different browsers (Chrome, Firefox, Safari, Edge)
- Consider Progressive Web App (PWA) features

## Performance Optimizations

The engine includes several performance optimizations:

1. **Frustum Culling**: Only renders visible objects
2. **Memory Pooling**: Reduces allocation overhead
3. **Parallel System Updates**: Uses multiple CPU cores
4. **Batch Rendering**: Minimizes draw calls

These optimizations are automatically enabled and work across all platforms.

## Troubleshooting

### Common Build Issues

1. **Missing Dependencies**: Install platform-specific development packages
2. **CMake Version**: Ensure CMake 3.16+ is installed
3. **Compiler Errors**: Use supported compiler versions
4. **Lua Not Found**: Install Lua development libraries

### Platform-Specific Issues

#### Windows
- Path length limitations: Use shorter build paths
- Antivirus interference: Add build directory to exclusions

#### macOS
- Xcode license: Run `sudo xcodebuild -license accept`
- Homebrew permissions: Fix with `brew doctor`

#### Linux
- Missing headers: Install `-dev` packages for dependencies
- OpenGL issues: Ensure graphics drivers are installed

#### Web
- Emscripten not found: Source the emsdk environment
- Memory issues: Increase WASM memory limits

## Continuous Integration

Example GitHub Actions workflow for cross-platform builds:

```yaml
name: Cross-Platform Build
on: [push, pull_request]

jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
    
    runs-on: ${{ matrix.os }}
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install dependencies (Ubuntu)
      if: matrix.os == 'ubuntu-latest'
      run: |
        sudo apt-get update
        sudo apt-get install -y liblua5.4-dev libx11-dev libxrandr-dev
    
    - name: Install dependencies (macOS)
      if: matrix.os == 'macos-latest'
      run: brew install lua
    
    - name: Build
      run: |
        mkdir build
        cd build
        cmake ..
        cmake --build . --config Release
    
    - name: Test
      run: ./build/CrossPlatformTest
```

## License and Distribution

Ensure compliance with all third-party library licenses when distributing the engine. The engine uses:
- Lua (MIT License)
- OpenGL (varies by implementation)
- Platform-specific libraries (varies)

## Support

For platform-specific issues or deployment questions, please refer to:
- Engine documentation
- Platform-specific development guides
- Community forums and support channels