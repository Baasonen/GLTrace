#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <stdint.h>

// Match std430 alignment for vec3 (16 Byte)
typedef struct 
{
    float x, y, z;
    float padding;
} GPUPackedVertex;

typedef struct 
{
    GPUPackedVertex* vertices;
    uint32_t* indices;

    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t triangleCount;

    // Bounding box
    float minBounds[3];
    float maxBounds[3];

    uint32_t* triangleMaterials;
} MeshData;

int loadObj(const char* filename, MeshData* mesh);

void freeMeshData(MeshData* mesh);

#endif