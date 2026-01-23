#include "sceneloader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void zeroScene(SceneDescription* scene)
{
    memset(scene, 0, sizeof(SceneDescription));
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

    zeroScene(scene);
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
        
        if ((fscanf(file, "%511s", path) != 1) || (!loadObj(path, &scene->meshSources[i])))
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

        inst->pos.a = 1.0f;
        inst->scale.a = 1.0f;
        inst->rotation.a = 1.0f;
    }

    fclose(file);
    return 1;
}