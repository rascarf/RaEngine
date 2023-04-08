#pragma once
#include "Platform/Vulkan/VulkanBuffers/VulkanIndexBuffer.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanVertexBuffer.h"

class VulkanPrimitive
{
public:
    Ref<VulkanIndexBuffer> IndexBuffer = nullptr;
    Ref<VulkanVertexBuffer> VertexBuffer = nullptr;

    int32 VertexCount = 0;
    int32 IndexCount = 0;

    std::vector<float>  vertices;
    std::vector<float>  instanceDatas;
    std::vector<uint16> indices;

    int32   vertexCount = 0;
    int32   triangleNum = 0;

    VulkanPrimitive()
    {
        
    }

    ~VulkanPrimitive()
    {
        IndexBuffer = nullptr;
        VertexBuffer = nullptr;
    }

    void BindDraw(VkCommandBuffer CmdBuffer)
    {
        VertexBuffer->Bind(CmdBuffer);
        IndexBuffer->BindAndDraw(CmdBuffer);
    }
    
};
