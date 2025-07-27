#pragma once

#include "IGraphicsAPI.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace RPGEngine {
namespace Graphics {

/**
 * Shader Manager
 * Handles shader loading, compilation, and management
 */
class ShaderManager {
public:
    /**
     * Constructor
     * @param graphicsAPI Graphics API to use
     */
    ShaderManager(std::shared_ptr<IGraphicsAPI> graphicsAPI);
    
    /**
     * Destructor
     */
    ~ShaderManager();
    
    /**
     * Initialize the shader manager
     * @return true if initialization was successful
     */
    bool initialize();
    
    /**
     * Shutdown the shader manager
     */
    void shutdown();
    
    /**
     * Load a shader from files
     * @param name Shader name
     * @param vertexPath Path to vertex shader file
     * @param fragmentPath Path to fragment shader file
     * @return true if shader was loaded successfully
     */
    bool loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    
    /**
     * Load a shader from source strings
     * @param name Shader name
     * @param vertexSource Vertex shader source
     * @param fragmentSource Fragment shader source
     * @return true if shader was loaded successfully
     */
    bool loadShaderFromSource(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
    
    /**
     * Get a shader program handle
     * @param name Shader name
     * @return Shader program handle, or INVALID_HANDLE if not found
     */
    ShaderProgramHandle getShader(const std::string& name) const;
    
    /**
     * Use a shader program
     * @param name Shader name
     * @return true if shader was found and used
     */
    bool useShader(const std::string& name);
    
    /**
     * Set a uniform value in the current shader
     * @param name Uniform name
     * @param value Uniform value
     */
    void setUniform(const std::string& name, int value);
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, float x, float y);
    void setUniform(const std::string& name, float x, float y, float z);
    void setUniform(const std::string& name, float x, float y, float z, float w);
    void setUniformMatrix4(const std::string& name, const float* matrix);
    
    /**
     * Check if a shader exists
     * @param name Shader name
     * @return true if shader exists
     */
    bool hasShader(const std::string& name) const;
    
    /**
     * Delete a shader
     * @param name Shader name
     * @return true if shader was deleted
     */
    bool deleteShader(const std::string& name);
    
    /**
     * Delete all shaders
     */
    void deleteAllShaders();
    
private:
    /**
     * Load shader source from file
     * @param filepath Path to shader file
     * @return Shader source code
     */
    std::string loadShaderSource(const std::string& filepath);
    
    // Graphics API
    std::shared_ptr<IGraphicsAPI> m_graphicsAPI;
    
    // Shader storage
    std::unordered_map<std::string, ShaderProgramHandle> m_shaders;
    
    // Current shader
    ShaderProgramHandle m_currentShader;
    
    // Initialization state
    bool m_initialized;
};

} // namespace Graphics
} // namespace RPGEngine