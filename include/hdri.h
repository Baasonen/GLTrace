#ifndef HDRI_H
#define HDRI_H

#include <stdbool.h>
#include <glad/glad.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    GLuint texture;

    GLuint ssboCDF;
    int width;
    int height;

    bool loaded;
} HDRI;

extern HDRI g_hdri;

int hdriLoad(const char* filename);

void hdriBindUniforms(GLuint program);

void hdriShutdown(void);

#ifdef __cplusplus
}
#endif

#endif
