#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require


#include "lib/Common.h"

layout(location = 0) rayPayloadInEXT RayPayloadInfo rayInfo;

void main()
{
    // 没有命中任何
    rayInfo.hitT = -1.0;
}