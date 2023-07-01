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
    vec4 AnimIndex;
} uboMVP;

layout(set = 1,binding = 1) uniform sampler2D BonesTexture;


layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec4 outColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

mat4 ReadBoneAnim(int BoneIndex, int StartIndex)
{
    int index = StartIndex + BoneIndex * 4;
    int animY = index / int(uboMVP.AnimIndex.x);
    int animX = index % int(uboMVP.AnimIndex.x);
    
    vec2 Index0 = vec2(animX + 0.0f , animY) / uboMVP.AnimIndex.xy;
    vec2 Index1 = vec2(animX + 1.0f , animY) / uboMVP.AnimIndex.xy;
    vec2 Index2 = vec2(animX + 2.0f , animY) / uboMVP.AnimIndex.xy;
    vec2 Index3 = vec2(animX + 3.0f , animY) / uboMVP.AnimIndex.xy;
    
    mat4 AnimData;
    AnimData[0] = texture(BonesTexture,Index0);
    AnimData[1] = texture(BonesTexture,Index1);
    AnimData[2] = texture(BonesTexture,Index2);
    AnimData[3] = texture(BonesTexture,Index3);
    
    return AnimData;
}

mat4 GetBoneTransform(ivec4 SkinIndexs,vec4 SkinWeight,int StartIndex)
{
    mat4 Bone1Transform = (ReadBoneAnim(SkinIndexs.x,StartIndex)) ;
    mat4 Bone2Transform = (ReadBoneAnim(SkinIndexs.y,StartIndex));
    mat4 Bone3Transform = (ReadBoneAnim(SkinIndexs.z,StartIndex));
    mat4 Bone4Transform = (ReadBoneAnim(SkinIndexs.w,StartIndex));

    mat4 boneMatrix = Bone1Transform * SkinWeight.x;
    boneMatrix += Bone2Transform * SkinWeight.y;
    boneMatrix += Bone3Transform * SkinWeight.z;
    boneMatrix += Bone4Transform * SkinWeight.w;
    
    return boneMatrix;
}

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

    mat4 boneMatrix = GetBoneTransform(skinIndex,skinWeight,int(uboMVP.AnimIndex.z));

    mat4 modeMatrix   = uboMVP.modelMatrix * boneMatrix;
    mat3 normalMatrix = transpose(inverse(mat3(modeMatrix)));

    vec3 normal = normalize(normalMatrix * inNormal.xyz);

    outUV       = inUV0;
    outNormal   = normal;
    outColor    = vec4(inSkinPack,1.0f);

    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * modeMatrix * vec4(inPosition.xyz, 1.0);
}