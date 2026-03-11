#ifndef SCENE_BUILDER_H
#define SCENE_BUILDER_H

#include "obj_loader.h"
#include "shader_structs.h"
#include "shader.h"

MeshData buildSceneMesh(SceneDescription* scene);
void setupSceneData(SSBOS ssbo, SceneDescription* sceneDesc);

#endif