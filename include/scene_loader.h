#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include "shader_structs.h"

int loadScene(const char* scenePath, SceneDescription* scene);
void freeScene(SceneDescription* scene);

#endif