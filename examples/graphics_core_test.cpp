#include <iostream>
#include <memory>
#include "../src/graphics/OpenGLAPI.h"
#include "../src/graphics/ShaderManager.h"
#include "../src/graphics/Texture.h"
#include "../src/graphics/Camera.h"

using namespace RPGEngine::Graphics;

/**
 * Core graphics test
 * Tests the core graphics API without the full system dependencies
 */
int main() {
    std::cout << "=== RPG Engine Core Graphics Test ===" << std::endl;
    
    // Test 1: Graphics API initialization
    std::cout << "1. Testing Graphics API..." << std::endl;
    auto graphicsAPI = std::make_shared<OpenGLAPI>();
    
    if (!graphicsAPI->initialize(800, 600, "Core Graphics Test", false)) {
        std::cerr << "❌ Failed to initialize graphics API" << std::endl;
        return 1;
    }
    
    std::cout << "  ✅ Graphics API initialized" << std::endl;
    std::cout << "  ✅ API: " << graphicsAPI->getAPIName() << std::endl;
    std::cout << "  ✅ Version: " << graphicsAPI->getAPIVersion() << std::endl;
    std::cout << "  ✅ Window size: " << graphicsAPI->getWindowWidth() << "x" << graphicsAPI->getWindowHeight() << std::endl;
    
    // Test 2: Basic rendering operations
    std::cout << "2. Testing basic rendering..." << std::endl;
    
    // Clear screen with different colors
    for (int i = 0; i < 3; ++i) {
        graphicsAPI->beginFrame();
        
        float r = (i == 0) ? 1.0f : 0.0f;
        float g = (i == 1) ? 1.0f : 0.0f;
        float b = (i == 2) ? 1.0f : 0.0f;
        
        graphicsAPI->clear(r, g, b, 1.0f);
        graphicsAPI->endFrame();
        graphicsAPI->pollEvents();
        
        std::cout << "  ✅ Frame " << (i+1) << " rendered with color (" << r << ", " << g << ", " << b << ")" << std::endl;
    }
    
    // Test 3: Shader Manager
    std::cout << "3. Testing Shader Manager..." << std::endl;
    auto shaderManager = std::make_shared<ShaderManager>(graphicsAPI);
    
    if (!shaderManager->initialize()) {
        std::cerr << "❌ Failed to initialize shader manager" << std::endl;
        graphicsAPI->shutdown();
        return 1;
    }
    
    std::cout << "  ✅ Shader manager initialized" << std::endl;
    
    // Test 4: Texture creation
    std::cout << "4. Testing Texture creation..." << std::endl;
    auto texture = std::make_shared<Texture>(graphicsAPI);
    
    // Create a simple 2x2 white texture
    uint32_t whitePixels[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    if (!texture->createFromData(2, 2, TextureFormat::RGBA, whitePixels)) {
        std::cerr << "❌ Failed to create texture" << std::endl;
    } else {
        std::cout << "  ✅ Texture created: " << texture->getWidth() << "x" << texture->getHeight() << std::endl;
        std::cout << "  ✅ Texture handle: " << texture->getHandle() << std::endl;
        std::cout << "  ✅ Texture valid: " << (texture->isValid() ? "Yes" : "No") << std::endl;
    }
    
    // Test 5: Camera functionality
    std::cout << "5. Testing Camera..." << std::endl;
    auto camera = std::make_shared<Camera>();
    
    camera->setPosition(100.0f, 200.0f);
    camera->setViewportSize(800, 600);
    camera->setZoom(1.5f);
    
    float x, y;
    camera->getPosition(x, y);
    std::cout << "  ✅ Camera position: (" << x << ", " << y << ")" << std::endl;
    std::cout << "  ✅ Camera zoom: " << camera->getZoom() << std::endl;
    
    int width, height;
    camera->getViewportSize(width, height);
    std::cout << "  ✅ Camera viewport: " << width << "x" << height << std::endl;
    
    // Test 6: Buffer creation
    std::cout << "6. Testing Buffer creation..." << std::endl;
    
    // Create a simple vertex buffer
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  // Bottom left
         0.5f, -0.5f, 0.0f,  // Bottom right
         0.0f,  0.5f, 0.0f   // Top center
    };
    
    auto vertexBuffer = graphicsAPI->createVertexBuffer(vertices, sizeof(vertices), false);
    if (vertexBuffer != INVALID_HANDLE) {
        std::cout << "  ✅ Vertex buffer created: " << vertexBuffer << std::endl;
        graphicsAPI->deleteVertexBuffer(vertexBuffer);
        std::cout << "  ✅ Vertex buffer deleted" << std::endl;
    } else {
        std::cerr << "  ❌ Failed to create vertex buffer" << std::endl;
    }
    
    // Create a simple index buffer
    uint16_t indices[] = {0, 1, 2};
    auto indexBuffer = graphicsAPI->createIndexBuffer(indices, sizeof(indices), false);
    if (indexBuffer != INVALID_HANDLE) {
        std::cout << "  ✅ Index buffer created: " << indexBuffer << std::endl;
        graphicsAPI->deleteIndexBuffer(indexBuffer);
        std::cout << "  ✅ Index buffer deleted" << std::endl;
    } else {
        std::cerr << "  ❌ Failed to create index buffer" << std::endl;
    }
    
    // Cleanup
    std::cout << "7. Cleaning up..." << std::endl;
    texture.reset();
    camera.reset();
    shaderManager->shutdown();
    graphicsAPI->shutdown();
    
    std::cout << "✅ All core graphics tests completed successfully!" << std::endl;
    std::cout << "=== Core Graphics Test Complete ===" << std::endl;
    
    return 0;
}