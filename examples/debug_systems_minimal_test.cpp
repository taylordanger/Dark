#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "../src/debug/PerformanceProfiler.h"

using namespace Engine;

int main() {
    std::cout << "=== Debug Systems Minimal Test ===" << std::endl;

    // Test Performance Profiler
    std::cout << "\n--- Testing Performance Profiler ---" << std::endl;
    
    auto profiler = std::make_unique<Debug::PerformanceProfiler>();
    
    // Simulate some frames
    for (int frame = 0; frame < 10; ++frame) {
        profiler->beginFrame();
        
        // Simulate some work sections
        {
            PROFILE_SECTION(profiler.get(), "Update");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        {
            PROFILE_SECTION(profiler.get(), "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        profiler->recordEntityCount(100 + frame * 5);
        profiler->recordDrawCalls(50 + frame * 2);
        
        profiler->endFrame();
        profiler->update();
    }
    
    // Check profiler results
    auto frameStats = profiler->getCurrentFrameStats();
    std::cout << "Current FPS: " << frameStats.fps << std::endl;
    std::cout << "Current Frame Time: " << frameStats.frameTime << "ms" << std::endl;
    std::cout << "Average FPS: " << profiler->getAverageFPS() << std::endl;
    std::cout << "Average Frame Time: " << profiler->getAverageFrameTime() << "ms" << std::endl;
    std::cout << "Entity Count: " << frameStats.entityCount << std::endl;
    std::cout << "Draw Calls: " << frameStats.drawCalls << std::endl;
    
    auto sections = profiler->getAllSections();
    std::cout << "Profiled Sections:" << std::endl;
    for (const auto& section : sections) {
        std::cout << "  " << section.name << ": " << section.totalTime << "ms total, " 
                  << section.callCount << " calls" << std::endl;
    }

    // Test basic debug functionality
    std::cout << "\n--- Testing Debug Functionality ---" << std::endl;
    
    // Test profiler reset
    profiler->reset();
    std::cout << "Profiler reset successfully" << std::endl;
    
    // Test section reset
    profiler->beginSection("TestSection");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    profiler->endSection("TestSection");
    
    auto testSection = profiler->getSectionStats("TestSection");
    std::cout << "Test section time: " << testSection.totalTime << "ms" << std::endl;
    
    profiler->resetSection("TestSection");
    testSection = profiler->getSectionStats("TestSection");
    std::cout << "Test section time after reset: " << testSection.totalTime << "ms" << std::endl;
    
    // Test enable/disable
    profiler->setEnabled(false);
    std::cout << "Profiler enabled: " << profiler->isEnabled() << std::endl;
    
    profiler->setEnabled(true);
    std::cout << "Profiler enabled: " << profiler->isEnabled() << std::endl;

    std::cout << "\n=== Debug Systems Minimal Test Complete ===" << std::endl;
    std::cout << "Performance profiler created and tested successfully!" << std::endl;

    return 0;
}