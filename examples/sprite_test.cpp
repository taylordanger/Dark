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

using namespace RPGEngine;
using namespace RPGEngine::Graphics;

// Simple animation class for testing
class Animation {
public:
    Animation(int frameCount, int frameWidth, int frameHeight, float frameTime)
        : m_frameCount(frameCount)
        , m_frameWidth(frameWidth)
        , m_frameHeight(frameHeight)
        , m_frameTime(frameTime)
        , m_currentFrame(0)
        , m_elapsed(0.0f)
    {
    }
    
    void update(float deltaTime) {
        m_elapsed += deltaTime;
        
        if (m_elapsed >= m_frameTime) {
            m_currentFrame = (m_currentFrame + 1) % m_frameCount;
            m_elapsed -= m_frameTime;
        }
    }
    
    void applyToSprite(Sprite& sprite) const {
        int row = m_currentFrame / 4;
        int col = m_currentFrame % 4;
        
        sprite.setTextureRect(Rect(
            col * m_frameWidth,
            row * m_frameHeight,
            m_frameWidth,
            m_frameHeight
        ));
    }
    
private:
    int m_frameCount;
    int m_frameWidth;
    int m_frameHeight;
    float m_frameTime;
    int m_currentFrame;
    float m_elapsed;
};

int main() {
    std::cout << "=== Sprite Renderer Test ===\n" << std::endl;
    
    // Create graphics API
    auto graphicsAPI = createGraphicsAPI();
    
    // Initialize graphics API
    if (!graphicsAPI->initialize(800, 600, "RPG Engine Sprite Test", false)) {
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
    
    // Create textures
    auto backgroundTexture = std::make_shared<Texture>(graphicsAPI);
    if (!backgroundTexture->loadFromFile("assets/background.png")) {
        std::cerr << "Failed to load background texture, creating a placeholder" << std::endl;
        
        // Create a checkerboard pattern as placeholder
        const int texSize = 256;
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
    
    auto spriteSheetTexture = std::make_shared<Texture>(graphicsAPI);
    if (!spriteSheetTexture->loadFromFile("assets/character.png")) {
        std::cerr << "Failed to load character texture, creating a placeholder" << std::endl;
        
        // Create a simple character sprite sheet as placeholder
        const int texSize = 128;
        uint8_t textureData[texSize * texSize * 4];
        
        for (int y = 0; y < texSize; ++y) {
            for (int x = 0; x < texSize; ++x) {
                int frameX = x / 32;
                int frameY = y / 32;
                int frame = frameY * 4 + frameX;
                
                int centerX = (x % 32) - 16;
                int centerY = (y % 32) - 16;
                float distance = std::sqrt(centerX * centerX + centerY * centerY);
                
                uint8_t r = 255;
                uint8_t g = 100 + (frame * 20) % 155;
                uint8_t b = 100;
                uint8_t a = (distance < 14) ? 255 : 0;
                
                int index = (y * texSize + x) * 4;
                textureData[index + 0] = r;
                textureData[index + 1] = g;
                textureData[index + 2] = b;
                textureData[index + 3] = a;
            }
        }
        
        spriteSheetTexture->createFromData(texSize, texSize, TextureFormat::RGBA, textureData);
    }
    
    // Create sprites
    Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setPosition(400, 300);
    backgroundSprite.setScale(2.0f);
    
    Sprite characterSprite(spriteSheetTexture);
    characterSprite.setTextureRect(Rect(0, 0, 32, 32));
    characterSprite.setPosition(400, 300);
    characterSprite.setScale(2.0f);
    
    // Create animation
    Animation walkAnimation(8, 32, 32, 0.1f);
    
    // Create some additional sprites for testing
    std::vector<Sprite> testSprites;
    for (int i = 0; i < 10; ++i) {
        Sprite sprite(spriteSheetTexture);
        sprite.setTextureRect(Rect(0, 0, 32, 32));
        sprite.setPosition(100 + i * 60, 100);
        sprite.setRotation(i * 36.0f);
        sprite.setColor(Color(1.0f, 0.5f + i * 0.05f, 0.5f));
        testSprites.push_back(sprite);
    }
    
    // Main loop
    float time = 0.0f;
    float deltaTime = 0.016f;
    
    while (!graphicsAPI->shouldClose()) {
        // Begin frame
        graphicsAPI->beginFrame();
        graphicsAPI->clear(0.2f, 0.3f, 0.3f, 1.0f);
        
        // Update animation
        walkAnimation.update(deltaTime);
        walkAnimation.applyToSprite(characterSprite);
        
        // Update character position
        float x = 400 + std::cos(time) * 200;
        float y = 300 + std::sin(time * 0.5f) * 100;
        characterSprite.setPosition(x, y);
        
        // Update test sprites
        for (size_t i = 0; i < testSprites.size(); ++i) {
            testSprites[i].setRotation(testSprites[i].getRotation() + deltaTime * 50.0f);
            testSprites[i].setPosition(
                100 + i * 60,
                100 + std::sin(time + i * 0.5f) * 50
            );
        }
        
        // Begin sprite rendering
        spriteRenderer->begin();
        
        // Draw background
        spriteRenderer->drawSprite(backgroundSprite);
        
        // Draw test sprites
        for (const auto& sprite : testSprites) {
            spriteRenderer->drawSprite(sprite);
        }
        
        // Draw character
        spriteRenderer->drawSprite(characterSprite);
        
        // Draw some shapes
        spriteRenderer->drawRectangle(50, 50, 100, 100, Color(1.0f, 0.0f, 0.0f, 0.5f), true);
        spriteRenderer->drawRectangle(200, 50, 100, 100, Color(0.0f, 1.0f, 0.0f, 0.5f), false);
        
        // End sprite rendering
        spriteRenderer->end();
        
        // End frame
        graphicsAPI->endFrame();
        
        // Update time
        time += deltaTime;
    }
    
    // Clean up
    spriteRenderer->shutdown();
    shaderManager->shutdown();
    graphicsAPI->shutdown();
    
    std::cout << "\nSprite Renderer Test completed successfully!" << std::endl;
    return 0;
}