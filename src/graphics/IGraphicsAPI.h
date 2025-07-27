#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace RPGEngine {
namespace Graphics {

/**
 * Texture format enumeration
 */
enum class TextureFormat {
    RGB,
    RGBA,
    BGR,
    BGRA
};

/**
 * Shader type enumeration
 */
enum class ShaderType {
    Vertex,
    Fragment,
    Geometry,
    Compute
};

/**
 * Primitive type enumeration
 */
enum class PrimitiveType {
    Points,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan
};

/**
 * Blend mode enumeration
 */
enum class BlendMode {
    None,
    Alpha,
    Additive,
    Multiply
};

/**
 * Texture filtering mode
 */
enum class TextureFilter {
    Nearest,
    Linear,
    MipmapNearest,
    MipmapLinear
};

/**
 * Texture wrap mode
 */
enum class TextureWrap {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

/**
 * Vertex attribute data types
 */
enum class VertexDataType : uint32_t {
    Float = 0x1406,      // GL_FLOAT
    UnsignedByte = 0x1401, // GL_UNSIGNED_BYTE
    UnsignedShort = 0x1403, // GL_UNSIGNED_SHORT
    UnsignedInt = 0x1405   // GL_UNSIGNED_INT
};

/**
 * Texture handle
 */
using TextureHandle = uint32_t;

/**
 * Shader handle
 */
using ShaderHandle = uint32_t;

/**
 * Shader program handle
 */
using ShaderProgramHandle = uint32_t;

/**
 * Buffer handle
 */
using BufferHandle = uint32_t;

/**
 * Vertex array handle
 */
using VertexArrayHandle = uint32_t;

/**
 * Framebuffer handle
 */
using FramebufferHandle = uint32_t;

/**
 * Invalid handle constant
 */
constexpr uint32_t INVALID_HANDLE = 0;

/**
 * Vertex attribute structure
 */
struct VertexAttribute {
    std::string name;
    uint32_t location;
    uint32_t size;
    VertexDataType type;
    bool normalized;
    uint32_t stride;
    uint32_t offset;
};

/**
 * Graphics API interface
 * Abstracts the underlying graphics API (OpenGL, DirectX, etc.)
 */
class IGraphicsAPI {
public:
    virtual ~IGraphicsAPI() = default;
    
    /**
     * Initialize the graphics API
     * @param windowWidth Window width
     * @param windowHeight Window height
     * @param windowTitle Window title
     * @param fullscreen Whether to use fullscreen mode
     * @return true if initialization was successful
     */
    virtual bool initialize(int windowWidth, int windowHeight, const std::string& windowTitle, bool fullscreen) = 0;
    
    /**
     * Shutdown the graphics API
     */
    virtual void shutdown() = 0;
    
    /**
     * Begin a new frame
     */
    virtual void beginFrame() = 0;
    
    /**
     * End the current frame and present it
     */
    virtual void endFrame() = 0;
    
    /**
     * Clear the screen with the specified color
     * @param r Red component (0-1)
     * @param g Green component (0-1)
     * @param b Blue component (0-1)
     * @param a Alpha component (0-1)
     */
    virtual void clear(float r, float g, float b, float a) = 0;
    
    /**
     * Set the viewport dimensions
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     */
    virtual void setViewport(int x, int y, int width, int height) = 0;
    
    /**
     * Create a texture from raw data
     * @param width Texture width
     * @param height Texture height
     * @param format Texture format
     * @param data Texture data
     * @return Texture handle
     */
    virtual TextureHandle createTexture(int width, int height, TextureFormat format, const void* data) = 0;
    
    /**
     * Load a texture from a file
     * @param filepath Path to texture file
     * @return Texture handle
     */
    virtual TextureHandle loadTexture(const std::string& filepath) = 0;
    
    /**
     * Delete a texture
     * @param handle Texture handle
     */
    virtual void deleteTexture(TextureHandle handle) = 0;
    
    /**
     * Bind a texture to the specified texture unit
     * @param handle Texture handle
     * @param unit Texture unit
     */
    virtual void bindTexture(TextureHandle handle, uint32_t unit) = 0;
    
    /**
     * Set texture filtering mode
     * @param handle Texture handle
     * @param minFilter Minification filter
     * @param magFilter Magnification filter
     */
    virtual void setTextureFilter(TextureHandle handle, TextureFilter minFilter, TextureFilter magFilter) = 0;
    
    /**
     * Set texture wrap mode
     * @param handle Texture handle
     * @param wrapS Horizontal wrap mode
     * @param wrapT Vertical wrap mode
     */
    virtual void setTextureWrap(TextureHandle handle, TextureWrap wrapS, TextureWrap wrapT) = 0;
    
    /**
     * Create a shader
     * @param type Shader type
     * @param source Shader source code
     * @return Shader handle
     */
    virtual ShaderHandle createShader(ShaderType type, const std::string& source) = 0;
    
    /**
     * Delete a shader
     * @param handle Shader handle
     */
    virtual void deleteShader(ShaderHandle handle) = 0;
    
    /**
     * Create a shader program
     * @param vertexShader Vertex shader handle
     * @param fragmentShader Fragment shader handle
     * @return Shader program handle
     */
    virtual ShaderProgramHandle createShaderProgram(ShaderHandle vertexShader, ShaderHandle fragmentShader) = 0;
    
