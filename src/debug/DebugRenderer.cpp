#include "DebugRenderer.h"
#include <cmath>

namespace Engine {
    namespace Debug {

        DebugRenderer::DebugRenderer(std::shared_ptr<RPGEngine::Graphics::IGraphicsAPI> graphicsAPI)
            : m_graphicsAPI(graphicsAPI), m_enabled(true) {
        }

        DebugRenderer::~DebugRenderer() {
        }

        void DebugRenderer::drawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a) {
            if (!m_enabled) return;
            m_lines.push_back({x1, y1, x2, y2, r, g, b, a});
        }

        void DebugRenderer::drawRect(float x, float y, float width, float height, bool filled, float r, float g, float b, float a) {
            if (!m_enabled) return;
            m_rects.push_back({x, y, width, height, r, g, b, a, filled});
        }

        void DebugRenderer::drawCircle(float x, float y, float radius, bool filled, float r, float g, float b, float a) {
            if (!m_enabled) return;
            m_circles.push_back({x, y, radius, r, g, b, a, filled});
        }

        void DebugRenderer::drawCollisionShape(const RPGEngine::Physics::CollisionShape& shape, float r, float g, float b, float a) {
            if (!m_enabled) return;

            auto shapeType = shape.getType();
            auto position = shape.getPosition();

            switch (shapeType) {
                case RPGEngine::Physics::ShapeType::Circle: {
                    const auto* circleShape = dynamic_cast<const RPGEngine::Physics::CircleShape*>(&shape);
                    if (circleShape) {
                        drawCircle(position.x, position.y, circleShape->getRadius(), false, r, g, b, a);
                    }
                    break;
                }
                case RPGEngine::Physics::ShapeType::Rectangle: {
                    const auto* rectShape = dynamic_cast<const RPGEngine::Physics::RectangleShape*>(&shape);
                    if (rectShape) {
                        drawRect(position.x - rectShape->getHalfWidth(), 
                                position.y - rectShape->getHalfHeight(),
                                rectShape->getWidth(), 
                                rectShape->getHeight(), 
                                false, r, g, b, a);
                    }
                    break;
                }
                case RPGEngine::Physics::ShapeType::Polygon: {
                    const auto* polyShape = dynamic_cast<const RPGEngine::Physics::PolygonShape*>(&shape);
                    if (polyShape) {
                        auto vertices = polyShape->getTransformedVertices();
                        for (size_t i = 0; i < vertices.size(); ++i) {
                            size_t next = (i + 1) % vertices.size();
                            const auto& v1 = vertices[i];
                            const auto& v2 = vertices[next];
                            drawLine(v1.x, v1.y, v2.x, v2.y, r, g, b, a);
                        }
                    }
                    break;
                }
                case RPGEngine::Physics::ShapeType::Point: {
                    // Draw a small cross for point shapes
                    float size = 2.0f;
                    drawLine(position.x - size, position.y, position.x + size, position.y, r, g, b, a);
                    drawLine(position.x, position.y - size, position.x, position.y + size, r, g, b, a);
                    break;
                }
            }
        }

        void DebugRenderer::drawAABB(float minX, float minY, float maxX, float maxY, float r, float g, float b, float a) {
            if (!m_enabled) return;
            drawRect(minX, minY, maxX - minX, maxY - minY, false, r, g, b, a);
        }

        void DebugRenderer::drawVelocityVector(float x, float y, float vx, float vy, float scale) {
            if (!m_enabled) return;
            
            float endX = x + vx * scale;
            float endY = y + vy * scale;
            
            // Draw velocity line
            drawLine(x, y, endX, endY, 0.0f, 0.0f, 1.0f, 1.0f);
            
            // Draw arrowhead
            float arrowLength = 5.0f;
            float angle = std::atan2(vy, vx);
            float arrowAngle1 = angle + 2.618f; // 150 degrees
            float arrowAngle2 = angle - 2.618f; // -150 degrees
            
            float arrow1X = endX + arrowLength * std::cos(arrowAngle1);
            float arrow1Y = endY + arrowLength * std::sin(arrowAngle1);
            float arrow2X = endX + arrowLength * std::cos(arrowAngle2);
            float arrow2Y = endY + arrowLength * std::sin(arrowAngle2);
            
            drawLine(endX, endY, arrow1X, arrow1Y, 0.0f, 0.0f, 1.0f, 1.0f);
            drawLine(endX, endY, arrow2X, arrow2Y, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        void DebugRenderer::drawGrid(float cellSize, float r, float g, float b, float a) {
            if (!m_enabled) return;
            
            // Get viewport dimensions (simplified - would need camera info in real implementation)
            float viewWidth = 800.0f;  // Would get from camera/viewport
            float viewHeight = 600.0f;
            
            // Draw vertical lines
            for (float x = 0; x < viewWidth; x += cellSize) {
                drawLine(x, 0, x, viewHeight, r, g, b, a);
            }
            
            // Draw horizontal lines
            for (float y = 0; y < viewHeight; y += cellSize) {
                drawLine(0, y, viewWidth, y, r, g, b, a);
            }
        }

        void DebugRenderer::render() {
            if (!m_enabled || !m_graphicsAPI) return;
            
            renderLines();
            renderRects();
            renderCircles();
        }

        void DebugRenderer::clear() {
            m_lines.clear();
            m_rects.clear();
            m_circles.clear();
        }

        void DebugRenderer::renderLines() {
            // In a real implementation, this would create vertex buffers and render them
            // For now, we'll just store the debug data for later use
            // The actual rendering would be done by a higher-level system
        }

        void DebugRenderer::renderRects() {
            // Convert rectangles to lines for rendering
            for (const auto& rect : m_rects) {
                if (!rect.filled) {
                    // Draw rectangle outline as lines
                    m_lines.push_back({rect.x, rect.y, rect.x + rect.width, rect.y, rect.r, rect.g, rect.b, rect.a});
                    m_lines.push_back({rect.x + rect.width, rect.y, rect.x + rect.width, rect.y + rect.height, rect.r, rect.g, rect.b, rect.a});
                    m_lines.push_back({rect.x + rect.width, rect.y + rect.height, rect.x, rect.y + rect.height, rect.r, rect.g, rect.b, rect.a});
                    m_lines.push_back({rect.x, rect.y + rect.height, rect.x, rect.y, rect.r, rect.g, rect.b, rect.a});
                }
            }
        }

        void DebugRenderer::renderCircles() {
            // Convert circles to lines for rendering
            for (const auto& circle : m_circles) {
                // Draw circle as line segments
                const int segments = 32;
                const float angleStep = 2.0f * 3.14159f / segments;
                
                for (int i = 0; i < segments; ++i) {
                    float angle1 = i * angleStep;
                    float angle2 = (i + 1) * angleStep;
                    
                    float x1 = circle.x + circle.radius * std::cos(angle1);
                    float y1 = circle.y + circle.radius * std::sin(angle1);
                    float x2 = circle.x + circle.radius * std::cos(angle2);
                    float y2 = circle.y + circle.radius * std::sin(angle2);
                    
                    m_lines.push_back({x1, y1, x2, y2, circle.r, circle.g, circle.b, circle.a});
                }
            }
        }

    } // namespace Debug
} // namespace Engine