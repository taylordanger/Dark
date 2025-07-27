#!/bin/bash

# Deployment script for RPG Engine
# Creates distribution packages for all supported platforms

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"
DEPLOY_DIR="$PROJECT_ROOT/deploy"

echo "=== RPG Engine Deployment Script ==="
echo "Project root: $PROJECT_ROOT"

# Clean previous deployments
echo "Cleaning previous deployments..."
rm -rf "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR"

# Function to create a deployment package
create_package() {
    local platform=$1
    local build_dir=$2
    local package_name=$3
    
    echo ""
    echo "=== Creating Package: $package_name ==="
    
    local package_dir="$DEPLOY_DIR/$package_name"
    mkdir -p "$package_dir"
    
    # Copy executables
    echo "Copying executables..."
    find "$build_dir" -type f \( -name "*.exe" -o -perm +111 \) -not -path "*/CMakeFiles/*" | while read -r file; do
        if [[ -f "$file" && -x "$file" ]]; then
            local filename=$(basename "$file")
            cp "$file" "$package_dir/"
            echo "  Added: $filename"
        fi
    done
    
    # Copy assets
    if [[ -d "$PROJECT_ROOT/assets" ]]; then
        echo "Copying assets..."
        cp -r "$PROJECT_ROOT/assets" "$package_dir/"
    fi
    
    # Copy documentation
    echo "Copying documentation..."
    cp "$PROJECT_ROOT/README.md" "$package_dir/" 2>/dev/null || true
    cp "$PROJECT_ROOT/DEPLOYMENT.md" "$package_dir/" 2>/dev/null || true
    
    # Create platform-specific files
    case "$platform" in
        "windows")
            create_windows_package "$package_dir"
            ;;
        "macos")
            create_macos_package "$package_dir"
            ;;
        "linux")
            create_linux_package "$package_dir"
            ;;
        "web")
            create_web_package "$package_dir" "$build_dir"
            ;;
    esac
    
    # Create archive
    cd "$DEPLOY_DIR"
    if command -v zip >/dev/null 2>&1; then
        zip -r "${package_name}.zip" "$package_name"
        echo "Created: ${package_name}.zip"
    else
        tar -czf "${package_name}.tar.gz" "$package_name"
        echo "Created: ${package_name}.tar.gz"
    fi
    
    cd "$PROJECT_ROOT"
}

# Platform-specific package creation functions
create_windows_package() {
    local package_dir=$1
    
    # Create batch file launcher
    cat > "$package_dir/run_engine.bat" << 'EOF'
@echo off
echo RPG Engine - Windows
echo.
echo Available demos:
echo 1. Cross-Platform Test
echo 2. Performance Test
echo 3. UI Framework Test
echo 4. Scripting Test
echo.
set /p choice="Select demo (1-4): "

if "%choice%"=="1" (
    CrossPlatformTest.exe
) else if "%choice%"=="2" (
    PerformanceOptimizationSimpleTest.exe
) else if "%choice%"=="3" (
    UIFrameworkSimpleTest.exe
) else if "%choice%"=="4" (
    ScriptingMinimalTest.exe
) else (
    echo Invalid choice
)

pause
EOF
    
    # Create README
    cat > "$package_dir/README_Windows.txt" << 'EOF'
RPG Engine - Windows Distribution
=================================

Installation:
1. Extract all files to a folder
2. Run run_engine.bat to start the demo launcher
3. Or run individual test executables directly

Requirements:
- Windows 10 or later
- Visual C++ Redistributable (if not statically linked)

Troubleshooting:
- If you get DLL errors, install Visual C++ Redistributable
- Run as administrator if you encounter permission issues
- Check Windows Defender exclusions if executables don't run
EOF
}

