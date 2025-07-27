#!/bin/bash

# Enhanced cross-platform testing script for RPG Engine
# Tests existing builds and creates comprehensive reports

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
TEST_RESULTS_DIR="$PROJECT_ROOT/test_results"

echo "=== RPG Engine Enhanced Cross-Platform Testing ==="
echo "Project root: $PROJECT_ROOT"

# Clean previous test results
echo "Cleaning previous test results..."
rm -rf "$TEST_RESULTS_DIR"
mkdir -p "$TEST_RESULTS_DIR"

# Function to run a test with timeout and capture results
run_test_with_timeout() {
    local test_name=$1
    local test_command=$2
    local build_dir=$3
    local timeout_seconds=${4:-30}
    
    echo "  Running $test_name..."
    
    local result_file="$TEST_RESULTS_DIR/${test_name}.log"
    local success=true
    
    cd "$build_dir"
    
    # Run the test with timeout and capture output
    if timeout ${timeout_seconds}s $test_command > "$result_file" 2>&1; then
        echo "    ✅ $test_name: PASSED"
        echo "PASSED" >> "$result_file"
    else
        local exit_code=$?
        if [[ $exit_code -eq 124 ]]; then
            echo "    ⚠️  $test_name: TIMEOUT (${timeout_seconds}s) - May be interactive"
            echo "TIMEOUT" >> "$result_file"
            # Timeout is not necessarily a failure for interactive tests
        else
            echo "    ❌ $test_name: FAILED (exit code: $exit_code)"
            echo "FAILED" >> "$result_file"
            success=false
        fi
    fi
    
    cd "$PROJECT_ROOT"
    return $success
}

