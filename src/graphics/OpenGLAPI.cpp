#include "OpenGLAPI.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

// STB Image for texture loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace RPGEngine {
namespace Graphics {

// Static callback for GLFW errors
static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

#ifndef PLATFORM_MACOS
// Static callback for OpenGL debug messages
static void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                    GLsizei length, const GLchar* message, const void* userParam) {
    // Ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }
    
    std::string sourceStr;
    switch (source) {
        case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
    }
    
    std::string typeStr;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    }
    
    std::string severityStr;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    }
    
    std::cerr << "OpenGL Debug - " << id << " - " << sourceStr << ", " << typeStr << ", " << severityStr << ": " << message << std::endl;
}
#endif

OpenGLAPI::OpenGLAPI()
    : m_window(nullptr)
    , m_windowWidth(0)
    , m_windowHeight(0)
    , m_windowTitle("")
    , m_apiName("OpenGL")
    , m_apiVersion("")
    , m_currentProgram(0)
    , m_currentVAO(0)
    , m_currentTexture(0)
    , m_currentBlendMode(BlendMode::None)
    , m_depthTestEnabled(false)
    , m_faceCullingEnabled(false)
    , m_initialized(false)
{
}

OpenGLAPI::~OpenGLAPI() {
    if (m_initialized) {
        shutdown();
    }
}

bool OpenGLAPI::initialize(int windowWidth, int windowHeight, const std::string& windowTitle, bool fullscreen) {
    if (m_initialized) {
        std::cerr << "OpenGL API already initialized" << std::endl;
        return true;
    }
    
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    m_windowTitle = windowTitle;
    
    // Initialize GLFW
    if (!initializeGLFW()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Enable debug output in debug builds
#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    
    // Create window
    m_window = glfwCreateWindow(
        m_windowWidth,
        m_windowHeight,
        m_windowTitle.c_str(),
        fullscreen ? glfwGetPrimaryMonitor() : nullptr,
        nullptr
    );
    
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    // Make context current
    glfwMakeContextCurrent(m_window);
    
    // Initialize GLAD
    if (!initializeGLAD()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }
    
    // Set up debug callbacks
#ifdef _DEBUG
    setupDebugCallbacks();
#endif
    
    // Get OpenGL version
    m_apiVersion = std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::cout << "OpenGL Version: " << m_apiVersion << std::endl;
    
    // Set up default OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set up viewport
    glViewport(0, 0, m_windowWidth, m_windowHeight);
    
    // Enable vsync
    glfwSwapInterval(1);
    
    // Set up stb_image
    stbi_set_flip_vertically_on_load(true);
    
    m_initialized = true;
    std::cout << "OpenGL API initialized" << std::endl;
    return true;
}

void OpenGLAPI::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Clean up GLFW
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    
    glfwTerminate();
    
    m_initialized = false;
    std::cout << "OpenGL API shutdown" << std::endl;
}

void OpenGLAPI::beginFrame() {
    if (!m_initialized) {
        return;
    }
    
    // Poll events
    pollEvents();
}

void OpenGLAPI::endFrame() {
    if (!m_initialized) {
        return;
    }
    
    // Swap buffers
    glfwSwapBuffers(m_window);
}

void OpenGLAPI::clear(float r, float g, float b, float a) {
    if (!m_initialized) {
        return;
    }
    
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLAPI::setViewport(int x, int y, int width, int height) {
    if (!m_initialized) {
        return;
    }
    
    glViewport(x, y, width, height);
}

TextureHandle OpenGLAPI::createTexture(int width, int height, TextureFormat format, const void* data) {
    if (!m_initialized) {
        return INVALID_HANDLE;
    }
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Set default texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data
    GLenum glFormat = convertTextureFormat(format);
    GLenum internalFormat = (format == TextureFormat::RGBA || format == TextureFormat::BGRA) ? GL_RGBA : GL_RGB;
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return texture;
}

TextureHandle OpenGLAPI::loadTexture(const std::string& filepath) {
    if (!m_initialized) {
        return INVALID_HANDLE;
    }
    
    // Load image using stb_image
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return INVALID_HANDLE;
    }
    
    // Determine format based on channels
    TextureFormat format;
    switch (channels) {
        case 1: format = TextureFormat::RGB; break;  // Grayscale, treat as RGB
        case 3: format = TextureFormat::RGB; break;
        case 4: format = TextureFormat::RGBA; break;
        default:
            std::cerr << "Unsupported number of channels: " << channels << std::endl;
            stbi_image_free(data);
            return INVALID_HANDLE;
    }
    
    // Create texture
    TextureHandle handle = createTexture(width, height, format, data);
    
    // Free image data
    stbi_image_free(data);
    
    return handle;
}

