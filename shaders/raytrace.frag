#version 460 core

#define SKY false
#define MAX_BOUNCES  5
#define SAMPLES_PER_PIXEL 1

// HEADER
out vec4 fragColor;
in vec2 v_uv;

struct Sphere 
{
    vec3 pos;
    float radius;
    int materialIndex;
    float padding[3];
};

struct Material
{
    vec3 color;
    float visibility;
    float roughness;
    float metallic;
    float emission;
    float opacity;
};

struct Vertex 
{
    vec3 pos;
    float pad;
};

struct BVHNode
{
    vec3 aabbMin;
    uint leftFirst;
    vec3 aabbMax;
    uint triCount;
};

const float M_PI = 3.1415926;

layout(std430, binding = 0) buffer SceneData {Sphere spheres[];};
layout(std430, binding = 1) buffer MaterialData {Material materials[];};
layout(std430, binding = 2) buffer VertexData {Vertex vertices[];};
layout(std430, binding = 3) buffer IndexData {uint indices[];};
layout(std430, binding = 4) buffer BVHData {BVHNode bvhNodes[];};
layout(std430, binding = 5) buffer TriangleMaterialData {uint triangleMaterials[];};

uniform vec2 u_resolution;
uniform int u_frameCount;
uniform sampler2D u_historyTexture;
uniform vec3 u_cameraPos;
uniform float u_cameraYaw;
uniform float u_cameraPitch;

uniform vec3 u_camForward;
uniform vec3 u_camRight;
uniform vec3 u_camUp;

uint pcgHash(inout uint state)
{
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^state) * 277803737u;
    return (word >> 22u) ^ word;
}

float random(inout uint state)
{
    return float(pcgHash(state) * 2.3283064365386963e-10);
}

