#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;
//layout (location = 1) out vec4 outNormal;

layout (set = 1,binding = 0) uniform sampler2D DiffuseMap;

void main() 
{
    vec3 normal = normalize(inNormal);

    // [-1, 1] -> [0, 1]
    normal       = (normal + 1) / 2;
//    outNormal    = vec4(normal, 1.0);

    vec3 lightDir = vec3(0, 1, -1);
    vec4 diffuse  = texture(DiffuseMap, inUV);
    diffuse.xyz   = dot(lightDir, inNormal) * diffuse.xyz;
    outFragColor = diffuse;
//    outFragColor = vec4(1.0f,1.0f,1.0f,1.0f);
}