void OpenGLAPI::deleteTexture(TextureHandle handle) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    glDeleteTextures(1, &handle);
}

void OpenGLAPI::bindTexture(TextureHandle handle, uint32_t unit) {
    if (!m_initialized) {
        return;
    }
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, handle);
    m_currentTexture = handle;
}

void OpenGLAPI::setTextureFilter(TextureHandle handle, TextureFilter minFilter, TextureFilter magFilter) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, convertTextureFilter(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, convertTextureFilter(magFilter));
}

void OpenGLAPI::setTextureWrap(TextureHandle handle, TextureWrap wrapS, TextureWrap wrapT) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, convertTextureWrap(wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, convertTextureWrap(wrapT));
}

ShaderHandle OpenGLAPI::createShader(ShaderType type, const std::string& source) {
    if (!m_initialized) {
        return INVALID_HANDLE;
    }
    
    // Determine shader type
    GLenum glType;
    switch (type) {
        case ShaderType::Vertex:   glType = GL_VERTEX_SHADER; break;
        case ShaderType::Fragment: glType = GL_FRAGMENT_SHADER; break;
        case ShaderType::Geometry: glType = GL_GEOMETRY_SHADER; break;
        case ShaderType::Compute:  
            std::cerr << "Compute shaders not supported in OpenGL 3.3" << std::endl;
            return INVALID_HANDLE;
        default:
            std::cerr << "Unsupported shader type" << std::endl;
            return INVALID_HANDLE;
    }
    
    // Create shader
    GLuint shader = glCreateShader(glType);
    
    // Set shader source
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    
    // Compile shader
    glCompileShader(shader);
    
    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return INVALID_HANDLE;
    }
    
    return shader;
}

void OpenGLAPI::deleteShader(ShaderHandle handle) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    glDeleteShader(handle);
}

ShaderProgramHandle OpenGLAPI::createShaderProgram(ShaderHandle vertexShader, ShaderHandle fragmentShader) {
    if (!m_initialized || vertexShader == INVALID_HANDLE || fragmentShader == INVALID_HANDLE) {
        return INVALID_HANDLE;
    }
    
    // Create program
    GLuint program = glCreateProgram();
    
    // Attach shaders
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    // Link program
    glLinkProgram(program);
    
    // Check for linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteProgram(program);
        return INVALID_HANDLE;
    }
    
    // Detach shaders after linking
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    
    return program;
}

void OpenGLAPI::deleteShaderProgram(ShaderProgramHandle handle) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    // Clear uniform location cache for this program
    m_uniformLocationCache.erase(handle);
    
    glDeleteProgram(handle);
    
    if (m_currentProgram == handle) {
        m_currentProgram = 0;
    }
}

void OpenGLAPI::useShaderProgram(ShaderProgramHandle handle) {
    if (!m_initialized) {
        return;
    }
    
    glUseProgram(handle);
    m_currentProgram = handle;
}

void OpenGLAPI::setUniform(ShaderProgramHandle handle, const std::string& name, int value) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    GLint location = getUniformLocation(handle, name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void OpenGLAPI::setUniform(ShaderProgramHandle handle, const std::string& name, float value) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    GLint location = getUniformLocation(handle, name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void OpenGLAPI::setUniform(ShaderProgramHandle handle, const std::string& name, float x, float y) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    GLint location = getUniformLocation(handle, name);
    if (location != -1) {
        glUniform2f(location, x, y);
    }
}

void OpenGLAPI::setUniform(ShaderProgramHandle handle, const std::string& name, float x, float y, float z) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    GLint location = getUniformLocation(handle, name);
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

void OpenGLAPI::setUniform(ShaderProgramHandle handle, const std::string& name, float x, float y, float z, float w) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    GLint location = getUniformLocation(handle, name);
    if (location != -1) {
        glUniform4f(location, x, y, z, w);
    }
}

void OpenGLAPI::setUniformMatrix4(ShaderProgramHandle handle, const std::string& name, const float* matrix) {
    if (!m_initialized || handle == INVALID_HANDLE || !matrix) {
        return;
    }
    
    GLint location = getUniformLocation(handle, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }
}

