#include "gltf_loader.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <bvec/bvec.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "texture.h"

typedef struct 
{
    mat4 model;
    vec3 cfX, cfY, cfZ;
    float sign;
    int flipW;
} NT;

static NT makeNodeTransform(const float* m)
{
    NT t;

    t.model.a = vec4Make(m[0], m[4], m[8],  m[12]);
    t.model.b = vec4Make(m[1], m[5], m[9],  m[13]);
    t.model.c = vec4Make(m[2], m[6], m[10], m[14]);
    t.model.d = vec4Make(m[3], m[7], m[11], m[15]);

    vec3 c0 = vec3Make(t.model.a.x, t.model.b.x, t.model.c.x);
    vec3 c1 = vec3Make(t.model.a.y, t.model.b.y, t.model.c.y);
    vec3 c2 = vec3Make(t.model.a.z, t.model.b.z, t.model.c.z);

    t.cfX = vec3Cross(c1, c2);
    t.cfY = vec3Cross(c2, c0);
    t.cfZ = vec3Cross(c0, c1);

    float det = vec3Dot(c0, t.cfX);
    t.sign = det < 0.0f ? -1.0f : 1.0f;
    t.flipW = det < 0.0f; // mirrored node

    return t;
}

static vec3 transformPoint(const NT* t, vec3 p)
{
    vec4 r = mat4MulVec4(t->model, vec4Make(p.x, p.y, p.z, 1.0f));

    return vec3Make(r.x, r.y, r.z);
}

static vec3 transformNormal(const NT* t, vec3 n)
{
    vec3 r = vec3Add(vec3Add(vec3Scale(t->cfX, n.x),
                             vec3Scale(t->cfY, n.y)),
                             vec3Scale(t->cfZ, n.z));

    return vec3Normalize(vec3Scale(r, t->sign));
}

static const cgltf_accessor* findAttribute(const cgltf_primitive* prim, cgltf_attribute_type type)
{
    for (cgltf_size a = 0; a < prim->attributes_count; a++)
    {
        const cgltf_attribute* atr = &prim->attributes[a];

        if (atr->type == type && atr->index == 0) {return atr->data;}
    }

    return NULL;
}

