#include "scene_builder.h"

#include "bvh.h"

MeshData buildSceneMesh(SceneDescription* scene)
{
    MeshData combinedMesh = {0};

    int totalVertices = 0;
    int totalIndices = 0;

    for (int i = 0; i < scene->numberOfInstances; i++)
    {
        int srcIndex = scene->meshInstances[i].meshSourceIndex;

        if (srcIndex >= scene->numberOfSources) {continue;}

        totalVertices += scene->meshSources[srcIndex].vertexCount;
        totalIndices += scene->meshSources[srcIndex].indexCount;
    }

    combinedMesh.vertices = (GPUPackedVertex*)malloc(sizeof(GPUPackedVertex) * totalVertices);
    combinedMesh.indices = (uint32_t*)malloc(sizeof(uint32_t) * totalIndices);
    combinedMesh.triangleMaterials = (uint32_t*)malloc(sizeof(uint32_t) * (totalIndices / 3));

    combinedMesh.vertexCount = totalVertices;
    combinedMesh.indexCount = totalIndices;

    int vOffset = 0;
    int iOffset = 0;
    int tOffset = 0;
    
    for (int i = 0; i < scene->numberOfInstances; i++)
    {
        MeshInstance* instance = &scene->meshInstances[i];
        int srcIndex = instance->meshSourceIndex;

        if (srcIndex >= scene->numberOfSources) {continue;}

        MeshData* sourceMesh = &scene->meshSources[srcIndex];

        vec3 axisY = {.x = 0.0f, .y = 1.0f, .z = 0.0f};
        mat4 modelMatrix = mat4Transform(instance->pos, axisY, instance->rotation.y, instance->scale);

        for (int v = 0; v < sourceMesh->vertexCount; v++)
        {
            vec4 localPos;
            localPos.x = sourceMesh->vertices[v].x;
            localPos.y = sourceMesh->vertices[v].y;
            localPos.z = sourceMesh->vertices[v].z;
            localPos.w = 1.0f;

            vec4 worldPos = mat4MulVec4(modelMatrix, localPos);

            combinedMesh.vertices[vOffset + v].x = worldPos.x;
            combinedMesh.vertices[vOffset + v].y = worldPos.y;
            combinedMesh.vertices[vOffset + v].z = worldPos.z;
        }
        for (int idx = 0; idx < sourceMesh->indexCount; idx++)
        {
            combinedMesh.indices[iOffset + idx] = sourceMesh->indices[idx] + vOffset;
        }

        int triangleCount = sourceMesh->indexCount / 3;
        for (int t = 0; t < triangleCount; t++)
        {
            combinedMesh.triangleMaterials[tOffset + t] = instance->materialIndex;
        }

        vOffset += sourceMesh->vertexCount;
        iOffset += sourceMesh->indexCount;
        tOffset += triangleCount;
        }

    combinedMesh.triangleCount = totalIndices / 3;

    calculateMeshNormals(&combinedMesh);

    return combinedMesh;
}

void setupSceneData(SSBOS ssbo, SceneDescription* sceneDesc)
{
    MeshData sceneMesh;
    sceneMesh = buildSceneMesh(sceneDesc);

    BVH bvh;
    buildBVH(&bvh, &sceneMesh);
    // Upload vertices
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.ssboVertices);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUPackedVertex) * sceneMesh.vertexCount, sceneMesh.vertices, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo.ssboVertices);
    // Upload indices
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.ssboIndices);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * sceneMesh.indexCount, sceneMesh.indices, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo.ssboIndices);
    // Upload BVH nodes
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.ssboBVH);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVHNode) * bvh.nodeCount, bvh.nodes, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo.ssboBVH);

    // Material data for triangles
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.ssboTriangleMaterial);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * sceneMesh.triangleCount, sceneMesh.triangleMaterials, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo.ssboTriangleMaterial);

    // Materials
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.ssboMaterials);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * sceneDesc->materialCount, sceneDesc->materials, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo.ssboMaterials);

    free(bvh.nodes);
    freeMeshData(&sceneMesh);

    // Sphere data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.ssboSpheres);

    if (sceneDesc->sphereCount > 0)
    {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere) * sceneDesc->sphereCount, sceneDesc->spheres, GL_STATIC_DRAW);
    }
    else
    {
        // Allocate some memory to prevent GL errors if empty
        Sphere placeholderSphere = {0};
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere), &placeholderSphere, GL_STATIC_DRAW);
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo.ssboSpheres);
}