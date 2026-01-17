#include "bvh.h"

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

void subdivide(BVH* bvh, uint32_t nodeIdx, MeshData* mesh)
{
    BVHNode* node = &bvh->nodes[nodeIdx];

    if (node->triCount <= 2) {return;}

    float extent[] =
    {
        node->aabbMax[0] - node->aabbMin[0],
        node->aabbMax[1] - node->aabbMin[1],
        node->aabbMax[2] - node->aabbMin[2]
    };

    int axis = 0;
    if (extent[1] > extent[0]) {axis = 1;}
    if (extent[2] > extent[axis]) {axis = 2;}

    float splitPos = node->aabbMin[axis] + extent[axis] * 0.5f;

    // Partition triangles
    int i = node->leftFirst;
    int j = i + node->triCount - 1;

    while (i <= j)
    {
        // Calculate centroid 
        int idx0 = mesh->indices[i * 3 + 0];
        int idx1 = mesh->indices[i * 3 + 1];
        int idx2 = mesh->indices[i * 3 + 2];

        float centroid = (mesh->vertices[idx0].x + mesh->vertices[idx1].x + mesh->vertices[idx2].x) / 3.0f;
        if (axis == 1) {centroid = (mesh->vertices[idx0].y + mesh->vertices[idx1].y + mesh->vertices[idx2].y) / 3.0f;}
        if (axis == 2) {centroid = (mesh->vertices[idx0].z + mesh->vertices[idx1].z + mesh->vertices[idx2].z) / 3.0f;}

        if (centroid < splitPos) 
        {
            i++;
        }
        else
        {
            // Swap triangle i and j in index buffer
            int temp[3] = {mesh->indices[i * 3], mesh->indices[i * 3 + 1], mesh->indices[i * 3 + 2]};

            mesh->indices[i * 3 + 0] = mesh->indices[j * 3 + 0];
            mesh->indices[i * 3 + 1] = mesh->indices[j * 3 + 1];
            mesh->indices[i * 3 + 2] = mesh->indices[j * 3 + 2];

            mesh->indices[j * 3 + 0] = temp[0];
            mesh->indices[j * 3 + 1] = temp[1];
            mesh->indices[j * 3 + 2] = temp[2];
            j--;
        }
    }

    int leftCount = i - node->leftFirst;
    if (leftCount == 0 || leftCount == node->triCount) {return;}

    // Create child nodes
    int leftChildIdx = bvh->nodeCount++;
    int rightChildIdx = bvh->nodeCount++;

    bvh->nodes[leftChildIdx].leftFirst = node->leftFirst;
    bvh->nodes[leftChildIdx].triCount = leftCount;

    bvh->nodes[rightChildIdx].leftFirst = i;
    bvh->nodes[rightChildIdx].triCount = node->triCount - leftCount;

    node->leftFirst = leftChildIdx;
    node->triCount = 0;

    updateNodeBounds(bvh, leftChildIdx, mesh, mesh->indices);
    updateNodeBounds(bvh, rightChildIdx, mesh, mesh->indices);

    subdivide(bvh, leftChildIdx, mesh);
    subdivide(bvh, rightChildIdx, mesh);
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
    subdivide(bvh, 0, mesh);

    printf("BVH built, nodes: %f\n", bvh->nodeCount);
}