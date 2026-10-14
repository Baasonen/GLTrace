#include "mouse_select.h"

#include <math.h>
#include <stdlib.h>
#include <float.h>

#include "shader.h"

#define SEL_STACK_SIZE 128
#define SEL_MAX_T 10000.0f

static BVHNode* sNodes = NULL;
static uint32_t SNodeCount = 0;
static MeshData sMesh = {0};

void selSceneFree(void) {
  free(sNodes);

  sNodes = NULL;
  SNodeCount = 0;

  if (sMesh.vertices) {
    freeMeshData(&sMesh);
  }
  sMesh = (MeshData){0};
}

void selSceneSet(BVHNode* nodes, uint32_t nodeCount, const MeshData* mesh) {
  selSceneFree();

  sNodes = nodes;
  SNodeCount = nodeCount;
  sMesh = *mesh;
}

static vec3 getVertex(uint32_t idx) {
  const GPUPackedVertex* v = &sMesh.vertices[idx];

  return vec3Make(v->x, v->y, v->z);
}

static bool selHitAABB(const float bmin[3], const float bmax[3], vec3 ro,
                       vec3 inv, float tMax) {
  float tNear = -FLT_MAX;
  float tFar = FLT_MAX;

  for (int a = 0; a < 3; a++) {
    float t0 = (bmin[a] - ro.elements[a]) * inv.elements[a];
    float t1 = (bmax[a] - ro.elements[a]) * inv.elements[a];

    if (t0 > t1) {
      float tmp = t0;
      t0 = t1;
      t1 = tmp;
    }
    if (t0 > tNear) {
      tNear = t0;
    }
    if (t1 < tFar) {
      tFar = t1;
    }
  }

  return tFar >= tNear && tFar > 0.0f && tNear < tMax;
}

static float selHitTriangle(uint32_t tri, vec3 ro, vec3 rd, bool bothSides) {
  vec3 v0 = getVertex(sMesh.indices[3 * tri + 0]);
  vec3 v1 = getVertex(sMesh.indices[3 * tri + 1]);
  vec3 v2 = getVertex(sMesh.indices[3 * tri + 2]);

  vec3 edge1 = vec3Sub(v1, v0);
  vec3 edge2 = vec3Sub(v2, v0);

  vec3 h = vec3Cross(rd, edge2);
  float a = vec3Dot(edge1, h);

  if (bothSides) 
  {
    if (fabsf(a) < 0.0001f) {return -1.0f;}
  } 
  else 
  {
    if (a < 0.0001f) {return -1.0f;}
  }

  float f = 1.0f / a;
  vec3 s = vec3Sub(ro, v0);

  float u = f * vec3Dot(s, h);
  if (u < 0.0f || u > 1.0f) {return -1.0f;}

  vec3 q = vec3Cross(s, edge1);

  float v = f * vec3Dot(rd, q);
  if (v < 0.0f || u + v > 1.0f) {return -1.0f;}

  float t = f * vec3Dot(edge2, q);
  return (t > 0.001f) ? t : -1.0f;
}

SelResult selRay(vec3 ro, vec3 rd, bool bothSides) 
{
  SelResult res = {SEL_NONE, -1, -1, SEL_MAX_T};

  if (vec3LengthSq(rd) == 0.0f) { return res;}
  vec3 rdNorm = vec3Normalize(rd);

  vec3 inv = vec3Make(1.0f / rdNorm.x, 1.0f / rdNorm.y, 1.0f / rdNorm.z);

  const Material* mats = g_matEditor.materials;
  int matCount = g_matEditor.matCount;

  for (int i = 0; i < g_sphereEditor.sphereCount; i++)
  {
    const Sphere* sp = &g_sphereEditor.spheres[i];

    if (!mats || sp->materialIndex < 0 || sp->materialIndex >= matCount) {continue;}
    if (mats[sp->materialIndex].visibility > 0.5f) {continue;}

    vec3 oc = vec3Sub(ro, vec3Make(sp->px, sp->py, sp->pz));

    float b = vec3Dot(oc, rdNorm);
    float c = vec3Dot(oc, oc) - sp->radius * sp->radius;
    float h = b * b - c;

    if (h < 0.0f) {continue;}

    float t = -b - sqrtf(h);

    if (t > 0.001f && t < res.t)
    {
        res.type = SEL_SPHERE;
        res.idx = i;
        res.matIdx = sp->materialIndex;
        res.t = t;
    }
  }

  if (sNodes && sMesh.triangleCount > 0)
  {
    uint32_t stack[SEL_STACK_SIZE];
    int sp = 0;
    stack[sp++] = 0;

    while (sp > 0)
    {
        const BVHNode* node = &sNodes[stack[--sp]];

        if (!selHitAABB(node->aabbMin, node->aabbMax, ro, inv, res.t)) {continue;}

        if (node -> triCount > 0)
        {
            for (uint32_t i = 0; i < node->triCount; i++)
            {
                uint32_t tri = node->leftFirst + i;
                float t = selHitTriangle(tri, ro, rdNorm, bothSides);

                if (t > 0.0f && t < res.t && (mats[sMesh.triangleMaterials[tri]].visibility < 0.5f))
                {
                    res.type = SEL_TRI;
                    res.idx = (int)tri;
                    res.matIdx = (int)sMesh.triangleMaterials[tri];
                    res.t = t;
                }
            }
        }

        else if (sp + 2 <= SEL_STACK_SIZE)
        {
            stack[sp++] = node->leftFirst;
            stack[sp++] = node->leftFirst + 1;
        }
    }
  }

  return res;
}