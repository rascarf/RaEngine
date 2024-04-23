#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0,binding = 0) uniform MVPBlock
{
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
}uboMVP;

layout(location = 0) out vec3 outUVW; // Clip空间直接输出

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    outUVW = inPosition;
    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(inPosition , 1.0);
}