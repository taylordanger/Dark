#include "ShaderManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace RPGEngine {
namespace Graphics {

ShaderManager::ShaderManager(std::shared_ptr<IGraphicsAPI> graphicsAPI)
    : m_graphicsAPI(graphicsAPI)
    , m_currentShader(INVALID_HANDLE)
    , m_initialized(false)
{
}

ShaderManager::~ShaderManager() {
    if (m_initialized) {
        shutdown();
    }
}

bool ShaderManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    if (!m_graphicsAPI) {
        std::cerr << "Graphics API not provided to ShaderManager" << std::endl;
        return false;
    }
    
    m_initialized = true;
    std::cout << "ShaderManager initialized" << std::endl;
    return true;
}

void ShaderManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Delete all shaders
    deleteAllShaders();
    
    m_initialized = false;
    std::cout << "ShaderManager shutdown" << std::endl;
}

bool ShaderManager::loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    if (!m_initialized) {
        std::cerr << "ShaderManager not initialized" << std::endl;
        return false;
    }
    
    // Load shader sources
    std::string vertexSource = loadShaderSource(vertexPath);
    std::string fragmentSource = loadShaderSource(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "Failed to load shader sources for " << name << std::endl;
        return false;
    }
    
    // Load shader from sources
    return loadShaderFromSource(name, vertexSource, fragmentSource);
}

bool ShaderManager::loadShaderFromSource(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource) {
    if (!m_initialized) {
        std::cerr << "ShaderManager not initialized" << std::endl;
        return false;
    }
    
    // Check if shader already exists
    if (hasShader(name)) {
        std::cerr << "Shader '" << name << "' already exists" << std::endl;
        return false;
    }
    
    // Create shaders
    ShaderHandle vertexShader = m_graphicsAPI->createShader(ShaderType::Vertex, vertexSource);
    if (vertexShader == INVALID_HANDLE) {
        std::cerr << "Failed to create vertex shader for " << name << std::endl;
        return false;
    }
    
    ShaderHandle fragmentShader = m_graphicsAPI->createShader(ShaderType::Fragment, fragmentSource);
    if (fragmentShader == INVALID_HANDLE) {
        m_graphicsAPI->deleteShader(vertexShader);
        std::cerr << "Failed to create fragment shader for " << name << std::endl;
        return false;
    }
    
    // Create shader program
    ShaderProgramHandle program = m_graphicsAPI->createShaderProgram(vertexShader, fragmentShader);
    
    // Delete shaders (they're linked into the program now)
    m_graphicsAPI->deleteShader(vertexShader);
    m_graphicsAPI->deleteShader(fragmentShader);
    
    if (program == INVALID_HANDLE) {
        std::cerr << "Failed to create shader program for " << name << std::endl;
        return false;
    }
    
    // Store shader program
    m_shaders[name] = program;
    
    std::cout << "Loaded shader '" << name << "'" << std::endl;
    return true;
}

ShaderProgramHandle ShaderManager::getShader(const std::string& name) const {
    if (!m_initialized) {
        return INVALID_HANDLE;
    }
    
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }
    
    return INVALID_HANDLE;
}

bool ShaderManager::useShader(const std::string& name) {
    if (!m_initialized) {
        return false;
    }
    
    ShaderProgramHandle handle = getShader(name);
    if (handle == INVALID_HANDLE) {
        std::cerr << "Shader '" << name << "' not found" << std::endl;
        return false;
    }
    
    m_graphicsAPI->useShaderProgram(handle);
    m_currentShader = handle;
    
    return true;
}

void ShaderManager::setUniform(const std::string& name, int value) {
    if (!m_initialized || m_currentShader == INVALID_HANDLE) {
        return;
    }
    
    m_graphicsAPI->setUniform(m_currentShader, name, value);
}

void ShaderManager::setUniform(const std::string& name, float value) {
    if (!m_initialized || m_currentShader == INVALID_HANDLE) {
        return;
    }
    
    m_graphicsAPI->setUniform(m_currentShader, name, value);
}

void ShaderManager::setUniform(const std::string& name, float x, float y) {
    if (!m_initialized || m_currentShader == INVALID_HANDLE) {
        return;
    }
    
    m_graphicsAPI->setUniform(m_currentShader, name, x, y);
}

void ShaderManager::setUniform(const std::string& name, float x, float y, float z) {
    if (!m_initialized || m_currentShader == INVALID_HANDLE) {
        return;
    }
    
    m_graphicsAPI->setUniform(m_currentShader, name, x, y, z);
}

void ShaderManager::setUniform(const std::string& name, float x, float y, float z, float w) {
    if (!m_initialized || m_currentShader == INVALID_HANDLE) {
        return;
    }
    
    m_graphicsAPI->setUniform(m_currentShader, name, x, y, z, w);
}

void ShaderManager::setUniformMatrix4(const std::string& name, const float* matrix) {
    if (!m_initialized || m_currentShader == INVALID_HANDLE || !matrix) {
        return;
    }
    
    m_graphicsAPI->setUniformMatrix4(m_currentShader, name, matrix);
}

bool ShaderManager::hasShader(const std::string& name) const {
    if (!m_initialized) {
        return false;
    }
    
    return m_shaders.find(name) != m_shaders.end();
}

bool ShaderManager::deleteShader(const std::string& name) {
    if (!m_initialized) {
        return false;
    }
    
    auto it = m_shaders.find(name);
    if (it == m_shaders.end()) {
        return false;
    }
    
    // Delete shader program
    m_graphicsAPI->deleteShaderProgram(it->second);
    
    // Remove from map
    m_shaders.erase(it);
    
    // Reset current shader if it was deleted
    if (m_currentShader == it->second) {
        m_currentShader = INVALID_HANDLE;
    }
    
    return true;
}

void ShaderManager::deleteAllShaders() {
    if (!m_initialized) {
        return;
    }
    
    // Delete all shader programs
    for (const auto& pair : m_shaders) {
        m_graphicsAPI->deleteShaderProgram(pair.second);
    }
    
    // Clear map
    m_shaders.clear();
    
    // Reset current shader
    m_currentShader = INVALID_HANDLE;
}

std::string ShaderManager::loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return buffer.str();
}

} // namespace Graphics
} // namespace RPGEngine