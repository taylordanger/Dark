#pragma once

#include <memory>
#include <vector>

namespace RPGEngine {
namespace Physics {

/**
 * Shape type enumeration
 */
enum class ShapeType {
    Circle,
    Rectangle,
    Polygon,
    Point
};

/**
 * Vector2 structure
 * Represents a 2D vector
 */
struct Vector2 {
    float x;
    float y;
    
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float x, float y) : x(x), y(y) {}
    
    // Vector operations
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }
    
    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }
    
    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }
    
    Vector2 operator/(float scalar) const {
        return Vector2(x / scalar, y / scalar);
    }
    
    // Dot product
    float dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }
    
    // Cross product (returns scalar for 2D vectors)
    float cross(const Vector2& other) const {
        return x * other.y - y * other.x;
    }
    
    // Length squared
    float lengthSquared() const {
        return x * x + y * y;
    }
    
    // Length
    float length() const {
        return std::sqrt(lengthSquared());
    }
    
    // Normalize
    Vector2 normalized() const {
        float len = length();
        if (len > 0.0f) {
            return Vector2(x / len, y / len);
        }
        return *this;
    }
    
    // Perpendicular vector (90 degrees counterclockwise)
    Vector2 perpendicular() const {
        return Vector2(-y, x);
    }
};

/**
 * Base collision shape class
 */
class CollisionShape {
public:
    /**
     * Constructor
     */
    CollisionShape() : m_position(0.0f, 0.0f), m_rotation(0.0f) {}
    
    /**
     * Virtual destructor
     */
    virtual ~CollisionShape() = default;
    
    /**
     * Get the shape type
     * @return Shape type
     */
    virtual ShapeType getType() const = 0;
    
    /**
     * Set the position
     * @param position Position
     */
    void setPosition(const Vector2& position) { m_position = position; }
    
    /**
     * Set the position
     * @param x X position
     * @param y Y position
     */
    void setPosition(float x, float y) { m_position.x = x; m_position.y = y; }
    
    /**
     * Get the position
     * @return Position
     */
    const Vector2& getPosition() const { return m_position; }
    
    /**
     * Set the rotation
     * @param rotation Rotation in radians
     */
    void setRotation(float rotation) { m_rotation = rotation; }
    
    /**
     * Get the rotation
     * @return Rotation in radians
     */
    float getRotation() const { return m_rotation; }
    
    /**
     * Check if this shape contains a point
     * @param point Point to check
     * @return true if the shape contains the point
     */
    virtual bool containsPoint(const Vector2& point) const = 0;
    
    /**
     * Get the closest point on the shape to the given point
     * @param point Point to check
     * @return Closest point on the shape
     */
    virtual Vector2 getClosestPoint(const Vector2& point) const = 0;
    
    /**
     * Get the bounding radius
     * @return Bounding radius
     */
    virtual float getBoundingRadius() const = 0;
    
    /**
     * Get the axis-aligned bounding box
     * @param min Output minimum point
     * @param max Output maximum point
     */
    virtual void getAABB(Vector2& min, Vector2& max) const = 0;
    
    /**
     * Clone the shape
     * @return New shape instance
     */
    virtual std::unique_ptr<CollisionShape> clone() const = 0;
    
protected:
    Vector2 m_position;
    float m_rotation;
};

/**
 * Circle collision shape
 */
class CircleShape : public CollisionShape {
public:
    /**
     * Constructor
     * @param radius Circle radius
     */
    CircleShape(float radius) : m_radius(radius) {}
    
    /**
     * Get the shape type
     * @return Shape type
     */
    ShapeType getType() const override { return ShapeType::Circle; }
    
    /**
     * Get the radius
     * @return Radius
     */
    float getRadius() const { return m_radius; }
    
    /**
     * Set the radius
     * @param radius Radius
     */
    void setRadius(float radius) { m_radius = radius; }
    
    /**
     * Check if this shape contains a point
     * @param point Point to check
     * @return true if the shape contains the point
     */
    bool containsPoint(const Vector2& point) const override {
        Vector2 delta = point - m_position;
        return delta.lengthSquared() <= m_radius * m_radius;
    }
    
