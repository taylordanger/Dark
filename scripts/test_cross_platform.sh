#!/bin/bash

# Cross-platform testing script for RPG Engine
# Tests the engine on different platforms and configurations

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
TEST_RESULTS_DIR="$PROJECT_ROOT/test_results"

echo "=== RPG Engine Cross-Platform Testing Script ==="
echo "Project root: $PROJECT_ROOT"

# Clean previous test results
echo "Cleaning previous test results..."
rm -rf "$TEST_RESULTS_DIR"
mkdir -p "$TEST_RESULTS_DIR"

# Function to run a test and capture results
run_test() {
    local test_name=$1
    local test_command=$2
    local build_dir=$3
    
    echo ""
    echo "=== Running Test: $test_name ==="
    
    local result_file="$TEST_RESULTS_DIR/${test_name}.log"
    local success=true
    
    # Run the test and capture output
    if cd "$build_dir" && eval "$test_command" > "$result_file" 2>&1; then
        echo "✅ $test_name: PASSED"
        echo "PASSED" >> "$result_file"
    else
        echo "❌ $test_name: FAILED"
        echo "FAILED" >> "$result_file"
        success=false
    fi
    
    cd "$PROJECT_ROOT"
    return $success
}

# Function to build and test a configuration
test_configuration() {
    local config_name=$1
    local cmake_args=$2
    local build_name=$3
    
    echo ""
    echo "=== Testing Configuration: $config_name ==="
    
    local config_build_dir="$BUILD_DIR/$build_name"
    
    # Check if build already exists and has executables
    if [[ -d "$config_build_dir" ]] && [[ -f "$config_build_dir/CrossPlatformTest" ]]; then
        echo "✅ Using existing build: $config_name"
    else
        mkdir -p "$config_build_dir"
        cd "$config_build_dir"
        
        # Configure
        echo "Configuring $config_name..."
        if cmake $cmake_args "$PROJECT_ROOT" > "$TEST_RESULTS_DIR/${build_name}_configure.log" 2>&1; then
            echo "✅ Configure: PASSED"
        else
            echo "❌ Configure: FAILED"
            cd "$PROJECT_ROOT"
            return 1
        fi
        
        # Build - use ninja if available, otherwise make
        echo "Building $config_name..."
        local build_cmd="make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
        if command -v ninja >/dev/null 2>&1 && [[ -f "build.ninja" ]]; then
            build_cmd="ninja"
        fi
        
        if $build_cmd > "$TEST_RESULTS_DIR/${build_name}_build.log" 2>&1; then
            echo "✅ Build: PASSED"
        else
            echo "❌ Build: FAILED"
            cd "$PROJECT_ROOT"
            return 1
        fi
    fi
    
    # Run tests
    local tests_passed=0
    local tests_total=0
    
    # Cross-platform test
    if [[ -f "CrossPlatformTest" ]]; then
        ((tests_total++))
        if run_test "${build_name}_cross_platform" "./CrossPlatformTest" "$config_build_dir"; then
            ((tests_passed++))
        fi
    fi
    
    # Performance optimization test
    if [[ -f "PerformanceOptimizationSimpleTest" ]]; then
        ((tests_total++))
        if run_test "${build_name}_performance" "./PerformanceOptimizationSimpleTest" "$config_build_dir"; then
            ((tests_passed++))
        fi
    fi
    
    # Configuration test
    if [[ -f "ConfigSimpleTest" ]]; then
        ((tests_total++))
        if run_test "${build_name}_config" "./ConfigSimpleTest" "$config_build_dir"; then
            ((tests_passed++))
        fi
    fi
    
    # Debug systems test
    if [[ -f "DebugSystemsMinimalTest" ]]; then
        ((tests_total++))
        if run_test "${build_name}_debug" "./DebugSystemsMinimalTest" "$config_build_dir"; then
            ((tests_passed++))
        fi
    fi
    
    # UI tests
    if [[ -f "UIFrameworkSimpleTest" ]]; then
        ((tests_total++))
        if run_test "${build_name}_ui_framework" "./UIFrameworkSimpleTest" "$config_build_dir"; then
            ((tests_passed++))
        fi
    fi
    
    if [[ -f "UIGameSpecificTest" ]]; then
        ((tests_total++))
        if run_test "${build_name}_ui_game" "./UIGameSpecificTest" "$config_build_dir"; then
            ((tests_passed++))
        fi
    fi
    
    # Scripting test
    if [[ -f "ScriptingMinimalTest" ]]; then
        ((tests_total++))
        if run_test "${build_name}_scripting" "./ScriptingMinimalTest" "$config_build_dir"; then
            ((tests_passed++))
        fi
    fi
    
    # Content creation tools test
    if [[ -f "ContentCreationToolsTest" ]]; then
        ((tests_total++))
        if run_test "${build_name}_tools" "./ContentCreationToolsTest" "$config_build_dir"; then
            ((tests_passed++))
        fi
    fi
    
    cd "$PROJECT_ROOT"
    
    echo "Configuration $config_name: $tests_passed/$tests_total tests passed"
    
    # Write summary
    echo "Configuration: $config_name" >> "$TEST_RESULTS_DIR/${build_name}_summary.txt"
    echo "Tests Passed: $tests_passed/$tests_total" >> "$TEST_RESULTS_DIR/${build_name}_summary.txt"
    echo "Success Rate: $(( tests_passed * 100 / tests_total ))%" >> "$TEST_RESULTS_DIR/${build_name}_summary.txt"
    
    return $(( tests_total - tests_passed ))
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

# Test configurations
total_configs=0
passed_configs=0

# Test Release build
((total_configs++))
if test_configuration "Release Build" "-DCMAKE_BUILD_TYPE=Release" "release"; then
    ((passed_configs++))
fi

# Test Debug build
((total_configs++))
if test_configuration "Debug Build" "-DCMAKE_BUILD_TYPE=Debug" "debug"; then
    ((passed_configs++))
fi

# Test without graphics (if dependencies are missing)
((total_configs++))
if test_configuration "No Graphics Build" "-DCMAKE_BUILD_TYPE=Release -DNO_GRAPHICS=ON" "no_graphics"; then
    ((passed_configs++))
fi

# Test Web build (if Emscripten is available)
if command -v emcmake >/dev/null 2>&1; then
    echo ""
    echo "=== Testing Web Configuration ==="
    
    ((total_configs++))
    local web_build_dir="$BUILD_DIR/web_test"
    mkdir -p "$web_build_dir"
    cd "$web_build_dir"
    
    # Configure with Emscripten
    echo "Configuring Web build..."
    if emcmake cmake -DCMAKE_BUILD_TYPE=Release "$PROJECT_ROOT" > "$TEST_RESULTS_DIR/web_configure.log" 2>&1; then
        echo "✅ Web Configure: PASSED"
        
        # Build
        echo "Building Web..."
        if emmake make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) > "$TEST_RESULTS_DIR/web_build.log" 2>&1; then
            echo "✅ Web Build: PASSED"
            ((passed_configs++))
            
            # Check for generated files
            if [[ -f "CrossPlatformTest.html" ]]; then
                echo "✅ Web artifacts generated successfully"
                
                # Test web artifacts
                local web_tests_passed=0
                local web_tests_total=0
                
                # Check HTML file
                if [[ -f "CrossPlatformTest.html" ]]; then
                    ((web_tests_total++))
                    if grep -q "canvas" "CrossPlatformTest.html"; then
                        echo "  ✅ HTML file contains canvas element"
                        ((web_tests_passed++))
                    else
                        echo "  ❌ HTML file missing canvas element"
                    fi
                fi
                
                # Check WASM file
                if [[ -f "CrossPlatformTest.wasm" ]]; then
                    ((web_tests_total++))
                    if [[ -s "CrossPlatformTest.wasm" ]]; then
                        echo "  ✅ WASM file generated and not empty"
                        ((web_tests_passed++))
                    else
                        echo "  ❌ WASM file is empty"
                    fi
                fi
                
                # Check JS file
                if [[ -f "CrossPlatformTest.js" ]]; then
                    ((web_tests_total++))
                    if grep -q "Module" "CrossPlatformTest.js"; then
                        echo "  ✅ JS file contains Module definition"
                        ((web_tests_passed++))
                    else
                        echo "  ❌ JS file missing Module definition"
                    fi
                fi
                
                echo "  Web artifacts test: $web_tests_passed/$web_tests_total passed"
            else
                echo "⚠️  Web artifacts not found"
            fi
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
    echo "Emscripten not found. To enable web builds:"
    echo "1. Install Emscripten SDK: https://emscripten.org/docs/getting_started/downloads.html"
    echo "2. Activate emsdk: source /path/to/emsdk/emsdk_env.sh"
    echo "3. Re-run this test script"
fi

# Generate final report
echo ""
echo "=== Cross-Platform Testing Summary ==="
echo "Configurations tested: $total_configs"
echo "Configurations passed: $passed_configs"
echo "Success rate: $(( passed_configs * 100 / total_configs ))%"

# Create comprehensive report
report_file="$TEST_RESULTS_DIR/cross_platform_report.txt"
echo "RPG Engine Cross-Platform Testing Report" > "$report_file"
echo "Generated: $(date)" >> "$report_file"
echo "Platform: $CURRENT_OS" >> "$report_file"
echo "" >> "$report_file"
echo "Summary:" >> "$report_file"
echo "- Configurations tested: $total_configs" >> "$report_file"
echo "- Configurations passed: $passed_configs" >> "$report_file"
echo "- Success rate: $(( passed_configs * 100 / total_configs ))%" >> "$report_file"
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
    echo "🎉 All cross-platform tests PASSED!"
    exit 0
else
    echo ""
    echo "⚠️  Some cross-platform tests FAILED. Check the logs for details."
    exit 1
fi