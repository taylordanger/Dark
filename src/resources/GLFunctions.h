#pragma once

// Mock OpenGL functions for TextureResource
// In a real implementation, these would be provided by the graphics API

#ifndef GL_FUNCTIONS_H
#define GL_FUNCTIONS_H

// OpenGL function declarations
void glGenTextures(int n, unsigned int* textures);
void glBindTexture(unsigned int target, unsigned int texture);
void glTexParameteri(unsigned int target, unsigned int pname, int param);
void glTexImage2D(unsigned int target, int level, int internalformat, int width, int height, int border, unsigned int format, unsigned int type, const void* data);
void glGenerateMipmap(unsigned int target);
void glDeleteTextures(int n, const unsigned int* textures);

#endif // GL_FUNCTIONS_H