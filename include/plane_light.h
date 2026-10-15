#ifndef PLANE_LIGHT_H
#define PLANE_LIGHT_H

#include <stdbool.h>

#include <glad/glad.h>
#include <bvec/bvec.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_SSBO_BIND 6
#define PL_MAX 128

typedef struct 
{
    float px, py, pz;
    float intensity;
    float nx, ny, nz;
    float width;
    float cr, cg, cb;
    float height;
} PlaneLight;

typedef struct 
{
    PlaneLight* lights;

    int count;
    int capacity;
    int selected;

    bool needSelect; // When set the next scene select aims the selected light
    
    GLuint ssbo;
} PLSystem;

extern PLSystem g_planeLights;

void plInit(void);
void plShutdown(void);

void plLoad(const PlaneLight* src, int count);

int plAdd(vec3 pos);
void plRemove(int idx);
void plFlip(int idx);
void plAim(int idx, vec3 target);
void plUpload(void);

bool plDirty(void);

void plBindUniforms(GLuint program);

void plBasis(const PlaneLight* pl, vec3* tangent, vec3* bitangent);
bool plSelect(vec3 ro, vec3 rdNormalized, float maxT, int* outIdx, float* outT);

#ifdef __cplusplus
}
#endif

#endif