#version 450

layout (location = 0) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outNormal;

void main() 
{
    vec3 normal = normalize(inNormal);

    // [-1, 1] -> [0, 1]
    normal       = (normal + 1) / 2;
    
    outNormal    = vec4(normal, 1.0);
    outFragColor = vec4(1.0, 1.0, 1.0, 1.0);
}