    /**
     * Get the closest point on the shape to the given point
     * @param point Point to check
     * @return Closest point on the shape
     */
    Vector2 getClosestPoint(const Vector2& point) const override {
        Vector2 direction = point - m_position;
        float distance = direction.length();
        
        // If the point is inside the circle or at the center, return the point
        if (distance <= m_radius || distance < 0.0001f) {
            return point;
        }
        
        // Otherwise, return the point on the circle's edge in the direction of the point
        return m_position + direction.normalized() * m_radius;
    }
    
    /**
     * Get the bounding radius
     * @return Bounding radius
     */
    float getBoundingRadius() const override { return m_radius; }
    
    /**
     * Get the axis-aligned bounding box
     * @param min Output minimum point
     * @param max Output maximum point
     */
    void getAABB(Vector2& min, Vector2& max) const override {
        min.x = m_position.x - m_radius;
        min.y = m_position.y - m_radius;
        max.x = m_position.x + m_radius;
        max.y = m_position.y + m_radius;
    }
    
    /**
     * Clone the shape
     * @return New shape instance
     */
    std::unique_ptr<CollisionShape> clone() const override {
        auto shape = std::make_unique<CircleShape>(m_radius);
        shape->setPosition(m_position);
        shape->setRotation(m_rotation);
        return shape;
    }
    
private:
    float m_radius;
};

/**
 * Rectangle collision shape
 */
class RectangleShape : public CollisionShape {
public:
    /**
     * Constructor
     * @param width Rectangle width
     * @param height Rectangle height
     */
    RectangleShape(float width, float height) : m_width(width), m_height(height) {}
    
    /**
     * Get the shape type
     * @return Shape type
     */
    ShapeType getType() const override { return ShapeType::Rectangle; }
    
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
     * Get the half width
     * @return Half width
     */
    float getHalfWidth() const { return m_width * 0.5f; }
    
    /**
     * Get the half height
     * @return Half height
     */
    float getHalfHeight() const { return m_height * 0.5f; }
    
    /**
     * Check if this shape contains a point
     * @param point Point to check
     * @return true if the shape contains the point
     */
    bool containsPoint(const Vector2& point) const override {
        // If rotated, transform the point to local space
        if (m_rotation != 0.0f) {
            float cos = std::cos(-m_rotation);
            float sin = std::sin(-m_rotation);
            
            Vector2 localPoint;
            localPoint.x = cos * (point.x - m_position.x) - sin * (point.y - m_position.y);
            localPoint.y = sin * (point.x - m_position.x) + cos * (point.y - m_position.y);
            
            float halfWidth = m_width * 0.5f;
            float halfHeight = m_height * 0.5f;
            
            return localPoint.x >= -halfWidth && localPoint.x <= halfWidth &&
                   localPoint.y >= -halfHeight && localPoint.y <= halfHeight;
        } else {
            // No rotation, simple AABB check
            float halfWidth = m_width * 0.5f;
            float halfHeight = m_height * 0.5f;
            
            return point.x >= m_position.x - halfWidth && point.x <= m_position.x + halfWidth &&
                   point.y >= m_position.y - halfHeight && point.y <= m_position.y + halfHeight;
        }
    }
    