BufferHandle OpenGLAPI::createVertexBuffer(const void* data, size_t size, bool dynamic) {
    if (!m_initialized) {
        return INVALID_HANDLE;
    }
    
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    
    return buffer;
}

void OpenGLAPI::updateVertexBuffer(BufferHandle handle, const void* data, size_t size) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

void OpenGLAPI::deleteVertexBuffer(BufferHandle handle) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    glDeleteBuffers(1, &handle);
}

BufferHandle OpenGLAPI::createIndexBuffer(const void* data, size_t size, bool dynamic) {
    if (!m_initialized) {
        return INVALID_HANDLE;
    }
    
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    
    return buffer;
}

void OpenGLAPI::updateIndexBuffer(BufferHandle handle, const void* data, size_t size) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, data);
}

void OpenGLAPI::deleteIndexBuffer(BufferHandle handle) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    glDeleteBuffers(1, &handle);
}

VertexArrayHandle OpenGLAPI::createVertexArray(BufferHandle vertexBuffer, BufferHandle indexBuffer, 
                                             const std::vector<VertexAttribute>& attributes) {
    if (!m_initialized || vertexBuffer == INVALID_HANDLE) {
        return INVALID_HANDLE;
    }
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // Bind vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    
    // Bind index buffer if provided
    if (indexBuffer != INVALID_HANDLE) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    }
    
    // Set up vertex attributes
    for (const auto& attr : attributes) {
        glEnableVertexAttribArray(attr.location);
        glVertexAttribPointer(
            attr.location,
            attr.size,
            static_cast<GLenum>(attr.type),
            attr.normalized ? GL_TRUE : GL_FALSE,
            attr.stride,
            reinterpret_cast<const void*>(attr.offset)
        );
    }
    
    // Unbind VAO
    glBindVertexArray(0);
    
    return vao;
}

void OpenGLAPI::deleteVertexArray(VertexArrayHandle handle) {
    if (!m_initialized || handle == INVALID_HANDLE) {
        return;
    }
    
    glDeleteVertexArrays(1, &handle);
    
    if (m_currentVAO == handle) {
        m_currentVAO = 0;
    }
}

void OpenGLAPI::bindVertexArray(VertexArrayHandle handle) {
    if (!m_initialized) {
        return;
    }
    
    glBindVertexArray(handle);
    m_currentVAO = handle;
}

void OpenGLAPI::drawArrays(PrimitiveType type, int start, int count) {
    if (!m_initialized) {
        return;
    }
    
    glDrawArrays(convertPrimitiveType(type), start, count);
}

void OpenGLAPI::drawElements(PrimitiveType type, int count, uint32_t indexType, int offset) {
    if (!m_initialized) {
        return;
    }
    
    glDrawElements(convertPrimitiveType(type), count, indexType, reinterpret_cast<const void*>(offset));
}

void OpenGLAPI::setBlendMode(BlendMode mode) {
    if (!m_initialized || m_currentBlendMode == mode) {
        return;
    }
    
    switch (mode) {
        case BlendMode::None:
            glDisable(GL_BLEND);
            break;
            
        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
            
        case BlendMode::Additive:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
            
        case BlendMode::Multiply:
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            break;
    }
    
    m_currentBlendMode = mode;
}

void OpenGLAPI::setDepthTest(bool enable) {
    if (!m_initialized || m_depthTestEnabled == enable) {
        return;
    }
    
    if (enable) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    
    m_depthTestEnabled = enable;
}

void OpenGLAPI::setFaceCulling(bool enable) {
    if (!m_initialized || m_faceCullingEnabled == enable) {
        return;
    }
    
    if (enable) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    
    m_faceCullingEnabled = enable;
}

bool OpenGLAPI::shouldClose() const {
    if (!m_initialized || !m_window) {
        return true;
    }
    
    return glfwWindowShouldClose(m_window);
}

int OpenGLAPI::getWindowWidth() const {
    return m_windowWidth;
}

int OpenGLAPI::getWindowHeight() const {
    return m_windowHeight;
}

float OpenGLAPI::getAspectRatio() const {
    if (m_windowHeight == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight);
}

void OpenGLAPI::pollEvents() {
    if (!m_initialized) {
        return;
    }
    
    glfwPollEvents();
}

