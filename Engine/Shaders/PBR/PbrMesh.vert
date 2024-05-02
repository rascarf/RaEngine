#version 450 core

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inTangent;
layout(location=3) in vec3 inBitangent;
layout(location=4) in vec2 inUV0;

layout(set = 0,binding = 0) uniform MVPBlock
{
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
}uboMVP;

layout(location=0) out Vertex
{
	vec3 position;
	vec2 texcoord;
	mat3 tangentBasis;
} vout;

void main()
{
    vout.position = vec3(uboMVP.modelMatrix * vec4(inPosition,1.0));
    vout.texcoord = vec2(inUV0.x,inUV0.y);

    vout.tangentBasis = mat3(uboMVP.modelMatrix) * mat3(inTangent, inBitangent, inNormal);
    
    gl_Position =  uboMVP.projectionMatrix * uboMVP.viewMatrix * vec4(vout.position,1.0);
}