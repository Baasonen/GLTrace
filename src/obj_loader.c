#include "obj_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "matrix.h"

void freeMeshData(MeshData* mesh)
{
    if (mesh->vertices) free(mesh->vertices);
    if (mesh->indices) free(mesh->indices);

    mesh->vertices = NULL;
    mesh->indices = NULL;
    mesh->vertexCount = 0;
    mesh->indexCount = 0;
}

void calculateMeshNormals(MeshData* mesh)
{
    for (int i = 0; i < mesh->vertexCount; i++)
    {
        mesh->vertices[i].nx = 0.0f;
        mesh->vertices[i].ny = 0.0f;
        mesh->vertices[i].nz = 0.0f;
        mesh->vertices[i].padding2 = 0.0f;
    }

    for (int i = 0; i < mesh->indexCount; i += 3)
    {
        uint32_t i0 = mesh->indices[i + 0];
        uint32_t i1 = mesh->indices[i + 1];
        uint32_t i2 = mesh->indices[i + 2];

        GPUPackedVertex* v0 = &mesh->vertices[i0];
        GPUPackedVertex* v1 = &mesh->vertices[i1];
        GPUPackedVertex* v2 = &mesh->vertices[i2];

        Vec4 e1 = {v1->x - v0->x, v1->y - v0->y, v1->z - v0->z, 0.0f};
        Vec4 e2 = {v2->x - v0->x, v2->y - v0->y, v2->z - v0->z, 0.0f};

        Vec4 normal = crossProduct(e1, e2);

        v0->nx += normal.x; v0->ny += normal.y; v0->nz += normal.z;
        v1->nx += normal.x; v1->ny += normal.y; v1->nz += normal.z;
        v2->nx += normal.x; v2->ny += normal.y; v2->nz += normal.z;
    }

    for (int i = 0; i < mesh->vertexCount; i++)
    {
        Vec4 normal;

        normal.x = mesh->vertices[i].nx;
        normal.y = mesh->vertices[i].ny;
        normal.z = mesh->vertices[i].nz;
        normal.a = 0.0f;

        normalize(&normal);

        mesh->vertices[i].nx = normal.x;
        mesh->vertices[i].ny = normal.y;
        mesh->vertices[i].nz = normal.z;
    }
    printf("\nBuilt normals for %i vertices\n", mesh->vertexCount);
}

int loadObj(const char* filename, MeshData* mesh)
{
    FILE* file = fopen(filename, "r");

    if (!file)
    {
        fprintf(stderr, "Could not open OBJ file: %s\n", filename);
        return 0;
    }

    mesh->minBounds[0] = mesh->minBounds[1] = mesh->minBounds[2] = FLT_MAX;
    mesh->maxBounds[0] = mesh->maxBounds[1] = mesh->maxBounds[2] = -FLT_MAX;

    uint32_t vCount = 0;
    uint32_t tCount = 0;
    char line[1024];

    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == 'v' && (line[1] == ' ' || line[1] == '\t'))
        {
            vCount++;
        }
        else if (line[0] == 'f' && (line[1] == ' ' || line[1] == '\t'))
        {
            int verticesInFace = 0;
            char* p = line + 1;

            while (*p != '\0')
            {
                while (*p == ' ' || *p == '\t') p++;

                if (*p == '\0' || *p == '\n' || *p == '\r') break;

                verticesInFace++;

                while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') p++;
            }

            // 3 vertices -> 1 triangle
            // 4 vertices -> 2 triangles
            if (verticesInFace >= 3)
            {
                tCount += (verticesInFace - 2);
            }
        }
    }

    mesh->vertexCount = vCount;
    mesh->triangleCount = tCount;
    mesh->indexCount = tCount * 3;

    // Allocate memory
    mesh->vertices = (GPUPackedVertex*)malloc(sizeof(GPUPackedVertex) * mesh->vertexCount);
    mesh->indices = (uint32_t*)malloc(sizeof(uint32_t) * mesh->indexCount);

    if (!mesh->vertices || !mesh->indices)
    {
        fprintf(stderr, "Memory allocation failed for OBJ: %s\n", filename);
        fclose(file);
        return 0;
    }

    rewind(file);

    uint32_t vPtr = 0;
    uint32_t iPtr = 0;

    while (fgets(line, sizeof(line), file))
    {
        // Parse vertex
        if (line[0] == 'v' && line[1] == ' ')
        {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);

            mesh->vertices[vPtr].x = x;
            mesh->vertices[vPtr].y = y;
            mesh->vertices[vPtr].z = z;
            mesh->vertices[vPtr].padding = 0.0f;

            // Update bounding box
            if (x < mesh->minBounds[0]) {mesh->minBounds[0] = x;}
            if (y < mesh->minBounds[1]) {mesh->minBounds[1] = y;}
            if (z < mesh->minBounds[2]) {mesh->minBounds[2] = z;}

            if (x > mesh->maxBounds[0]) {mesh->maxBounds[0] = x;}
            if (y > mesh->maxBounds[1]) {mesh->maxBounds[1] = y;}
            if (z > mesh->maxBounds[2]) {mesh->maxBounds[2] = z;}    
            
            vPtr++;
        }

        // Parse face
        else if (line[0] == 'f' && line[1] == ' ')
        {
            unsigned int v[4];
            int count = 0;

            char* p = line + 1;
            
            while (count < 4)
            {
                while (*p == ' ' || *p == '\t') {p++;}
                if (*p == '\0' || *p == '\n' || *p == '\r') {break;}

                char* endPtr;
                v[count] = (unsigned int)strtoul(p, &endPtr, 10);

                if (endPtr == p) 
                {
                    p++;
                    continue; 
                }

                count++;
                p = endPtr; 

                while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {p++;}
            }

            if (count >= 3)
            {
                // First triangle
                mesh->indices[iPtr++] = v[0] - 1;
                mesh->indices[iPtr++] = v[1] - 1;
                mesh->indices[iPtr++] = v[2] - 1;

                if (count == 4)
                {
                    // Second triangle (if quad)
                    mesh->indices[iPtr++] = v[0] - 1;
                    mesh->indices[iPtr++] = v[2] - 1;
                    mesh->indices[iPtr++] = v[3] - 1;
                }
            }
        }
    }

    fclose(file);
    printf("Loaded %s: %d vertices, %d triangles\n", filename, mesh->vertexCount, mesh->triangleCount);
    return 1;
}