    /**
     * Get the closest point on the shape to the given point
     * @param point Point to check
     * @return Closest point on the shape
     */
    Vector2 getClosestPoint(const Vector2& point) const override {
        // If the point is inside the rectangle, return the point
        if (containsPoint(point)) {
            return point;
        }
        
        // Transform point to local space
        Vector2 localPoint;
        if (m_rotation != 0.0f) {
            float cos = std::cos(-m_rotation);
            float sin = std::sin(-m_rotation);
            
            localPoint.x = cos * (point.x - m_position.x) - sin * (point.y - m_position.y);
            localPoint.y = sin * (point.x - m_position.x) + cos * (point.y - m_position.y);
        } else {
            localPoint = point - m_position;
        }
        
        // Clamp to rectangle bounds
        float halfWidth = m_width * 0.5f;
        float halfHeight = m_height * 0.5f;
        
        Vector2 clampedPoint;
        clampedPoint.x = std::max(-halfWidth, std::min(halfWidth, localPoint.x));
        clampedPoint.y = std::max(-halfHeight, std::min(halfHeight, localPoint.y));
        
        // Transform back to world space
        if (m_rotation != 0.0f) {
            float cos = std::cos(m_rotation);
            float sin = std::sin(m_rotation);
            
            Vector2 worldPoint;
            worldPoint.x = cos * clampedPoint.x - sin * clampedPoint.y + m_position.x;
            worldPoint.y = sin * clampedPoint.x + cos * clampedPoint.y + m_position.y;
            return worldPoint;
        } else {
            return clampedPoint + m_position;
        }
    }
    
    /**
     * Get the bounding radius
     * @return Bounding radius
     */
    float getBoundingRadius() const override {
        // Radius of the circle that contains the rectangle
        return std::sqrt(m_width * m_width + m_height * m_height) * 0.5f;
    }
    
    /**
     * Get the axis-aligned bounding box
     * @param min Output minimum point
     * @param max Output maximum point
     */
    void getAABB(Vector2& min, Vector2& max) const override {
        // If rotated, compute the AABB of the rotated rectangle
        if (m_rotation != 0.0f) {
            float cos = std::cos(m_rotation);
            float sin = std::sin(m_rotation);
            
            float halfWidth = m_width * 0.5f;
            float halfHeight = m_height * 0.5f;
            
            // Compute the four corners of the rectangle
            Vector2 corners[4] = {
                Vector2(-halfWidth, -halfHeight),
                Vector2(halfWidth, -halfHeight),
                Vector2(halfWidth, halfHeight),
                Vector2(-halfWidth, halfHeight)
            };
            
            // Rotate and translate the corners
            for (int i = 0; i < 4; ++i) {
                float x = corners[i].x;
                float y = corners[i].y;
                
                corners[i].x = cos * x - sin * y + m_position.x;
                corners[i].y = sin * x + cos * y + m_position.y;
            }
            
            // Find the min and max points
            min = corners[0];
            max = corners[0];
            
            for (int i = 1; i < 4; ++i) {
                min.x = std::min(min.x, corners[i].x);
                min.y = std::min(min.y, corners[i].y);
                max.x = std::max(max.x, corners[i].x);
                max.y = std::max(max.y, corners[i].y);
            }
        } else {
            // No rotation, simple AABB
            float halfWidth = m_width * 0.5f;
            float halfHeight = m_height * 0.5f;
            
            min.x = m_position.x - halfWidth;
            min.y = m_position.y - halfHeight;
            max.x = m_position.x + halfWidth;
            max.y = m_position.y + halfHeight;
        }
    }
    
    /**
     * Clone the shape
     * @return New shape instance
     */
    std::unique_ptr<CollisionShape> clone() const override {
        auto shape = std::make_unique<RectangleShape>(m_width, m_height);
        shape->setPosition(m_position);
        shape->setRotation(m_rotation);
        return shape;
    }
    
    /**
     * Get the vertices of the rectangle
     * @return Vector of vertices
     */
    std::vector<Vector2> getVertices() const {
        float halfWidth = m_width * 0.5f;
        float halfHeight = m_height * 0.5f;
        
        std::vector<Vector2> vertices = {
            Vector2(-halfWidth, -halfHeight),
            Vector2(halfWidth, -halfHeight),
            Vector2(halfWidth, halfHeight),
            Vector2(-halfWidth, halfHeight)
        };
        
        // If rotated, rotate the vertices
        if (m_rotation != 0.0f) {
            float cos = std::cos(m_rotation);
            float sin = std::sin(m_rotation);
            
            for (auto& vertex : vertices) {
                float x = vertex.x;
                float y = vertex.y;
                
                vertex.x = cos * x - sin * y + m_position.x;
                vertex.y = sin * x + cos * y + m_position.y;
            }
        } else {
            // Just translate the vertices
            for (auto& vertex : vertices) {
                vertex.x += m_position.x;
                vertex.y += m_position.y;
            }
        }
        
        return vertices;
    }
    
private:
    float m_width;
    float m_height;
};

