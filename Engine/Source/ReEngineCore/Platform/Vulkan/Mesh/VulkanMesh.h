#pragma once
#include "Animation/AnimationClip.h"
#include "Animation/Bone.h"
#include "assimp/material.h"
#include "assimp/matrix4x4.h"
#include "assimp/StringComparison.h"
#include "Core/Core.h"
#include "glm/ext/matrix_transform.hpp"
#include "Mesh/BoundingBox.h"
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/Mesh/VulkanPrimitive.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanTexture.h"

struct VulkanMeshNode;
struct aiNode;
struct aiScene;
struct aiMesh;

using namespace std;

struct VertexSkin
{
    int32 Used = 0;
    int32 Indices[4];
    float Weights[4]; // sum to 1.0f
};

struct VulkanMaterialInfo
{
    std::string Diffuse;
    std::string Normal;
    std::string Specular;
    std::string Metalic;
};

class VulkanMesh
{
public:
    typedef std::vector<Ref<VulkanPrimitive>> VulkanPrimitives;

    VulkanPrimitives m_Primitives;
    BoundingBox m_BoundingBox;
    std::weak_ptr<VulkanMeshNode> LinkNode;

    std::vector<int32> Bones;
    bool IsSkin = false;
    
    int32 VertexCount;
    int32 TriangleCount;

    VulkanMaterialInfo Material;

    VulkanMesh():LinkNode(),VertexCount(0),TriangleCount(0){}
    
    void BindDraw(VkCommandBuffer cmdBuffer)
    {
        for(auto& Primitive : m_Primitives)
        {
            Primitive->BindDraw(cmdBuffer);
        }
    }

    ~VulkanMesh(){}
};

//会保存节点的基本信息
//场景会由很多节点构成
//节点可能会有Mesh，也可能只是单纯的其他结构
class VulkanMeshNode
{
public:
    std::string			name;
    std::vector<Ref<VulkanMesh>> Meshes;

    std::weak_ptr<VulkanMeshNode> Parent;
    std::vector<Ref<VulkanMeshNode>> Children;
    
    glm::mat4 LocalMatrix;
    glm::mat4 GlobalMatrix;

    int32 Index;

    VulkanMeshNode():
    name("None"),
    Index(-1),
    LocalMatrix(glm::identity<glm::mat4>()),
    GlobalMatrix(glm::identity<glm::mat4>())
    {
        
    }

    ~VulkanMeshNode()
    {
        
    }

    const glm::mat4& GetLocalMatrix()
    {
        return LocalMatrix;
    }

    glm::mat4 GetGlobalMatrix()
    {
        GlobalMatrix = LocalMatrix;
        if (!Parent.expired())
        {
            GlobalMatrix = Parent.lock()->GetGlobalMatrix() * GlobalMatrix;
        }

        return GlobalMatrix;
    }

    void CalcBounds(BoundingBox& OutBounds)
    {
        if(Meshes.size() > 0 )
        {
            for (int32 i = 0; i < Meshes.size(); ++i)
            {
                const glm::mat4& matrix = GetGlobalMatrix();

                glm::vec4 TempMin(Meshes[i]->m_BoundingBox.Min.x,Meshes[i]->m_BoundingBox.Min.y,Meshes[i]->m_BoundingBox.Min.z,1.0);
                glm::vec4 mmin = TempMin * glm::transpose(matrix);

                glm::vec4 TempMax(Meshes[i]->m_BoundingBox.Max.x,Meshes[i]->m_BoundingBox.Max.y,Meshes[i]->m_BoundingBox.Max.z,1.0);
                glm::vec4 mmax = TempMax * glm::transpose(matrix);

                OutBounds.Min.x = min(OutBounds.Min.x, mmin.x);
                OutBounds.Min.y = min(OutBounds.Min.y, mmin.y);
                OutBounds.Min.z = min(OutBounds.Min.z, mmin.z);

                OutBounds.Max.x = max(OutBounds.Max.x, mmax.x);
                OutBounds.Max.y = max(OutBounds.Max.y, mmax.y);
                OutBounds.Max.z = max(OutBounds.Max.z, mmax.z);
            }
        }

        for (int32 i = 0; i < Children.size(); ++i)
        {
            Children[i]->CalcBounds(OutBounds);
        }
    }

