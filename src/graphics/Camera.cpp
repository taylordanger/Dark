#include "Camera.h"
#include <cmath>
#include <algorithm>

namespace RPGEngine {
namespace Graphics {

Camera::Camera()
    : m_x(0.0f)
    , m_y(0.0f)
    , m_rotation(0.0f)
    , m_zoom(1.0f)
    , m_viewportWidth(800)
    , m_viewportHeight(600)
    , m_followOffsetX(0.0f)
    , m_followOffsetY(0.0f)
    , m_hasBounds(false)
    , m_boundLeft(0.0f)
    , m_boundRight(0.0f)
    , m_boundTop(0.0f)
    , m_boundBottom(0.0f)
    , m_isShaking(false)
    , m_shakeTimer(0.0f)
    , m_shakeDuration(0.0f)
    , m_shakeIntensity(0.0f)
    , m_isMoving(false)
    , m_moveTimer(0.0f)
    , m_moveDuration(0.0f)
    , m_moveStartX(0.0f)
    , m_moveStartY(0.0f)
    , m_moveTargetX(0.0f)
    , m_moveTargetY(0.0f)
    , m_isZooming(false)
    , m_zoomTimer(0.0f)
    , m_zoomDuration(0.0f)
    , m_zoomStart(1.0f)
    , m_zoomTarget(1.0f)
    , m_isRotating(false)
    , m_rotateTimer(0.0f)
    , m_rotateDuration(0.0f)
    , m_rotateStart(0.0f)
    , m_rotateTarget(0.0f)
{
    // Initialize matrices to identity
    for (int i = 0; i < 16; ++i) {
        m_viewMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        m_projectionMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    
    updateViewMatrix();
    updateProjectionMatrix();
}

void Camera::setPosition(float x, float y) {
    m_x = x;
    m_y = y;
    
    if (m_hasBounds) {
        applyBounds();
    }
    
    updateViewMatrix();
}

void Camera::getPosition(float& x, float& y) const {
    x = m_x;
    y = m_y;
}

void Camera::setRotation(float rotation) {
    m_rotation = rotation;
    
    // Normalize rotation to 0-360 degrees
    while (m_rotation >= 360.0f) {
        m_rotation -= 360.0f;
    }
    
    while (m_rotation < 0.0f) {
        m_rotation += 360.0f;
    }
    
    updateViewMatrix();
}

void Camera::setZoom(float zoom) {
    // Clamp zoom to reasonable values
    m_zoom = std::max(0.1f, zoom);
    updateViewMatrix();
}

void Camera::setViewportSize(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
    updateProjectionMatrix();
}

void Camera::getViewportSize(int& width, int& height) const {
    width = m_viewportWidth;
    height = m_viewportHeight;
}

void Camera::followEntity(Entity entity, float offsetX, float offsetY) {
    m_followEntity = entity;
    m_followOffsetX = offsetX;
    m_followOffsetY = offsetY;
}

void Camera::stopFollowing() {
    m_followEntity = Entity();
}

void Camera::update(float deltaTime) {
    // Update camera effects
    
    // Shake effect
    if (m_isShaking) {
        m_shakeTimer += deltaTime;
        
        if (m_shakeTimer >= m_shakeDuration) {
            m_isShaking = false;
        } else {
            // Apply random offset based on intensity and remaining time
            float remainingFactor = 1.0f - (m_shakeTimer / m_shakeDuration);
            float intensity = m_shakeIntensity * remainingFactor;
            
            // Generate random offset
            float offsetX = (std::rand() / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * intensity;
            float offsetY = (std::rand() / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * intensity;
            
            // Apply offset
            m_x += offsetX;
            m_y += offsetY;
        }
    }
    
    // Smooth movement
    if (m_isMoving) {
        m_moveTimer += deltaTime;
        
        if (m_moveTimer >= m_moveDuration) {
            m_isMoving = false;
            m_x = m_moveTargetX;
            m_y = m_moveTargetY;
        } else {
            // Calculate interpolation factor
            float t = m_moveTimer / m_moveDuration;
            
            // Smooth interpolation (ease in-out)
            t = t * t * (3.0f - 2.0f * t);
            
            // Interpolate position
            m_x = m_moveStartX + (m_moveTargetX - m_moveStartX) * t;
            m_y = m_moveStartY + (m_moveTargetY - m_moveStartY) * t;
        }
    }
    
    // Smooth zoom
    if (m_isZooming) {
        m_zoomTimer += deltaTime;
        
        if (m_zoomTimer >= m_zoomDuration) {
            m_isZooming = false;
            m_zoom = m_zoomTarget;
        } else {
            // Calculate interpolation factor
            float t = m_zoomTimer / m_zoomDuration;
            
            // Smooth interpolation (ease in-out)
            t = t * t * (3.0f - 2.0f * t);
            
            // Interpolate zoom
            m_zoom = m_zoomStart + (m_zoomTarget - m_zoomStart) * t;
        }
    }
    
    // Smooth rotation
    if (m_isRotating) {
        m_rotateTimer += deltaTime;
        
        if (m_rotateTimer >= m_rotateDuration) {
            m_isRotating = false;
            m_rotation = m_rotateTarget;
        } else {
            // Calculate interpolation factor
            float t = m_rotateTimer / m_rotateDuration;
            
            // Smooth interpolation (ease in-out)
            t = t * t * (3.0f - 2.0f * t);
            
            // Interpolate rotation
            m_rotation = m_rotateStart + (m_rotateTarget - m_rotateStart) * t;
        }
    }
    
    // Follow entity if set
    if (m_followEntity.isValid()) {
        // TODO: Get entity position from component manager
        // For now, we'll just use the entity ID as position for testing
        float entityX = static_cast<float>(m_followEntity.getID()) * 10.0f;
        float entityY = static_cast<float>(m_followEntity.getID()) * 5.0f;
        
        // Set camera position with offset
        setPosition(entityX + m_followOffsetX, entityY + m_followOffsetY);
    }
    
    // Apply bounds
    if (m_hasBounds) {
        applyBounds();
    }
    
    // Update view matrix
    updateViewMatrix();
}

void Camera::screenToWorld(float screenX, float screenY, float& worldX, float& worldY) const {
    // Convert screen coordinates to normalized device coordinates (-1 to 1)
    float ndcX = (screenX / m_viewportWidth) * 2.0f - 1.0f;
    float ndcY = 1.0f - (screenY / m_viewportHeight) * 2.0f; // Y is inverted
    
    // Calculate inverse matrices
    float invProjection[16];
    float invView[16];
    
    // Inverse projection matrix (simple for orthographic)
    invProjection[0] = 1.0f / m_projectionMatrix[0];
    invProjection[5] = 1.0f / m_projectionMatrix[5];
    invProjection[10] = 1.0f / m_projectionMatrix[10];
    invProjection[12] = -m_projectionMatrix[12] * invProjection[0];
    invProjection[13] = -m_projectionMatrix[13] * invProjection[5];
    invProjection[14] = -m_projectionMatrix[14] * invProjection[10];
    invProjection[15] = 1.0f;
    
    // Inverse view matrix
    float s = std::sin(-m_rotation * 3.14159f / 180.0f);
    float c = std::cos(-m_rotation * 3.14159f / 180.0f);
    float invZoom = 1.0f / m_zoom;
    
    invView[0] = c * invZoom;
    invView[1] = s * invZoom;
    invView[4] = -s * invZoom;
    invView[5] = c * invZoom;
    invView[10] = 1.0f;
    invView[12] = m_x;
    invView[13] = m_y;
    invView[15] = 1.0f;
    
    // Apply inverse projection
    float clipX = ndcX;
    float clipY = ndcY;
    float clipZ = -1.0f;
    float clipW = 1.0f;
    
    float eyeX = clipX * invProjection[0] + clipW * invProjection[12];
    float eyeY = clipY * invProjection[5] + clipW * invProjection[13];
    float eyeZ = clipZ * invProjection[10] + clipW * invProjection[14];
    float eyeW = clipW;
    
    // Apply inverse view
    worldX = eyeX * invView[0] + eyeY * invView[4] + eyeZ * invView[8] + eyeW * invView[12];
    worldY = eyeX * invView[1] + eyeY * invView[5] + eyeZ * invView[9] + eyeW * invView[13];
}

void Camera::worldToScreen(float worldX, float worldY, float& screenX, float& screenY) const {
    // Apply view matrix
    float s = std::sin(m_rotation * 3.14159f / 180.0f);
    float c = std::cos(m_rotation * 3.14159f / 180.0f);
    
    float viewX = (worldX - m_x) * c - (worldY - m_y) * s;
    float viewY = (worldX - m_x) * s + (worldY - m_y) * c;
    
    // Apply zoom
    viewX *= m_zoom;
    viewY *= m_zoom;
    
    // Apply projection matrix (orthographic)
    float ndcX = viewX * m_projectionMatrix[0] + m_projectionMatrix[12];
    float ndcY = viewY * m_projectionMatrix[5] + m_projectionMatrix[13];
    
    // Convert to screen coordinates
    screenX = (ndcX + 1.0f) * 0.5f * m_viewportWidth;
    screenY = (1.0f - ndcY) * 0.5f * m_viewportHeight; // Y is inverted
}

bool Camera::isPointVisible(float x, float y) const {
    float screenX, screenY;
    worldToScreen(x, y, screenX, screenY);
    
    return screenX >= 0 && screenX <= m_viewportWidth &&
           screenY >= 0 && screenY <= m_viewportHeight;
}

bool Camera::isRectVisible(const Rect& rect) const {
    // Get camera bounds
    Rect cameraBounds = getBounds();
    
    // Check for intersection
    return !(rect.x + rect.width < cameraBounds.x ||
             rect.x > cameraBounds.x + cameraBounds.width ||
             rect.y + rect.height < cameraBounds.y ||
             rect.y > cameraBounds.y + cameraBounds.height);
}

Rect Camera::getBounds() const {
    // Calculate camera bounds in world coordinates
    float halfWidth = m_viewportWidth * 0.5f / m_zoom;
    float halfHeight = m_viewportHeight * 0.5f / m_zoom;
    
    return Rect(m_x - halfWidth, m_y - halfHeight, halfWidth * 2.0f, halfHeight * 2.0f);
}

void Camera::setBounds(float left, float right, float top, float bottom) {
    m_hasBounds = true;
    m_boundLeft = left;
    m_boundRight = right;
    m_boundTop = top;
    m_boundBottom = bottom;
    
    // Apply bounds immediately
    applyBounds();
    updateViewMatrix();
}

void Camera::clearBounds() {
    m_hasBounds = false;
}

void Camera::shake(float duration, float intensity) {
    m_isShaking = true;
    m_shakeTimer = 0.0f;
    m_shakeDuration = duration;
    m_shakeIntensity = intensity;
}

void Camera::moveTo(float x, float y, float duration) {
    if (duration <= 0.0f) {
        // Instant movement
        setPosition(x, y);
        return;
    }
    
    m_isMoving = true;
    m_moveTimer = 0.0f;
    m_moveDuration = duration;
    m_moveStartX = m_x;
    m_moveStartY = m_y;
    m_moveTargetX = x;
    m_moveTargetY = y;
}

void Camera::zoomTo(float zoom, float duration) {
    if (duration <= 0.0f) {
        // Instant zoom
        setZoom(zoom);
        return;
    }
    
    m_isZooming = true;
    m_zoomTimer = 0.0f;
    m_zoomDuration = duration;
    m_zoomStart = m_zoom;
    m_zoomTarget = std::max(0.1f, zoom);
}

void Camera::rotateTo(float rotation, float duration) {
    if (duration <= 0.0f) {
        // Instant rotation
        setRotation(rotation);
        return;
    }
    
    m_isRotating = true;
    m_rotateTimer = 0.0f;
    m_rotateDuration = duration;
    m_rotateStart = m_rotation;
    m_rotateTarget = rotation;
    
    // Normalize target rotation
    while (m_rotateTarget >= 360.0f) {
        m_rotateTarget -= 360.0f;
    }
    
    while (m_rotateTarget < 0.0f) {
        m_rotateTarget += 360.0f;
    }
    
    // Choose shortest rotation path
    float diff = m_rotateTarget - m_rotateStart;
    if (diff > 180.0f) {
        m_rotateStart += 360.0f;
    } else if (diff < -180.0f) {
        m_rotateTarget += 360.0f;
    }
}

void Camera::updateViewMatrix() {
    // Create view matrix (inverse of camera transform)
    float s = std::sin(m_rotation * 3.14159f / 180.0f);
    float c = std::cos(m_rotation * 3.14159f / 180.0f);
    
    // Scale (zoom)
    m_viewMatrix[0] = c * m_zoom;
    m_viewMatrix[1] = s * m_zoom;
    m_viewMatrix[4] = -s * m_zoom;
    m_viewMatrix[5] = c * m_zoom;
    
    // Translation
    m_viewMatrix[12] = -m_x * c * m_zoom - m_y * -s * m_zoom;
    m_viewMatrix[13] = -m_x * s * m_zoom - m_y * c * m_zoom;
    
    // Identity for other elements
    m_viewMatrix[2] = 0.0f;
    m_viewMatrix[3] = 0.0f;
    m_viewMatrix[6] = 0.0f;
    m_viewMatrix[7] = 0.0f;
    m_viewMatrix[8] = 0.0f;
    m_viewMatrix[9] = 0.0f;
    m_viewMatrix[10] = 1.0f;
    m_viewMatrix[11] = 0.0f;
    m_viewMatrix[14] = 0.0f;
    m_viewMatrix[15] = 1.0f;
}

void Camera::updateProjectionMatrix() {
    // Create orthographic projection matrix
    float left = 0.0f;
    float right = static_cast<float>(m_viewportWidth);
    float bottom = static_cast<float>(m_viewportHeight);
    float top = 0.0f;
    float near = -1.0f;
    float far = 1.0f;
    
    m_projectionMatrix[0] = 2.0f / (right - left);
    m_projectionMatrix[1] = 0.0f;
    m_projectionMatrix[2] = 0.0f;
    m_projectionMatrix[3] = 0.0f;
    
    m_projectionMatrix[4] = 0.0f;
    m_projectionMatrix[5] = 2.0f / (top - bottom);
    m_projectionMatrix[6] = 0.0f;
    m_projectionMatrix[7] = 0.0f;
    
    m_projectionMatrix[8] = 0.0f;
    m_projectionMatrix[9] = 0.0f;
    m_projectionMatrix[10] = -2.0f / (far - near);
    m_projectionMatrix[11] = 0.0f;
    
    m_projectionMatrix[12] = -(right + left) / (right - left);
    m_projectionMatrix[13] = -(top + bottom) / (top - bottom);
    m_projectionMatrix[14] = -(far + near) / (far - near);
    m_projectionMatrix[15] = 1.0f;
}

void Camera::applyBounds() {
    if (!m_hasBounds) {
        return;
    }
    
    // Calculate camera half size
    float halfWidth = m_viewportWidth * 0.5f / m_zoom;
    float halfHeight = m_viewportHeight * 0.5f / m_zoom;
    
    // Apply bounds
    m_x = std::max(m_boundLeft + halfWidth, std::min(m_boundRight - halfWidth, m_x));
    m_y = std::max(m_boundTop + halfHeight, std::min(m_boundBottom - halfHeight, m_y));
}

} // namespace Graphics
} // namespace RPGEngine