/**
 * Polygon collision shape
 */
class PolygonShape : public CollisionShape {
public:
    /**
     * Constructor
     * @param vertices Polygon vertices in local space
     */
    PolygonShape(const std::vector<Vector2>& vertices) : m_vertices(vertices) {
        updateNormals();
    }
    
    /**
     * Get the shape type
     * @return Shape type
     */
    ShapeType getType() const override { return ShapeType::Polygon; }
    
    /**
     * Get the vertices
     * @return Vertices in local space
     */
    const std::vector<Vector2>& getVertices() const { return m_vertices; }
    
    /**
     * Get the normals
     * @return Edge normals
     */
    const std::vector<Vector2>& getNormals() const { return m_normals; }
    
    /**
     * Set the vertices
     * @param vertices Vertices in local space
     */
    void setVertices(const std::vector<Vector2>& vertices) {
        m_vertices = vertices;
        updateNormals();
    }
    
    /**
     * Get the transformed vertices
     * @return Vertices in world space
     */
    std::vector<Vector2> getTransformedVertices() const {
        std::vector<Vector2> transformed;
        transformed.reserve(m_vertices.size());
        
        float cos = std::cos(m_rotation);
        float sin = std::sin(m_rotation);
        
        for (const auto& vertex : m_vertices) {
            Vector2 v;
            v.x = cos * vertex.x - sin * vertex.y + m_position.x;
            v.y = sin * vertex.x + cos * vertex.y + m_position.y;
            transformed.push_back(v);
        }
        
        return transformed;
    }
    
    /**
     * Check if this shape contains a point
     * @param point Point to check
     * @return true if the shape contains the point
     */
    bool containsPoint(const Vector2& point) const override {
        // Get transformed vertices
        std::vector<Vector2> transformed = getTransformedVertices();
        
        // Check if point is inside the polygon using the ray casting algorithm
        bool inside = false;
        for (size_t i = 0, j = transformed.size() - 1; i < transformed.size(); j = i++) {
            if (((transformed[i].y > point.y) != (transformed[j].y > point.y)) &&
                (point.x < (transformed[j].x - transformed[i].x) * (point.y - transformed[i].y) / 
                           (transformed[j].y - transformed[i].y) + transformed[i].x)) {
                inside = !inside;
            }
        }
        
        return inside;
    }
    
    /**
     * Get the closest point on the shape to the given point
     * @param point Point to check
     * @return Closest point on the shape
     */
    Vector2 getClosestPoint(const Vector2& point) const override {
        // If the point is inside the polygon, return the point
        if (containsPoint(point)) {
            return point;
        }
        
        // Get transformed vertices
        std::vector<Vector2> transformed = getTransformedVertices();
        
        if (transformed.empty()) {
            return m_position;
        }
        
        // Find the closest point on each edge
        float minDistanceSquared = std::numeric_limits<float>::max();
        Vector2 closestPoint = transformed[0];
        
        for (size_t i = 0; i < transformed.size(); ++i) {
            size_t j = (i + 1) % transformed.size();
            
            Vector2 edge = transformed[j] - transformed[i];
            Vector2 pointToVertex = point - transformed[i];
            
            // Project point onto edge
            float edgeLengthSquared = edge.lengthSquared();
            float dot = pointToVertex.dot(edge);
            
            // Calculate the closest point on the edge
            Vector2 closest;
            
            if (edgeLengthSquared < 0.0001f) {
                // Edge is too short, use the vertex
                closest = transformed[i];
            } else {
                // Calculate the projection parameter
                float t = dot / edgeLengthSquared;
                
                if (t < 0.0f) {
                    // Closest to vertex i
                    closest = transformed[i];
                } else if (t > 1.0f) {
                    // Closest to vertex j
                    closest = transformed[j];
                } else {
                    // Closest to point on the edge
                    closest = transformed[i] + edge * t;
                }
            }
            
            // Calculate distance squared
            float distanceSquared = (point - closest).lengthSquared();
            
            // Update if this is the closest edge
            if (distanceSquared < minDistanceSquared) {
                minDistanceSquared = distanceSquared;
                closestPoint = closest;
            }
        }
        
        return closestPoint;
    }
    
