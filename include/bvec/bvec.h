// Copyright (c) 2026 Henri Paasonen - GPLv2
// See LICENSE for details

// NOTE: Matrices are row-major: a = first row, b = second row, ...
// When using OpenGL transpose = GL_TRUE may be required in glUniformMatrix4fv

#ifndef BVEC_H
#define BVEC_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BVEC_NO_STD_LIB
    #include <math.h>
    #define BVEC_SQRT(x) sqrtf(x)
    #define BVEC_COS(x) cosf(x)
    #define BVEC_SIN(x) sinf(x)
    #define BVEC_TAN(x) tanf(x)
    #define BVEC_ACOS(x) acosf(x)
#else 
    // Provide macros if math.h unavailable
#endif

#ifndef BVEC_PI
    #ifdef M_PI
        #define BVEC_PI M_PI 
    #else   
        #define BVEC_PI 3.14159265358979323846f
    #endif
#endif

static inline float bvecClamp(float d, float min, float max)
{
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

typedef union 
{
    struct {float x, y;};
    float elements[2];
} vec2;

typedef union 
{
    struct {float x, y, z;};
    float elements[3];
} vec3;

typedef union 
{
    struct {float x, y, z, w;};
    float elements[4];
} vec4;

static inline vec2 vec2Make(float x, float y)
{
    vec2 v = {{x, y}};
    return v;
}

static inline vec3 vec3Make(float x, float y, float z)
{
    vec3 v = {{x, y, z}};
    return v;
}

static inline vec4 vec4Make(float x, float y, float z, float w)
{
    vec4 v = {{x, y, z, w}};
    return v;
}

typedef struct 
{
    vec2 a, b;
} mat2;

typedef struct 
{
    vec3 a, b, c;
} mat3;

typedef struct 
{
    vec4 a, b, c, d;
} mat4;

static inline vec2 vec2Zero(void)
{
    vec2 result = vec2Make(0.0f, 0.0f);

    return result;
}

static inline vec3 vec3Zero(void)
{
    vec3 result = vec3Make(0.0f, 0.0f, 0.0f);

    return result;
}

static inline vec4 vec4Zero(void)
{
    vec4 result = vec4Make(0.0f, 0.0f, 0.0f, 0.0f);

    return result;
}

static inline mat2 mat2Zero(void)
{
    mat2 result;

    result.a = vec2Zero();
    result.b = vec2Zero();

    return result;
}

static inline mat3 mat3Zero(void)
{
    mat3 result;

    result.a = vec3Zero();
    result.b = vec3Zero();
    result.c = vec3Zero();

    return result;
}

static inline mat4 mat4Zero(void)
{
    mat4 result;

    result.a = vec4Zero();
    result.b = vec4Zero();
    result.c = vec4Zero();
    result.d = vec4Zero();

    return result;
}

static inline mat2 mat2Identity(void)
{
    mat2 result;

    vec2 a = vec2Make(1.0f, 0.0f);
    vec2 b = vec2Make(0.0f, 1.0f);

    result.a = a;
    result.b = b;

    return result;
}

static inline mat3 mat3Identity(void)
{
    mat3 result;

    vec3 a = vec3Make(1.0f, 0.0f, 0.0f);
    vec3 b = vec3Make(0.0f, 1.0f, 0.0f);
    vec3 c = vec3Make(0.0f, 0.0f, 1.0f);

    result.a = a;
    result.b = b;
    result.c = c;

    return result;
}

static inline mat4 mat4Identity(void)
{
    mat4 result;

    vec4 a = vec4Make(1.0f, 0.0f, 0.0f, 0.0f);
    vec4 b = vec4Make(0.0f, 1.0f, 0.0f, 0.0f);
    vec4 c = vec4Make(0.0f, 0.0f, 1.0f, 0.0f);
    vec4 d = vec4Make(0.0f, 0.0f, 0.0f, 1.0f);

    result.a = a;
    result.b = b;
    result.c = c;
    result.d = d;

    return result;
}

static inline vec2 vec2Add(const vec2 a, const vec2 b)
{
    vec2 result = vec2Make(a.x + b.x, a.y + b.y);
    
    return result;
}

static inline vec3 vec3Add(const vec3 a, const vec3 b)
{
    vec3 result = vec3Make(a.x + b.x, a.y + b.y, a.z + b.z);
    
    return result;
}

static inline vec4 vec4Add(const vec4 a, const vec4 b)
{
    vec4 result = vec4Make(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);

    return result;
}

static inline vec2 vec2Sub(const vec2 a, const vec2 b)
{
    vec2 result = vec2Make(a.x - b.x, a.y - b.y);
    
    return result;
}

static inline vec3 vec3Sub(const vec3 a, const vec3 b)
{
    vec3 result = vec3Make(a.x - b.x, a.y - b.y, a.z - b.z);
    
    return result;
}

static inline vec4 vec4Sub(const vec4 a, const vec4 b)
{
    vec4 result = vec4Make(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);

    return result;
}

static inline vec2 vec2Scale(const vec2 a, const float t)
{
    vec2 result = vec2Make(a.x * t, a.y * t);

    return result;
}

static inline vec3 vec3Scale(const vec3 a, const float t)
{
    vec3 result = vec3Make(a.x * t, a.y * t, a.z * t);
    
    return result;
}

static inline vec4 vec4Scale(const vec4 a, const float t)
{
    vec4 result = vec4Make(a.x * t, a.y * t, a.z * t, a.w * t);

    return result;
}

static inline float vec2Dot(const vec2 a, const vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

static inline float vec3Dot(const vec3 a, const vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float vec4Dot(const vec4 a, const vec4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

static inline float vec2Cross(const vec2 a, const vec2 b)
{
    return a.x * b.y - a.y * b.x;
} 

static inline vec3 vec3Cross(const vec3 a, const vec3 b)
{
    vec3 result;

    result.x = (a.y * b.z - a.z * b.y);
    result.y = (a.z * b.x - a.x * b.z);
    result.z = (a.x * b.y - a.y * b.x);

    return result;
}

static inline float vec2Length(const vec2 v)
{
    return BVEC_SQRT(v.x * v.x + v.y * v.y);
}

static inline float vec3Length(const vec3 v)
{
    return BVEC_SQRT(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline float vec4Length(const vec4 v)
{
    return BVEC_SQRT(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

static inline float vec2Angle(const vec2 v, const vec2 u)
{
    float theta = (vec2Dot(v, u) / (vec2Length(v) * vec2Length(u)));

    return BVEC_ACOS(bvecClamp(theta, -1.0f, 1.0f));
}

static inline float vec3Angle(const vec3 v, const vec3 u)
{
    float theta =  (vec3Dot(v, u) / (vec3Length(v) * vec3Length(u)));

    return BVEC_ACOS(bvecClamp(theta, -1.0f, 1.0f));
}

static inline float vec4Angle(const vec4 v, const vec4 u)
{
    float theta = (vec4Dot(v, u) / (vec4Length(v) * vec4Length(u)));

    return BVEC_ACOS(bvecClamp(theta, -1.0f, 1.0f));
}

static inline float vec2LengthSq(const vec2 v)
{
    return vec2Dot(v, v);
}

static inline float vec3LengthSq(const vec3 v)
{
    return vec3Dot(v, v);
}

static inline float vec4LengthSq(const vec4 v)
{
    return vec4Dot(v, v);
}

static inline vec2 vec2Normalize(const vec2 v)
{
    float len = vec2Length(v);
    if (len == 0.0f) {return vec2Zero();}
    float invLen = 1.0f / len;

    vec2 norm = vec2Make(v.x * invLen, v.y * invLen);

    return norm;
}

static inline vec3 vec3Normalize(const vec3 v)
{
    float len = vec3Length(v);
    if (len == 0.0f) {return vec3Zero();}
    float invLen = 1.0f / len;

    vec3 norm = vec3Make(v.x * invLen, v.y * invLen, v.z * invLen);

    return norm;
}

static inline vec4 vec4Normalize(const vec4 v)
{
    float len = vec4Length(v);
    if (len == 0.0f) {return vec4Zero();}
    float invLen = 1.0f / len;

    vec4 norm = vec4Make(v.x * invLen, v.y * invLen, v.z * invLen, v.w * invLen);

    return norm;
}

static inline mat2 mat2Scale(const mat2 m, const float t)
{
    mat2 result;

    result.a = vec2Scale(m.a, t);
    result.b = vec2Scale(m.b, t);

    return result;
}

static inline mat3 mat3Scale(const mat3 m, const float t)
{
    mat3 result;

    result.a = vec3Scale(m.a, t);
    result.b = vec3Scale(m.b, t);
    result.c = vec3Scale(m.c, t);

    return result;
}

static inline mat4 mat4Scale(const mat4 m, const float t)
{
    mat4 result;

    result.a = vec4Scale(m.a, t);
    result.b = vec4Scale(m.b, t);
    result.c = vec4Scale(m.c, t);
    result.d = vec4Scale(m.d, t);

    return result;
}

static inline mat2 mat2Diag(const vec2 v)
{
    mat2 result;

    vec2 a = vec2Make(v.x, 0.0f);
    vec2 b = vec2Make(0.0f, v.y);

    result.a = a;
    result.b = b;

    return result;
}

static inline mat3 mat3Diag(const vec3 v)
{
    mat3 result;

    vec3 a = vec3Make(v.x, 0.0f, 0.0f);
    vec3 b = vec3Make(0.0f, v.y, 0.0f);
    vec3 c = vec3Make(0.0f, 0.0f, v.z);

    result.a = a;
    result.b = b;
    result.c = c;

    return result;
}

static inline mat4 mat4Diag(const vec4 v)
{
    mat4 result;

    vec4 a = vec4Make(v.x, 0.0f, 0.0f, 0.0f);
    vec4 b = vec4Make(0.0f, v.y, 0.0f, 0.0f);
    vec4 c = vec4Make(0.0f, 0.0f, v.z, 0.0f);
    vec4 d = vec4Make(0.0f, 0.0f, 0.0f, v.w);

    result.a = a;
    result.b = b;
    result.c = c;
    result.d = d;

    return result;
}

static inline mat2 mat2Transpose(const mat2 m)
{
    mat2 result;

    result.a.x = m.a.x;
    result.a.y = m.b.x;

    result.b.x = m.a.y;
    result.b.y = m.b.y;

    return result;
}

static inline mat3 mat3Transpose(const mat3 m)
{
    mat3 result;

    result.a.x = m.a.x;
    result.a.y = m.b.x;
    result.a.z = m.c.x;

    result.b.x = m.a.y;
    result.b.y = m.b.y;
    result.b.z = m.c.y;

    result.c.x = m.a.z;
    result.c.y = m.b.z;
    result.c.z = m.c.z;

    return result;
}

static inline mat4 mat4Transpose(const mat4 m)
{
    mat4 result;

    result.a.x = m.a.x;
    result.a.y = m.b.x;
    result.a.z = m.c.x;
    result.a.w = m.d.x;

    result.b.x = m.a.y;
    result.b.y = m.b.y;
    result.b.z = m.c.y;
    result.b.w = m.d.y;

    result.c.x = m.a.z;
    result.c.y = m.b.z;
    result.c.z = m.c.z;
    result.c.w = m.d.z;

    result.d.x = m.a.w;
    result.d.y = m.b.w;
    result.d.z = m.c.w;
    result.d.w = m.d.w;

    return result;
}

static inline mat4 mat4Mul(const mat4 i, const mat4 j)
{
    mat4 result;

    vec4 col_x = vec4Make(j.a.x, j.b.x, j.c.x, j.d.x);
    vec4 col_y = vec4Make(j.a.y, j.b.y, j.c.y, j.d.y);
    vec4 col_z = vec4Make(j.a.z, j.b.z, j.c.z, j.d.z);
    vec4 col_w = vec4Make(j.a.w, j.b.w, j.c.w, j.d.w);

    result.a.x = vec4Dot(i.a, col_x);
    result.a.y = vec4Dot(i.a, col_y);
    result.a.z = vec4Dot(i.a, col_z);
    result.a.w = vec4Dot(i.a, col_w);

    result.b.x = vec4Dot(i.b, col_x);
    result.b.y = vec4Dot(i.b, col_y);
    result.b.z = vec4Dot(i.b, col_z);
    result.b.w = vec4Dot(i.b, col_w);

    result.c.x = vec4Dot(i.c, col_x);
    result.c.y = vec4Dot(i.c, col_y);
    result.c.z = vec4Dot(i.c, col_z);
    result.c.w = vec4Dot(i.c, col_w);

    result.d.x = vec4Dot(i.d, col_x);
    result.d.y = vec4Dot(i.d, col_y);
    result.d.z = vec4Dot(i.d, col_z);
    result.d.w = vec4Dot(i.d, col_w);

    return result;
}

static inline vec4 mat4MulVec4(const mat4 m, const vec4 v)
{
    vec4 result;

    result.x = vec4Dot(m.a, v);
    result.y = vec4Dot(m.b, v);
    result.z = vec4Dot(m.c, v);
    result.w = vec4Dot(m.d, v);

    return result;
}

static inline mat4 mat2ToMat4(const mat2 m)
{
    mat4 result = mat4Identity();

    result.a.x = m.a.x; result.a.y = m.a.y;
    result.b.x = m.b.x; result.b.y = m.b.y;

    return result;
}

static inline mat4 mat3ToMat4(const mat3 m)
{
    mat4 result = mat4Identity();

    result.a.x = m.a.x; result.a.y = m.a.y; result.a.z = m.a.z;
    result.b.x = m.b.x; result.b.y = m.b.y; result.b.z = m.b.z;
    result.c.x = m.c.x; result.c.y = m.c.y; result.c.z = m.c.z;

    return result;
}

static inline mat4 mat4Translate(const vec3 v)
{
    mat4 result = mat4Identity();

    result.a.w = v.x;
    result.b.w = v.y;
    result.c.w = v.z;

    return result;
}

static inline mat4 mat4ScaleVec3(const vec3 v)
{
    mat4 result = mat4Identity();

    result.a.x = v.x;
    result.b.y = v.y;
    result.c.z = v.z;

    return result;
}

static inline mat4 mat4Rotate(const vec3 axis, float angle)
{
    mat4 result = mat4Identity();

    float c = BVEC_COS(angle);
    float s = BVEC_SIN(angle);
    float t = 1.0f - c;

    vec3 u = vec3Normalize(axis);

    result.a.x = t * u.x * u.x + c;
    result.a.y = t * u.x * u.y - s * u.z;
    result.a.z = t * u.x * u.z + s * u.y;

    result.b.x = t * u.x * u.y + s * u.z;
    result.b.y = t * u.y * u.y + c;
    result.b.z = t * u.y * u.z - s * u.x;

    result.c.x = t * u.x * u.z - s * u.y;
    result.c.y = t * u.y * u.z + s * u.x;
    result.c.z = t * u.z * u.z + c;

    return result;
}

static inline mat4 mat4Transform(vec3 translation, vec3 axis, float angle, vec3 scale)
{
    mat4 T = mat4Translate(translation);
    mat4 R = mat4Rotate(axis, angle);
    mat4 S = mat4ScaleVec3(scale);

    mat4 RS = mat4Mul(R, S);
    mat4 TRS = mat4Mul(T, RS);

    return TRS;
}

static inline mat4 mat4Perspective(float fovDeg, float aspectRatio, float nearPlane, float farPlane)
{
    float fovRad = fovDeg * (BVEC_PI / 180.0f);

    mat4 result = mat4Zero();

    float tanHalfFov = BVEC_TAN(fovRad / 2.0f);

    result.a.x = 1.0f / (aspectRatio * tanHalfFov);
    result.b.y = 1.0f / tanHalfFov;
    result.c.z = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result.c.w = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    result.d.z = -1.0f;

    return result;
}

static inline vec3 vec3Lerp(const vec3 a, const vec3 b, float t)
{
    float i = a.x + (b.x - a.x) * t;
    float j = a.y + (b.y - a.y) * t;
    float k = a.z + (b.z - a.z) * t;

    return vec3Make(i, j, k);
}

static inline vec3 vec3Reflect(const vec3 v, const vec3 normal)
{
    float d = vec3Dot(v, normal);

    float i = v.x - (2.0f * d * normal.x);
    float j = v.y - (2.0f * d * normal.y);
    float k = v.z - (2.0f * d * normal.z);

    return vec3Make(i, j, k);
}

static inline vec3 vec3Project(const vec3 a, const vec3 b)
{
    float denom = vec3Dot(b, b);
    if (denom == 0.0f) {return vec3Zero();}

    float scale = vec3Dot(a, b) / denom;

    return vec3Scale(b, scale);
}

static inline mat4 mat4LookAt(vec3 eye, vec3 target, vec3 up)
{
    vec3 f = vec3Normalize(vec3Sub(target, eye)); 
    vec3 s = vec3Normalize(vec3Cross(f, up));
    vec3 u = vec3Cross(s, f);

    mat4 result = mat4Identity();

    result.a.x = s.x;
    result.a.y = s.y;
    result.a.z = s.z;
    result.a.w = -vec3Dot(s, eye);

    result.b.x = u.x;
    result.b.y = u.y;
    result.b.z = u.z;
    result.b.w = -vec3Dot(u, eye);

    result.c.x = -f.x;
    result.c.y = -f.y;
    result.c.z = -f.z;
    result.c.w = vec3Dot(f, eye);

    return result;
}

#ifdef __cplusplus
}
#endif

#endif