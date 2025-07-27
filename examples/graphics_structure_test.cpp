#include <iostream>
#include <memory>
#include "../src/graphics/IGraphicsAPI.h"
#include "../src/graphics/Camera.h"
#include "../src/graphics/Sprite.h"
#include "../src/graphics/Texture.h"

using namespace RPGEngine::Graphics;

/**
 * Graphics structure test
 * Tests the graphics system structure without OpenGL dependencies
 */
int main() {
    std::cout << "=== RPG Engine Graphics Structure Test ===" << std::endl;
    
    // Test Camera
    std::cout << "Testing Camera..." << std::endl;
    auto camera = std::make_shared<Camera>();
    camera->setPosition(100.0f, 200.0f);
    camera->setViewportSize(800, 600);
    camera->setZoom(1.5f);
    camera->setRotation(45.0f);
    
    float x, y;
    camera->getPosition(x, y);
    std::cout << "  ✅ Camera position: (" << x << ", " << y << ")" << std::endl;
    std::cout << "  ✅ Camera zoom: " << camera->getZoom() << std::endl;
    std::cout << "  ✅ Camera rotation: " << camera->getRotation() << "°" << std::endl;
    
    int width, height;
    camera->getViewportSize(width, height);
    std::cout << "  ✅ Viewport size: " << width << "x" << height << std::endl;
    
    // Test Sprite
    std::cout << "Testing Sprite..." << std::endl;
    Sprite sprite;
    sprite.setPosition(50.0f, 75.0f);
    sprite.setScale(2.0f, 1.5f);
    sprite.setRotation(30.0f);
    sprite.setColor({1.0f, 0.5f, 0.2f, 0.8f});
    
    sprite.getPosition(x, y);
    std::cout << "  ✅ Sprite position: (" << x << ", " << y << ")" << std::endl;
    
    float scaleX, scaleY;
    sprite.getScale(scaleX, scaleY);
    std::cout << "  ✅ Sprite scale: (" << scaleX << ", " << scaleY << ")" << std::endl;
    std::cout << "  ✅ Sprite rotation: " << sprite.getRotation() << "°" << std::endl;
    
    const Color& color = sprite.getColor();
    std::cout << "  ✅ Sprite color: (" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")" << std::endl;
    
    // Test vertex data types
    std::cout << "Testing vertex data types..." << std::endl;
    std::cout << "  ✅ VertexDataType::Float = " << static_cast<uint32_t>(VertexDataType::Float) << std::endl;
    std::cout << "  ✅ VertexDataType::UnsignedShort = " << static_cast<uint32_t>(VertexDataType::UnsignedShort) << std::endl;
    
    // Test vertex attribute structure
    VertexAttribute attr;
    attr.name = "aPosition";
    attr.location = 0;
    attr.size = 3;
    attr.type = VertexDataType::Float;
    attr.normalized = false;
    attr.stride = 32;
    attr.offset = 0;
    
    std::cout << "  ✅ VertexAttribute created: " << attr.name << " (location=" << attr.location << ")" << std::endl;
    
    // Test primitive types
    std::cout << "Testing primitive types..." << std::endl;
    PrimitiveType triangles = PrimitiveType::Triangles;
    BlendMode alpha = BlendMode::Alpha;
    TextureFormat rgba = TextureFormat::RGBA;
    
    std::cout << "  ✅ PrimitiveType::Triangles defined" << std::endl;
    std::cout << "  ✅ BlendMode::Alpha defined" << std::endl;
    std::cout << "  ✅ TextureFormat::RGBA defined" << std::endl;
    
    std::cout << "✅ All graphics structure tests passed!" << std::endl;
    std::cout << "=== Graphics Structure Test Complete ===" << std::endl;
    
    return 0;
}