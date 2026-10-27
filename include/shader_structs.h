#ifndef SHADER_STRUCTS_H
#define SHADER_STRUCTS_H

#include <bvec/bvec.h>

#include "obj_loader.h"
#include "plane_light.h"
#include "material.h"

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

    PlaneLight* planeLights;
    int planeLightCount;
} SceneDescription;

#endif