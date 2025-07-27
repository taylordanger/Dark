#pragma once

#include "Sprite.h"
#include "Camera.h"
#include <vector>

namespace RPGEngine {
namespace Graphics {

/**
 * Frustum culling utility for efficient rendering
 * Only renders objects that are visible in the camera view
 */
class FrustumCuller {
public:
    /**
     * Constructor
     */
    FrustumCuller();
    
    /**
     * Update the frustum based on camera properties
     * @param camera Camera to use for frustum calculation
     */
    void updateFrustum(const Camera& camera);
    
    /**
     * Check if a point is inside the frustum
     * @param x World X coordinate
     * @param y World Y coordinate
     * @return true if the point is visible
     */
    bool isPointVisible(float x, float y) const;
    
    /**
     * Check if a rectangle is inside or intersects the frustum
     * @param rect Rectangle in world coordinates
     * @return true if the rectangle is visible
     */
    bool isRectVisible(const Rect& rect) const;
    
    /**
     * Check if a sprite is visible
     * @param sprite Sprite to check
     * @return true if the sprite is visible
     */
    bool isSpriteVisible(const Sprite& sprite) const;
    
    /**
     * Cull a list of sprites, returning only visible ones
     * @param sprites Input list of sprites
     * @param visibleSprites Output list of visible sprites
     */
    void cullSprites(const std::vector<Sprite>& sprites, std::vector<const Sprite*>& visibleSprites) const;
    
    /**
     * Get the frustum bounds
     * @return Frustum bounds in world coordinates
     */
    const Rect& getFrustumBounds() const { return m_frustumBounds; }
    
private:
    Rect m_frustumBounds;
    bool m_frustumValid;
};

} // namespace Graphics
} // namespace RPGEngine