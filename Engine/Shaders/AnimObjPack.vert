#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV0;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3  inSkinPack;

layout (binding = 0) uniform MVPBlock
{
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
} uboMVP;

#define MAX_BONES 64

layout(binding = 1)uniform BonesTransformBlock
{
    mat4 Bones[MAX_BONES];
}BonesData;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec4 outColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

ivec4 UnPackUInt32To4Byte(uint packIndex)
{
    uint idx0 = (packIndex >> 24) & 0xFF;
    uint idx1 = (packIndex >> 16) & 0xFF;
    uint idx2 = (packIndex >> 8)  & 0xFF;
    uint idx3 = (packIndex >> 0)  & 0xFF;
    return ivec4(idx0, idx1, idx2, idx3);
}

ivec2 UnPackUInt32To2Short(uint packIndex)
{
    uint idx0 = (packIndex >> 16) & 0xFFFF;
    uint idx1 = (packIndex >> 0)  & 0xFFFF;
    return ivec2(idx0, idx1);
}

void main()
{
    // skin info
    ivec4 skinIndex   = UnPackUInt32To4Byte(uint(inSkinPack.x));
    ivec2 skinWeight0 = UnPackUInt32To2Short(uint(inSkinPack.y));
    ivec2 skinWeight1 = UnPackUInt32To2Short(uint(inSkinPack.z));
    vec4  skinWeight  = vec4(skinWeight0 / 65535.0, skinWeight1 / 65535.0);
    
    mat4 boneMatrix = BonesData.Bones[skinIndex.x] * skinWeight.x;
    boneMatrix += BonesData.Bones[skinIndex.y] * skinWeight.y;
    boneMatrix += BonesData.Bones[skinIndex.z] * skinWeight.z;
    boneMatrix += BonesData.Bones[skinIndex.w] * skinWeight.w;

    mat4 modeMatrix   = uboMVP.modelMatrix * boneMatrix;
    mat3 normalMatrix = transpose(inverse(mat3(modeMatrix)));

    vec3 normal = normalize(normalMatrix * inNormal.xyz);

    outUV       = inUV0;
    outNormal   = normal;
    outColor    = vec4(inSkinPack,1.0f);

    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * modeMatrix * vec4(inPosition.xyz, 1.0);
}