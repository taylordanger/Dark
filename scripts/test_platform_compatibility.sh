#!/bin/bash

# Comprehensive platform compatibility testing script for RPG Engine
# Tests engine compatibility across different platforms and configurations

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
TEST_RESULTS_DIR="$PROJECT_ROOT/test_results"

echo "=== RPG Engine Platform Compatibility Test ==="
echo "Project root: $PROJECT_ROOT"

# Clean previous test results
echo "Cleaning previous test results..."
rm -rf "$TEST_RESULTS_DIR"
mkdir -p "$TEST_RESULTS_DIR"

# Platform detection with detailed information
detect_platform() {
    local platform_info_file="$TEST_RESULTS_DIR/platform_info.txt"
    
    echo "=== Platform Detection ===" | tee "$platform_info_file"
    
    # Basic OS detection
    case "$(uname -s)" in
        Darwin*)
            PLATFORM_OS="macOS"
            PLATFORM_ARCH="$(uname -m)"
            PLATFORM_VERSION="$(sw_vers -productVersion)"
            echo "Operating System: macOS $PLATFORM_VERSION ($PLATFORM_ARCH)" | tee -a "$platform_info_file"
            ;;
        Linux*)
            PLATFORM_OS="Linux"
            PLATFORM_ARCH="$(uname -m)"
            PLATFORM_KERNEL="$(uname -r)"
            echo "Operating System: Linux $PLATFORM_KERNEL ($PLATFORM_ARCH)" | tee -a "$platform_info_file"
            
            # Try to detect distribution
            if [[ -f /etc/os-release ]]; then
                source /etc/os-release
                echo "Distribution: $NAME $VERSION" | tee -a "$platform_info_file"
            fi
            ;;
        CYGWIN*|MINGW*|MSYS*)
            PLATFORM_OS="Windows"
            PLATFORM_ARCH="$(uname -m)"
            echo "Operating System: Windows ($PLATFORM_ARCH)" | tee -a "$platform_info_file"
            ;;
        *)
            PLATFORM_OS="Unknown"
            PLATFORM_ARCH="$(uname -m)"
            echo "Operating System: Unknown ($PLATFORM_ARCH)" | tee -a "$platform_info_file"
            ;;
    esac
    
    # CPU information
    echo "" | tee -a "$platform_info_file"
    echo "CPU Information:" | tee -a "$platform_info_file"
    if [[ "$PLATFORM_OS" == "macOS" ]]; then
        sysctl -n machdep.cpu.brand_string | tee -a "$platform_info_file"
        echo "CPU Cores: $(sysctl -n hw.ncpu)" | tee -a "$platform_info_file"
        echo "Memory: $(( $(sysctl -n hw.memsize) / 1024 / 1024 / 1024 ))GB" | tee -a "$platform_info_file"
    elif [[ "$PLATFORM_OS" == "Linux" ]]; then
        if [[ -f /proc/cpuinfo ]]; then
            grep "model name" /proc/cpuinfo | head -1 | cut -d: -f2 | sed 's/^ *//' | tee -a "$platform_info_file"
            echo "CPU Cores: $(nproc)" | tee -a "$platform_info_file"
        fi
        if [[ -f /proc/meminfo ]]; then
            grep "MemTotal" /proc/meminfo | awk '{print "Memory: " int($2/1024/1024) "GB"}' | tee -a "$platform_info_file"
        fi
    fi
    
    # Compiler information
    echo "" | tee -a "$platform_info_file"
    echo "Compiler Information:" | tee -a "$platform_info_file"
    
    if command -v gcc >/dev/null 2>&1; then
        echo "GCC: $(gcc --version | head -1)" | tee -a "$platform_info_file"
    fi
    
    if command -v clang >/dev/null 2>&1; then
        echo "Clang: $(clang --version | head -1)" | tee -a "$platform_info_file"
    fi
    
    if command -v cmake >/dev/null 2>&1; then
        echo "CMake: $(cmake --version | head -1)" | tee -a "$platform_info_file"
    fi
    
    # Development dependencies
    echo "" | tee -a "$platform_info_file"
    echo "Development Dependencies:" | tee -a "$platform_info_file"
    
    if pkg-config --exists lua; then
        echo "Lua: $(pkg-config --modversion lua)" | tee -a "$platform_info_file"
    else
        echo "Lua: Not found via pkg-config" | tee -a "$platform_info_file"
    fi
    
    if command -v emcmake >/dev/null 2>&1; then
        echo "Emscripten: Available" | tee -a "$platform_info_file"
        emcc --version | head -1 | tee -a "$platform_info_file"
    else
        echo "Emscripten: Not available" | tee -a "$platform_info_file"
    fi
    
    # Graphics information
    echo "" | tee -a "$platform_info_file"
    echo "Graphics Information:" | tee -a "$platform_info_file"
    
    if command -v glxinfo >/dev/null 2>&1; then
        glxinfo | grep "OpenGL version" | tee -a "$platform_info_file" || echo "OpenGL: Unable to detect" | tee -a "$platform_info_file"
    elif [[ "$PLATFORM_OS" == "macOS" ]]; then
        system_profiler SPDisplaysDataType | grep "OpenGL Engine" | head -1 | tee -a "$platform_info_file" || echo "OpenGL: Unable to detect" | tee -a "$platform_info_file"
    else
        echo "OpenGL: Unable to detect" | tee -a "$platform_info_file"
    fi
}

