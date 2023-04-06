#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 normal = normalize(inNormal);
    vec3 lightDir = vec3(0, 0, -1);
    float diffuse = dot(normal, lightDir);
    outColor = vec4(diffuse, diffuse, diffuse, 1.0);
}