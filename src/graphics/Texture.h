#pragma once

#include "IGraphicsAPI.h"
#include <string>
#include <memory>

namespace RPGEngine {
namespace Graphics {

/**
 * Texture class
 * Manages a texture resource
 */
class Texture {
public:
    /**
     * Constructor
     * @param graphicsAPI Graphics API to use
     */
    Texture(std::shared_ptr<IGraphicsAPI> graphicsAPI);
    
    /**
     * Destructor
     */
    ~Texture();
    
    /**
     * Load texture from file
     * @param filepath Path to texture file
     * @return true if texture was loaded successfully
     */
    bool loadFromFile(const std::string& filepath);
    
    /**
     * Create texture from raw data
     * @param width Texture width
     * @param height Texture height
     * @param format Texture format
     * @param data Texture data
     * @return true if texture was created successfully
     */
    bool createFromData(int width, int height, TextureFormat format, const void* data);
    
    /**
     * Bind texture to the specified texture unit
     * @param unit Texture unit
     */
    void bind(uint32_t unit = 0) const;
    
    /**
     * Set texture filtering mode
     * @param minFilter Minification filter
     * @param magFilter Magnification filter
     */
    void setFilter(TextureFilter minFilter, TextureFilter magFilter);
    
    /**
     * Set texture wrap mode
     * @param wrapS Horizontal wrap mode
     * @param wrapT Vertical wrap mode
     */
    void setWrap(TextureWrap wrapS, TextureWrap wrapT);
    
    /**
     * Get texture width
     * @return Texture width
     */
    int getWidth() const { return m_width; }
    
    /**
     * Get texture height
     * @return Texture height
     */
    int getHeight() const { return m_height; }
    
    /**
     * Get texture handle
     * @return Texture handle
     */
    TextureHandle getHandle() const { return m_handle; }
    
    /**
     * Check if texture is valid
     * @return true if texture is valid
     */
    bool isValid() const { return m_handle != INVALID_HANDLE; }
    
private:
    /**
     * Release texture resources
     */
    void release();
    
    // Graphics API
    std::shared_ptr<IGraphicsAPI> m_graphicsAPI;
    
    // Texture properties
    TextureHandle m_handle;
    int m_width;
    int m_height;
    TextureFormat m_format;
    std::string m_filepath;
};

} // namespace Graphics
} // namespace RPGEngine