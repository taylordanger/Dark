#pragma once

#include "IGraphicsAPI.h"
#include <iostream>

namespace RPGEngine {
    namespace Graphics {

        /**
         * Mock graphics API implementation for testing and platforms without graphics
         */
        class MockGraphicsAPI : public IGraphicsAPI {
        public:
            MockGraphicsAPI() = default;
            ~MockGraphicsAPI() override = default;

            // Window management
            bool initialize(int width, int height, const std::string& title, bool fullscreen) override {
                std::cout << "MockGraphicsAPI: Initialize " << width << "x" << height << " '" << title << "'" << std::endl;
                m_width = width;
                m_height = height;
                m_title = title;
                return true;
            }

            void shutdown() override {
                std::cout << "MockGraphicsAPI: Shutdown" << std::endl;
            }

            bool shouldClose() const override {
                return false; // Never close in mock mode
            }

            void swapBuffers() {
                // No-op
            }

            void pollEvents() override {
                // No-op
            }

            // Viewport and clearing
            void setViewport(int x, int y, int width, int height) override {
                std::cout << "MockGraphicsAPI: SetViewport " << x << "," << y << " " << width << "x" << height << std::endl;
            }

            void clear(float r, float g, float b, float a) override {
                // No-op
            }

            // Shader management
            ShaderHandle createShader(ShaderType type, const std::string& source) {
                static unsigned int nextId = 1;
                std::cout << "MockGraphicsAPI: CreateShader " << nextId << std::endl;
                return nextId++;
            }

            void useShader(unsigned int shaderId) override {
                std::cout << "MockGraphicsAPI: UseShader " << shaderId << std::endl;
            }

            void deleteShader(unsigned int shaderId) override {
                std::cout << "MockGraphicsAPI: DeleteShader " << shaderId << std::endl;
            }

            // Uniform management
            int getUniformLocation(unsigned int shaderId, const std::string& name) override {
                return 0; // Mock location
            }

            void setUniformMatrix4(int location, const float* matrix) override {
                // No-op
            }

            void setUniformFloat(int location, float value) override {
                // No-op
            }

            void setUniformInt(int location, int value) override {
                // No-op
            }

            void setUniformVec2(int location, float x, float y) override {
                // No-op
            }

            void setUniformVec3(int location, float x, float y, float z) override {
                // No-op
            }

            void setUniformVec4(int location, float x, float y, float z, float w) override {
                // No-op
            }

            // Texture management
            unsigned int createTexture(int width, int height, const void* data, TextureFormat format) override {
                static unsigned int nextId = 1;
                std::cout << "MockGraphicsAPI: CreateTexture " << width << "x" << height << " format=" << static_cast<int>(format) << " id=" << nextId << std::endl;
                return nextId++;
            }

            void bindTexture(unsigned int textureId, int slot) override {
                std::cout << "MockGraphicsAPI: BindTexture " << textureId << " slot=" << slot << std::endl;
            }

            void deleteTexture(unsigned int textureId) override {
                std::cout << "MockGraphicsAPI: DeleteTexture " << textureId << std::endl;
            }

            // Buffer management
            unsigned int createVertexBuffer(const void* data, size_t size, BufferUsage usage) override {
                static unsigned int nextId = 1;
                std::cout << "MockGraphicsAPI: CreateVertexBuffer size=" << size << " usage=" << static_cast<int>(usage) << " id=" << nextId << std::endl;
                return nextId++;
            }

            unsigned int createIndexBuffer(const void* data, size_t size, BufferUsage usage) override {
                static unsigned int nextId = 1;
                std::cout << "MockGraphicsAPI: CreateIndexBuffer size=" << size << " usage=" << static_cast<int>(usage) << " id=" << nextId << std::endl;
                return nextId++;
            }

            void updateBuffer(unsigned int bufferId, const void* data, size_t size, size_t offset) override {
                std::cout << "MockGraphicsAPI: UpdateBuffer " << bufferId << " size=" << size << " offset=" << offset << std::endl;
            }

            void bindVertexBuffer(unsigned int bufferId) override {
                std::cout << "MockGraphicsAPI: BindVertexBuffer " << bufferId << std::endl;
            }

            void bindIndexBuffer(unsigned int bufferId) override {
                std::cout << "MockGraphicsAPI: BindIndexBuffer " << bufferId << std::endl;
            }

            void deleteBuffer(unsigned int bufferId) override {
                std::cout << "MockGraphicsAPI: DeleteBuffer " << bufferId << std::endl;
            }

            // Vertex array management
            unsigned int createVertexArray() override {
                static unsigned int nextId = 1;
                std::cout << "MockGraphicsAPI: CreateVertexArray " << nextId << std::endl;
                return nextId++;
            }

            void bindVertexArray(unsigned int vaoId) override {
                std::cout << "MockGraphicsAPI: BindVertexArray " << vaoId << std::endl;
            }

            void setVertexAttribute(int index, int size, DataType type, bool normalized, int stride, int offset) override {
                std::cout << "MockGraphicsAPI: SetVertexAttribute " << index << " size=" << size << " type=" << static_cast<int>(type) << std::endl;
            }

            void enableVertexAttribute(int index) override {
                std::cout << "MockGraphicsAPI: EnableVertexAttribute " << index << std::endl;
            }

            void deleteVertexArray(unsigned int vaoId) override {
                std::cout << "MockGraphicsAPI: DeleteVertexArray " << vaoId << std::endl;
            }

            // Drawing
            void drawElements(PrimitiveType type, int count, DataType indexType, const void* indices) override {
                std::cout << "MockGraphicsAPI: DrawElements type=" << static_cast<int>(type) << " count=" << count << std::endl;
            }

            void drawArrays(PrimitiveType type, int first, int count) override {
                std::cout << "MockGraphicsAPI: DrawArrays type=" << static_cast<int>(type) << " first=" << first << " count=" << count << std::endl;
            }

            // State management
            void enableBlending() override {
                std::cout << "MockGraphicsAPI: EnableBlending" << std::endl;
            }

            void disableBlending() override {
                std::cout << "MockGraphicsAPI: DisableBlending" << std::endl;
            }

            void setBlendFunction(BlendFactor srcFactor, BlendFactor dstFactor) override {
                std::cout << "MockGraphicsAPI: SetBlendFunction src=" << static_cast<int>(srcFactor) << " dst=" << static_cast<int>(dstFactor) << std::endl;
            }

            void enableDepthTest() override {
                std::cout << "MockGraphicsAPI: EnableDepthTest" << std::endl;
            }

            void disableDepthTest() override {
                std::cout << "MockGraphicsAPI: DisableDepthTest" << std::endl;
            }

            void enableCulling() override {
                std::cout << "MockGraphicsAPI: EnableCulling" << std::endl;
            }

            void disableCulling() override {
                std::cout << "MockGraphicsAPI: DisableCulling" << std::endl;
            }

            // Information
            std::string getVersion() const override {
                return "MockGraphicsAPI 1.0";
            }

            std::string getRenderer() const override {
                return "Mock Renderer";
            }

            std::string getVendor() const override {
                return "RPG Engine";
            }

            // Window properties
            int getWidth() const override {
                return m_width;
            }

            int getHeight() const override {
                return m_height;
            }

        private:
            int m_width = 800;
            int m_height = 600;
            std::string m_title = "Mock Window";
        };

    } // namespace Graphics
} // namespace RPGEngine