#ifndef SCENELOADER_H
#define SCENELOADER_H

#include "shader_structs.h"

int loadScene(const char* scenePath, SceneDescription* scene);
void freeScene(SceneDescription* scene);

#endif