    BoundingBox GetBounds()
    {
        BoundingBox Bounds;
        
        Bounds.Min = glm::vec3( MAX_FLT,  MAX_FLT,  MAX_FLT);
        Bounds.Max = glm::vec3(-MAX_FLT, -MAX_FLT, -MAX_FLT);
        
        CalcBounds(Bounds);
        Bounds.UpdateCorners();
        
        return Bounds;
    }
    
};

class VulkanModel
{
public:
    
    VulkanModel():Device(nullptr),RootNode(nullptr)
    {
    }
    
    ~VulkanModel()
    {
        LinearNodes.clear();
        Meshes.clear();
        
        CmdBuffer.reset();
    }

    VkVertexInputBindingDescription GetInputBinding();
    std::vector<VkVertexInputAttributeDescription> GetInputAttributes();

    static Ref<VulkanModel> LoadFromFile(const std::string& filename, Ref<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer, const std::vector<VertexAttribute>& attributes);
    static Ref<VulkanModel> Create(std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer, const std::vector<float>& vertices, const std::vector<uint16>& indices, const std::vector<VertexAttribute>& attributes);
        
    Ref<VulkanMeshNode> LoadNode(const aiNode* node, const aiScene* scene);
    Ref<VulkanMesh> LoadMesh(const aiMesh* mesh, const aiScene* scene);
    void LoadBones(const aiScene* aiScene);
    void LoadAnimations(const aiScene* aiScene);
    void LoadSkin(std::unordered_map<uint32, VertexSkin>& skinInfoMap, Ref<VulkanMesh> mesh, const aiMesh* aiMesh, const aiScene* aiScene);
    
    Ref<VulkanDevice>	Device;
    Ref<VulkanMeshNode>	RootNode;
    std::vector<Ref<VulkanMeshNode>> LinearNodes;
    std::vector<Ref<VulkanMesh>> Meshes;
    std::vector<VertexAttribute> Attributes;
    Ref<VulkanCommandBuffer>	CmdBuffer;

    std::vector<Ref<VulkanTexture>> AnimationTexture;

public:

    int32 AnimIndex = -1;
    std::vector<Ref<Bone>> Bones;
    std::vector<ReEngine::Animation> Animations;
    
    std::unordered_map<std::string,weak_ptr<Bone>> BonesMap;
    std::unordered_map<std::string,weak_ptr<VulkanMeshNode>> NodesMap;


    void UpdateAnimation(float DeltaTime);
    void SetAnimation(int32 index);
    Animation& GetAnimation(int32 index = -1);
    void EvaluateAnimation(float time);

    // -----创建AnimTexture-----
    std::vector<float> Keys;
    Ref<VulkanTexture> GenerateAnimationTexture(int index);
    void GetKeys(int index);
    int AnimationIndex();

public:
    bool  loadSkin = false;
    
    void FillMatrixWithAiMatrix(glm::mat4x4& OutMatix,const aiMatrix4x4& aiMatrix);
    void FillMaterialTextures(aiMaterial* aiMaterial, VulkanMaterialInfo& material);
    
    void LoadVertexDatas(std::unordered_map<uint32,VertexSkin>& skinInfoMap,std::vector<float>& vertices, glm::vec3& mmax, glm::vec3& mmin, Ref<VulkanMesh> mesh, const aiMesh* aiMesh, const aiScene* aiScene);
    void LoadIndices(std::vector<uint32>& indices, const aiMesh* aiMesh, const aiScene* aiScene);
    void LoadPrimitives(std::vector<float>& vertices, std::vector<uint32>& indices, Ref<VulkanMesh> mesh,const aiMesh* aiMesh, const aiScene* aiScene);
};

