#ifndef CAMERA_H
#define CAMERA_H

typedef struct 
{
    float px, py, pz;
    float yaw, pitch;
    float focalLength;
    float padding[2];
} Camera;

#endif