const std::string& OpenGLAPI::getAPIName() const {
    return m_apiName;
}

const std::string& OpenGLAPI::getAPIVersion() const {
    return m_apiVersion;
}

bool OpenGLAPI::initializeGLFW() {
    // Set error callback
    glfwSetErrorCallback(glfwErrorCallback);
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    return true;
}

bool OpenGLAPI::initializeGLAD() {
#ifdef PLATFORM_MACOS
    // On macOS, we use the system OpenGL headers, no GLAD needed
    return true;
#else
    // Initialize GLAD on other platforms
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    return true;
#endif
}

void OpenGLAPI::setupDebugCallbacks() {
#ifdef PLATFORM_MACOS
    // Debug callbacks not available on macOS with basic OpenGL 3.3
    std::cout << "OpenGL debug output not available on macOS" << std::endl;
#else
    // Check if debug output is available
    if (glDebugMessageCallback) {
        // Enable debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        
        // Set debug message callback
        glDebugMessageCallback(glDebugCallback, nullptr);
        
        // Control the amount of debug output
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        
        std::cout << "OpenGL debug output enabled" << std::endl;
    } else {
        std::cout << "OpenGL debug output not available" << std::endl;
    }
#endif
}

std::string OpenGLAPI::getErrorString(uint32_t error) const {
    switch (error) {
        case GL_NO_ERROR:          return "No error";
        case GL_INVALID_ENUM:      return "Invalid enum";
        case GL_INVALID_VALUE:     return "Invalid value";
        case GL_INVALID_OPERATION: return "Invalid operation";
        // Stack overflow/underflow not available in OpenGL 3.3
        // case GL_STACK_OVERFLOW:    return "Stack overflow";
        // case GL_STACK_UNDERFLOW:   return "Stack underflow";
        case GL_OUT_OF_MEMORY:     return "Out of memory";
        default:                   return "Unknown error";
    }
}

int OpenGLAPI::getUniformLocation(ShaderProgramHandle program, const std::string& name) {
    // Check if program is valid
    if (program == INVALID_HANDLE) {
        return -1;
    }
    
    // Check if location is cached
    auto& programCache = m_uniformLocationCache[program];
    auto it = programCache.find(name);
    
    if (it != programCache.end()) {
        return it->second;
    }
    
    // Get location from OpenGL
    int location = glGetUniformLocation(program, name.c_str());
    
    // Cache location
    programCache[name] = location;
    
    // Log warning if uniform not found
    if (location == -1) {
        std::cerr << "Uniform '" << name << "' not found in shader program " << program << std::endl;
    }
    
    return location;
}

uint32_t OpenGLAPI::convertPrimitiveType(PrimitiveType type) const {
    switch (type) {
        case PrimitiveType::Points:        return GL_POINTS;
        case PrimitiveType::Lines:         return GL_LINES;
        case PrimitiveType::LineStrip:     return GL_LINE_STRIP;
        case PrimitiveType::Triangles:     return GL_TRIANGLES;
        case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
        case PrimitiveType::TriangleFan:   return GL_TRIANGLE_FAN;
        default:                           return GL_TRIANGLES;
    }
}

uint32_t OpenGLAPI::convertTextureFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGB:  return GL_RGB;
        case TextureFormat::RGBA: return GL_RGBA;
        case TextureFormat::BGR:  return GL_BGR;
        case TextureFormat::BGRA: return GL_BGRA;
        default:                  return GL_RGB;
    }
}

uint32_t OpenGLAPI::convertTextureFilter(TextureFilter filter) const {
    switch (filter) {
        case TextureFilter::Nearest:       return GL_NEAREST;
        case TextureFilter::Linear:        return GL_LINEAR;
        case TextureFilter::MipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFilter::MipmapLinear:  return GL_LINEAR_MIPMAP_LINEAR;
        default:                           return GL_LINEAR;
    }
}

uint32_t OpenGLAPI::convertTextureWrap(TextureWrap wrap) const {
    switch (wrap) {
        case TextureWrap::Repeat:         return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder:  return GL_CLAMP_TO_BORDER;
        default:                          return GL_REPEAT;
    }
}

// Factory function implementation
std::shared_ptr<IGraphicsAPI> createGraphicsAPI() {
    return std::make_shared<OpenGLAPI>();
}

} // namespace Graphics
} // namespace RPGEngine