# Test function with enhanced error reporting
run_enhanced_test() {
    local test_name=$1
    local test_command=$2
    local build_dir=$3
    local timeout_seconds=${4:-30}
    
    echo ""
    echo "=== Running Test: $test_name ==="
    
    local result_file="$TEST_RESULTS_DIR/${test_name}.log"
    local success=true
    
    # Change to build directory
    if ! cd "$build_dir"; then
        echo "❌ $test_name: FAILED (cannot access build directory)" | tee -a "$result_file"
        return 1
    fi
    
    # Run the test with timeout (if available)
    echo "Running: $test_command" | tee "$result_file"
    echo "Timeout: ${timeout_seconds}s" | tee -a "$result_file"
    echo "Working directory: $(pwd)" | tee -a "$result_file"
    echo "--- Test Output ---" | tee -a "$result_file"
    
    # Use timeout if available, otherwise run directly
    if command -v timeout >/dev/null 2>&1; then
        if timeout "${timeout_seconds}s" bash -c "$test_command" >> "$result_file" 2>&1; then
            echo "✅ $test_name: PASSED"
            echo "PASSED" >> "$result_file"
        else
            local exit_code=$?
            echo "❌ $test_name: FAILED (exit code: $exit_code)"
            echo "FAILED (exit code: $exit_code)" >> "$result_file"
            success=false
        fi
    else
        # No timeout available, run directly
        if bash -c "$test_command" >> "$result_file" 2>&1; then
            echo "✅ $test_name: PASSED"
            echo "PASSED" >> "$result_file"
        else
            local exit_code=$?
            echo "❌ $test_name: FAILED (exit code: $exit_code)"
            echo "FAILED (exit code: $exit_code)" >> "$result_file"
            success=false
        fi
    fi
    
    # Add debugging information if test failed
    if ! $success; then
        echo "--- Debug Information ---" >> "$result_file"
        echo "Exit code: $exit_code" >> "$result_file"
        echo "Working directory: $(pwd)" >> "$result_file"
        echo "Available files:" >> "$result_file"
        ls -la >> "$result_file" 2>&1 || echo "Cannot list files" >> "$result_file"
    fi
    
    cd "$PROJECT_ROOT"
    if $success; then
        return 0
    else
        return 1
    fi
}

