#include "hdri.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <bvec/bvec.h>

#include "state.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

HDRI g_hdri = {0};

static void buildCDF(const float* d, int w, int h)
{
    size_t count = (size_t)(h + 1) + (size_t)h * (w + 1);
    
    float* cdf = (float*)malloc(count * sizeof(float));
    float* marg = cdf;
    float* cond = cdf + (h + 1);
    double* rSum = (double*)malloc(h * sizeof(double));

    for (int y = 0; y < h; y++)
    {
        float sinT = sinf(BVEC_PI * (y + 0.5f) / (float)h);

        float* row = cond + (size_t)y * (w + 1);
        double acc = 0.0;
        row[0] = 0.0f;

        for (int x = 0; x < w; x++)
        {
            const float* p = d + 4 * ((size_t)y * w + x);
            float lum = 0.2126f * p[0] + 0.7150f * p[1] + 0.0722f * p[2];
            
            acc += (double)(lum * sinT) + 1e-10;
            row[x + 1] = (float)acc;
        }

        for (int x = 1; x <= w; x++) {row[x] = (float)((double)row[x] / acc);}

        row[w] = 1.0f;
        rSum[y] = acc;
    }

    double tot = 0.0;
    marg[0] = 0.0f;

    for (int y = 0; y < h; y++) {tot += rSum[y]; marg[y + 1] = (float)tot;}
    for (int y = 1; y <= h; y++) {marg[y] = (float)((double)marg[y] / tot);}
    marg[h] = 1.0f;

    if (g_hdri.ssboCDF) {glDeleteBuffers(1, &g_hdri.ssboCDF);}
    glGenBuffers(1, &g_hdri.ssboCDF);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_hdri.ssboCDF);
    glBufferData(GL_SHADER_STORAGE_BUFFER, count * sizeof(float), cdf, GL_STATIC_DRAW);

    g_hdri.width = w;
    g_hdri.height = h;

    free(rSum);
    free(cdf);
}

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

    buildCDF(data, width, height);

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
    static GLint locSize = -1;

    if (cachedProgram != program)
    {
        locEnabled = glGetUniformLocation(program, "u_hdriEnabled");
        locStrength = glGetUniformLocation(program, "u_hdriStrength");
        locRotation = glGetUniformLocation(program, "u_hdriRotation");
        locSize = glGetUniformLocation(program, "u_hdriSize");
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

        glUniform2i(locSize, g_hdri.width, g_hdri.height);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, g_hdri.ssboCDF);
    }
}

void hdriShutdown(void)
{
    if (g_hdri.texture) {glDeleteTextures(1, &g_hdri.texture);}
    if (g_hdri.ssboCDF) {glDeleteBuffers(1, &g_hdri.ssboCDF);}

    g_hdri.texture = 0;
    g_hdri.ssboCDF = 0;
    g_hdri.loaded = false;
}
