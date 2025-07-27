#pragma once

#include "IGraphicsAPI.h"
#include <unordered_map>
#include <string>

// Include OpenGL headers
#ifdef PLATFORM_MACOS
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
#else
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
#endif

namespace RPGEngine {
namespace Graphics {

/**
 * OpenGL implementation of the graphics API
 */
class OpenGLAPI : public IGraphicsAPI {
public:
    /**
     * Constructor
     */
    OpenGLAPI();
    
    /**
     * Destructor
     */
    ~OpenGLAPI();
    
    // IGraphicsAPI implementation
    bool initialize(int windowWidth, int windowHeight, const std::string& windowTitle, bool fullscreen) override;
    void shutdown() override;
    void beginFrame() override;
    void endFrame() override;
    void clear(float r, float g, float b, float a) override;
    void setViewport(int x, int y, int width, int height) override;
    
    TextureHandle createTexture(int width, int height, TextureFormat format, const void* data) override;
    TextureHandle loadTexture(const std::string& filepath) override;
    void deleteTexture(TextureHandle handle) override;
    void bindTexture(TextureHandle handle, uint32_t unit) override;
    void setTextureFilter(TextureHandle handle, TextureFilter minFilter, TextureFilter magFilter) override;
    void setTextureWrap(TextureHandle handle, TextureWrap wrapS, TextureWrap wrapT) override;
    
    ShaderHandle createShader(ShaderType type, const std::string& source) override;
    void deleteShader(ShaderHandle handle) override;
    ShaderProgramHandle createShaderProgram(ShaderHandle vertexShader, ShaderHandle fragmentShader) override;
    void deleteShaderProgram(ShaderProgramHandle handle) override;
    void useShaderProgram(ShaderProgramHandle handle) override;
    
    void setUniform(ShaderProgramHandle handle, const std::string& name, int value) override;
    void setUniform(ShaderProgramHandle handle, const std::string& name, float value) override;
    void setUniform(ShaderProgramHandle handle, const std::string& name, float x, float y) override;
    void setUniform(ShaderProgramHandle handle, const std::string& name, float x, float y, float z) override;
    void setUniform(ShaderProgramHandle handle, const std::string& name, float x, float y, float z, float w) override;
    void setUniformMatrix4(ShaderProgramHandle handle, const std::string& name, const float* matrix) override;
    
    BufferHandle createVertexBuffer(const void* data, size_t size, bool dynamic) override;
    void updateVertexBuffer(BufferHandle handle, const void* data, size_t size) override;
    void deleteVertexBuffer(BufferHandle handle) override;
    
    BufferHandle createIndexBuffer(const void* data, size_t size, bool dynamic) override;
    void updateIndexBuffer(BufferHandle handle, const void* data, size_t size) override;
    void deleteIndexBuffer(BufferHandle handle) override;
    
    VertexArrayHandle createVertexArray(BufferHandle vertexBuffer, BufferHandle indexBuffer, 
                                       const std::vector<VertexAttribute>& attributes) override;
    void deleteVertexArray(VertexArrayHandle handle) override;
    void bindVertexArray(VertexArrayHandle handle) override;
    
    void drawArrays(PrimitiveType type, int start, int count) override;
    void drawElements(PrimitiveType type, int count, uint32_t indexType, int offset) override;
    
    void setBlendMode(BlendMode mode) override;
    void setDepthTest(bool enable) override;
    void setFaceCulling(bool enable) override;
    
    bool shouldClose() const override;
    int getWindowWidth() const override;
    int getWindowHeight() const override;
    float getAspectRatio() const override;
    void pollEvents() override;
    
    const std::string& getAPIName() const override;
    const std::string& getAPIVersion() const override;
    
private:
    /**
     * Initialize GLFW
     * @return true if initialization was successful
     */
    bool initializeGLFW();
    
    /**
     * Initialize GLAD (OpenGL loader)
     * @return true if initialization was successful
     */
    bool initializeGLAD();
    
    /**
     * Set up debug callbacks for OpenGL
     */
    void setupDebugCallbacks();
    
    /**
     * Convert OpenGL error to string
     * @param error OpenGL error code
     * @return Error string
     */
    std::string getErrorString(uint32_t error) const;
    
    /**
     * Get uniform location from cache or query OpenGL
     * @param program Shader program handle
     * @param name Uniform name
     * @return Uniform location
     */
    int getUniformLocation(ShaderProgramHandle program, const std::string& name);
    
    /**
     * Convert primitive type to OpenGL enum
     * @param type Primitive type
     * @return OpenGL primitive type
     */
    uint32_t convertPrimitiveType(PrimitiveType type) const;
    
    /**
     * Convert texture format to OpenGL enum
     * @param format Texture format
     * @return OpenGL texture format
     */
    uint32_t convertTextureFormat(TextureFormat format) const;
    
    /**
     * Convert texture filter to OpenGL enum
     * @param filter Texture filter
     * @return OpenGL texture filter
     */
    uint32_t convertTextureFilter(TextureFilter filter) const;
    
    /**
     * Convert texture wrap to OpenGL enum
     * @param wrap Texture wrap
     * @return OpenGL texture wrap
     */
    uint32_t convertTextureWrap(TextureWrap wrap) const;
    
    // Window and context
    GLFWwindow* m_window;
    int m_windowWidth;
    int m_windowHeight;
    std::string m_windowTitle;
    
    // API information
    std::string m_apiName;
    std::string m_apiVersion;
    
    // Uniform location cache
    std::unordered_map<ShaderProgramHandle, std::unordered_map<std::string, int>> m_uniformLocationCache;
    
    // Current state
    ShaderProgramHandle m_currentProgram;
    VertexArrayHandle m_currentVAO;
    TextureHandle m_currentTexture;
    BlendMode m_currentBlendMode;
    bool m_depthTestEnabled;
    bool m_faceCullingEnabled;
    
    // Initialization state
    bool m_initialized;
};

} // namespace Graphics
} // namespace RPGEngine