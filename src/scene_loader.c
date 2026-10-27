#include "scene_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "gltf_loader.h"

static void zeroScene(SceneDescription* scene)
{
    memset(scene, 0, sizeof(SceneDescription));
}

static int hasExtension(const char* path, const char* ext)
{
    size_t lp = strlen(path);
    size_t le = strlen(ext);
    if (lp < le) {return 0;}

    for (size_t i = 0; i < le; i++)
    {
        if (tolower((unsigned char)path[lp - le + i]) != ext[i]) {return 0;}
    }
    return 1;
}

static int loadMeshFile(const char* path, MeshData* mesh)
{
    if (hasExtension(path, ".gltf") || hasExtension(path, ".glb")) {return loadGLTF(path, mesh);}
    return loadObj(path, mesh);
}

void freeScene(SceneDescription* scene)
{
    if (!scene) {return;}

    if (scene->meshSources)
    {
        for (int i = 0; i < scene->numberOfSources; i++)
        {
            freeMeshData(&scene->meshSources[i]);
        }
        free(scene->meshSources);
    }

    free(scene->materials);
    free(scene->meshInstances);
    free(scene->spheres);
    free(scene->planeLights);

    zeroScene(scene);
}

static int appendSourceMaterials(SceneDescription* scene, MeshData* mesh)
{
    if (!mesh->materials || mesh->materialCount == 0) {return 1;}

    int base = scene->materialCount;
    Material* grown = (Material*)realloc(scene->materials, sizeof(Material) * (base + mesh->materialCount));
    if (!grown) {return 0;}

    scene->materials = grown;
    memcpy(&scene->materials[base], mesh->materials, sizeof(Material) * mesh->materialCount);
    scene->materialCount = base + (int)mesh->materialCount;

    for (uint32_t t = 0; t < mesh->triangleCount; t++) {mesh->triangleMaterials[t] += (uint32_t)base;}

    return 1;
}

int loadScene(const char* scenePath, SceneDescription* scene)
{
    FILE* file = fopen(scenePath, "r");

    if (!file) 
    {
        fprintf(stderr, "Failed to open scene file: %s\n", scenePath);
        return 0;
    }

    zeroScene(scene);

    if (fscanf(file, "%d", &scene->materialCount) != 1)
    {
        fclose(file);
        return 0;
    }

    scene->materials = calloc(scene->materialCount, sizeof(Material));

    if (!scene->materials)
    {
        freeScene(scene);
        fclose(file);
        return 0;
    }

    for (int i = 0; i < scene->materialCount; i++)
    {
        Material* m = &scene->materials[i];

        if (fscanf(
            file,
            "%f %f %f %f %f %f %f %f",
            &m->cr, &m->cg, &m->cb,
            &m->visibility,
            &m->roughness,
            &m->metallic,
            &m->emission,
            &m->opacity
        ) != 8)
        {
            fprintf(stderr, "Failed to read material %d\n", i);
            freeScene(scene);
            fclose(file);
            return 0;
        }
        m->baseTex = m->normalTex = m->mrTex = m->emissiveTex = -1;
    }

    if (fscanf(file, "%d", &scene->numberOfSources) != 1)
    {
        freeScene(scene);
        fclose(file);
        return 0;
    }

    scene->meshSources = calloc(scene->numberOfSources, sizeof(MeshData));
    if(!scene->meshSources)
    {
        freeScene(scene);
        fclose(file);
        return 0;
    }

    for (int i = 0; i < scene->numberOfSources; i++)
    {
        char path[512];
        
        if ((fscanf(file, "%511s", path) != 1) || (!loadMeshFile(path, &scene->meshSources[i])) || (!appendSourceMaterials(scene, &scene->meshSources[i])))
        {
            fprintf(stderr, "Failed to load mesh: %s\n", path);
            freeScene(scene);
            fclose(file);
            return 0;
        }
    }

    if (fscanf(file, "%d", &scene->numberOfInstances) != 1)
    {
        freeScene(scene);
        fclose(file);
        return 0;
    }

    scene->meshInstances = calloc(scene->numberOfInstances, sizeof(MeshInstance));

    if (!scene->meshInstances)
    {
        freeScene(scene);
        fclose(file);
        return 0;
    }

    for (int i = 0; i < scene->numberOfInstances; i++)
    {
        MeshInstance* inst = &scene->meshInstances[i];

        if (fscanf(
            file,
            "%f %f %f %f %f %f %f %f %f %d %d",
            &inst->pos.x, &inst->pos.y, &inst->pos.z,
            &inst->scale.x, &inst->scale.y, &inst->scale.z,
            &inst->rotation.x, &inst->rotation.y, &inst->rotation.z,
            &inst->materialIndex,
            &inst->meshSourceIndex
        ) != 11)
        {
            fprintf(stderr, "Failed to read instance %d\n", i);
            freeScene(scene);
            fclose(file);
            return 0;
        }
    }

    if (fscanf(file, "%d", &scene->sphereCount) != 1)
    {
        scene->sphereCount = 0;
        fclose(file);
        return 1;
    }

    if (scene->sphereCount > 0)
    {
        scene->spheres = calloc(scene->sphereCount, sizeof(Sphere));
        if (!scene->spheres)
        {
            freeScene(scene);
            fclose(file);
            return 0;
        }

        for (int i = 0; i < scene->sphereCount; i++)
        {
            Sphere* s = &scene->spheres[i];
            if (fscanf(file, "%f %f %f %f %d", &s->px, &s->py, &s->pz, &s->radius, &s->materialIndex) != 5)
            {
                fprintf(stderr, "Failed to read sphere %d\n", i);
                freeScene(scene);
                fclose(file);
                return 0;
            }
            s->padding[0] = 1.0f;
            s->padding[1] = 1.0f;
            s->padding[2] = 1.0f;
        }
    }

    if (fscanf(file, "%d", &scene->planeLightCount) != 1 || scene->planeLightCount <= 0)
    {
        scene->planeLightCount = 0;
    }
    else
    {
        if (scene->planeLightCount > PL_MAX) {scene->planeLightCount = PL_MAX;}
 
        scene->planeLights = calloc(scene->planeLightCount, sizeof(PlaneLight));
        if (!scene->planeLights)
        {
            freeScene(scene);
            fclose(file);
            return 0;
        }
 
        for (int i = 0; i < scene->planeLightCount; i++)
        {
            PlaneLight* l = &scene->planeLights[i];
            if (fscanf(file, "%f %f %f %f %f %f %f %f %f %f %f %f",
                       &l->px, &l->py, &l->pz,
                       &l->nx, &l->ny, &l->nz,
                       &l->width, &l->height,
                       &l->cr, &l->cg, &l->cb,
                       &l->intensity) != 12)
            {
                fprintf(stderr, "Failed to read plane light %d\n", i);
                freeScene(scene);
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return 1;
}