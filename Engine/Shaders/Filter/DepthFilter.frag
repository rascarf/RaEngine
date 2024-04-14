#version 450

layout (location = 0) in vec2 TextureCoordinate;

layout (set = 0 , binding = 0) uniform sampler2D InputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main()
{
    vec4 color = vec4(0,0,0,1);
    color.r = texture(InputImageTexture, TextureCoordinate).r;

    color.r = (color.r - 0.995) / 0.005;
    outFragColor = color;
}