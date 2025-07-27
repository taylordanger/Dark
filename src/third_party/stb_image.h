/*
 * stb_image.h - v2.27 - public domain image loader
 * This is a simple wrapper for the stb_image library.
 * The actual implementation is included in TextureResource.cpp.
 */

#ifndef STB_IMAGE_INCLUDE_STB_IMAGE_H
#define STB_IMAGE_INCLUDE_STB_IMAGE_H

// Standard headers
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Primary API
extern unsigned char *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
extern void stbi_image_free(void *retval_from_stbi_load);
extern char *stbi_failure_reason(void);

#ifdef __cplusplus
}
#endif

#endif // STB_IMAGE_INCLUDE_STB_IMAGE_H