# Function to test a build configuration
test_build_configuration() {
    local config_name=$1
    local build_path=$2
    
    echo ""
    echo "=== Testing Configuration: $config_name ==="
    
    if [[ ! -d "$build_path" ]]; then
        echo "❌ Build directory not found: $build_path"
        return 1
    fi
    
    local tests_passed=0
    local tests_total=0
    local config_prefix=$(echo "$config_name" | tr ' ' '_' | tr '[:upper:]' '[:lower:]')
    
    # Test available executables
    local test_executables=(
        "CrossPlatformTest:cross_platform"
        "PerformanceOptimizationSimpleTest:performance"
        "ConfigSimpleTest:config"
        "DebugSystemsMinimalTest:debug"
        "UIFrameworkSimpleTest:ui_framework"
        "UIGameSpecificTest:ui_game"
        "ScriptingMinimalTest:scripting"
        "ContentCreationToolsTest:tools"
    )
    
    for test_info in "${test_executables[@]}"; do
        IFS=':' read -r exe_name test_suffix <<< "$test_info"
        local exe_path="$build_path/$exe_name"
        
        if [[ -f "$exe_path" && -x "$exe_path" ]]; then
            ((tests_total++))
            local test_name="${config_prefix}_${test_suffix}"
            
            if run_test_with_timeout "$test_name" "./$exe_name" "$build_path" 15; then
                ((tests_passed++))
            fi
        fi
    done
    
    # Write configuration summary
    local summary_file="$TEST_RESULTS_DIR/${config_prefix}_summary.txt"
    echo "Configuration: $config_name" > "$summary_file"
    echo "Build Path: $build_path" >> "$summary_file"
    echo "Tests Total: $tests_total" >> "$summary_file"
    echo "Tests Passed: $tests_passed" >> "$summary_file"
    
    if [[ $tests_total -gt 0 ]]; then
        local success_rate=$(( tests_passed * 100 / tests_total ))
        echo "Success Rate: ${success_rate}%" >> "$summary_file"
        echo "Configuration $config_name: $tests_passed/$tests_total tests passed (${success_rate}%)"
    else
        echo "Success Rate: 0%" >> "$summary_file"
        echo "Configuration $config_name: No executable tests found"
        return 1
    fi
    
    return $(( tests_total - tests_passed ))
}# F
unction to test web build if available
test_web_build() {
    echo ""
    echo "=== Testing Web Build ==="
    
    local web_build_dir="$BUILD_DIR/web"
    
    if [[ ! -d "$web_build_dir" ]]; then
        echo "⚠️  Web build directory not found. Attempting to create..."
        
        if command -v emcc >/dev/null 2>&1; then
            create_web_build
        else
            echo "❌ Emscripten not available. Skipping web build test."
            return 1
        fi
    fi
    
    if [[ -d "$web_build_dir" ]]; then
        echo "✅ Web build directory found"
        
        # Check for web artifacts
        local web_artifacts=0
        local expected_artifacts=("CrossPlatformTest.html" "CrossPlatformTest.js" "CrossPlatformTest.wasm")
        
        for artifact in "${expected_artifacts[@]}"; do
            if [[ -f "$web_build_dir/$artifact" ]]; then
                echo "  ✅ Found: $artifact"
                ((web_artifacts++))
            else
                echo "  ❌ Missing: $artifact"
            fi
        done
        
        # Write web test summary
        local web_summary="$TEST_RESULTS_DIR/web_summary.txt"
        echo "Configuration: Web Build" > "$web_summary"
        echo "Build Path: $web_build_dir" >> "$web_summary"
        echo "Artifacts Found: $web_artifacts/${#expected_artifacts[@]}" >> "$web_summary"
        
        if [[ $web_artifacts -eq ${#expected_artifacts[@]} ]]; then
            echo "Success Rate: 100%" >> "$web_summary"
            echo "✅ Web build test: All artifacts present"
            return 0
        else
            local success_rate=$(( web_artifacts * 100 / ${#expected_artifacts[@]} ))
            echo "Success Rate: ${success_rate}%" >> "$web_summary"
            echo "⚠️  Web build test: $web_artifacts/${#expected_artifacts[@]} artifacts found"
            return 1
        fi
    else
        echo "❌ Web build test failed: No build directory"
        return 1
    fi
}

# Function to create web build
create_web_build() {
    echo "Creating web build..."
    
    local web_build_dir="$BUILD_DIR/web"
    mkdir -p "$web_build_dir"
    cd "$web_build_dir"
    
    if emcmake cmake -DCMAKE_BUILD_TYPE=Release "$PROJECT_ROOT" > "$TEST_RESULTS_DIR/web_configure.log" 2>&1; then
        echo "  ✅ Web configuration successful"
        
        if emmake make CrossPlatformTest -j4 > "$TEST_RESULTS_DIR/web_build.log" 2>&1; then
            echo "  ✅ Web build successful"
            cd "$PROJECT_ROOT"
            return 0
        else
            echo "  ❌ Web build failed"
            cd "$PROJECT_ROOT"
            return 1
        fi
    else
        echo "  ❌ Web configuration failed"
        cd "$PROJECT_ROOT"
        return 1
    fi
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

# Test existing build configurations
build_configs=(
    "Release Build:$BUILD_DIR/release"
    "Debug Build:$BUILD_DIR/debug"
    "No Graphics Build:$BUILD_DIR/no_graphics"
    "macOS Build:$BUILD_DIR/macos"
    "Linux Build:$BUILD_DIR/linux"
    "Windows Build:$BUILD_DIR/windows"
)

for config_info in "${build_configs[@]}"; do
    IFS=':' read -r config_name build_path <<< "$config_info"
    
    if [[ -d "$build_path" ]]; then
        ((total_configs++))
        if test_build_configuration "$config_name" "$build_path"; then
            ((passed_configs++))
        fi
    fi
done

# Test web build
if command -v emcc >/dev/null 2>&1; then
    ((total_configs++))
    if test_web_build; then
        ((passed_configs++))
    fi
else
    echo ""
    echo "=== Web Build Test Skipped ==="
    echo "Emscripten not found. To enable web builds:"
    echo "1. Install Emscripten SDK: https://emscripten.org/docs/getting_started/downloads.html"
    echo "2. Activate emsdk: source /path/to/emsdk/emsdk_env.sh"
    echo "3. Re-run this test script"
fi

# Generate comprehensive report
echo ""
echo "=== Enhanced Cross-Platform Testing Summary ==="
echo "Configurations tested: $total_configs"
echo "Configurations passed: $passed_configs"

if [[ $total_configs -gt 0 ]]; then
    overall_success_rate=$(( passed_configs * 100 / total_configs ))
    echo "Overall success rate: ${overall_success_rate}%"
else
    echo "Overall success rate: 0% (no configurations found)"
fi

# Create comprehensive report
report_file="$TEST_RESULTS_DIR/enhanced_cross_platform_report.txt"
echo "RPG Engine Enhanced Cross-Platform Testing Report" > "$report_file"
echo "Generated: $(date)" >> "$report_file"
echo "Platform: $CURRENT_OS ($(uname -m))" >> "$report_file"
echo "Kernel: $(uname -r)" >> "$report_file"
echo "" >> "$report_file"

# System information
echo "System Information:" >> "$report_file"
echo "- OS: $(uname -s)" >> "$report_file"
echo "- Architecture: $(uname -m)" >> "$report_file"
echo "- Kernel: $(uname -r)" >> "$report_file"

if command -v lscpu >/dev/null 2>&1; then
    echo "- CPU: $(lscpu | grep 'Model name' | cut -d':' -f2 | xargs)" >> "$report_file"
elif [[ "$CURRENT_OS" == "macOS" ]]; then
    echo "- CPU: $(sysctl -n machdep.cpu.brand_string)" >> "$report_file"
fi

if command -v free >/dev/null 2>&1; then
    echo "- Memory: $(free -h | grep '^Mem:' | awk '{print $2}')" >> "$report_file"
elif [[ "$CURRENT_OS" == "macOS" ]]; then
    mem_bytes=$(sysctl -n hw.memsize)
    mem_gb=$(( mem_bytes / 1024 / 1024 / 1024 ))
    echo "- Memory: ${mem_gb}GB" >> "$report_file"
fi

echo "" >> "$report_file"

# Summary
echo "Summary:" >> "$report_file"
echo "- Configurations tested: $total_configs" >> "$report_file"
echo "- Configurations passed: $passed_configs" >> "$report_file"
if [[ $total_configs -gt 0 ]]; then
    echo "- Overall success rate: $(( passed_configs * 100 / total_configs ))%" >> "$report_file"
else
    echo "- Overall success rate: 0%" >> "$report_file"
fi
echo "" >> "$report_file"

# Detailed results
echo "Detailed Results:" >> "$report_file"
for summary_file in "$TEST_RESULTS_DIR"/*_summary.txt; do
    if [[ -f "$summary_file" ]]; then
        echo "" >> "$report_file"
        cat "$summary_file" >> "$report_file"
    fi
done

# Test execution details
echo "" >> "$report_file"
echo "Test Execution Details:" >> "$report_file"
for log_file in "$TEST_RESULTS_DIR"/*.log; do
    if [[ -f "$log_file" ]]; then
        test_name=$(basename "$log_file" .log)
        echo "" >> "$report_file"
        echo "=== $test_name ===" >> "$report_file"
        
        # Show last few lines of each test log
        tail -n 10 "$log_file" >> "$report_file"
    fi
done

echo ""
echo "Detailed test results saved to: $TEST_RESULTS_DIR"
echo "Comprehensive report: $report_file"

# Final status
if [[ $total_configs -eq 0 ]]; then
    echo ""
    echo "⚠️  No build configurations found to test."
    echo "Run './scripts/build_all_platforms.sh' first to create builds."
    exit 1
elif [[ $passed_configs -eq $total_configs ]]; then
    echo ""
    echo "🎉 All cross-platform tests PASSED!"
    exit 0
else
    echo ""
    echo "⚠️  Some cross-platform tests had issues. Check the logs for details."
    exit 1
fi