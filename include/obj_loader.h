#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <stdint.h>

typedef struct 
{
    float x, y, z;
    float padding;
    float nx, ny, nz;
    float padding2;
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
} MeshData;

void calculateMeshNormals(MeshData* mesh);

int loadObj(const char* filename, MeshData* mesh);

void freeMeshData(MeshData* mesh);

#endif