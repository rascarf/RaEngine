#version 450

layout (location = 0) in vec2 TextureCoordinate;

layout (set = 0 , binding = 0) uniform sampler2D InputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main()
{
    vec4 color = texture(InputImageTexture, TextureCoordinate);
    outFragColor = color;
}