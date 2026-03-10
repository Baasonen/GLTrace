// Copyright (c) 2026 Henri Paasonen - GPLv2
// See LICENSE for details
#ifndef BVEC_H
#define BVEC_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

// NOTE: Row-Major matrices

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

static inline vec2 vec2Add(const vec2 a, const vec2 b)
{
    vec2 result = {a.x + b.x, a.y + b.y};
    
    return result;
}

static inline vec3 vec3Add(const vec3 a, const vec3 b)
{
    vec3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    
    return result;
}

static inline vec4 vec4Add(const vec4 a, const vec4 b)
{
    vec4 result = {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};

    return result;
}

static inline vec2 vec2Sub(const vec2 a, const vec2 b)
{
    vec2 result = {a.x - b.x, a.y - b.y};
    
    return result;
}

static inline vec3 vec3Sub(const vec3 a, const vec3 b)
{
    vec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    
    return result;
}

static inline vec4 vec4Sub(const vec4 a, const vec4 b)
{
    vec4 result = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};

    return result;
}

static inline vec2 vec2Scale(const vec2 a, const float t)
{
    vec2 result = {a.x * t, a.y * t};

    return result;
}

static inline vec3 vec3Scale(const vec3 a, const float t)
{
    vec3 result = {a.x * t, a.y * t, a.z * t};
    
    return result;
}

static inline vec4 vec4Scale(const vec4 a, const float t)
{
    vec4 result = {a.x * t, a.y * t, a.z * t, a.w * t};

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
    return sqrtf(v.x * v.x + v.y * v.y);
}

static inline float vec3Length(const vec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline float vec4Length(const vec4 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

static inline float vec2Angle(vec2 v, vec2 u)
{
    double theta = (vec2Dot(v, u) / (vec2Length(v) * vec2Length(u)));

    return acos(theta);
}

static inline float vec3Angle(const vec3 v, const vec3 u)
{
    double theta =  (vec3Dot(v, u) / (vec3Length(v) * vec3Length(u)));

    return acos(theta);
}

static inline float vec4Angle(const vec4 v, const vec4 u)
{
    double theta = (vec4Dot(v, u) / (vec4Length(v) * vec4Length(u)));

    return acos(theta);
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
    float invLen = 1.0f / vec2Length(v);

    vec2 norm = {v.x * invLen, v.y * invLen};

    return norm;
}

static inline vec3 vec3Normalize(const vec3 v)
{
    float invLen = 1.0f / vec3Length(v);

    vec3 norm = {v.x * invLen, v.y * invLen, v.z * invLen};

    return norm;
}

static inline vec4 vec4Normalize(const vec4 v)
{
    float invLen = 1.0f / vec4Length(v);

    vec4 norm = {v.x * invLen, v.y * invLen, v.z * invLen, v.w * invLen};

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

static inline mat2 mat2Identity(void)
{
    mat2 result;

    vec2 a = {1.0f, 0.0f};
    vec2 b = {0.0f, 1.0f};

    result.a = a;
    result.b = b;

    return result;
}

static inline mat3 mat3Identity(void)
{
    mat3 result;

    vec3 a = {1.0f, 0.0f, 0.0f};
    vec3 b = {0.0f, 1.0f, 0.0f};
    vec3 c = {0.0f, 0.0f, 1.0f};

    result.a = a;
    result.b = b;
    result.c = c;

    return result;
}

static inline mat4 mat4Identity(void)
{
    mat4 result;

    vec4 a = {1.0f, 0.0f, 0.0f, 0.0f};
    vec4 b = {0.0f, 1.0f, 0.0f, 0.0f};
    vec4 c = {0.0f, 0.0f, 1.0f, 0.0f};
    vec4 d = {0.0f, 0.0f, 0.0f, 1.0f};

    result.a = a;
    result.b = b;
    result.c = c;
    result.d = d;

    return result;
}

static inline vec2 vec2Zero(void)
{
    vec2 result = {0.0f, 0.0f};

    return result;
}

static inline vec3 vec3Zero(void)
{
    vec3 result = {0.0f, 0.0f, 0.0f};

    return result;
}

static inline vec4 vec4Zero(void)
{
    vec4 result = {0.0f, 0.0f, 0.0f, 0.0f};

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

static inline mat2 mat2Diag(const vec2 v)
{
    mat2 result;

    vec2 a = {v.x, 0.0f};
    vec2 b = {0.0f, v.y};

    result.a = a;
    result.b = b;

    return result;
}

static inline mat3 mat3Diag(const vec3 v)
{
    mat3 result;

    vec3 a = {v.x, 0.0f, 0.0f};
    vec3 b = {0.0f, v.y, 0.0f};
    vec3 c = {0.0f, 0.0f, v.z};

    result.a = a;
    result.b = b;
    result.c = c;

    return result;
}

static inline mat4 mat4Diag(const vec4 v)
{
    mat4 result;

    vec4 a = {v.x, 0.0f, 0.0f, 0.0f};
    vec4 b = {0.0f, v.y, 0.0f, 0.0f};
    vec4 c = {0.0f, 0.0f, v.z, 0.0f};
    vec4 d = {0.0f, 0.0f, 0.0f, v.w};

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

    vec4 col_x = {j.a.x, j.b.x, j.c.x, j.d.x};
    vec4 col_y = {j.a.y, j.b.y, j.c.y, j.d.y};
    vec4 col_z = {j.a.z, j.b.z, j.c.z, j.d.z};
    vec4 col_w = {j.a.w, j.b.w, j.c.w, j.d.w};

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

    float c = cosf(angle);
    float s = sinf(angle);
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
    float fovRad = fovDeg * (3.14159265f / 180.0f);

    mat4 result = mat4Zero();

    float tanHalfFov = tanf(fovRad / 2.0f);

    result.a.x = 1.0f / (aspectRatio * tanHalfFov);
    result.b.y = 1.0f / tanHalfFov;
    result.c.z = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result.c.w = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    result.d.z = -1.0f;

    return result;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif