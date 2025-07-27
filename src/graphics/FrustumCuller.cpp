#include "FrustumCuller.h"
#include <algorithm>

namespace RPGEngine {
namespace Graphics {

FrustumCuller::FrustumCuller()
    : m_frustumBounds(0, 0, 0, 0)
    , m_frustumValid(false)
{
}

void FrustumCuller::updateFrustum(const Camera& camera) {
    m_frustumBounds = camera.getBounds();
    m_frustumValid = true;
}

bool FrustumCuller::isPointVisible(float x, float y) const {
    if (!m_frustumValid) {
        return true; // If frustum is not valid, assume everything is visible
    }
    
    return x >= m_frustumBounds.x && 
           x <= m_frustumBounds.x + m_frustumBounds.width &&
           y >= m_frustumBounds.y && 
           y <= m_frustumBounds.y + m_frustumBounds.height;
}

bool FrustumCuller::isRectVisible(const Rect& rect) const {
    if (!m_frustumValid) {
        return true; // If frustum is not valid, assume everything is visible
    }
    
    // Check for intersection using separating axis theorem
    return !(rect.x + rect.width < m_frustumBounds.x ||
             rect.x > m_frustumBounds.x + m_frustumBounds.width ||
             rect.y + rect.height < m_frustumBounds.y ||
             rect.y > m_frustumBounds.y + m_frustumBounds.height);
}

bool FrustumCuller::isSpriteVisible(const Sprite& sprite) const {
    if (!m_frustumValid) {
        return true; // If frustum is not valid, assume everything is visible
    }
    
    // Get sprite bounds
    float x, y;
    sprite.getPosition(x, y);
    
    float scaleX, scaleY;
    sprite.getScale(scaleX, scaleY);
    
    const Rect& textureRect = sprite.getTextureRect();
    
    // Calculate sprite bounds in world coordinates
    Rect spriteBounds(
        x - (textureRect.width * scaleX * 0.5f),
        y - (textureRect.height * scaleY * 0.5f),
        textureRect.width * scaleX,
        textureRect.height * scaleY
    );
    
    return isRectVisible(spriteBounds);
}

void FrustumCuller::cullSprites(const std::vector<Sprite>& sprites, std::vector<const Sprite*>& visibleSprites) const {
    visibleSprites.clear();
    visibleSprites.reserve(sprites.size()); // Reserve space to avoid reallocations
    
    for (const auto& sprite : sprites) {
        if (isSpriteVisible(sprite)) {
            visibleSprites.push_back(&sprite);
        }
    }
}

} // namespace Graphics
} // namespace RPGEngine