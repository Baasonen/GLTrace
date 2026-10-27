#ifndef MATERIAL_H
#define MATERIAL_H

typedef struct 
{
    float cr, cg, cb;

    float visibility;
    float roughness;
    float metallic;
    float emission;
    float opacity;

    int baseTex;
    int normalTex;
    int mrTex; // G: Roughness, B: Metallic
    int emissiveTex;
} Material;

#endif