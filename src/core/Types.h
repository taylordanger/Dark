#pragma once

#include <cstdint>
#include <limits>

namespace RPGEngine {
    
    // Entity system types
    using EntityId = uint32_t;
    using ComponentId = uint32_t;
    using SystemId = uint32_t;
    using SceneId = uint32_t;
    
    // Invalid/null identifiers
    constexpr EntityId INVALID_ENTITY_ID = std::numeric_limits<EntityId>::max();
    constexpr ComponentId INVALID_COMPONENT_ID = std::numeric_limits<ComponentId>::max();
    constexpr SystemId INVALID_SYSTEM_ID = std::numeric_limits<SystemId>::max();
    constexpr SceneId INVALID_SCENE_ID = std::numeric_limits<SceneId>::max();
    
    // Math types
    struct Vector2 {
        float x, y;
        
        Vector2() : x(0.0f), y(0.0f) {}
        Vector2(float x, float y) : x(x), y(y) {}
        
        Vector2 operator+(const Vector2& other) const {
            return Vector2(x + other.x, y + other.y);
        }
        
        Vector2 operator-(const Vector2& other) const {
            return Vector2(x - other.x, y - other.y);
        }
        
        Vector2 operator*(float scalar) const {
            return Vector2(x * scalar, y * scalar);
        }
    };
    
    struct Vector3 {
        float x, y, z;
        
        Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    };
    
    struct Color {
        float r, g, b, a;
        
        Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
        Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    };
    
    struct Rectangle {
        float x, y, width, height;
        
        Rectangle() : x(0.0f), y(0.0f), width(0.0f), height(0.0f) {}
        Rectangle(float x, float y, float width, float height) 
            : x(x), y(y), width(width), height(height) {}
    };
    
} // namespace RPGEngine