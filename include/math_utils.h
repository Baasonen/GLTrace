#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415
#endif

static inline float radians(float deg)
{
    return deg * (M_PI / 180.0f);
}

#endif