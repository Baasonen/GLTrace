#ifndef MATERIALS_H
#define MATERIALS_H

#include "shader_structs.h"

Material g_materials[] =
{
    // Matte Green 0
    {0.7f, 0.06f, 0.07f, 0.0f, 0.85f, 0.05f, 0.0f, 1.0f}, 

    // 1: Emissive green
    {0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 10.0f, 1.0f},

    // 2: Emissive red
    {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 10.0f, 1.0f},

    // 3: Emissive blue
    {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 5.0f, 1.0f},

    // 4: Stronger emissive blue
    {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 15.0f, 1.0f},
 
    // 5: Matte white
    {1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f}, 

    // 6: Metallic
    {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f} 
};

const int NUM_MATERIALS = sizeof(g_materials) / sizeof(Material);

#endif