#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include "../src/graphics/IGraphicsAPI.h"
#include "../src/graphics/ShaderManager.h"
#include "../src/graphics/SpriteRenderer.h"
#include "../src/graphics/Texture.h"
#include "../src/graphics/Sprite.h"
#include "../src/graphics/Camera.h"
#include "../src/graphics/CameraSystem.h"
#include "../src/components/ComponentManager.h"
#include "../src/entities/EntityManager.h"

using namespace RPGEngine;
using namespace RPGEngine::Graphics;

int main() {
    std::cout << "=== Camera System Test ===\n" << std::endl;
    
    // Create graphics API
    auto graphicsAPI = createGraphicsAPI();
    
    // Initialize graphics API
    if (!graphicsAPI->initialize(800, 600, "RPG Engine Camera Test", false)) {
        std::cerr << "Failed to initialize graphics API" << std::endl;
        return -1;
    }
    
    std::cout << "Graphics API: " << graphicsAPI->getAPIName() << " " << graphicsAPI->getAPIVersion() << std::endl;
    
    // Create shader manager
    auto shaderManager = std::make_shared<ShaderManager>(graphicsAPI);
    if (!shaderManager->initialize()) {
        std::cerr << "Failed to initialize shader manager" << std::endl;
        graphicsAPI->shutdown();
        return -1;
    }
    
    // Create sprite renderer
    auto spriteRenderer = std::make_shared<SpriteRenderer>(graphicsAPI, shaderManager);
    if (!spriteRenderer->initialize()) {
        std::cerr << "Failed to initialize sprite renderer" << std::endl;
        shaderManager->shutdown();
        graphicsAPI->shutdown();
        return -1;
    }
    
    // Create entity manager
    EntityManager entityManager;
    if (!entityManager.initialize()) {
        std::cerr << "Failed to initialize entity manager" << std::endl;
        spriteRenderer->shutdown();
        shaderManager->shutdown();
        graphicsAPI->shutdown();
        return -1;
    }
    
    // Create component manager
    auto componentManager = std::make_shared<ComponentManager>();
    if (!componentManager->initialize()) {
        std::cerr << "Failed to initialize component manager" << std::endl;
        entityManager.shutdown();
        spriteRenderer->shutdown();
        shaderManager->shutdown();
        graphicsAPI->shutdown();
        return -1;
    }
    
    // Create camera system
    auto cameraSystem = std::make_shared<CameraSystem>(componentManager);
    if (!cameraSystem->initialize()) {
        std::cerr << "Failed to initialize camera system" << std::endl;
        componentManager->shutdown();
        entityManager.shutdown();
        spriteRenderer->shutdown();
        shaderManager->shutdown();
        graphicsAPI->shutdown();
        return -1;
    }
    
    // Create textures
    auto backgroundTexture = std::make_shared<Texture>(graphicsAPI);
    if (!backgroundTexture->loadFromFile("assets/background.png")) {
        std::cerr << "Failed to load background texture, creating a placeholder" << std::endl;
        
        // Create a checkerboard pattern as placeholder
        const int texSize = 512;
        uint8_t textureData[texSize * texSize * 4];
        
        for (int y = 0; y < texSize; ++y) {
            for (int x = 0; x < texSize; ++x) {
                uint8_t color = ((x / 32 + y / 32) % 2) ? 200 : 100;
                int index = (y * texSize + x) * 4;
                textureData[index + 0] = color;        // R
                textureData[index + 1] = color;        // G
                textureData[index + 2] = color;        // B
                textureData[index + 3] = 255;          // A
            }
        }
        
        backgroundTexture->createFromData(texSize, texSize, TextureFormat::RGBA, textureData);
    }
    
    auto playerTexture = std::make_shared<Texture>(graphicsAPI);
    if (!playerTexture->loadFromFile("assets/player.png")) {
        std::cerr << "Failed to load player texture, creating a placeholder" << std::endl;
        
        // Create a simple player texture as placeholder
        const int texSize = 32;
        uint8_t textureData[texSize * texSize * 4];
        
        for (int y = 0; y < texSize; ++y) {
            for (int x = 0; x < texSize; ++x) {
                int centerX = x - texSize / 2;
                int centerY = y - texSize / 2;
                float distance = std::sqrt(centerX * centerX + centerY * centerY);
                
                uint8_t r = 255;
                uint8_t g = 100;
                uint8_t b = 100;
                uint8_t a = (distance < texSize / 2) ? 255 : 0;
                
                int index = (y * texSize + x) * 4;
                textureData[index + 0] = r;
                textureData[index + 1] = g;
                textureData[index + 2] = b;
                textureData[index + 3] = a;
            }
        }
        
        playerTexture->createFromData(texSize, texSize, TextureFormat::RGBA, textureData);
    }
    
    // Create sprites
    Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setPosition(0, 0);
    
    Sprite playerSprite(playerTexture);
    playerSprite.setPosition(400, 300);
    
    // Create grid of sprites
    std::vector<Sprite> gridSprites;
    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            Sprite sprite(playerTexture);
            sprite.setPosition(x * 100.0f, y * 100.0f);
            sprite.setColor(Color(
                0.5f + x * 0.05f,
                0.5f + y * 0.05f,
                0.5f
            ));
            gridSprites.push_back(sprite);
        }
    }
    
    // Create player entity
    Entity playerEntity = entityManager.createEntity("Player");
    
    // Set up cameras
    auto mainCamera = cameraSystem->getCamera("main");
    mainCamera->setPosition(400, 300);
    
    auto zoomedCamera = cameraSystem->createCamera("zoomed");
    zoomedCamera->setPosition(400, 300);
    zoomedCamera->setZoom(2.0f);
    
    auto followCamera = cameraSystem->createCamera("follow");
    cameraSystem->setCameraToFollowEntity("follow", playerEntity);
    
    // Main loop
    float time = 0.0f;
    float deltaTime = 0.016f;
    float playerX = 400.0f;
    float playerY = 300.0f;
    float playerSpeed = 200.0f;
    std::string currentCamera = "main";
    
    while (!graphicsAPI->shouldClose()) {
        // Begin frame
        graphicsAPI->beginFrame();
        graphicsAPI->clear(0.2f, 0.3f, 0.3f, 1.0f);
        
        // Handle input (simulated)
        bool moveLeft = (std::sin(time * 0.5f) < 0);
        bool moveRight = (std::sin(time * 0.5f) > 0);
        bool moveUp = (std::cos(time * 0.7f) < 0);
        bool moveDown = (std::cos(time * 0.7f) > 0);
        
        // Update player position
        if (moveLeft) playerX -= playerSpeed * deltaTime;
        if (moveRight) playerX += playerSpeed * deltaTime;
        if (moveUp) playerY -= playerSpeed * deltaTime;
        if (moveDown) playerY += playerSpeed * deltaTime;
        
        // Update player sprite
        playerSprite.setPosition(playerX, playerY);
        
        // Switch camera every 3 seconds
        if (std::fmod(time, 3.0f) < 0.1f && std::fmod(time - deltaTime, 3.0f) >= 0.1f) {
            if (currentCamera == "main") {
                currentCamera = "zoomed";
            } else if (currentCamera == "zoomed") {
                currentCamera = "follow";
            } else {
                currentCamera = "main";
            }
            
            cameraSystem->setActiveCamera(currentCamera);
            std::cout << "Switched to camera: " << currentCamera << std::endl;
        }
        
        // Update camera system
        cameraSystem->update(deltaTime);
        
        // Get active camera
        auto activeCamera = cameraSystem->getActiveCamera();
        
        // Set camera matrices for sprite renderer
        spriteRenderer->setViewMatrix(activeCamera->getViewMatrix());
        spriteRenderer->setProjectionMatrix(activeCamera->getProjectionMatrix());
        
        // Begin sprite rendering
        spriteRenderer->begin();
        
        // Draw background
        spriteRenderer->drawSprite(backgroundSprite);
        
        // Draw grid sprites
        for (const auto& sprite : gridSprites) {
            // Only draw sprites that are visible in the camera view
            Rect spriteBounds = sprite.getGlobalBounds();
            if (activeCamera->isRectVisible(spriteBounds)) {
                spriteRenderer->drawSprite(sprite);
            }
        }
        
        // Draw player
        spriteRenderer->drawSprite(playerSprite);
        
        // Draw camera bounds
        Rect cameraBounds = activeCamera->getBounds();
        spriteRenderer->drawRectangle(
            cameraBounds.x, cameraBounds.y,
            cameraBounds.width, cameraBounds.height,
            Color(1.0f, 1.0f, 1.0f, 0.2f),
            false
        );
        
        // End sprite rendering
        spriteRenderer->end();
        
        // End frame
        graphicsAPI->endFrame();
        
        // Update time
        time += deltaTime;
    }
    
    // Clean up
    cameraSystem->shutdown();
    componentManager->shutdown();
    entityManager.shutdown();
    spriteRenderer->shutdown();
    shaderManager->shutdown();
    graphicsAPI->shutdown();
    
    std::cout << "\nCamera System Test completed successfully!" << std::endl;
    return 0;
}