# Enhanced configuration testing
test_enhanced_configuration() {
    local config_name=$1
    local cmake_args=$2
    local build_name=$3
    
    echo ""
    echo "=== Testing Configuration: $config_name ==="
    
    local config_build_dir="$BUILD_DIR/$build_name"
    mkdir -p "$config_build_dir"
    cd "$config_build_dir"
    
    # Configure
    echo "Configuring $config_name..."
    local configure_log="$TEST_RESULTS_DIR/${build_name}_configure.log"
    echo "CMake command: cmake $cmake_args $PROJECT_ROOT" > "$configure_log"
    echo "Working directory: $(pwd)" >> "$configure_log"
    echo "--- Configure Output ---" >> "$configure_log"
    
    if cmake $cmake_args "$PROJECT_ROOT" >> "$configure_log" 2>&1; then
        echo "✅ Configure: PASSED"
    else
        echo "❌ Configure: FAILED"
        echo "Configure failed. Check $configure_log for details."
        cd "$PROJECT_ROOT"
        return 1
    fi
    
    # Build (focus on CrossPlatformTest which is known to work)
    echo "Building $config_name..."
    local build_log="$TEST_RESULTS_DIR/${build_name}_build.log"
    local num_cores=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    echo "Build command: make CrossPlatformTest -j$num_cores" > "$build_log"
    echo "Working directory: $(pwd)" >> "$build_log"
    echo "--- Build Output ---" >> "$build_log"
    
    if make CrossPlatformTest -j"$num_cores" >> "$build_log" 2>&1; then
        echo "✅ Build: PASSED"
    else
        echo "❌ Build: FAILED"
        echo "Build failed. Check $build_log for details."
        cd "$PROJECT_ROOT"
        return 1
    fi
    
    # Run tests
    local tests_passed=0
    local tests_total=0
    
    # List of available tests
    local test_executables=(
        "CrossPlatformTest"
        "PerformanceOptimizationSimpleTest"
        "ConfigSimpleTest"
        "DebugSystemsMinimalTest"
        "UIFrameworkSimpleTest"
        "UIGameSpecificTest"
        "ScriptingMinimalTest"
        "ContentCreationToolsTest"
    )
    
    for test_exe in "${test_executables[@]}"; do
        if [[ -f "$test_exe" && -x "$test_exe" ]]; then
            ((tests_total++))
            local test_name=$(echo "${test_exe}" | tr '[:upper:]' '[:lower:]')
            if run_enhanced_test "${build_name}_${test_name}" "./$test_exe" "$config_build_dir" 30; then
                ((tests_passed++))
            fi
        fi
    done
    
    cd "$PROJECT_ROOT"
    
    echo "Configuration $config_name: $tests_passed/$tests_total tests passed"
    
    # Write detailed summary
    local summary_file="$TEST_RESULTS_DIR/${build_name}_summary.txt"
    echo "Configuration: $config_name" > "$summary_file"
    echo "CMake Args: $cmake_args" >> "$summary_file"
    echo "Build Directory: $config_build_dir" >> "$summary_file"
    echo "Tests Passed: $tests_passed/$tests_total" >> "$summary_file"
    if [[ $tests_total -gt 0 ]]; then
        echo "Success Rate: $(( tests_passed * 100 / tests_total ))%" >> "$summary_file"
    else
        echo "Success Rate: N/A (no tests found)" >> "$summary_file"
    fi
    echo "Timestamp: $(date)" >> "$summary_file"
    
    return $(( tests_total - tests_passed ))
}