create_macos_package() {
    local package_dir=$1
    
    # Create shell script launcher
    cat > "$package_dir/run_engine.sh" << 'EOF'
#!/bin/bash

echo "🎮 RPG Engine - macOS Distribution"
echo "=================================="
echo ""

# Check system requirements
check_requirements() {
    local missing_deps=()
    
    # Check Lua
    if ! command -v lua >/dev/null 2>&1 && ! pkg-config --exists lua; then
        missing_deps+=("Lua")
    fi
    
    # Check for required libraries
    if ! otool -L CrossPlatformTest 2>/dev/null | grep -q lua; then
        if ! command -v lua >/dev/null 2>&1; then
            missing_deps+=("Lua runtime")
        fi
    fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        echo "⚠️  Missing dependencies detected:"
        for dep in "${missing_deps[@]}"; do
            echo "  - $dep"
        done
        echo ""
        echo "To install missing dependencies:"
        echo "  brew install lua"
        echo ""
        read -p "Continue anyway? (y/N): " continue_choice
        if [[ "$continue_choice" != "y" && "$continue_choice" != "Y" ]]; then
            exit 1
        fi
    fi
}

# Get available executables
get_available_demos() {
    local demos=()
    local demo_names=()
    
    if [[ -x "./CrossPlatformTest" ]]; then
        demos+=("./CrossPlatformTest")
        demo_names+=("Cross-Platform Test - Basic engine functionality")
    fi
    
    if [[ -x "./PerformanceOptimizationSimpleTest" ]]; then
        demos+=("./PerformanceOptimizationSimpleTest")
        demo_names+=("Performance Test - Optimization features")
    fi
    
    if [[ -x "./UIFrameworkSimpleTest" ]]; then
        demos+=("./UIFrameworkSimpleTest")
        demo_names+=("UI Framework Test - User interface system")
    fi
    
    if [[ -x "./ScriptingMinimalTest" ]]; then
        demos+=("./ScriptingMinimalTest")
        demo_names+=("Scripting Test - Lua integration")
    fi
    
    if [[ -x "./ConfigSimpleTest" ]]; then
        demos+=("./ConfigSimpleTest")
        demo_names+=("Configuration Test - Settings system")
    fi
    
    if [[ -x "./DebugSystemsMinimalTest" ]]; then
        demos+=("./DebugSystemsMinimalTest")
        demo_names+=("Debug Systems Test - Development tools")
    fi
    
    if [[ ${#demos[@]} -eq 0 ]]; then
        echo "❌ No executable demos found!"
        echo "Please ensure the package was extracted correctly."
        exit 1
    fi
    
    echo "Available demos:"
    for i in "${!demo_names[@]}"; do
        echo "$((i+1)). ${demo_names[i]}"
    done
    echo "$((${#demos[@]}+1)). Exit"
    echo ""
    
    while true; do
        read -p "Select demo (1-$((${#demos[@]}+1))): " choice
        
        if [[ "$choice" =~ ^[0-9]+$ ]] && [[ $choice -ge 1 ]] && [[ $choice -le ${#demos[@]} ]]; then
            echo ""
            echo "Running: ${demo_names[$((choice-1))]}"
            echo "========================================"
            ${demos[$((choice-1))]}
            echo ""
            echo "Demo completed. Press Enter to return to menu..."
            read
            break
        elif [[ "$choice" -eq $((${#demos[@]}+1)) ]]; then
            echo "Goodbye!"
            exit 0
        else
            echo "Invalid choice. Please select 1-$((${#demos[@]}+1))."
        fi
    done
}

# Main execution
check_requirements

while true; do
    get_available_demos
done
EOF
    
    chmod +x "$package_dir/run_engine.sh"
    
    # Create system info script
    cat > "$package_dir/system_info.sh" << 'EOF'
#!/bin/bash

echo "🖥️  System Information"
echo "===================="
echo ""
echo "Operating System: $(sw_vers -productName) $(sw_vers -productVersion)"
echo "Architecture: $(uname -m)"
echo "Kernel: $(uname -r)"
echo ""
echo "Hardware:"
echo "  CPU: $(sysctl -n machdep.cpu.brand_string)"
echo "  Cores: $(sysctl -n hw.ncpu)"
echo "  Memory: $(( $(sysctl -n hw.memsize) / 1024 / 1024 / 1024 ))GB"
echo ""
echo "Development Tools:"
if command -v clang >/dev/null 2>&1; then
    echo "  Clang: $(clang --version | head -1)"
fi
if command -v cmake >/dev/null 2>&1; then
    echo "  CMake: $(cmake --version | head -1)"
fi
if command -v lua >/dev/null 2>&1; then
    echo "  Lua: $(lua -v 2>&1)"
elif pkg-config --exists lua; then
    echo "  Lua: $(pkg-config --modversion lua)"
else
    echo "  Lua: Not found"
fi
echo ""
EOF
    
    chmod +x "$package_dir/system_info.sh"
    
    # Create README
    cat > "$package_dir/README_macOS.txt" << 'EOF'
🎮 RPG Engine - macOS Distribution
==================================

QUICK START:
1. Extract all files to a folder
2. Double-click run_engine.sh or run it from Terminal
3. Select a demo from the menu

SYSTEM REQUIREMENTS:
- macOS 10.15 (Catalina) or later
- Intel or Apple Silicon Mac
- 4GB RAM recommended
- Lua runtime (auto-installed with Homebrew)

INSTALLATION:
1. Extract the archive to your desired location
2. Open Terminal and navigate to the extracted folder
3. Run: ./run_engine.sh

AVAILABLE DEMOS:
- Cross-Platform Test: Tests basic engine functionality
- Performance Test: Demonstrates optimization features  
- UI Framework Test: Shows user interface capabilities
- Scripting Test: Lua integration demonstration
- Configuration Test: Settings and config system
- Debug Systems Test: Development and debugging tools

TROUBLESHOOTING:

"Cannot be opened" errors:
  Run in Terminal: xattr -d com.apple.quarantine *
  Or: System Preferences > Security & Privacy > Allow

Missing Lua dependency:
  Install Homebrew: /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  Install Lua: brew install lua

Permission issues:
  Make scripts executable: chmod +x *.sh
  Check file permissions: ls -la

Performance issues:
  Close other applications
  Check Activity Monitor for resource usage
  Run ./system_info.sh to check system specs

ADDITIONAL TOOLS:
- system_info.sh: Display system information
- Individual executables can be run directly

SUPPORT:
- Check README.md for general documentation
- Run ./system_info.sh for system diagnostics
- Check Console.app for system logs if issues persist

For more information, visit the project repository.
EOF
}

create_linux_package() {
    local package_dir=$1
    
    # Create shell script launcher
    cat > "$package_dir/run_engine.sh" << 'EOF'
#!/bin/bash

echo "RPG Engine - Linux"
echo ""
echo "Available demos:"
echo "1. Cross-Platform Test"
echo "2. Performance Test"
echo "3. UI Framework Test"
echo "4. Scripting Test"
echo ""
read -p "Select demo (1-4): " choice

case $choice in
    1)
        ./CrossPlatformTest
        ;;
    2)
        ./PerformanceOptimizationSimpleTest
        ;;
    3)
        ./UIFrameworkSimpleTest
        ;;
    4)
        ./ScriptingMinimalTest
        ;;
    *)
        echo "Invalid choice"
        ;;
esac

read -p "Press Enter to exit..."
EOF
    
    chmod +x "$package_dir/run_engine.sh"
    
    # Create install script
    cat > "$package_dir/install_dependencies.sh" << 'EOF'
#!/bin/bash

echo "Installing RPG Engine dependencies..."

# Detect distribution
if command -v apt-get >/dev/null 2>&1; then
    # Debian/Ubuntu
    sudo apt-get update
    sudo apt-get install -y liblua5.4-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev libgl1-mesa-dev
elif command -v yum >/dev/null 2>&1; then
    # RHEL/CentOS/Fedora
    sudo yum install -y lua-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel mesa-libGL-devel
elif command -v pacman >/dev/null 2>&1; then
    # Arch Linux
    sudo pacman -S lua libx11 libxrandr libxi libxcursor libxinerama mesa
else
    echo "Unknown distribution. Please install dependencies manually:"
    echo "- Lua development libraries"
    echo "- X11 development libraries"
    echo "- OpenGL development libraries"
fi

echo "Dependencies installation complete!"
EOF
    
    chmod +x "$package_dir/install_dependencies.sh"
    
    # Create README
    cat > "$package_dir/README_Linux.txt" << 'EOF'
RPG Engine - Linux Distribution
===============================

Installation:
1. Extract all files to a folder
2. Run ./install_dependencies.sh to install system dependencies
3. Run ./run_engine.sh to start the demo launcher
4. Or run individual test executables directly

Requirements:
- Linux with X11
- GCC 7+ or Clang 6+
- Lua development libraries
- X11 development libraries
- OpenGL libraries

Troubleshooting:
- Run install_dependencies.sh if you get library errors
- Ensure you have graphics drivers installed
- For permission issues, check executable permissions: chmod +x *
EOF
}

create_web_package() {
    local package_dir=$1
    local build_dir=$2
    
    # Copy web-specific files
    find "$build_dir" -name "*.html" -o -name "*.js" -o -name "*.wasm" -o -name "*.data" | while read -r file; do
        if [[ -f "$file" ]]; then
            cp "$file" "$package_dir/"
            echo "  Added web file: $(basename "$file")"
        fi
    done
    
    # Create index.html that lists available demos
    cat > "$package_dir/index.html" << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RPG Engine - Web Demos</title>
    <style>
        body {
            margin: 0;
            padding: 20px;
            background-color: #1a1a1a;
            color: #ffffff;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        .header {
            text-align: center;
            margin-bottom: 30px;
        }
        
        .header h1 {
            color: #4CAF50;
            font-size: 3em;
            margin: 0;
        }
        
        .demos {
            max-width: 800px;
            margin: 0 auto;
        }
        
        .demo-card {
            background-color: #2a2a2a;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
            border: 2px solid #4CAF50;
        }
        
        .demo-card h3 {
            color: #4CAF50;
            margin-top: 0;
        }
        
        .demo-card a {
            display: inline-block;
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            text-decoration: none;
            border-radius: 5px;
            margin-top: 10px;
        }
        
        .demo-card a:hover {
            background-color: #45a049;
        }
        
        .footer {
            text-align: center;
            margin-top: 50px;
            color: #888888;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>🎮 RPG Engine</h1>
        <p>Web Demos</p>
    </div>
    
    <div class="demos">
        <div class="demo-card">
            <h3>Cross-Platform Test</h3>
            <p>Tests basic engine functionality across different platforms including threading, timing, and memory allocation.</p>
            <a href="CrossPlatformTest.html">Run Demo</a>
        </div>
        
        <div class="demo-card">
            <h3>Performance Optimization Test</h3>
            <p>Demonstrates performance optimizations including frustum culling, memory pooling, and multi-threading.</p>
            <a href="PerformanceOptimizationSimpleTest.html">Run Demo</a>
        </div>
    </div>
    
    <div class="footer">
        <p>RPG Engine Framework © 2025</p>
        <p>Built with <a href="https://emscripten.org/" target="_blank" style="color: #4CAF50;">Emscripten</a></p>
    </div>
</body>
</html>
EOF
    
    # Create deployment instructions
    cat > "$package_dir/README_Web.txt" << 'EOF'
RPG Engine - Web Distribution
=============================

Deployment:
1. Upload all files to a web server
2. Ensure proper MIME types are configured:
   - .wasm -> application/wasm
   - .data -> application/octet-stream
3. Access index.html to see available demos

Requirements:
- Modern web browser with WebAssembly support
- HTTPS (required for some features)
- Web server (cannot run from file:// protocol)

Supported Browsers:
- Chrome 57+
- Firefox 52+
- Safari 11+
- Edge 16+

Troubleshooting:
- Ensure CORS headers are properly configured
- Check browser console for errors
- Verify all files are accessible via HTTP/HTTPS
EOF
}

# Run the build script first
echo "Running cross-platform build..."
if ! "$PROJECT_ROOT/scripts/build_all_platforms.sh"; then
    echo "❌ Build failed. Cannot create deployment packages."
    exit 1
fi

# Create packages for each platform
if [[ -d "$BUILD_DIR/macos" ]]; then
    create_package "macos" "$BUILD_DIR/macos" "rpg-engine-macos"
fi

if [[ -d "$BUILD_DIR/linux" ]]; then
    create_package "linux" "$BUILD_DIR/linux" "rpg-engine-linux"
fi

if [[ -d "$BUILD_DIR/windows" ]]; then
    create_package "windows" "$BUILD_DIR/windows" "rpg-engine-windows"
fi

if [[ -d "$BUILD_DIR/web" ]]; then
    create_package "web" "$BUILD_DIR/web" "rpg-engine-web"
fi

# Create master distribution
echo ""
echo "=== Creating Master Distribution ==="
master_dir="$DEPLOY_DIR/rpg-engine-complete"
mkdir -p "$master_dir"

# Copy all platform packages
for package in "$DEPLOY_DIR"/*.zip "$DEPLOY_DIR"/*.tar.gz; do
    if [[ -f "$package" ]]; then
        cp "$package" "$master_dir/"
    fi
done

# Create master README
cat > "$master_dir/README.txt" << 'EOF'
RPG Engine - Complete Distribution
==================================

This package contains the RPG Engine Framework for all supported platforms.

Contents:
- rpg-engine-macos.zip     - macOS distribution
- rpg-engine-linux.tar.gz  - Linux distribution  
- rpg-engine-windows.zip   - Windows distribution
- rpg-engine-web.zip       - Web distribution

Choose the appropriate package for your target platform.

For detailed installation and usage instructions, see the README file
in each platform-specific package.

System Requirements:
- Modern CPU with multi-core support
- 4GB+ RAM recommended
- Graphics card with OpenGL 3.3+ support (for native platforms)
- Modern web browser with WebAssembly support (for web platform)

Support:
- Documentation: See DEPLOYMENT.md in each package
- Issues: Check the project repository
- Platform-specific help: See platform README files
EOF

# Create final archive
cd "$DEPLOY_DIR"
if command -v zip >/dev/null 2>&1; then
    zip -r "rpg-engine-complete.zip" "rpg-engine-complete"
    echo "Created: rpg-engine-complete.zip"
else
    tar -czf "rpg-engine-complete.tar.gz" "rpg-engine-complete"
    echo "Created: rpg-engine-complete.tar.gz"
fi

cd "$PROJECT_ROOT"

# Generate deployment summary
echo ""
echo "=== Deployment Summary ==="
echo "Deployment directory: $DEPLOY_DIR"
echo ""
echo "Created packages:"
ls -la "$DEPLOY_DIR"/*.zip "$DEPLOY_DIR"/*.tar.gz 2>/dev/null || echo "No packages created"

echo ""
echo "🎉 Deployment complete!"
echo ""
echo "To distribute:"
echo "1. Upload platform-specific packages to your distribution channels"
echo "2. For web deployment, extract rpg-engine-web.zip to your web server"
echo "3. For complete distribution, use rpg-engine-complete.zip"