#pragma once

#include "Component.h"
#include <cmath>

namespace RPGEngine {
namespace Components {

/**
 * Transform Component
 * Stores position, rotation, and scale of an entity
 */
class TransformComponent : public Component<TransformComponent> {
public:
    /**
     * Constructor
     * @param entityId Entity ID
     * @param x X position
     * @param y Y position
     * @param rotation Rotation in degrees
     * @param scaleX X scale
     * @param scaleY Y scale
     */
    TransformComponent(EntityId entityId, float x = 0.0f, float y = 0.0f, 
                      float rotation = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f)
        : Component<TransformComponent>(entityId)
        , m_x(x)
        , m_y(y)
        , m_rotation(rotation)
        , m_scaleX(scaleX)
        , m_scaleY(scaleY)
    {
    }
    
    // Position
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setPosition(float x, float y) { m_x = x; m_y = y; }
    
    // Rotation
    float getRotation() const { return m_rotation; }
    void setRotation(float rotation) { m_rotation = rotation; }
    void rotate(float degrees) { m_rotation += degrees; }
    
    // Scale
    float getScaleX() const { return m_scaleX; }
    float getScaleY() const { return m_scaleY; }
    void setScaleX(float scaleX) { m_scaleX = scaleX; }
    void setScaleY(float scaleY) { m_scaleY = scaleY; }
    void setScale(float scale) { m_scaleX = scale; m_scaleY = scale; }
    void setScale(float scaleX, float scaleY) { m_scaleX = scaleX; m_scaleY = scaleY; }
    
    // Movement
    void translate(float dx, float dy) { m_x += dx; m_y += dy; }
    
    // Distance calculation
    float distanceTo(const TransformComponent& other) const {
        float dx = m_x - other.m_x;
        float dy = m_y - other.m_y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
private:
    float m_x;        // X position
    float m_y;        // Y position
    float m_rotation; // Rotation in degrees
    float m_scaleX;   // X scale
    float m_scaleY;   // Y scale
};

} // namespace Components
} // namespace RPGEngine