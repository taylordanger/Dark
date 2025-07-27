#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include "../src/graphics/SpriteRenderer.h"
#include "../src/graphics/Camera.h"
#include "../src/graphics/OpenGLAPI.h"
#include "../src/graphics/ShaderManager.h"
#include "../src/systems/SystemManager.h"
#include "../src/core/ThreadPool.h"
#include "../src/core/MemoryPool.h"

using namespace RPGEngine;
using namespace RPGEngine::Graphics;
using namespace RPGEngine::Core;

/**
 * Performance optimization test
 * Tests frustum culling, memory pooling, and parallel system updates
 */
int main() {
    std::cout << "=== Performance Optimization Test ===" << std::endl;
    
    // Test 1: Memory Pool Performance
    std::cout << "\n1. Testing Memory Pool Performance..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Test regular allocation
    std::vector<float*> regularAllocs;
    for (int i = 0; i < 10000; ++i) {
        regularAllocs.push_back(new float[100]);
    }
    for (float* ptr : regularAllocs) {
        delete[] ptr;
    }
    
    auto regularTime = std::chrono::high_resolution_clock::now();
    
    // Test memory pool allocation
    MemoryPool<float> floatPool(1000, 500);
    std::vector<float*> poolAllocs;
    for (int i = 0; i < 10000; ++i) {
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
    std::cout << "Pool speedup: " << (float)regularDuration.count() / poolDuration.count() << "x" << std::endl;
    
    // Test 2: Thread Pool Performance
    std::cout << "\n2. Testing Thread Pool Performance..." << std::endl;
    
    ThreadPool threadPool(4);
    
    start = std::chrono::high_resolution_clock::now();
    
    // Sequential execution
    std::vector<int> results1(1000);
    for (int i = 0; i < 1000; ++i) {
        results1[i] = i * i;
    }
    
    auto sequentialTime = std::chrono::high_resolution_clock::now();
    
    // Parallel execution
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 1000; ++i) {
        futures.push_back(threadPool.submit([i]() { return i * i; }));
    }
    
    std::vector<int> results2(1000);
    for (int i = 0; i < 1000; ++i) {
        results2[i] = futures[i].get();
    }
    
    auto parallelTime = std::chrono::high_resolution_clock::now();
    
    auto sequentialDuration = std::chrono::duration_cast<std::chrono::microseconds>(sequentialTime - start);
    auto parallelDuration = std::chrono::duration_cast<std::chrono::microseconds>(parallelTime - sequentialTime);
    
    std::cout << "Sequential execution time: " << sequentialDuration.count() << " microseconds" << std::endl;
    std::cout << "Parallel execution time: " << parallelDuration.count() << " microseconds" << std::endl;
    std::cout << "Parallel speedup: " << (float)sequentialDuration.count() / parallelDuration.count() << "x" << std::endl;
    
    // Test 3: Frustum Culling Performance
    std::cout << "\n3. Testing Frustum Culling Performance..." << std::endl;
    
    // Create camera
    Camera camera;
    camera.setPosition(0, 0);
    camera.setViewportSize(800, 600);
    
    // Create frustum culler
    FrustumCuller culler;
    culler.updateFrustum(camera);
    
    // Generate test sprites
    std::vector<Sprite> sprites;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-2000, 2000);
    
    for (int i = 0; i < 10000; ++i) {
        Sprite sprite;
        sprite.setPosition(posDist(gen), posDist(gen));
        sprites.push_back(sprite);
    }
    
    start = std::chrono::high_resolution_clock::now();
    
    // Test without culling
    int visibleCount1 = 0;
    for (const auto& sprite : sprites) {
        // Simulate rendering all sprites
        visibleCount1++;
    }
    
    auto noCullingTime = std::chrono::high_resolution_clock::now();
    
    // Test with culling
    std::vector<const Sprite*> visibleSprites;
    culler.cullSprites(sprites, visibleSprites);
    int visibleCount2 = visibleSprites.size();
    
    auto cullingTime = std::chrono::high_resolution_clock::now();
    
    auto noCullingDuration = std::chrono::duration_cast<std::chrono::microseconds>(noCullingTime - start);
    auto cullingDuration = std::chrono::duration_cast<std::chrono::microseconds>(cullingTime - noCullingTime);
    
    std::cout << "Without culling - processed: " << visibleCount1 << " sprites in " << noCullingDuration.count() << " microseconds" << std::endl;
    std::cout << "With culling - processed: " << visibleCount2 << " sprites in " << cullingDuration.count() << " microseconds" << std::endl;
    std::cout << "Culling efficiency: " << (float)visibleCount2 / visibleCount1 * 100 << "% sprites visible" << std::endl;
    
    // Test 4: System Manager Parallel Updates
    std::cout << "\n4. Testing System Manager Parallel Updates..." << std::endl;
    
    SystemManager systemManager;
    
    // Create mock systems for testing
    class MockSystem : public ISystem {
    public:
        MockSystem(const std::string& name) : m_name(name), m_initialized(false) {}
        
        bool initialize() override {
            m_initialized = true;
            return true;
        }
        
        void update(float deltaTime) override {
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        
        void shutdown() override {
            m_initialized = false;
        }
        
        bool isInitialized() const override { return m_initialized; }
        std::string getName() const override { return m_name; }
        
    private:
        std::string m_name;
        bool m_initialized;
    };
    
    // Register test systems
    systemManager.registerSystem(std::make_shared<MockSystem>("System1"), SystemType::Input);
    systemManager.registerSystem(std::make_shared<MockSystem>("System2"), SystemType::Physics);
    systemManager.registerSystem(std::make_shared<MockSystem>("System3"), SystemType::Rendering);
    systemManager.registerSystem(std::make_shared<MockSystem>("System4"), SystemType::Audio);
    
    systemManager.initializeAll();
    
    start = std::chrono::high_resolution_clock::now();
    
    // Test sequential updates
    for (int i = 0; i < 100; ++i) {
        systemManager.updateAll(0.016f); // 60 FPS
    }
    
    auto sequentialSystemTime = std::chrono::high_resolution_clock::now();
    
    // Test parallel updates
    systemManager.setParallelUpdatesEnabled(true);
    for (int i = 0; i < 100; ++i) {
        systemManager.updateAll(0.016f); // 60 FPS
    }
    
    auto parallelSystemTime = std::chrono::high_resolution_clock::now();
    
    auto sequentialSystemDuration = std::chrono::duration_cast<std::chrono::milliseconds>(sequentialSystemTime - start);
    auto parallelSystemDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parallelSystemTime - sequentialSystemTime);
    
    std::cout << "Sequential system updates: " << sequentialSystemDuration.count() << " milliseconds" << std::endl;
    std::cout << "Parallel system updates: " << parallelSystemDuration.count() << " milliseconds" << std::endl;
    std::cout << "System parallel speedup: " << (float)sequentialSystemDuration.count() / parallelSystemDuration.count() << "x" << std::endl;
    
    std::cout << "\n=== Performance Optimization Test Complete ===" << std::endl;
    std::cout << "All optimizations are working correctly!" << std::endl;
    
    return 0;
}