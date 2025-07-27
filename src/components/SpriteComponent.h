#pragma once

#include "Component.h"
#include <string>

namespace RPGEngine {
namespace Components {

/**
 * Sprite Component
 * Stores visual representation data for an entity
 */
class SpriteComponent : public Component<SpriteComponent> {
public:
    /**
     * Constructor
     * @param entityId Entity ID
     * @param texturePath Path to texture file
     * @param width Sprite width
     * @param height Sprite height
     * @param layer Rendering layer (higher = in front)
     * @param visible Visibility flag
     */
    SpriteComponent(EntityId entityId, const std::string& texturePath = "", 
                   int width = 0, int height = 0, int layer = 0, bool visible = true)
        : Component<SpriteComponent>(entityId)
        , m_texturePath(texturePath)
        , m_width(width)
        , m_height(height)
        , m_layer(layer)
        , m_visible(visible)
        , m_flipX(false)
        , m_flipY(false)
        , m_color(0xFFFFFFFF) // White, fully opaque
    {
    }
    
    // Texture
    const std::string& getTexturePath() const { return m_texturePath; }
    void setTexturePath(const std::string& texturePath) { m_texturePath = texturePath; }
    
    // Dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    void setWidth(int width) { m_width = width; }
    void setHeight(int height) { m_height = height; }
    void setDimensions(int width, int height) { m_width = width; m_height = height; }
    
    // Layer
    int getLayer() const { return m_layer; }
    void setLayer(int layer) { m_layer = layer; }
    
    // Visibility
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    
    // Flipping
    bool isFlippedX() const { return m_flipX; }
    bool isFlippedY() const { return m_flipY; }
    void setFlipX(bool flip) { m_flipX = flip; }
    void setFlipY(bool flip) { m_flipY = flip; }
    
    // Color tint (RGBA format)
    uint32_t getColor() const { return m_color; }
    void setColor(uint32_t color) { m_color = color; }
    
    // Color components
    uint8_t getRed() const { return (m_color >> 24) & 0xFF; }
    uint8_t getGreen() const { return (m_color >> 16) & 0xFF; }
    uint8_t getBlue() const { return (m_color >> 8) & 0xFF; }
    uint8_t getAlpha() const { return m_color & 0xFF; }
    
    void setRed(uint8_t red) { m_color = (m_color & 0x00FFFFFF) | (red << 24); }
    void setGreen(uint8_t green) { m_color = (m_color & 0xFF00FFFF) | (green << 16); }
    void setBlue(uint8_t blue) { m_color = (m_color & 0xFFFF00FF) | (blue << 8); }
    void setAlpha(uint8_t alpha) { m_color = (m_color & 0xFFFFFF00) | alpha; }
    
    void setRGBA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
        m_color = (red << 24) | (green << 16) | (blue << 8) | alpha;
    }
    
private:
    std::string m_texturePath; // Path to texture file
    int m_width;               // Sprite width
    int m_height;              // Sprite height
    int m_layer;               // Rendering layer
    bool m_visible;            // Visibility flag
    bool m_flipX;              // Horizontal flip
    bool m_flipY;              // Vertical flip
    uint32_t m_color;          // Color tint (RGBA)
};

} // namespace Components
} // namespace RPGEngine