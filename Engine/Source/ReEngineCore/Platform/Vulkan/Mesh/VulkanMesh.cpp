#include "VulkanMesh.h"

#include "assimp/config.h"
#include <assimp/Importer.hpp>
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Math/Math.h"
#include "Resource/AssetManager/AssetManager.h"

using namespace std;

inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
{
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}


VkVertexInputBindingDescription VulkanModel::GetInputBinding()
{
    int32 stride = 0;
    for (int32 i = 0; i < Attributes.size(); ++i)
    {
        stride += VertexAttributeToSize(Attributes[i]);
    }  

    VkVertexInputBindingDescription vertexInputBinding = {};
    vertexInputBinding.binding   = 0;
    vertexInputBinding.stride    = stride;
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return vertexInputBinding;
}

std::vector<VkVertexInputAttributeDescription> VulkanModel::GetInputAttributes()
{
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
    int32 offset = 0;

    for (int32 i = 0; i < Attributes.size(); ++i)
    {
        VkVertexInputAttributeDescription inputAttribute = {};
        inputAttribute.binding  = 0;
        inputAttribute.location = i;
        inputAttribute.format   = VertexAttributeToVkFormat(Attributes[i]);
        inputAttribute.offset   = offset;
        offset += VertexAttributeToSize(Attributes[i]);
        vertexInputAttributs.push_back(inputAttribute);
    }

    return vertexInputAttributs;
}

void VulkanModel::LoadAnimations(const aiScene* aiScene)
{
    for(int32 i = 0 ;i < (int32)aiScene->mNumAnimations; i++)
    {
        aiAnimation* aianimation = aiScene->mAnimations[i];
        float timeTick = aianimation->mTicksPerSecond != 0 ? (float)aianimation->mTicksPerSecond : 25.0f;

        Animations.push_back(Animation());
        Animation& Animation = Animations.back();

        for(int32 j = 0 ; j < (int32)aianimation->mNumChannels ; ++j )
        {
            aiNodeAnim* nodeAnim = aianimation->mChannels[j];
            std::string NodeName = nodeAnim->mNodeName.C_Str();

            Animation.Clips.insert(std::make_pair(NodeName,AnimationClip()));
            
            AnimationClip& animClip = Animation.Clips[NodeName];
            animClip.NodeName = NodeName;
            animClip.Duration = 0.0f;

            for(int32 Index = 0 ; Index < (int32)nodeAnim->mNumPositionKeys;++Index)
            {
                aiVectorKey& aiKey = nodeAnim->mPositionKeys[Index];

                animClip.Positions.Keys.push_back((float)aiKey.mTime / timeTick);
                animClip.Positions.Values.push_back(glm::vec3(aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z));
                animClip.Duration = Math::Max((float)aiKey.mTime / timeTick,animClip.Duration);
            }

            // scale
            for (int32 index = 0; index < (int32)nodeAnim->mNumScalingKeys; ++index)
            {
                aiVectorKey& aikey = nodeAnim->mScalingKeys[index];
                animClip.Scales.Keys.push_back((float)aikey.mTime / timeTick);
                animClip.Scales.Values.push_back(glm::vec3(aikey.mValue.x, aikey.mValue.y, aikey.mValue.z));
                animClip.Duration = Math::Max((float)aikey.mTime / timeTick, animClip.Duration);
            }

            // rotation
            for (int32 index = 0; index < (int32)nodeAnim->mNumRotationKeys; ++index)
            {
                aiQuatKey& aikey = nodeAnim->mRotationKeys[index];
                animClip.Rotations.Keys.push_back((float)aikey.mTime / timeTick);
                animClip.Rotations.Values.push_back(GetGLMQuat(aikey.mValue));
                animClip.Duration = Math::Max((float)aikey.mTime / timeTick, animClip.Duration);
            }

            Animation.Duration = Math::Max(animClip.Duration,Animation.Duration);
        }
    }
}

