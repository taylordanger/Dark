#pragma once

#include "Texture.h"
#include <memory>
#include <string>

namespace RPGEngine {
namespace Graphics {

/**
 * Rectangle structure
 */
struct Rect {
    float x, y, width, height;
    
    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}
};

/**
 * Color structure (RGBA)
 */
struct Color {
    float r, g, b, a;
    
    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    
    // Predefined colors
    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    static const Color Transparent;
};

/**
 * Sprite class
 * Represents a 2D image that can be rendered
 */
class Sprite {
public:
    /**
     * Constructor
     */
    Sprite();
    
    /**
     * Constructor with texture
     * @param texture Texture to use
     */
    explicit Sprite(std::shared_ptr<Texture> texture);
    
    /**
     * Set the texture
     * @param texture Texture to use
     */
    void setTexture(std::shared_ptr<Texture> texture);
    
    /**
     * Get the texture
     * @return Texture
     */
    std::shared_ptr<Texture> getTexture() const { return m_texture; }
    
    /**
     * Set the texture rectangle (source rectangle in the texture)
     * @param rect Texture rectangle
     */
    void setTextureRect(const Rect& rect);
    
    /**
     * Get the texture rectangle
     * @return Texture rectangle
     */
    const Rect& getTextureRect() const { return m_textureRect; }
    
    /**
     * Set the color
     * @param color Color
     */
    void setColor(const Color& color);
    
    /**
     * Get the color
     * @return Color
     */
    const Color& getColor() const { return m_color; }
    
    /**
     * Set the position
     * @param x X position
     * @param y Y position
     */
    void setPosition(float x, float y);
    
    /**
     * Get the position
     * @param x Output X position
     * @param y Output Y position
     */
    void getPosition(float& x, float& y) const;
    
    /**
     * Set the rotation (in degrees)
     * @param rotation Rotation angle
     */
    void setRotation(float rotation);
    
    /**
     * Get the rotation
     * @return Rotation angle
     */
    float getRotation() const { return m_rotation; }
    
    /**
     * Set the scale
     * @param scaleX X scale
     * @param scaleY Y scale
     */
    void setScale(float scaleX, float scaleY);
    
    /**
     * Set the scale (uniform)
     * @param scale Scale factor
     */
    void setScale(float scale);
    
    /**
     * Get the scale
     * @param scaleX Output X scale
     * @param scaleY Output Y scale
     */
    void getScale(float& scaleX, float& scaleY) const;
    
    /**
     * Set the origin (pivot point)
     * @param x X origin (0-1)
     * @param y Y origin (0-1)
     */
    void setOrigin(float x, float y);
    
    /**
     * Get the origin
     * @param x Output X origin
     * @param y Output Y origin
     */
    void getOrigin(float& x, float& y) const;
    
    /**
     * Set the visibility
     * @param visible Visibility flag
     */
    void setVisible(bool visible);
    
    /**
     * Check if sprite is visible
     * @return true if sprite is visible
     */
    bool isVisible() const { return m_visible; }
    
    /**
     * Set the flip state
     * @param flipX Horizontal flip
     * @param flipY Vertical flip
     */
    void setFlip(bool flipX, bool flipY);
    
    /**
     * Get the flip state
     * @param flipX Output horizontal flip
     * @param flipY Output vertical flip
     */
    void getFlip(bool& flipX, bool& flipY) const;
    
    /**
     * Get the width
     * @return Width
     */
    float getWidth() const;
    
    /**
     * Get the height
     * @return Height
     */
    float getHeight() const;
    
    /**
     * Get the global bounds (in world space)
     * @return Global bounds
     */
    Rect getGlobalBounds() const;
    
    /**
     * Get the local bounds (in local space)
     * @return Local bounds
     */
    Rect getLocalBounds() const;
    
private:
    std::shared_ptr<Texture> m_texture;
    Rect m_textureRect;
    Color m_color;
    float m_x;
    float m_y;
    float m_rotation;
    float m_scaleX;
    float m_scaleY;
    float m_originX;
    float m_originY;
    bool m_visible;
    bool m_flipX;
    bool m_flipY;
};

} // namespace Graphics
} // namespace RPGEngine