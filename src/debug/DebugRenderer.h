#pragma once

#include <vector>
#include <memory>
#include "../graphics/IGraphicsAPI.h"
#include "../physics/CollisionShape.h"
#include "../core/Types.h"

namespace Engine {
    namespace Debug {

        struct DebugLine {
            float x1, y1, x2, y2;
            float r, g, b, a;
        };

        struct DebugRect {
            float x, y, width, height;
            float r, g, b, a;
            bool filled;
        };

        struct DebugCircle {
            float x, y, radius;
            float r, g, b, a;
            bool filled;
        };

        class DebugRenderer {
        public:
            DebugRenderer(std::shared_ptr<RPGEngine::Graphics::IGraphicsAPI> graphicsAPI);
            ~DebugRenderer();

            // Basic drawing primitives
            void drawLine(float x1, float y1, float x2, float y2, float r = 1.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
            void drawRect(float x, float y, float width, float height, bool filled = false, float r = 1.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
            void drawCircle(float x, float y, float radius, bool filled = false, float r = 1.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

            // Collision shape rendering
            void drawCollisionShape(const RPGEngine::Physics::CollisionShape& shape, float r = 0.0f, float g = 1.0f, float b = 0.0f, float a = 0.5f);
            void drawAABB(float minX, float minY, float maxX, float maxY, float r = 1.0f, float g = 1.0f, float b = 0.0f, float a = 0.5f);

            // Physics debugging
            void drawVelocityVector(float x, float y, float vx, float vy, float scale = 1.0f);
            void drawGrid(float cellSize, float r = 0.3f, float g = 0.3f, float b = 0.3f, float a = 0.5f);

            // Render all debug primitives
            void render();
            void clear();

            // Enable/disable debug rendering
            void setEnabled(bool enabled) { m_enabled = enabled; }
            bool isEnabled() const { return m_enabled; }

        private:
            std::shared_ptr<RPGEngine::Graphics::IGraphicsAPI> m_graphicsAPI;
            std::vector<DebugLine> m_lines;
            std::vector<DebugRect> m_rects;
            std::vector<DebugCircle> m_circles;
            bool m_enabled;

            void renderLines();
            void renderRects();
            void renderCircles();
        };

    } // namespace Debug
} // namespace Engine