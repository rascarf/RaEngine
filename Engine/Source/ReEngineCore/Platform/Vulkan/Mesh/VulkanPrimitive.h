#pragma once
#include "Platform/Vulkan/VulkanBuffers/VulkanIndexBuffer.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanVertexBuffer.h"

class VulkanPrimitive
{
public:
    int32   vertexCount = 0;
    int32   triangleNum = 0;
    
    std::vector<uint32> indices;
    std::vector<float>  vertices;
    std::vector<float>  instanceDatas;
    
    std::unordered_map<VertexAttribute,std::vector<float>> AttributeDatas;
    
    Ref<VulkanIndexBuffer> IndexBuffer = nullptr;
    Ref<VulkanVertexBuffer> VertexBuffer = nullptr;
    Ref<VulkanVertexFactory> VertexFactory = nullptr;

    VulkanPrimitive()
    {
        vertices.clear();
        instanceDatas.clear();
        indices.clear();
        AttributeDatas.clear();
    }

    ~VulkanPrimitive()
    {
        if(VertexFactory)
        {
            VertexFactory.reset();
        }
        
        IndexBuffer = nullptr;
        VertexBuffer = nullptr;
    }

    void BindDraw(VkCommandBuffer CmdBuffer)
    {
        if(VertexBuffer)
        {
            VertexBuffer->Bind(CmdBuffer);
        }
        else
        {
            RE_ERROR("VertexBuffer is nullptr");    
        }
        
        if(IndexBuffer)
        {
            IndexBuffer->BindAndDraw(CmdBuffer);
        }
        else
        {
            vkCmdDraw(CmdBuffer, vertexCount, 1, 0, 0);
        }
    }

    void BindDraw(VkCommandBuffer CmdBuffer,const std::vector<AttributeStream>& AttributeStream)
    {
        if(VertexFactory)
        {
            VertexFactory->Bind(CmdBuffer,AttributeStream);
        }
        
        if(IndexBuffer)
        {
            IndexBuffer->BindAndDraw(CmdBuffer);
        }
        else
        {
            vkCmdDraw(CmdBuffer, vertexCount, 1, 0, 0);
        }
    }
    
};
