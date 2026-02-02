#include "bvh.h"
float getSurfaceArea(float* min, float* max)
{
    float x = max[0] - min[0];
    float y = max[1] - min[1];
    float z = max[2] - min[2];

    return 2.0f * (x * y + y * z + z * x);
}

void growBounds(float* min, float* max, float* p)
{
    if (p[0] < min[0]) {min[0] = p[0];}
    if (p[1] < min[1]) {min[1] = p[1];}
    if (p[2] < min[2]) {min[2] = p[2];}

    if (p[0] > max[0]) {max[0] = p[0];}
    if (p[1] > max[1]) {max[1] = p[1];}
    if (p[2] > max[2]) {max[2] = p[2];}
}

float getAxisValue(GPUPackedVertex* v, int axis)
{
    if (axis == 0) {return v->x;}
    if (axis == 1) {return v->y;}

    return v->z;
}

void updateNodeBounds(BVH* bvh, uint32_t nodeIdx, MeshData* mesh, const uint32_t* indices)
{
    BVHNode* node = &bvh->nodes[nodeIdx];
    node->aabbMin[0] = node->aabbMin[1] = node->aabbMin[2] = FLT_MAX;
    node->aabbMax[0] = node->aabbMax[1] = node->aabbMax[2] = -FLT_MAX;
    for (int i = 0; i < node->triCount; i++)
    {
        int leafTriIdx = node->leftFirst + i;
        int indexBase = leafTriIdx * 3; // 3 indices per triangle
        for (int j = 0; j < 3; j++)
        {
            int vIdx = indices[indexBase + j];
            float x = mesh->vertices[vIdx].x;
            float y = mesh->vertices[vIdx].y;
            float z = mesh->vertices[vIdx].z;

            if (x < node->aabbMin[0]) node->aabbMin[0] = x;
            if (y < node->aabbMin[1]) node->aabbMin[1] = y;
            if (z < node->aabbMin[2]) node->aabbMin[2] = z;

            if (x > node->aabbMax[0]) node->aabbMax[0] = x;
            if (y > node->aabbMax[1]) node->aabbMax[1] = y;
            if (z > node->aabbMax[2]) node->aabbMax[2] = z;
        }
    }
}

