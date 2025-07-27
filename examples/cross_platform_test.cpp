#include <iostream>
#include <string>
#include <chrono>
#include <thread>

// Platform detection
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #define PLATFORM_NAME "Windows"
#elif defined(PLATFORM_MACOS)
    #include <sys/utsname.h>
    #define PLATFORM_NAME "macOS"
#elif defined(PLATFORM_LINUX)
    #include <sys/utsname.h>
    #define PLATFORM_NAME "Linux"
#elif defined(PLATFORM_WEB)
    #include <emscripten.h>
    #define PLATFORM_NAME "Web"
#else
    #define PLATFORM_NAME "Unknown"
#endif

/**
 * Cross-platform compatibility test
 * Tests basic functionality across different platforms
 */
class CrossPlatformTest {
public:
    void runAllTests() {
        std::cout << "=== RPG Engine Cross-Platform Test ===" << std::endl;
        std::cout << "Platform: " << PLATFORM_NAME << std::endl;
        std::cout << "Compiler: " << getCompilerInfo() << std::endl;
        std::cout << "Build: " << getBuildInfo() << std::endl;
        std::cout << std::endl;
        
        bool allPassed = true;
        
        allPassed &= testBasicTypes();
        allPassed &= testThreading();
        allPassed &= testTiming();
        allPassed &= testMemoryAllocation();
        allPassed &= testPlatformSpecific();
        
        std::cout << std::endl;
        if (allPassed) {
            std::cout << "✅ All cross-platform tests PASSED!" << std::endl;
        } else {
            std::cout << "❌ Some cross-platform tests FAILED!" << std::endl;
        }
        
        std::cout << "=== Cross-Platform Test Complete ===" << std::endl;
    }
    
private:
    bool testBasicTypes() {
        std::cout << "Testing basic types..." << std::endl;
        
        // Test integer sizes
        bool passed = true;
        
        if (sizeof(int) < 4) {
            std::cout << "  ❌ int size too small: " << sizeof(int) << " bytes" << std::endl;
            passed = false;
        } else {
            std::cout << "  ✅ int size: " << sizeof(int) << " bytes" << std::endl;
        }
        
        if (sizeof(float) != 4) {
            std::cout << "  ❌ float size incorrect: " << sizeof(float) << " bytes" << std::endl;
            passed = false;
        } else {
            std::cout << "  ✅ float size: " << sizeof(float) << " bytes" << std::endl;
        }
        
        if (sizeof(double) != 8) {
            std::cout << "  ❌ double size incorrect: " << sizeof(double) << " bytes" << std::endl;
            passed = false;
        } else {
            std::cout << "  ✅ double size: " << sizeof(double) << " bytes" << std::endl;
        }
        
        // Test pointer size
        std::cout << "  ✅ pointer size: " << sizeof(void*) << " bytes" << std::endl;
        
        return passed;
    }
    
