﻿#pragma once
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "glm/glm.hpp"
#include "imgui.h"
#include "Event/Event.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace ReEngine
{
    class VulkanContext;
    
    class VulkanImGui
    {
    public:
        VulkanImGui();

        virtual ~VulkanImGui();
    
    protected:
    
        struct UIBuffer
        {
            VkBuffer        buffer;
            VkDeviceMemory  memory;
            VkDevice        device;
            void*           mapped;
            VkDeviceSize    size;
            VkDeviceSize    alignment;

            UIBuffer()
                : buffer(VK_NULL_HANDLE)
                , memory(VK_NULL_HANDLE)
                , device(VK_NULL_HANDLE)
                , mapped(nullptr)
                , size(0)
                , alignment(0)
            {

            }

            void Unmap()
            {
                if (mapped)
                {
                    vkUnmapMemory(device, memory);
                    mapped = nullptr;
                }
            }

            VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
            {
                return vkMapMemory(device, memory, offset, size, 0, &mapped);
            }

            void CopyFrom(void* data, VkDeviceSize size)
            {
                memcpy(mapped, data, size);
            }

            VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
            {
                VkMappedMemoryRange mappedRange = {};
                mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                mappedRange.memory = memory;
                mappedRange.offset = offset;
                mappedRange.size   = size;
                return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
            }

            void Destroy()
            {
                if (buffer)
                {
                    vkDestroyBuffer(device, buffer, VULKAN_CPU_ALLOCATOR);
                    buffer = VK_NULL_HANDLE;
                }

                if (memory)
                {
                    vkFreeMemory(device, memory, VULKAN_CPU_ALLOCATOR);
                    memory = VK_NULL_HANDLE;
                }

                device = VK_NULL_HANDLE;
            }
        };

        struct PushConstBlock
        {
            glm::vec2 scale;
            glm::vec2 translate;
        };

    public:

        void Init(const std::string& font,VulkanContext * Context);

        void Destroy();

        void Resize(uint32 width, uint32 height);

        bool Update();

        void StartFrame();

        void EndFrame();

        void BindDrawCmd(const VkCommandBuffer& commandBuffer, const VkRenderPass& renderPass, int32 subpass = 0, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

        void OnEvent(std::shared_ptr<Event> e);

        FORCE_INLINE float GetScale() const
        {
            return m_Scale;
        }

    protected:

        void PrepareFontResources();

        void PreparePipelineResources();

        void PreparePipeline(VkRenderPass renderPass, int32 subpass, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

        void CreateBuffer(UIBuffer& buffer, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size);

    protected:

        typedef std::shared_ptr<VulkanDevice> VulkanDeviceRef;

        VulkanDeviceRef         m_VulkanDevice;

        UIBuffer                m_VertexBuffer;
        UIBuffer                m_IndexBuffer;
        int32                   m_VertexCount;
        int32                   m_IndexCount;

        int32                   m_Subpass;

        VkDescriptorPool        m_DescriptorPool;
        VkDescriptorSetLayout   m_DescriptorSetLayout;
        VkDescriptorSet         m_DescriptorSet;
        VkPipelineLayout        m_PipelineLayout;

        VkPipelineCache         m_PipelineCache;
        VkPipeline              m_Pipeline;

        VkRenderPass            m_LastRenderPass;
        int32                   m_LastSubPass = -1;
        VkSampleCountFlagBits   m_LastSampleCount = VK_SAMPLE_COUNT_1_BIT;

        VkDeviceMemory          m_FontMemory;
        VkImage                 m_FontImage;
        VkImageView             m_FontView;
        VkSampler               m_FontSampler;

        PushConstBlock          m_PushData;

        bool                    m_Visible;
        bool                    m_Updated;
        float                   m_Scale;

        std::string             m_FontPath;
    };
}