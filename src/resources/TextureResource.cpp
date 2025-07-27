#include "TextureResource.h"
#include "GLFunctions.h"
#include <iostream>

// Include stb_image for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

namespace RPGEngine {
namespace Resources {

TextureResource::TextureResource(const std::string& id, const std::string& path)
    : Resource(id, path)
    , m_width(0)
    , m_height(0)
    , m_format(0)
    , m_handle(0)
{
}

TextureResource::~TextureResource() {
    if (isLoaded()) {
        unload();
    }
}

bool TextureResource::load() {
    // Check if already loaded
    if (isLoaded()) {
        return true;
    }
    
    // Set state to loading
    setState(ResourceState::Loading);
    
    // Load image data using stb_image
    int width, height, channels;
    unsigned char* data = stbi_load(getPath().c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load texture: " << getPath() << std::endl;
        std::cerr << "Error: " << stbi_failure_reason() << std::endl;
        setState(ResourceState::Failed);
        return false;
    }
    
    // Determine format based on channels
    int format;
    switch (channels) {
        case 1: format = 0x1909; break; // GL_RED
        case 2: format = 0x190A; break; // GL_RG
        case 3: format = 0x1907; break; // GL_RGB
        case 4: format = 0x1908; break; // GL_RGBA
        default: format = 0x1907; break; // GL_RGB
    }
    
    // Generate OpenGL texture
    unsigned int handle;
    glGenTextures(1, &handle);
    glBindTexture(0x0DE1, handle); // GL_TEXTURE_2D
    
    // Set texture parameters
    glTexParameteri(0x0DE1, 0x2801, 0x2601); // GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR
    glTexParameteri(0x0DE1, 0x2800, 0x2601); // GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR
    glTexParameteri(0x0DE1, 0x2802, 0x812F); // GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE
    glTexParameteri(0x0DE1, 0x2803, 0x812F); // GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE
    
    // Upload texture data
    glTexImage2D(0x0DE1, 0, format, width, height, 0, format, 0x1401, data); // GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data
    
    // Generate mipmaps
    glGenerateMipmap(0x0DE1); // GL_TEXTURE_2D
    
    // Unbind texture
    glBindTexture(0x0DE1, 0); // GL_TEXTURE_2D
    
    // Free image data
    stbi_image_free(data);
    
    // Store texture information
    m_width = width;
    m_height = height;
    m_format = format;
    m_handle = handle;
    
    // Set state to loaded
    setState(ResourceState::Loaded);
    
    std::cout << "Loaded texture: " << getPath() << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
    
    return true;
}

void TextureResource::unload() {
    // Check if loaded
    if (!isLoaded()) {
        return;
    }
    
    // Delete OpenGL texture
    if (m_handle != 0) {
        glDeleteTextures(1, &m_handle);
        m_handle = 0;
    }
    
    // Reset texture information
    m_width = 0;
    m_height = 0;
    m_format = 0;
    
    // Set state to unloaded
    setState(ResourceState::Unloaded);
    
    std::cout << "Unloaded texture: " << getPath() << std::endl;
}

} // namespace Resources
} // namespace RPGEngine