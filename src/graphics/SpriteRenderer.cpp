#include "SpriteRenderer.h"
#include "Texture.h"
#include <iostream>
#include <cmath>

namespace RPGEngine {
namespace Graphics {

// Vertex shader source
const std::string spriteVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 vertexColor;
out vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    vertexColor = aColor;
    texCoord = aTexCoord;
}
)";

// Fragment shader source
const std::string spriteFragmentShaderSource = R"(
#version 330 core
in vec4 vertexColor;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D textureSampler;

void main() {
    vec4 texColor = texture(textureSampler, texCoord);
    FragColor = texColor * vertexColor;
}
)";

SpriteRenderer::SpriteRenderer(std::shared_ptr<IGraphicsAPI> graphicsAPI, std::shared_ptr<ShaderManager> shaderManager)
    : System("SpriteRenderer")
    , m_graphicsAPI(graphicsAPI)
    , m_shaderManager(shaderManager)
    , m_currentBatch(nullptr)
    , m_whiteTexture(nullptr)
    , m_shaderName("sprite")
    , m_isDrawing(false)
    , m_camera(nullptr)
    , m_vertexPool(1024)  // Pre-allocate vertex pool
    , m_indexPool(512)    // Pre-allocate index pool
{
    // Initialize matrices to identity
    for (int i = 0; i < 16; ++i) {
        m_projectionMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        m_viewMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

SpriteRenderer::~SpriteRenderer() {
    if (isInitialized()) {
        shutdown();
    }
}

bool SpriteRenderer::onInitialize() {
    if (!m_graphicsAPI || !m_shaderManager) {
        std::cerr << "Graphics API or Shader Manager not provided to SpriteRenderer" << std::endl;
        return false;
    }
    
    // Create shader
    if (!createShader()) {
        std::cerr << "Failed to create sprite shader" << std::endl;
        return false;
    }
    
    // Create white texture
    if (!createWhiteTexture()) {
        std::cerr << "Failed to create white texture" << std::endl;
        return false;
    }
    
    // Set default orthographic projection
    int width = m_graphicsAPI->getWindowWidth();
    int height = m_graphicsAPI->getWindowHeight();
    setOrthographicProjection(0, width, height, 0);
    
    std::cout << "SpriteRenderer initialized" << std::endl;
    return true;
}

void SpriteRenderer::onUpdate(float deltaTime) {
    // Nothing to do here
}

void SpriteRenderer::onShutdown() {
    // Clean up batches
    for (auto& batch : m_batches) {
        if (batch.vertexArray != INVALID_HANDLE) {
            m_graphicsAPI->deleteVertexArray(batch.vertexArray);
        }
        
        if (batch.vertexBuffer != INVALID_HANDLE) {
            m_graphicsAPI->deleteVertexBuffer(batch.vertexBuffer);
        }
        
        if (batch.indexBuffer != INVALID_HANDLE) {
            m_graphicsAPI->deleteIndexBuffer(batch.indexBuffer);
        }
    }
    
    m_batches.clear();
    m_currentBatch = nullptr;
    
    std::cout << "SpriteRenderer shutdown" << std::endl;
}

void SpriteRenderer::begin() {
    if (m_isDrawing) {
        std::cerr << "SpriteRenderer::begin() called while already drawing" << std::endl;
        return;
    }
    
    m_isDrawing = true;
    m_currentBatch = nullptr;
    
    // Update frustum culling if camera is set
    if (m_camera) {
        m_frustumCuller.updateFrustum(*m_camera);
    }
    
    // Clear batch map for new frame
    m_batchMap.clear();
    
    // Use sprite shader
    m_shaderManager->useShader(m_shaderName);
    
    // Set matrices
    m_shaderManager->setUniformMatrix4("projection", m_projectionMatrix);
    m_shaderManager->setUniformMatrix4("view", m_viewMatrix);
    
    // Set texture sampler
    m_shaderManager->setUniform("textureSampler", 0);
    
    // Enable blending
    m_graphicsAPI->setBlendMode(BlendMode::Alpha);
}

void SpriteRenderer::end() {
    if (!m_isDrawing) {
        std::cerr << "SpriteRenderer::end() called without begin()" << std::endl;
        return;
    }
    
    // Optimize batches before final flush
    optimizeBatches();
    
    // Flush any remaining sprites
    flushBatch();
    
    m_isDrawing = false;
}

void SpriteRenderer::drawSprites(const std::vector<Sprite>& sprites) {
    if (!m_isDrawing) {
        std::cerr << "SpriteRenderer::drawSprites() called without begin()" << std::endl;
        return;
    }
    
    // Use frustum culling to filter visible sprites
    std::vector<const Sprite*> visibleSprites;
    if (m_camera) {
        m_frustumCuller.cullSprites(sprites, visibleSprites);
    } else {
        // No culling, add all sprites
        visibleSprites.reserve(sprites.size());
        for (const auto& sprite : sprites) {
            visibleSprites.push_back(&sprite);
        }
    }
    
    // Draw visible sprites
    for (const Sprite* sprite : visibleSprites) {
        drawSprite(*sprite);
    }
}

void SpriteRenderer::setCamera(const Camera* camera) {
    m_camera = camera;
}

void SpriteRenderer::drawSprite(const Sprite& sprite) {
    if (!m_isDrawing) {
        std::cerr << "SpriteRenderer::drawSprite() called without begin()" << std::endl;
        return;
    }
    
    // Frustum culling check
    if (m_camera && !m_frustumCuller.isSpriteVisible(sprite)) {
        return; // Skip invisible sprites
    }
    
    // Get sprite properties
    std::shared_ptr<Texture> texture = sprite.getTexture();
    if (!texture || !texture->isValid()) {
        return;
    }
    
    float x, y;
    sprite.getPosition(x, y);
    
    float scaleX, scaleY;
    sprite.getScale(scaleX, scaleY);
    
    float originX, originY;
    sprite.getOrigin(originX, originY);
    
    bool flipX, flipY;
    sprite.getFlip(flipX, flipY);
    
    const Rect& textureRect = sprite.getTextureRect();
    const Color& color = sprite.getColor();
    
    // Find or create batch for this texture
    SpriteBatch* batch = findBatch(texture);
    if (!batch) {
        batch = createBatch(texture);
    }
    
    // Add sprite to batch
    addSpriteToBatch(
        batch,
        x, y,
        textureRect.width * scaleX, textureRect.height * scaleY,
        textureRect.x, textureRect.y,
        textureRect.width, textureRect.height,
        color,
        sprite.getRotation(),
        originX, originY,
        flipX, flipY
    );
}

void SpriteRenderer::drawTexture(std::shared_ptr<Texture> texture, float x, float y, float width, float height, const Color& color) {
    if (!m_isDrawing) {
        std::cerr << "SpriteRenderer::drawTexture() called without begin()" << std::endl;
        return;
    }
    
    if (!texture || !texture->isValid()) {
        return;
    }
    
    // Find or create batch for this texture
    SpriteBatch* batch = findBatch(texture);
    if (!batch) {
        batch = createBatch(texture);
    }
    
    // Add sprite to batch
    addSpriteToBatch(
        batch,
        x, y, width, height,
        0, 0,
        texture->getWidth(), texture->getHeight(),
        color
    );
}

void SpriteRenderer::drawTextureRegion(std::shared_ptr<Texture> texture, float x, float y, float width, float height,
                                     float texX, float texY, float texWidth, float texHeight, const Color& color) {
    if (!m_isDrawing) {
        std::cerr << "SpriteRenderer::drawTextureRegion() called without begin()" << std::endl;
        return;
    }
    
    if (!texture || !texture->isValid()) {
        return;
    }
    
    // Find or create batch for this texture
    SpriteBatch* batch = findBatch(texture);
    if (!batch) {
        batch = createBatch(texture);
    }
    
    // Add sprite to batch
    addSpriteToBatch(
        batch,
        x, y, width, height,
        texX, texY, texWidth, texHeight,
        color
    );
}

void SpriteRenderer::drawRectangle(float x, float y, float width, float height, const Color& color, bool filled) {
    if (!m_isDrawing) {
        std::cerr << "SpriteRenderer::drawRectangle() called without begin()" << std::endl;
        return;
    }
    
    if (!m_whiteTexture || !m_whiteTexture->isValid()) {
        return;
    }
    
    if (filled) {
        // Find or create batch for white texture
        SpriteBatch* batch = findBatch(m_whiteTexture);
        if (!batch) {
            batch = createBatch(m_whiteTexture);
        }
        
        // Add sprite to batch
        addSpriteToBatch(
            batch,
            x, y, width, height,
            0, 0, 1, 1,
            color
        );
    } else {
        // Draw outline using four rectangles
        float lineWidth = 1.0f;
        
        // Top
        drawRectangle(x, y, width, lineWidth, color, true);
        // Bottom
        drawRectangle(x, y + height - lineWidth, width, lineWidth, color, true);
        // Left
        drawRectangle(x, y + lineWidth, lineWidth, height - 2 * lineWidth, color, true);
        // Right
        drawRectangle(x + width - lineWidth, y + lineWidth, lineWidth, height - 2 * lineWidth, color, true);
    }
}

void SpriteRenderer::setProjectionMatrix(const float* matrix) {
    if (!matrix) {
        return;
    }
    
    for (int i = 0; i < 16; ++i) {
        m_projectionMatrix[i] = matrix[i];
    }
    
    if (m_isDrawing) {
        m_shaderManager->setUniformMatrix4("projection", m_projectionMatrix);
    }
}

void SpriteRenderer::setViewMatrix(const float* matrix) {
    if (!matrix) {
        return;
    }
    
    for (int i = 0; i < 16; ++i) {
        m_viewMatrix[i] = matrix[i];
    }
    
    if (m_isDrawing) {
        m_shaderManager->setUniformMatrix4("view", m_viewMatrix);
    }
}

void SpriteRenderer::setOrthographicProjection(float left, float right, float bottom, float top, float near, float far) {
    // Create orthographic projection matrix
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
    
    if (m_isDrawing) {
        m_shaderManager->setUniformMatrix4("projection", m_projectionMatrix);
    }
}

void SpriteRenderer::flushBatch() {
    if (!m_currentBatch || m_currentBatch->spriteCount == 0) {
        return;
    }
    
    // Bind texture
    m_currentBatch->texture->bind(0);
    
    // Update vertex buffer
    m_graphicsAPI->updateVertexBuffer(
        m_currentBatch->vertexBuffer,
        m_currentBatch->vertices.data(),
        m_currentBatch->vertices.size() * sizeof(float)
    );
    
    // Update index buffer if needed
    if (m_currentBatch->indices.size() > 0) {
        m_graphicsAPI->updateIndexBuffer(
            m_currentBatch->indexBuffer,
            m_currentBatch->indices.data(),
            m_currentBatch->indices.size() * sizeof(uint16_t)
        );
    }
    
    // Bind vertex array
    m_graphicsAPI->bindVertexArray(m_currentBatch->vertexArray);
    
    // Draw elements
    m_graphicsAPI->drawElements(
        PrimitiveType::Triangles,
        m_currentBatch->spriteCount * INDICES_PER_SPRITE,
        static_cast<uint32_t>(VertexDataType::UnsignedShort),
        0
    );
    
    // Reset sprite count
    m_currentBatch->spriteCount = 0;
    m_currentBatch->vertices.clear();
    m_currentBatch->indices.clear();
}

SpriteBatch* SpriteRenderer::createBatch(std::shared_ptr<Texture> texture) {
    // Flush current batch if different texture
    if (m_currentBatch && m_currentBatch->texture != texture) {
        flushBatch();
    }
    
    // Create new batch
    m_batches.emplace_back();
    SpriteBatch& batch = m_batches.back();
    
    batch.texture = texture;
    batch.spriteCount = 0;
    
    // Reserve space for vertices and indices
    batch.vertices.reserve(MAX_SPRITES_PER_BATCH * VERTICES_PER_SPRITE * VERTEX_SIZE);
    batch.indices.reserve(MAX_SPRITES_PER_BATCH * INDICES_PER_SPRITE);
    
    // Create vertex buffer
    batch.vertexBuffer = m_graphicsAPI->createVertexBuffer(
        nullptr,
        MAX_SPRITES_PER_BATCH * VERTICES_PER_SPRITE * VERTEX_SIZE * sizeof(float),
        true
    );
    
    // Create index buffer
    batch.indexBuffer = m_graphicsAPI->createIndexBuffer(
        nullptr,
        MAX_SPRITES_PER_BATCH * INDICES_PER_SPRITE * sizeof(uint16_t),
        true
    );
    
    // Define vertex attributes
    std::vector<VertexAttribute> attributes = {
        { "aPos",      0, 3, VertexDataType::Float, false, VERTEX_SIZE * sizeof(float), 0 },
        { "aColor",    1, 4, VertexDataType::Float, false, VERTEX_SIZE * sizeof(float), 3 * sizeof(float) },
        { "aTexCoord", 2, 2, VertexDataType::Float, false, VERTEX_SIZE * sizeof(float), 7 * sizeof(float) }
    };
    
    // Create vertex array
    batch.vertexArray = m_graphicsAPI->createVertexArray(
        batch.vertexBuffer,
        batch.indexBuffer,
        attributes
    );
    
    m_currentBatch = &batch;
    return m_currentBatch;
}

SpriteBatch* SpriteRenderer::findBatch(std::shared_ptr<Texture> texture) {
    // Check batch map for fast lookup
    auto it = m_batchMap.find(texture);
    if (it != m_batchMap.end()) {
        SpriteBatch* batch = it->second;
        if (batch->spriteCount < MAX_SPRITES_PER_BATCH) {
            // Flush current batch if different
            if (m_currentBatch && m_currentBatch != batch && m_currentBatch->spriteCount > 0) {
                flushBatch();
            }
            m_currentBatch = batch;
            return m_currentBatch;
        }
    }
    
    // Check if current batch can be used
    if (m_currentBatch && m_currentBatch->texture == texture) {
        // Check if batch is full
        if (m_currentBatch->spriteCount >= MAX_SPRITES_PER_BATCH) {
            flushBatch();
        } else {
            return m_currentBatch;
        }
    }
    
    // Look for existing batch with same texture
    for (auto& batch : m_batches) {
        if (batch.texture == texture && batch.spriteCount < MAX_SPRITES_PER_BATCH) {
            // Flush current batch if different
            if (m_currentBatch && m_currentBatch != &batch && m_currentBatch->spriteCount > 0) {
                flushBatch();
            }
            
            m_currentBatch = &batch;
            m_batchMap[texture] = m_currentBatch; // Update map
            return m_currentBatch;
        }
    }
    
    // No suitable batch found
    return nullptr;
}

void SpriteRenderer::addSpriteToBatch(SpriteBatch* batch, float x, float y, float width, float height,
                                    float texX, float texY, float texWidth, float texHeight,
                                    const Color& color, float rotation,
                                    float originX, float originY,
                                    bool flipX, bool flipY) {
    if (!batch) {
        return;
    }
    
    // Calculate texture coordinates
    float texLeft = texX;
    float texRight = texX + texWidth;
    float texTop = texY;
    float texBottom = texY + texHeight;
    
    // Normalize texture coordinates
    if (batch->texture) {
        float texW = static_cast<float>(batch->texture->getWidth());
        float texH = static_cast<float>(batch->texture->getHeight());
        
        if (texW > 0 && texH > 0) {
            texLeft /= texW;
            texRight /= texW;
            texTop /= texH;
            texBottom /= texH;
        }
    }
    
    // Apply flipping
    if (flipX) {
        std::swap(texLeft, texRight);
    }
    
    if (flipY) {
        std::swap(texTop, texBottom);
    }
    
    // Calculate vertex positions
    float originOffsetX = width * originX;
    float originOffsetY = height * originY;
    
    // Define the four corners (local space)
    float x0 = -originOffsetX;
    float y0 = -originOffsetY;
    float x1 = width - originOffsetX;
    float y1 = height - originOffsetY;
    
    // Apply rotation if needed
    if (rotation != 0.0f) {
        float radians = rotation * 3.14159f / 180.0f;
        float cos = std::cos(radians);
        float sin = std::sin(radians);
        
        // Rotated positions
        float rx0 = cos * x0 - sin * y0;
        float ry0 = sin * x0 + cos * y0;
        float rx1 = cos * x1 - sin * y0;
        float ry1 = sin * x1 + cos * y0;
        float rx2 = cos * x1 - sin * y1;
        float ry2 = sin * x1 + cos * y1;
        float rx3 = cos * x0 - sin * y1;
        float ry3 = sin * x0 + cos * y1;
        
        // Update positions
        x0 = rx0;
        y0 = ry0;
        x1 = rx2;
        y1 = ry2;
        
        // Add vertices (position, color, texcoord)
        // Bottom-left
        batch->vertices.push_back(x + rx0);
        batch->vertices.push_back(y + ry0);
        batch->vertices.push_back(0.0f);
        batch->vertices.push_back(color.r);
        batch->vertices.push_back(color.g);
        batch->vertices.push_back(color.b);
        batch->vertices.push_back(color.a);
        batch->vertices.push_back(texLeft);
        batch->vertices.push_back(texTop);
        
        // Bottom-right
        batch->vertices.push_back(x + rx1);
        batch->vertices.push_back(y + ry1);
        batch->vertices.push_back(0.0f);
        batch->vertices.push_back(color.r);
        batch->vertices.push_back(color.g);
        batch->vertices.push_back(color.b);
        batch->vertices.push_back(color.a);
        batch->vertices.push_back(texRight);
        batch->vertices.push_back(texTop);
        
        // Top-right
        batch->vertices.push_back(x + rx2);
        batch->vertices.push_back(y + ry2);
        batch->vertices.push_back(0.0f);
        batch->vertices.push_back(color.r);
        batch->vertices.push_back(color.g);
        batch->vertices.push_back(color.b);
        batch->vertices.push_back(color.a);
        batch->vertices.push_back(texRight);
        batch->vertices.push_back(texBottom);
        
        // Top-left
        batch->vertices.push_back(x + rx3);
        batch->vertices.push_back(y + ry3);
        batch->vertices.push_back(0.0f);
        batch->vertices.push_back(color.r);
        batch->vertices.push_back(color.g);
        batch->vertices.push_back(color.b);
        batch->vertices.push_back(color.a);
        batch->vertices.push_back(texLeft);
        batch->vertices.push_back(texBottom);
    } else {
        // Add vertices (position, color, texcoord)
        // Bottom-left
        batch->vertices.push_back(x + x0);
        batch->vertices.push_back(y + y0);
        batch->vertices.push_back(0.0f);
        batch->vertices.push_back(color.r);
        batch->vertices.push_back(color.g);
        batch->vertices.push_back(color.b);
        batch->vertices.push_back(color.a);
        batch->vertices.push_back(texLeft);
        batch->vertices.push_back(texTop);
        
        // Bottom-right
        batch->vertices.push_back(x + x1);
        batch->vertices.push_back(y + y0);
        batch->vertices.push_back(0.0f);
        batch->vertices.push_back(color.r);
        batch->vertices.push_back(color.g);
        batch->vertices.push_back(color.b);
        batch->vertices.push_back(color.a);
        batch->vertices.push_back(texRight);
        batch->vertices.push_back(texTop);
        
        // Top-right
        batch->vertices.push_back(x + x1);
        batch->vertices.push_back(y + y1);
        batch->vertices.push_back(0.0f);
        batch->vertices.push_back(color.r);
        batch->vertices.push_back(color.g);
        batch->vertices.push_back(color.b);
        batch->vertices.push_back(color.a);
        batch->vertices.push_back(texRight);
        batch->vertices.push_back(texBottom);
        
        // Top-left
        batch->vertices.push_back(x + x0);
        batch->vertices.push_back(y + y1);
        batch->vertices.push_back(0.0f);
        batch->vertices.push_back(color.r);
        batch->vertices.push_back(color.g);
        batch->vertices.push_back(color.b);
        batch->vertices.push_back(color.a);
        batch->vertices.push_back(texLeft);
        batch->vertices.push_back(texBottom);
    }
    
    // Add indices
    uint16_t baseIndex = batch->spriteCount * VERTICES_PER_SPRITE;
    batch->indices.push_back(baseIndex + 0);
    batch->indices.push_back(baseIndex + 1);
    batch->indices.push_back(baseIndex + 2);
    batch->indices.push_back(baseIndex + 0);
    batch->indices.push_back(baseIndex + 2);
    batch->indices.push_back(baseIndex + 3);
    
    // Increment sprite count
    batch->spriteCount++;
    
    // Flush if batch is full
    if (batch->spriteCount >= MAX_SPRITES_PER_BATCH) {
        flushBatch();
    }
}

bool SpriteRenderer::createWhiteTexture() {
    // Create a 1x1 white texture
    const int texSize = 1;
    uint8_t textureData[texSize * texSize * 4];
    
    // Fill with white
    for (int i = 0; i < texSize * texSize * 4; ++i) {
        textureData[i] = 255;
    }
    
    // Create texture
    m_whiteTexture = std::make_shared<Texture>(m_graphicsAPI);
    return m_whiteTexture->createFromData(texSize, texSize, TextureFormat::RGBA, textureData);
}

bool SpriteRenderer::createShader() {
    // Create sprite shader
    return m_shaderManager->loadShaderFromSource(m_shaderName, spriteVertexShaderSource, spriteFragmentShaderSource);
}

void SpriteRenderer::optimizeBatches() {
    // Sort batches by texture to minimize texture switches
    std::sort(m_batches.begin(), m_batches.end(), [](const SpriteBatch& a, const SpriteBatch& b) {
        if (a.spriteCount == 0) return false;
        if (b.spriteCount == 0) return true;
        return a.texture.get() < b.texture.get();
    });
    
    // Update batch map after sorting
    m_batchMap.clear();
    for (auto& batch : m_batches) {
        if (batch.spriteCount > 0) {
            m_batchMap[batch.texture] = &batch;
        }
    }
}

} // namespace Graphics
} // namespace RPGEngine