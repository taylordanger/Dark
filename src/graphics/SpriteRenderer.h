#pragma once

#include "IGraphicsAPI.h"
#include "ShaderManager.h"
#include "Sprite.h"
#include "FrustumCuller.h"
#include "../systems/System.h"
#include "../core/MemoryPool.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace RPGEngine {
namespace Graphics {

/**
 * Sprite batch structure
 * Used for batch rendering of sprites
 */
struct SpriteBatch {
    std::shared_ptr<Texture> texture;
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    BufferHandle vertexBuffer;
    BufferHandle indexBuffer;
    VertexArrayHandle vertexArray;
    int spriteCount;
    
    SpriteBatch() : texture(nullptr), vertexBuffer(INVALID_HANDLE), 
                   indexBuffer(INVALID_HANDLE), vertexArray(INVALID_HANDLE), spriteCount(0) {}
};

/**
 * Optimized Sprite Renderer
 * Renders sprites with batching, frustum culling, and memory pooling for performance
 */
class SpriteRenderer : public System {
public:
    /**
     * Constructor
     * @param graphicsAPI Graphics API to use
     * @param shaderManager Shader manager to use
     */
    SpriteRenderer(std::shared_ptr<IGraphicsAPI> graphicsAPI, std::shared_ptr<ShaderManager> shaderManager);
    
    /**
     * Destructor
     */
    ~SpriteRenderer();
    
    /**
     * Initialize the sprite renderer
     * @return true if initialization was successful
     */
    bool onInitialize() override;
    
    /**
     * Update the sprite renderer
     * @param deltaTime Time elapsed since last update
     */
    void onUpdate(float deltaTime) override;
    
    /**
     * Shutdown the sprite renderer
     */
    void onShutdown() override;
    
    /**
     * Begin rendering
     * Call this before drawing any sprites
     */
    void begin();
    
    /**
     * End rendering
     * Call this after drawing all sprites to flush the batch
     */
    void end();
    
    /**
     * Draw a sprite (with frustum culling)
     * @param sprite Sprite to draw
     */
    void drawSprite(const Sprite& sprite);
    
    /**
     * Draw multiple sprites with automatic batching and culling
     * @param sprites Vector of sprites to draw
     */
    void drawSprites(const std::vector<Sprite>& sprites);
    
    /**
     * Set the camera for frustum culling
     * @param camera Camera to use for culling
     */
    void setCamera(const Camera* camera);
    
    /**
     * Draw a texture
     * @param texture Texture to draw
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param color Color
     */
    void drawTexture(std::shared_ptr<Texture> texture, float x, float y, float width, float height, const Color& color = Color::White);
    
    /**
     * Draw a texture region
     * @param texture Texture to draw
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param texX X position in texture
     * @param texY Y position in texture
     * @param texWidth Width in texture
     * @param texHeight Height in texture
     * @param color Color
     */
    void drawTextureRegion(std::shared_ptr<Texture> texture, float x, float y, float width, float height,
                          float texX, float texY, float texWidth, float texHeight, const Color& color = Color::White);
    
    /**
     * Draw a rectangle
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param color Color
     * @param filled Whether to fill the rectangle
     */
    void drawRectangle(float x, float y, float width, float height, const Color& color, bool filled = true);
    
    /**
     * Set the projection matrix
     * @param matrix Projection matrix
     */
    void setProjectionMatrix(const float* matrix);
    
    /**
     * Set the view matrix
     * @param matrix View matrix
     */
    void setViewMatrix(const float* matrix);
    
    /**
     * Create an orthographic projection matrix
     * @param left Left coordinate
     * @param right Right coordinate
     * @param bottom Bottom coordinate
     * @param top Top coordinate
     * @param near Near plane
     * @param far Far plane
     */
    void setOrthographicProjection(float left, float right, float bottom, float top, float near = -1.0f, float far = 1.0f);
    
private:
    /**
     * Flush the current batch
     */
    void flushBatch();
    
    /**
     * Optimize batches by sorting and merging
     */
    void optimizeBatches();
    
    /**
     * Create a new batch
     * @param texture Texture to use
     * @return Pointer to the new batch
     */
    SpriteBatch* createBatch(std::shared_ptr<Texture> texture);
    
    /**
     * Find a batch for the specified texture
     * @param texture Texture to find batch for
     * @return Pointer to the batch, or nullptr if not found
     */
    SpriteBatch* findBatch(std::shared_ptr<Texture> texture);
    
    /**
     * Add a sprite to a batch
     * @param batch Batch to add to
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param texX X position in texture
     * @param texY Y position in texture
     * @param texWidth Width in texture
     * @param texHeight Height in texture
     * @param color Color
     * @param rotation Rotation angle
     * @param originX Origin X (0-1)
     * @param originY Origin Y (0-1)
     * @param flipX Horizontal flip
     * @param flipY Vertical flip
     */
    void addSpriteToBatch(SpriteBatch* batch, float x, float y, float width, float height,
                         float texX, float texY, float texWidth, float texHeight,
                         const Color& color, float rotation = 0.0f,
                         float originX = 0.5f, float originY = 0.5f,
                         bool flipX = false, bool flipY = false);
    
    /**
     * Create a white texture for drawing shapes
     * @return true if texture was created successfully
     */
    bool createWhiteTexture();
    
    /**
     * Create shader for sprite rendering
     * @return true if shader was created successfully
     */
    bool createShader();
    
    // Graphics API
    std::shared_ptr<IGraphicsAPI> m_graphicsAPI;
    
    // Shader manager
    std::shared_ptr<ShaderManager> m_shaderManager;
    
    // Batches
    std::vector<SpriteBatch> m_batches;
    std::unordered_map<std::shared_ptr<Texture>, SpriteBatch*> m_batchMap;
    
    // Current batch
    SpriteBatch* m_currentBatch;
    
    // Frustum culling
    FrustumCuller m_frustumCuller;
    const Camera* m_camera;
    
    // Memory pools for optimization
    Core::MemoryPool<float> m_vertexPool;
    Core::MemoryPool<uint16_t> m_indexPool;
    
    // White texture for drawing shapes
    std::shared_ptr<Texture> m_whiteTexture;
    
    // Matrices
    float m_projectionMatrix[16];
    float m_viewMatrix[16];
    
    // Shader name
    std::string m_shaderName;
    
    // Batch settings (increased for better performance)
    static const int MAX_SPRITES_PER_BATCH = 2000;
    static const int VERTICES_PER_SPRITE = 4;
    static const int INDICES_PER_SPRITE = 6;
    static const int VERTEX_SIZE = 9; // 3 position + 4 color + 2 texcoord
    
    // Rendering state
    bool m_isDrawing;
};

} // namespace Graphics
} // namespace RPGEngine