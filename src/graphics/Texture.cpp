#include "Texture.h"
#include <iostream>

namespace RPGEngine {
namespace Graphics {

Texture::Texture(std::shared_ptr<IGraphicsAPI> graphicsAPI)
    : m_graphicsAPI(graphicsAPI)
    , m_handle(INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_format(TextureFormat::RGBA)
    , m_filepath("")
{
}

Texture::~Texture() {
    release();
}

bool Texture::loadFromFile(const std::string& filepath) {
    if (!m_graphicsAPI) {
        std::cerr << "Graphics API not provided to Texture" << std::endl;
        return false;
    }
    
    // Release existing texture
    release();
    
    // Load texture from file
    m_handle = m_graphicsAPI->loadTexture(filepath);
    
    if (m_handle == INVALID_HANDLE) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return false;
    }
    
    // Store filepath
    m_filepath = filepath;
    
    // Set default filtering
    setFilter(TextureFilter::Linear, TextureFilter::Linear);
    
    std::cout << "Loaded texture: " << filepath << std::endl;
    return true;
}

bool Texture::createFromData(int width, int height, TextureFormat format, const void* data) {
    if (!m_graphicsAPI) {
        std::cerr << "Graphics API not provided to Texture" << std::endl;
        return false;
    }
    
    // Release existing texture
    release();
    
    // Create texture from data
    m_handle = m_graphicsAPI->createTexture(width, height, format, data);
    
    if (m_handle == INVALID_HANDLE) {
        std::cerr << "Failed to create texture" << std::endl;
        return false;
    }
    
    // Store properties
    m_width = width;
    m_height = height;
    m_format = format;
    
    // Set default filtering
    setFilter(TextureFilter::Linear, TextureFilter::Linear);
    
    return true;
}

void Texture::bind(uint32_t unit) const {
    if (!m_graphicsAPI || m_handle == INVALID_HANDLE) {
        return;
    }
    
    m_graphicsAPI->bindTexture(m_handle, unit);
}

void Texture::setFilter(TextureFilter minFilter, TextureFilter magFilter) {
    if (!m_graphicsAPI || m_handle == INVALID_HANDLE) {
        return;
    }
    
    m_graphicsAPI->setTextureFilter(m_handle, minFilter, magFilter);
}

void Texture::setWrap(TextureWrap wrapS, TextureWrap wrapT) {
    if (!m_graphicsAPI || m_handle == INVALID_HANDLE) {
        return;
    }
    
    m_graphicsAPI->setTextureWrap(m_handle, wrapS, wrapT);
}

void Texture::release() {
    if (m_graphicsAPI && m_handle != INVALID_HANDLE) {
        m_graphicsAPI->deleteTexture(m_handle);
        m_handle = INVALID_HANDLE;
    }
    
    m_width = 0;
    m_height = 0;
}

} // namespace Graphics
} // namespace RPGEngine