#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <stdint.h>

#include "material.h"

typedef struct 
{
    float x, y, z;
    float u;
    float nx, ny, nz;
    float v;
} GPUPackedVertex;

typedef struct 
{
    GPUPackedVertex* vertices;
    uint32_t* indices;

    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t triangleCount;

    float minBounds[3];
    float maxBounds[3];

    uint32_t* triangleMaterials;
    Material* materials;
    uint32_t materialCount;
} MeshData;

void calculateMeshNormals(MeshData* mesh);

int loadObj(const char* filename, MeshData* mesh);

void freeMeshData(MeshData* mesh);

#endif