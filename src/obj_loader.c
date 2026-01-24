#include "obj_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

void freeMeshData(MeshData* mesh)
{
    if (mesh->vertices) free(mesh->vertices);
    if (mesh->indices) free(mesh->indices);

    mesh->vertices = NULL;
    mesh->indices = NULL;
    mesh->vertexCount = 0;
    mesh->indexCount = 0;
}

int loadObj(const char* filename, MeshData* mesh)
{
    FILE* file = fopen(filename, "r");

    if (!file)
    {
        fprintf(stderr, "Could not open OBJ file: %s\n", filename);
        return 0;
    }

    // Init bounds
    mesh->minBounds[0] = mesh->minBounds[1] = mesh->minBounds[2] = FLT_MAX;
    mesh->maxBounds[0] = mesh->maxBounds[1] = mesh->maxBounds[2] = -FLT_MAX;

    uint32_t vCount = 0;
    uint32_t tCount = 0;
    char line[1024];

    // Count vertices and triangles
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

    // Parse data
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
            mesh->vertices[vPtr].padding = 1.0f;

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
            unsigned int v[4], vt[4], vn[4];
            int count = 0;

            char* p = line + 1;
            while (*p == ' ' || *p == '\t') p++;

            
            while (count < 4 && sscanf(p, "%u", &v[count]) == 1)
            {
                count++;
                while (*p != '\0' && *p != ' ' && *p != '\t') p++;
                while (*p == ' ' || *p == '\t') p++;
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
                    mesh->indices[iPtr++] = v[1] - 1;
                    mesh->indices[iPtr++] = v[2] - 1;
                }
            }
        }
    }

    fclose(file);
    printf("\nLoaded %s: %d vertices, %d triangles\n", filename, mesh->vertexCount, mesh->triangleCount);
    return 1;
}
