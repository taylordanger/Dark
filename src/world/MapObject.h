#pragma once

#include "../physics/CollisionShape.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace RPGEngine {
namespace World {

/**
 * Map object class
 * Represents an object in a map
 */
class MapObject {
public:
    /**
     * Constructor
     * @param id Object ID
     * @param name Object name
     * @param type Object type
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param rotation Rotation in degrees
     */
    MapObject(uint32_t id, const std::string& name, const std::string& type,
             float x, float y, float width, float height, float rotation = 0.0f);
    
    /**
     * Destructor
     */
    ~MapObject();
    
    /**
     * Get the object ID
     * @return Object ID
     */
    uint32_t getId() const { return m_id; }
    
    /**
     * Get the object name
     * @return Object name
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * Set the object name
     * @param name Object name
     */
    void setName(const std::string& name) { m_name = name; }
    
    /**
     * Get the object type
     * @return Object type
     */
    const std::string& getType() const { return m_type; }
    
    /**
     * Set the object type
     * @param type Object type
     */
    void setType(const std::string& type) { m_type = type; }
    
    /**
     * Get the X position
     * @return X position
     */
    float getX() const { return m_x; }
    
    /**
     * Set the X position
     * @param x X position
     */
    void setX(float x) { m_x = x; }
    
    /**
     * Get the Y position
     * @return Y position
     */
    float getY() const { return m_y; }
    
    /**
     * Set the Y position
     * @param y Y position
     */
    void setY(float y) { m_y = y; }
    
    /**
     * Get the width
     * @return Width
     */
    float getWidth() const { return m_width; }
    
    /**
     * Set the width
     * @param width Width
     */
    void setWidth(float width) { m_width = width; }
    
    /**
     * Get the height
     * @return Height
     */
    float getHeight() const { return m_height; }
    
    /**
     * Set the height
     * @param height Height
     */
    void setHeight(float height) { m_height = height; }
    
    /**
     * Get the rotation
     * @return Rotation in degrees
     */
    float getRotation() const { return m_rotation; }
    
    /**
     * Set the rotation
     * @param rotation Rotation in degrees
     */
    void setRotation(float rotation) { m_rotation = rotation; }
    
    /**
     * Get a property
     * @param name Property name
     * @param defaultValue Default value if property doesn't exist
     * @return Property value
     */
    std::string getProperty(const std::string& name, const std::string& defaultValue = "") const;
    
    /**
     * Set a property
     * @param name Property name
     * @param value Property value
     */
    void setProperty(const std::string& name, const std::string& value);
    
    /**
     * Check if a property exists
     * @param name Property name
     * @return true if the property exists
     */
    bool hasProperty(const std::string& name) const;
    
    /**
     * Get all properties
     * @return Map of properties
     */
    const std::unordered_map<std::string, std::string>& getProperties() const { return m_properties; }
    
    /**
     * Get the collision shape
     * @return Collision shape
     */
    std::shared_ptr<Physics::CollisionShape> getCollisionShape() const { return m_collisionShape; }
    
    /**
     * Set the collision shape
     * @param shape Collision shape
     */
    void setCollisionShape(std::shared_ptr<Physics::CollisionShape> shape) { m_collisionShape = shape; }
    
    /**
     * Create a collision shape based on the object's dimensions
     * @param shapeType Shape type
     * @return Created collision shape
     */
    std::shared_ptr<Physics::CollisionShape> createCollisionShape(Physics::ShapeType shapeType);
    
private:
    uint32_t m_id;                                              // Object ID
    std::string m_name;                                         // Object name
    std::string m_type;                                         // Object type
    float m_x;                                                  // X position
    float m_y;                                                  // Y position
    float m_width;                                              // Width
    float m_height;                                             // Height
    float m_rotation;                                           // Rotation in degrees
    std::unordered_map<std::string, std::string> m_properties;  // Object properties
    std::shared_ptr<Physics::CollisionShape> m_collisionShape;  // Collision shape
};

} // namespace World
} // namespace RPGEngine