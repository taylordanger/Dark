#pragma once

#include "Sprite.h"
#include "../entities/Entity.h"
#include <memory>

namespace RPGEngine {
namespace Graphics {

/**
 * Camera class
 * Handles view transformations and viewport management
 */
class Camera {
public:
    /**
     * Constructor
     */
    Camera();
    
    /**
     * Set the camera position
     * @param x X position
     * @param y Y position
     */
    void setPosition(float x, float y);
    
    /**
     * Get the camera position
     * @param x Output X position
     * @param y Output Y position
     */
    void getPosition(float& x, float& y) const;
    
    /**
     * Set the camera rotation (in degrees)
     * @param rotation Rotation angle
     */
    void setRotation(float rotation);
    
    /**
     * Get the camera rotation
     * @return Rotation angle
     */
    float getRotation() const { return m_rotation; }
    
    /**
     * Set the camera zoom
     * @param zoom Zoom factor (1.0 = no zoom)
     */
    void setZoom(float zoom);
    
    /**
     * Get the camera zoom
     * @return Zoom factor
     */
    float getZoom() const { return m_zoom; }
    
    /**
     * Set the viewport size
     * @param width Viewport width
     * @param height Viewport height
     */
    void setViewportSize(int width, int height);
    
    /**
     * Get the viewport size
     * @param width Output viewport width
     * @param height Output viewport height
     */
    void getViewportSize(int& width, int& height) const;
    
    /**
     * Set the camera to follow an entity
     * @param entity Entity to follow
     * @param offsetX X offset from entity position
     * @param offsetY Y offset from entity position
     */
    void followEntity(Entity entity, float offsetX = 0.0f, float offsetY = 0.0f);
    
    /**
     * Stop following an entity
     */
    void stopFollowing();
    
    /**
     * Check if the camera is following an entity
     * @return true if the camera is following an entity
     */
    bool isFollowingEntity() const { return m_followEntity.isValid(); }
    
    /**
     * Get the entity being followed
     * @return Entity being followed
     */
    Entity getFollowedEntity() const { return m_followEntity; }
    
    /**
     * Update the camera
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);
    
    /**
     * Get the view matrix
     * @return View matrix (column-major)
     */
    const float* getViewMatrix() const { return m_viewMatrix; }
    
    /**
     * Get the projection matrix
     * @return Projection matrix (column-major)
     */
    const float* getProjectionMatrix() const { return m_projectionMatrix; }
    
    /**
     * Convert screen coordinates to world coordinates
     * @param screenX Screen X coordinate
     * @param screenY Screen Y coordinate
     * @param worldX Output world X coordinate
     * @param worldY Output world Y coordinate
     */
    void screenToWorld(float screenX, float screenY, float& worldX, float& worldY) const;
    
    /**
     * Convert world coordinates to screen coordinates
     * @param worldX World X coordinate
     * @param worldY World Y coordinate
     * @param screenX Output screen X coordinate
     * @param screenY Output screen Y coordinate
     */
    void worldToScreen(float worldX, float worldY, float& screenX, float& screenY) const;
    
    /**
     * Check if a point is visible in the camera view
     * @param x World X coordinate
     * @param y World Y coordinate
     * @return true if the point is visible
     */
    bool isPointVisible(float x, float y) const;
    
    /**
     * Check if a rectangle is visible in the camera view
     * @param rect Rectangle in world coordinates
     * @return true if the rectangle is visible
     */
    bool isRectVisible(const Rect& rect) const;
    
    /**
     * Get the camera bounds in world coordinates
     * @return Camera bounds
     */
    Rect getBounds() const;
    
    /**
     * Set camera bounds (limits for camera movement)
     * @param left Left bound
     * @param right Right bound
     * @param top Top bound
     * @param bottom Bottom bound
     */
    void setBounds(float left, float right, float top, float bottom);
    
    /**
     * Clear camera bounds (no limits for camera movement)
     */
    void clearBounds();
    
    /**
     * Check if camera has bounds set
     * @return true if camera has bounds
     */
    bool hasBounds() const { return m_hasBounds; }
    
    /**
     * Shake the camera
     * @param duration Shake duration in seconds
     * @param intensity Shake intensity
     */
    void shake(float duration, float intensity);
    
    /**
     * Move the camera smoothly to a position
     * @param x Target X position
     * @param y Target Y position
     * @param duration Movement duration in seconds
     */
    void moveTo(float x, float y, float duration);
    
    /**
     * Zoom the camera smoothly
     * @param zoom Target zoom factor
     * @param duration Zoom duration in seconds
     */
    void zoomTo(float zoom, float duration);
    
    /**
     * Rotate the camera smoothly
     * @param rotation Target rotation angle
     * @param duration Rotation duration in seconds
     */
    void rotateTo(float rotation, float duration);
    
private:
    /**
     * Update the view matrix
     */
    void updateViewMatrix();
    
    /**
     * Update the projection matrix
     */
    void updateProjectionMatrix();
    
    /**
     * Apply camera bounds to position
     */
    void applyBounds();
    
    // Camera properties
    float m_x;
    float m_y;
    float m_rotation;
    float m_zoom;
    
    // Viewport properties
    int m_viewportWidth;
    int m_viewportHeight;
    
    // Matrices
    float m_viewMatrix[16];
    float m_projectionMatrix[16];
    
    // Entity following
    Entity m_followEntity;
    float m_followOffsetX;
    float m_followOffsetY;
    
    // Camera bounds
    bool m_hasBounds;
    float m_boundLeft;
    float m_boundRight;
    float m_boundTop;
    float m_boundBottom;
    
    // Camera effects
    bool m_isShaking;
    float m_shakeTimer;
    float m_shakeDuration;
    float m_shakeIntensity;
    
    // Smooth movement
    bool m_isMoving;
    float m_moveTimer;
    float m_moveDuration;
    float m_moveStartX;
    float m_moveStartY;
    float m_moveTargetX;
    float m_moveTargetY;
    
    // Smooth zoom
    bool m_isZooming;
    float m_zoomTimer;
    float m_zoomDuration;
    float m_zoomStart;
    float m_zoomTarget;
    
    // Smooth rotation
    bool m_isRotating;
    float m_rotateTimer;
    float m_rotateDuration;
    float m_rotateStart;
    float m_rotateTarget;
};

} // namespace Graphics
} // namespace RPGEngine