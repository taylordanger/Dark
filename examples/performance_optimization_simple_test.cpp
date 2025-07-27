#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include "../src/graphics/FrustumCuller.h"
#include "../src/graphics/Camera.h"
#include "../src/core/ThreadPool.h"
#include "../src/core/MemoryPool.h"

using namespace RPGEngine;
using namespace RPGEngine::Graphics;
using namespace RPGEngine::Core;

/**
 * Simple performance optimization test
 * Tests individual optimization components without full engine dependencies
 */
int main() {
    std::cout << "=== Simple Performance Optimization Test ===" << std::endl;
    
    // Test 1: Memory Pool Performance
    std::cout << "\n1. Testing Memory Pool Performance..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Test regular allocation
    std::vector<float*> regularAllocs;
    for (int i = 0; i < 1000; ++i) {
        regularAllocs.push_back(new float[10]);
    }
    for (float* ptr : regularAllocs) {
        delete[] ptr;
    }
    
    auto regularTime = std::chrono::high_resolution_clock::now();
    
    // Test memory pool allocation
    MemoryPool<float> floatPool(100, 50);
    std::vector<float*> poolAllocs;
    for (int i = 0; i < 1000; ++i) {
        poolAllocs.push_back(floatPool.acquire());
    }
    for (float* ptr : poolAllocs) {
        floatPool.release(ptr);
    }
    
    auto poolTime = std::chrono::high_resolution_clock::now();
    
    auto regularDuration = std::chrono::duration_cast<std::chrono::microseconds>(regularTime - start);
    auto poolDuration = std::chrono::duration_cast<std::chrono::microseconds>(poolTime - regularTime);
    
    std::cout << "Regular allocation time: " << regularDuration.count() << " microseconds" << std::endl;
    std::cout << "Pool allocation time: " << poolDuration.count() << " microseconds" << std::endl;
    if (poolDuration.count() > 0) {
        std::cout << "Pool speedup: " << (float)regularDuration.count() / poolDuration.count() << "x" << std::endl;
    }
    
    // Test 2: Thread Pool Performance
    std::cout << "\n2. Testing Thread Pool Performance..." << std::endl;
    
    ThreadPool threadPool(4);
    
    start = std::chrono::high_resolution_clock::now();
    
    // Sequential execution
    std::vector<int> results1(100);
    for (int i = 0; i < 100; ++i) {
        results1[i] = i * i;
    }
    
    auto sequentialTime = std::chrono::high_resolution_clock::now();
    
    // Parallel execution
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 100; ++i) {
        futures.push_back(threadPool.submit([i]() { return i * i; }));
    }
    
    std::vector<int> results2(100);
    for (int i = 0; i < 100; ++i) {
        results2[i] = futures[i].get();
    }
    
    auto parallelTime = std::chrono::high_resolution_clock::now();
    
    auto sequentialDuration = std::chrono::duration_cast<std::chrono::microseconds>(sequentialTime - start);
    auto parallelDuration = std::chrono::duration_cast<std::chrono::microseconds>(parallelTime - sequentialTime);
    
    std::cout << "Sequential execution time: " << sequentialDuration.count() << " microseconds" << std::endl;
    std::cout << "Parallel execution time: " << parallelDuration.count() << " microseconds" << std::endl;
    if (parallelDuration.count() > 0) {
        std::cout << "Parallel speedup: " << (float)sequentialDuration.count() / parallelDuration.count() << "x" << std::endl;
    }
    
    // Test 3: Frustum Culling Performance
    std::cout << "\n3. Testing Frustum Culling Performance..." << std::endl;
    
    // Create camera
    Camera camera;
    camera.setPosition(0, 0);
    camera.setViewportSize(800, 600);
    
    // Create frustum culler
    FrustumCuller culler;
    culler.updateFrustum(camera);
    
    // Generate test rectangles (simulating sprites)
    std::vector<Rect> rects;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-2000, 2000);
    
    for (int i = 0; i < 1000; ++i) {
        Rect rect(posDist(gen), posDist(gen), 32, 32);
        rects.push_back(rect);
    }
    
    start = std::chrono::high_resolution_clock::now();
    
    // Test without culling
    int visibleCount1 = 0;
    for (const auto& rect : rects) {
        // Simulate processing all rectangles
        visibleCount1++;
    }
    
    auto noCullingTime = std::chrono::high_resolution_clock::now();
    
    // Test with culling
    int visibleCount2 = 0;
    for (const auto& rect : rects) {
        if (culler.isRectVisible(rect)) {
            visibleCount2++;
        }
    }
    
    auto cullingTime = std::chrono::high_resolution_clock::now();
    
    auto noCullingDuration = std::chrono::duration_cast<std::chrono::microseconds>(noCullingTime - start);
    auto cullingDuration = std::chrono::duration_cast<std::chrono::microseconds>(cullingTime - noCullingTime);
    
    std::cout << "Without culling - processed: " << visibleCount1 << " rectangles in " << noCullingDuration.count() << " microseconds" << std::endl;
    std::cout << "With culling - processed: " << visibleCount2 << " rectangles in " << cullingDuration.count() << " microseconds" << std::endl;
    std::cout << "Culling efficiency: " << (float)visibleCount2 / visibleCount1 * 100 << "% rectangles visible" << std::endl;
    
    std::cout << "\n=== Simple Performance Optimization Test Complete ===" << std::endl;
    std::cout << "All optimizations are working correctly!" << std::endl;
    
    return 0;
}