void VulkanModel::LoadSkin(std::unordered_map<uint32, VertexSkin>& skinInfoMap, Ref<VulkanMesh> mesh,const aiMesh* aiMesh, const aiScene* aiScene)
{
    std::unordered_map<int32, int32> BoneIndexMap;

    for(int32 i = 0; i < (int32) aiMesh->mNumBones; ++i)
    {
        aiBone* boneInfo = aiMesh->mBones[i];
        std::string BoneName(boneInfo->mName.C_Str());

        int32 BoneIndex = BonesMap[BoneName].lock()->Index;

        //Bone 在Mesh中的索引
        int32 MeshBoneIndex = 0;
        auto it = BoneIndexMap.find(BoneIndex);
        if(it == BoneIndexMap.end())
        {
            MeshBoneIndex = (int32)mesh->Bones.size();
            mesh->Bones.push_back(BoneIndex);
            BoneIndexMap.insert(std::make_pair(BoneIndex,MeshBoneIndex));
        }
        else
        {
            MeshBoneIndex = it->second;
        }

        for(uint32 j = 0 ; j < boneInfo->mNumWeights ; ++j)
        {
            uint32 VertexID = boneInfo->mWeights[j].mVertexId;
            float Weight = boneInfo->mWeights[j].mWeight;

            if(skinInfoMap.find(VertexID) == skinInfoMap.end())
            {
                skinInfoMap.insert(std::make_pair(VertexID,VertexSkin()));
            }

            VertexSkin& Info = skinInfoMap[VertexID];
            Info.Indices[Info.Used] = MeshBoneIndex;
            Info.Weights[Info.Used] = Weight;
            Info.Used += 1 ;

            if(Info.Used >= 4 )
            {
                RE_INFO("Only Support 4 Weights for a vertex");
                break;
            }
        }

        for(auto it = skinInfoMap.begin(); it != skinInfoMap.end() ; ++it)
        {
            VertexSkin& Info = it->second;
            for(int32 i = Info.Used ; i < 4 ; ++i)
            {
                Info.Indices[i] = 0;
                Info.Weights[i] = 0.0f;
            }
        }
        
        mesh->IsSkin = true;
    }
}

