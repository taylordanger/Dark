#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include "../src/graphics/IGraphicsAPI.h"
#include "../src/graphics/ShaderManager.h"

using namespace RPGEngine;
using namespace RPGEngine::Graphics;

// Vertex structure
struct Vertex {
    float x, y, z;    // Position
    float r, g, b, a; // Color
    float u, v;       // Texture coordinates
};

// Simple vertex shader
const std::string vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 vertexColor;
out vec2 texCoord;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(aPos, 1.0);
    vertexColor = aColor;
    texCoord = aTexCoord;
}
)";

// Simple fragment shader
const std::string fragmentShaderSource = R"(
#version 330 core
in vec4 vertexColor;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform bool useTexture;

void main() {
    if (useTexture) {
        FragColor = texture(texture1, texCoord) * vertexColor;
    } else {
        FragColor = vertexColor;
    }
}
)";

// Simple identity matrix
float identityMatrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

// Simple orthographic projection matrix
void createOrthoMatrix(float* matrix, float left, float right, float bottom, float top, float near, float far) {
    matrix[0] = 2.0f / (right - left);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;
    
    matrix[4] = 0.0f;
    matrix[5] = 2.0f / (top - bottom);
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;
    
    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -2.0f / (far - near);
    matrix[11] = 0.0f;
    
    matrix[12] = -(right + left) / (right - left);
    matrix[13] = -(top + bottom) / (top - bottom);
    matrix[14] = -(far + near) / (far - near);
    matrix[15] = 1.0f;
}

