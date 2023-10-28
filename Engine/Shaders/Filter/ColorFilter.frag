#version 450

layout (location = 0) in vec2 TextureCoordinate;

layout (set = 0 , binding = 0) uniform sampler2D InputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main()
{
    vec4 color = texture(InputImageTexture, TextureCoordinate);

    // gamma
	color.xyz = pow(color.xyz, vec3(1.0 / 2.2));
    outFragColor = color;
}