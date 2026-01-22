#include "matrix.h"

void normalize(Vec4* v)
{
    float sqrSum = (v->x) * (v->x) + (v->y) * (v->y) + (v->z) * (v->z);
    float length = sqrtf(sqrSum);

    v->x /= length;
    v->y /= length;
    v->z /= length;
}

Vec4 crossProduct(Vec4 a, Vec4 b)
{
    Vec4 result;

    result.x = (a.y * b.z) - (a.z * b.y);
    result.y = (a.z * b.x) - (a.x * b.z);
    result.z = (a.x * b.y) - (a.y * b.x);

    return result;
}

Mat4 mat4Multiply(Mat4 a, Mat4 b)
{
    Mat4 result = {0};

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            for (int k = 0; k < 4; k++)
            {
                result.m[row][col] += a.m[row][k] * b.m[k][col];
            }
        }
    }

    return result;
}

void vecScale(Vec4* a, float b)
{
    a->x *= b;
    a->y *= b;
    a->z *= b;
}

Mat4 createIdentity()
{
    Mat4 m = {0};
    m.m[0][0] = 1;
    m.m[1][1] = 1;
    m.m[2][2] = 1;
    m.m[3][3] = 1;

    return m;
}

Mat4 translationMatrix(Vec4 rotation)
{
    Mat4 m = createIdentity();

    m.m[0][3] = rotation.x;
    m.m[1][3] = rotation.y;
    m.m[2][3] = rotation.z;
    m.m[3][3] = 1;

    return m;
}

Mat4 scaleMatrix(Vec4 scale)
{
    Mat4 m = {0};

    m.m[0][0] = scale.x;
    m.m[1][1] = scale.y;
    m.m[2][2] = scale.z;
    m.m[3][3] = 1;

    return m;
}

Mat4 rotationX(float angle)
{
    Mat4 m = createIdentity();

    float c = cosf(angle);
    float s = sinf(angle);

    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;

    return m;
}

Mat4 rotationY(float angle)
{
    Mat4 m = createIdentity();

    float c = cosf(angle);
    float s = sinf(angle);

    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;

    return m;
}

Mat4 rotationZ(float angle)
{
    Mat4 m = createIdentity();

    float c = cosf(angle);
    float s = sinf(angle);

    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;

    return m;
}

Mat4 rotationMatrix(Vec4 rotation)
{
    Mat4 rx = rotationX(rotation.x);
    Mat4 ry = rotationY(rotation.y);
    Mat4 rz = rotationZ(rotation.z);

    return mat4Multiply(rz, mat4Multiply(ry, rx));
}