void subdivideSAH(BVH* bvh, uint32_t nodeIdx, MeshData* mesh)
{
    BVHNode* node = &bvh->nodes[nodeIdx];
    
    if (node->triCount <= 2) {return;}

    int bestAxis = -1;
    float bestSplitPos = 0;
    float bestCost = FLT_MAX;

    float parentArea = getSurfaceArea(node->aabbMin, node->aabbMax);
    float parentCost = node->triCount * parentArea;

    for (int axis = 0; axis < 3; axis++)
    {
        float boundsMin = node->aabbMin[axis];
        float boundsMax = node->aabbMax[axis];

        if (boundsMax - boundsMin < 0.001f) {continue;}

        Bin bins[BINS];

        // Init bin
        for (int k = 0; k < BINS; k++)
        {
            bins[k].count = 0;
            bins[k].min[0] = bins[k].min[1] = bins[k].min[2] = FLT_MAX;
            bins[k].max[0] = bins[k].max[1] = bins[k].max[2] = -FLT_MAX;
        }
        // Populate bins
        float scale = BINS / (boundsMax - boundsMin);
        for (int i = 0; i < node->triCount; i++)
        {
            int triIdx = node->leftFirst + i;
            int idx0 = mesh->indices[triIdx * 3 + 0];
            int idx1 = mesh->indices[triIdx * 3 + 1];
            int idx2 = mesh->indices[triIdx * 3 + 2];

            float v0[3] = {mesh->vertices[idx0].x, mesh->vertices[idx0].y, mesh->vertices[idx0].z};
            float v1[3] = {mesh->vertices[idx1].x, mesh->vertices[idx1].y, mesh->vertices[idx1].z};
            float v2[3] = {mesh->vertices[idx2].x, mesh->vertices[idx2].y, mesh->vertices[idx2].z};

            float centroid = (v0[axis] + v1[axis] + v2[axis]) / 3.0f;
            int binIdx = (int)((centroid - boundsMin) * scale);
            if (binIdx >= BINS) {binIdx = BINS - 1;}
            if (binIdx < 0) {binIdx = 0;}

            bins[binIdx].count++;

            growBounds(bins[binIdx].min, bins[binIdx].max, v0);
            growBounds(bins[binIdx].min, bins[binIdx].max, v1);
            growBounds(bins[binIdx].min, bins[binIdx].max, v2);
        }

        // Eval split planes
        float leftArea[BINS - 1], rightArea[BINS - 1];
        int leftCount[BINS - 1], rightCount[BINS - 1];

        float currentMin[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
        float currentMax[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        int currentCount = 0;

        for (int i = 0; i < BINS - 1; i++)
        {
            currentCount += bins[i].count;
            if (bins[i].count > 0)
            {
                growBounds(currentMin, currentMax, bins[i].min);
                growBounds(currentMin, currentMax, bins[i].max);
            }
            leftArea[i] = getSurfaceArea(currentMin, currentMax);
            leftCount[i] = currentCount;
        }
        currentMin[0] = currentMin[1] = currentMin[2] = FLT_MAX;
        currentMax[0] = currentMax[1] = currentMax[2] = -FLT_MAX;
        currentCount = 0;

        for (int i = BINS - 1; i > 0; i--)
        {
            currentCount += bins[i].count;
            if (bins[i].count > 0)
            {
                growBounds(currentMin, currentMax, bins[i].min);
                growBounds(currentMin, currentMax, bins[i].max);
            }

            rightArea[i - 1] = getSurfaceArea(currentMin, currentMax);
            rightCount[i - 1] = currentCount;
        }
        // Find best split
        for (int i = 0; i < BINS - 1; i++)
        {
            float cost = leftArea[i] * leftCount[i] + rightArea[i] * rightCount[i];

            if (cost < bestCost)
            {
                bestCost = cost;
                bestAxis = axis;
                bestSplitPos = boundsMin + (i + 1) * (boundsMax - boundsMin) / BINS;
            }
        }
    }
    // Create children, only split if cost lower than parent
    if (bestCost >= parentCost) {return;}
    int i = node->leftFirst;
    int j = i + node->triCount - 1;
    while (i <= j)
    {
        int idx0 = mesh->indices[i * 3 + 0];
        int idx1 = mesh->indices[i * 3 + 1];
        int idx2 = mesh->indices[i * 3 + 2];

        float v0 = getAxisValue(&mesh->vertices[idx0], bestAxis);
        float v1 = getAxisValue(&mesh->vertices[idx1], bestAxis);
        float v2 = getAxisValue(&mesh->vertices[idx2], bestAxis);

        float centroid = (v0 + v1 + v2) / 3.0f;

        if (centroid < bestSplitPos) 
        {
            i++;
        }
        else
        {
            // Swap
            int temp[3] = {mesh->indices[i * 3], mesh->indices[i * 3 + 1], mesh->indices[i * 3 + 2]};

            mesh->indices[i * 3 + 0] = mesh->indices[j * 3 + 0];
            mesh->indices[i * 3 + 1] = mesh->indices[j * 3 + 1];
            mesh->indices[i * 3 + 2] = mesh->indices[j * 3 + 2];

            mesh->indices[j * 3 + 0] = temp[0];
            mesh->indices[j * 3 + 1] = temp[1];
            mesh->indices[j * 3 + 2] = temp[2];

            // Smap material ID (fixes bug with wrong materials)
            if (mesh->triangleMaterials)
            {
                uint32_t tempMaterial = mesh->triangleMaterials[i];
                mesh->triangleMaterials[i] = mesh->triangleMaterials[j];
                mesh->triangleMaterials[j] = tempMaterial;
            }

            j--;
        }
    }
    int leftCount = i - node->leftFirst;

    if (leftCount == 0 || leftCount == node->triCount) {return;}

    int leftChildIdx = bvh->nodeCount++;
    int rightChildIdx = bvh->nodeCount++;

    bvh->nodes[leftChildIdx].leftFirst = node->leftFirst;
    bvh->nodes[leftChildIdx].triCount = leftCount;
    bvh->nodes[rightChildIdx].leftFirst = i;
    bvh->nodes[rightChildIdx].triCount = node->triCount - leftCount;
    //printf("%i", (node->triCount - leftCount));

    node->leftFirst = leftChildIdx;
    node->triCount = 0;

    updateNodeBounds(bvh, leftChildIdx, mesh, mesh->indices);
    updateNodeBounds(bvh, rightChildIdx, mesh, mesh->indices);

    subdivideSAH(bvh, leftChildIdx, mesh);
    subdivideSAH(bvh, rightChildIdx, mesh);
}

void buildBVH(BVH* bvh, MeshData* mesh)
{
    // Max potential nodes 2 * N - 1
    bvh->nodes = malloc(sizeof(BVHNode) * mesh->triangleCount * 2);
    if (!bvh->nodes)
    {
        fprintf(stderr, "Memory allocation for BVH nodes failed");
        return;
    }
    bvh->nodeCount = 1;
    // Root node
    bvh->nodes[0].leftFirst = 0;
    bvh->nodes[0].triCount = mesh->triangleCount;
    updateNodeBounds(bvh, 0, mesh, mesh->indices);
    subdivideSAH(bvh, 0, mesh);
    printf("BVH built\n");
    analyzeBVH(bvh);
}

void getStatsRecursive(BVH* bvh, uint32_t nodeIdx, int currentDepth, BVHStats* stats)
{
    BVHNode* node = &bvh->nodes[nodeIdx];
    if (node->triCount > 0)
    {
        if (currentDepth > stats->maxDepth) {stats->maxDepth = currentDepth;}
        
        stats->leafCount++;
        stats->totalTrisInLeaves += node->triCount;

        return;
    }
    getStatsRecursive(bvh, node->leftFirst, currentDepth + 1, stats);
    getStatsRecursive(bvh, node->leftFirst + 1, currentDepth + 1, stats);
}

void analyzeBVH(BVH* bvh)
{
    BVHStats stats = {0, 0, 0};

    if (bvh->nodeCount > 0) {getStatsRecursive(bvh, 0, 1, &stats);}
    
    float avgTris = stats.leafCount > 0 ? (float)stats.totalTrisInLeaves / stats.leafCount : 0;

    size_t bvhSize = sizeof(BVHNode) * bvh->nodeCount;

    printf("Total Nodes:      %u\n", bvh->nodeCount);
    printf("Leaf Nodes:       %d\n", stats.leafCount);
    printf("Max Depth:        %d\n", stats.maxDepth);
    printf("Avg Tris/Leaf:    %.2f\n", avgTris);
    printf("BVH Size:         %.2f KB\n", bvhSize / 1024.0f);
}