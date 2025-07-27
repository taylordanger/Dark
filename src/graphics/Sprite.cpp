#include "Sprite.h"
#include <cmath>

namespace RPGEngine {
namespace Graphics {

// Define predefined colors
const Color Color::White(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::Black(0.0f, 0.0f, 0.0f, 1.0f);
const Color Color::Red(1.0f, 0.0f, 0.0f, 1.0f);
const Color Color::Green(0.0f, 1.0f, 0.0f, 1.0f);
const Color Color::Blue(0.0f, 0.0f, 1.0f, 1.0f);
const Color Color::Yellow(1.0f, 1.0f, 0.0f, 1.0f);
const Color Color::Magenta(1.0f, 0.0f, 1.0f, 1.0f);
const Color Color::Cyan(0.0f, 1.0f, 1.0f, 1.0f);
const Color Color::Transparent(0.0f, 0.0f, 0.0f, 0.0f);

Sprite::Sprite()
    : m_texture(nullptr)
    , m_textureRect(0, 0, 0, 0)
    , m_color(Color::White)
    , m_x(0)
    , m_y(0)
    , m_rotation(0)
    , m_scaleX(1)
    , m_scaleY(1)
    , m_originX(0.5f)
    , m_originY(0.5f)
    , m_visible(true)
    , m_flipX(false)
    , m_flipY(false)
{
}

Sprite::Sprite(std::shared_ptr<Texture> texture)
    : m_texture(texture)
    , m_color(Color::White)
    , m_x(0)
    , m_y(0)
    , m_rotation(0)
    , m_scaleX(1)
    , m_scaleY(1)
    , m_originX(0.5f)
    , m_originY(0.5f)
    , m_visible(true)
    , m_flipX(false)
    , m_flipY(false)
{
    if (texture && texture->isValid()) {
        m_textureRect = Rect(0, 0, static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight()));
    } else {
        m_textureRect = Rect(0, 0, 0, 0);
    }
}

void Sprite::setTexture(std::shared_ptr<Texture> texture) {
    m_texture = texture;
    
    if (texture && texture->isValid()) {
        m_textureRect = Rect(0, 0, static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight()));
    } else {
        m_textureRect = Rect(0, 0, 0, 0);
    }
}

void Sprite::setTextureRect(const Rect& rect) {
    m_textureRect = rect;
}

void Sprite::setColor(const Color& color) {
    m_color = color;
}

void Sprite::setPosition(float x, float y) {
    m_x = x;
    m_y = y;
}

void Sprite::getPosition(float& x, float& y) const {
    x = m_x;
    y = m_y;
}

void Sprite::setRotation(float rotation) {
    m_rotation = rotation;
    
    // Normalize rotation to 0-360 degrees
    while (m_rotation >= 360.0f) {
        m_rotation -= 360.0f;
    }
    
    while (m_rotation < 0.0f) {
        m_rotation += 360.0f;
    }
}

void Sprite::setScale(float scaleX, float scaleY) {
    m_scaleX = scaleX;
    m_scaleY = scaleY;
}

void Sprite::setScale(float scale) {
    m_scaleX = scale;
    m_scaleY = scale;
}

void Sprite::getScale(float& scaleX, float& scaleY) const {
    scaleX = m_scaleX;
    scaleY = m_scaleY;
}

void Sprite::setOrigin(float x, float y) {
    m_originX = x;
    m_originY = y;
}

void Sprite::getOrigin(float& x, float& y) const {
    x = m_originX;
    y = m_originY;
}

void Sprite::setVisible(bool visible) {
    m_visible = visible;
}

void Sprite::setFlip(bool flipX, bool flipY) {
    m_flipX = flipX;
    m_flipY = flipY;
}

void Sprite::getFlip(bool& flipX, bool& flipY) const {
    flipX = m_flipX;
    flipY = m_flipY;
}

float Sprite::getWidth() const {
    return m_textureRect.width * std::abs(m_scaleX);
}

float Sprite::getHeight() const {
    return m_textureRect.height * std::abs(m_scaleY);
}

Rect Sprite::getLocalBounds() const {
    return Rect(0, 0, m_textureRect.width, m_textureRect.height);
}

Rect Sprite::getGlobalBounds() const {
    float width = getWidth();
    float height = getHeight();
    
    // Calculate origin offset
    float originX = width * m_originX;
    float originY = height * m_originY;
    
    // If no rotation, return simple bounds
    if (m_rotation == 0.0f) {
        return Rect(m_x - originX, m_y - originY, width, height);
    }
    
    // For rotated sprite, we need to calculate the bounding box
    // This is a simplified approximation
    float radians = m_rotation * 3.14159f / 180.0f;
    float cos = std::cos(radians);
    float sin = std::sin(radians);
    
    // Calculate the four corners of the rotated rectangle
    float x1 = -originX;
    float y1 = -originY;
    float x2 = width - originX;
    float y2 = height - originY;
    
    // Rotate the corners
    float rx1 = cos * x1 - sin * y1;
    float ry1 = sin * x1 + cos * y1;
    float rx2 = cos * x2 - sin * y1;
    float ry2 = sin * x2 + cos * y1;
    float rx3 = cos * x2 - sin * y2;
    float ry3 = sin * x2 + cos * y2;
    float rx4 = cos * x1 - sin * y2;
    float ry4 = sin * x1 + cos * y2;
    
    // Find the bounding box
    float minX = std::min(std::min(rx1, rx2), std::min(rx3, rx4));
    float maxX = std::max(std::max(rx1, rx2), std::max(rx3, rx4));
    float minY = std::min(std::min(ry1, ry2), std::min(ry3, ry4));
    float maxY = std::max(std::max(ry1, ry2), std::max(ry3, ry4));
    
    return Rect(m_x + minX, m_y + minY, maxX - minX, maxY - minY);
}

} // namespace Graphics
} // namespace RPGEngine