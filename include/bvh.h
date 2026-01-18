#ifndef BVH_H
#define BVH_H

#include <stdint.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "obj_loader.h"

#define BINS 8

typedef struct 
{
    float min[3];
    float max[3];
} AABB;

typedef struct 
{
    float min[3];
    float max[3];
    int count;
} Bin;

typedef struct 
{
    float aabbMin[3];
    uint32_t leftFirst;
    float aabbMax[3];
    uint32_t triCount;
} BVHNode;

typedef struct 
{
    BVHNode* nodes;
    uint32_t nodeCount;
} BVH;

typedef struct 
{
    int maxDepth;
    int leafCount;
    int totalTrisInLeaves;
} BVHStats;


void updateNodeBounds(BVH* bvh, uint32_t nodeIdx, MeshData* mesh, const uint32_t* indices);

void subdivideSAH(BVH* bvh, uint32_t nodeIdx, MeshData* mesh);

void buildBVH(BVH* bvh, MeshData* mesh);

void analyzeBVH(BVH* bvh);

void getStatsRecursive(BVH* bvh, uint32_t nodeIdx, int currentDepth, BVHStats* stats);

#endif