    bool testThreading() {
        std::cout << "Testing threading..." << std::endl;
        
        try {
            std::atomic<int> counter(0);
            const int numThreads = 4;
            const int incrementsPerThread = 1000;
            
            std::vector<std::thread> threads;
            
            for (int i = 0; i < numThreads; ++i) {
                threads.emplace_back([&counter, incrementsPerThread]() {
                    for (int j = 0; j < incrementsPerThread; ++j) {
                        counter++;
                    }
                });
            }
            
            for (auto& thread : threads) {
                thread.join();
            }
            
            int expected = numThreads * incrementsPerThread;
            if (counter.load() == expected) {
                std::cout << "  ✅ Threading test passed: " << counter.load() << " increments" << std::endl;
                return true;
            } else {
                std::cout << "  ❌ Threading test failed: expected " << expected << ", got " << counter.load() << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cout << "  ❌ Threading test failed with exception: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool testTiming() {
        std::cout << "Testing timing..." << std::endl;
        
        try {
            auto start = std::chrono::high_resolution_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            if (duration.count() >= 90 && duration.count() <= 150) {
                std::cout << "  ✅ Timing test passed: " << duration.count() << "ms" << std::endl;
                return true;
            } else {
                std::cout << "  ❌ Timing test failed: " << duration.count() << "ms (expected ~100ms)" << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cout << "  ❌ Timing test failed with exception: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool testMemoryAllocation() {
        std::cout << "Testing memory allocation..." << std::endl;
        
        try {
            const size_t allocSize = 1024 * 1024; // 1MB
            const int numAllocs = 10;
            
            std::vector<void*> allocations;
            
            // Allocate memory
            for (int i = 0; i < numAllocs; ++i) {
                void* ptr = std::malloc(allocSize);
                if (!ptr) {
                    std::cout << "  ❌ Memory allocation failed at iteration " << i << std::endl;
                    
                    // Clean up previous allocations
                    for (void* p : allocations) {
                        std::free(p);
                    }
                    return false;
                }
                allocations.push_back(ptr);
                
                // Write to memory to ensure it's accessible
                std::memset(ptr, i, allocSize);
            }
            
            // Verify memory contents
            for (int i = 0; i < numAllocs; ++i) {
                unsigned char* ptr = static_cast<unsigned char*>(allocations[i]);
                if (ptr[0] != static_cast<unsigned char>(i) || 
                    ptr[allocSize - 1] != static_cast<unsigned char>(i)) {
                    std::cout << "  ❌ Memory verification failed at allocation " << i << std::endl;
                    
                    // Clean up
                    for (void* p : allocations) {
                        std::free(p);
                    }
                    return false;
                }
            }
            
            // Clean up
            for (void* ptr : allocations) {
                std::free(ptr);
            }
            
            std::cout << "  ✅ Memory allocation test passed: " << numAllocs << " x " << allocSize << " bytes" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "  ❌ Memory allocation test failed with exception: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool testPlatformSpecific() {
        std::cout << "Testing platform-specific features..." << std::endl;
        
        #ifdef PLATFORM_WINDOWS
            return testWindows();
        #elif defined(PLATFORM_MACOS)
            return testMacOS();
        #elif defined(PLATFORM_LINUX)
            return testLinux();
        #elif defined(PLATFORM_WEB)
            return testWeb();
        #else
            std::cout << "  ⚠️  No platform-specific tests for unknown platform" << std::endl;
            return true;
        #endif
    }
    
    #ifdef PLATFORM_WINDOWS
    bool testWindows() {
        // Test Windows-specific functionality
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        
        std::cout << "  ✅ Windows system info:" << std::endl;
        std::cout << "    Processors: " << sysInfo.dwNumberOfProcessors << std::endl;
        std::cout << "    Page size: " << sysInfo.dwPageSize << " bytes" << std::endl;
        
        return true;
    }
    #endif
    
    #if defined(PLATFORM_MACOS) || defined(PLATFORM_LINUX)
    bool testUnix() {
        struct utsname unameData;
        if (uname(&unameData) == 0) {
            std::cout << "  ✅ Unix system info:" << std::endl;
            std::cout << "    System: " << unameData.sysname << std::endl;
            std::cout << "    Release: " << unameData.release << std::endl;
            std::cout << "    Machine: " << unameData.machine << std::endl;
            return true;
        } else {
            std::cout << "  ❌ Failed to get Unix system info" << std::endl;
            return false;
        }
    }
    #endif
    
    #ifdef PLATFORM_MACOS
    bool testMacOS() {
        return testUnix();
    }
    #endif
    
    #ifdef PLATFORM_LINUX
    bool testLinux() {
        return testUnix();
    }
    #endif
    
    #ifdef PLATFORM_WEB
    bool testWeb() {
        std::cout << "  ✅ Web platform detected" << std::endl;
        std::cout << "    Emscripten version: " << __EMSCRIPTEN_major__ << "." << __EMSCRIPTEN_minor__ << "." << __EMSCRIPTEN_tiny__ << std::endl;
        return true;
    }
    #endif
    
    std::string getCompilerInfo() {
        #ifdef _MSC_VER
            return "MSVC " + std::to_string(_MSC_VER);
        #elif defined(__clang__)
            return "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__);
        #elif defined(__GNUC__)
            return "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
        #else
            return "Unknown";
        #endif
    }
    
    std::string getBuildInfo() {
        #ifdef NDEBUG
            return "Release";
        #else
            return "Debug";
        #endif
    }
};

#ifdef PLATFORM_WEB
// Web platform main loop
void webMainLoop() {
    static CrossPlatformTest test;
    static bool hasRun = false;
    
    if (!hasRun) {
        test.runAllTests();
        hasRun = true;
    }
}
#endif

int main() {
    #ifdef PLATFORM_WEB
        // For web platform, use Emscripten's main loop
        emscripten_set_main_loop(webMainLoop, 0, 1);
    #else
        // For native platforms, run directly
        CrossPlatformTest test;
        test.runAllTests();
    #endif
    
    return 0;
}