    /**
     * Delete a shader program
     * @param handle Shader program handle
     */
    virtual void deleteShaderProgram(ShaderProgramHandle handle) = 0;
    
    /**
     * Use a shader program
     * @param handle Shader program handle
     */
    virtual void useShaderProgram(ShaderProgramHandle handle) = 0;
    
    /**
     * Set a uniform value in a shader program
     * @param handle Shader program handle
     * @param name Uniform name
     * @param value Uniform value
     */
    virtual void setUniform(ShaderProgramHandle handle, const std::string& name, int value) = 0;
    virtual void setUniform(ShaderProgramHandle handle, const std::string& name, float value) = 0;
    virtual void setUniform(ShaderProgramHandle handle, const std::string& name, float x, float y) = 0;
    virtual void setUniform(ShaderProgramHandle handle, const std::string& name, float x, float y, float z) = 0;
    virtual void setUniform(ShaderProgramHandle handle, const std::string& name, float x, float y, float z, float w) = 0;
    virtual void setUniformMatrix4(ShaderProgramHandle handle, const std::string& name, const float* matrix) = 0;
    
    /**
     * Create a vertex buffer
     * @param data Buffer data
     * @param size Buffer size in bytes
     * @param dynamic Whether the buffer will be updated frequently
     * @return Buffer handle
     */
    virtual BufferHandle createVertexBuffer(const void* data, size_t size, bool dynamic) = 0;
    
    /**
     * Update a vertex buffer
     * @param handle Buffer handle
     * @param data New buffer data
     * @param size New buffer size in bytes
     */
    virtual void updateVertexBuffer(BufferHandle handle, const void* data, size_t size) = 0;
    
    /**
     * Delete a vertex buffer
     * @param handle Buffer handle
     */
    virtual void deleteVertexBuffer(BufferHandle handle) = 0;
    
    /**
     * Create an index buffer
     * @param data Buffer data
     * @param size Buffer size in bytes
     * @param dynamic Whether the buffer will be updated frequently
     * @return Buffer handle
     */
    virtual BufferHandle createIndexBuffer(const void* data, size_t size, bool dynamic) = 0;
    
    /**
     * Update an index buffer
     * @param handle Buffer handle
     * @param data New buffer data
     * @param size New buffer size in bytes
     */
    virtual void updateIndexBuffer(BufferHandle handle, const void* data, size_t size) = 0;
    
    /**
     * Delete an index buffer
     * @param handle Buffer handle
     */
    virtual void deleteIndexBuffer(BufferHandle handle) = 0;
    
    /**
     * Create a vertex array
     * @param vertexBuffer Vertex buffer handle
     * @param indexBuffer Index buffer handle
     * @param attributes Vertex attributes
     * @return Vertex array handle
     */
    virtual VertexArrayHandle createVertexArray(BufferHandle vertexBuffer, BufferHandle indexBuffer, 
                                               const std::vector<VertexAttribute>& attributes) = 0;
    
    /**
     * Delete a vertex array
     * @param handle Vertex array handle
     */
    virtual void deleteVertexArray(VertexArrayHandle handle) = 0;
    
    /**
     * Bind a vertex array
     * @param handle Vertex array handle
     */
    virtual void bindVertexArray(VertexArrayHandle handle) = 0;
    
    /**
     * Draw primitives
     * @param type Primitive type
     * @param start Start index
     * @param count Number of vertices
     */
    virtual void drawArrays(PrimitiveType type, int start, int count) = 0;
    
    /**
     * Draw indexed primitives
     * @param type Primitive type
     * @param count Number of indices
     * @param indexType Type of indices
     * @param offset Offset in the index buffer
     */
    virtual void drawElements(PrimitiveType type, int count, uint32_t indexType, int offset) = 0;
    
    /**
     * Set the blend mode
     * @param mode Blend mode
     */
    virtual void setBlendMode(BlendMode mode) = 0;
    
    /**
     * Enable or disable depth testing
     * @param enable Whether to enable depth testing
     */
    virtual void setDepthTest(bool enable) = 0;
    
    /**
     * Enable or disable face culling
     * @param enable Whether to enable face culling
     */
    virtual void setFaceCulling(bool enable) = 0;
    
    /**
     * Check if the window should close
     * @return true if the window should close
     */
    virtual bool shouldClose() const = 0;
    
    /**
     * Get the window width
     * @return Window width
     */
    virtual int getWindowWidth() const = 0;
    
    /**
     * Get the window height
     * @return Window height
     */
    virtual int getWindowHeight() const = 0;
    
    /**
     * Get the aspect ratio
     * @return Aspect ratio (width / height)
     */
    virtual float getAspectRatio() const = 0;
    
    /**
     * Poll window events
     */
    virtual void pollEvents() = 0;
    
    /**
     * Get the name of the graphics API
     * @return Graphics API name
     */
    virtual const std::string& getAPIName() const = 0;
    
    /**
     * Get the version of the graphics API
     * @return Graphics API version
     */
    virtual const std::string& getAPIVersion() const = 0;
};

/**
 * Create a graphics API instance
 * @return Shared pointer to graphics API
 */
std::shared_ptr<IGraphicsAPI> createGraphicsAPI();

} // namespace Graphics
} // namespace RPGEngine