    /**
     * Get the bounding radius
     * @return Bounding radius
     */
    float getBoundingRadius() const override {
        float maxDistanceSquared = 0.0f;
        
        for (const auto& vertex : m_vertices) {
            float distanceSquared = vertex.lengthSquared();
            if (distanceSquared > maxDistanceSquared) {
                maxDistanceSquared = distanceSquared;
            }
        }
        
        return std::sqrt(maxDistanceSquared);
    }
    
    /**
     * Get the axis-aligned bounding box
     * @param min Output minimum point
     * @param max Output maximum point
     */
    void getAABB(Vector2& min, Vector2& max) const override {
        // Get transformed vertices
        std::vector<Vector2> transformed = getTransformedVertices();
        
        if (transformed.empty()) {
            min = m_position;
            max = m_position;
            return;
        }
        
        min = transformed[0];
        max = transformed[0];
        
        for (size_t i = 1; i < transformed.size(); ++i) {
            min.x = std::min(min.x, transformed[i].x);
            min.y = std::min(min.y, transformed[i].y);
            max.x = std::max(max.x, transformed[i].x);
            max.y = std::max(max.y, transformed[i].y);
        }
    }
    
    /**
     * Clone the shape
     * @return New shape instance
     */
    std::unique_ptr<CollisionShape> clone() const override {
        auto shape = std::make_unique<PolygonShape>(m_vertices);
        shape->setPosition(m_position);
        shape->setRotation(m_rotation);
        return shape;
    }
    
private:
    /**
     * Update the edge normals
     */
    void updateNormals() {
        m_normals.clear();
        m_normals.reserve(m_vertices.size());
        
        for (size_t i = 0; i < m_vertices.size(); ++i) {
            size_t j = (i + 1) % m_vertices.size();
            Vector2 edge = m_vertices[j] - m_vertices[i];
            Vector2 normal = edge.perpendicular().normalized();
            m_normals.push_back(normal);
        }
    }
    
    std::vector<Vector2> m_vertices;
    std::vector<Vector2> m_normals;
};

/**
 * Point collision shape
 */
class PointShape : public CollisionShape {
public:
    /**
     * Constructor
     */
    PointShape() {}
    
    /**
     * Get the shape type
     * @return Shape type
     */
    ShapeType getType() const override { return ShapeType::Point; }
    
    /**
     * Check if this shape contains a point
     * @param point Point to check
     * @return true if the shape contains the point
     */
    bool containsPoint(const Vector2& point) const override {
        // A point contains another point only if they are exactly the same
        return point.x == m_position.x && point.y == m_position.y;
    }
    
    /**
     * Get the closest point on the shape to the given point
     * @param point Point to check
     * @return The position of this point
     */
    Vector2 getClosestPoint(const Vector2& point) const override {
        // A point's closest point is always itself
        return m_position;
    }
    
    /**
     * Get the bounding radius
     * @return Bounding radius
     */
    float getBoundingRadius() const override { return 0.0f; }
    
    /**
     * Get the axis-aligned bounding box
     * @param min Output minimum point
     * @param max Output maximum point
     */
    void getAABB(Vector2& min, Vector2& max) const override {
        min = m_position;
        max = m_position;
    }
    
    /**
     * Clone the shape
     * @return New shape instance
     */
    std::unique_ptr<CollisionShape> clone() const override {
        auto shape = std::make_unique<PointShape>();
        shape->setPosition(m_position);
        shape->setRotation(m_rotation);
        return shape;
    }
};

} // namespace Physics
} // namespace RPGEngine