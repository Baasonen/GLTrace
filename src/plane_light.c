#include "plane_light.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

PLSystem g_planeLights = {0};

static bool sDirty = false;
static GLuint sLocProgram = 0;
static GLint sLocCount = -1;

static void setDefaults(PlaneLight* pl, vec3 pos)
{
    pl->px = pos.x;
    pl->py = pos.y;
    pl->pz = pos.z;

    pl->nx = 0.0f;
    pl->ny = -1.0f;
    pl->nz = 0.0f;

    pl->intensity = 10.0f;

    pl->width = 50.0f;
    pl->height = 50.0f;

    pl->cr = 1.0f;
    pl->cg = 1.0f;
    pl->cb = 1.0f;
}

static void normalizeNormal(PlaneLight* pl)
{
    vec3 n = vec3Make(pl->nx, pl->ny, pl->nz);

    if (vec3LengthSq(n) < 1e-10f) {n = vec3Make(0.0f, -1.0f, 0.0f);}
    else {n = vec3Normalize(n);}

    pl->nx = n.x;
    pl->ny = n.y;
    pl->nz = n.z;
}

static bool ensureCapacity(int needed)
{
    if (needed <= g_planeLights.capacity) {return true;}

    int new = g_planeLights.capacity ? g_planeLights.capacity * 2 : 4;
    while (new < needed) {new *= 2;}

    PlaneLight* p = (PlaneLight*)realloc(g_planeLights.lights, sizeof(PlaneLight) * new);
    if (!p) {return false;}
    
    g_planeLights.lights = p;
    g_planeLights.capacity = new;

    return true;
}

void plInit(void)
{
    glGenBuffers(1, &g_planeLights.ssbo);

    g_planeLights.count = 0;
    g_planeLights.selected = 0;
    g_planeLights.needSelect = false;

    plUpload();
}

void plShutdown(void)
{
    free(g_planeLights.lights);

    glDeleteBuffers(1, &g_planeLights.ssbo);

    g_planeLights = (PLSystem){0};
}

void plUpload(void)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_planeLights.ssbo);

    if (g_planeLights.count > 0)
    {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PlaneLight) * g_planeLights.count, g_planeLights.lights, GL_DYNAMIC_DRAW);
    }
    else
    {
        // Keep binding valid by never emptying buffer
        PlaneLight placeholder;
        memset(&placeholder, 0, sizeof(placeholder));
        
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PlaneLight), &placeholder, GL_DYNAMIC_DRAW);
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PL_SSBO_BIND, g_planeLights.ssbo);
    sDirty = true;
}

void plLoad(const PlaneLight* src, int count)
{
    if (count < 0) {count = 0;}
    if (count > PL_MAX) {count = PL_MAX;}

    g_planeLights.count = 0;

    if (count > 0 && src && ensureCapacity(count))
    {
        memcpy(g_planeLights.lights, src, sizeof(PlaneLight) * count);
        g_planeLights.count = count;

        for (int i = 0; i < count; i++)
        {
            normalizeNormal(&g_planeLights.lights[i]);
        }
    }

    g_planeLights.selected = 0;
    plUpload();
}

int plAdd(vec3 pos)
{
    if (g_planeLights.count >= PL_MAX) {return -1;}
    if (!ensureCapacity(g_planeLights.count + 1)) {return -1;}

    int idx = g_planeLights.count++;
    setDefaults(&g_planeLights.lights[idx], pos);

    g_planeLights.selected = idx;
    plUpload();

    return idx;
}

void plRemove(int idx)
{
    if (idx < 0 || idx >= g_planeLights.count) {return;}

    for (int i = idx; i < g_planeLights.count - 1; i++)
    {
        g_planeLights.lights[i] = g_planeLights.lights[i + 1];
    }

    g_planeLights.count--;

    if (g_planeLights.selected >= g_planeLights.count) {g_planeLights.selected = g_planeLights.count - 1;}
    if (g_planeLights.selected < 0) {g_planeLights.selected = 0;}

    plUpload();
}

void plFlip(int idx)
{
    if (idx < 0 || idx >= g_planeLights.count) {return;}

    PlaneLight* pl = &g_planeLights.lights[idx];

    pl->nx = -pl->nx;
    pl->ny = -pl->ny;
    pl->nz = -pl->nz;

    plUpload();
}

void plAim(int idx, vec3 target)
{
    if (idx < 0 || idx >= g_planeLights.count) {return;}

    PlaneLight* pl = &g_planeLights.lights[idx];
    vec3 d = vec3Sub(target, vec3Make(pl->px, pl->py, pl->pz));

    if (vec3LengthSq(d) < 1e-10f) {return;}

    d = vec3Normalize(d);

    pl->nx = d.x;
    pl->ny = d.y;
    pl->nz = d.z;

    plUpload();
}

bool plDirty(void)
{
    bool d = sDirty;
    sDirty = false;

    return d;
}

void plBindUniforms(GLuint program)
{
    if (program != sLocProgram)
    {
        sLocCount = glGetUniformLocation(program, "u_planeLightCount");
        sLocProgram = program;
    }

    glUniform1i(sLocCount, g_planeLights.count);
}

// Keep identical to hitPlaneLight() in rt.comp
void plBasis(const PlaneLight* pl, vec3* tangent, vec3* bitangent)
{
    vec3 n = vec3Make(pl->nx, pl->ny, pl->nz);
    vec3 up = (fabsf(n.y) < 0.999f) ? vec3Make(0.0f, 1.0f, 0.0f) : vec3Make(1.0f, 0.0f, 0.0f);
 
    vec3 t = vec3Normalize(vec3Cross(up, n));
 
    *tangent = t;
    *bitangent = vec3Cross(n, t);
}

bool plSelect(vec3 ro, vec3 rd, float maxT, int* outIdx, float* outT)
{
    float best = maxT;
    int bestIdx = -1;

    for (int i = 0; i < g_planeLights.count; i++)
    {
        const PlaneLight* pl = &g_planeLights.lights[i];

        vec3 n = vec3Make(pl->nx, pl->ny, pl->nz);
        vec3 pos = vec3Make(pl->px, pl->py, pl->pz);

        float denom = vec3Dot(n, rd);
        if (fabsf(denom) < 1e-6f) {continue;}

        float t = vec3Dot(vec3Sub(pos, ro), n) / denom;
        if (t <= 0.001f || t >= best) {continue;}

        vec3 hit = vec3Make(ro.x + rd.x * t, ro.y + rd.y * t, ro.z + rd.z * t);
        vec3 p = vec3Sub(hit, pos);

        vec3 tan, bitan;
        plBasis(pl, &tan, &bitan);

        if (fabsf(vec3Dot(p, tan)) > 0.5f * pl->width) {continue;}
        if (fabsf(vec3Dot(p, bitan)) > 0.5f * pl->height) {continue;}

        best = t;
        bestIdx = i;
    }

    if (bestIdx < 0) {return false;}

    if (outIdx) {*outIdx = bestIdx;}
    if (outT) {*outT = best;}

    return true;
}