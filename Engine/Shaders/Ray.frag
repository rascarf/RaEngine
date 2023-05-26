#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;

layout(binding = 2) uniform ParamBlock
{
    vec3    color;
    float   power;
    vec3    viewDir;
    float   padding;
}rayParam;

layout(location = 0) out vec4 outFragColor;
layout(location = 1) out vec4 outNormal;

void main()
{
    float vdotn = dot(normalize(rayParam.viewDir), inNormal);
    vdotn = 1.0 - clamp(vdotn, 0, 1);
    vdotn = pow(vdotn, rayParam.power);
    outFragColor.xyz = rayParam.color * vdotn;
    outFragColor.w   = 1.0;

    vec3 normal = normalize(inNormal);

    // [-1, 1] -> [0, 1]
    normal       = (normal + 1) / 2;
    outNormal    = vec4(normal, 1.0);
}