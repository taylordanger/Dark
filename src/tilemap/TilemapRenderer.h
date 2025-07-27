#pragma once

#include "Tilemap.h"
#include "../graphics/IGraphicsAPI.h"
#include "../systems/System.h"
#include "../graphics/Camera.h"
#include <memory>
#include <unordered_map>

namespace RPGEngine {
namespace Tilemap {

/**
 * Tilemap renderer class
 * Renders a tilemap
 */
class TilemapRenderer : public System {
public:
    /**
     * Constructor
     * @param graphics Graphics API
     */
    TilemapRenderer(std::shared_ptr<Graphics::IGraphicsAPI> graphics);
    
    /**
     * Destructor
     */
    ~TilemapRenderer();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Set the tilemap to render
     * @param tilemap Tilemap to render
     */
    void setTilemap(std::shared_ptr<Tilemap> tilemap) { m_tilemap = tilemap; }
    
    /**
     * Get the tilemap being rendered
     * @return Tilemap being rendered
     */
    std::shared_ptr<Tilemap> getTilemap() const { return m_tilemap; }
    
    /**
     * Set the camera to use for rendering
     * @param camera Camera to use
     */
    void setCamera(std::shared_ptr<Graphics::Camera> camera) { m_camera = camera; }
    
    /**
     * Get the camera being used for rendering
     * @return Camera being used
     */
    std::shared_ptr<Graphics::Camera> getCamera() const { return m_camera; }
    
    /**
     * Set whether to use frustum culling
     * @param useFrustumCulling Whether to use frustum culling
     */
    void setUseFrustumCulling(bool useFrustumCulling) { m_useFrustumCulling = useFrustumCulling; }
    
    /**
     * Check if frustum culling is enabled
     * @return true if frustum culling is enabled
     */
    bool isUsingFrustumCulling() const { return m_useFrustumCulling; }
    
    /**
     * Set whether to render tile colliders
     * @param renderColliders Whether to render tile colliders
     */
    void setRenderColliders(bool renderColliders) { m_renderColliders = renderColliders; }
    
    /**
     * Check if tile colliders are being rendered
     * @return true if tile colliders are being rendered
     */
    bool isRenderingColliders() const { return m_renderColliders; }
    
    /**
     * Set the collider color
     * @param color Collider color
     */
    void setColliderColor(uint32_t color) { m_colliderColor = color; }
    
    /**
     * Get the collider color
     * @return Collider color
     */
    uint32_t getColliderColor() const { return m_colliderColor; }
    
    /**
     * Update tile animations
     * @param deltaTime Time since last update
     */
    void updateAnimations(float deltaTime);
    
private:
    /**
     * Render a tile layer
     * @param layer Tile layer to render
     */
    void renderLayer(std::shared_ptr<TileLayer> layer);
    
    /**
     * Render a tile
     * @param x X position in tiles
     * @param y Y position in tiles
     * @param tile Tile to render
     * @param layer Layer being rendered
     */
    void renderTile(int x, int y, const Tile& tile, std::shared_ptr<TileLayer> layer);
    
    /**
     * Render tile colliders
     * @param layer Tile layer to render colliders for
     */
    void renderColliders(std::shared_ptr<TileLayer> layer);
    
    /**
     * Get the current animation frame for a tile
     * @param tileId Tile ID
     * @param tileset Tileset
     * @return Current animation frame tile ID, or the original tile ID if not animated
     */
    uint32_t getCurrentAnimationFrame(uint32_t tileId, std::shared_ptr<Tileset> tileset);
    
    // Graphics API
    std::shared_ptr<Graphics::IGraphicsAPI> m_graphics;
    
    // Tilemap
    std::shared_ptr<Tilemap> m_tilemap;
    
    // Camera
    std::shared_ptr<Graphics::Camera> m_camera;
    
    // Rendering options
    bool m_useFrustumCulling;
    bool m_renderColliders;
    uint32_t m_colliderColor;
    
    // Animation state
    struct AnimationState {
        float time;          // Current animation time
        size_t frameIndex;   // Current frame index
        
        AnimationState() : time(0.0f), frameIndex(0) {}
    };
    
    std::unordered_map<uint32_t, AnimationState> m_animationStates;
};

} // namespace Tilemap
} // namespace RPGEngine