Ref<VulkanModel> VulkanModel::LoadFromFile(const std::string& filename, Ref<VulkanDevice> vulkanDevice,Ref<VulkanCommandBuffer> cmdBuffer, const std::vector<VertexAttribute>& attributes)
{
    Ref<VulkanModel> model   = CreateRef<VulkanModel>();
    model->Device     = vulkanDevice;
    model->Attributes = attributes;
    model->CmdBuffer  = cmdBuffer;
        
    int assimpFlags = aiProcess_Triangulate | aiProcess_FlipUVs;
        
    for (int32 i = 0; i < attributes.size(); ++i)
    {
        if (attributes[i] == VertexAttribute::VA_Tangent)
        {
            assimpFlags = assimpFlags | aiProcess_CalcTangentSpace;
        }
        else if (attributes[i] == VertexAttribute::VA_UV0)
        {
            assimpFlags = assimpFlags | aiProcess_GenUVCoords;
        }
        else if (attributes[i] == VertexAttribute::VA_Normal)
        {
            assimpFlags = assimpFlags | aiProcess_GenSmoothNormals;
        }
        else if (attributes[i] == VertexAttribute::VA_SkinIndex)
        {
            model->loadSkin = true;
        }
        else if (attributes[i] == VertexAttribute::VA_SkinWeight)
        {
            model->loadSkin = true;
        }
        else if (attributes[i] == VertexAttribute::VA_SkinPack)
        {
            model->loadSkin = true;
        }
    }

    uint32 dataSize = 0;
    uint8* dataPtr  = nullptr;
    if (!AssetManager::ReadFile(filename, dataPtr, dataSize))
    {
        RE_CORE_ERROR("Can't Load File");
        return model;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFileFromMemory(dataPtr, dataSize, assimpFlags);

    model->LoadBones(scene);
    model->LoadNode(scene->mRootNode, scene);
    model->LoadAnimations(scene);
    
    delete[] dataPtr;

    return model;
}

Ref<VulkanModel> VulkanModel::Create(std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer,const std::vector<float>& vertices, const std::vector<uint16>& indices,const std::vector<VertexAttribute>& attributes)
{
    Ref<VulkanModel> model   = CreateRef<VulkanModel>();
    model->Device     = vulkanDevice;
    model->Attributes = attributes;
    model->CmdBuffer  = cmdBuffer;

    int32 stride = 0;
    for (int32 i = 0; i < attributes.size(); ++i)
    {
        stride += VertexAttributeToSize(attributes[i]);
    }

    Ref<VulkanPrimitive> primitive = CreateRef<VulkanPrimitive>();
    primitive->vertices     = vertices;
    primitive->indices      = indices;
    primitive->vertexCount  = (int32)vertices.size() / stride * 4;

    if (cmdBuffer)
    {
        if (vertices.size() > 0)
        {
            primitive->VertexBuffer = VulkanVertexBuffer::Create(vulkanDevice, cmdBuffer, primitive->vertices, attributes);
        }
        if (indices.size() > 0)
        {
            primitive->IndexBuffer = VulkanIndexBuffer::Create(vulkanDevice, cmdBuffer, primitive->indices);
        }
    }

    Ref<VulkanMesh> mesh = CreateRef<VulkanMesh>();
    mesh->m_Primitives.push_back(primitive);
    mesh->m_BoundingBox.Min = glm::vec3(-1.0f, -1.0f, 0.0f);
    mesh->m_BoundingBox.Max = glm::vec3(1.0f, 1.0f, 0.0f);
    
    Ref<VulkanMeshNode> rootNode = CreateRef<VulkanMeshNode>();
    rootNode->name = "RootNode";
    rootNode->Meshes.push_back(mesh);
    rootNode->LocalMatrix = glm::identity<glm::mat4>();
    mesh->LinkNode = rootNode;

    model->RootNode = rootNode;
    model->Meshes.push_back(mesh);

    return model;
}

Ref<VulkanMeshNode> VulkanModel::LoadNode(const aiNode* Innode, const aiScene* Inscene)
{
    Ref<VulkanMeshNode> Node = CreateRef<VulkanMeshNode>();
    Node->name = Innode->mName.C_Str();

    if(RootNode == nullptr)
    {
        RootNode = Node;
    }

    FillMatrixWithAiMatrix(Node->LocalMatrix,Innode->mTransformation);
    
    // mesh
    if(Innode->mNumMeshes > 0)
    {
        for(uint32 i = 0 ; i < Innode->mNumMeshes ; i++)
        {
            Ref<VulkanMesh> Mesh = LoadMesh(Inscene->mMeshes[Innode->mMeshes[i]],Inscene);
            Mesh->LinkNode = Node;
            Node->Meshes.push_back(Mesh);
            Meshes.push_back(Mesh);
        }
    }

    NodesMap.insert(std::make_pair(Node->name,Node));
    LinearNodes.push_back(Node);

    //Bone Parent
    int32 BoneParentIndex = -1;
    {
        auto it = BonesMap.find(Node->name);
        if(it != BonesMap.end())
        {
            BoneParentIndex = it->second.lock()->Index;
        }
    }
    
    // children node
    for (int32 i = 0; i < (int32)Innode->mNumChildren; ++i)
    {
        Ref<VulkanMeshNode> childNode = LoadNode(Innode->mChildren[i], Inscene);
        childNode->Parent  = std::weak_ptr(Node);
        Node->Children.push_back(childNode);

        //子节点
        {
            auto it = BonesMap.find(childNode->name);
            if (it != BonesMap.end())
            {
                it->second.lock()->Parent = BoneParentIndex;
            }
        }
    }

    return Node;
}

void VulkanModel::LoadBones(const aiScene* aiScene)
{
    std::unordered_map<std::string,int32> BoneIndexMap;

    for(int32 i = 0 ; i < (int32)aiScene->mNumMeshes;++i)
    {
        aiMesh* aimesh = aiScene->mMeshes[i];
        for(int32 BoneIndex = 0; BoneIndex < aimesh->mNumBones;BoneIndex++)
        {
            aiBone* AiBone = aimesh->mBones[BoneIndex];
            std::string name = AiBone->mName.C_Str();

            auto it = BoneIndexMap.find(name);
            if(it == BoneIndexMap.end())
            {
                int32 Index = (int32)Bones.size();
                Ref<Bone> bone = CreateRef<Bone>();

                bone->Index = Index;
                bone->Parent = -1;
                bone->Name = name;

                FillMatrixWithAiMatrix(bone->InverseBindPose,AiBone->mOffsetMatrix);

                Bones.push_back(bone);
                BonesMap.insert(std::make_pair(name,bone));
                BoneIndexMap.insert(std::make_pair(name,Index));
            }
        }
    }
}

Ref<VulkanMesh> VulkanModel::LoadMesh(const aiMesh* Inmesh, const aiScene* Inscene)
{
    Ref<VulkanMesh> Mesh = CreateRef<VulkanMesh>();

    // load material
    aiMaterial* material = Inscene->mMaterials[Inmesh->mMaterialIndex];
    if (material)
    {
        FillMaterialTextures(material, Mesh->Material);
    }

    // // load bones
    std::unordered_map<uint32, VertexSkin> skinInfoMap;
    if (Inmesh->mNumBones > 0 && loadSkin)
    {
        LoadSkin(skinInfoMap, Mesh, Inmesh, Inscene);
    }

    // load vertex data
    std::vector<float> vertices;
    glm::vec3 mmin( MAX_FLT,  MAX_FLT,  MAX_FLT);
    glm::vec3 mmax(-MAX_FLT, -MAX_FLT, -MAX_FLT);
    LoadVertexDatas(skinInfoMap,vertices, mmax, mmin, Mesh, Inmesh, Inscene);

    // load indices
    std::vector<uint32> indices;
    LoadIndices(indices, Inmesh, Inscene);

    // load primitives
    LoadPrimitives(vertices, indices, Mesh, Inmesh, Inscene);

    Mesh->m_BoundingBox.Min = mmin;
    Mesh->m_BoundingBox.Max = mmax;
    Mesh->m_BoundingBox.UpdateCorners();
    
    return Mesh;
}

void VulkanModel::UpdateAnimation(float DeltaTime)
{
    if(AnimIndex == -1)
    {
        RE_WARN("AnimIndex is a invalid value");
        return;
    }

    Animation& animation = Animations[AnimIndex];
    animation.Time += DeltaTime * animation.Speed;

    if(animation.Time >= animation.Duration)
    {
        animation.Time = animation.Time - animation.Duration;
    }

    EvaluateAnimation(animation.Time);
}

void VulkanModel::SetAnimation(int32 index)
{

    if (index >= Animations.size())
    {
        RE_WARN("AnimIndex is a invalid value");
        return;
    }
    
    if (index < 0)
    {
        RE_WARN("AnimIndex is a invalid value");
        return;
    }
    
    AnimIndex = index;
}

Animation& VulkanModel::GetAnimation(int32 index)
{
    if (index == -1)
    {
        RE_WARN("input Index is a invalid value, return current Animation");
        index = AnimIndex;
    }
    return Animations[index];
}

void VulkanModel::EvaluateAnimation(float time)
{
    if(AnimIndex == -1)
    {
        RE_WARN("AnimIndex is a invalid value");
        return;
    }

    Animation& animation = Animations[AnimIndex];
    animation.Time = Math::Clamp(time,0.0f,animation.Time);

    for(auto it = animation.Clips.begin(); it != animation.Clips.end() ; ++it)
    {
        AnimationClip& clip = it->second;
        Ref<VulkanMeshNode> node = NodesMap[clip.NodeName].lock();

        float alpha = 0.0f;
        // rotation
        glm::quat prevRot(1, 0, 0, 0);
        glm::quat nextRot(1, 0, 0, 0);
        clip.Rotations.GetValue(animation.Time, prevRot, nextRot, alpha);
        glm::quat retRot = glm::slerp(prevRot, nextRot, alpha);

        // position
        glm::vec3 prevPos(0, 0, 0);
        glm::vec3 nextPos(0, 0, 0);
        clip.Positions.GetValue(animation.Time, prevPos, nextPos, alpha);
        glm::vec3 retPos = glm::mix(prevPos, nextPos, alpha);
        glm::mat4 RotationMatrix = glm::toMat4(retRot);

        // scale
        glm::vec3 prevScale(1, 1, 1);
        glm::vec3 nextScale(1, 1, 1);
        clip.Scales.GetValue(animation.Time, prevScale, nextScale, alpha);
        glm::vec3 retScale = glm::mix(prevScale, nextScale, alpha);

        node->LocalMatrix = glm::identity<glm::mat4>();
        node->LocalMatrix = glm::scale(node->LocalMatrix,retScale);
        node->LocalMatrix = RotationMatrix * node->LocalMatrix;
        node->LocalMatrix = glm::translate(glm::identity<glm::mat4>(),retPos) * node->LocalMatrix;
        
    }

    // update bones
    for (int32 i = 0; i < Bones.size(); ++i)
    {
        Ref<Bone> bone = Bones[i];
        Ref<VulkanMeshNode> node = NodesMap[bone->Name].lock();
        
        bone->FinalTransform = bone->InverseBindPose;
        bone->FinalTransform = node->GetGlobalMatrix() * bone->FinalTransform;
    }
}

void VulkanModel::FillMatrixWithAiMatrix(glm::mat4x4& OutMatix, const aiMatrix4x4& from)
{

    OutMatix = glm::mat4x4{
        (double)from.a1, (double)from.b1, (double)from.c1, (double)from.d1,
        (double)from.a2, (double)from.b2, (double)from.c2, (double)from.d2,
        (double)from.a3, (double)from.b3, (double)from.c3, (double)from.d3,
        (double)from.a4, (double)from.b4, (double)from.c4, (double)from.d4};
}

void SimplifyTexturePath(std::string& path)
{
    const size_t lastSlashIdx = path.find_last_of("\\/");
    if (std::string::npos != lastSlashIdx)
    {
        path.erase(0, lastSlashIdx + 1);
    }

    const size_t periodIdx = path.rfind('.');
    if (std::string::npos != periodIdx)
    {
        path.erase(periodIdx);
    }
}

void VulkanModel::FillMaterialTextures(aiMaterial* aiMaterial, VulkanMaterialInfo& material)
{
    if (aiMaterial->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE))
    {
        aiString texturePath;
        aiMaterial->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &texturePath);
        material.Diffuse = texturePath.C_Str();
        SimplifyTexturePath(material.Diffuse);
    }

    if (aiMaterial->GetTextureCount(aiTextureType::aiTextureType_NORMALS))
    {
        aiString texturePath;
        aiMaterial->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &texturePath);
        material.Normal = texturePath.C_Str();
        SimplifyTexturePath(material.Normal);
    }

    if (aiMaterial->GetTextureCount(aiTextureType::aiTextureType_SPECULAR))
    {
        aiString texturePath;
        aiMaterial->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &texturePath);
        material.Specular = texturePath.C_Str();
        SimplifyTexturePath(material.Specular);
    }
}

