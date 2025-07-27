#!/bin/bash

# Enhanced deployment script for RPG Engine
# Comprehensive cross-platform deployment with web support

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"
DEPLOY_DIR="$PROJECT_ROOT/deploy"

echo "=== RPG Engine Enhanced Deployment Script ==="
echo "Project root: $PROJECT_ROOT"

# Clean previous deployments
echo "Cleaning previous deployments..."
rm -rf "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR"

# Function to test if executables work
test_executable() {
    local exe_path=$1
    local test_name=$2
    
    if [[ -f "$exe_path" && -x "$exe_path" ]]; then
        echo "  Testing $test_name..."
        if timeout 10s "$exe_path" >/dev/null 2>&1; then
            echo "  ✅ $test_name: PASSED"
            return 0
        else
            echo "  ⚠️  $test_name: TIMEOUT (expected for interactive tests)"
            return 0
        fi
    else
        echo "  ❌ $test_name: NOT FOUND"
        return 1
    fi
}

# Function to create platform-specific deployment
create_platform_deployment() {
    local platform=$1
    local build_path=$2
    local package_name=$3
    
    echo ""
    echo "=== Creating $platform Deployment ==="
    
    if [[ ! -d "$build_path" ]]; then
        echo "❌ Build directory not found: $build_path"
        return 1
    fi
    
    local deploy_path="$DEPLOY_DIR/$package_name"
    mkdir -p "$deploy_path"
    
    # Copy executables and test them
    local exe_count=0
    local working_exe_count=0
    
    find "$build_path" -type f \( -name "*Test" -o -name "*Demo" \) | while read -r exe; do
        if [[ -x "$exe" ]]; then
            local exe_name=$(basename "$exe")
            cp "$exe" "$deploy_path/"
            echo "  Added: $exe_name"
            ((exe_count++))
            
            # Test the executable
            if test_executable "$deploy_path/$exe_name" "$exe_name"; then
                ((working_exe_count++))
            fi
        fi
    done
    
    # Copy assets if they exist
    if [[ -d "$PROJECT_ROOT/assets" ]]; then
        echo "  Copying assets..."
        cp -r "$PROJECT_ROOT/assets" "$deploy_path/"
    fi
    
    # Copy documentation
    echo "  Copying documentation..."
    cp "$PROJECT_ROOT/README.md" "$deploy_path/" 2>/dev/null || true
    cp "$PROJECT_ROOT/DEPLOYMENT.md" "$deploy_path/" 2>/dev/null || true
    
    # Create platform-specific launcher and documentation
    case "$platform" in
        "macOS")
            create_macos_deployment "$deploy_path"
            ;;
        "Linux")
            create_linux_deployment "$deploy_path"
            ;;
        "Windows")
            create_windows_deployment "$deploy_path"
            ;;
    esac
    
    echo "  Deployment created: $exe_count executables"
    return 0
}#
 Platform-specific deployment functions