vec3 cosHemisphere(vec3 n, inout uint seed)
{
    float r1 = random(seed);
    float r2 = random(seed);

    float phi = 2.0 * M_PI * r1;
    float cosTheta = sqrt(1.0 - r2);
    float sinTheta = sqrt(r2);

    vec3 localRay = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    
    vec3 up = abs(n.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 tangent = normalize(cross(up, n));
    vec3 biTangent = cross(n, tangent);

    return tangent * localRay.x + biTangent * localRay.y + n * localRay.z;
}

float hitAABB(vec3 aabbMin, vec3 aabbMax, vec3 ro, vec3 invDir)
{
    
    vec3 t0s = (aabbMin - ro) * invDir;
    vec3 t1s = (aabbMax - ro) * invDir;

    vec3 tMin = min(t0s, t1s);
    vec3 tMax = max(t0s, t1s);

    float tNear = max(tMin.x, max(tMin.y, tMin.z));
    float tFar = min(tMax.x, min(tMax.y, tMax.z));

    return (tFar >= tNear && tFar > 0.0) ? tNear : 1e30;
}

float hitTriangleIndexed(int triIndex, vec3 ro, vec3 rd)
{
    uint i0 = indices[3 * triIndex + 0];
    uint i1 = indices[3 * triIndex + 1];
    uint i2 = indices[3 * triIndex + 2];

    vec3 v0 = vertices[i0].pos;
    vec3 v1 = vertices[i1].pos;
    vec3 v2 = vertices[i2].pos;

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(rd, edge2);
    float a = dot(edge1, h);

    // Check parallel
    if (abs(a) < 0.00001) {return -1.0;}

    float f = 1.0 / a;
    vec3 s = ro - v0;
    float u = f * dot(s, h);

    if (u < 0.0 || u > 1.0) {return -1.0;}

    vec3 q = cross(s, edge1);
    float v = f * dot(rd, q);

    if (v < 0.0 || u + v > 1.0) {return -1.0;}

    float t = f * dot(edge2, q);

    return (t > 0.001) ? t : -1.0;
}

float hitSphere(Sphere s, vec3 ro, vec3 rd)
{
    vec3 oc = ro - s.pos;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - s.radius * s.radius;
    float h = b * b - c; // Simplified discriminant

    //if (h < 0.0) {return -1.0;}

    // We want the closest hit
    //return -b - sqrt(h);

    return (h < 0.0) ? -1.0 : (-b - sqrt(h));
}

// SHADER

void findClosestHit(vec3 ro, vec3 rd, vec3 invDir, bool primaryRay, out float minT, out int hitIndex, out int hitType)
{
    minT = 10000.0;
    hitIndex = -1;
    hitType = 0;

    // Check for sphere
    for(int i = 0; i < spheres.length(); i++)
    {
        float t = hitSphere(spheres[i], ro , rd);
        if (t > 0.001 && t < minT)
        {
            bool invisible = materials[spheres[i].materialIndex].visibility > 0.5;

            // Skip if material is invisible
            if (primaryRay && invisible) {continue;}

            minT = t;
            hitIndex = i;
            hitType = 1;
        }
    }

    // Triangle
    int stack[32];
    int stackPtr = 0;   
    stack[stackPtr++] = 0;

    while (stackPtr > 0)
    {
        // Pop next node
        int nodeIdx = stack[--stackPtr];
        BVHNode node = bvhNodes[nodeIdx];

        // Skip if further than current best hit
        float distToBox = hitAABB(node.aabbMin, node.aabbMax, ro, invDir);
        if (distToBox >= minT) {continue;}

        if (node.triCount > 0)
        {
            for (uint i = 0; i < node.triCount; i++)
            {
                int triIdx = int(node.leftFirst + i);
                float t = hitTriangleIndexed(triIdx, ro, rd);

                if (t > 0.001 && t < minT)
                {
                    minT = t;
                    hitIndex = triIdx;
                    hitType = 2;
                }
            }
        }
        else // Internal node
        {
            int leftChild = int(node.leftFirst);
            int rightChild = int(node.leftFirst + 1);

            float distL = hitAABB(bvhNodes[leftChild].aabbMin, bvhNodes[leftChild].aabbMax, ro, invDir);
            float distR = hitAABB(bvhNodes[rightChild].aabbMin, bvhNodes[rightChild].aabbMax, ro, invDir);

            if (distL < distR)
            {
                if (distR < minT) {stack[stackPtr++] = rightChild;}
                if (distL < minT) {stack[stackPtr++] = leftChild;}
            }
            else
            {
                if (distL < minT) {stack[stackPtr++] = leftChild;}
                if (distR < minT) {stack[stackPtr++] = rightChild;}
            }
        }
    }
}

vec3 shade(vec3 hitPos, vec3 normal, vec3 rd, int materialIndex)
{
    Material material = materials[materialIndex];
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 v = -rd;

    // Emission
    vec3 emissive = material.color.rgb * material.emission;

    // Lambertian diffuse
    vec3 diffuseColor = material.color.rgb * (1.0 - material.metallic);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * diffuseColor;

    // Specular
    //vec3 specularColor = mix(vec3(0.04), material.color.rgb, material.metallic);
    //vec3 halfwayDir = normalize(lightDir + v);
    //float shininess = 1.0 / (material.roughness * material.roughness + 0.001);
    //float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    //vec3 specular = specularColor * spec;

    // Ambient 
    vec3 ambient = 0.1 * material.color.rgb;

    return emissive + ambient + diffuse;
}

void main()
{
    vec2 pixelCoord = gl_FragCoord.xy;

    uint seed = uint(pixelCoord.x) + uint(pixelCoord.y) * uint(u_resolution) + uint(u_frameCount) * 7125413u;

    // Camera
    float aspect = u_resolution.x / u_resolution.y;

    vec3 ro = u_cameraPos;

    float fov = 45.0;
    float tanFov = tan(radians(fov) * 0.5);

    const int maxBounces = MAX_BOUNCES;
    const int samplesPerPixel = SAMPLES_PER_PIXEL;

    // Recursive raytracing
    vec3 frameColor = vec3(0.0, 0.0, 0.0);

    for (int s = 0; s < samplesPerPixel; s++)
    {
        uint sampleSeed = seed + uint(s) * 9781u;

        vec2 jitter = (vec2(random(sampleSeed), random(sampleSeed)) - 0.5);
        vec2 uv = (pixelCoord + jitter) / u_resolution * 2.0 - 1.0;
        uv.x *= aspect;

        vec3 rd = normalize(u_camForward + uv.x * tanFov * u_camRight + uv.y * tanFov * u_camUp);

        vec3 accumulatedLight = vec3(0.0, 0.0, 0.0);
        vec3 throughput = vec3(1.0, 1.0, 1.0);
        vec3 currentRo = u_cameraPos;
        vec3 currentRd = rd;

        for (int bounce = 0; bounce < maxBounces; bounce++)
        {
            float minT;
            int hitIndex;
            int hitType;

            vec3 invDir = 1.0 / currentRd;
            findClosestHit(currentRo, currentRd, invDir, (bounce == 0), minT, hitIndex, hitType);

            if (hitIndex != -1)
            {
                vec3 hitPos = currentRo + currentRd * minT;
                vec3 normal;
                int materialIndex;

                if (hitType == 1)
                {
                    Sphere s = spheres[hitIndex];
                    normal = normalize(hitPos - s.pos);
                    materialIndex = s.materialIndex;
                }
                else if (hitType == 2)
                {
                    uint i0 = indices[3 * hitIndex + 0];
                    uint i1 = indices[3 * hitIndex + 1];
                    uint i2 = indices[3 * hitIndex + 2];

                    vec3 v0 = vertices[i0].pos;
                    vec3 v1 = vertices[i1].pos;
                    vec3 v2 = vertices[i2].pos;

                    vec3 edge1 = v1 - v0;
                    vec3 edge2 = v2 - v0;

                    normal = normalize(cross(edge1, edge2));
                    
                    materialIndex = int(triangleMaterials[hitIndex]);

                    // Flip normal if hit back face
                    if (dot(normal, currentRd) > 0.0) {normal = -normal;}
                }
                Material material = materials[materialIndex];

                accumulatedLight += material.color.rgb * material.emission * throughput;

                // Simple Schlik Fresnel approx
                float fresnel = 0.04 + (1.0 - 0.04) * pow(1.0 - max(dot(normal, -currentRd), 0.0), 5.0);
                bool isSpecular = random(sampleSeed) < mix(fresnel, 1.0, material.metallic);

                if (isSpecular)
                {
                    vec3 reflectDir = reflect(currentRd, normal);
                    
                    currentRd = normalize(mix(reflectDir, cosHemisphere(normal, sampleSeed), material.roughness * material.roughness));
                    throughput *= mix(vec3(1.0, 1.0, 1.0), material.color.rgb, material.metallic);
                }
                else
                {
                    currentRd = cosHemisphere(normal, sampleSeed);
                    throughput *= material.color.rgb;
                }

                // Offset to prevent self intersection
                currentRo = hitPos + normal * 0.001;

                // Russian roulette
                float p = max(throughput.r, max(throughput.g, throughput.b));
                if (random(sampleSeed) > p) {break;}
                throughput /= p;
            }
            else
            {
                vec3 skyColor = vec3(0.0, 0.0, 0.0);

                if (SKY)
                {
                    float t = clamp(currentRd.y, -1.0, 1.0);

                    vec3 top    = vec3(0.5, 0.7, 1.0);
                    vec3 middle = vec3(0.7, 0.8, 1.0);
                    vec3 bottom = vec3(0.1, 0.1, 0.2);

                    skyColor = mix
                    (
                        mix(middle, top, smoothstep(0.0, 1.0, t)),
                        bottom,
                        smoothstep(0.0, 1.0, -t)
                    );
                }

                //vec3 skyColor = mix(vec3(1.0), vec3(0.5, 0.7, 1.0), 0.5 * (currentRd.y + 1.0));
                //vec3 skyColor = vec3(0.0, 0.0, 0.0);
                accumulatedLight += throughput * skyColor;
                break;
            }
        }

        frameColor += accumulatedLight;
    }
    
    // Average SPP
    frameColor /= float(samplesPerPixel);

    // Temporal accumulation
    vec3 prevColor = texture(u_historyTexture, pixelCoord / u_resolution).rgb;
    prevColor = pow(prevColor, vec3(2.2));

    float weight = 1.0 / float(u_frameCount);
    vec3 finalLinear = mix(prevColor, frameColor, weight);

    fragColor = vec4(pow(finalLinear, vec3(1.0 / 2.2)), 1.0);
}