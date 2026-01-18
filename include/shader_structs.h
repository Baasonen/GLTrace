#ifndef SHADER_STRUCTS_H
#define SHADER_STRUCTS_H

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
    unsigned int x, y, z, pad;
} Index4;

typedef struct 
{
    float x, y, z, a;
} Vec4;

#endif