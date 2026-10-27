#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

typedef struct 
{
    char* key;
    unsigned char* pixels;
} TexLayer;

typedef struct 
{
    TexLayer* layers;
    int count;
    int capacity;
    GLuint glTexture;
} TexArray;

static TexArray s_arrays[2];

static int findLayer(const TexArray* a, const char* key)
{
    for (int i = 0; i < a->count; i++)
    {
        if (strcmp(a->layers[i].key, key) == 0) {return i;}
    }

    return -1;
}

static int addPixels(TexArray* a, const char* key, const unsigned char* decoded, int w, int h, int srgb)
{
    if (a->count == a->capacity)
    {
        int cap = a->capacity ? a->capacity * 2 : 8;
        TexLayer* grown = (TexLayer*)realloc(a->layers, sizeof(TexLayer) * cap);
        if (!grown) {return -1;}

        a->layers = grown;
        a->capacity = cap;
    }

    size_t bytes = (size_t)TEX_RES * TEX_RES * 4;
    unsigned char* out = (unsigned char*)malloc(bytes);
    char* keyCopy = (char*)malloc(strlen(key) + 1);

    if (!out || !keyCopy) {free(out); free(keyCopy); return -1;}

    if (w == TEX_RES && h == TEX_RES) {memcpy(out, decoded, bytes);}
    else if (srgb) {stbir_resize_uint8_srgb(decoded, w, h, 0, out, TEX_RES, TEX_RES, 0, STBIR_RGBA);}
    else {stbir_resize_uint8_linear(decoded, w, h, 0, out, TEX_RES, TEX_RES, 0, STBIR_RGBA);}

    strcpy(keyCopy, key);
    a->layers[a->count].key = keyCopy;
    a->layers[a->count].pixels = out;

    return a->count++;
}

int texAddFile(const char* path, int srgb)
{
    TexArray* a = &s_arrays[srgb ? 1 : 0];

    int existing = findLayer(a, path);
    if (existing >= 0) {return existing;}

    int w, h, n;
    unsigned char* decoded = stbi_load(path, &w, &h, &n, 4);

    if (!decoded)
    {
        fprintf(stderr, "Could not load texture %s: %s\n", path, stbi_failure_reason());
        return -1;
    }

    int layer = addPixels(a, path, decoded, w, h, srgb);
    stbi_image_free(decoded);

    return layer;
}

int texAddMemory(const char* key, const unsigned char* bytes, size_t size, int srgb)
{
    TexArray* a = &s_arrays[srgb ? 1 : 0];

    int existing = findLayer(a, key);
    if (existing >= 0) {return existing;}

    int w, h, n;
    unsigned char* decoded = stbi_load_from_memory(bytes, (int)size, &w, &h, &n, 4);

    if (!decoded)
    {
        fprintf(stderr, "Could not decode embedded texture %s: %s\n", key, stbi_failure_reason());
        return -1;
    }

    int layer = addPixels(a, key, decoded, w, h, srgb);
    stbi_image_free(decoded);

    return layer;
}

void texUpload(void)
{
    for (int k = 0; k < 2; k++)
    {
        TexArray* a = &s_arrays[k];
        if (a->count == 0) {continue;}

        if (a->glTexture) {glDeleteTextures(1, &a->glTexture);}

        glGenTextures(1, &a->glTexture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, a->glTexture);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, k ? GL_SRGB8_ALPHA8 : GL_RGBA8, TEX_RES, TEX_RES, a->count);

        for (int i = 0; i < a->count; i++)
        {
            if (!a->layers[i].pixels) {continue;}

            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, TEX_RES, TEX_RES, 1,
                            GL_RGBA, GL_UNSIGNED_BYTE, a->layers[i].pixels);

            free(a->layers[i].pixels);
            a->layers[i].pixels = NULL;
        }

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

        printf("Texture array (%s): %d layers @ %dx%d\n", k ? "sRGB" : "linear", a->count, TEX_RES, TEX_RES);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void texBind(void)
{
    glActiveTexture(GL_TEXTURE0 + TEX_UNIT_LINEAR);
    glBindTexture(GL_TEXTURE_2D_ARRAY, s_arrays[0].glTexture);

    glActiveTexture(GL_TEXTURE0 + TEX_UNIT_SRGB);
    glBindTexture(GL_TEXTURE_2D_ARRAY, s_arrays[1].glTexture);

    glActiveTexture(GL_TEXTURE0);
}

void texFree(void)
{
    for (int k = 0; k < 2; k++)
    {
        TexArray* a = &s_arrays[k];

        for (int i = 0; i < a->count; i++)
        {
            free(a->layers[i].key);
            free(a->layers[i].pixels);
        }

        free(a->layers);
        if (a->glTexture) {glDeleteTextures(1, &a->glTexture);}

        memset(a, 0, sizeof(TexArray));
    }
}