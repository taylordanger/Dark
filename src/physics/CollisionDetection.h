#pragma once

#include "CollisionShape.h"
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>

namespace RPGEngine {
namespace Physics {

/**
 * Collision result structure
 * Contains information about a collision
 */
struct CollisionResult {
    bool colliding;           // Whether the shapes are colliding
    Vector2 normal;           // Collision normal (points from shape1 to shape2)
    float penetration;        // Penetration depth
    Vector2 contactPoint;     // Contact point
    
    CollisionResult() : colliding(false), normal(0.0f, 0.0f), penetration(0.0f), contactPoint(0.0f, 0.0f) {}
};

/**
 * Collision detection class
 * Provides static methods for collision detection between shapes
 */
class CollisionDetection {
public:
    /**
     * Check if two shapes are colliding
     * @param shape1 First shape
     * @param shape2 Second shape
     * @param result Optional collision result
     * @return true if the shapes are colliding
     */
    static bool checkCollision(const CollisionShape& shape1, const CollisionShape& shape2, CollisionResult* result = nullptr);
    
    /**
     * Check if a point is inside a shape
     * @param point Point to check
     * @param shape Shape to check against
     * @return true if the point is inside the shape
     */
    static bool pointInShape(const Vector2& point, const CollisionShape& shape);
    
    /**
     * Check if two circles are colliding
     * @param circle1 First circle
     * @param circle2 Second circle
     * @param result Optional collision result
     * @return true if the circles are colliding
     */
    static bool circleVsCircle(const CircleShape& circle1, const CircleShape& circle2, CollisionResult* result = nullptr);
    
    /**
     * Check if a circle and a rectangle are colliding
     * @param circle Circle
     * @param rect Rectangle
     * @param result Optional collision result
     * @return true if the shapes are colliding
     */
    static bool circleVsRectangle(const CircleShape& circle, const RectangleShape& rect, CollisionResult* result = nullptr);
    
    /**
     * Check if two rectangles are colliding
     * @param rect1 First rectangle
     * @param rect2 Second rectangle
     * @param result Optional collision result
     * @return true if the rectangles are colliding
     */
    static bool rectangleVsRectangle(const RectangleShape& rect1, const RectangleShape& rect2, CollisionResult* result = nullptr);
    
    /**
     * Check if a circle and a polygon are colliding
     * @param circle Circle
     * @param polygon Polygon
     * @param result Optional collision result
     * @return true if the shapes are colliding
     */
    static bool circleVsPolygon(const CircleShape& circle, const PolygonShape& polygon, CollisionResult* result = nullptr);
    
    /**
     * Check if a rectangle and a polygon are colliding
     * @param rect Rectangle
     * @param polygon Polygon
     * @param result Optional collision result
     * @return true if the shapes are colliding
     */
    static bool rectangleVsPolygon(const RectangleShape& rect, const PolygonShape& polygon, CollisionResult* result = nullptr);
    
    /**
     * Check if two polygons are colliding using the Separating Axis Theorem
     * @param polygon1 First polygon
     * @param polygon2 Second polygon
     * @param result Optional collision result
     * @return true if the polygons are colliding
     */
    static bool polygonVsPolygon(const PolygonShape& polygon1, const PolygonShape& polygon2, CollisionResult* result = nullptr);
    
private:
    /**
     * Project a shape onto an axis
     * @param shape Shape to project
     * @param axis Axis to project onto
     * @param min Output minimum projection
     * @param max Output maximum projection
     */
    static void projectShape(const CollisionShape& shape, const Vector2& axis, float& min, float& max);
    
    /**
     * Project a circle onto an axis
     * @param circle Circle to project
     * @param axis Axis to project onto
     * @param min Output minimum projection
     * @param max Output maximum projection
     */
    static void projectCircle(const CircleShape& circle, const Vector2& axis, float& min, float& max);
    
    /**
     * Project vertices onto an axis
     * @param vertices Vertices to project
     * @param axis Axis to project onto
     * @param min Output minimum projection
     * @param max Output maximum projection
     */
    static void projectVertices(const std::vector<Vector2>& vertices, const Vector2& axis, float& min, float& max);
    
    /**
     * Find the closest point on a rectangle to a point
     * @param rect Rectangle
     * @param point Point
     * @return Closest point on the rectangle
     */
    static Vector2 closestPointOnRectangle(const RectangleShape& rect, const Vector2& point);
    
    /**
     * Find the closest point on a polygon to a point
     * @param polygon Polygon
     * @param point Point
     * @return Closest point on the polygon
     */
    static Vector2 closestPointOnPolygon(const PolygonShape& polygon, const Vector2& point);
    
    /**
     * Find the closest edge on a polygon to a point
     * @param polygon Polygon
     * @param point Point
     * @param edgeIndex Output edge index
     * @param closestPoint Output closest point on the edge
     */
    static void closestEdgeOnPolygon(const PolygonShape& polygon, const Vector2& point, 
                                    int& edgeIndex, Vector2& closestPoint);
};

} // namespace Physics
} // namespace RPGEngine