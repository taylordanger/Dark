#include "MapObject.h"

namespace RPGEngine {
namespace World {

MapObject::MapObject(uint32_t id, const std::string& name, const std::string& type,
                   float x, float y, float width, float height, float rotation)
    : m_id(id)
    , m_name(name)
    , m_type(type)
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
    , m_rotation(rotation)
{
}

MapObject::~MapObject() {
    // Nothing to do here
}

std::string MapObject::getProperty(const std::string& name, const std::string& defaultValue) const {
    auto it = m_properties.find(name);
    if (it != m_properties.end()) {
        return it->second;
    }
    return defaultValue;
}

void MapObject::setProperty(const std::string& name, const std::string& value) {
    m_properties[name] = value;
}

bool MapObject::hasProperty(const std::string& name) const {
    return m_properties.find(name) != m_properties.end();
}

std::shared_ptr<Physics::CollisionShape> MapObject::createCollisionShape(Physics::ShapeType shapeType) {
    // Create collision shape based on shape type
    switch (shapeType) {
        case Physics::ShapeType::Rectangle: {
            auto shape = std::make_shared<Physics::RectangleShape>(m_width, m_height);
            shape->setPosition(m_x + m_width / 2.0f, m_y + m_height / 2.0f);
            shape->setRotation(m_rotation);
            m_collisionShape = shape;
            break;
        }
        case Physics::ShapeType::Circle: {
            float radius = std::min(m_width, m_height) / 2.0f;
            auto shape = std::make_shared<Physics::CircleShape>(radius);
            shape->setPosition(m_x + m_width / 2.0f, m_y + m_height / 2.0f);
            m_collisionShape = shape;
            break;
        }
        case Physics::ShapeType::Point: {
            auto shape = std::make_shared<Physics::PointShape>();
            shape->setPosition(m_x, m_y);
            m_collisionShape = shape;
            break;
        }
        case Physics::ShapeType::Polygon: {
            // Create a rectangular polygon
            std::vector<Physics::Vector2> vertices = {
                Physics::Vector2(0.0f, 0.0f),
                Physics::Vector2(m_width, 0.0f),
                Physics::Vector2(m_width, m_height),
                Physics::Vector2(0.0f, m_height)
            };
            auto shape = std::make_shared<Physics::PolygonShape>(vertices);
            shape->setPosition(m_x, m_y);
            shape->setRotation(m_rotation);
            m_collisionShape = shape;
            break;
        }
    }
    
    return m_collisionShape;
}

} // namespace World
} // namespace RPGEngine