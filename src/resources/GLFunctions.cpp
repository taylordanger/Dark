#include "GLFunctions.h"
#include <iostream>

// Mock OpenGL functions for TextureResource
// In a real implementation, these would be provided by the graphics API

// Global texture counter for generating unique texture IDs
static unsigned int g_nextTextureId = 1;

void glGenTextures(int n, unsigned int* textures) {
    for (int i = 0; i < n; ++i) {
        textures[i] = g_nextTextureId++;
    }
}

void glBindTexture(unsigned int target, unsigned int texture) {
    // Mock implementation
}

void glTexParameteri(unsigned int target, unsigned int pname, int param) {
    // Mock implementation
}

void glTexImage2D(unsigned int target, int level, int internalformat, int width, int height, int border, unsigned int format, unsigned int type, const void* data) {
    // Mock implementation
}

void glGenerateMipmap(unsigned int target) {
    // Mock implementation
}

void glDeleteTextures(int n, const unsigned int* textures) {
    // Mock implementation
}