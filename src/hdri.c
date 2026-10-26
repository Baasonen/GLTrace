#include "hdri.h"

#include <stdio.h>

#include "state.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

HDRI g_hdri = {0};

int hdriLoad(const char* filename)
{
    int width, height, channels;

    float* data = stbi_loadf(filename, &width, &height, &channels, 4);

    if (!data)
    {
        fprintf(stderr, "Could not load HDRI %s\n", filename);
        return 0;
    }

    for (int i = 0; i < width * height * 4; i++)
    {
        if (data[i] > 60000.0f) {data[i] = 60000.0f;}
    }

    if (g_hdri.texture) {glDeleteTextures(1, &g_hdri.texture);}

    glGenTextures(1, &g_hdri.texture);
    glBindTexture(GL_TEXTURE_2D, g_hdri.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    stbi_image_free(data);

    g_hdri.loaded = true;
    printf("Loaded HDRI %s: %dx%d\n", filename, width, height);
    return 1;
}

void hdriBindUniforms(GLuint program)
{
    static GLuint cachedProgram = 0;
    static GLint locEnabled = -1;
    static GLint locStrength = -1;
    static GLint locRotation = -1;

    if (cachedProgram != program)
    {
        locEnabled = glGetUniformLocation(program, "u_hdriEnabled");
        locStrength = glGetUniformLocation(program, "u_hdriStrength");
        locRotation = glGetUniformLocation(program, "u_hdriRotation");
        cachedProgram = program;
    }

    glUniform1i(locEnabled, (g_program.hdriEnabled && g_hdri.loaded) ? 1 : 0);
    glUniform1f(locStrength, g_program.hdriStrength);
    glUniform1f(locRotation, g_program.hdriRotation * 0.0174532925f);

    if (g_hdri.loaded)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_hdri.texture);
        glActiveTexture(GL_TEXTURE0);
    }
}

void hdriShutdown(void)
{
    if (g_hdri.texture) {glDeleteTextures(1, &g_hdri.texture);}

    g_hdri.texture = 0;
    g_hdri.loaded = false;
}
