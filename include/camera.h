#ifndef CAMERA_H
#define CAMERA_H

typedef struct 
{
    float x, y, z;
    float yaw, pitch;
    float focalLength;
    float padding[2];
} Camera;

#endif