#pragma once
#include "Core/Core.h"
#include "glm/vec4.hpp"
#include "Light.h"
#include "tiny_gltf.h"
#include "Mesh/BoundingBox.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanTexture.h"
#include "Platform/Vulkan/RayTracing/AccelerationStruct.h"

namespace  ReEngine
{
    struct Material
    {
        glm::vec4 Albedo = glm::vec4(1.0,1.0,1.0,1.0);
        glm::vec4 Params = glm::vec4(1.0,1.0,1.0,1.0);
        glm::vec4 TextureID = glm::vec4(1.0,1.0,1.0,1.0);
    };

    struct glTFMesh
    {
        uint32 VertexCount = 0;
        uint32 VertexStride = 0;
        Ref<VulkanBuffer> VertexBuffer = nullptr;

        uint32 IndexCount = 0;
        Ref<VulkanBuffer> IndexBuffer = nullptr;

        VkFormat VertexPositionFormat = VK_FORMAT_R32G32B32_SFLOAT;
        VkIndexType MeshIndexFormat = VK_INDEX_TYPE_UINT32;

        int32 Material = -1;
        BoundingBox Bounds;

        ~glTFMesh()
        {
            VertexBuffer.reset();
            IndexBuffer.reset();
        }
    };

    struct Node
    {
        std::string Name;
        glm::mat4 Transform;

        int32 MeshIndex = -1;

        Node* Parent = nullptr;
        std::vector<Node*> Children;

        glm::mat4 GetWorldTransform()
        {
            glm::mat4 WorldMatrix = Transform;
            if(Parent)
            {
                WorldMatrix = Parent->GetWorldTransform() * WorldMatrix;
            }

            return WorldMatrix;
        }
    };

    struct glTFScene
    {
    public:
        Node* RootNode = nullptr;

        std::vector<Node*> Nodes;
        std::vector<Ref<VulkanTexture>> Textures;
        std::vector<Material> Materials;
        std::vector<glTFMesh*> Meshes;
    
        std::vector<Node*> Entities; // Meshes + Node = Entities , so Entities >= Meshes , for TLAS 

        std::vector<Light*> Lights;
        std::vector<LightData> LightDatas;

        Ref<VulkanDevice> Device;
        Ref<VulkanCommandPool> CmdBufferPool;

    public:
        std::vector<AccelKHR> Blases;
        AccelKHR Tla;
        
        // Mesh -> Blas
        void CreateBlas();
        
        // Create all the BLAS from the vector of BlasInput
        void BuildBlas(const std::vector<BlasInput>& input, VkBuildAccelerationStructureFlagsKHR flags);
        
        // Actual allocation of buffer and acceleration structure.
        void CmdCreateBlas(VkCommandBuffer cmdBuffer,std::vector<uint32_t> indices,std::vector<BuildAccelerationStructure>& buildAs, VkDeviceAddress scratchAddress,VkQueryPool queryPool);
        void CmdCreateCompact(VkCommandBuffer cmdBuffer,std::vector<uint32_t> indices,std::vector<BuildAccelerationStructure>& buildAs,VkQueryPool queryPool);


        // Entities -> Tlas
        void CreateTlas();

        // Create all the Tlas from the vector of RayInstance
        void BuildTlas(std::vector<VkAccelerationStructureInstanceKHR>& instances,VkBuildAccelerationStructureFlagsKHR flags, bool update);
        void CmdBuildTlas(VkCommandBuffer cmdBuffer, uint32_t CountInstance,Ref<VulkanBuffer>& ScratchBuffer,VkDeviceAddress InstBufferAddr,VkBuildAccelerationStructureFlagsKHR Flags,bool update);
        
    public:
        void LoadglTFModel(Ref<VulkanDevice> device,Ref<VulkanCommandPool> CommandPool,std::string FilePath, bool bIsBinary = true);

        void LoadTextures(Ref<VulkanCommandBuffer> cmdBuffer,tinygltf::Model &gltfModel);
        void LoadMaterials(Ref<VulkanCommandBuffer> cmdBuffer,tinygltf::Model &gltfModel);
        void LoadMeshes(Ref<VulkanCommandBuffer> cmdBuffer,tinygltf::Model &gltfModel);

        // LoadLights() -> UpdateLightData()
        void LoadLights(Ref<VulkanCommandBuffer> cmdBuffer,tinygltf::Model &gltfModel);
        
        void LoadNodes(Ref<VulkanCommandBuffer> cmdBuffer,tinygltf::Model &gltfModel);
        void LoadNode(Node* parent, tinygltf::Node& gltfNode, tinygltf::Model &gltfModel);
        
        void UpdateLightData()
        {
            LightDatas.resize(Lights.size());

            for (int32 i = 0; i < Lights.size(); ++i)
            {
                Light* light = Lights[i];
                if (light->type == LIGHT_TYPE_DIRECTIONAL)
                {
                    LightDatas[i].SetLight((DirectionalLight*)light);
                }
                else if (light->type == LIGHT_TYPE_POINT)
                {
                    LightDatas[i].SetLight((PointLight*)light);
                }
                else if (light->type == LIGHT_TYPE_RECT)
                {
                    LightDatas[i].SetLight((RectLight*)light);
                }
                else if (light->type == LIGHT_TYPE_SKY)
                {
                    LightDatas[i].SetLight((SkyEnvLight*)light);
                }
                else if (light->type == LIGHT_TYPE_SPOT)
                {
                    LightDatas[i].SetLight((SpotLight*)light);
                }
            }
        }
        
        void Destory()
        {
            if(RootNode)
            {
                delete RootNode;
            }
            
            for (int32 i = 0; i < Nodes.size(); ++i)
            {
                delete Nodes[i];
            }
            Nodes.clear();

            for (int32 i = 0; i < Textures.size(); ++i)
            {
                Textures[i].reset();
            }
            Textures.clear();

            for (int32 i = 0; i < Meshes.size(); ++i)
            {
                delete Meshes[i];
            }
            Meshes.clear();

            for (int32 i = 0; i < Lights.size(); ++i)
            {
                delete Lights[i];
            }
            Lights.clear();
        }
    };
}