static void gather(const cgltf_data* data, MeshData* mesh, uint32_t* vCount, uint32_t* iCount, int* allHaveNormals)
{
    uint32_t vBase = 0;
    uint32_t iBase = 0;

    for (cgltf_size n = 0; n < data->nodes_count; n++)
    {
        const cgltf_node* node = &data->nodes[n];
        if (!node->mesh) {continue;}

        float m[16];
        cgltf_node_transform_world(node, m);
        NT xf = makeNodeTransform(m);

        for (cgltf_size p = 0; p < node->mesh->primitives_count; p++)
        {
            const cgltf_primitive* prim = &node->mesh->primitives[p];
            if (prim->type != cgltf_primitive_type_triangles) {continue;}

            const cgltf_accessor* pos = findAttribute(prim, cgltf_attribute_type_position);
            const cgltf_accessor* nrm = findAttribute(prim, cgltf_attribute_type_normal);
            const cgltf_accessor* uv  = findAttribute(prim, cgltf_attribute_type_texcoord);
            if (!pos) {continue;}

            uint32_t primVerts = (uint32_t)pos->count;
            uint32_t primIdx = prim->indices ? (uint32_t)prim->indices->count : primVerts;
            primIdx -= primIdx % 3;

            if (!nrm) {*allHaveNormals = 0;}

            if (mesh)
            {
                for (uint32_t vi = 0; vi < primVerts; vi++)
                {
                    vec3 localPos = vec3Zero();
                    vec3 localNrm = vec3Zero();
                    vec2 localUv = vec2Zero();

                    cgltf_accessor_read_float(pos, vi, localPos.elements, 3);
                    if (nrm) {cgltf_accessor_read_float(nrm, vi, localNrm.elements, 3);}
                    if (uv)  {cgltf_accessor_read_float(uv, vi, localUv.elements, 2);}

                    vec3 wp = transformPoint(&xf, localPos);
                    vec3 wn = nrm ? transformNormal(&xf, localNrm) : vec3Zero();

                    GPUPackedVertex* out = &mesh->vertices[vBase + vi];
                    out->x = wp.x;  out->y = wp.y;  out->z = wp.z;
                    out->nx = wn.x; out->ny = wn.y; out->nz = wn.z;
                    out->u = localUv.x; out->v = localUv.y;

                    for (int k = 0; k < 3; k++)
                    {
                        if (wp.elements[k] < mesh->minBounds[k]) {mesh->minBounds[k] = wp.elements[k];}
                        if (wp.elements[k] > mesh->maxBounds[k]) {mesh->maxBounds[k] = wp.elements[k];}
                    }
                }

                uint32_t material = prim->material ? (uint32_t)(prim->material - data->materials) : (uint32_t)data->materials_count;

                for (uint32_t i = 0; i < primIdx; i += 3)
                {
                    uint32_t i0 = prim->indices ? (uint32_t)cgltf_accessor_read_index(prim->indices, i + 0) : i + 0;
                    uint32_t i1 = prim->indices ? (uint32_t)cgltf_accessor_read_index(prim->indices, i + 1) : i + 1;
                    uint32_t i2 = prim->indices ? (uint32_t)cgltf_accessor_read_index(prim->indices, i + 2) : i + 2;

                    if (xf.flipW) {uint32_t t = i1; i1 = i2; i2 = t;}

                    mesh->indices[iBase + i + 0] = vBase + i0;
                    mesh->indices[iBase + i + 1] = vBase + i1;
                    mesh->indices[iBase + i + 2] = vBase + i2;
                    mesh->triangleMaterials[(iBase + i) / 3] = material;
                }
            }

            vBase += primVerts;
            iBase += primIdx;
        }
    }

    *vCount = vBase;
    *iCount = iBase;
}

static size_t dirLength(const char* path)
{
    size_t n = strlen(path);
    while (n > 0 && path[n - 1] != '/' && path[n - 1] != '\\') {n--;}
    return n;
}

static int gltfTexture(const cgltf_data* data, const char* gltfPath, const cgltf_texture* tex, int srgb)
{
    if (!tex || !tex->image) {return -1;}

    const cgltf_image* img = tex->image;

    if (img->buffer_view && img->buffer_view->buffer->data)
    {
        char key[1100];
        snprintf(key, sizeof(key), "%s#img%d", gltfPath, (int)(img - data->images));

        const unsigned char* bytes = (const unsigned char*)img->buffer_view->buffer->data + img->buffer_view->offset;
        return texAddMemory(key, bytes, (size_t)img->buffer_view->size, srgb);
    }

    if (img->uri && strncmp(img->uri, "data:", 5) != 0)
    {
        char rel[512];
        snprintf(rel, sizeof(rel), "%s", img->uri);
        cgltf_decode_uri(rel);

        char path[1100];
        snprintf(path, sizeof(path), "%.*s%s", (int)dirLength(gltfPath), gltfPath, rel);
        return texAddFile(path, srgb);
    }

    fprintf(stderr, "Unsupported image source (data URI) in %s\n", gltfPath);
    return -1;
}

