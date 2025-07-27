#include "CollisionDetection.h"
#include <limits>
#include <cmath>

namespace RPGEngine {
namespace Physics {

bool CollisionDetection::checkCollision(const CollisionShape& shape1, const CollisionShape& shape2, CollisionResult* result) {
    // First, perform a quick bounding radius check
    Vector2 delta = shape2.getPosition() - shape1.getPosition();
    float radiusSum = shape1.getBoundingRadius() + shape2.getBoundingRadius();
    
    if (delta.lengthSquared() > radiusSum * radiusSum) {
        return false; // Shapes are too far apart
    }
    
    // Dispatch to specific collision detection functions based on shape types
    ShapeType type1 = shape1.getType();
    ShapeType type2 = shape2.getType();
    
    if (type1 == ShapeType::Circle && type2 == ShapeType::Circle) {
        return circleVsCircle(static_cast<const CircleShape&>(shape1), static_cast<const CircleShape&>(shape2), result);
    } else if (type1 == ShapeType::Circle && type2 == ShapeType::Rectangle) {
        return circleVsRectangle(static_cast<const CircleShape&>(shape1), static_cast<const RectangleShape&>(shape2), result);
    } else if (type1 == ShapeType::Rectangle && type2 == ShapeType::Circle) {
        bool colliding = circleVsRectangle(static_cast<const CircleShape&>(shape2), static_cast<const RectangleShape&>(shape1), result);
        
        // Flip the normal if we have a result
        if (result && colliding) {
            result->normal = result->normal * -1.0f;
        }
        
        return colliding;
    } else if (type1 == ShapeType::Rectangle && type2 == ShapeType::Rectangle) {
        return rectangleVsRectangle(static_cast<const RectangleShape&>(shape1), static_cast<const RectangleShape&>(shape2), result);
    } else if (type1 == ShapeType::Circle && type2 == ShapeType::Polygon) {
        return circleVsPolygon(static_cast<const CircleShape&>(shape1), static_cast<const PolygonShape&>(shape2), result);
    } else if (type1 == ShapeType::Polygon && type2 == ShapeType::Circle) {
        bool colliding = circleVsPolygon(static_cast<const CircleShape&>(shape2), static_cast<const PolygonShape&>(shape1), result);
        
        // Flip the normal if we have a result
        if (result && colliding) {
            result->normal = result->normal * -1.0f;
        }
        
        return colliding;
    } else if (type1 == ShapeType::Rectangle && type2 == ShapeType::Polygon) {
        return rectangleVsPolygon(static_cast<const RectangleShape&>(shape1), static_cast<const PolygonShape&>(shape2), result);
    } else if (type1 == ShapeType::Polygon && type2 == ShapeType::Rectangle) {
        bool colliding = rectangleVsPolygon(static_cast<const RectangleShape&>(shape2), static_cast<const PolygonShape&>(shape1), result);
        
        // Flip the normal if we have a result
        if (result && colliding) {
            result->normal = result->normal * -1.0f;
        }
        
        return colliding;
    } else if (type1 == ShapeType::Polygon && type2 == ShapeType::Polygon) {
        return polygonVsPolygon(static_cast<const PolygonShape&>(shape1), static_cast<const PolygonShape&>(shape2), result);
    } else if (type1 == ShapeType::Point) {
        return pointInShape(shape1.getPosition(), shape2);
    } else if (type2 == ShapeType::Point) {
        return pointInShape(shape2.getPosition(), shape1);
    }
    
    // Unsupported shape combination
    return false;
}

bool CollisionDetection::pointInShape(const Vector2& point, const CollisionShape& shape) {
    return shape.containsPoint(point);
}

bool CollisionDetection::circleVsCircle(const CircleShape& circle1, const CircleShape& circle2, CollisionResult* result) {
    Vector2 delta = circle2.getPosition() - circle1.getPosition();
    float distanceSquared = delta.lengthSquared();
    float radiusSum = circle1.getRadius() + circle2.getRadius();
    
    if (distanceSquared > radiusSum * radiusSum) {
        return false; // No collision
    }
    
    // Collision detected
    if (result) {
        result->colliding = true;
        
        float distance = std::sqrt(distanceSquared);
        
        // Handle the case where circles are at the same position
        if (distance < 0.0001f) {
            result->normal = Vector2(1.0f, 0.0f); // Arbitrary direction
            result->penetration = circle1.getRadius() + circle2.getRadius();
        } else {
            result->normal = delta / distance; // Normalized direction from circle1 to circle2
            result->penetration = radiusSum - distance;
        }
        
        // Calculate contact point (on the surface of circle1 in the direction of circle2)
        result->contactPoint = circle1.getPosition() + result->normal * circle1.getRadius();
    }
    
    return true;
}

bool CollisionDetection::circleVsRectangle(const CircleShape& circle, const RectangleShape& rect, CollisionResult* result) {
    // Find the closest point on the rectangle to the circle center
    Vector2 closestPoint = closestPointOnRectangle(rect, circle.getPosition());
    
    // Calculate the vector from the closest point to the circle center
    Vector2 delta = circle.getPosition() - closestPoint;
    float distanceSquared = delta.lengthSquared();
    
    // Check if the closest point is inside the circle
    if (distanceSquared > circle.getRadius() * circle.getRadius()) {
        return false; // No collision
    }
    
    // Collision detected
    if (result) {
        result->colliding = true;
        
        float distance = std::sqrt(distanceSquared);
        
        // Handle the case where the circle center is exactly on the closest point
        if (distance < 0.0001f) {
            // Find the closest edge normal
            std::vector<Vector2> vertices = rect.getVertices();
            std::vector<Vector2> normals;
            
            // Calculate edge normals
            for (size_t i = 0; i < vertices.size(); ++i) {
                size_t j = (i + 1) % vertices.size();
                Vector2 edge = vertices[j] - vertices[i];
                Vector2 normal = edge.perpendicular().normalized();
                normals.push_back(normal);
            }
            
            // Find the closest edge
            float minDistance = std::numeric_limits<float>::max();
            Vector2 closestNormal;
            
            for (size_t i = 0; i < vertices.size(); ++i) {
                size_t j = (i + 1) % vertices.size();
                Vector2 edge = vertices[j] - vertices[i];
                Vector2 edgeNormal = normals[i];
                
                // Project circle center onto the edge
                Vector2 circleToVertex = circle.getPosition() - vertices[i];
                float projection = circleToVertex.dot(edge.normalized());
                
                // Check if the projection is on the edge
                if (projection >= 0.0f && projection <= edge.length()) {
                    float dist = std::abs(circleToVertex.dot(edgeNormal));
                    if (dist < minDistance) {
                        minDistance = dist;
                        closestNormal = edgeNormal;
                    }
                }
            }
            
            result->normal = closestNormal;
            result->penetration = circle.getRadius();
        } else {
            result->normal = delta / distance; // Normalized direction from closest point to circle center
            result->penetration = circle.getRadius() - distance;
        }
        
        // Calculate contact point (on the surface of the rectangle)
        result->contactPoint = closestPoint;
    }
    
    return true;
}

bool CollisionDetection::rectangleVsRectangle(const RectangleShape& rect1, const RectangleShape& rect2, CollisionResult* result) {
    // If either rectangle is rotated, use the polygon vs polygon algorithm
    if (rect1.getRotation() != 0.0f || rect2.getRotation() != 0.0f) {
        // Convert rectangles to polygons
        PolygonShape poly1(rect1.getVertices());
        PolygonShape poly2(rect2.getVertices());
        
        return polygonVsPolygon(poly1, poly2, result);
    }
    
    // AABB vs AABB collision
    Vector2 min1, max1, min2, max2;
    rect1.getAABB(min1, max1);
    rect2.getAABB(min2, max2);
    
    // Check for overlap
    if (max1.x < min2.x || min1.x > max2.x || max1.y < min2.y || min1.y > max2.y) {
        return false; // No collision
    }
    
    // Collision detected
    if (result) {
        result->colliding = true;
        
        // Calculate penetration depths along each axis
        float penetrationX = std::min(max1.x - min2.x, max2.x - min1.x);
        float penetrationY = std::min(max1.y - min2.y, max2.y - min1.y);
        
        // Use the minimum penetration depth
        if (penetrationX < penetrationY) {
            result->penetration = penetrationX;
            
            // Determine the direction of the normal
            if (rect1.getPosition().x < rect2.getPosition().x) {
                result->normal = Vector2(1.0f, 0.0f);
            } else {
                result->normal = Vector2(-1.0f, 0.0f);
            }
        } else {
            result->penetration = penetrationY;
            
            // Determine the direction of the normal
            if (rect1.getPosition().y < rect2.getPosition().y) {
                result->normal = Vector2(0.0f, 1.0f);
            } else {
                result->normal = Vector2(0.0f, -1.0f);
            }
        }
        
        // Calculate contact point (at the center of the overlap region)
        Vector2 overlapMin(std::max(min1.x, min2.x), std::max(min1.y, min2.y));
        Vector2 overlapMax(std::min(max1.x, max2.x), std::min(max1.y, max2.y));
        result->contactPoint = Vector2(
            (overlapMin.x + overlapMax.x) * 0.5f,
            (overlapMin.y + overlapMax.y) * 0.5f
        );
    }
    
    return true;
}

bool CollisionDetection::circleVsPolygon(const CircleShape& circle, const PolygonShape& polygon, CollisionResult* result) {
    // Find the closest point on the polygon to the circle center
    Vector2 closestPoint;
    int closestEdge;
    closestEdgeOnPolygon(polygon, circle.getPosition(), closestEdge, closestPoint);
    
    // Calculate the vector from the closest point to the circle center
    Vector2 delta = circle.getPosition() - closestPoint;
    float distanceSquared = delta.lengthSquared();
    
    // Check if the closest point is inside the circle
    if (distanceSquared > circle.getRadius() * circle.getRadius()) {
        return false; // No collision
    }
    
    // Collision detected
    if (result) {
        result->colliding = true;
        
        float distance = std::sqrt(distanceSquared);
        
        // Handle the case where the circle center is exactly on the closest point
        if (distance < 0.0001f) {
            // Use the edge normal
            std::vector<Vector2> normals = polygon.getNormals();
            result->normal = normals[closestEdge];
            result->penetration = circle.getRadius();
        } else {
            result->normal = delta / distance; // Normalized direction from closest point to circle center
            result->penetration = circle.getRadius() - distance;
        }
        
        // Calculate contact point (on the surface of the polygon)
        result->contactPoint = closestPoint;
    }
    
    return true;
}

bool CollisionDetection::rectangleVsPolygon(const RectangleShape& rect, const PolygonShape& polygon, CollisionResult* result) {
    // Convert rectangle to polygon
    PolygonShape rectPoly(rect.getVertices());
    
    return polygonVsPolygon(rectPoly, polygon, result);
}

bool CollisionDetection::polygonVsPolygon(const PolygonShape& polygon1, const PolygonShape& polygon2, CollisionResult* result) {
    // Separating Axis Theorem (SAT) implementation
    std::vector<Vector2> axes;
    
    // Get the normals from both polygons
    const std::vector<Vector2>& normals1 = polygon1.getNormals();
    const std::vector<Vector2>& normals2 = polygon2.getNormals();
    
    // Add all normals to the axes list
    axes.insert(axes.end(), normals1.begin(), normals1.end());
    axes.insert(axes.end(), normals2.begin(), normals2.end());
    
    float minOverlap = std::numeric_limits<float>::max();
    Vector2 minAxis;
    
    // Check each axis for separation
    for (const auto& axis : axes) {
        float min1, max1, min2, max2;
        
        // Project both polygons onto the axis
        projectShape(polygon1, axis, min1, max1);
        projectShape(polygon2, axis, min2, max2);
        
        // Check for separation
        if (min1 > max2 || min2 > max1) {
            return false; // Separation found, no collision
        }
        
        // Calculate overlap
        float overlap = std::min(max1 - min2, max2 - min1);
        
        // Keep track of minimum overlap
        if (overlap < minOverlap) {
            minOverlap = overlap;
            minAxis = axis;
            
            // Ensure the normal points from polygon1 to polygon2
            Vector2 centerDiff = polygon2.getPosition() - polygon1.getPosition();
            if (centerDiff.dot(minAxis) < 0.0f) {
                minAxis = minAxis * -1.0f;
            }
        }
    }
    
    // Collision detected
    if (result) {
        result->colliding = true;
        result->normal = minAxis;
        result->penetration = minOverlap;
        
        // Calculate contact point (approximate)
        // Find the closest vertex of polygon1 to polygon2 along the collision normal
        const std::vector<Vector2>& vertices1 = polygon1.getTransformedVertices();
        float maxDot = -std::numeric_limits<float>::max();
        Vector2 furthestVertex;
        
        for (const auto& vertex : vertices1) {
            float dot = vertex.dot(minAxis);
            if (dot > maxDot) {
                maxDot = dot;
                furthestVertex = vertex;
            }
        }
        
        result->contactPoint = furthestVertex - minAxis * minOverlap;
    }
    
    return true;
}

void CollisionDetection::projectShape(const CollisionShape& shape, const Vector2& axis, float& min, float& max) {
    ShapeType type = shape.getType();
    
    if (type == ShapeType::Circle) {
        projectCircle(static_cast<const CircleShape&>(shape), axis, min, max);
    } else if (type == ShapeType::Rectangle) {
        projectVertices(static_cast<const RectangleShape&>(shape).getVertices(), axis, min, max);
    } else if (type == ShapeType::Polygon) {
        projectVertices(static_cast<const PolygonShape&>(shape).getTransformedVertices(), axis, min, max);
    } else if (type == ShapeType::Point) {
        // Project a point
        float projection = shape.getPosition().dot(axis);
        min = max = projection;
    }
}

void CollisionDetection::projectCircle(const CircleShape& circle, const Vector2& axis, float& min, float& max) {
    // Project center
    float centerProjection = circle.getPosition().dot(axis);
    
    // Add/subtract radius
    min = centerProjection - circle.getRadius();
    max = centerProjection + circle.getRadius();
}

void CollisionDetection::projectVertices(const std::vector<Vector2>& vertices, const Vector2& axis, float& min, float& max) {
    min = std::numeric_limits<float>::max();
    max = -std::numeric_limits<float>::max();
    
    for (const auto& vertex : vertices) {
        float projection = vertex.dot(axis);
        min = std::min(min, projection);
        max = std::max(max, projection);
    }
}

Vector2 CollisionDetection::closestPointOnRectangle(const RectangleShape& rect, const Vector2& point) {
    // If the rectangle is rotated, transform the point to the rectangle's local space
    if (rect.getRotation() != 0.0f) {
        float cos = std::cos(-rect.getRotation());
        float sin = std::sin(-rect.getRotation());
        
        Vector2 localPoint;
        localPoint.x = cos * (point.x - rect.getPosition().x) - sin * (point.y - rect.getPosition().y);
        localPoint.y = sin * (point.x - rect.getPosition().x) + cos * (point.y - rect.getPosition().y);
        
        // Clamp the point to the rectangle bounds
        float halfWidth = rect.getWidth() * 0.5f;
        float halfHeight = rect.getHeight() * 0.5f;
        
        Vector2 clampedPoint;
        clampedPoint.x = std::max(-halfWidth, std::min(halfWidth, localPoint.x));
        clampedPoint.y = std::max(-halfHeight, std::min(halfHeight, localPoint.y));
        
        // Transform back to world space
        Vector2 worldPoint;
        worldPoint.x = cos * clampedPoint.x + sin * clampedPoint.y + rect.getPosition().x;
        worldPoint.y = -sin * clampedPoint.x + cos * clampedPoint.y + rect.getPosition().y;
        
        return worldPoint;
    } else {
        // No rotation, simple AABB
        float halfWidth = rect.getWidth() * 0.5f;
        float halfHeight = rect.getHeight() * 0.5f;
        
        Vector2 clampedPoint;
        clampedPoint.x = std::max(rect.getPosition().x - halfWidth, std::min(rect.getPosition().x + halfWidth, point.x));
        clampedPoint.y = std::max(rect.getPosition().y - halfHeight, std::min(rect.getPosition().y + halfHeight, point.y));
        
        return clampedPoint;
    }
}

Vector2 CollisionDetection::closestPointOnPolygon(const PolygonShape& polygon, const Vector2& point) {
    Vector2 closestPoint;
    int edgeIndex;
    closestEdgeOnPolygon(polygon, point, edgeIndex, closestPoint);
    return closestPoint;
}

void CollisionDetection::closestEdgeOnPolygon(const PolygonShape& polygon, const Vector2& point, 
                                            int& edgeIndex, Vector2& closestPoint) {
    const std::vector<Vector2>& vertices = polygon.getTransformedVertices();
    
    if (vertices.empty()) {
        edgeIndex = -1;
        closestPoint = polygon.getPosition();
        return;
    }
    
    float minDistanceSquared = std::numeric_limits<float>::max();
    edgeIndex = -1;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        size_t j = (i + 1) % vertices.size();
        
        Vector2 edge = vertices[j] - vertices[i];
        Vector2 pointToVertex = point - vertices[i];
        
        // Project point onto edge
        float edgeLengthSquared = edge.lengthSquared();
        float dot = pointToVertex.dot(edge);
        
        // Calculate the closest point on the edge
        Vector2 closest;
        
        if (edgeLengthSquared < 0.0001f) {
            // Edge is too short, use the vertex
            closest = vertices[i];
        } else {
            // Calculate the projection parameter
            float t = dot / edgeLengthSquared;
            
            if (t < 0.0f) {
                // Closest to vertex i
                closest = vertices[i];
            } else if (t > 1.0f) {
                // Closest to vertex j
                closest = vertices[j];
            } else {
                // Closest to point on the edge
                closest = vertices[i] + edge * t;
            }
        }
        
        // Calculate distance squared
        float distanceSquared = (point - closest).lengthSquared();
        
        // Update if this is the closest edge
        if (distanceSquared < minDistanceSquared) {
            minDistanceSquared = distanceSquared;
            edgeIndex = static_cast<int>(i);
            closestPoint = closest;
        }
    }
}

} // namespace Physics
} // namespace RPGEngine