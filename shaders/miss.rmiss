#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout(location = 0) rayPayloadInEXT IntersectionInfo isect;

void main()
{
    // On a miss, all other payload attributes will be garbage values
    isect.t = kMiss;
}
