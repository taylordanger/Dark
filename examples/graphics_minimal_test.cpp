#include <iostream>
#include "../src/graphics/OpenGLAPI.h"
#include "../src/graphics/SpriteRenderer.h"
#include "../src/graphics/ShaderManager.h"
#include "../src/graphics/Texture.h"
#include "../src/graphics/Camera.h"

using namespace RPGEngine::Graphics;

/**
 * Minimal graphics test
 * Tests basic graphics functionality without physics dependencies
 */
int main() {
    std::cout << "=== RPG Engine Graphics System Test ===" << std::endl;
    
    // Create graphics API
    auto graphicsAPI = std::make_shared<OpenGLAPI>();
    
    // Initialize graphics
    std::cout << "Initializing graphics..." << std::endl;
    if (!graphicsAPI->initialize(800, 600, "Graphics Test", false)) {
        std::cerr << "❌ Failed to initialize graphics API" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Graphics API initialized" << std::endl;
    std::cout << "  API: " << graphicsAPI->getAPIName() << std::endl;
    std::cout << "  Version: " << graphicsAPI->getAPIVersion() << std::endl;
    
    // Create shader manager
    auto shaderManager = std::make_shared<ShaderManager>(graphicsAPI);
    if (!shaderManager->initialize()) {
        std::cerr << "❌ Failed to initialize shader manager" << std::endl;
        graphicsAPI->shutdown();
        return 1;
    }
    
    std::cout << "✅ Shader manager initialized" << std::endl;
    
    // Create camera
    auto camera = std::make_shared<Camera>();
    camera->setPosition(0.0f, 0.0f);
    camera->setViewportSize(800, 600);
    
    std::cout << "✅ Camera created" << std::endl;
    
    // Create sprite renderer
    auto spriteRenderer = std::make_shared<SpriteRenderer>(graphicsAPI, shaderManager);
    if (!spriteRenderer->initialize()) {
        std::cerr << "❌ Failed to initialize sprite renderer" << std::endl;
        shaderManager->shutdown();
        graphicsAPI->shutdown();
        return 1;
    }
    
    // Set camera for sprite renderer
    spriteRenderer->setCamera(camera.get());
    
    std::cout << "✅ Sprite renderer initialized" << std::endl;
    
    // Test basic rendering loop (just a few frames)
    std::cout << "Testing rendering loop..." << std::endl;
    
    int frameCount = 0;
    const int maxFrames = 10;
    
    while (!graphicsAPI->shouldClose() && frameCount < maxFrames) {
        // Begin frame
        graphicsAPI->beginFrame();
        graphicsAPI->clear(0.2f, 0.3f, 0.3f, 1.0f);
        
        // Begin sprite rendering
        spriteRenderer->begin();
        
        // Draw a simple colored rectangle (using white texture)
        spriteRenderer->drawRectangle(100.0f, 100.0f, 200.0f, 150.0f, 
                                    {1.0f, 0.5f, 0.2f, 1.0f}, true);
        
        // End sprite rendering
        spriteRenderer->end();
        
        // End frame
        graphicsAPI->endFrame();
        graphicsAPI->pollEvents();
        
        frameCount++;
    }
    
    std::cout << "✅ Rendered " << frameCount << " frames successfully" << std::endl;
    
    // Cleanup
    spriteRenderer.reset();
    shaderManager->shutdown();
    graphicsAPI->shutdown();
    
    std::cout << "✅ Graphics system test completed successfully!" << std::endl;
    std::cout << "=== Graphics Test Complete ===" << std::endl;
    
    return 0;
}