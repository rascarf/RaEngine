#version 450
layout(location = 0) in vec3 inUVM;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube diffuseMap;

void main()
{
    vec3 color = textureLod(diffuseMap,inUVM,1.0).xyz;
    color = pow(color, vec3(1.0 / 2.2));
    outColor = vec4(color,1.0);
}