int main() {
    std::cout << "=== Graphics API Test ===\n" << std::endl;
    
    // Create graphics API
    auto graphicsAPI = createGraphicsAPI();
    
    // Initialize graphics API
    if (!graphicsAPI->initialize(800, 600, "RPG Engine Graphics Test", false)) {
        std::cerr << "Failed to initialize graphics API" << std::endl;
        return -1;
    }
    
    std::cout << "Graphics API: " << graphicsAPI->getAPIName() << " " << graphicsAPI->getAPIVersion() << std::endl;
    
    // Create shader manager
    ShaderManager shaderManager(graphicsAPI);
    if (!shaderManager.initialize()) {
        std::cerr << "Failed to initialize shader manager" << std::endl;
        graphicsAPI->shutdown();
        return -1;
    }
    
    // Load shader
    if (!shaderManager.loadShaderFromSource("basic", vertexShaderSource, fragmentShaderSource)) {
        std::cerr << "Failed to load basic shader" << std::endl;
        shaderManager.shutdown();
        graphicsAPI->shutdown();
        return -1;
    }
    
    // Create triangle vertices
    std::vector<Vertex> triangleVertices = {
        { -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f }, // Bottom left (red)
        {  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f }, // Bottom right (green)
        {  0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 1.0f }  // Top (blue)
    };
    
    // Create quad vertices
    std::vector<Vertex> quadVertices = {
        { -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f }, // Bottom left (white)
        {  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f }, // Bottom right (white)
        {  0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f }, // Top right (white)
        { -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f }  // Top left (white)
    };
    
    // Create quad indices
    std::vector<uint16_t> quadIndices = {
        0, 1, 2, // First triangle
        0, 2, 3  // Second triangle
    };
    
    // Create vertex buffer for triangle
    BufferHandle triangleVBO = graphicsAPI->createVertexBuffer(
        triangleVertices.data(),
        triangleVertices.size() * sizeof(Vertex),
        false
    );
    
    // Create vertex buffer for quad
    BufferHandle quadVBO = graphicsAPI->createVertexBuffer(
        quadVertices.data(),
        quadVertices.size() * sizeof(Vertex),
        false
    );
    
    // Create index buffer for quad
    BufferHandle quadIBO = graphicsAPI->createIndexBuffer(
        quadIndices.data(),
        quadIndices.size() * sizeof(uint16_t),
        false
    );
    
    // Define vertex attributes
    std::vector<VertexAttribute> attributes = {
        { "aPos",      0, 3, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, x) },
        { "aColor",    1, 4, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, r) },
        { "aTexCoord", 2, 2, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, u) }
    };
    
    // Create vertex arrays
    VertexArrayHandle triangleVAO = graphicsAPI->createVertexArray(triangleVBO, INVALID_HANDLE, attributes);
    VertexArrayHandle quadVAO = graphicsAPI->createVertexArray(quadVBO, quadIBO, attributes);
    
    // Create a simple texture (checkerboard pattern)
    const int texWidth = 8;
    const int texHeight = 8;
    uint8_t textureData[texWidth * texHeight * 4];
    
    for (int y = 0; y < texHeight; ++y) {
        for (int x = 0; x < texWidth; ++x) {
            uint8_t color = ((x + y) % 2) ? 255 : 0;
            int index = (y * texWidth + x) * 4;
            textureData[index + 0] = color;        // R
            textureData[index + 1] = color;        // G
            textureData[index + 2] = color;        // B
            textureData[index + 3] = 255;          // A
        }
    }
    
    // Create texture
    TextureHandle texture = graphicsAPI->createTexture(
        texWidth,
        texHeight,
        TextureFormat::RGBA,
        textureData
    );
    
    // Set texture filtering
    graphicsAPI->setTextureFilter(texture, TextureFilter::Nearest, TextureFilter::Nearest);
    
    // Create orthographic projection matrix
    float projectionMatrix[16];
    createOrthoMatrix(projectionMatrix, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    // Main loop
    float rotation = 0.0f;
    while (!graphicsAPI->shouldClose()) {
        // Begin frame
        graphicsAPI->beginFrame();
        
        // Clear screen
        graphicsAPI->clear(0.2f, 0.3f, 0.3f, 1.0f);
        
        // Use shader
        shaderManager.useShader("basic");
        
        // Update rotation
        rotation += 0.01f;
        if (rotation > 360.0f) {
            rotation -= 360.0f;
        }
        
        // Create rotation matrix
        float rotationMatrix[16];
        float s = std::sin(rotation);
        float c = std::cos(rotation);
        
        rotationMatrix[0] = c;
        rotationMatrix[1] = s;
        rotationMatrix[2] = 0.0f;
        rotationMatrix[3] = 0.0f;
        
        rotationMatrix[4] = -s;
        rotationMatrix[5] = c;
        rotationMatrix[6] = 0.0f;
        rotationMatrix[7] = 0.0f;
        
        rotationMatrix[8] = 0.0f;
        rotationMatrix[9] = 0.0f;
        rotationMatrix[10] = 1.0f;
        rotationMatrix[11] = 0.0f;
        
        rotationMatrix[12] = 0.0f;
        rotationMatrix[13] = 0.0f;
        rotationMatrix[14] = 0.0f;
        rotationMatrix[15] = 1.0f;
        
        // Draw triangle
        graphicsAPI->bindVertexArray(triangleVAO);
        shaderManager.setUniform("useTexture", 0);
        shaderManager.setUniformMatrix4("transform", projectionMatrix);
        graphicsAPI->drawArrays(PrimitiveType::Triangles, 0, 3);
        
        // Draw quad with texture
        graphicsAPI->bindVertexArray(quadVAO);
        graphicsAPI->bindTexture(texture, 0);
        shaderManager.setUniform("useTexture", 1);
        shaderManager.setUniform("texture1", 0);
        shaderManager.setUniformMatrix4("transform", rotationMatrix);
        graphicsAPI->drawElements(PrimitiveType::Triangles, 6, GL_UNSIGNED_SHORT, 0);
        
        // End frame
        graphicsAPI->endFrame();
    }
    
    // Clean up
    graphicsAPI->deleteTexture(texture);
    graphicsAPI->deleteVertexArray(triangleVAO);
    graphicsAPI->deleteVertexArray(quadVAO);
    graphicsAPI->deleteVertexBuffer(triangleVBO);
    graphicsAPI->deleteVertexBuffer(quadVBO);
    graphicsAPI->deleteIndexBuffer(quadIBO);
    
    shaderManager.shutdown();
    graphicsAPI->shutdown();
    
    std::cout << "\nGraphics API Test completed successfully!" << std::endl;
    return 0;
}