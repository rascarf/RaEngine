#version 450

layout (set = 1 , binding = 0) uniform sampler2D DiffuseMap;

layout (location = 0) in vec2 inUV0;

layout (location = 0) out vec4 OutFragColor;

void main()
{
    OutFragColor = texture(DiffuseMap,inUV0);
}
