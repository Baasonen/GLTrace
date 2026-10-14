#ifndef MOUSE_SELECT_H
#define MOUSE_SELECT_H

#include <stdbool.h>
#include <stdint.h>

#include <bvec/bvec.h>

#include "bvh.h"
#include "obj_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {SEL_NONE = 0, SEL_SPHERE = 1, SEL_TRI = 2};

typedef struct 
{
    int type; 
    int idx;
    int matIdx;

    float t;
} SelResult;

void selSceneSet(BVHNode* nodes, uint32_t nodeCount, const MeshData* mesh);
void selSceneFree(void);

SelResult selRay(vec3 ro, vec3 rd, bool bothSides);

#ifdef __cplusplus
}
#endif

#endif