void VulkanModel::LoadVertexDatas(std::unordered_map<uint32,VertexSkin>& skinInfoMap,std::vector<float>& vertices, glm::vec3& mmax, glm::vec3& mmin, Ref<VulkanMesh> mesh,const aiMesh* aiMesh, const aiScene* aiScene)
{
    glm::vec3 defaultColor(0.5,0.5,0.5);

    for (int32 i = 0; i < (int32)aiMesh->mNumVertices; ++i)
        {
            for (int32 j = 0; j < Attributes.size(); ++j)
            {
                if (Attributes[j] == VertexAttribute::VA_Position)
                {
                    float v0 = aiMesh->mVertices[i].x;
                    float v1 = aiMesh->mVertices[i].y;
                    float v2 = aiMesh->mVertices[i].z;

                    vertices.push_back(v0);
                    vertices.push_back(v1);
                    vertices.push_back(v2);

                    mmin.x = min(v0, mmin.x);
                    mmin.y = min(v1, mmin.y);
                    mmin.z = min(v2, mmin.z);
                    mmax.x = max(v0, mmax.x);
                    mmax.y = max(v1, mmax.y);
                    mmax.z = max(v2, mmax.z);
                }
                else if (Attributes[j] == VertexAttribute::VA_UV0)
                {
                    if (aiMesh->HasTextureCoords(0))
                    {
                        vertices.push_back(aiMesh->mTextureCoords[0][i].x);
                        vertices.push_back(aiMesh->mTextureCoords[0][i].y);
                    }
                    else
                    {
                        vertices.push_back(0);
                        vertices.push_back(0);
                    }
                }
                else if (Attributes[j] == VertexAttribute::VA_UV1)
                {
                    if (aiMesh->HasTextureCoords(1))
                    {
                        vertices.push_back(aiMesh->mTextureCoords[1][i].x);
                        vertices.push_back(aiMesh->mTextureCoords[1][i].y);
                    }
                    else
                    {
                        vertices.push_back(0);
                        vertices.push_back(0);
                    }
                }
                else if (Attributes[j] == VertexAttribute::VA_Normal)
                {
                    vertices.push_back(aiMesh->mNormals[i].x);
                    vertices.push_back(aiMesh->mNormals[i].y);
                    vertices.push_back(aiMesh->mNormals[i].z);
                }
                else if (Attributes[j] == VertexAttribute::VA_Tangent)
                {
                    vertices.push_back(aiMesh->mTangents[i].x);
                    vertices.push_back(aiMesh->mTangents[i].y);
                    vertices.push_back(aiMesh->mTangents[i].z);
                    vertices.push_back(1);
                }
                 else if (Attributes[j] == VertexAttribute::VA_SkinPack) //PackIndex
                {
                    if (mesh->IsSkin)
                    {
                        VertexSkin& skin = skinInfoMap[i]; 

                        int32 idx0 = skin.Indices[0];
                        int32 idx1 = skin.Indices[1];
                        int32 idx2 = skin.Indices[2];
                        int32 idx3 = skin.Indices[3]; 
                        uint32 packIndex = (idx0 << 24) + (idx1 << 16) + (idx2 << 8) + idx3;

                        uint16 weight0 = uint16(skin.Weights[0] * 65535);
                        uint16 weight1 = uint16(skin.Weights[1] * 65535);
                        uint16 weight2 = uint16(skin.Weights[2] * 65535);
                        uint16 weight3 = uint16(skin.Weights[3] * 65535);
                        uint32 packWeight0 = (weight0 << 16) + weight1;
                        uint32 packWeight1 = (weight2 << 16) + weight3;

                        vertices.push_back(packIndex);
                        vertices.push_back(packWeight0);
                        vertices.push_back(packWeight1);
                    }
                    else
                    {
                        vertices.push_back(0);
                        vertices.push_back(65535);
                        vertices.push_back(0);
                    }
                }
                else if (Attributes[j] == VertexAttribute::VA_SkinIndex)
                {
                    if (mesh->IsSkin)
                    {
                        VertexSkin& skin = skinInfoMap[i];
                        vertices.push_back((float)skin.Indices[0]);
                        vertices.push_back((float)skin.Indices[1]);
                        vertices.push_back((float)skin.Indices[2]);
                        vertices.push_back((float)skin.Indices[3]);
                    }
                    else
                    {
                        vertices.push_back(0);
                        vertices.push_back(0);
                        vertices.push_back(0);
                        vertices.push_back(0);
                    }
                }
                else if (Attributes[j] == VertexAttribute::VA_SkinWeight)
                {
                    if (mesh->IsSkin)
                    {
                        VertexSkin& skin = skinInfoMap[i];
                        vertices.push_back(skin.Weights[0]);
                        vertices.push_back(skin.Weights[1]);
                        vertices.push_back(skin.Weights[2]);
                        vertices.push_back(skin.Weights[3]);
                    }
                    else
                    {
                        vertices.push_back(1.0f);
                        vertices.push_back(0.0f);
                        vertices.push_back(0.0f);
                        vertices.push_back(0.0f);
                    }
                }
                else if (Attributes[j] == VertexAttribute::VA_Color)
                {
                    if (aiMesh->HasVertexColors(i))
                    {
                        vertices.push_back(aiMesh->mColors[0][i].r);
                        vertices.push_back(aiMesh->mColors[0][i].g);
                        vertices.push_back(aiMesh->mColors[0][i].b);
                    }
                    else
                    {
                        vertices.push_back(defaultColor.x);
                        vertices.push_back(defaultColor.y);
                        vertices.push_back(defaultColor.z);
                    }
                }
                else if (Attributes[j] == VertexAttribute::VA_Custom0 ||
                         Attributes[j] == VertexAttribute::VA_Custom1 ||
                         Attributes[j] == VertexAttribute::VA_Custom2 ||
                         Attributes[j] == VertexAttribute::VA_Custom3
                )
                {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }
            }
        }
}

