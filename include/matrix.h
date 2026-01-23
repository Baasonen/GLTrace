#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

#include "shader_structs.h"

void normalize(Vec4* v);
Vec4 crossProduct(Vec4 a, Vec4 b);
Mat4 mat4Multiply(Mat4 a, Mat4 b);
void vecScale(Vec4* a, float b);
Mat4 createIdentity();
Mat4 translationMatrix(Vec4 rotation);
Mat4 scaleMatrix(Vec4 scale);
Mat4 rotationX(float angle);
Mat4 rotationY(float angle);
Mat4 roationZ(float angle);
Mat4 rotationMatrix(Vec4 rotation);
Mat4 transformMatrix(Vec4 position, Vec4 scale, Vec4 rotation);
Vec4 matrixMultiplyVec4(Mat4 m, Vec4 v);

#endif