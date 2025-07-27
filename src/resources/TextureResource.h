#pragma once

#include "Resource.h"
#include <string>
#include <memory>

namespace RPGEngine {
namespace Resources {

/**
 * Texture resource
 * Represents a loadable texture
 */
class TextureResource : public Resource {
public:
    /**
     * Constructor
     * @param id Resource ID
     * @param path Resource path
     */
    TextureResource(const std::string& id, const std::string& path);
    
    /**
     * Destructor
     */
    ~TextureResource();
    
    /**
     * Load the resource
     * @return true if the resource was loaded successfully
     */
    bool load() override;
    
    /**
     * Unload the resource
     */
    void unload() override;
    
    /**
     * Get the texture width
     * @return Texture width
     */
    int getWidth() const { return m_width; }
    
    /**
     * Get the texture height
     * @return Texture height
     */
    int getHeight() const { return m_height; }
    
    /**
     * Get the texture format
     * @return Texture format
     */
    int getFormat() const { return m_format; }
    
    /**
     * Get the texture handle
     * @return Texture handle
     */
    unsigned int getHandle() const { return m_handle; }
    
private:
    int m_width;           // Texture width
    int m_height;          // Texture height
    int m_format;          // Texture format
    unsigned int m_handle; // Texture handle
};

} // namespace Resources
} // namespace RPGEngine