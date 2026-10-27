#ifndef TEXTURE_H
#define TEXTURE_H

#include <stddef.h>

#define TEX_RES 2048
#define TEX_UNIT_LINEAR 8
#define TEX_UNIT_SRGB 9

#ifdef __cplusplus
extern "C" {
#endif

int texAddFile(const char* path, int srgb);
int texAddMemory(const char* key, const unsigned char* bytes, size_t size, int srgb);

void texUpload(void); // Call after scene loaded, needs GL context
void texBind(void);
void texFree(void);

#ifdef __cplusplus
}
#endif

#endif