#pragma once

#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanCommandPool.h"

class VulkanVertexBuffer
{
public:
    ~VulkanVertexBuffer()
    {
        Buffer.reset();
        Buffer = nullptr;
    }

    void Bind(VkCommandBuffer cmdBuffer)
    {
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &(Buffer->Buffer), &Offset);
    }

    VkVertexInputBindingDescription GetInputBinding();

    std::vector<VkVertexInputAttributeDescription> GetInputAttributes(const std::vector<VertexAttribute>& shaderInputs = std::vector<VertexAttribute>());

    static Ref<VulkanVertexBuffer> Create(std::shared_ptr<VulkanDevice> device, Ref<VulkanCommandBuffer> cmdBuffer, std::vector<float> vertices, const std::vector<VertexAttribute>& attributes);
    
    VkDevice                        Device = VK_NULL_HANDLE;
    VkDeviceSize                    Offset = 0;
    std::vector<VertexAttribute>    Attributes;
    Ref<VulkanBuffer>               Buffer = nullptr;
};

class VulkanVertexFactory
{
public:
    ~VulkanVertexFactory()
    {
        for(auto& Pair : Streams)
        {
            Pair.second.reset();
            Pair.second = nullptr;
        }
    }

    void AddStream(std::shared_ptr<VulkanDevice> device , Ref<VulkanCommandBuffer> cmdBuffer, std::vector<float> vertices, const VertexAttribute& attributes);
    void Bind(VkCommandBuffer cmdBuffer,const std::vector<AttributeStream>& BindStream);
    
    VkDevice                        Device = VK_NULL_HANDLE;
    std::unordered_map<VertexAttribute,Ref<VulkanBuffer>> Streams;
};