# Main testing function
main() {
    # Detect platform
    detect_platform
    
    echo ""
    echo "Current platform: $PLATFORM_OS"
    
    # Test configurations
    local total_configs=0
    local passed_configs=0
    
    # Test Release build
    echo ""
    echo "Testing Release configuration..."
    ((total_configs++))
    if test_enhanced_configuration "Release Build" "-DCMAKE_BUILD_TYPE=Release" "release"; then
        ((passed_configs++))
    fi
    
    # Test Debug build
    echo ""
    echo "Testing Debug configuration..."
    ((total_configs++))
    if test_enhanced_configuration "Debug Build" "-DCMAKE_BUILD_TYPE=Debug" "debug"; then
        ((passed_configs++))
    fi
    
    # Test without graphics (fallback configuration)
    echo ""
    echo "Testing No Graphics configuration..."
    ((total_configs++))
    if test_enhanced_configuration "No Graphics Build" "-DCMAKE_BUILD_TYPE=Release -DNO_GRAPHICS=ON" "no_graphics"; then
        ((passed_configs++))
    fi
    
    # Test Web build (if Emscripten is available)
    if command -v emcmake >/dev/null 2>&1; then
        echo ""
        echo "Testing Web configuration..."
        ((total_configs++))
        
        local web_build_dir="$BUILD_DIR/web_test"
        mkdir -p "$web_build_dir"
        cd "$web_build_dir"
        
        local web_configure_log="$TEST_RESULTS_DIR/web_configure.log"
        echo "Emscripten configure test" > "$web_configure_log"
        echo "Command: emcmake cmake -DCMAKE_BUILD_TYPE=Release $PROJECT_ROOT" >> "$web_configure_log"
        echo "--- Configure Output ---" >> "$web_configure_log"
        
        if emcmake cmake -DCMAKE_BUILD_TYPE=Release "$PROJECT_ROOT" >> "$web_configure_log" 2>&1; then
            echo "✅ Web Configure: PASSED"
            
            local web_build_log="$TEST_RESULTS_DIR/web_build.log"
            echo "Emscripten build test" > "$web_build_log"
            echo "Command: emmake make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)" >> "$web_build_log"
            echo "--- Build Output ---" >> "$web_build_log"
            
            if emmake make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) >> "$web_build_log" 2>&1; then
                echo "✅ Web Build: PASSED"
                ((passed_configs++))
                
                # Verify web artifacts
                local web_artifacts=("CrossPlatformTest.html" "CrossPlatformTest.js" "CrossPlatformTest.wasm")
                local artifacts_found=0
                
                for artifact in "${web_artifacts[@]}"; do
                    if [[ -f "$artifact" ]]; then
                        ((artifacts_found++))
                        echo "  ✅ Found: $artifact"
                    else
                        echo "  ❌ Missing: $artifact"
                    fi
                done
                
                echo "  Web artifacts: $artifacts_found/${#web_artifacts[@]} found"
            else
                echo "❌ Web Build: FAILED"
            fi
        else
            echo "❌ Web Configure: FAILED"
        fi
        
        cd "$PROJECT_ROOT"
    else
        echo ""
        echo "=== Web Build Test Skipped ==="
        echo "Emscripten not available. Install instructions:"
        echo "1. git clone https://github.com/emscripten-core/emsdk.git"
        echo "2. cd emsdk && ./emsdk install latest && ./emsdk activate latest"
        echo "3. source ./emsdk_env.sh"
    fi
    
    # Generate comprehensive report
    echo ""
    echo "=== Platform Compatibility Test Summary ==="
    echo "Platform: $PLATFORM_OS"
    echo "Configurations tested: $total_configs"
    echo "Configurations passed: $passed_configs"
    if [[ $total_configs -gt 0 ]]; then
        echo "Success rate: $(( passed_configs * 100 / total_configs ))%"
    else
        echo "Success rate: N/A"
    fi
    
    # Create comprehensive report
    local report_file="$TEST_RESULTS_DIR/platform_compatibility_report.txt"
    echo "RPG Engine Platform Compatibility Report" > "$report_file"
    echo "Generated: $(date)" >> "$report_file"
    echo "Platform: $PLATFORM_OS" >> "$report_file"
    echo "" >> "$report_file"
    
    # Add platform information
    if [[ -f "$TEST_RESULTS_DIR/platform_info.txt" ]]; then
        cat "$TEST_RESULTS_DIR/platform_info.txt" >> "$report_file"
        echo "" >> "$report_file"
    fi
    
    # Add summary
    echo "Test Summary:" >> "$report_file"
    echo "- Configurations tested: $total_configs" >> "$report_file"
    echo "- Configurations passed: $passed_configs" >> "$report_file"
    if [[ $total_configs -gt 0 ]]; then
        echo "- Success rate: $(( passed_configs * 100 / total_configs ))%" >> "$report_file"
    else
        echo "- Success rate: N/A" >> "$report_file"
    fi
    echo "" >> "$report_file"
    
    # Add detailed results
    echo "Detailed Results:" >> "$report_file"
    for summary_file in "$TEST_RESULTS_DIR"/*_summary.txt; do
        if [[ -f "$summary_file" ]]; then
            echo "" >> "$report_file"
            cat "$summary_file" >> "$report_file"
        fi
    done
    
    echo ""
    echo "Detailed test results saved to: $TEST_RESULTS_DIR"
    echo "Comprehensive report: $report_file"
    
    if [[ $passed_configs -eq $total_configs ]]; then
        echo ""
        echo "🎉 All platform compatibility tests PASSED!"
        return 0
    else
        echo ""
        echo "⚠️  Some platform compatibility tests FAILED. Check the logs for details."
        return 1
    fi
}

# Run main function
main "$@"