#ifndef SHADER_STRUCTS_H
#define SHADER_STRUCTS_H

#include <bvec/bvec.h>

#include "obj_loader.h"

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
    vec3 pos;
    vec3 scale;
    vec3 rotation;
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

    Sphere* spheres;
    int sphereCount;
} SceneDescription;

#endif