void VulkanModel::LoadIndices(std::vector<uint32>& indices, const aiMesh* aiMesh, const aiScene* aiScene)
{
    for (int32 i = 0; i < (int32)aiMesh->mNumFaces; ++i)
    {
        indices.push_back(aiMesh->mFaces[i].mIndices[0]);
        indices.push_back(aiMesh->mFaces[i].mIndices[1]);
        indices.push_back(aiMesh->mFaces[i].mIndices[2]);
    }
}

void VulkanModel::LoadPrimitives(std::vector<float>& vertices, std::vector<uint32>& indices, Ref<VulkanMesh> mesh,const aiMesh* aiMesh, const aiScene* aiScene)
{
    int32 stride = (int32)vertices.size() / aiMesh->mNumVertices;

    //面数太多了要拆分
    if (indices.size() > 65535)
    {
        std::unordered_map<uint32, uint32> indicesMap;
        Ref<VulkanPrimitive> primitive = nullptr;

        for (int32 i = 0; i < indices.size(); ++i)
        {
            uint32 idx = indices[i];
            if (primitive == nullptr)
            {
                primitive = CreateRef<VulkanPrimitive>();
                indicesMap.clear();
                
                mesh->m_Primitives.push_back(primitive);
            }

            uint32 newIdx = 0;
            auto it = indicesMap.find(idx);
            if (it == indicesMap.end())
            {
                uint32 start = idx * stride;
                newIdx = (uint32)primitive->vertices.size() / stride;
                primitive->vertices.insert(primitive->vertices.end(), vertices.begin() + start, vertices.begin() + start + stride);
                indicesMap.insert(std::make_pair(idx, newIdx));
            }
            else
            {
                newIdx = it->second;
            }

            primitive->indices.push_back(newIdx);

            if (primitive->indices.size() == 65535)
            {
                primitive = nullptr;
            }
        }

        if (CmdBuffer)
        {
            for (int32 i = 0; i < mesh->m_Primitives.size(); ++i)
            {
                primitive = mesh->m_Primitives[i];
                primitive->VertexBuffer = VulkanVertexBuffer::Create(Device, CmdBuffer, primitive->vertices, Attributes);
                primitive->IndexBuffer  = VulkanIndexBuffer::Create(Device, CmdBuffer, primitive->indices);
            }
        }
    }
    else
    {
        Ref<VulkanPrimitive> primitive = CreateRef<VulkanPrimitive>();
        primitive->vertices = vertices;
        for (uint16 i = 0; i < indices.size(); ++i)
        {
            primitive->indices.push_back(indices[i]);
        }
        mesh->m_Primitives.push_back(primitive);

        if (CmdBuffer)
        {
            primitive->VertexBuffer = VulkanVertexBuffer::Create(Device, CmdBuffer, primitive->vertices, Attributes);
            primitive->IndexBuffer  = VulkanIndexBuffer::Create(Device, CmdBuffer, primitive->indices);
        }
    }

    for (int32 i = 0; i < mesh->m_Primitives.size(); ++i)
    {
        Ref<VulkanPrimitive> primitive = mesh->m_Primitives[i];
        primitive->vertexCount  = (int32)primitive->vertices.size() / stride;
        primitive->triangleNum  = (int32)primitive->indices.size() / 3;

        mesh->VertexCount   += primitive->vertexCount;
        mesh->TriangleCount += primitive->triangleNum;
    }
}
