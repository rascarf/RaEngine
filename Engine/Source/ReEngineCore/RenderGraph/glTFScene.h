#pragma once
#include "RenderScene.h"
#include "SceneGraph.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace ReEngine
{
    struct glTFMaterial
    {
        Ref<VulkanMaterial> Material;
        Ref<VulkanBuffer> MaterialBuffer;

        // Indices Used for Bindless Textures
        uint16      DiffuseTextureIndex;
        uint16      RoughnessTextureIndex;
        uint16      NormalTextureIndex;
        uint16      OcclusionTextureIndex;

        glm::vec4   BaseColorFactor;
        glm::vec4   MetallicRoughnessOcclusionFactor;

        float       AlphaCutoff;
        uint32      Flags;
    };

    struct glTFMesh
    {
        glTFMaterial        PbrMaterial;

        Ref<VulkanBuffer>   IndexBuffer;
        Ref<VulkanBuffer>   PositionBuffer;
        Ref<VulkanBuffer>   TangentBuffer;
        Ref<VulkanBuffer>   NormalBuffer;
        Ref<VulkanBuffer>   TexcoordBuffer;

        uint32              PositionOffset;
        uint32              TangentOffset;
        uint32              NormalOffset;
        uint32              TexcoordOffset;

        VkIndexType         IndexType;
        uint32              IndexOffset;

        uint32              PrimitiveCout;
        uint32              SceneGraphNodeIndex = UINT32_MAX;

        bool                IsTransparent() const { return ( PbrMaterial.Flags & ( DrawFlags_AlphaMask | DrawFlags_Transparent ) ) != 0;}
        bool                IsDoubleSided() const { return ( PbrMaterial.Flags & DrawFlags_DoubleSided ) == DrawFlags_DoubleSided; }
    };

    // Drawable Mesh
    struct MeshInstance
    {
        Ref<glTFMesh>       Mesh;
        uint32              MaterialPassIndex;
    };

    struct GpuMeshData
    {
        glm::mat4       World;
        glm::mat4       InverseWorld;

        uint32          Textures[4]; // diffuse, roughness, normal, occlusion
        glm::vec4       BaseColorFactor;
        glm::vec4       Metallic_Roughness_Occlusion_Factor; //metallic, roughness, occlusion

        float           AlphaCutoff;
        float           Padding[3];

        uint32          Flags;
        uint32          Padding_1[3];
    };
    
    // class glTFScene : public RenderScene
    // {
    // public:
    //     virtual void Init(cstring FileName, cstring Path) override{};
    //     virtual void ShutDown() override{};
    //
    //     virtual void UploadMaterials() override{};
    //     virtual void PrepareDraws() override{};
    //
    //     virtual void SubmitDrawTask() override{};
    //
    // public:
    //     void DrawMesh(Ref<VulkanCommandBuffer> Command,glTFMesh& Mesh){};
    //
    // public:
    //     void GetMeshVertexBuffer(int32_t AccessorIndex, Ref<VulkanBuffer> OutBuffer, uint32_t& OutBufferOffset){};
    //     // uint16 GetMaterialTexture(VulkanContext* Gpu, gltf::TextureInfo* texture_info);
    //     // uint16 GetMaterialTexture(VulkanContext* Gpu, int32 texture_info);
    //
    //     // void FillGltfMaterial()
    //     
    //     
    // };
}