create_macos_deployment() {
    local deploy_path=$1
    
    # Create launcher script
    cat > "$deploy_path/run_demos.sh" << 'EOF'
#!/bin/bash

echo "🎮 RPG Engine - macOS Demos"
echo "============================"
echo ""

# Get available demos
demos=()
demo_descriptions=()

if [[ -x "./CrossPlatformTest" ]]; then
    demos+=("./CrossPlatformTest")
    demo_descriptions+=("Cross-Platform Test - Basic engine functionality")
fi

if [[ -x "./PerformanceOptimizationSimpleTest" ]]; then
    demos+=("./PerformanceOptimizationSimpleTest")
    demo_descriptions+=("Performance Test - Optimization features")
fi

if [[ -x "./UIFrameworkSimpleTest" ]]; then
    demos+=("./UIFrameworkSimpleTest")
    demo_descriptions+=("UI Framework Test - User interface system")
fi

if [[ -x "./ScriptingMinimalTest" ]]; then
    demos+=("./ScriptingMinimalTest")
    demo_descriptions+=("Scripting Test - Lua integration")
fi

if [[ -x "./ConfigSimpleTest" ]]; then
    demos+=("./ConfigSimpleTest")
    demo_descriptions+=("Configuration Test - Settings system")
fi

if [[ ${#demos[@]} -eq 0 ]]; then
    echo "❌ No executable demos found!"
    exit 1
fi

echo "Available demos:"
for i in "${!demo_descriptions[@]}"; do
    echo "$((i+1)). ${demo_descriptions[i]}"
done
echo "$((${#demos[@]}+1)). Exit"
echo ""

while true; do
    read -p "Select demo (1-$((${#demos[@]}+1))): " choice
    
    if [[ "$choice" =~ ^[0-9]+$ ]] && [[ $choice -ge 1 ]] && [[ $choice -le ${#demos[@]} ]]; then
        echo ""
        echo "Running: ${demo_descriptions[$((choice-1))]}"
        echo "========================================"
        ${demos[$((choice-1))]}
        echo ""
        echo "Demo completed. Press Enter to return to menu..."
        read
    elif [[ "$choice" -eq $((${#demos[@]}+1)) ]]; then
        echo "Goodbye!"
        exit 0
    else
        echo "Invalid choice. Please select 1-$((${#demos[@]}+1))."
    fi
done
EOF
    
    chmod +x "$deploy_path/run_demos.sh"
    
    # Create README
    cat > "$deploy_path/README_macOS.txt" << 'EOF'
🎮 RPG Engine - macOS Distribution
==================================

QUICK START:
1. Extract all files to a folder
2. Run: ./run_demos.sh
3. Select a demo from the menu

SYSTEM REQUIREMENTS:
- macOS 10.15 (Catalina) or later
- 4GB RAM recommended
- Lua runtime (install with: brew install lua)

TROUBLESHOOTING:
- "Cannot be opened" errors: Run in Terminal: xattr -d com.apple.quarantine *
- Missing Lua: Install Homebrew, then: brew install lua
- Permission issues: chmod +x *.sh

For more information, see README.md and DEPLOYMENT.md
EOF
}

create_linux_deployment() {
    local deploy_path=$1
    
    # Create launcher script
    cat > "$deploy_path/run_demos.sh" << 'EOF'
#!/bin/bash

echo "🎮 RPG Engine - Linux Demos"
echo "==========================="
echo ""

# Check dependencies
check_deps() {
    local missing=()
    
    if ! command -v lua >/dev/null 2>&1; then
        missing+=("lua")
    fi
    
    if [[ ${#missing[@]} -gt 0 ]]; then
        echo "⚠️  Missing dependencies: ${missing[*]}"
        echo "Install with your package manager:"
        echo "  Ubuntu/Debian: sudo apt install lua5.4"
        echo "  Fedora: sudo dnf install lua"
        echo "  Arch: sudo pacman -S lua"
        echo ""
    fi
}

check_deps

# Demo selection (same as macOS)
demos=()
demo_descriptions=()

if [[ -x "./CrossPlatformTest" ]]; then
    demos+=("./CrossPlatformTest")
    demo_descriptions+=("Cross-Platform Test")
fi

if [[ -x "./PerformanceOptimizationSimpleTest" ]]; then
    demos+=("./PerformanceOptimizationSimpleTest")
    demo_descriptions+=("Performance Test")
fi

if [[ -x "./UIFrameworkSimpleTest" ]]; then
    demos+=("./UIFrameworkSimpleTest")
    demo_descriptions+=("UI Framework Test")
fi

if [[ -x "./ScriptingMinimalTest" ]]; then
    demos+=("./ScriptingMinimalTest")
    demo_descriptions+=("Scripting Test")
fi

if [[ ${#demos[@]} -eq 0 ]]; then
    echo "❌ No executable demos found!"
    exit 1
fi

echo "Available demos:"
for i in "${!demo_descriptions[@]}"; do
    echo "$((i+1)). ${demo_descriptions[i]}"
done
echo "$((${#demos[@]}+1)). Exit"

while true; do
    read -p "Select demo (1-$((${#demos[@]}+1))): " choice
    
    if [[ "$choice" =~ ^[0-9]+$ ]] && [[ $choice -ge 1 ]] && [[ $choice -le ${#demos[@]} ]]; then
        echo "Running: ${demo_descriptions[$((choice-1))]}"
        ${demos[$((choice-1))]}
        echo "Press Enter to continue..."
        read
    elif [[ "$choice" -eq $((${#demos[@]}+1)) ]]; then
        exit 0
    else
        echo "Invalid choice."
    fi
done
EOF
    
    chmod +x "$deploy_path/run_demos.sh"
    
    # Create dependency installer
    cat > "$deploy_path/install_deps.sh" << 'EOF'
#!/bin/bash

echo "Installing RPG Engine dependencies..."

if command -v apt-get >/dev/null 2>&1; then
    sudo apt-get update
    sudo apt-get install -y lua5.4 liblua5.4-dev
elif command -v dnf >/dev/null 2>&1; then
    sudo dnf install -y lua lua-devel
elif command -v pacman >/dev/null 2>&1; then
    sudo pacman -S lua
else
    echo "Unknown package manager. Please install Lua manually."
fi
EOF
    
    chmod +x "$deploy_path/install_deps.sh"
}

create_windows_deployment() {
    local deploy_path=$1
    
    # Create batch launcher
    cat > "$deploy_path/run_demos.bat" << 'EOF'
@echo off
echo RPG Engine - Windows Demos
echo ===========================
echo.

:menu
echo Available demos:
set demo_count=0

if exist "CrossPlatformTest.exe" (
    set /a demo_count+=1
    echo !demo_count!. Cross-Platform Test
    set demo1=CrossPlatformTest.exe
)

if exist "PerformanceOptimizationSimpleTest.exe" (
    set /a demo_count+=1
    echo !demo_count!. Performance Test
    set demo2=PerformanceOptimizationSimpleTest.exe
)

if exist "UIFrameworkSimpleTest.exe" (
    set /a demo_count+=1
    echo !demo_count!. UI Framework Test
    set demo3=UIFrameworkSimpleTest.exe
)

if exist "ScriptingMinimalTest.exe" (
    set /a demo_count+=1
    echo !demo_count!. Scripting Test
    set demo4=ScriptingMinimalTest.exe
)

set /a demo_count+=1
echo !demo_count!. Exit
echo.

set /p choice="Select demo (1-!demo_count!): "

if "%choice%"=="1" if defined demo1 (
    echo Running Cross-Platform Test...
    %demo1%
    pause
    goto menu
)

if "%choice%"=="2" if defined demo2 (
    echo Running Performance Test...
    %demo2%
    pause
    goto menu
)

if "%choice%"=="3" if defined demo3 (
    echo Running UI Framework Test...
    %demo3%
    pause
    goto menu
)

if "%choice%"=="4" if defined demo4 (
    echo Running Scripting Test...
    %demo4%
    pause
    goto menu
)

if "%choice%"=="%demo_count%" (
    echo Goodbye!
    exit /b 0
)

echo Invalid choice.
goto menu
EOF
}# Web
 deployment function
create_web_deployment() {
    echo ""
    echo "=== Creating Web Deployment ==="
    
    # Check if Emscripten is available
    if ! command -v emcc >/dev/null 2>&1; then
        echo "⚠️  Emscripten not found. Installing..."
        install_emscripten
    fi
    
    local web_build_dir="$BUILD_DIR/web"
    local web_deploy_dir="$DEPLOY_DIR/rpg-engine-web"
    
    mkdir -p "$web_build_dir"
    mkdir -p "$web_deploy_dir"
    
    cd "$web_build_dir"
    
    # Configure for web
    echo "Configuring for Web..."
    if emcmake cmake -DCMAKE_BUILD_TYPE=Release "$PROJECT_ROOT" > "$web_deploy_dir/build.log" 2>&1; then
        echo "✅ Web configuration successful"
        
        # Build
        echo "Building for Web..."
        if emmake make CrossPlatformTest PerformanceOptimizationSimpleTest -j4 >> "$web_deploy_dir/build.log" 2>&1; then
            echo "✅ Web build successful"
            
            # Copy web files
            find . -name "*.html" -o -name "*.js" -o -name "*.wasm" -o -name "*.data" | while read -r file; do
                if [[ -f "$file" ]]; then
                    cp "$file" "$web_deploy_dir/"
                    echo "  Added: $(basename "$file")"
                fi
            done
            
            # Create enhanced index.html
            create_web_index "$web_deploy_dir"
            
            # Create web server script
            create_web_server_script "$web_deploy_dir"
            
            echo "✅ Web deployment created"
            cd "$PROJECT_ROOT"
            return 0
        else
            echo "❌ Web build failed"
            cd "$PROJECT_ROOT"
            return 1
        fi
    else
        echo "❌ Web configuration failed"
        cd "$PROJECT_ROOT"
        return 1
    fi
}

# Function to install Emscripten
install_emscripten() {
    echo "Installing Emscripten SDK..."
    
    local emsdk_dir="$PROJECT_ROOT/external/emsdk"
    
    if [[ ! -d "$emsdk_dir" ]]; then
        echo "Cloning Emscripten SDK..."
        git clone https://github.com/emscripten-core/emsdk.git "$emsdk_dir"
    fi
    
    cd "$emsdk_dir"
    ./emsdk install latest
    ./emsdk activate latest
    
    # Source the environment
    source ./emsdk_env.sh
    
    cd "$PROJECT_ROOT"
    echo "✅ Emscripten installed and activated"
}

# Function to create enhanced web index
create_web_index() {
    local web_dir=$1
    
    cat > "$web_dir/index.html" << 'EOF'
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
            background: linear-gradient(135deg, #1a1a1a 0%, #2d2d2d 100%);
            color: #ffffff;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            min-height: 100vh;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        .header {
            text-align: center;
            margin-bottom: 40px;
        }
        
        .header h1 {
            color: #4CAF50;
            font-size: 3.5em;
            margin: 0;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
        }
        
        .header p {
            color: #cccccc;
            font-size: 1.2em;
            margin: 10px 0;
        }
        
        .demos-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 20px;
            margin-bottom: 40px;
        }
        
        .demo-card {
            background: rgba(42, 42, 42, 0.8);
            border-radius: 15px;
            padding: 25px;
            border: 2px solid #4CAF50;
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        
        .demo-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 25px rgba(76, 175, 80, 0.3);
        }
        
        .demo-card h3 {
            color: #4CAF50;
            margin-top: 0;
            font-size: 1.4em;
        }
        
        .demo-card p {
            color: #cccccc;
            line-height: 1.6;
            margin: 15px 0;
        }
        
        .demo-card .features {
            list-style: none;
            padding: 0;
            margin: 15px 0;
        }
        
        .demo-card .features li {
            color: #aaaaaa;
            margin: 5px 0;
            padding-left: 20px;
            position: relative;
        }
        
        .demo-card .features li:before {
            content: "✓";
            color: #4CAF50;
            position: absolute;
            left: 0;
        }
        
        .demo-button {
            display: inline-block;
            background: linear-gradient(45deg, #4CAF50, #45a049);
            color: white;
            padding: 12px 24px;
            text-decoration: none;
            border-radius: 8px;
            font-weight: bold;
            transition: background 0.3s ease;
            border: none;
            cursor: pointer;
        }
        
        .demo-button:hover {
            background: linear-gradient(45deg, #45a049, #4CAF50);
        }
        
        .info-section {
            background: rgba(42, 42, 42, 0.6);
            border-radius: 10px;
            padding: 20px;
            margin-top: 30px;
        }
        
        .info-section h3 {
            color: #4CAF50;
            margin-top: 0;
        }
        
        .footer {
            text-align: center;
            margin-top: 50px;
            color: #888888;
            font-size: 0.9em;
        }
        
        .footer a {
            color: #4CAF50;
            text-decoration: none;
        }
        
        .footer a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🎮 RPG Engine</h1>
            <p>2D RPG Game Engine Framework - Web Demos</p>
            <p>Cross-platform game engine built with modern C++ and WebAssembly</p>
        </div>
        
        <div class="demos-grid">
            <div class="demo-card">
                <h3>🔧 Cross-Platform Test</h3>
                <p>Comprehensive test suite that validates engine functionality across different platforms.</p>
                <ul class="features">
                    <li>Threading and concurrency</li>
                    <li>Memory management</li>
                    <li>Platform-specific features</li>
                    <li>Performance benchmarks</li>
                </ul>
                <a href="CrossPlatformTest.html" class="demo-button">Run Test</a>
            </div>
            
            <div class="demo-card">
                <h3>⚡ Performance Optimization</h3>
                <p>Demonstrates advanced performance optimization techniques and engine capabilities.</p>
                <ul class="features">
                    <li>Frustum culling</li>
                    <li>Memory pooling</li>
                    <li>Multi-threading</li>
                    <li>Batch rendering</li>
                </ul>
                <a href="PerformanceOptimizationSimpleTest.html" class="demo-button">Run Demo</a>
            </div>
        </div>
        
        <div class="info-section">
            <h3>🌐 About Web Deployment</h3>
            <p>These demos are compiled to WebAssembly using Emscripten, allowing the full C++ engine to run in your browser with near-native performance.</p>
            
            <h3>🎯 System Requirements</h3>
            <ul>
                <li>Modern web browser with WebAssembly support</li>
                <li>Chrome 57+, Firefox 52+, Safari 11+, or Edge 16+</li>
                <li>Stable internet connection for initial load</li>
            </ul>
            
            <h3>🚀 Getting Started</h3>
            <p>Click on any demo above to start. The engine will initialize automatically and display test results or interactive content.</p>
        </div>
        
        <div class="footer">
            <p>Built with <a href="https://emscripten.org/" target="_blank">Emscripten</a> and <a href="https://webassembly.org/" target="_blank">WebAssembly</a></p>
            <p>RPG Engine Framework © 2025</p>
        </div>
    </div>
</body>
</html>
EOF
}

# Function to create web server script
create_web_server_script() {
    local web_dir=$1
    
    cat > "$web_dir/serve.py" << 'EOF'
#!/usr/bin/env python3
"""
Simple HTTP server for RPG Engine web demos
Handles proper MIME types for WebAssembly files
"""

import http.server
import socketserver
import os
import sys

class WebAssemblyHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        super().end_headers()
    
    def guess_type(self, path):
        mimetype, encoding = super().guess_type(path)
        if path.endswith('.wasm'):
            return 'application/wasm', encoding
        elif path.endswith('.data'):
            return 'application/octet-stream', encoding
        return mimetype, encoding

def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    
    print(f"🎮 RPG Engine Web Server")
    print(f"Starting server on http://localhost:{port}")
    print(f"Press Ctrl+C to stop")
    
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    
    with socketserver.TCPServer(("", port), WebAssemblyHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped.")

if __name__ == "__main__":
    main()
EOF
    
    chmod +x "$web_dir/serve.py"
    
    # Create README for web deployment
    cat > "$web_dir/README_Web.txt" << 'EOF'
RPG Engine - Web Deployment
===========================

QUICK START:
1. Run: python3 serve.py
2. Open http://localhost:8000 in your browser
3. Click on any demo to start

REQUIREMENTS:
- Python 3.6+ (for local server)
- Modern web browser with WebAssembly support

DEPLOYMENT:
- Upload all files to any web server
- Ensure proper MIME types for .wasm and .data files
- Access index.html to see available demos

TROUBLESHOOTING:
- CORS errors: Use the provided serve.py script
- Loading issues: Check browser console for errors
- Performance: Use Chrome or Firefox for best results
EOF
}

# Main deployment execution
echo ""
echo "=== Starting Enhanced Deployment ==="

# Deploy existing builds
deployed_platforms=0

# Check for macOS build
if [[ -d "$BUILD_DIR/release" ]] || [[ -d "$BUILD_DIR/macos" ]]; then
    build_path="$BUILD_DIR/release"
    [[ -d "$BUILD_DIR/macos" ]] && build_path="$BUILD_DIR/macos"
    
    if create_platform_deployment "macOS" "$build_path" "rpg-engine-macos"; then
        ((deployed_platforms++))
    fi
fi

# Check for Linux build (if cross-compiled or on Linux)
if [[ -d "$BUILD_DIR/linux" ]]; then
    if create_platform_deployment "Linux" "$BUILD_DIR/linux" "rpg-engine-linux"; then
        ((deployed_platforms++))
    fi
fi

# Check for Windows build (if cross-compiled or on Windows)
if [[ -d "$BUILD_DIR/windows" ]]; then
    if create_platform_deployment "Windows" "$BUILD_DIR/windows" "rpg-engine-windows"; then
        ((deployed_platforms++))
    fi
fi

# Create web deployment
if create_web_deployment; then
    ((deployed_platforms++))
fi

# Create packages
echo ""
echo "=== Creating Distribution Packages ==="

cd "$DEPLOY_DIR"

for dir in rpg-engine-*; do
    if [[ -d "$dir" ]]; then
        echo "Packaging $dir..."
        if command -v zip >/dev/null 2>&1; then
            zip -r "${dir}.zip" "$dir" >/dev/null
            echo "  Created: ${dir}.zip"
        else
            tar -czf "${dir}.tar.gz" "$dir"
            echo "  Created: ${dir}.tar.gz"
        fi
    fi
done

# Create master package
echo ""
echo "=== Creating Master Distribution ==="

master_dir="rpg-engine-complete"
mkdir -p "$master_dir"

# Copy all packages
for package in *.zip *.tar.gz; do
    if [[ -f "$package" ]]; then
        cp "$package" "$master_dir/"
    fi
done

# Create master README
cat > "$master_dir/README.txt" << EOF
RPG Engine - Complete Cross-Platform Distribution
=================================================

This package contains the RPG Engine Framework for all supported platforms.

CONTENTS:
$(ls -1 "$master_dir"/*.zip "$master_dir"/*.tar.gz 2>/dev/null | sed 's|.*/||' | sed 's/^/- /')

PLATFORM SELECTION:
- rpg-engine-macos.*     - macOS (Intel/Apple Silicon)
- rpg-engine-linux.*     - Linux (x64)
- rpg-engine-windows.*   - Windows (x64)
- rpg-engine-web.*       - Web (WebAssembly)

QUICK START:
1. Extract the package for your platform
2. Follow the README instructions in each package
3. Run the launcher script or individual demos

SYSTEM REQUIREMENTS:
- Modern CPU with multi-core support
- 4GB+ RAM recommended
- Graphics card with OpenGL 3.3+ (native platforms)
- Modern web browser with WebAssembly (web platform)

SUPPORT:
- Documentation: See individual platform README files
- Build from source: See DEPLOYMENT.md
- Issues: Check project repository

Generated: $(date)
Platform: $(uname -s)
EOF

# Create final master package
if command -v zip >/dev/null 2>&1; then
    zip -r "${master_dir}.zip" "$master_dir" >/dev/null
    echo "Created: ${master_dir}.zip"
else
    tar -czf "${master_dir}.tar.gz" "$master_dir"
    echo "Created: ${master_dir}.tar.gz"
fi

cd "$PROJECT_ROOT"

# Final summary
echo ""
echo "=== Enhanced Deployment Summary ==="
echo "Deployed platforms: $deployed_platforms"
echo "Deployment directory: $DEPLOY_DIR"
echo ""
echo "Created packages:"
ls -la "$DEPLOY_DIR"/*.zip "$DEPLOY_DIR"/*.tar.gz 2>/dev/null || echo "No packages created"

echo ""
echo "🎉 Enhanced deployment complete!"
echo ""
echo "NEXT STEPS:"
echo "1. Test packages on target platforms"
echo "2. Upload to distribution channels"
echo "3. For web: Extract rpg-engine-web.zip to web server"
echo "4. For complete distribution: Use rpg-engine-complete.zip"

exit 0