static void importMaterials(const cgltf_data* data, const char* gltfPath, MeshData* mesh)
{
    mesh->materialCount = (uint32_t)data->materials_count + 1;
    mesh->materials = (Material*)calloc(mesh->materialCount, sizeof(Material));

    if (!mesh->materials) {mesh->materialCount = 0; return;}

    for (cgltf_size i = 0; i < data->materials_count; i++)
    {
        const cgltf_material* src = &data->materials[i];
        Material* m = &mesh->materials[i];

        m->cr = m->cg = m->cb = 1.0f;
        m->roughness = 1.0f;
        m->metallic = 1.0f;
        m->opacity = 1.0f;
        m->baseTex = m->normalTex = m->mrTex = m->emissiveTex = -1;

        if (src->has_pbr_metallic_roughness)
        {
            const cgltf_pbr_metallic_roughness* pbr = &src->pbr_metallic_roughness;

            m->cr = pbr->base_color_factor[0];
            m->cg = pbr->base_color_factor[1];
            m->cb = pbr->base_color_factor[2];
            m->roughness = pbr->roughness_factor;
            m->metallic = pbr->metallic_factor;

            m->baseTex = gltfTexture(data, gltfPath, pbr->base_color_texture.texture, 1);
            m->mrTex = gltfTexture(data, gltfPath, pbr->metallic_roughness_texture.texture, 0);
        }

        m->normalTex = gltfTexture(data, gltfPath, src->normal_texture.texture, 0);
        m->emissiveTex = gltfTexture(data, gltfPath, src->emissive_texture.texture, 1);

        float em = fmaxf(src->emissive_factor[0], fmaxf(src->emissive_factor[1], src->emissive_factor[2]));
        float strength = src->has_emissive_strength ? src->emissive_strength.emissive_strength : 1.0f;
        m->emission = em * strength;

        if (src->has_transmission) {m->opacity = 1.0f - src->transmission.transmission_factor;}
    }

    Material* def = &mesh->materials[data->materials_count];
    def->cr = def->cg = def->cb = 0.8f;
    def->roughness = 0.8f;
    def->opacity = 1.0f;
    def->baseTex = def->normalTex = def->mrTex = def->emissiveTex = -1;
}

int loadGLTF(const char* filename, MeshData* mesh)
{
    memset(mesh, 0, sizeof(MeshData));

    cgltf_options options = {0};
    cgltf_data* data = NULL;

    if (cgltf_parse_file(&options, filename, &data) != cgltf_result_success)
    {
        fprintf(stderr, "Could not parse GLTF file: %s\n", filename);
        
        return 0;
    }

    if (cgltf_load_buffers(&options, data, filename) !=  cgltf_result_success)
    {
        fprintf(stderr, "Could not load GLTF buffers: %s\n", filename);
        cgltf_free(data);

        return 0;
    }

    uint32_t vCount = 0, iCount = 0;
    int allHaveNormals = 1;

    gather(data, NULL, &vCount, &iCount, &allHaveNormals);

    if (vCount == 0 || iCount == 0)
    {
        fprintf(stderr, "No triangle meshes in GLTF: %s\n", filename);
        cgltf_free(data);

        return 0;
    }

    mesh->vertices = (GPUPackedVertex*)calloc(vCount, sizeof(GPUPackedVertex));
    mesh->indices = (uint32_t*)malloc(iCount * sizeof(uint32_t));
    mesh->triangleMaterials = (uint32_t*)malloc(sizeof(uint32_t) * (iCount / 3));

    if (!mesh->vertices || !mesh->indices || !mesh->triangleMaterials)
    {
        fprintf(stderr, "Failed to allocate memory for: %s\n", filename);
        freeMeshData(mesh);
        cgltf_free(data);

        return 0;
    }

    mesh->vertexCount = vCount;
    mesh->indexCount = iCount;
    mesh->triangleCount = iCount / 3;

    mesh->minBounds[0] = mesh->minBounds[1] = mesh->minBounds[2] = FLT_MAX;
    mesh->maxBounds[0] = mesh->maxBounds[1] = mesh->maxBounds[2] = -FLT_MAX;

    gather(data, mesh, &vCount, &iCount, &allHaveNormals);
    importMaterials(data, filename, mesh);
    cgltf_free(data);

    if (!allHaveNormals) {calculateMeshNormals(mesh);}

    printf("Loaded %s: %u vertices, %u triangles\n", filename, mesh->vertexCount, mesh->triangleCount);
    return 1;
}