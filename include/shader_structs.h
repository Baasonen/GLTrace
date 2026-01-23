#ifndef SHADER_STRUCTS_H
#define SHADER_STRUCTS_H

#include "obj_loader.h"


typedef struct 
{
    unsigned int x, y, z, pad;
} Index4;

typedef struct 
{
    float x, y, z, a;
} Vec4;

typedef struct 
{
    float m[4][4];
} Mat4;


typedef struct 
{
    float x, y, z;
    float yaw, pitch;
    float focalLength;
    float padding[2];
} Camera;

typedef struct 
{
    float cr, cg, cb;
    float visibility;
    float roughness;
    float metallic;
    float emission;
    float opacity;
} Material;

typedef struct 
{
    float px, py, pz;
    float radius;
    int materialIndex;
    float padding[3];
} Sphere;

typedef struct 
{
    Vec4 pos;
    Vec4 scale;
    Vec4 rotation;
    int materialIndex;
    int meshSourceIndex;
} MeshInstance;

typedef struct 
{
    MeshData* meshSources;
    int numberOfSources;

    MeshInstance* meshInstances;
    int numberOfInstances;

    Material* materials;
    int materialCount;
} SceneDescription;

#endif